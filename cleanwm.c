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

/* UNUSED */
typedef struct {
	int master_size;
	Client *head;
	Client *curr;
	Client *prev;
} Desktop;

/** Function prototypes **/
static void addwindow(Window w);
static void buttonpress(XEvent *e);
static void destroynotify(XEvent *e);
static void die(const char *errstr, ...);
static void enternotify(XEvent *e);
static void focuscurrent(void);
static void fullscreen(const Arg *arg);
static unsigned long getcolor(const char *color);
static void grabbuttons(void);
static void grabkeys(void);
static void keypress(XEvent *e);
static void killcurrent(const Arg *arg);
static void killwindow(Window w);
static void maprequest(XEvent *e);
static void mousemove(const Arg *arg);
static void nextwindow(const Arg *arg);
static void printstatus(void);
static void run(void);
static void setup(void);
static void spawn(const Arg *arg);
static void quit(const Arg *arg);

/** Include config **/
#include "config.h"

/** Variables **/
static int sh;
static int sw;
static int screen;
static int current_desktop_id;
static unsigned int win_focus;
static unsigned int win_unfocus;
static Display *dis;
static Window root;
static Bool running = True;
static Desktop desktops[DESKTOPS];	/* UNUSED */
static Client *head_client;
static Client *current_client;

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

	for (i = 0; i < LENGTH(buttons); i++) {
		XGrabButton(dis, buttons[i].button, buttons[i].mask, DefaultRootWindow(dis),
			    False, BUTTONMASK, GrabModeAsync, GrabModeAsync, None, None);
	}
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

	for (i = 0; i < LENGTH(keys); i++) {
		if ((code = XKeysymToKeycode(dis, keys[i].keysym))) {
			XGrabKey(dis, code, keys[i].mod, root, True,
				 GrabModeAsync, GrabModeAsync);
		}
	}
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
	Window w = e->xcrossing.window;
	
	for (c = head_client; c; c = c->next)
		if (c->win == w) {
			current_client = c;
			focuscurrent();
	/* DBG */	printstatus();
			return;
		}
}

void printstatus(void)
{
	fprintf(stderr, "CLIENTS: ");
	Client *c;

	for (c = head_client; c; c = c->next)
		if (c == current_client) {
			fprintf(stderr, "M%s", ((c->next) ? (" ") : ("\n")));
		} else {
			fprintf(stderr, "C%s", ((c->next) ? (" ") : ("\n")));
		}
}

/* REMAKE THIS */
void killwindow(Window w)
{
	/* DBG */	fprintf(stderr, "killwindow()\n");
	XKillClient(dis, w);
	/* DBG */	printstatus();
}

void killcurrent(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "killcurrent()\n");
	killwindow(current_client->win);
}

void addwindow(Window w)
{
	/* DBG */	fprintf(stderr, "addwindow()\n");
	Client *c = NULL;
	Client *n = NULL;

	if (!(c = (Client *)calloc(1, sizeof(Client))))
		die("error: client calloc error\n");
	if (!head_client) {
		c->win = w;
		c->next = NULL;
		c->prev = NULL;
		head_client = c;			
	} else {
		c->win = w;
		c->next = NULL;
		for (n = head_client; n->next; n = n->next)
			;
		c->prev = n;
		n->next = c;
	}
	current_client = c;	/* new window is set to master */
	XSelectInput(dis, current_client->win, EnterWindowMask);
	focuscurrent();
	/* DBG */	printstatus();
}

void nextwindow(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "nextwindow()\n");
	Client *c = NULL;

	if (head_client && current_client)
		if (current_client->next) {
			current_client = current_client->next;
		} else {
			current_client = head_client;
		}
	focuscurrent();
	/* DBG */	printstatus();
}

void fullscreen(const Arg *arg)
{
	/* DBG */	fprintf(stderr, "fullscreen()\n");
	if (current_client)
		XMoveResizeWindow(dis,
				  current_client->win,
				  BORDER_OFFSET,
				  BORDER_OFFSET,
			          sw - 2*BORDER_OFFSET - 2*BORDER_WIDTH,
				  sh - 2*BORDER_OFFSET - 2*BORDER_WIDTH);
}

void focuscurrent(void)
{
	Client *c = NULL;

	for (c = head_client; c; c = c->next)
		if (c == current_client) {
			XSetWindowBorderWidth(dis, c->win, BORDER_WIDTH);
			XSetWindowBorder(dis, c->win, win_focus);
			XSetInputFocus(dis, c->win, RevertToParent, CurrentTime);
			XRaiseWindow(dis, c->win);
		} else {
			XSetWindowBorder(dis, c->win, win_unfocus);
		}
}

void removewindow(Window w)
{
	/* DBG */	fprintf(stderr, "removewindow()\n");

	Client *c = NULL;

	for (c = head_client; c; c = c->next) {
		if (c->win == w) {
			if (!c->next && !c->prev) {
				fprintf(stderr, "head only window\n");
				free(head_client);
				head_client = NULL;
				current_client = NULL;
				return;
			} else if (!c->prev) {
				fprintf(stderr, "head window\n");
				head_client = c->next;
				c->next->prev = NULL;
				current_client = c->next;
			} else if (!c->next) {
				fprintf(stderr, "last window\n");
				c->prev->next = NULL;
				current_client = c->prev;
			} else {
				fprintf(stderr, "mid window\n");
				c->prev->next = c->next;
				c->next->prev = c->prev;
				current_client = c->next;
			}
			printstatus();
			focuscurrent();
			free(c);
			return;
		}
	}
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
	addwindow(w);
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

	if (!current_client || !XGetWindowAttributes(dis, current_client->win, &wa))
		return;
	if (arg->i == RESIZE)
		XWarpPointer(dis, current_client->win, current_client->win,
			     0, 0, 0, 0, --wa.width, --wa.height);	
	if (!XQueryPointer(dis, root, &w, &w, &rx, &ry, &c, &c, &v) || w != current_client->win)
		return;
	if (XGrabPointer(dis, root, False, BUTTONMASK|PointerMotionMask, GrabModeAsync,
			 GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
		return;

	do {
		XMaskEvent(dis, BUTTONMASK|PointerMotionMask|SubstructureRedirectMask, &ev);
		if (ev.type == MotionNotify) {
			xw = ((arg->i == MOVE) ? wa.x : wa.width) + ev.xmotion.x - rx;
			yh = ((arg->i == MOVE) ? wa.y : wa.height) + ev.xmotion.y - ry;
			if (arg->i == RESIZE) {
				XResizeWindow(dis, current_client->win,
					      (xw > MIN_WINDOW_SIZE) ? xw : wa.width,
					      (yh > MIN_WINDOW_SIZE) ? yh : wa.height);
			} else if (arg->i == MOVE) {
				XMoveWindow(dis, current_client->win, xw, yh);
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
	/* DBG */	fprintf(stdout, "ending\n");
	running = False;
}

void setup(void)
{
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
	win_unfocus = getcolor(UNFOCUS_COLOR);

	/* head & current client init */
	head_client = NULL;
	current_client = NULL;

	/* catch maprequests */
	XSelectInput(dis, root, SubstructureNotifyMask|SubstructureRedirectMask);

	/* default desktop */
	current_desktop_id = 0;
}

void run(void)
{
	/* DBG */	fprintf(stdout, "in run\n");
	XEvent e;

	while (running && !XNextEvent(dis, &e))
		if (events[e.type])
			events[e.type](&e);
}

int main(int argc, char **argv)
{
	if (!(dis = XOpenDisplay(NULL)))
		die("error: cannot open display\n");
	setup();
	run();

	return EXIT_SUCCESS;
}
