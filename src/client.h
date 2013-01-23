/* See LICENSE file for copyright and license details */

#ifndef _CLIENT_H
#define _CLIENT_H

#include <X11/Xlib.h>


/** Structures **/

typedef struct Client {
	struct Client *next;
	struct Client *prev;
	char *title;
	int title_len;
	Window win;
} Client;


/** Functions **/

extern void kill_client(const Arg *arg);
extern void send_kill_signal(Window w);
extern void client_to_desktop(const Arg *arg);
extern void addwindow(Window w);
extern void nextwindow(const Arg *arg);
extern void fullscreen(const Arg *arg);
extern void maximize(Window w);
extern void maximize_current(const Arg *arg);
extern void removewindow(Window w);
extern void move_resize_window(Window win, int x, int y, int w, int h);
extern void focuscurrent(void);


#endif /* _CLIENT_H */


/* vim: set ts=8 sts=8 sw=8 : */
