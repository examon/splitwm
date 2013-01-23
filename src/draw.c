/* See LICENSE file for copyright and license details */

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "splitwm.h"
#include "draw.h"
#include "client.h"
#include "desktop.h"
#include "dbg.h"
#include "event.h"
#include "tile.h"
#include "mouse.h"
#include "config.h"


/** Variables **/

int bar_height;
int bar_y;
int font_height;
unsigned int font_top_offset;
unsigned int font_bottom_offset;
Colormap color_map;
XFontStruct *font_struct;
Pixmap bar_buffer;
GC gc;
Window bar;
Window separator;


/** Functions **/

void prepare_draw(void)
{
	dbg("prepare_draw(): IN\n");
	color_map = DefaultColormap(dpy, screen);
	XGCValues val;
	val.font = XLoadFont(dpy, FONT);
	font_struct = XQueryFont(dpy, val.font);
	font_height = font_struct->ascent - 1 + font_top_offset;
	bar_height = font_struct->ascent + font_struct->descent - 2
		     + font_top_offset + font_bottom_offset;
	gc = XCreateGC(dpy, root, GCFont, &val);

	if (BAR_POSITION == BOTTOM) {
		bar_y = sh - bar_height;
	} else if (BAR_POSITION == TOP) {
		bar_y = 0;
	}
	create_bar();
	create_map_separator();
	dbg("prepare_draw(): OUT\n");
}

void create_map_separator(void)
{
	XColor color;
	XAllocNamedColor(dpy, color_map, BAR_BG_COLOR, &color, &color);
	separator = XCreateSimpleWindow(dpy, root, 1, 1, 1, 1, 0, 0, color.pixel);
	XMapWindow(dpy, separator);
}

void create_bar(void)
{
	bar = XCreateSimpleWindow(dpy, root, 0, bar_y, sw, bar_height, 0, 0, 0);
	bar_buffer = XCreatePixmap(dpy, root, sw, bar_height, DefaultDepth(dpy, screen));
}

void map_bar(void)
{
	if (BAR_POSITION != NONE) {
		XChangeWindowAttributes(dpy, bar, CWOverrideRedirect|CWEventMask, &wa);
		XMapWindow(dpy, bar);
	}
}

void draw_separator(void)
{
	dbg("draw_separator(): IN\n");
	move_resize_window(separator,
			   views[cv_id].split_width_x - SEPARATOR_WIDTH/2,
			   0,
			   SEPARATOR_WIDTH,
			   sh);
	dbg("draw_separator(): OUT\n");
}

