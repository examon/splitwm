/* See LICENSE file for copyright and license details */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "view.h"


/** Font **/
#define  FONT	"-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-*"


/** Bar **/
#define  BAR_POSITION	TOP	/* TOP, BOTTOM or NONE */
#define  TILE_TAG	"[T]"	/* Tile tag shown in bar */
#define  FLOAT_TAG	"[F]"	/* float tag shown in bar */
#define  CHAR_SPACE	5 	/* spaces between bar tags */


/** external bar **/
#define  EXTERNAL_BAR_POSITION	NONE	/* TOP, BOTTOM or NONE */
#define  EXTERNAL_BAR_HEIGHT	20	/* external bar height in pixels */


/** Views **/
static const char *tags_views[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };	/* max 9 */
#define  DEFAULT_VIEW  1	/* default view */


/** Desktops **/
static const char  *tags_left[]  = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };	/* max 9 */
static const char  *tags_right[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };	/* max 9 */
#define  DEFAULT_LEFT_DESKTOP	1
#define  DEFAULT_RIGHT_DESKTOP	1
#define  MASTER_SIZE  		0	/* master window size, if 0 then master_size = screen_width/2 */
#define  MASTER_SIZE_INC	20 	/* master_size increase distance */
#define  MASTER_SIZE_DEC	20 	/* master_size decrease distance */


/** Window **/
#define  FOLLOW_MOUSE_FOCUS	True
#define  BORDER_WIDTH		4	/* window border width */
#define  BORDER_OFFSET		2	/* spaces betweed windows borders */


/** Separator **/
#define  SHOW_SEPARATOR		True	/* False to hide separator */
#define  SEPARATOR_WIDTH	4	/* width of the split seperator */
#define  SEPARATOR_INC		50	/* separator increase distance */
#define  SEPARATOR_DEC		50	/* separator decrease distance */


/** Colors **/
#define  FOCUS_COLOR            "#d87a16"
#define  LEFT_UNFOCUS_COLOR     "#005577"
#define  RIGHT_UNFOCUS_COLOR    "#288428"
#define  BAR_BG_COLOR           "#111111"

#define  LEFT_TAG_FOCUS_BG      "#005577"
#define  LEFT_TAG_FOCUS_FG      "#eeeeee"
#define  LEFT_TAG_NORMAL_BG     "#111111"
#define  LEFT_TAG_NORMAL_FG     "#bbbbbb"
#define  LEFT_TAG_OCCUPIED_BG   "#bbbbbb"
#define  LEFT_TAG_OCCUPIED_FG   "#bbbbbb"

#define  RIGHT_TAG_FOCUS_BG     "#288428"
#define  RIGHT_TAG_FOCUS_FG     "#eeeeee"
#define  RIGHT_TAG_NORMAL_BG    "#111111"
#define  RIGHT_TAG_NORMAL_FG    "#bbbbbb"
#define  RIGHT_TAG_OCCUPIED_BG  "#bbbbbb"
#define  RIGHT_TAG_OCCUPIED_FG  "#bbbbbb"

#define  VIEW_TAG_FOCUS_BG      "#686868"
#define  VIEW_TAG_FOCUS_FG      "#eeeeee"
#define  VIEW_TAG_NORMAL_BG     "#111111"
#define  VIEW_TAG_NORMAL_FG     "#bbbbbb"
#define  VIEW_TAG_OCCUPIED_BG   "#bbbbbb"
#define  VIEW_TAG_OCCUPIED_FG   "#bbbbbb"


/** Commands **/
static const char  *spawn_terminal[] = { "urxvt", NULL };
static const char  *spawn_dmenu[]    = { "dmenu_run", "-fn", FONT, "-nb", BAR_BG_COLOR, "-nf", VIEW_TAG_NORMAL_FG, "-sb", BAR_BG_COLOR, "-sf", FOCUS_COLOR, NULL };


/** Modifiers **/
#define MOD1		Mod1Mask	/* ALT key */
#define MOD4		Mod4Mask	/* Super/Win key */
#define SHIFT		ShiftMask	/* Shift key */


/** Cursor **/
#define CURSOR		XC_left_ptr	/* default cursor */


/** Anti-multiple paste macro **/
#define CLIENT_TO_DESKTOP(K, N) \
	{  MOD1|SHIFT,       K,             client_to_desktop,      { .i = N }}, \
	{  MOD4|SHIFT,       K,             client_to_desktop,      { .i = N }},

#define CHANGE_VIEW(K, N) \
	{  MOD1,             K,             change_view,            { .i = N }}, \
	{  MOD4,             K,             change_view,            { .i = N }},


