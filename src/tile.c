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
	/* DBG */	dbg("tile(): IN\n");
	/** 
	 * +-----+---+
	 * |     |   |
	 * |     +---+
	 * |     |   |
	 * |     +---+
	 * |     |   |
	 * +-----+---+
	 */

	if (d->layout == FLOAT)
		return;

	Client *c = NULL;
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
	d->layout = TILE;
	draw();
	/* DBG */	dbg("tile(): OUT\n");
}

void tile_current(const Arg *arg)
{
	/* DBG */	dbg("tile_current(): IN\n");
	Desktop *d = NULL;

	if (!(d = get_current_desktop()))
		return;
	d->layout = TILE;
	tile(d);
	/* DBG */	dbg("tile_current(): OUT\n");
}


/* vim: set ts=8 sts=8 sw=8 : */
