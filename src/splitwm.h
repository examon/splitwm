/* See LICENSE file for copyright and license details */

#ifndef _SPLITWM_H
#define _SPLITWM_H

#define LENGTH(X)	(sizeof(X)/sizeof(X[0]))
#define BUTTONMASK	ButtonPressMask|ButtonReleaseMask
#define DESKTOPS_LEFT	(sizeof(tags_left) / sizeof(tags_left[0]))
#define DESKTOPS_RIGHT	(sizeof(tags_right) / sizeof(tags_right[0]))
#define VIEWS		(sizeof(tags_views) / sizeof(tags_views[0]))
#define MAX(X, Y)	((X > Y) ? (X) : (Y))


/** Enums **/

enum { MOVE, RESIZE };
enum { LEFT, RIGHT };
enum { TILE, FLOAT };
enum { GRID, MASTER };
enum { TOP, BOTTOM, NONE };
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


/* Variables */

extern int sh;
extern int sw;
extern int screen;
extern unsigned int win_focus;
extern unsigned int left_win_unfocus;
extern unsigned int right_win_unfocus;
extern unsigned int min_window_size;
extern Display *dpy;
extern Window root;
extern XSetWindowAttributes wa;
extern Atom wmatoms[WM_COUNT];


/** Functions **/

extern void die(const char *errstr, ...);
extern void run(void);
extern void setup(void);
extern void sigchld(int sig);
extern void spawn(const Arg *arg);
extern void quit(const Arg *arg);
extern int xerror(Display *dis, XErrorEvent *ee);


#endif /* _SPLITWM_H */


/* vim: set ts=8 sts=8 sw=8 : */
