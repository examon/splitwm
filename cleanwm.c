#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#define LENGTH(X)		(sizeof(X)/sizeof(X[0]))
#define BUTTONMASK		ButtonPressMask|ButtonReleaseMask

enum { MOVE, RESIZE };
enum { LEFT, RIGHT };

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
	Window win;
} Client;

typedef struct {
	Client *head;
	Client *curr;
} Desktop;

typedef struct {
	Desktop	ld[10];		/* left desktops */
	Desktop	rd[10];		/* right desktops */
	int curr_left_id;
	int prev_left_id;
	int curr_right_id;
	int prev_right_id;
	int curr_desk;		/* current view, LEFT/RIGHT */
} View;

/** Function prototypes **/
static void addwindow(Window w);
static void buttonpress(XEvent *e);
static void change_left_desktop(const Arg *arg);
static void change_right_desktop(const Arg *arg);
static void destroynotify(XEvent *e);
static void die(const char *errstr, ...);
static void enternotify(XEvent *e);
static void focuscurrent(void);
static void fullscreen(const Arg *arg);
static unsigned long getcolor(const char *color);
Desktop *get_current_desktop(void);
static void grabbuttons(void);
static void grabkeys(void);
static void keypress(XEvent *e);
static void killcurrent(const Arg *arg);
static void killwindow(Window w);
static void maprequest(XEvent *e);
static void maximize(Window w);
static void maximize_current(const Arg *arg);
static void mousemove(const Arg *arg);
static void nextview(const Arg *arg);
static void nextwindow(const Arg *arg);
static void previous_desktop(const Arg *arg);
static void printstatus(void);
static void run(void);
static void setup(void);
static void spawn(const Arg *arg);
static void status(const Arg* arg);
static void tile(void);
static void quit(const Arg *arg);

/** Include config **/
#include "config.h"

/** Variables **/
static int sh;
static int sw;
static int screen;
static int cv_id;	/* current view ID */
static int pv_id;	/* previous view ID */
static unsigned int win_focus;
static unsigned int left_win_unfocus;
static unsigned int right_win_unfocus;
static Display *dis;
static Window root;
static Bool running = True;
static Client *head_client;
static Client *current_client;
static View views[VIEWS];

/** Event handlers **/
static void (*events[LASTEvent])(XEvent *e) = {
	[KeyPress]      = keypress,
	[ButtonPress]   = buttonpress,
	[MapRequest]    = maprequest,
	[DestroyNotify] = destroynotify,
	[EnterNotify]	= enternotify
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
		XGrabButton(dis, buttons[i].button, buttons[i].mask, DefaultRootWindow(dis),
			    False, BUTTONMASK, GrabModeAsync, GrabModeAsync, None, None);
}

unsigned long getcolor(const char *color)
{
	Colormap cm = DefaultColormap(dis, screen);
	XColor xc;

	if (!XAllocNamedColor(dis, cm, color, &xc, &xc))
		die("error: cannot get color\n");
	return xc.pixel;
}

void grabkeys(void)
{
	unsigned int i;
	KeyCode code;

	for (i = 0; i < LENGTH(keys); i++)
		if ((code = XKeysymToKeycode(dis, keys[i].keysym)))
			XGrabKey(dis, code, keys[i].mod, root, True,
				 GrabModeAsync, GrabModeAsync);
}

void keypress(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ke;

	ke = &e->xkey;
	keysym = XkbKeycodeToKeysym(dis, (KeyCode)ke->keycode, 0, 0);
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
	/* DBG */	fprintf(stderr, "enternotify()\n");
	Client *c = NULL;
	Desktop *d = NULL;
	Window w = e->xcrossing.window;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next)
		if (c->win == w) {
			d->curr = c;
			focuscurrent();
			return;
		}
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

void status(const Arg *arg)
{
	printstatus();
}

