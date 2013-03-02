/* See LICENSE file for copyright and license details */

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "splitwm.h"
#include "client.h"
#include "desktop.h"
#include "tile.h"
#include "mouse.h"
#include "config.h"
#include "view.h"
#include "draw.h"
#include "dbg.h"


int cv_id;
int pv_id;
View views[13];


void activate_left_view(const Arg *arg)
{
	if (views[cv_id].right_view_activated)
		activate_both_views(0);
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].curr_desk == RIGHT)
		nextview(0);
	views[cv_id].split_width_x = sw + SEPARATOR_WIDTH / 2;
	draw();
	Arg a = { .i = 0 };
	change_right_desktop(&a);
	views[cv_id].left_view_activated = True;
	views[cv_id].both_views_activated = False;
	views[cv_id].right_view_activated = False;
	tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	focuscurrent();
}

void activate_right_view(const Arg *arg)
{
	if (views[cv_id].left_view_activated)
		activate_both_views(0);
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].curr_desk == LEFT)
		nextview(0);
	views[cv_id].split_width_x = 0;
	draw();
	Arg a = { .i = 0 };
	change_left_desktop(&a);
	views[cv_id].right_view_activated = True;
	views[cv_id].both_views_activated = False;
	views[cv_id].left_view_activated = False;
	tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	focuscurrent();
}

void activate_both_views(const Arg *arg)
{
	if (views[cv_id].both_views_activated)
		return;
	unsigned int i;
	views[cv_id].split_width_x = sw / 2;

	for (i = 1; i <= DESKTOPS_LEFT; i++) {
		views[cv_id].ld[i].tile_or_float = TILE;
		views[cv_id].ld[i].master_size =
			((MASTER_SIZE) ? (MASTER_SIZE) : (views[cv_id].split_width_x / 2));
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	}
	for (i = 1; i <= DESKTOPS_RIGHT; i++) {
		views[cv_id].rd[i].tile_or_float = TILE;
		views[cv_id].rd[i].master_size =
			((MASTER_SIZE) ? (MASTER_SIZE) : (views[cv_id].split_width_x / 2));
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	}
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
		previous_desktop(0);
		views[cv_id].curr_desk = LEFT;
	} else if (views[cv_id].curr_desk == RIGHT) {
		views[cv_id].curr_desk = LEFT;
		previous_desktop(0);
		views[cv_id].curr_desk = RIGHT;
	}
	views[cv_id].left_view_activated = False;
	views[cv_id].right_view_activated = False;
	views[cv_id].both_views_activated = True;
	focuscurrent();
}

void client_to_view(const Arg *arg)
{
	dbg("client_to_view(): IN\n");
	Client *c = NULL;
	Desktop *d = NULL;
	
	if (!(d = get_current_desktop()))
		return;
	c = d->curr;
	if (!c || views[cv_id].left_view_activated || views[cv_id].right_view_activated)
		return;
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
		addwindow(c->win);
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
		focuscurrent();
		views[cv_id].curr_desk = LEFT;
		removewindow(c->win);
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
	} else if (views[cv_id].curr_desk == RIGHT) {
		views[cv_id].curr_desk = LEFT;
		addwindow(c->win);
		tile(&views[cv_id].ld[views[cv_id].curr_left_id]);
		focuscurrent();
		views[cv_id].curr_desk = RIGHT;
		removewindow(c->win);
		tile(&views[cv_id].rd[views[cv_id].curr_right_id]);
	}
	focuscurrent();
	dbg("client_to_view(): OUT\n");
}

