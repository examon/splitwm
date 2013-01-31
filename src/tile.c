/* See LICENSE file for copyright and license details */

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "splitwm.h"
#include "tile.h"
#include "mouse.h"
#include "config.h"
#include "draw.h"
#include "dbg.h"


/** Functions **/

void tile(Desktop *d)
{
	dbg("tile(): IN\n");
	if (d->tile_or_float == FLOAT)
		return;
	
	if (d->layout == MASTER) {
		tile_layout_master(0);
	} else if (d->layout == GRID) {
		tile_layout_grid(0);
	}
	draw();
	dbg("tile(): OUT\n");
}

void tile_layout_master(const Arg *arg)
{
	dbg("tile_layout_master(): IN\n");
	/** 
	 * +-----+---+
	 * |     |   |
	 * |     +---+
	 * |     |   |
	 * |     +---+
	 * |     |   |
	 * +-----+---+
	 */

	Client *c = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	d->tile_or_float = TILE;
	d->layout = MASTER;
	draw();

	int n = 0;
	int bar = 0;
	int ext_top_bar = 0;
	int ext_bottom_bar = 0;
	int y = bar;
	int wx, wy, ww, wh;
	int sep = ((views[cv_id].right_view_activated) ? (0) : (SEPARATOR_WIDTH / 2));

	if (BAR_POSITION == TOP) {
		bar = bar_height;
	} else if (BAR_POSITION == BOTTOM || BAR_POSITION == NONE) {
		bar = 0;
	}
	
	if (EXTERNAL_BAR_POSITION == TOP && BAR_POSITION != TOP) {
		ext_top_bar = EXTERNAL_BAR_HEIGHT;
	} else if (EXTERNAL_BAR_POSITION == BOTTOM && BAR_POSITION != BOTTOM) {
		ext_bottom_bar = EXTERNAL_BAR_HEIGHT;
	}

	if (d->head && !d->head->next) {
		maximize(d->head->win);
		return;
	} else if (d->head && views[cv_id].curr_desk == LEFT) {
		wx = BORDER_OFFSET;
		wy = BORDER_OFFSET + bar + ext_top_bar;
		ww = d->master_size - 2 * BORDER_OFFSET - 2 * BORDER_WIDTH;
		wh = (views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar) - 2 * BORDER_OFFSET - 2 * BORDER_WIDTH;
		move_resize_window(d->curr->win, wx, wy, ww, wh);
		for (c = d->head; c; c = c->next)
			if (!(c == d->curr))
				++n;
		for (c = d->head; c; c = c->next) {
			if (c == d->curr)
				continue;
			wx = d->master_size;
			wy = y + BORDER_OFFSET + ext_top_bar + bar;
			ww = views[cv_id].split_width_x - d->master_size - BORDER_OFFSET - 2 \
			     * BORDER_WIDTH - SEPARATOR_WIDTH / 2,
			wh = (views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar) / n
			     - 2 * BORDER_OFFSET - 2 * BORDER_WIDTH;
			move_resize_window(c->win, wx, wy, ww, wh);
			y += (views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar) / n;
		}
	} else if (d->head && views[cv_id].curr_desk == RIGHT) {
		wx = views[cv_id].split_width_x + BORDER_OFFSET + sep;
		wy = BORDER_OFFSET + bar + ext_top_bar;
		ww = d->master_size - 2 * BORDER_OFFSET - 2 * BORDER_WIDTH;
		wh = (views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar) - 2 * BORDER_WIDTH - 2 * BORDER_OFFSET;
		move_resize_window(d->curr->win, wx, wy, ww, wh);
		for (c = d->head; c; c = c->next)
			if (!(c == d->curr))
				++n;
		for (c = d->head; c; c = c->next) {
			if (c == d->curr)
				continue;
			wx = views[cv_id].split_width_x + d->master_size + sep;
			wy = y + BORDER_OFFSET + ext_top_bar + bar;
			ww = sw - views[cv_id].split_width_x - d->master_size - 2 * BORDER_WIDTH \
			     - BORDER_OFFSET - sep;
			wh = (views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar) / n
			     - 2 * BORDER_WIDTH - 2 * BORDER_OFFSET;
			move_resize_window(c->win, wx, wy, ww, wh);
			y += (views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar) / n;
		}
	}
	dbg("tile_layout_master(): OUT\n");
}