/* REMAKE THIS */
void printstatus(void)
{
	unsigned int i;

	for (i = 0; i < DESKTOPS; i++) {
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

	for (i = 0; i < DESKTOPS; i++) {
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

/* REMAKE THIS */
void killwindow(Window w)
{
	/* DBG */	fprintf(stderr, "killwindow()\n");
	XKillClient(dis, w);
	/* DBG */	//printstatus();
}

void killcurrent(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "killcurrent()\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (d->curr)
		killwindow(d->curr->win);
}

void change_left_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "change_left_desktop(): %d -> %d\n", views[cv_id].curr_left_id, arg->i);
	/* DBG */	//printstatus();
	Client *c;
	Desktop *d = &views[cv_id].ld[views[cv_id].curr_left_id];
	Desktop *n = &views[cv_id].ld[arg->i];

	if (arg->i == views[cv_id].curr_left_id)
		return;
	if ((c = d->head)) {
	/* DBG */	fprintf(stderr, "change_left_desktop(): unmapping\n");
		for ( ; c; c = c->next)
			XUnmapWindow(dis, c->win);
	}

	if ((c = n->head)) {
	/* DBG */	fprintf(stderr, "change_left_desktop(): mapping\n");
		for ( ; c; c = c->next)
			XMapWindow(dis, c->win);
	}

	views[cv_id].prev_left_id = views[cv_id].curr_left_id;
	views[cv_id].curr_left_id = arg->i;
	/* DBG */	printstatus();
}

void change_right_desktop(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "change_right_desktop(): %d -> %d\n", views[cv_id].curr_right_id, arg->i);
	/* DBG */	//printstatus();
	Client *c;
	Desktop *d = &views[cv_id].rd[views[cv_id].curr_right_id];
	Desktop *n = &views[cv_id].rd[arg->i];

	if (arg->i == views[cv_id].curr_right_id)
		return;
	if ((c = d->head)) {
	/* DBG */	fprintf(stderr, "change_right_desktop(): unmapping\n");
		for ( ; c; c = c->next)
			XUnmapWindow(dis, c->win);
	}

	if ((c = n->head)) {
	/* DBG */	fprintf(stderr, "change_right_desktop(): mapping\n");
		for ( ; c; c = c->next)
			XMapWindow(dis, c->win);
	}

	views[cv_id].prev_right_id = views[cv_id].curr_right_id;
	views[cv_id].curr_right_id = arg->i;
	/* DBG */	printstatus();
}


void previous_desktop(const Arg *arg)
{
	if (views[cv_id].curr_desk == LEFT) {
		Arg a = { .i = views[cv_id].prev_left_id };
		change_left_desktop(&a);
	} else {
		Arg a = { .i = views[cv_id].prev_right_id };
		change_right_desktop(&a);
	}
}

void addwindow(Window w)
{
	/* DBG */	fprintf(stderr, "addwindow()\n");
	Client *c = NULL;
	Client *n = NULL;
	Desktop *d = NULL;
	
	if (!(d = get_current_desktop()))
		return;
	if (!(c = (Client *)calloc(1, sizeof(Client))))
		die("error: client calloc error\n");
	if (!d->head) {
		c->win = w;
		c->next = NULL;
		c->prev = NULL;
		d->head = c;
	} else {
		c->win = w;
		c->next = NULL;
		for (n = d->head; n->next; n = n->next)
			;
		c->prev = n;
		n->next = c;
	}
	d->curr = c;	/* new client is set to master */
	XSelectInput(dis, d->curr->win, EnterWindowMask);
	focuscurrent();
	/* DBG */	//printstatus();
}

void nextwindow(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "nextwindow()\n");
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
	/* DBG */	//printstatus();
}

void nextview(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "nextview()\n");
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
	} else {
		views[cv_id].curr_desk = LEFT;
	}
	focuscurrent();
}

void fullscreen(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "fullscreen()\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (d->curr)
		XMoveResizeWindow(dis, d->curr->win, 0, 0,
				  sw - 2 * BORDER_WIDTH,
				  sh - 2 * BORDER_WIDTH);
}

void maximize(Window w)
{
	if (views[cv_id].curr_desk == LEFT) {
		XMoveResizeWindow(dis, w, 0, 0,
				  sw / W_SPLIT_COEFFICIENT - 2 * BORDER_WIDTH - SPLIT_SEPARATOR_WIDTH,
				  sh / H_SPLIT_COEFFICIENT - 2 * BORDER_WIDTH);
	} else if (views[cv_id].curr_desk == RIGHT) {
		XMoveResizeWindow(dis, w, sw / W_SPLIT_COEFFICIENT + SPLIT_SEPARATOR_WIDTH, 0,
				  sw  - sw / W_SPLIT_COEFFICIENT - 2 * BORDER_WIDTH,
				  sh - 2 * BORDER_WIDTH);
	}
}

void maximize_current(const Arg *arg)
{
	if (views[cv_id].curr_desk == LEFT)
		maximize(views[cv_id].ld[views[cv_id].curr_left_id].curr->win);
	else
		maximize(views[cv_id].rd[views[cv_id].curr_right_id].curr->win);
}

