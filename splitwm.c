/* See LICENSE file for copyright and license details */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>
#include <unistd.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#define LENGTH(X)	(sizeof(X)/sizeof(X[0]))
#define BUTTONMASK	ButtonPressMask|ButtonReleaseMask
#define DESKTOPS_LEFT	(sizeof(tags_left) / sizeof(tags_left[0]))
#define DESKTOPS_RIGHT	(sizeof(tags_right) / sizeof(tags_right[0]))
#define VIEWS		(sizeof(tags_views) / sizeof(tags_views[0]))

enum { MOVE, RESIZE };
enum { LEFT, RIGHT };
enum { TILE, FLOAT };
enum { WM_PROTOCOLS, WM_DELETE_WINDOW, WM_COUNT };

/** Structures **/
typedef union {
	const int i;
	const char **com;
} Arg;
	
typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *);
	const Arg arg;
} Button;

typedef struct Client {
	struct Client *next;
	struct Client *prev;
	char *title;
	int title_len;
	Window win;
} Client;

typedef struct {
	Client *head;
	Client *curr;
	int master_size;
	int layout;
} Desktop;

/** Function prototypes **/
static void activate_both_views(const Arg *arg);
static void activate_left_view(const Arg *arg);
static void activate_right_view(const Arg *arg);
static void addwindow(Window w);
static void change_left_desktop(const Arg *arg);
static void change_right_desktop(const Arg *arg);
static void change_view(const Arg *arg);
static void client_to_desktop(const Arg *arg);
static void client_to_view(const Arg *arg);
static void draw(void);
static void draw_bar(void);
static void draw_rectangle(const char *fill_color, int x, int y, int w, int h);
static void draw_separator(void);
static void draw_string(const char *string_color, const char *string, int x, int h);
static void draw_tags(void);
static void draw_title(void);
static void die(const char *errstr, ...);
static void focuscurrent(void);
static void fullscreen(const Arg *arg);
static unsigned long getcolor(const char *color);
Desktop *get_current_desktop(void);
static void grabbuttons(void);
static void grabkeys(void);
static void kill_client(const Arg *arg);
static void master_size_increase(const Arg *arg);
static void master_size_decrease(const Arg *arg);
static void maximize(Window w);
static void maximize_current(const Arg *arg);
static void mousemove(const Arg *arg);
static void move_resize_window(Window win, int x, int y, int w, int h);
static void nextview(const Arg *arg);
static void nextwindow(const Arg *arg);
static void prepare_draw(void);
static void previous_desktop(const Arg *arg);
static void previous_view(const Arg *arg);
static void printstatus(void);
static void removewindow(Window w);
static void run(void);
static void send_kill_signal(Window w);
static void separator_increase(const Arg *arg);
static void separator_decrease(const Arg *arg);
static void setup(void);
static void show_or_hide_bar(const Arg *arg);
static void sigchld(int sig);
static void spawn(const Arg *arg);
static void tile(Desktop *d);
static void tile_current(const Arg *arg);
static void toggle_float(const Arg *arg);
static void quit(const Arg *arg);
static int xerror(Display *dis, XErrorEvent *ee);

static void buttonpress(XEvent *e);
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void keypress(XEvent *e);
static void maprequest(XEvent *e);

/** Include config **/
#include "config.h"

/* view struct */
typedef struct {
	Desktop ld[DESKTOPS_LEFT+1];	/* left desktops */
	Desktop rd[DESKTOPS_RIGHT+1];	/* right desktops */
	int curr_left_id;
	int prev_left_id;
	int curr_right_id;
	int prev_right_id;
	int curr_desk;		/* current view, LEFT/RIGHT */
	float split_width_x;
	float split_height_y;
	Bool both_views_activated;
	Bool left_view_activated;
	Bool right_view_activated;
} View;

/** Variables **/
static int bar_height;
static int sh;
static int sw;
static int screen;
static int font_height;
static int cv_id;	/* current view ID */
static int pv_id;	/* previous view ID */
static unsigned int def_left;
static unsigned int def_right;
static unsigned int def_view;
static unsigned int left_win_unfocus;
static unsigned int right_win_unfocus;
static unsigned int win_focus;
static const unsigned int bottom_font_offset = 5;
static const unsigned int min_window_size = 20;
static const unsigned int top_font_offset = 3;
static Pixmap bar_buffer;
static Pixmap separator_buffer;
static Colormap color_map;
static Display *dpy;
static XFontStruct *font_struct;
static GC gc;
static Window bar;
static Window separator;
static Window root;
static Bool running = True;
static Atom wmatoms[WM_COUNT];
static View views[VIEWS+1];
static XSetWindowAttributes wa;

/** Event handlers **/
static void (*events[LASTEvent]) (XEvent *e) = {
	[ButtonPress]      = buttonpress,
	[ConfigureRequest] = configurerequest,
	[DestroyNotify]    = destroynotify,
	[EnterNotify]      = enternotify,
	[Expose]           = expose,
	[KeyPress]         = keypress,
	[MapRequest]       = maprequest
};

