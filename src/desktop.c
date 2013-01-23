/* See LICENSE file for copyright and license details */

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "splitwm.h"
#include "desktop.h"
#include "tile.h"
#include "mouse.h"
#include "config.h"
#include "dbg.h"
#include "draw.h"


/** Functions **/

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

void change_left_desktop(const Arg *arg)
{
	/* DBG */	dbg("change_left_desktop(): IN\n");
	Client *c = NULL;
	Desktop *d = &views[cv_id].ld[views[cv_id].curr_left_id];
	Desktop *n = &views[cv_id].ld[arg->i];

	if (arg->i == views[cv_id].curr_left_id || arg->i > DESKTOPS_LEFT)
		return;
	if (views[cv_id].curr_desk == RIGHT && views[cv_id].right_view_activated) {
		return;
	}
	for (c = d->head; d->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = n->head; n->head && c; c = c->next)
		XMapWindow(dpy, c->win);
	views[cv_id].prev_left_id = views[cv_id].curr_left_id;
	views[cv_id].curr_left_id = arg->i;

	if (views[cv_id].curr_desk == LEFT) {
		tile(n);
	} else if (views[cv_id].curr_desk == RIGHT) {
		views[cv_id].curr_desk = LEFT;
		tile(n);
		views[cv_id].curr_desk = RIGHT;
	} else if (views[cv_id].left_view_activated) {
		tile(n);
	}

	focuscurrent();
	/* DBG */	dbg("change_left_desktop(): OUT\n");
	/* DBG */	printstatus();
}

void change_right_desktop(const Arg *arg)
{
	/* DBG */	dbg("change_right_desktop(): IN\n");
	Client *c = NULL;
	Desktop *d = &views[cv_id].rd[views[cv_id].curr_right_id];
	Desktop *n = &views[cv_id].rd[arg->i];

	if (arg->i == views[cv_id].curr_right_id || arg->i > DESKTOPS_RIGHT)
		return;
	if (views[cv_id].curr_desk == LEFT && views[cv_id].left_view_activated) {
		return;
	}
	for (c = d->head; d->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = n->head; n->head && c; c = c->next)
			XMapWindow(dpy, c->win);
	views[cv_id].prev_right_id = views[cv_id].curr_right_id;
	views[cv_id].curr_right_id = arg->i;
	
	if (views[cv_id].curr_desk == RIGHT) {
		tile(n);
	} else if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
		tile(n);
		views[cv_id].curr_desk = LEFT;
	} else if (views[cv_id].right_view_activated) {
		tile(n);
	}

	focuscurrent();
	/* DBG */	dbg("change_right_desktop(): OUT\n");
	/* DBG */	printstatus();
}

void previous_desktop(const Arg *arg)
{
	/* DBG */	dbg("previous_desktop(): IN\n");
	if (views[cv_id].curr_desk == LEFT) {
		Arg a = { .i = views[cv_id].prev_left_id };
		change_left_desktop(&a);
	} else {
		Arg a = { .i = views[cv_id].prev_right_id };
		change_right_desktop(&a);
	}
	/* DBG */	dbg("previous_desktop(): OUT\n");
}

void master_size_increase(const Arg *arg)
{
	/* DBG */	dbg("MASTER_SIZE_INCrease(): IN\n");
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	if ((views[cv_id].curr_desk == LEFT && d->master_size + 2 * MASTER_SIZE_INC > views[cv_id].split_width_x)
	 || (views[cv_id].curr_desk == RIGHT && views[cv_id].split_width_x + d->master_size + 2 * MASTER_SIZE_INC > sw))
		return;
	d->master_size += MASTER_SIZE_INC;
	tile(d);
	/* DBG */	dbg("MASTER_SIZE_INCrease(): OUT\n");
}

void master_size_decrease(const Arg *arg)
{
	/* DBG */	dbg("MASTER_SIZE_DECrease(): IN\n");
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	if ((views[cv_id].curr_desk == LEFT && d->master_size < 2 * MASTER_SIZE_DEC)
	 || (views[cv_id].curr_desk == RIGHT && views[cv_id].split_width_x + d->master_size
	   < views[cv_id].split_width_x + 2 * MASTER_SIZE_DEC))
		return;
	d->master_size -= MASTER_SIZE_DEC;
	tile(d);
	/* DBG */	dbg("MASTER_SIZE_DECrease(): OUT\n");
}

void toggle_float(const Arg *arg)
{
	/* DBG */	dbg("toggle_current(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	d->layout = FLOAT;
	draw();
	/* DBG */	dbg("toggle_current(): OUT\n");
}


/* vim: set ts=8 sts=8 sw=8 : */
