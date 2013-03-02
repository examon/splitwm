/* See LICENSE file for copyright and license details */

/*! \file
 * \details This file is header for draw.c
 */

#ifndef _DRAW_H
#define _DRAW_H


/**
 * Bar height (in pixels).
 */
extern int bar_height;

/**
 * Bar y position on the axis (vertical position).
 */
extern int bar_y;

/**
 * Font height (in pixels).
 */
extern int font_height;

/**
 * Font top offset (in pixels).
 */
extern unsigned int font_top_offset;

/**
 * Font bottom offset (in pixels).
 */
extern unsigned int font_bottom_offset;

/**
 * Color Map.
 */
extern Colormap color_map;

/**
 * Font Structure.
 */
extern XFontStruct *font_struct;

/**
 * Bar Buffer.
 */
extern Pixmap bar_buffer;

/**
 * Graphics Context.
 */
extern GC gc;

/**
 * Bar window.
 */
extern Window bar;

/**
 * Split screen separator window.
 */
extern Window separator;


/**
 * Prepares everything needed for draw() (font, graphics context, etc.)
 * Creates bar (create_bar()) and creates and maps split screen separator
 * (create_map_separator()).
 */
extern void prepare_draw(void);

/**
 * Creates and maps (shows) split screen separator.
 */
extern void create_map_separator(void);

/**
 * Creates bar (and bar_buffer).
 */
extern void create_bar(void);

/**
 * Maps bar (shows bar on the screen).
 */
extern void map_bar(void);

/**
 * Draws split screen separator.
 */
extern void draw_separator(void);

/**
 * Draws multiple graphics object (separator, bar, tags).
 * Used when graphics should be refreshed.
 */
extern void draw(void);

/**
 * Draws bar to the bar_buffer.
 */
extern void draw_bar(void);

/**
 * Draws and fills rectangle.
 */
extern void draw_rectangle(const char *fill_color, int x, int y, int w, int h);

/**
 * Draws and colors string.
 */
extern void draw_string(const char *string_color, const char *string, int x, int h);

/**
 * Draws desktop & view tags, window titles in the bar.
 */
extern void draw_tags(void);


#endif /* _DRAW_H */

/* vim: set ts=8 sts=8 sw=8 : */