/** Function definitions **/
void die(const char *errstr, ...)
{
	va_list ap;
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void grabbuttons(void)
{
	unsigned int i;

	for (i = 0; i < LENGTH(buttons); i++)
		XGrabButton(dpy, buttons[i].button, buttons[i].mask, DefaultRootWindow(dpy),
			    False, BUTTONMASK, GrabModeAsync, GrabModeAsync, None, None);
}

unsigned long getcolor(const char *color)
{
	Colormap cm = DefaultColormap(dpy, screen);
	XColor xc;

	if (!XAllocNamedColor(dpy, cm, color, &xc, &xc))
		die("error: cannot get color\n");
	return xc.pixel;
}

void grabkeys(void)
{
	unsigned int i;
	KeyCode code;

	for (i = 0; i < LENGTH(keys); i++)
		if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
			XGrabKey(dpy, code, keys[i].mod, root, True,
				 GrabModeAsync, GrabModeAsync);
}

void keypress(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ke;

	ke = &e->xkey;
	keysym = XkbKeycodeToKeysym(dpy, (KeyCode)ke->keycode, 0, 0);
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].keysym && keys[i].mod == ke->state && keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void buttonpress(XEvent *e)
{
	/* DBG */	fprintf(stderr, "buttonpress(): IN\n");
	unsigned int i;
	Bool client_found = False;
	Desktop *d = NULL;
	Client *c = NULL;
	XButtonEvent ev = e->xbutton;

	if (click_focus) {
		if (!(d = get_current_desktop()))
			return;
		for (c = d->head; c; c = c->next) {
			if (c->win == ev.subwindow) {
				d->curr = c;
				client_found = True;
				focuscurrent();
			}
		}
		if (!client_found && ev.subwindow != root) {
			nextview(0);
			if (!(d = get_current_desktop()))
				return;
			for (c = d->head; c; c = c->next) {
				if (c->win == ev.subwindow) {
					d->curr = c;
					focuscurrent();
					break;
				}
			}
		}
	}
	draw();
	
	for (i = 0; i < LENGTH(buttons); i++) {
		if ((buttons[i].mask == e->xbutton.state) &&
		    (buttons[i].button == e->xbutton.button) && buttons[i].func)
			buttons[i].func(&(buttons[i].arg));
	}
	/* DBG */	fprintf(stderr, "buttonpress(): OUT\n");
}

void enternotify(XEvent *e)
{
	/* DBG */	fprintf(stderr, "enternotify(): IN\n");
	if (!follow_mouse_focus)
		return;
	Client *c = NULL;
	Desktop *d = NULL;
	Bool client_found = False;
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	if (!(d = get_current_desktop()))
		return;

	for (c = d->head; c; c = c->next) {
		if (c->win == ev->window) {
			fprintf(stderr, "enternotify(): FOCUSCURRENT()\n");
			client_found = True;
			d->curr = c;
			focuscurrent();
			return;
		}
	}

	if (!client_found && ev->window != root) {
		fprintf(stderr, "enternotify(): NEXTVIEW()\n");
		nextview(0);
		if (!(d = get_current_desktop()))
			return;
		for (c = d->head; c; c = c->next) {
			if (c->win == ev->window) {
				d->curr = c;
				focuscurrent();
				return;
			}
		}
	}
	/* DBG */	fprintf(stderr, "enternotify(): OUT\n");
}

void expose(XEvent *e)
{
	/* DBG */	fprintf(stderr, "expose(): IN\n");
	if (!show_bar)
		return;
	draw_bar();
	draw_tags();

	XCopyArea(dpy, bar_buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
	XRaiseWindow(dpy, bar);
	XFlush(dpy);
	/* DBG */	fprintf(stderr, "expose(): OUT\n");
}

Desktop *get_current_desktop(void)
{
	if (views[cv_id].curr_desk == LEFT) {
		return &views[cv_id].ld[views[cv_id].curr_left_id];
	} else if (views[cv_id].curr_desk == RIGHT) {
		return &views[cv_id].rd[views[cv_id].curr_right_id];
	} else {
		return NULL;
	}
}

/* REMAKE THIS */
void printstatus(void)
{
	unsigned int i;

	for (i = 0; i < DESKTOPS_LEFT; i++) {
		fprintf(stderr, "LEFT DESKTOP %d: ", i);
		Desktop *d = &views[cv_id].ld[i];
		Client *c = d->head;
		
		if (!c) {
			fprintf(stderr, ".\n");
			continue;
		}

		for ( c; c; c = c->next)
			if (c == d->curr && d->curr) {
				fprintf(stderr, "M ");
			} else {
				fprintf(stderr, "C ");
			}
		fprintf(stderr, "\n");
	}

	for (i = 0; i < DESKTOPS_RIGHT; i++) {
		fprintf(stderr, "RIGHT DESKTOP %d: ", i);
		Desktop *d = &views[cv_id].rd[i];
		Client *c = d->head;
		
		if (!c) {
			fprintf(stderr, ".\n");
			continue;
		}

		for ( c; c; c = c->next)
			if (c == d->curr && d->curr) {
				fprintf(stderr, "M ");
			} else {
				fprintf(stderr, "C ");
			}
		fprintf(stderr, "\n");
	}
}

void kill_client(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "kill_client(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (!d->curr)
		return;

	Atom *prot = NULL;
	int n = -1;

	if (XGetWMProtocols(dpy, d->curr->win, &prot, &n)) {
		while (--n >= 0 && prot[n] != wmatoms[WM_DELETE_WINDOW])
			;
	}

	if (n < 0) {
		XKillClient(dpy, d->curr->win);
		removewindow(d->curr->win);
		/* DBG */	fprintf(stderr, "kill_client(): n < 0\n");
	} else {
		send_kill_signal(d->curr->win);
		/* DBG */	fprintf(stderr, "kill_client(): else\n");
	}
	
	if (prot)
		XFree(prot);
	/* DBG */	fprintf(stderr, "kill_client(): OUT\n");
}

void send_kill_signal(Window w)
{
	/* DBG */	fprintf(stderr, "send_kill_signal(): IN\n");
	XEvent e;
	e.type = ClientMessage;
	e.xclient.window = w;
	e.xclient.format = 32;
	e.xclient.message_type = wmatoms[WM_PROTOCOLS];
	e.xclient.data.l[0] = wmatoms[WM_DELETE_WINDOW];
	e.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, w, False, NoEventMask, &e);
	/* DBG */	fprintf(stderr, "send_kill_signal(): OUT\n");
}

void client_to_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "client_to_desktop(): IN\n");
	/* DBG */	fprintf(stderr, "client_to_desktop(): %d\n", arg->i);
	Client *c = NULL;
	Desktop *d = NULL;
	const Arg a = { .i = arg->i };
	int tmp_id;

	if (!(d = get_current_desktop()))
		return;
	c = d->curr;
	if (!c)
		return;
	if (views[cv_id].curr_desk == LEFT) {
		tmp_id = views[cv_id].curr_left_id;
		if (tmp_id == arg->i || arg->i > DESKTOPS_LEFT)
			return;
		views[cv_id].curr_left_id = arg->i;
	} else {
		tmp_id = views[cv_id].curr_right_id;
		if (tmp_id == arg->i || arg->i > DESKTOPS_RIGHT)
			return;
		views[cv_id].curr_right_id = arg->i;
	}
	addwindow(c->win);
	focuscurrent();
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_left_id = tmp_id;
	} else {
		views[cv_id].curr_right_id = tmp_id;
	}
	XUnmapWindow(dpy, c->win);
	removewindow(d->curr->win);
	tile(d);
	focuscurrent();
	/* DBG */	fprintf(stderr, "client_to_desktop(): OUT\n");
}

void client_to_view(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "client_to_view(): IN\n");
	Client *c = NULL;
	Desktop *d = NULL;
	
	if (!(d = get_current_desktop()))
		return;
	c = d->curr;
	if (!c || views[cv_id].left_view_activated || views[cv_id].right_view_activated)
		return;
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
		addwindow(c->win);
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
		focuscurrent();
		views[cv_id].curr_desk = LEFT;
		removewindow(c->win);
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	} else if (views[cv_id].curr_desk == RIGHT) {
		views[cv_id].curr_desk = LEFT;
		addwindow(c->win);
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
		focuscurrent();
		views[cv_id].curr_desk = RIGHT;
		removewindow(c->win);
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	}
	focuscurrent();
	/* DBG */	fprintf(stderr, "client_to_view(): OUT\n");
}

