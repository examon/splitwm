/* See LICENSE file for copyright and license details */

#ifndef _SPLITWM_H
#define _SPLITWM_H

/**
 * Gets length of X.
 */
#define  LENGTH(X)  (sizeof(X)/sizeof(X[0]))

/**
 * Press & release buttonmask.
 */
#define  BUTTONMASK  ButtonPressMask|ButtonReleaseMask

/**
 * Gets length of the tags_left array.
 */
#define  DESKTOPS_LEFT  (sizeof(tags_left) / sizeof(tags_left[0]))

/**
 * Gets length of the tags_right array.
 */
#define  DESKTOPS_RIGHT  (sizeof(tags_right) / sizeof(tags_right[0]))

/**
 * Gets length of the tags_views array.
 */
#define  VIEWS  (sizeof(tags_views) / sizeof(tags_views[0]))


/**
 * Mouse move/resize.
 */
enum { MOVE, RESIZE };

/**
 * Left/right desktop.
 */
enum { LEFT, RIGHT };

/**
 * Tiling/floating mode.
 */
enum { TILE, FLOAT };

/**
 * Grid/master tiling layout.
 */
enum { GRID, MASTER };

/**
 * Bar position (NONE = bar hidden).
 */
enum { TOP, BOTTOM, NONE };

/**
 * Atoms.
 */
enum { WM_PROTOCOLS, WM_DELETE_WINDOW, WM_COUNT };


/**
 * Union storing argument of the function accessible thought the key/mouse shortcuts.
 */
typedef union {
	
	/**
	 * Integer argument. Usually desktop/view number, etc.
	 */
	const int i;

	/**
	 * Array of strings. Usually storing info about custom runnable commands.
	 */
	const char **com;
} Arg;

/**
 * Key structure storing the most important information about key.
 */
typedef struct {

	/**
	 * Modifier. Usually MOD1, MOD4 or SHIFT.
	 */
	unsigned int mod;

	/**
	 * Key representation (e.g. XK_a).
	 */
	KeySym keysym;

	/**
	 * Function called when key is pressed.
	 */
	void (*func)(const Arg *);

	/**
	 * Argument for the called function.
	 */
	const Arg arg;
} Key;

/**
 * Button structure storing the most important information about button.
 */
typedef struct {

	/**
	 * Button mask. Usually MOD1 or MOD4.
	 */
	unsigned int mask;

	/**
	 * Button (e.g. Button1, Button3, etc.).
	 */
	unsigned int button;

	/**
	 * Function called when button is pressed.
	 */
	void (*func)(const Arg *);

	/**
	 * Argument for the called function.
	 */
	const Arg arg;
} Button;


/**
 * Screen height (in pixels).
 */
extern int sh;

/**
 * Screen width (in pixels).
 */
extern int sw;

/**
 * Default screen number.
 */
extern int screen;

/**
 * Currently focused window border color.
 */
extern unsigned int win_focus;

/**
 * Left desktop unfocused window border color.
 */
extern unsigned int left_win_unfocus;

/**
 * Right desktop unfocused window border color.
 */
extern unsigned int right_win_unfocus;

/**
 * Minimum window size (in pixels, length of the window's side).
 */
extern unsigned int min_window_size;

/**
 * Display structure.
 */
extern Display *dpy;

/**
 * Root window.
 */
extern Window root;

/**
 * Window attributes structure.
 */
extern XSetWindowAttributes wa;

/**
 * Atom.
 */
extern Atom wmatoms[WM_COUNT];


/**
 * Prints debug message and exits with EXIT_FAILURE.
 */
extern void die(const char *errstr, ...);

/**
 * Starts event loop.
 */
extern void run(void);

/**
 * Setup all necessary variables that splitwm needs to run.
 */
extern void setup(void);

/**
 * Install SIGCHLD handler.
 */
extern void sigchld(int sig);

/**
 * Executes user defined command (e.g. launch terminal emulator, etc.).
 */
extern void spawn(const Arg *arg);

/**
 * Quits splitwm.
 */
extern void quit(const Arg *arg);

/**
 * Tries to handle some annoying errors (e.g. BadMatch).
 */
extern int xerror(Display *dis, XErrorEvent *ee);


#endif /* _SPLITWM_H */

/* vim: set ts=8 sts=8 sw=8 : */