void focuscurrent(void)
{
	/* DBG */	fprintf(stderr, "focuscurrent()\n");
	Client *c = NULL;
	Client *n = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (!d->head)
		if (views[cv_id].curr_desk == LEFT) {
			if ((n = views[cv_id].rd[views[cv_id].curr_right_id].curr))
				XSetWindowBorder(dis, n->win, right_win_unfocus);
		} else {
			if ((n = views[cv_id].ld[views[cv_id].curr_left_id].curr))
				XSetWindowBorder(dis, n->win, left_win_unfocus);
		}

	for (c = d->head; c; c = c->next)
		if (c == d->curr) {
			XSetWindowBorderWidth(dis, c->win, BORDER_WIDTH);
			if (views[cv_id].curr_desk == LEFT) {
				XSetWindowBorder(dis, c->win, win_focus);
				if ((n = views[cv_id].rd[views[cv_id].curr_right_id].curr))
					XSetWindowBorder(dis, n->win, right_win_unfocus);
			} else {
				XSetWindowBorder(dis, c->win, win_focus);
				if ((n = views[cv_id].ld[views[cv_id].curr_left_id].curr))
					XSetWindowBorder(dis, n->win, left_win_unfocus);
			}
			XSetInputFocus(dis, c->win, RevertToParent, CurrentTime);
			XRaiseWindow(dis, c->win);
		} else {
			if (views[cv_id].curr_desk == LEFT) {
				XSetWindowBorder(dis, c->win, left_win_unfocus);
			} else {
				XSetWindowBorder(dis, c->win, right_win_unfocus);
			}
		}
}

void removewindow(Window w)
{
	/* DBG */	fprintf(stderr, "removewindow()\n");
	Client *c = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;

	for (c = d->head; c; c = c->next) {
		if (c->win == w) {
			if (!c->next && !c->prev) {
				fprintf(stderr, "head only window\n");
				free(head_client);
				d->head = NULL;
				d->curr = NULL;
				return;
			} else if (!c->prev) {
				fprintf(stderr, "head window\n");
				d->head = c->next;
				c->next->prev = NULL;
				current_client = c->next;
			} else if (!c->next) {
				fprintf(stderr, "last window\n");
				c->prev->next = NULL;
				d->curr = c->prev;
			} else {
				fprintf(stderr, "mid window\n");
				c->prev->next = c->next;
				c->next->prev = c->prev;
				d->curr = c->next;
			}
			focuscurrent();
			free(c);
			return;
		}
	}
	/* DBG */	fprintf(stderr, "removewindow() exit\n");
}

void destroynotify(XEvent *e)
{
	/* DBG */	fprintf(stderr, "destroynotify()\n");
	removewindow(e->xdestroywindow.window);
}

void maprequest(XEvent *e)
{
	/* DBG */	fprintf(stderr, "maprequest()\n");
	Window w = e->xmaprequest.window;

	XMapWindow(dis, w);
	maximize(w);
	addwindow(w);
}

/* IMPLEMENT LATER */
void tile(void)
{
	return;
}