void change_view(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "change_view(): IN\n");
	/* DBG */	fprintf(stderr, "change_view(): %d\n", arg->i);
	if (arg->i == cv_id || arg->i > VIEWS)
		return;

	Client *c = NULL;
	Desktop *ld = &views[cv_id].ld[views[cv_id].curr_left_id];
	Desktop *rd = &views[cv_id].rd[views[cv_id].curr_right_id];
	Desktop *ln = &views[arg->i].ld[views[arg->i].curr_left_id];
	Desktop *rn = &views[arg->i].rd[views[arg->i].curr_right_id];

	for (c = ld->head; ld->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = rd->head; rd->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = ln->head; ln->head && c; c = c->next)
		XMapWindow(dpy, c->win);
	for (c = rn->head; rn->head && c; c = c->next)
		XMapWindow(dpy, c->win);

	pv_id = cv_id;
	cv_id = arg->i;
	Arg a = { .i = views[cv_id].curr_left_id };
	Arg b = { .i = views[cv_id].curr_right_id };
	change_left_desktop(&a);
	change_right_desktop(&b);

	if (views[arg->i].both_views_activated) {
		activate_both_views(0);
	} else if (views[arg->i].left_view_activated) {
		activate_left_view(0);
	} else if (views[arg->i].right_view_activated) {
		activate_right_view(0);
	}

	draw();
	/* DBG */	fprintf(stderr, "change_views(): OUT\n");
}

void change_left_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "change_left_desktop(): IN\n");
	Client *c = NULL;
	Desktop *d = &views[cv_id].ld[views[cv_id].curr_left_id];
	Desktop *n = &views[cv_id].ld[arg->i];

	if (arg->i == views[cv_id].curr_left_id || arg->i > DESKTOPS_LEFT)
		return;
	if (views[cv_id].curr_desk == RIGHT && views[cv_id].right_view_activated) {
		return;
	}
	for (c = d->head; d->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = n->head; n->head && c; c = c->next)
		XMapWindow(dpy, c->win);
	views[cv_id].prev_left_id = views[cv_id].curr_left_id;
	views[cv_id].curr_left_id = arg->i;

	if (views[cv_id].curr_desk == LEFT) {
		tile(n);
	} else if (views[cv_id].curr_desk == RIGHT) {
		views[cv_id].curr_desk = LEFT;
		tile(n);
		views[cv_id].curr_desk = RIGHT;
	} else if (views[cv_id].left_view_activated) {
		tile(n);
	}

	focuscurrent();
	/* DBG */	fprintf(stderr, "change_left_desktop(): OUT\n");
	/* DBG */	printstatus();
}

void change_right_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "change_right_desktop(): IN\n");
	Client *c = NULL;
	Desktop *d = &views[cv_id].rd[views[cv_id].curr_right_id];
	Desktop *n = &views[cv_id].rd[arg->i];

	if (arg->i == views[cv_id].curr_right_id || arg->i > DESKTOPS_RIGHT)
		return;
	if (views[cv_id].curr_desk == LEFT && views[cv_id].left_view_activated) {
		return;
	}
	for (c = d->head; d->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = n->head; n->head && c; c = c->next)
			XMapWindow(dpy, c->win);
	views[cv_id].prev_right_id = views[cv_id].curr_right_id;
	views[cv_id].curr_right_id = arg->i;
	
	if (views[cv_id].curr_desk == RIGHT) {
		tile(n);
	} else if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
		tile(n);
		views[cv_id].curr_desk = LEFT;
	} else if (views[cv_id].right_view_activated) {
		tile(n);
	}

	focuscurrent();
	/* DBG */	fprintf(stderr, "change_right_desktop(): OUT\n");
	/* DBG */	printstatus();
}

void previous_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "previous_desktop(): IN\n");
	if (views[cv_id].curr_desk == LEFT) {
		Arg a = { .i = views[cv_id].prev_left_id };
		change_left_desktop(&a);
	} else {
		Arg a = { .i = views[cv_id].prev_right_id };
		change_right_desktop(&a);
	}
	/* DBG */	fprintf(stderr, "previous_desktop(): OUT\n");
}

void previous_view(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "previous_view(): IN\n");
	Arg a = { .i = pv_id };
	change_view(&a);
	/* DBG */	fprintf(stderr, "previous_view(): OUT\n");
}

void addwindow(Window w)
{
	/* DBG */	fprintf(stderr, "addwindow(): IN\n");
	Client *c = NULL;
	Client *n = NULL;
	Desktop *d = NULL;
	
	if (!(d = get_current_desktop()))
		return;
	if (!(c = (Client *)calloc(1, sizeof(Client))))
		die("error: client calloc error\n");
	if (d->head == NULL) {
		c->win = w;
		c->next = NULL;
		c->prev = NULL;
		d->head = c;
	} else {
		for (n = d->head; n->next; n = n->next)
			;
		c->win = w;
		c->next = NULL;
		c->prev = n;
		n->next = c;
	}
	if (XFetchName(dpy, c->win, &c->title)) {
		c->title_len = strlen(c->title);
		if (c->title == NULL) {
			c->title = "X";
			c->title_len = strlen("X");
		}
	}

	d->curr = c;	/* new client is set to master */
	XSelectInput(dpy, d->curr->win, EnterWindowMask);
	/* DBG */	fprintf(stderr, "addwindow(): OUT\n");
	/* DBG */	printstatus();
}

void nextwindow(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "nextwindow(): IN\n");
	Client *c = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (d->head && d->curr)
		if (d->curr->next) {
			d->curr = d->curr->next;
		} else {
			d->curr = d->head;
		}
	focuscurrent();
	/* DBG */	fprintf(stderr, "nextwindow(): OUT\n");
}

void nextview(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "nextview(): IN\n");
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
	} else {
		views[cv_id].curr_desk = LEFT;
	}
	focuscurrent();
	/* DBG */	fprintf(stderr, "nextview(): OUT\n");
}

void fullscreen(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "fullscreen(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (d->curr)
		XMoveResizeWindow(dpy, d->curr->win, 0, bar_height,
				  sw - 2 * border_width, sh - 2 * border_width - bar_height);
	/* DBG */	fprintf(stderr, "fullscreen(): OUT\n");
}