/** Keyboard shortcuts **/
static Key keys[] = {
	/* modifier          key            function                argument */
	{  MOD1|SHIFT,       XK_q,          quit,                   { 0 }},
	{  MOD1,             XK_j,          nextwindow,             { 0 }},
	{  MOD4,             XK_j,          nextview,               { 0 }},
	{  MOD4|SHIFT,       XK_j,          client_to_view,         { 0 }},
	{  MOD4,             XK_v,          activate_left_view,     { 0 }},
	{  MOD1,             XK_v,          activate_right_view,    { 0 }},
	{  MOD1,             XK_b,          activate_both_views,    { 0 }},
	{  MOD4,             XK_b,          activate_both_views,    { 0 }},
	{  MOD1|SHIFT,       XK_c,          kill_client,            { 0 }},
	{  MOD4|SHIFT,       XK_c,          kill_client,            { 0 }},
	{  MOD1,             XK_f,          fullscreen,             { 0 }},
	{  MOD1,             XK_t,          tile_current,           { 0 }},
	{  MOD1|SHIFT,       XK_f,          toggle_float,           { 0 }},
	{  MOD1,             XK_space,      maximize_current,       { 0 }},
	{  MOD1,             XK_Tab,        previous_desktop,       { 0 }},
	{  MOD4,             XK_Tab,        previous_view,          { 0 }},
	{  MOD1|SHIFT,       XK_h,          separator_decrease,     { 0 }},
	{  MOD1|SHIFT,       XK_l,          separator_increase,     { 0 }},
	{  MOD4|SHIFT,       XK_h,          master_size_decrease,   { 0 }},
	{  MOD4|SHIFT,       XK_l,          master_size_increase,   { 0 }},
	{  MOD1,             XK_Return,     spawn,                  { .com = spawn_terminal }},
	{  MOD4,             XK_Return,     spawn,                  { .com = spawn_terminal }},
	{  MOD1,             XK_p,          spawn,                  { .com = spawn_dmenu }},
	{  MOD4,             XK_p,          spawn,                  { .com = spawn_dmenu }},

	{  MOD4,             XK_1,          change_left_desktop,    { .i = 1 }},
	{  MOD4,             XK_2,          change_left_desktop,    { .i = 2 }},
	{  MOD4,             XK_3,          change_left_desktop,    { .i = 3 }},
	{  MOD4,             XK_4,          change_left_desktop,    { .i = 4 }},
	{  MOD4,             XK_5,          change_left_desktop,    { .i = 5 }},
	{  MOD4,             XK_6,          change_left_desktop,    { .i = 6 }},
	{  MOD4,             XK_7,          change_left_desktop,    { .i = 7 }},
	{  MOD4,             XK_8,          change_left_desktop,    { .i = 8 }},
	{  MOD4,             XK_9,          change_left_desktop,    { .i = 9 }},

	{  MOD1,             XK_1,          change_right_desktop,   { .i = 1 }},
	{  MOD1,             XK_2,          change_right_desktop,   { .i = 2 }},
	{  MOD1,             XK_3,          change_right_desktop,   { .i = 3 }},
	{  MOD1,             XK_4,          change_right_desktop,   { .i = 4 }},
	{  MOD1,             XK_5,          change_right_desktop,   { .i = 5 }},
	{  MOD1,             XK_6,          change_right_desktop,   { .i = 6 }},
	{  MOD1,             XK_7,          change_right_desktop,   { .i = 7 }},
	{  MOD1,             XK_8,          change_right_desktop,   { .i = 8 }},
	{  MOD1,             XK_9,          change_right_desktop,   { .i = 9 }},

	CHANGE_VIEW(         XK_F1,                                   1)
	CHANGE_VIEW(         XK_F2,                                   2)
	CHANGE_VIEW(         XK_F3,                                   3)
	CHANGE_VIEW(         XK_F4,                                   4)
	CHANGE_VIEW(         XK_F5,                                   5)
	CHANGE_VIEW(         XK_F6,                                   6)
	CHANGE_VIEW(         XK_F7,                                   7)
	CHANGE_VIEW(         XK_F8,                                   8)
	CHANGE_VIEW(         XK_F9,                                   9)

	CLIENT_TO_DESKTOP(   XK_1,                                    1)
	CLIENT_TO_DESKTOP(   XK_2,                                    2)
	CLIENT_TO_DESKTOP(   XK_3,                                    3)
	CLIENT_TO_DESKTOP(   XK_4,                                    4)
	CLIENT_TO_DESKTOP(   XK_5,                                    5)
	CLIENT_TO_DESKTOP(   XK_6,                                    6)
	CLIENT_TO_DESKTOP(   XK_7,                                    7)
	CLIENT_TO_DESKTOP(   XK_8,                                    8)
	CLIENT_TO_DESKTOP(   XK_9,                                    9)
};


/** Mouse shortcuts **/
static Button buttons[] = {
	/* event mask        buttoon        function           argument */
	{  MOD1,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD1,             Button3,       mousemove,         { .i = RESIZE }},
	{  MOD4,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD4,             Button3,       mousemove,         { .i = RESIZE }}
};


#endif /* _CONFIG_H */


/* vim: set ts=8 sts=8 sw=8 : */