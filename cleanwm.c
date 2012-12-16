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
enum { LEFT, RIGHT, NONE };
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
} Desktop;

/** Function prototypes **/
static void activate_both_views(const Arg *arg);
static void activate_left_view(const Arg *arg);
static void activate_right_view(const Arg *arg);
static void addwindow(Window w);
static void change_left_desktop(const Arg *arg);
static void change_right_desktop(const Arg *arg);
static void client_to_desktop(const Arg *arg);
static void client_to_view(const Arg *arg);
static void draw(void);
static void draw_bar(void);
static void draw_separator(void);
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
static void maximize(Window w);
static void maximize_current(const Arg *arg);
static void mousemove(const Arg *arg);
static void move_resize_window(Window win, int x, int y, int w, int h);
static void nextview(const Arg *arg);
static void nextwindow(const Arg *arg);
static void prepare_draw(void);
static void previous_desktop(const Arg *arg);
static void printstatus(void);
static void removewindow(Window w);
static void run(void);
static void send_kill_signal(Window w);
static void separator_increase(const Arg *arg);
static void separator_decrease(const Arg *arg);
static void setup(void);
static void sigchld(int sig);
static void spawn(const Arg *arg);
static void tile(Desktop *d);
static void tile_current(const Arg *arg);
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
} View;

/** Variables **/
static int bar_height;
static int sh;
static int sw;
static int screen;
static int font_height;
static int cv_id;	/* current view ID */
static int pv_id;	/* previous view ID */
static unsigned int win_focus;
static unsigned int left_win_unfocus;
static unsigned int right_win_unfocus;
static float split_width_x;
static float split_height_y;
static Pixmap buffer;
static Colormap color_map;
static Display *dpy;
static XFontStruct *font_struct;
static GC gc;
static Window bar;
static Window root;
static Bool running = True;
static Bool both_views_actvated;
static Bool left_view_activated;
static Bool right_view_activated;
static Atom wmatoms[WM_COUNT];
static View views[VIEWS];
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
	unsigned int i;

	for (i = 0; i < LENGTH(buttons); i++) {
		if ((buttons[i].mask == e->xbutton.state) &&
		    (buttons[i].button == e->xbutton.button) && buttons[i].func)
			buttons[i].func(&(buttons[i].arg));
	}
}

void enternotify(XEvent *e)
{
	/* DBG */	fprintf(stderr, "enternotify(): IN\n");
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
	/* DBG */	fprintf(stderr, "enternotify(): FOCUSCURRENT()\n");
			client_found = True;
			d->curr = c;
			focuscurrent();
			return;
		}
	}

	if (!client_found && ev->window != root) {
	/* DBG */	fprintf(stderr, "enternotify(): NEXTVIEW()\n");
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
	draw();
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
		views[cv_id].curr_left_id = arg->i;
	} else {
		tmp_id = views[cv_id].curr_right_id;
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
	if (!c || left_view_activated || right_view_activated)
		return;
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
		addwindow(c->win);
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
		focuscurrent();
		views[cv_id].curr_desk = LEFT;
		removewindow(c->win);
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	} else {
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

void change_view(const Arg *arg) {
	/* DBG */	fprintf(stderr, "change_view(): IN\n");

	/* DBG */	fprintf(stderr, "change_views(): OUT\n");
}

void change_left_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "change_left_desktop(): IN\n");
	/* DBG */	fprintf(stderr, "change_left_desktop(): %d -> %d\n", views[cv_id].curr_left_id, arg->i);
	Client *c;
	Desktop *d = &views[cv_id].ld[views[cv_id].curr_left_id];
	Desktop *n = &views[cv_id].ld[arg->i];
	
	if (arg->i == views[cv_id].curr_left_id || arg->i > DESKTOPS_LEFT)
		return;
	if (views[cv_id].curr_desk == RIGHT && right_view_activated)
		activate_left_view(0);
	if ((c = d->head)) {
	/* DBG */	fprintf(stderr, "change_left_desktop(): unmapping\n");
		for ( ; c; c = c->next)
			XUnmapWindow(dpy, c->win);
	}
	if ((c = n->head)) {
	/* DBG */	fprintf(stderr, "change_left_desktop(): mapping\n");
		for ( ; c; c = c->next)
			XMapWindow(dpy, c->win);
	}
	views[cv_id].prev_left_id = views[cv_id].curr_left_id;
	views[cv_id].curr_left_id = arg->i;
	if (left_view_activated || right_view_activated || views[cv_id].curr_desk == LEFT)
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	focuscurrent();
	/* DBG */	fprintf(stderr, "change_left_desktop(): OUT\n");
	/* DBG */	printstatus();
}

