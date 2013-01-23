/* See LICENSE file for copyright and license details */

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>

#include "splitwm.h"
#include "tile.h"
#include "client.h"
#include "desktop.h"
#include "mouse.h"
#include "config.h"
#include "draw.h"
#include "grab.h"
#include "event.h"
#include "dbg.h"
#include "view.h"


/** Functions **/

void kill_client(const Arg *arg)
{
	dbg("kill_client(): IN\n");
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
		dbg("kill_client(): n < 0\n");
	} else {
		send_kill_signal(d->curr->win);
		dbg("kill_client(): else\n");
	}
	
	if (prot)
		XFree(prot);
	dbg("kill_client(): OUT\n");
}

void send_kill_signal(Window w)
{
	dbg("send_kill_signal(): IN\n");
	XEvent e;
	e.type = ClientMessage;
	e.xclient.window = w;
	e.xclient.format = 32;
	e.xclient.message_type = wmatoms[WM_PROTOCOLS];
	e.xclient.data.l[0] = wmatoms[WM_DELETE_WINDOW];
	e.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, w, False, NoEventMask, &e);
	dbg("send_kill_signal(): OUT\n");
}


void client_to_desktop(const Arg *arg)
{
	dbg("client_to_desktop(): IN\n");
	dbg("client_to_desktop(): %d\n", arg->i);
	Client *c = NULL;
	Desktop *d = NULL;
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
	dbg("client_to_desktop(): OUT\n");
}

void addwindow(Window w)
{
	dbg("addwindow(): IN\n");
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
	dbg("addwindow(): OUT\n");
	printstatus();
}

void nextwindow(const Arg *arg)
{
	dbg("nextwindow(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (d->head && d->curr) {
		if (d->curr->next) {
			d->curr = d->curr->next;
		} else {
			d->curr = d->head;
		}
	}
	focuscurrent();
	dbg("nextwindow(): OUT\n");
}

void fullscreen(const Arg *arg)
{
	dbg("fullscreen(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	if (d->curr)
		XMoveResizeWindow(dpy, d->curr->win, 0, bar_height,
				  sw - 2 * BORDER_WIDTH, sh - 2 * BORDER_WIDTH - bar_height);
	dbg("fullscreen(): OUT\n");
}

void maximize(Window w)
{
	dbg("maximize(): IN\n");
	int sep = ((views[cv_id].right_view_activated) ? (0) : (SEPARATOR_WIDTH / 2));
	int wx, wy, ww, wh;

	if (BAR_POSITION == TOP) {
		wy = BORDER_OFFSET + bar_height;
	} else if (BAR_POSITION == BOTTOM || BAR_POSITION == NONE) {
		wy = BORDER_OFFSET;
	}
	wh = views[cv_id].split_height_y - 2 * BORDER_WIDTH - 2 * BORDER_OFFSET;

	if (BAR_POSITION == BOTTOM) {
		if (EXTERNAL_BAR_POSITION == TOP) {
			wy = EXTERNAL_BAR_HEIGHT + BORDER_OFFSET;
			wh -= EXTERNAL_BAR_HEIGHT;
		}
	} else if (BAR_POSITION == TOP) {
		if (EXTERNAL_BAR_POSITION == BOTTOM) {
			wy = BORDER_OFFSET + bar_height;
			wh -= EXTERNAL_BAR_HEIGHT;
		}
	} else if (BAR_POSITION == NONE) {
		if (EXTERNAL_BAR_POSITION == BOTTOM) {
			wy = BORDER_OFFSET;
			wh -= EXTERNAL_BAR_HEIGHT;
		} else if (EXTERNAL_BAR_POSITION == TOP) {
			wy = EXTERNAL_BAR_HEIGHT + BORDER_OFFSET;
			wh -= EXTERNAL_BAR_HEIGHT;
		}
	}

	if (views[cv_id].curr_desk == LEFT) {
		wx = BORDER_OFFSET;
		ww = views[cv_id].split_width_x - 2 * BORDER_WIDTH - 2 * BORDER_OFFSET - SEPARATOR_WIDTH / 2;

	} else {
		wx = views[cv_id].split_width_x + BORDER_OFFSET + sep;
		ww = sw  - views[cv_id].split_width_x - 2 * BORDER_WIDTH - 2 * BORDER_OFFSET - sep;
	}
	move_resize_window(w, wx, wy, ww, wh);
	dbg("maximize(): OUT\n");
}

void maximize_current(const Arg *arg)
{
	dbg("maximize_current(): IN\n");
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	if (d->curr)
		maximize(d->curr->win);
	dbg("maximize_current(): OUT\n");
}

void removewindow(Window w)
{
	dbg("removewindow(): IN\n");
	Client *c = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next) {
		dbg("removewindow(): for\n");
		if (c->win == w) {
			if (!c->prev && !c->next) {
				dbg("removewindow(): HEAD ONLY WINDOW\n");
				d->head = NULL;
				d->curr = NULL;
				return;
			} else if (!c->prev) {
				dbg("removewindow(): HEAD WINDOW\n");
				d->head = c->next;
				c->next->prev = NULL;
				d->curr = c->next;
			} else if (!c->next) {
				dbg("removewindow(): LAST WINDOW\n");
				c->prev->next = NULL;
				d->curr = c->prev;
			} else {
				dbg("removewindow(): MID WINDOW\n");
				c->prev->next = c->next;
				c->next->prev = c->prev;
				d->curr = c->next;
			}
			free(c);
			return;
		}
	}
	dbg("removewindow(): OUT\n");
}

void move_resize_window(Window win, int x, int y, int w, int h)
{
	dbg("move_resize_window(): IN\n");
	XMoveResizeWindow(dpy, win, x, y, w, h);
	XSync(dpy, False);
	dbg("move_resize_window(): OUT\n");
}

void focuscurrent(void)
{
	dbg("focuscurrent(): IN\n");
	Client *c = NULL;
	Client *n = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop())) {
		dbg("FC 1\n");
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
			XSetWindowBorderWidth(dpy, c->win, BORDER_WIDTH);
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
			//XRaiseWindow(dpy, c->win);
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
	dbg("focuscurrent(): OUT\n");
}


/* vim: set ts=8 sts=8 sw=8 : */