void activate_left_view(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "activate_left_view(): IN\n");
	if (views[cv_id].right_view_activated)
		activate_both_views(0);
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].curr_desk == RIGHT)
		nextview(0);
	views[cv_id].split_width_x = sw + separator_width / 2;
	draw();
	Arg a = { .i = 0 };
	change_right_desktop(&a);
	views[cv_id].left_view_activated = True;
	views[cv_id].both_views_activated = False;
	views[cv_id].right_view_activated = False;
	tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	focuscurrent();
	/* DBG */	fprintf(stderr, "activate_left_view(): OUT\n");
}

void activate_right_view(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "activate_right_views(): IN\n");
	if (views[cv_id].left_view_activated)
		activate_both_views(0);
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].curr_desk == LEFT)
		nextview(0);
	views[cv_id].split_width_x = 0;
	draw();
	Arg a = { .i = 0 };
	change_left_desktop(&a);
	views[cv_id].right_view_activated = True;
	views[cv_id].both_views_activated = False;
	views[cv_id].left_view_activated = False;
	tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	focuscurrent();
	/* DBG */	fprintf(stderr, "activate_right_view(): OUT\n");
}

void activate_both_views(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "activate_both_views(): IN\n");
	if (views[cv_id].both_views_activated)
		return;
	unsigned int i;
	views[cv_id].split_width_x = sw / 2;

	for (i = 1; i <= DESKTOPS_LEFT; i++) {
		views[cv_id].ld[i].layout = TILE;
		views[cv_id].ld[i].master_size =
			((master_size) ? (master_size) : (views[cv_id].split_width_x / 2));
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	}
	for (i = 1; i <= DESKTOPS_RIGHT; i++) {
		views[cv_id].rd[i].layout = TILE;
		views[cv_id].rd[i].master_size =
			((master_size) ? (master_size) : (views[cv_id].split_width_x / 2));
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	}
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
		previous_desktop(0);
		views[cv_id].curr_desk = LEFT;
	} else if (views[cv_id].curr_desk == RIGHT) {
		views[cv_id].curr_desk = LEFT;
		previous_desktop(0);
		views[cv_id].curr_desk = RIGHT;
	}

	views[cv_id].left_view_activated = False;
	views[cv_id].right_view_activated = False;
	views[cv_id].both_views_activated = True;
	focuscurrent();
	/* DBG */	fprintf(stderr, "activate_both_views(): OUT\n");
}

void prepare_draw(void)
{
	/* DBG */	fprintf(stderr, "prepare_font(): IN\n");
	/* prepare bar */
	color_map = DefaultColormap(dpy, screen);
	XGCValues val;
	val.font = XLoadFont(dpy, font);
	font_struct = XQueryFont(dpy, val.font);
	font_height = font_struct->ascent - 1 + top_font_offset;
	bar_height = font_struct->ascent + font_struct->descent - 2
		     + top_font_offset + bottom_font_offset;
	gc = XCreateGC(dpy, root, GCFont, &val);
	bar = XCreateSimpleWindow(dpy, root, 0, 0, sw, bar_height, 0, 0, 0);
	bar_buffer = XCreatePixmap(dpy, root, sw, bar_height, DefaultDepth(dpy, screen));

	/* prepare separator */
	XColor color;
	XAllocNamedColor(dpy, color_map, bar_bg_color, &color, &color);
	separator = XCreateSimpleWindow(dpy, root, 1, 1, 1, 1, 0, 0, color.pixel);
	/* DBG */	fprintf(stderr, "prepare_font(): OUT\n");
}

void draw_separator(void)
{
	/* DBG */	fprintf(stderr, "draw_separator(): IN\n");
	XUnmapWindow(dpy, separator);
	move_resize_window(separator,
			   views[cv_id].split_width_x - separator_width/2,
			   (show_bar ? (bar_height) : (0)),
			   separator_width,
			   sh);
	XMapWindow(dpy, separator);
	/* DBG */	fprintf(stderr, "draw_separator(): OUT\n");
}

