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
#define MAX(A, B)		(((A) > (B)) ? (A) : (B))

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

/** Function prototypes **/
static void buttonpress(XEvent *e);
static void die(const char *errstr, ...);
static void grabbuttons(void);
static void grabkeys(void);
static void keypress(XEvent *e);
static void mousemove(const Arg *arg);
static void run(void);
static void setup(void);
static void spawn(const Arg *arg);
static void quit(const Arg *arg);

/** Variables **/
static Display *dis;
static Window root;
static int screen;
static Bool running = True;
static int sh;
static int sw;

static void (*events[LASTEvent])(XEvent *e) = {
	[KeyPress] = keypress,
	[ButtonPress] = buttonpress
};

/** Include config **/
#include "config.h"

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

void mousemove(const Arg *arg)
{
	fprintf(stderr, "in mousemotion\n");

	XEvent ev;

	if (XGrabPointer(dis, root, False, BUTTONMASK|PointerMotionMask, GrabModeAsync,
			 GrabModeAsync, None, None, CurrentTime) != GrabSuccess)
		return;

	int cnt = 0;
	do {
		XMaskEvent(dis, BUTTONMASK|PointerMotionMask, &ev);
		if (ev.type == MotionNotify) {
			fprintf(stderr, "%s %d\n", ((arg->i == MOVE) ? "moving" : "resizing"), cnt++);
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dis, CurrentTime);

	fprintf(stderr, "out mousemotion\n");
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
	fprintf(stdout, "ending\n");
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

}

void run(void)
{
	fprintf(stdout, "in run\n");
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