void tile_layout_grid(const Arg *arg)
{
	dbg("tile_layout_grid(): IN\n");
	/** 
	 * +---+---+---+
	 * |   |   |   |
	 * |---+---+---+
	 * |   |   |   |
	 * |---+---+---+
	 * |   |   |   |
	 * +---+---+---+
	 */

	Client *c = NULL;
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	d->tile_or_float = TILE;
	d->layout = GRID;
	draw();

	int n = 0;
	for (Client *c = d->head; c; c = c->next)
		++n;

	int cols = 0;
	int cn = 0;
	int rn = 0;
	int i = -1;
	for (cols = 0; cols <= n / 2; cols++) {
		if (cols * cols >= n)
			break;
	}

	if (n == 0) {
		return;
	} else if (n == 5) {
		cols = 2;
	}

	int rows = n / cols;
	int sep = ((views[cv_id].right_view_activated) ? (0) : (SEPARATOR_WIDTH / 2));

	int bar = 0;
	int ext_top_bar = 0;
	int ext_bottom_bar = 0;

	if (BAR_POSITION == TOP) {
		bar = bar_height;
	} else if (BAR_POSITION == BOTTOM || BAR_POSITION == NONE) {
		bar = 0;
	}
	if (EXTERNAL_BAR_POSITION == TOP && BAR_POSITION != TOP) {
		ext_top_bar = EXTERNAL_BAR_HEIGHT;
	} else if (EXTERNAL_BAR_POSITION == BOTTOM && BAR_POSITION != BOTTOM) {
		ext_bottom_bar = EXTERNAL_BAR_HEIGHT;
	}

	int x, y, w, h;

	if (d->head && !d->head->next) {
		maximize(d->head->win);
		return;
	} else if (d->head && views[cv_id].curr_desk == LEFT) {
		x = BORDER_OFFSET;
		y = BORDER_OFFSET + bar + ext_top_bar;
		w = views[cv_id].split_width_x - sep;
		h = views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar;
	} else if (d->head && views[cv_id].curr_desk == RIGHT) {
		x = views[cv_id].split_width_x + BORDER_OFFSET + sep;
		y = BORDER_OFFSET + bar + ext_top_bar;
		w = sw - views[cv_id].split_width_x - sep;
		h = views[cv_id].split_height_y - ext_bottom_bar - ext_top_bar;
	}

	int ch = h - BORDER_WIDTH - BORDER_OFFSET;
	int cw = (w - BORDER_WIDTH - BORDER_OFFSET) / (cols ? cols:1);
	int x_adj = 0;
	int rn_max = rn;

	for (c = d->head; c; c = c->next) {
		++i;
		if (i / rows + 1 > cols - n % cols)
			rows = n / cols + 1;
		move_resize_window(c->win,
				   x + cn * cw + x_adj,
				   y + rn * ch / rows + (rn > 0 ? BORDER_WIDTH : 0),
				   cw - BORDER_WIDTH - BORDER_OFFSET,
				   ch / rows - (rn + 1) * BORDER_WIDTH - BORDER_OFFSET);
		if (rn_max < rn)
			rn_max = rn;
		if (++rn >= rows) {
			rn = 0;
			cn++;
			x_adj += BORDER_WIDTH;
		}
	}

	/* adjust the most right window borders */
	unsigned int cnt = 1;
	XWindowAttributes wa;
	for (c = d->head; c; c = c->next) {
		if (cnt++ >= n - rn_max) {
			XGetWindowAttributes(dpy, c->win, &wa);
			XResizeWindow(dpy, c->win, wa.width - (cn - 1) * BORDER_WIDTH, wa.height);
		}
	}
	dbg("tile_layout_grid(): OUT\n");
}


void tile_current(const Arg *arg)
{
	dbg("tile_current(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	d->tile_or_float = TILE;
	tile(d);
	dbg("tile_current(): OUT\n");
}


/* vim: set ts=8 sts=8 sw=8 : */