void change_right_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "change_right_desktop(): IN\n");
	/* DBG */	fprintf(stderr, "change_right_desktop(): %d -> %d\n", views[cv_id].curr_right_id, arg->i);
	/* DBG */	//printstatus();
	Client *c;
	Desktop *d = &views[cv_id].rd[views[cv_id].curr_right_id];
	Desktop *n = &views[cv_id].rd[arg->i];

	if (arg->i == views[cv_id].curr_right_id || arg->i > DESKTOPS_RIGHT)
		return;
	if (views[cv_id].curr_desk == LEFT && left_view_activated)
		activate_right_view(0);
	if ((c = d->head)) {
	/* DBG */	fprintf(stderr, "change_right_desktop(): unmapping\n");
		for ( ; c; c = c->next)
			XUnmapWindow(dpy, c->win);
	}
	if ((c = n->head)) {
	/* DBG */	fprintf(stderr, "change_right_desktop(): mapping\n");
		for ( ; c; c = c->next)
			XMapWindow(dpy, c->win);
	}
	views[cv_id].prev_right_id = views[cv_id].curr_right_id;
	views[cv_id].curr_right_id = arg->i;
	if (left_view_activated || right_view_activated || views[cv_id].curr_desk == RIGHT)
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
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
	if (XFetchName(dpy, c->win, &c->title))
		c->title_len = strlen(c->title);
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
	if (!both_views_actvated)
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
	/* DBG */	fprintf(stderr, "activate_views(): IN\n");
	split_width_x = sw + separator_width / 2;
	draw();
	Arg a = { .i = 0 };
	views[cv_id].curr_desk = RIGHT;
	change_right_desktop(&a);
	views[cv_id].curr_desk = LEFT;
	tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	focuscurrent();
	if (right_view_activated)
		previous_desktop(0);
	left_view_activated = True;
	both_views_actvated = right_view_activated = False;
	/* DBG */	fprintf(stderr, "activate_views(): OUT\n");
}

void activate_right_view(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "activate_views(): IN\n");
	split_width_x = 0 - separator_width / 2;
	draw();
	Arg a = { .i = 0 };
	views[cv_id].curr_desk = LEFT;
	change_left_desktop(&a);
	views[cv_id].curr_desk = RIGHT;
	tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	focuscurrent();
	if (left_view_activated)
		previous_desktop(0);
	right_view_activated = True;
	both_views_actvated = left_view_activated = False;
	/* DBG */	fprintf(stderr, "activate_views(): OUT\n");
}

void activate_both_views(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "activate_both_views(): IN\n");
	if (both_views_actvated) {
	/* DBG */	fprintf(stderr, "activate_both_views(): BOTH ALREADY ACTIVATED\n");
		return;
	}
	split_width_x = sw / def_width_split_coef;
	if (views[cv_id].curr_desk == LEFT && left_view_activated) {
		views[cv_id].curr_desk = RIGHT;
		previous_desktop(0);
		views[cv_id].curr_desk = LEFT;
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
		focuscurrent();
	/* DBG */	fprintf(stderr, "activate_both_views(): LEFT\n");
	} else if (views[cv_id].curr_desk == RIGHT && right_view_activated) {
		views[cv_id].curr_desk = LEFT;
		previous_desktop(0);
		views[cv_id].curr_desk = RIGHT;
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
		focuscurrent();
	/* DBG */	fprintf(stderr, "activate_both_views(): RIGHT\n");
	}
	left_view_activated = right_view_activated = False;
	both_views_actvated = True;
	/* DBG */	fprintf(stderr, "activate_both_views(): OUT\n");
}

void prepare_draw(void)
{
	/* DBG */	fprintf(stderr, "prepare_font(): IN\n");
	color_map = DefaultColormap(dpy, screen);
	XGCValues val;
	val.font = XLoadFont(dpy, font);
	font_struct = XQueryFont(dpy, val.font);
	font_height = font_struct->ascent - 1 + top_font_offset;
	bar_height = font_struct->ascent + font_struct->descent - 2
		     + top_font_offset + bottom_font_offset;
	gc = XCreateGC(dpy, root, GCFont, &val);
	bar = XCreateSimpleWindow(dpy, root, 0, 0, sw, bar_height, 0, 0, 0);
	buffer = XCreatePixmap(dpy, root, sw, bar_height, DefaultDepth(dpy, screen));
	/* DBG */	fprintf(stderr, "prepare_font(): OUT\n");
}

