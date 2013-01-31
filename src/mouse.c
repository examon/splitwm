/* See LICENSE file for copyright and license details */

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "splitwm.h"
#include "mouse.h"
#include "tile.h"
#include "config.h"
#include "dbg.h"
#include "draw.h"
#include "event.h"

/** Functions **/

void mousemove(const Arg *arg)
{
	dbg("mousemotion(): IN\n");
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
		XWarpPointer(dpy, d->curr->win, d->curr->win, 0, 0, 0, 0, --wa.width, --wa.height);
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
			left_wall = (xw + wa.width + 2 * BORDER_WIDTH \
				   < views[cv_id].split_width_x + views[cv_id].split_width_x / 10 - SEPARATOR_WIDTH / 2);
			right_wall = (xw \
				   > views[cv_id].split_width_x - views[cv_id].split_width_x / 10 + SEPARATOR_WIDTH / 2);
			if (arg->i == RESIZE) {
				d->tile_or_float = FLOAT;
				XResizeWindow(dpy, d->curr->win,
					      (xw > min_window_size) ? xw : wa.width,
					      (yh > min_window_size) ? yh : wa.height);
			} else if (arg->i == MOVE && views[cv_id].curr_desk == LEFT) {
				d->tile_or_float = FLOAT;
				if (views[cv_id].left_view_activated
				 || (!views[cv_id].left_view_activated && left_wall)) {
					XMoveWindow(dpy, d->curr->win, xw, yh);
				} else if (!views[cv_id].left_view_activated && !left_wall) {
					/* mouse draw window from left desktop to right */
					client_to_view(0);
					tile_current(0);
					views[cv_id].curr_desk = RIGHT;
					if (views[cv_id].rd[views[cv_id].curr_right_id].tile_or_float == TILE) {
						tile_current(0);
					} else {
						maximize_current(0);
					}
					views[cv_id].curr_desk = LEFT;
					XUngrabPointer(dpy, CurrentTime);
					draw();
					return;
				}
			} else if (arg->i == MOVE && views[cv_id].curr_desk == RIGHT) {
				d->tile_or_float = FLOAT;
				if (views[cv_id].right_view_activated
				 || (!views[cv_id].right_view_activated && right_wall)) {
					XMoveWindow(dpy, d->curr->win, xw, yh);
				} else if (!views[cv_id].right_view_activated && !right_wall) {
					/* mouse draw window from right desktop to left */
					client_to_view(0);
					tile_current(0);
					views[cv_id].curr_desk = LEFT;
					if (views[cv_id].ld[views[cv_id].curr_left_id].tile_or_float == TILE) {
						tile_current(0);
					} else {
						maximize_current(0);
					}
					views[cv_id].curr_desk = RIGHT;
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
	dbg("mousemotion(): OUT\n");
}


/* vim: set ts=8 sts=8 sw=8 : */