void draw(void)
{
	/* DBG */	fprintf(stderr, "draw(): IN\n");
	if (!show_bar)
		return;
	draw_separator();
	draw_bar();
	draw_tags();

	XCopyArea(dpy, bar_buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
	XRaiseWindow(dpy, bar);
	XFlush(dpy);
	/* DBG */	fprintf(stderr, "draw(): OUT\n");
}

void draw_bar(void)
{
	/* DBG */	fprintf(stderr, "draw_bar(): IN\n");
	XColor color;
	XAllocNamedColor(dpy, color_map, bar_bg_color, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XFillRectangle(dpy, bar_buffer, gc, 0, 0, sw, bar_height);
	/* DBG */	fprintf(stderr, "draw_bar(): OUT\n");
}

void draw_rectangle(const char *fill_color, int x, int y, int w, int h)
{
//	/* DBG */	fprintf(stderr, "draw_rectangle(): IN\n");
	XColor color;
	XAllocNamedColor(dpy, color_map, fill_color, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XFillRectangle(dpy, bar_buffer, gc, x, y, w, h);
//	/* DBG */	fprintf(stderr, "draw_rectangle(): OUT\n");
}

void draw_string(const char *string_color, const char *string, int x, int h)
{
//	/* DBG */	fprintf(stderr, "draw_string(): IN\n");
	XColor color;
	XAllocNamedColor(dpy, color_map, string_color, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XDrawString(dpy, bar_buffer, gc, x, h, string, strlen(string));
//	/* DBG */	fprintf(stderr, "draw_string(): OUT\n");
}

void draw_tags(void)
{
	/* DBG */	fprintf(stderr, "draw_tags(): IN\n");
	unsigned int char_width = font_struct->per_char->width;
	unsigned int x_left = border_offset;
	unsigned int x_right = sw - border_offset;
	unsigned int x_views = (sw / 2);
	unsigned int i, j;
	unsigned int max_title_len = 30;
	int x, y, w, h;
	char c[max_title_len];
	char *font_color = (char *) malloc(sizeof(char) * 7);
	XColor color;

	if (views[cv_id].left_view_activated || views[cv_id].both_views_activated) {
		/* draw left desktop tags */
		for (i = 1; i <= DESKTOPS_LEFT; i++) {
			sprintf(c, "%s", tags_left[i-1]);
			x = x_left;
			y = 0;
			w = strlen(c) * char_width + 2 * char_space;
			h = bar_height - 2;
			if (views[cv_id].ld[i].head && views[cv_id].curr_left_id != i) {
				draw_rectangle(left_tag_occupied_bg, x + char_space,
						font_height + 2, w - 2 * char_space, 2);
				font_color = strcpy(font_color, left_tag_occupied_fg);
			} else if (views[cv_id].curr_left_id == i) {
				if (views[cv_id].curr_desk == LEFT) {
					draw_rectangle(focus_color, x, y, w, h);
					font_color = strcpy(font_color, bar_bg_color);
				} else {
					draw_rectangle(left_tag_focus_bg, x, y, w, h);
					font_color = strcpy(font_color, left_tag_focus_fg);
				}
			} else {
				draw_rectangle(left_tag_normal_bg, x, y, w, h);
				font_color = strcpy(font_color, left_tag_normal_fg);
			}
			draw_string(font_color, c, x_left + char_space, font_height);
			x_left += strlen(c) * char_width + 2 * char_space;
		}

		/* Draw layout tag for left desktop */
		if (views[cv_id].ld[views[cv_id].curr_left_id].layout == TILE) {
			sprintf(c, "%s", tile_tag);
		} else {
			sprintf(c, "%s", float_tag);
		}
		draw_string(left_tag_normal_fg, c, x_left + char_space, font_height);
		x_left += strlen(c) * char_width + 2 * char_space;

		/* Draw window title for left desktop */
		if (views[cv_id].ld[views[cv_id].curr_left_id].head) {
			if (views[cv_id].ld[views[cv_id].curr_left_id].curr->title == NULL) {
				sprintf(c, "%s", "unnamed");
				draw_string(left_tag_normal_fg, c, x_left + char_space, font_height);
			} else {
				strncpy(c, views[cv_id].ld[views[cv_id].curr_left_id].curr->title, max_title_len);
				draw_string(left_tag_normal_fg, c, x_left + char_space, font_height);
			}
		}
	}

	/* draw views tags */
	for (i= 0; i < VIEWS; i++)
		x_views -= (font_struct->per_char->width * strlen(tags_views[i]) / 2 + char_space);
	for (i = 1; i <= VIEWS; i++) {
		sprintf(c, "%s", tags_views[i-1]);
		x = x_views;
		y = 0;
		w = strlen(c) * char_width + 2 * char_space;
		h = bar_height - 2;
		Bool view_used = False;
		for (j = 1; j <= DESKTOPS_LEFT; j++)
			if (views[i].ld[j].head)
				view_used = True;
		for (j = 1; j <= DESKTOPS_RIGHT; j++)
			if (views[i].rd[j].head)
				view_used = True;
		if (i == cv_id) {
			draw_rectangle(view_tag_focus_bg, x, y, w, h);
			font_color = strcpy(font_color, view_tag_focus_fg);
		} else if (view_used && i != cv_id) {
			draw_rectangle(view_tag_occupied_bg, x + char_space,
				       font_height + 2, w - 2 * char_space, 2);
			font_color = strcpy(font_color, view_tag_occupied_fg);
		} else {
			draw_rectangle(view_tag_normal_bg, x, y, w, h);
			font_color = strcpy(font_color, view_tag_normal_fg);
		}
		draw_string(font_color, c, x_views + char_space, font_height);
		x_views += strlen(c) * char_width + 2 * char_space;
	}

	if (views[cv_id].right_view_activated || views[cv_id].both_views_activated) {
		/* calculate x_right */
		for (i = 0; i < DESKTOPS_RIGHT; i++)
			x_right -= strlen(tags_right[i]) * char_width + 2 * char_space;

		/* draw layout tag for right desktop */
		if (views[cv_id].rd[views[cv_id].curr_right_id].layout == TILE) {
			sprintf(c, "%s", tile_tag);
		} else {
			sprintf(c, "%s", float_tag);
		}
		x_right -= strlen(c) * char_width + char_space;
		draw_string(right_tag_normal_fg, c, x_right, font_height);
		unsigned int right_layout_tag_len = strlen(c) * char_width + char_space;

		/* Draw window title for right desktop */
		if (views[cv_id].rd[views[cv_id].curr_right_id].head) {
			if (views[cv_id].rd[views[cv_id].curr_right_id].curr->title == NULL) {
				sprintf(c, "%s", "unnamed");
				unsigned int title_length = strlen(c) * char_width + 2 * char_space;
				x_right -= title_length;
				draw_string(right_tag_normal_fg, c, x_right, font_height);
				x_right += title_length;
			} else {
				strncpy(c, views[cv_id].rd[views[cv_id].curr_right_id].curr->title, max_title_len);
				unsigned int title_length = strlen(c) * char_width + 2 * char_space;
				x_right -= title_length;
				draw_string(right_tag_normal_fg, c, x_right, font_height);
				x_right += title_length;
			}
		}
		x_right += right_layout_tag_len;

		/* draw right desktop tags */
		for (i = 1; i <= DESKTOPS_RIGHT; i++) {
			sprintf(c, "%s", tags_right[i-1]);
			x = x_right;
			y = 0;
			w = strlen(c) * char_width + 2 * char_space;
			h = bar_height - 2;

			if (views[cv_id].rd[i].head && views[cv_id].curr_right_id != i) {
				draw_rectangle(right_tag_occupied_bg, x + char_space,
						font_height + 2, w - 2 * char_space, 2);
				font_color = strcpy(font_color, right_tag_occupied_fg);
			} else if (views[cv_id].curr_right_id == i) {
				if (views[cv_id].curr_desk == RIGHT) {
					draw_rectangle(focus_color, x, y, w, h);
					font_color = strcpy(font_color, bar_bg_color);
				} else {
					draw_rectangle(right_tag_focus_bg, x, y, w, h);
					font_color = strcpy(font_color, right_tag_focus_fg);
				}
			} else {
				draw_rectangle(right_tag_normal_bg, x, y, w, h);
				font_color = strcpy(font_color, right_tag_normal_fg);
			}
			draw_string(font_color, c, x_right + char_space, font_height);
			x_right += strlen(c) * char_width + 2 * char_space;
		}
	}
	/* DBG */	fprintf(stderr, "draw_tags(): OUT\n");
}

void separator_increase(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "separator_increase(): IN\n");
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].split_width_x + views[cv_id].rd[views[cv_id].curr_right_id].master_size
	  < sw - min_window_size - separator_inc + separator_width)
		views[cv_id].split_width_x += separator_inc;
	views[cv_id].rd[views[cv_id].curr_right_id].master_size = (sw - views[cv_id].split_width_x) / 2;
	views[cv_id].ld[views[cv_id].curr_left_id].master_size = views[cv_id].split_width_x / 2;
	draw_separator();
	if (views[cv_id].curr_desk == LEFT)
		views[cv_id].curr_desk = RIGHT;
	tile_current(0);
	views[cv_id].curr_desk = LEFT;
	tile_current(0);
	/* DBG */	fprintf(stderr, "separator_increase(): OUT\n");
}

void separator_decrease(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "separator_decrease(): IN\n");
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].split_width_x > min_window_size + 2 * separator_dec)
		views[cv_id].split_width_x -= separator_dec;
	views[cv_id].rd[views[cv_id].curr_right_id].master_size = (sw - views[cv_id].split_width_x) / 2;
	views[cv_id].ld[views[cv_id].curr_left_id].master_size = views[cv_id].split_width_x / 2;
	draw_separator();
	if (views[cv_id].curr_desk == RIGHT)
		views[cv_id].curr_desk = LEFT;
	tile_current(0);
	views[cv_id].curr_desk = RIGHT;
	tile_current(0);
	/* DBG */	fprintf(stderr, "separator_decrease(): OUT\n");
}

