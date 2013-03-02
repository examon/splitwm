/* See LICENSE file for copyright and license details */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>
#include <unistd.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "splitwm.h"
#include "tile.h"
#include "desktop.h"
#include "client.h"
#include "grab.h"
#include "event.h"
#include "dbg.h"
#include "view.h"
#include "draw.h"
#include "mouse.h"
#include "config.h"


int sh;
int sw;
int screen;
unsigned int win_focus;
unsigned int left_win_unfocus;
unsigned int right_win_unfocus;
unsigned int min_window_size;
Display *dpy;
Window root;
XSetWindowAttributes wa;
Atom wmatoms[WM_COUNT];

unsigned int def_left;
unsigned int def_right;
unsigned int def_view;
Bool running = True;


void die(const char *errstr, ...)
{
	va_list ap;
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void spawn(const Arg *arg)
{
	dbg("spawn(): IN\n");
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->com)[0], (char **)arg->com);
		err(EXIT_SUCCESS, "execvp %s", (char *)arg->com[0]);
	}
	dbg("spawn(): OUT\n");
}

int xerror(__attribute__((unused)) Display *dpy, XErrorEvent *ee)
{
	dbg("xerror(): IN\n");
	if (ee->error_code == BadMatch) {
		dbg("xerror(): XErrorEvent: BadMatch\n");
	}
	return 0;
	dbg("xerror(): OUT\n");
}

void sigchld(int sig)
{
	dbg("sigchld(): IN\n");
	if (signal(SIGCHLD, sigchld) == SIG_ERR)
		die("error: can't install SIGCHLD handler\n");
	while (0 < waitpid(-1, NULL, WNOHANG))
		;
	dbg("sigchld(): OUT\n");
}

void quit(const Arg *arg)
{
	dbg("quit(): IN\n");
	running = False;
	dbg("quit(): OUT\n");
}

void setup(void)
{
	dbg("setup(): IN\n");
	/* setup signal */
	sigchld(0);

	/* screen & root window */
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	/* screen width & height */
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);

	/* font offset */
	font_top_offset = 5;
	font_bottom_offset = 5;

	/* minimum window size */
	min_window_size = 20;

	/* set cursor, bar, attributes, mask, etc. */
	XDefineCursor(dpy, root, XCreateFontCursor(dpy, CURSOR));
	wa.override_redirect = True;
	wa.event_mask = ExposureMask;

	prepare_draw();
	map_bar();

	if (SHOW_SEPARATOR)
		XChangeWindowAttributes(dpy, separator, CWOverrideRedirect|CWEventMask, &wa);
	wa.event_mask = SubstructureNotifyMask
		      | SubstructureRedirectMask
		      | ButtonPressMask
		      | PointerMotionMask
		      | EnterWindowMask
		      | LeaveWindowMask
		      | StructureNotifyMask
		      | PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	
	/* grab keys & buttons */
	grabkeys();
	grabbuttons();

	/* set window border colors */
	win_focus = grabcolor(FOCUS_COLOR);
	left_win_unfocus = grabcolor(LEFT_UNFOCUS_COLOR);
	right_win_unfocus = grabcolor(RIGHT_UNFOCUS_COLOR);

	/* set atoms */
	wmatoms[WM_PROTOCOLS]	  = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatoms[WM_DELETE_WINDOW] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	/* set error handler */
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);

	/* current & previouse view init */
	def_view = ((DEFAULT_VIEW > 0 && DEFAULT_VIEW <= VIEWS) ? (DEFAULT_VIEW) : (1));
	cv_id = def_view;
	pv_id = cv_id;

	/* init desktops & views */
	def_left = ((DEFAULT_LEFT_DESKTOP >= 0 && DEFAULT_LEFT_DESKTOP <= DESKTOPS_LEFT)
		     ? (DEFAULT_LEFT_DESKTOP) : (1));
	def_right = ((DEFAULT_RIGHT_DESKTOP >= 0 && DEFAULT_RIGHT_DESKTOP <= DESKTOPS_RIGHT)
		      ? (DEFAULT_RIGHT_DESKTOP) : (1));

	unsigned int n;
	for (n = 0; n <= VIEWS; n++) {
		views[n].curr_left_id = def_left;
		views[n].prev_left_id = def_left;
		views[n].curr_right_id = def_right;
		views[n].prev_right_id = def_right;
		views[n].curr_desk = LEFT;
		views[n].split_width_x = (float) sw / 2;
		views[n].split_height_y = sh - ((BAR_POSITION != NONE) ? (bar_height) : (0));
		views[n].left_view_activated = False;
		views[n].right_view_activated = False;
		views[n].both_views_activated = True;
	}

	unsigned int i, j, k;
	for (i = 1; i <= VIEWS; i++) {
		for (j = 0; j <= DESKTOPS_LEFT; j++) {
			views[i].ld[j].head = NULL;
			views[i].ld[j].curr = NULL;
			views[i].ld[j].master_size =
				((MASTER_SIZE != 0) ? (MASTER_SIZE)
				: (views[cv_id].split_width_x / 2));
			views[i].ld[j].tile_or_float = TILE;
			views[i].ld[j].layout = GRID;
		}
		for (k = 0; k <= DESKTOPS_RIGHT; k++) {
			views[i].rd[k].head = NULL;
			views[i].rd[k].curr = NULL;
			views[i].rd[k].master_size =
				((MASTER_SIZE != 0) ? (MASTER_SIZE)
				: (views[cv_id].split_width_x / 2));
			views[i].rd[k].tile_or_float = TILE;
			views[i].ld[j].layout = GRID;
		}
	}

	/* switch to default view */
	Arg a = { .i = def_view };
	change_view(&a);

	/* draw bar, tags, title, separator, etc. */
	draw();
	dbg("setup(): OUT\n");
}

void run(void)
{
	dbg("run(): IN\n");
	XEvent e;
	XSync(dpy, False);

	while (running && !XNextEvent(dpy, &e))
		if (events[e.type])
			events[e.type](&e);
	dbg("run(): OUT\n");
}

int main(int argc, char **argv)
{
	dbg("main(): IN\n");
	if (!(dpy = XOpenDisplay(NULL)))
		die("error: cannot open dpyplay\n");
	setup();
	run();
	return EXIT_SUCCESS;
}


/* vim: set ts=8 sts=8 sw=8 : */