void change_view(const Arg *arg)
{
	dbg("change_view(): IN\n");
	dbg("change_view(): %d\n", arg->i);
	if (arg->i == cv_id || arg->i > VIEWS)
		return;

	Client *c = NULL;
	Desktop *ld = &views[cv_id].ld[views[cv_id].curr_left_id];
	Desktop *rd = &views[cv_id].rd[views[cv_id].curr_right_id];
	Desktop *ln = &views[arg->i].ld[views[arg->i].curr_left_id];
	Desktop *rn = &views[arg->i].rd[views[arg->i].curr_right_id];

	for (c = ld->head; ld->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = rd->head; rd->head && c; c = c->next)
		XUnmapWindow(dpy, c->win);
	for (c = ln->head; ln->head && c; c = c->next)
		XMapWindow(dpy, c->win);
	for (c = rn->head; rn->head && c; c = c->next)
		XMapWindow(dpy, c->win);

	pv_id = cv_id;
	cv_id = arg->i;
	Arg a = { .i = views[cv_id].curr_left_id };
	Arg b = { .i = views[cv_id].curr_right_id };
	change_left_desktop(&a);
	change_right_desktop(&b);

	if (views[arg->i].both_views_activated) {
		activate_both_views(0);
	} else if (views[arg->i].left_view_activated) {
		activate_left_view(0);
	} else if (views[arg->i].right_view_activated) {
		activate_right_view(0);
	}
	draw();
	dbg("change_views(): OUT\n");
}

void nextview(const Arg *arg)
{
	dbg("nextview(): IN\n");
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].curr_desk == LEFT) {
		views[cv_id].curr_desk = RIGHT;
	} else {
		views[cv_id].curr_desk = LEFT;
	}
	focuscurrent();
	dbg("nextview(): OUT\n");
}

void previous_view(const Arg *arg)
{
	dbg("previous_view(): IN\n");
	Arg a = { .i = pv_id };
	change_view(&a);
	dbg("previous_view(): OUT\n");
}

void separator_increase(const Arg *arg)
{
	dbg("separator_increase(): IN\n");
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].split_width_x + views[cv_id].rd[views[cv_id].curr_right_id].master_size
	  < sw - min_window_size - SEPARATOR_INC + SEPARATOR_WIDTH)
		views[cv_id].split_width_x += SEPARATOR_INC;
	views[cv_id].rd[views[cv_id].curr_right_id].master_size = (sw - views[cv_id].split_width_x) / 2;
	views[cv_id].ld[views[cv_id].curr_left_id].master_size = views[cv_id].split_width_x / 2;
	draw_separator();
	if (views[cv_id].curr_desk == LEFT)
		views[cv_id].curr_desk = RIGHT;
	tile_current(0);
	views[cv_id].curr_desk = LEFT;
	tile_current(0);
	dbg("separator_increase(): OUT\n");
}

void separator_decrease(const Arg *arg)
{
	dbg("separator_decrease(): IN\n");
	if (!views[cv_id].both_views_activated)
		return;
	if (views[cv_id].split_width_x > min_window_size + 2 * SEPARATOR_DEC)
		views[cv_id].split_width_x -= SEPARATOR_DEC;
	views[cv_id].rd[views[cv_id].curr_right_id].master_size = (sw - views[cv_id].split_width_x) / 2;
	views[cv_id].ld[views[cv_id].curr_left_id].master_size = views[cv_id].split_width_x / 2;
	draw_separator();
	if (views[cv_id].curr_desk == RIGHT)
		views[cv_id].curr_desk = LEFT;
	tile_current(0);
	views[cv_id].curr_desk = RIGHT;
	tile_current(0);
	dbg("separator_decrease(): OUT\n");
}

void update_geometry(void)
{
	unsigned int n;
	for (n = 0; n <= VIEWS; n++) {
		views[n].split_width_x = (float) sw / 2;
		views[n].split_height_y =
			sh - ((BAR_POSITION != NONE) ? (bar_height) : (0));
	}

	unsigned int i, j, k;
	for (i = 1; i <= VIEWS; i++) {
		for (j = 0; j <= DESKTOPS_LEFT; j++) {
			views[i].ld[j].master_size =
				((MASTER_SIZE != 0) ? (MASTER_SIZE)
				 : (views[cv_id].split_width_x / 2));
		}
		for (k = 0; k <= DESKTOPS_RIGHT; k++) {
			views[i].rd[k].master_size =
				((MASTER_SIZE != 0) ? (MASTER_SIZE)
				 : (views[cv_id].split_width_x / 2));
		}
	}
}

/* vim: set ts=8 sts=8 sw=8 : */