void master_size_increase(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "master_size_increase(): IN\n");
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	if ((views[cv_id].curr_desk == LEFT && d->master_size + 2 * master_size_inc > views[cv_id].split_width_x)
	 || (views[cv_id].curr_desk == RIGHT && views[cv_id].split_width_x + d->master_size + 2 * master_size_inc > sw))
		return;
	d->master_size += master_size_inc;
	tile(d);
	/* DBG */	fprintf(stderr, "master_size_increase(): OUT\n");
}

void master_size_decrease(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "master_size_decrease(): IN\n");
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	if ((views[cv_id].curr_desk == LEFT && d->master_size < 2 * master_size_dec)
	 || (views[cv_id].curr_desk == RIGHT && views[cv_id].split_width_x + d->master_size
	   < views[cv_id].split_width_x + 2 * master_size_dec))
		return;
	d->master_size -= master_size_dec;
	tile(d);
	/* DBG */	fprintf(stderr, "master_size_decrease(): OUT\n");
}

void maximize(Window w)
{
	/* DBG */	fprintf(stderr, "maximize(): IN\n");
	int sep = ((views[cv_id].right_view_activated) ? (0) : (separator_width / 2));
	int wx, wy, ww, wh;
	if (views[cv_id].curr_desk == LEFT) {
		wx = border_offset;
		wy = border_offset + (show_bar ? (bar_height) : (0));
		ww = views[cv_id].split_width_x - 2 * border_width - 2 * border_offset - separator_width / 2;
		wh = views[cv_id].split_height_y - 2 * border_width - 2 * border_offset;
		move_resize_window(w, wx, wy, ww, wh);
	} else if (views[cv_id].curr_desk == RIGHT) {
		wx = views[cv_id].split_width_x + border_offset + sep;
		wy = border_offset + (show_bar ? (bar_height) : (0));
		ww = sw  - views[cv_id].split_width_x - 2 * border_width - 2 * border_offset - sep;
		wh = views[cv_id].split_height_y - 2 * border_width - 2 * border_offset;
		move_resize_window(w, wx, wy, ww, wh);
	}
	/* DBG */	fprintf(stderr, "maximize(): OUT\n");
}

void maximize_current(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "maximize_current(): IN\n");
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	if (d->curr)
		maximize(d->curr->win);
	/* DBG */	fprintf(stderr, "maximize_current(): OUT\n");
}

void focuscurrent(void)
{
	/* DBG */	fprintf(stderr, "focuscurrent(): IN\n");
	Client *c = NULL;
	Client *n = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop())) {
		fprintf(stderr, "FC 1\n");
		return;
	}

	if (!d->head) {
		if (views[cv_id].curr_desk == LEFT) {
			if ((n = views[cv_id].rd[views[cv_id].curr_right_id].curr))
				XSetWindowBorder(dpy, n->win, right_win_unfocus);
		} else {
			if ((n = views[cv_id].ld[views[cv_id].curr_left_id].curr))
				XSetWindowBorder(dpy, n->win, left_win_unfocus);
		}
	}

	for (c = d->head; c; c = c->next) {
		if (c == d->curr) {
			XSetWindowBorderWidth(dpy, c->win, border_width);
			if (views[cv_id].curr_desk == LEFT) {
				XSetWindowBorder(dpy, c->win, win_focus);
				if ((n = views[cv_id].rd[views[cv_id].curr_right_id].curr))
					XSetWindowBorder(dpy, n->win, right_win_unfocus);
			} else {
				XSetWindowBorder(dpy, c->win, win_focus);
				if ((n = views[cv_id].ld[views[cv_id].curr_left_id].curr))
					XSetWindowBorder(dpy, n->win, left_win_unfocus);
			}
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			XRaiseWindow(dpy, c->win);
			XSync(dpy, False);
		} else {
			if (views[cv_id].curr_desk == LEFT) {
				XSetWindowBorder(dpy, c->win, left_win_unfocus);
			} else {
				XSetWindowBorder(dpy, c->win, right_win_unfocus);
			}
		}
	}

	draw();
	/* DBG */	fprintf(stderr, "focuscurrent(): OUT\n");
}

void removewindow(Window w)
{
	/* DBG */	fprintf(stderr, "removewindow(): IN\n");
	Client *c = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next) {
		fprintf(stderr, "removewindow(): for\n");
		if (c->win == w) {
			if (!c->prev && !c->next) {
				fprintf(stderr, "removewindow(): HEAD ONLY WINDOW\n");
				d->head = NULL;
				d->curr = NULL;
				return;
			} else if (!c->prev) {
				fprintf(stderr, "removewindow(): HEAD WINDOW\n");
				d->head = c->next;
				c->next->prev = NULL;
				d->curr = c->next;
			} else if (!c->next) {
				fprintf(stderr, "removewindow(): LAST WINDOW\n");
				c->prev->next = NULL;
				d->curr = c->prev;
			} else {
				fprintf(stderr, "removewindow(): MID WINDOW\n");
				c->prev->next = c->next;
				c->next->prev = c->prev;
				d->curr = c->next;
			}
			free(c);
			return;
		}
	}
	/* DBG */	fprintf(stderr, "removewindow(): OUT\n");
}

void configurerequest(XEvent *e)
{
	/* DBG */	fprintf(stderr, "cofigurerequest(): IN\n");
	/*
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	wc.x = ev->x;
	wc.y = ev->y;
	wc.width = ev->width;
	wc.height = ev->height;
	wc.border_width = ev->border_width;
	wc.sibling = ev->above;
	wc.stack_mode = ev->detail;
	XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	XSync(dis, False);
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	d->layout = FLOAT;
	tile(d);
	*/
	/* DBG */	fprintf(stderr, "cofigurerequest(): OUT\n");
}