void draw(void)
{
	dbg("draw(): IN\n");
	if (SHOW_SEPARATOR)
		draw_separator();
	if (BAR_POSITION != NONE) {
		draw_bar();
		draw_tags();
		XCopyArea(dpy, bar_buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
		XRaiseWindow(dpy, bar);
		XFlush(dpy);
	}
	dbg("draw(): OUT\n");
}

void draw_bar(void)
{
	dbg("draw_bar(): IN\n");
	XColor color;
	XAllocNamedColor(dpy, color_map, BAR_BG_COLOR, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XFillRectangle(dpy, bar_buffer, gc, 0, 0, sw, bar_height);
	dbg("draw_bar(): OUT\n");
}

void draw_rectangle(const char *fill_color, int x, int y, int w, int h)
{
	XColor color;
	XAllocNamedColor(dpy, color_map, fill_color, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XFillRectangle(dpy, bar_buffer, gc, x, y, w, h);
}

void draw_string(const char *string_color, const char *string, int x, int h)
{
	XColor color;
	XAllocNamedColor(dpy, color_map, string_color, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	XDrawString(dpy, bar_buffer, gc, x, h, string, strlen(string));
}

void draw_tags(void)
{
	dbg("draw_tags(): IN\n");
	unsigned int char_width = font_struct->per_char->width;
	unsigned int x_left = BORDER_OFFSET;
	unsigned int x_right = sw - BORDER_OFFSET;
	unsigned int x_views = (sw / 2);
	unsigned int i, j;
	unsigned int max_title_len = 30;
	int x, y, w, h;
	char c[max_title_len];
	char *font_color = (char *) malloc(sizeof(char) * 7);

	if (views[cv_id].left_view_activated || views[cv_id].both_views_activated) {
		/* draw left desktop tags */
		for (i = 1; i <= DESKTOPS_LEFT; i++) {
			sprintf(c, "%s", tags_left[i-1]);
			x = x_left;
			y = 2;
			w = strlen(c) * char_width + 2 * CHAR_SPACE;
			h = bar_height - 4;
			if (views[cv_id].ld[i].head && views[cv_id].curr_left_id != i) {
				draw_rectangle(LEFT_TAG_OCCUPIED_BG, x + CHAR_SPACE,
						font_height + 2, w - 2 * CHAR_SPACE, 2);
				font_color = strcpy(font_color, LEFT_TAG_OCCUPIED_FG);
			} else if (views[cv_id].curr_left_id == i) {
				if (views[cv_id].curr_desk == LEFT) {
					draw_rectangle(FOCUS_COLOR, x, y, w, h);
					font_color = strcpy(font_color, BAR_BG_COLOR);
				} else {
					draw_rectangle(LEFT_TAG_FOCUS_BG, x, y, w, h);
					font_color = strcpy(font_color, LEFT_TAG_FOCUS_FG);
				}
			} else {
				draw_rectangle(LEFT_TAG_NORMAL_BG, x, y, w, h);
				font_color = strcpy(font_color, LEFT_TAG_NORMAL_FG);
			}
			draw_string(font_color, c, x_left + CHAR_SPACE, font_height);
			x_left += strlen(c) * char_width + 2 * CHAR_SPACE;
		}

		/* Draw layout tag for left desktop */
		if (views[cv_id].ld[views[cv_id].curr_left_id].layout == TILE) {
			sprintf(c, "%s", TILE_TAG);
		} else {
			sprintf(c, "%s", FLOAT_TAG);
		}
		draw_string(LEFT_TAG_NORMAL_FG, c, x_left + CHAR_SPACE, font_height);
		x_left += strlen(c) * char_width + 2 * CHAR_SPACE;

		/* Draw window title for left desktop */
		if (views[cv_id].ld[views[cv_id].curr_left_id].head) {
			if (views[cv_id].ld[views[cv_id].curr_left_id].curr->title == NULL) {
				sprintf(c, "%s", "unnamed");
				draw_string(LEFT_TAG_NORMAL_FG, c, x_left + CHAR_SPACE, font_height);
			} else {
				strncpy(c, views[cv_id].ld[views[cv_id].curr_left_id].curr->title, max_title_len);
				draw_string(LEFT_TAG_NORMAL_FG, c, x_left + CHAR_SPACE, font_height);
			}
		}
	}

	/* draw views tags */
	for (i= 0; i < VIEWS; i++)
		x_views -= (font_struct->per_char->width * strlen(tags_views[i]) / 2 + CHAR_SPACE);
	for (i = 1; i <= VIEWS; i++) {
		sprintf(c, "%s", tags_views[i-1]);
		x = x_views;
		y = 2;
		w = strlen(c) * char_width + 2 * CHAR_SPACE;
		h = bar_height - 4;
		Bool view_used = False;
		for (j = 1; j <= DESKTOPS_LEFT; j++)
			if (views[i].ld[j].head)
				view_used = True;
		for (j = 1; j <= DESKTOPS_RIGHT; j++)
			if (views[i].rd[j].head)
				view_used = True;
		if (i == cv_id) {
			draw_rectangle(VIEW_TAG_FOCUS_BG, x, y, w, h);
			font_color = strcpy(font_color, VIEW_TAG_FOCUS_FG);
		} else if (view_used && i != cv_id) {
			draw_rectangle(VIEW_TAG_OCCUPIED_BG, x + CHAR_SPACE,
				       font_height + 2, w - 2 * CHAR_SPACE, 2);
			font_color = strcpy(font_color, VIEW_TAG_OCCUPIED_FG);
		} else {
			draw_rectangle(VIEW_TAG_NORMAL_BG, x, y, w, h);
			font_color = strcpy(font_color, VIEW_TAG_NORMAL_FG);
		}
		draw_string(font_color, c, x_views + CHAR_SPACE, font_height);
		x_views += strlen(c) * char_width + 2 * CHAR_SPACE;
	}

	if (views[cv_id].right_view_activated || views[cv_id].both_views_activated) {
		/* calculate x_right */
		for (i = 0; i < DESKTOPS_RIGHT; i++)
			x_right -= strlen(tags_right[i]) * char_width + 2 * CHAR_SPACE;

		/* draw layout tag for right desktop */
		if (views[cv_id].rd[views[cv_id].curr_right_id].layout == TILE) {
			sprintf(c, "%s", TILE_TAG);
		} else {
			sprintf(c, "%s", FLOAT_TAG);
		}
		x_right -= strlen(c) * char_width + CHAR_SPACE;
		draw_string(RIGHT_TAG_NORMAL_FG, c, x_right, font_height);
		unsigned int right_layout_tag_len = strlen(c) * char_width + CHAR_SPACE;

		/* Draw window title for right desktop */
		if (views[cv_id].rd[views[cv_id].curr_right_id].head) {
			if (views[cv_id].rd[views[cv_id].curr_right_id].curr->title == NULL) {
				sprintf(c, "%s", "unnamed");
				unsigned int title_length = strlen(c) * char_width + 2 * CHAR_SPACE;
				x_right -= title_length;
				draw_string(RIGHT_TAG_NORMAL_FG, c, x_right, font_height);
				x_right += title_length;
			} else {
				strncpy(c, views[cv_id].rd[views[cv_id].curr_right_id].curr->title, max_title_len);
				unsigned int title_length = strlen(c) * char_width + 2 * CHAR_SPACE;
				x_right -= title_length;
				draw_string(RIGHT_TAG_NORMAL_FG, c, x_right, font_height);
				x_right += title_length;
			}
		}
		x_right += right_layout_tag_len;

		/* draw right desktop tags */
		for (i = 1; i <= DESKTOPS_RIGHT; i++) {
			sprintf(c, "%s", tags_right[i-1]);
			x = x_right;
			y = 2;
			w = strlen(c) * char_width + 2 * CHAR_SPACE;
			h = bar_height - 4;

			if (views[cv_id].rd[i].head && views[cv_id].curr_right_id != i) {
				draw_rectangle(RIGHT_TAG_OCCUPIED_BG, x + CHAR_SPACE,
						font_height + 2, w - 2 * CHAR_SPACE, 2);
				font_color = strcpy(font_color, RIGHT_TAG_OCCUPIED_FG);
			} else if (views[cv_id].curr_right_id == i) {
				if (views[cv_id].curr_desk == RIGHT) {
					draw_rectangle(FOCUS_COLOR, x, y, w, h);
					font_color = strcpy(font_color, BAR_BG_COLOR);
				} else {
					draw_rectangle(RIGHT_TAG_FOCUS_BG, x, y, w, h);
					font_color = strcpy(font_color, RIGHT_TAG_FOCUS_FG);
				}
			} else {
				draw_rectangle(RIGHT_TAG_NORMAL_BG, x, y, w, h);
				font_color = strcpy(font_color, RIGHT_TAG_NORMAL_FG);
			}
			draw_string(font_color, c, x_right + CHAR_SPACE, font_height);
			x_right += strlen(c) * char_width + 2 * CHAR_SPACE;
		}
	}
	dbg("draw_tags(): OUT\n");
}


/* vim: set ts=8 sts=8 sw=8 : */
