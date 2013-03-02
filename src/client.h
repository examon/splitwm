/* See LICENSE file for copyright and license details */

#ifndef _CLIENT_H
#define _CLIENT_H

#include <X11/Xlib.h>


/**
 * This structure is storing the most important client info.
 * Each client got its own Client structure.
 */
typedef struct Client {

	/**
	 * Pointer to the next Client.
	 */
	struct Client *next;

	/**
	 * Pointer to the previous Client.
	 */
	struct Client *prev;

	/**
	 * Title of the client's window.
	 */
	char *title;

	/**
	 * Title length.
	 */
	int title_len;

	/**
	 * Client's window.
	 */
	Window win;
} Client;

/**
 * Kills currently active client.
 *
 * Returns without any effect when:
 *  - current active client does not exist
 *  - when get_current_desktop() returned NULL
 */
extern void kill_client(const Arg *arg);

/**
 * Prepare and send kill event, so appropriate client can be killed.
 */
extern void send_kill_signal(Window w);

/**
 * Sends currently active client to the specified desktop.
 *
 * Returns without any effect when:
 *  - current active client does not exist
 *  - get_current_desktop() returned NULL
 *  - targeted desktop number (arg->i) is already active
 */
extern void client_to_desktop(const Arg *arg);

/**
 * Makes new Client for the window passed in as an argument
 * and inserts it into the list of all clients.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 *  - client calloc error occurs
 */
extern void addwindow(Window w);

/**
 * Focuses next client window.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void nextwindow(const Arg *arg);

/**
 * Sets currently active window to the fullscreen mode.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void fullscreen(const Arg *arg);

/**
 * Maximizes window passed in as an argument.
 */
extern void maximize(Window w);

/**
 * Maximizes currently active window.
 * 
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void maximize_current(const Arg *arg);

/**
 * Removes Client containing window passed in as an argument
 * from the list of all clients.
 */
extern void removewindow(Window w);

/**
 * Wrapper for the XMoveResizeWindow().
 */
extern void move_resize_window(Window win, int x, int y, int w, int h);

/**
 * Takes currently active window and gives this window input focus,
 * draws window border.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void focuscurrent(void);


#endif /* _CLIENT_H */

/* vim: set ts=8 sts=8 sw=8 : */