void destroynotify(XEvent *e)
{
	/* DBG */	fprintf(stderr, "destroynotify(): IN\n");

	unsigned int i = 0;
	Client *c = NULL;
	Desktop *d = NULL;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next) {
		if (ev->window == c->win)
			i++;
	}
	if (i == 0)
		return;
	if (ev->window != root)
		removewindow(ev->window);
	tile(d);
	focuscurrent();
	draw();
	/* DBG */	fprintf(stderr, "destroynotify(): OUT\n");
}

void maprequest(XEvent *e)
{
	/* DBG */	fprintf(stderr, "maprequest(): IN\n");
	XMapRequestEvent *ev = &e->xmaprequest;
	XWindowAttributes wa;
	Desktop *d = NULL;
	Client *c = NULL;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next) {
		if (ev->window == c->win) {
			XMapWindow(dpy, ev->window);
			return;
		}
	}
	addwindow(ev->window);
	XMapWindow(dpy, ev->window);
	if (views[cv_id].both_views_activated)
		d->layout = TILE;
	tile(d);
	focuscurrent();
	XGetWindowAttributes(dpy, d->curr->win, &wa);
	XWarpPointer(dpy, root, ev->window, 0, 0, 0, 0, --wa.width/2, --wa.height/2);
	draw();
	/* DBG */	fprintf(stderr, "maprequest(): OUT\n");
}

void tile_current(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "tile_current(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	d->layout = TILE;
	tile(d);
	/* DBG */	fprintf(stderr, "tile_current(): OUT\n");
}

void toggle_float(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "toggle_current(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	d->layout = FLOAT;
	draw();
	/* DBG */	fprintf(stderr, "toggle_current(): OUT\n");
}

void move_resize_window(Window win, int x, int y, int w, int h)
{
	/* DBG */	fprintf(stderr, "move_resize_window(): IN\n");
	XMoveResizeWindow(dpy, win, x, y, w, h);
	XSync(dpy, False);
	/* DBG */	fprintf(stderr, "move_resize_window(): OUT\n");
}

void tile(Desktop *d)
{
	/* DBG */	fprintf(stderr, "tile(): IN\n");
	/** 
	 * +-----+---+
	 * |     |   |
	 * |     +---+
	 * |     |   |
	 * |     +---+
	 * |     |   |
	 * +-----+---+
	 */

	if (d->layout == FLOAT)
		return;

	Client *c = NULL;
	int n = 0;
	int bar = (show_bar ? (bar_height) : (0));
	int y = bar;
	int wx, wy, ww, wh;
	int sep = ((views[cv_id].right_view_activated) ? (0) : (separator_width / 2));

	if (d->head && !d->head->next) {
		fprintf(stderr, "111\n");
		maximize(d->head->win);
		return;
	} else if (d->head && views[cv_id].curr_desk == LEFT) {
		wx = border_offset;
		wy = border_offset + bar;
		ww = d->master_size - 2 * border_offset - 2 * border_width;
		wh = views[cv_id].split_height_y - 2 * border_offset - 2 * border_width;
		move_resize_window(d->curr->win, wx, wy, ww, wh);
		fprintf(stderr, "222\n");
		for (c = d->head; c; c = c->next)
			if (!(c == d->curr))
				++n;
		for (c = d->head; c; c = c->next) {
			fprintf(stderr, "333\n");
			if (c == d->curr)
				continue;
			wx = d->master_size;
			wy = y + border_offset;
			ww = views[cv_id].split_width_x - d->master_size - border_offset - 2 \
			     * border_width - separator_width / 2,
			wh = views[cv_id].split_height_y / n  - 2 * border_offset - 2 * border_width;
			move_resize_window(c->win, wx, wy, ww, wh);
			y += views[cv_id].split_height_y / n;
		}
	} else if (d->head && views[cv_id].curr_desk == RIGHT) {
		wx = views[cv_id].split_width_x + border_offset + sep;
		wy = border_offset + bar;
		ww = d->master_size - 2 * border_offset - 2 * border_width;
		wh = views[cv_id].split_height_y - 2 * border_width - 2 * border_offset;
		move_resize_window(d->curr->win, wx, wy, ww, wh);
		for (c = d->head; c; c = c->next)
			if (!(c == d->curr))
				++n;
		for (c = d->head; c; c = c->next) {
			if (c == d->curr)
				continue;
			wx = views[cv_id].split_width_x + d->master_size + sep;
			wy = y + border_offset;
			ww = sw - views[cv_id].split_width_x - d->master_size - 2 * border_width \
			     - border_offset - sep;
			wh = views[cv_id].split_height_y / n - 2 * border_width - 2 * border_offset;
			move_resize_window(c->win, wx, wy, ww, wh);
			y += views[cv_id].split_height_y / n;
		}
	}
	d->layout = TILE;
	draw();
	/* DBG */	fprintf(stderr, "tile(): OUT\n");
}

void mousemove(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "mousemotion(): IN\n");
	int c;
	int rx, ry;
	int xw, yh;
	unsigned int v;
	Bool left_wall, right_wall;
	Window w;
	XEvent ev;
	XWindowAttributes wa;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (!d->curr || !XGetWindowAttributes(dpy, d->curr->win, &wa))
		return;
	if (arg->i == RESIZE)
		XWarpPointer(dpy, d->curr->win, d->curr->win, 0, 0, 0, 0, --wa.width, --wa.height);
	if (!XQueryPointer(dpy, root, &w, &w, &rx, &ry, &c, &c, &v) || w != d->curr->win)
		return;
	if (XGrabPointer(dpy, root, False, BUTTONMASK|PointerMotionMask, GrabModeAsync,
			 GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
		return;

	do {
		XMaskEvent(dpy, BUTTONMASK|PointerMotionMask|SubstructureRedirectMask, &ev);
		if (ev.type == MotionNotify) {
			xw = ((arg->i == MOVE) ? wa.x : wa.width) + ev.xmotion.x - rx;
			yh = ((arg->i == MOVE) ? wa.y : wa.height) + ev.xmotion.y - ry;
			left_wall = (xw + wa.width + 2 * border_width \
				   < views[cv_id].split_width_x + views[cv_id].split_width_x / 10 - separator_width / 2);
			right_wall = (xw \
				   > views[cv_id].split_width_x - views[cv_id].split_width_x / 10 + separator_width / 2);
			if (arg->i == RESIZE) {
				d->layout = FLOAT;
				XResizeWindow(dpy, d->curr->win,
					      (xw > min_window_size) ? xw : wa.width,
					      (yh > min_window_size) ? yh : wa.height);
			} else if (arg->i == MOVE && views[cv_id].curr_desk == LEFT) {
				d->layout = FLOAT;
				if (views[cv_id].left_view_activated
				 || (!views[cv_id].left_view_activated && left_wall)) {
					XMoveWindow(dpy, d->curr->win, xw, yh);
				} else if (!views[cv_id].left_view_activated && !left_wall) {
					/* mouse draw window from left desktop to right */
					client_to_view(0);
					tile_current(0);
					views[cv_id].curr_desk = RIGHT;
					if (views[cv_id].rd[views[cv_id].curr_right_id].layout == TILE) {
						tile_current(0);
					} else {
						maximize_current(0);
					}
					views[cv_id].curr_desk = LEFT;
					XUngrabPointer(dpy, CurrentTime);
					draw();
					return;
				}
			} else if (arg->i == MOVE && views[cv_id].curr_desk == RIGHT) {
				d->layout = FLOAT;
				if (views[cv_id].right_view_activated
				 || (!views[cv_id].right_view_activated && right_wall)) {
					XMoveWindow(dpy, d->curr->win, xw, yh);
				} else if (!views[cv_id].right_view_activated && !right_wall) {
					/* mouse draw window from right desktop to left */
					client_to_view(0);
					tile_current(0);
					views[cv_id].curr_desk = LEFT;
					if (views[cv_id].ld[views[cv_id].curr_left_id].layout == TILE) {
						tile_current(0);
					} else {
						maximize_current(0);
					}
					views[cv_id].curr_desk = RIGHT;
					XUngrabPointer(dpy, CurrentTime);
					draw();
					return;
				}
			}
		} else if (ev.type == MapRequest) {
			events[ev.type](&ev);
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	draw();
	/* DBG */	fprintf(stderr, "mousemotion(): OUT\n");
}

void spawn(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "spawn(): IN\n");
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->com)[0], (char **)arg->com);
		err(EXIT_SUCCESS, "execvp %s", (char **)arg->com[0]);
	}
	/* DBG */	fprintf(stderr, "spawn(): OUT\n");
}

