/* See LICENSE file for copyright and license details */

#ifndef _DRAW_H
#define _DRAW_H


/** Variables **/

extern int bar_height;
extern int bar_y;
extern int font_height;
extern unsigned int font_top_offset;
extern unsigned int font_bottom_offset;
extern Colormap color_map;
extern XFontStruct *font_struct;
extern Pixmap bar_buffer;
extern GC gc;
extern Window bar;
extern Window separator;


/** Functions **/

extern void prepare_draw(void);
extern void create_map_separator(void);
extern void create_bar(void);
extern void map_bar(void);
extern void draw_separator(void);
extern void draw(void);
extern void draw_bar(void);
extern void draw_rectangle(const char *fill_color, int x, int y, int w, int h);
extern void draw_string(const char *string_color, const char *string, int x, int h);
extern void draw_tags(void);


#endif /* _DRAW_H */


/* vim: set ts=8 sts=8 sw=8 : */