void draw(void)
{
	/* DBG */	fprintf(stderr, "draw(): IN\n");
	if (!show_bar)
		return;
	draw_bar();
	draw_tags();
	draw_title();
	draw_separator();
	XFlush(dpy);
	/* DBG */	fprintf(stderr, "draw(): OUT\n");
}

void draw_bar(void)
{
	/* DBG */	fprintf(stderr, "draw_bar(): IN\n");
	XColor color;
	XAllocNamedColor(dpy, color_map, bar_bg_color, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XFillRectangle(dpy, buffer, gc, 0, 0, sw, bar_height);
	XCopyArea(dpy, buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
	XRaiseWindow(dpy, bar);
	/* DBG */	fprintf(stderr, "draw_bar(): OUT\n");
}

void draw_title(void)
{
	/* DBG */	fprintf(stderr, "draw_title(): IN\n");
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	if (!d->curr)
		return;

	XColor color;
	XAllocNamedColor(dpy, color_map, bar_title_color, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XDrawString(dpy, buffer, gc, (sw / 2) - (font_struct->per_char->width * d->curr->title_len / 2),
		    font_height, d->curr->title, d->curr->title_len);
	XCopyArea(dpy, buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
	/* DBG */	fprintf(stderr, "draw_title(): OUT\n");
}

void draw_tags(void)
{
	/* DBG */	fprintf(stderr, "draw_tags(): IN\n");
	unsigned int char_width = font_struct->per_char->width;
	unsigned int x_left = char_width;
	unsigned int x_right = sw - (strlen(curr_desk_symbol[1]) + 1) * char_width;
	unsigned int i;
	char c[20];
	XColor color;

	for (i = 0; i < DESKTOPS_RIGHT; i++)
		x_right -= strlen(tags_right[i]) * char_width + char_width;
	
	if (views[cv_id].curr_desk == LEFT) {
		XAllocNamedColor(dpy, color_map, focus_color, &color, &color);
	} else {
		XAllocNamedColor(dpy, color_map, bar_tag_color, &color, &color);
	}
	XSetForeground(dpy, gc, color.pixel);
	XDrawString(dpy, buffer, gc, x_left, font_height, curr_desk_symbol[0], strlen(curr_desk_symbol[0]));
	XCopyArea(dpy, buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
	XFlush(dpy);
	x_left += (strlen(curr_desk_symbol[0]) + 1) * char_width;

	for (i = 1; i <= DESKTOPS_LEFT; i++) {
		sprintf(c, "%s", tags_left[i-1]);
		if (views[cv_id].ld[i].head && views[cv_id].curr_left_id != i) {
			XAllocNamedColor(dpy, color_map, left_unfocus_color, &color, &color);
		} else if (views[cv_id].curr_left_id == i) {
			XAllocNamedColor(dpy, color_map, focus_color, &color, &color);
		} else {
			XAllocNamedColor(dpy, color_map, bar_tag_color, &color, &color);
		}
		XSetForeground(dpy, gc, color.pixel);
		XDrawString(dpy, buffer, gc, x_left, font_height, c, strlen(c));
		XCopyArea(dpy, buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
		XFlush(dpy);
		x_left += strlen(tags_left[i-1]) * char_width + char_width;
	}

	for (i = 1; i <= DESKTOPS_RIGHT; i++) {
		sprintf(c, "%s", tags_right[i-1]);
		if (views[cv_id].rd[i].head && views[cv_id].curr_right_id != i) {
			XAllocNamedColor(dpy, color_map, right_unfocus_color, &color, &color);
		} else if (views[cv_id].curr_right_id == i) {
			XAllocNamedColor(dpy, color_map, focus_color, &color, &color);
		} else {
			XAllocNamedColor(dpy, color_map, bar_tag_color, &color, &color);
		}
		XSetForeground(dpy, gc, color.pixel);
		XDrawString(dpy, buffer, gc, x_right, font_height, c, strlen(c));
		XCopyArea(dpy, buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
		XFlush(dpy);
		x_right += strlen(tags_right[i-1]) * char_width + char_width;
	}
	/* DBG */	fprintf(stderr, "draw_tags(): OUT\n");

	if (views[cv_id].curr_desk == RIGHT) {
		XAllocNamedColor(dpy, color_map, focus_color, &color, &color);
	} else {
		XAllocNamedColor(dpy, color_map, bar_tag_color, &color, &color);
	}
	XSetForeground(dpy, gc, color.pixel);
	XDrawString(dpy, buffer, gc, x_right, font_height, curr_desk_symbol[1], strlen(curr_desk_symbol[1]));
	XCopyArea(dpy, buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
}

void draw_separator(void)
{
	/* DBG */	fprintf(stderr, "draw_separator(): IN\n");
	/* TODO */
	/* DBG */	fprintf(stderr, "draw_separator(): OUT\n");
}

void separator_increase(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "separator_increase(): IN\n");
	if (split_width_x + separator_inc < sw) {
		split_width_x += separator_inc;
	}
	/* DBG */	fprintf(stderr, "separator_increase(): w_split_coef: %f\n", split_width_x);
	draw();
	if (views[cv_id].curr_desk == LEFT)
		views[cv_id].curr_desk = RIGHT;
	views[cv_id].rd[views[cv_id].curr_right_id].master_size -= 10;
	tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	views[cv_id].curr_desk = LEFT;
	views[cv_id].ld[views[cv_id].curr_left_id].master_size += 10;
	tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	/* DBG */	fprintf(stderr, "separator_increase(): OUT\n");
}

void separator_decrease(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "separator_decrease(): IN\n");
	if (split_width_x - separator_dec > min_window_size)
		split_width_x -= separator_dec;
	/* DBG */	fprintf(stderr, "separator_increase(): w_split_coef: %f\n", split_width_x);
	draw();
	if (views[cv_id].curr_desk == RIGHT)
		views[cv_id].curr_desk = LEFT;
	views[cv_id].ld[views[cv_id].curr_left_id].master_size -= 10;
	tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	views[cv_id].curr_desk = RIGHT;
	views[cv_id].rd[views[cv_id].curr_right_id].master_size += 10;
	tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	/* DBG */	fprintf(stderr, "separator_decrease(): OUT\n");
}

void maximize(Window w)
{
	/* DBG */	fprintf(stderr, "maximize(): IN\n");
	if (views[cv_id].curr_desk == LEFT) {
		XMoveResizeWindow(dpy, w,
				  border_offset,
				  border_offset + (show_bar ? (bar_height) : (0)),
				  split_width_x - 2 * border_width - 2 * border_offset - separator_width / 2,
				  split_height_y - 2 * border_width - 2 * border_offset);
	} else if (views[cv_id].curr_desk == RIGHT) {
		XMoveResizeWindow(dpy, w,
				  split_width_x + border_offset + separator_width / 2,
				  border_offset + (show_bar ? (bar_height) : (0)),
				  sw  - split_width_x - 2 * border_width \
				  - 2 * border_offset - separator_width / 2,
				  split_height_y - 2 * border_width - 2 * border_offset);
	}
	/* DBG */	fprintf(stderr, "maximize(): OUT\n");
}

void maximize_current(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "maximize_current(): IN\n");
	Client *c = NULL;
	if (views[cv_id].curr_desk == LEFT) {
		if ((c = views[cv_id].ld[views[cv_id].curr_left_id].curr))
			maximize(c->win);
	} else if (views[cv_id].curr_desk == RIGHT) {
		if ((c = views[cv_id].rd[views[cv_id].curr_right_id].curr))
			maximize(c->win);
	} else {
		return;
	}
	/* DBG */	fprintf(stderr, "maximize_current(): OUT\n");
}

void focuscurrent(void)
{
	/* DBG */	fprintf(stderr, "focuscurrent(): IN\n");
	Client *c = NULL;
	Client *n = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (!d->head) {
	/* DBG */	fprintf(stderr, "focuscurrent(): if (!d->head)\n");
		if (views[cv_id].curr_desk == LEFT) {
			if ((n = views[cv_id].rd[views[cv_id].curr_right_id].curr))
				XSetWindowBorder(dpy, n->win, right_win_unfocus);
		} else {
			if ((n = views[cv_id].ld[views[cv_id].curr_left_id].curr))
				XSetWindowBorder(dpy, n->win, left_win_unfocus);
		}
	}

	for (c = d->head; c; c = c->next) {
	/* DBG */	fprintf(stderr, "focuscurrent(): for\n");
		if (c == d->curr) {
	/* DBG */	fprintf(stderr, "focuscurrent(): for -> if\n");
			XSetWindowBorderWidth(dpy, c->win, border_width);
			if (views[cv_id].curr_desk == LEFT) {
	/* DBG */	fprintf(stderr, "focuscurrent(): for -> if -> if\n");
				XSetWindowBorder(dpy, c->win, win_focus);
				if ((n = views[cv_id].rd[views[cv_id].curr_right_id].curr)) {
	/* DBG */	fprintf(stderr, "focuscurrent(): for -> if -> if -> if\n");
					XSetWindowBorder(dpy, n->win, right_win_unfocus);
				}
			} else {
	/* DBG */	fprintf(stderr, "focuscurrent(): for -> if -> else\n");
				XSetWindowBorder(dpy, c->win, win_focus);
				if ((n = views[cv_id].ld[views[cv_id].curr_left_id].curr)) {
	/* DBG */	fprintf(stderr, "focuscurrent(): for -> if -> else -> if\n");
					XSetWindowBorder(dpy, n->win, left_win_unfocus);
				}
			}
	/* DBG */	fprintf(stderr, "focuscurrent(): for -> if -> if != root\n");
			XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
			XRaiseWindow(dpy, c->win);
			XSync(dpy, False);
		} else {
	/* DBG */	fprintf(stderr, "focuscurrent(): for -> else\n");
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
	/* DBG */	fprintf(stderr, "removewindow()\t\t1\n");
				fprintf(stderr, "removewindow(): LAST WINDOW\n");
				c->prev->next = NULL;
				d->curr = c->prev;
			} else {
				fprintf(stderr, "removewindow(): MID WINDOW\n");
				c->prev->next = c->next;
				c->next->prev = c->prev;
				d->curr = c->next;
			}
	/* DBG */	fprintf(stderr, "removewindow()\t\t2\n");
	/* DBG */	fprintf(stderr, "removewindow()\t\tfree(c)\n");
			free(c);
	/* DBG */	fprintf(stderr, "removewindow()\t\tdone free(c)\n");
	/* DBG */	fprintf(stderr, "removewindow(): OUT\n");
			return;
		}
	}
	/* DBG */	fprintf(stderr, "removewindow()\t\t3\n");
	/* DBG */	fprintf(stderr, "removewindow(): OUT\n");
}

void configurerequest(XEvent *e)
{
	/* DBG */	fprintf(stderr, "cofigurerequest(): IN\n");
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

	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	tile(d);
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
		if (ev->window == c->win) {
			i++;
	/* DBG */	fprintf(stderr, "destroynotify(): ev->window == c->win | i == %d\n", i);
		}
	}
	if (i == 0) {
	/* DBG */	fprintf(stderr, "destroynotify(): i == 0\n");
		return;
	}

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
	Desktop *d = NULL;
	Client *c = NULL;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next) {
	/* DBG */	fprintf(stderr, "maprequest(): FOR\n");
		if (ev->window == c->win) {
			XMapWindow(dpy, ev->window);
			return;
		}
	}

	addwindow(ev->window);
	XMapWindow(dpy, ev->window);
	tile(d);
	focuscurrent();
	draw();
	/* DBG */	fprintf(stderr, "maprequest(): OUT\n");
}

void tile_current(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "tile_current(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	tile(d);
	/* DBG */	fprintf(stderr, "tile_current(): OUT\n");
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
	Client *c = NULL;
	int n = 0;
	int bar = (show_bar ? (bar_height) : (0));
	int y = bar;
	int wx, wy, ww, wh;

	if (d->head && !d->head->next) {
		maximize(d->head->win);
	} else if (d->head && views[cv_id].curr_desk == LEFT) {
		wx = border_offset;
		wy = border_offset + bar;
		ww = d->master_size - 2 * border_offset - 2 * border_width;
		wh = split_height_y - 2 * border_offset - 2 * border_width;
		move_resize_window(d->curr->win, wx, wy, ww, wh);
		
		for (c = d->head; c; c = c->next)
			if (!(c == d->curr))
				++n;
		for (c = d->head; c; c = c->next) {
			if (c == d->curr)
				continue;
			wx = d->master_size;
			wy = y + border_offset;
			ww = split_width_x - d->master_size - border_offset - 2 \
			     * border_width - separator_width / 2,
			wh = split_height_y / n  - 2 * border_offset - 2 * border_width;
			move_resize_window(c->win, wx, wy, ww, wh);
			y += split_height_y / n;
		}	
	} else if (d->head && views[cv_id].curr_desk == RIGHT) {
		wx = split_width_x + border_offset + separator_width / 2;
		wy = border_offset + bar;
		ww = d->master_size - 2 * border_offset - 2 * border_width;
		wh = split_height_y - 2 * border_width - 2 * border_offset;
		move_resize_window(d->curr->win, wx, wy, ww, wh);
		for (c = d->head; c; c = c->next)
			if (!(c == d->curr))
				++n;
		for (c = d->head; c; c = c->next) {
			if (c == d->curr)
				continue;
			wx = split_width_x + d->master_size + separator_width / 2;
			wy = y + border_offset;
			ww = sw - split_width_x - d->master_size - 2 * border_width \
			     - border_offset - separator_width / 2,
			wh = split_height_y / n - 2 * border_width - 2 * border_offset;
			move_resize_window(c->win, wx, wy, ww, wh);
			y += split_height_y / n;
		}	
	}
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
		XWarpPointer(dpy, d->curr->win, d->curr->win,
			     0, 0, 0, 0, --wa.width, --wa.height);
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
				   < split_width_x + split_width_x / 10 - separator_width / 2);
			right_wall = (xw > split_width_x - split_width_x / 10 + separator_width / 2);
			if (arg->i == RESIZE) {
				XResizeWindow(dpy, d->curr->win,
					      (xw > min_window_size) ? xw : wa.width,
					      (yh > min_window_size) ? yh : wa.height);
			} else if (arg->i == MOVE && views[cv_id].curr_desk == LEFT) {
				if (left_view_activated || (!left_view_activated && left_wall)) {
					XMoveWindow(dpy, d->curr->win, xw, yh);
				} else if (!left_view_activated && !left_wall) {
					client_to_view(0);
					XUngrabPointer(dpy, CurrentTime);
					draw();
					return;
				}
			} else if (arg->i == MOVE && views[cv_id].curr_desk == RIGHT) {
				if (right_view_activated || (!right_view_activated && right_wall)) {
					XMoveWindow(dpy, d->curr->win, xw, yh);
				} else if (!right_view_activated && !right_wall) {
					client_to_view(0);
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
		execvp((char *)arg->com[0], (char **)arg->com);
		err(EXIT_SUCCESS, "execvp %s", (char *)arg->com[0]);
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
	}
	wa.event_mask = SubstructureNotifyMask|SubstructureRedirectMask|PointerMotionMask|
			EnterWindowMask|LeaveWindowMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	
	/* grab keys & buttons */
	grabkeys();
	grabbuttons();

	/* setup width & heigh split coefficients */
	split_width_x = sw / def_width_split_coef;
	split_height_y = sh / def_height_split_coef - (show_bar ? (bar_height) : (0));

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
	cv_id = default_view;
	pv_id = cv_id;

	/* init desktops & views */
	unsigned int def_left = ((default_left_desktop >= 0 && default_left_desktop <= DESKTOPS_LEFT)
				? (default_left_desktop) : (1));
	unsigned int def_right = ((default_right_desktop >= 0 && default_right_desktop <= DESKTOPS_LEFT)
				 ? (default_right_desktop) : (1));
	views[cv_id].curr_desk = LEFT;
	views[cv_id].prev_left_id = views[cv_id].curr_left_id = def_left;
	views[cv_id].prev_right_id = views[cv_id].curr_right_id = def_right;

	unsigned int i;
	for (i = 0; i <= DESKTOPS_LEFT; i++) {
		views[cv_id].ld[i].head = NULL;
		views[cv_id].ld[i].curr = NULL;
		views[cv_id].ld[i].master_size = ((master_size) ? (master_size) : (split_width_x / 2));
	}
	for (i = 0; i <= DESKTOPS_RIGHT; i++) {
		views[cv_id].rd[i].head = NULL;
		views[cv_id].rd[i].curr = NULL;
		views[cv_id].rd[i].master_size = ((master_size) ? (master_size) : (split_width_x / 2));
	}

	/* change to default desktop */
	Arg a = { .i = views[cv_id].curr_left_id };
	Arg b = { .i = views[cv_id].curr_right_id };
	change_left_desktop(&a);
	change_right_desktop(&b);

	/* default views */
	left_view_activated = False;
	right_view_activated = False;
	both_views_actvated = True;

	if (default_focused_view == LEFT) {
		activate_left_view(0);
	} else if (default_focused_view == RIGHT) {
		activate_right_view(0);
	}

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