int xerror(__attribute__((unused)) Display *dpy, XErrorEvent *ee)
{
	/* DBG */	fprintf(stderr, "xerror(): IN\n");
	if (ee->error_code == BadMatch) {
		fprintf(stderr, "xerror(): XErrorEvent: BadMatch\n");
	}
	/* DBG */	fprintf(stderr, "xerror(): OUT\n");
}

void sigchld(int sig)
{
	/* DBG */	fprintf(stderr, "sigchld(): IN\n");
	if (signal(SIGCHLD, sigchld) == SIG_ERR)
		die("error: can't install SIGCHLD handler\n");
	while (0 < waitpid(-1, NULL, WNOHANG))
		;
	/* DBG */	fprintf(stderr, "sigchld(): OUT\n");
}

void quit(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "quit(): IN\n");
	running = False;
	/* DBG */	fprintf(stderr, "quit(): OUT\n");
}

void setup(void)
{
	/* DBG */	fprintf(stderr, "setup(): IN\n");
	/* setup signal */
	sigchld(0);

	/* screen & root window */
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	/* screen width & height */
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);

	/* set cursor, bar, attributes, mask, etc. */
	XDefineCursor(dpy, root, XCreateFontCursor(dpy, CURSOR));
	wa.override_redirect = True;
	wa.event_mask = ExposureMask;
	if (show_bar) {
		prepare_draw();
		XChangeWindowAttributes(dpy, bar, CWOverrideRedirect|CWEventMask, &wa);
		XMapWindow(dpy, bar);
		XChangeWindowAttributes(dpy, separator, CWOverrideRedirect|CWEventMask, &wa);
	}
	wa.event_mask = SubstructureNotifyMask|SubstructureRedirectMask|PointerMotionMask|
			EnterWindowMask|LeaveWindowMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	
	/* grab keys & buttons */
	grabkeys();
	grabbuttons();

	/* set window border colors */
	win_focus = getcolor(focus_color);
	left_win_unfocus = getcolor(left_unfocus_color);
	right_win_unfocus = getcolor(right_unfocus_color);

	/* set atoms */
	wmatoms[WM_PROTOCOLS]	  = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatoms[WM_DELETE_WINDOW] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	/* set error handler */
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);

	/* current & previouse view init */
	def_view = ((default_view > 0 && default_view <= VIEWS) ? (default_view) : (1));
	cv_id = def_view;
	pv_id = cv_id;

	/* init desktops & views */
	def_left = ((default_left_desktop >= 0 && default_left_desktop <= DESKTOPS_LEFT)
		     ? (default_left_desktop) : (1));
	def_right = ((default_right_desktop >= 0 && default_right_desktop <= DESKTOPS_RIGHT)
		      ? (default_right_desktop) : (1));

	unsigned int n;
	for (n = 0; n <= VIEWS; n++) {
		views[n].curr_left_id = def_left;
		views[n].prev_left_id = def_left;
		views[n].curr_right_id = def_right;
		views[n].prev_right_id = def_right;
		views[n].curr_desk = LEFT;
		views[n].split_width_x = (float) sw / 2;
		views[n].split_height_y = sh - (show_bar ? (bar_height) : (0));
		views[n].left_view_activated = False;
		views[n].right_view_activated = False;
		views[n].both_views_activated = True;
	}

	unsigned int i, j, k;
	for (i = 1; i <= VIEWS; i++) {
		for (j = 0; j <= DESKTOPS_LEFT; j++) {
			views[i].ld[j].head = NULL;
			views[i].ld[j].curr = NULL;
			views[i].ld[j].layout = TILE;
			views[i].ld[j].master_size = ((master_size != 0) ? (master_size) : (views[cv_id].split_width_x / 2));
		}
		for (k = 0; k <= DESKTOPS_RIGHT; k++) {
			views[i].rd[k].head = NULL;
			views[i].rd[k].curr = NULL;
			views[i].rd[k].layout = TILE;
			views[i].rd[k].master_size = ((master_size != 0) ? (master_size) : (views[cv_id].split_width_x / 2));
		}
	}

	/* switch to default view */
	Arg a = { .i = def_view };
	change_view(&a);

	/* draw bar, tags, title, separator, etc. */
	draw();

	/* DBG */	fprintf(stderr, "setup(): OUT\n");
}

void run(void)
{
	/* DBG */	fprintf(stderr, "run(): IN\n");
	XEvent e;
	XSync(dpy, False);

	while (running && !XNextEvent(dpy, &e))
		if (events[e.type])
			events[e.type](&e);
	/* DBG */	fprintf(stderr, "run(): OUT\n");
}

int main(int argc, char **argv)
{
	/* DBG */	fprintf(stderr, "main(): IN\n");
	if (!(dpy = XOpenDisplay(NULL)))
		die("error: cannot open dpyplay\n");
	setup();
	run();

	return EXIT_SUCCESS;
}