void mousemove(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "in mousemotion\n");
	int c;
	int rx, ry;
	int xw, yh;
	unsigned int v;
	Window w;
	XEvent ev;
	XWindowAttributes wa;
	Desktop *d = NULL;

	if (views[cv_id].curr_desk == LEFT) {
		d = &views[cv_id].ld[views[cv_id].curr_left_id];
	} else if (views[cv_id].curr_desk == RIGHT) {
		d = &views[cv_id].rd[views[cv_id].curr_right_id];
	} else {
		return;
	}

	if (!d->curr || !XGetWindowAttributes(dis, d->curr->win, &wa))
		return;
	if (arg->i == RESIZE)
		XWarpPointer(dis, d->curr->win, d->curr->win,
			     0, 0, 0, 0, --wa.width, --wa.height);	
	if (!XQueryPointer(dis, root, &w, &w, &rx, &ry, &c, &c, &v) || w != d->curr->win)
		return;
	if (XGrabPointer(dis, root, False, BUTTONMASK|PointerMotionMask, GrabModeAsync,
			 GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
		return;

	do {
		XMaskEvent(dis, BUTTONMASK|PointerMotionMask|SubstructureRedirectMask, &ev);
		if (ev.type == MotionNotify) {
			fprintf(stderr, "\trx: %d\try: %d\n", rx, ry);
			xw = ((arg->i == MOVE) ? wa.x : wa.width) + ev.xmotion.x - rx;
			yh = ((arg->i == MOVE) ? wa.y : wa.height) + ev.xmotion.y - ry;
			fprintf(stderr, "\txw: %d\tyh: %d\n", xw);
			fprintf(stderr, "\twa.width: %d\twa.height: %d\n\n", wa.width, wa.height);

			if (arg->i == RESIZE) {
	/* DBG */	fprintf(stderr, "in mousemotion(): RESIZE\n");
				XResizeWindow(dis, d->curr->win,
					      (xw > MIN_WINDOW_SIZE) ? xw : wa.width,
					      (yh > MIN_WINDOW_SIZE) ? yh : wa.height);
			} else if (arg->i == MOVE && views[cv_id].curr_desk == LEFT &&
				   xw + wa.width + 2 * BORDER_WIDTH < (sw / W_SPLIT_COEFFICIENT) &&
				   xw > 0 &&
			           yh + wa.height + 2 * BORDER_WIDTH < (sh / H_SPLIT_COEFFICIENT) &&
				   yh > 0) {
	/* DBG */	fprintf(stderr, "in mousemotion(): MOVE LEFT\n");
					XMoveWindow(dis, d->curr->win, xw, yh);
			} else if (arg->i == MOVE && views[cv_id].curr_desk == RIGHT &&
				   xw > (sw / W_SPLIT_COEFFICIENT) &&
				   xw + wa.width + 2 * BORDER_WIDTH < sw &&
				   yh + wa.height + 2 * BORDER_WIDTH < sh &&
				   yh > 0) {
	/* DBG */	fprintf(stderr, "in mousemotion(): MOVE RIGHT\n");
					XMoveWindow(dis, d->curr->win, xw, yh);
			}
		} else if (ev.type == MapRequest) {
			events[ev.type](&ev);
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dis, CurrentTime);
	/* DBG */	fprintf(stderr, "out mousemotion\n");
}

void spawn(const Arg *arg)
{
	if (fork() == 0) {
		if (dis)
			close(ConnectionNumber(dis));
		setsid();
		execvp((char *)arg->com[0], (char **)arg->com);
		err(EXIT_SUCCESS, "execvp %s", (char *)arg->com[0]);
	}
}

void quit(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "ending\n");
	running = False;
}

void setup(void)
{
	/* DBG */	fprintf(stderr, "in setup()\n");
	/* screen & root window */
	screen = DefaultScreen(dis);
	root = RootWindow(dis, screen);

	/* screen width & height */
	sw = DisplayWidth(dis, screen);
	sh = DisplayHeight(dis, screen);
	
	/* grab keys & buttons */
	grabkeys();
	grabbuttons();

	/* set window border colors */
	win_focus = getcolor(FOCUS_COLOR);
	left_win_unfocus = getcolor(LEFT_UNFOCUS_COLOR);
	right_win_unfocus = getcolor(RIGHT_UNFOCUS_COLOR);

	/* catch maprequests */
	XSelectInput(dis, root, SubstructureNotifyMask|SubstructureRedirectMask);

	/* current & previouse view init */
	cv_id = DEFAULT_VIEW;
	pv_id = cv_id;

	/* init desktops & views */
	views[cv_id].curr_desk = LEFT;
	views[cv_id].curr_left_id = DEFAULT_LEFT_DESKTOP;
	views[cv_id].curr_right_id = DEFAULT_RIGHT_DESKTOP;

	unsigned int i;
	for (i = 0; i < DESKTOPS; i++) {
		views[cv_id].ld[i].head = NULL;
		views[cv_id].ld[i].curr = NULL;
		views[cv_id].rd[i].head = NULL;
		views[cv_id].rd[i].curr = NULL;
	}

	/* change to default desktop */
	Arg a = { .i = views[cv_id].curr_left_id };
	Arg b = { .i = views[cv_id].curr_right_id };
	change_left_desktop(&a);
	change_right_desktop(&b);
}

void run(void)
{
	/* DBG */	fprintf(stderr, "in run()\n");
	XEvent e;

	while (running && !XNextEvent(dis, &e))
		if (events[e.type])
			events[e.type](&e);
}

int main(int argc, char **argv)
{
	/* DBG */	fprintf(stderr, "in main()\n");
	if (!(dis = XOpenDisplay(NULL)))
		die("error: cannot open display\n");
	setup();
	run();

	return EXIT_SUCCESS;
}
