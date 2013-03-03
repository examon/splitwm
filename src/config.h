/* See LICENSE file for copyright and license details */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "view.h"


/******\
* Font *
\******/

/**
 * Default font for the window manager, bar, etc.
 */
#define  FONT  "-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-*"


/*****\
* Bar *
\*****/ 

/**
 * Default bar position.
 *
 * Can be changed to:
 *  - TOP
 *  - BOTTOM
 *  - NONE
 */
#define  BAR_POSITION  TOP

/**
 * Spaces between bar tags.
 */
#define  CHAR_SPACE  5

/**
 * Grid tile layout string tag shown in the bar.
 */
#define  GRID_TILE_TAG  "[grid]"

/**
 * Master tile layout string tag shown in the bar.
 */
#define  MASTER_TILE_TAG  "[master]"

/**
 * Float layout string tag shown in the bar.
 */
#define  FLOAT_TAG  "[float]"


/**************\
* External Bar *
\**************/

/**
 * Default position for the external bar.
 *
 * Can be changed to:
 *  - TOP
 *  - BOTTOM
 *  - NONE
 */
#define  EXTERNAL_BAR_POSITION  NONE

/**
 * External bar height (in pixels).
 */
#define  EXTERNAL_BAR_HEIGHT  20


/*******\
* Views *
\*******/

/**
 * Views tag strings.
 * Can be changed but array must contain max 9 items!
 */
static const char *tags_views[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/**
 * Default view number.
 */
#define  DEFAULT_VIEW  1


/**********\
* Desktops *
\**********/

/**
 * Left desktop tag strings.
 * Can be changed but array must contain max 9 items!
 */
static const char  *tags_left[]  = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/**
 * Right desktop tag strings.
 * Can be changed but array must contain max 9 items!
 */
static const char  *tags_right[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/**
 * Default left desktop number.
 */
#define  DEFAULT_LEFT_DESKTOP  1

/**
 * Default right desktop number.
 */
#define  DEFAULT_RIGHT_DESKTOP  1

/**
 * Size of the master window in the "master" tile layout (in pixels).
 *
 * If MASTER_SIZE == 0, then master size is equal to the half of the screen width.
 */
#define  MASTER_SIZE  0

/**
 * Increase distance (in pixels) when changing master window size.
 */
#define  MASTER_SIZE_INC  20

/**
 * Decrease distance (in pixels) when changing master window size.
 */
#define  MASTER_SIZE_DEC  20


/********\
* Window *
\********/

/**
 * When True, focus will be automatically given to window when mouse enters it.
 */
#define  FOLLOW_MOUSE_FOCUS  True

/**
 * Window border width (in pixels).
 */
#define  BORDER_WIDTH  4

/**
 * Window border offset (in pixels).
 *
 * (spaces between windows borders)
 */
#define  BORDER_OFFSET  4


/************************\
* Split Screen Separator *
\************************/

/**
 * When True, split screen separator will be visible.
 */
#define  SHOW_SEPARATOR  True

/**
 * Split screen separator width (in pixels).
 */
#define  SEPARATOR_WIDTH  4

/**
 * Split screen separator increase distance (in pixels).
 */
#define  SEPARATOR_INC  50

/**
 * Split screen separator decrease distance (in pixels).
 */
#define  SEPARATOR_DEC  50


/****************\
* Default Colors *
\****************/

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


/**********\
* Commands *
\**********/

static const char  *spawn_terminal[] = { "urxvt", NULL };
static const char  *spawn_dmenu[]    = { "dmenu_run", "-fn", FONT, "-nb", BAR_BG_COLOR, "-nf", VIEW_TAG_NORMAL_FG, "-sb", BAR_BG_COLOR, "-sf", FOCUS_COLOR, NULL };


/***********\
* Modifiers *
\***********/

/**
 * Alt key
 */
#define  MOD1  Mod1Mask

/**
 * Super/Win key
 */
#define  MOD4  Mod4Mask

/**
 * Shift key
 */
#define  SHIFT  ShiftMask


/********\
* Cursor *
\********/

/**
 * Default cursor
 */
#define  CURSOR  XC_left_ptr


/*******\
* Debug *
\*******/

/**
 * If True, dbg() function will be enabled.
 */
#define  USE_DBG  True

/**
 * If True, printstatus() function will be enabled.
 */
#define  USE_PRINTSTATUS  True


/***************************\
* Anti-multiple Paste Macro *
\***************************/

#define CLIENT_TO_DESKTOP(K, N) \
	{  MOD1|SHIFT,       K,             client_to_desktop,      { .i = N }}, \
	{  MOD4|SHIFT,       K,             client_to_desktop,      { .i = N }},

#define CHANGE_VIEW(K, N) \
	{  MOD1,             K,             change_view,            { .i = N }}, \
	{  MOD4,             K,             change_view,            { .i = N }},


/********************\
* Keyboard Shortcuts *
\********************/

/**
 * Array containing all keyboard shortcuts that splitwm supports.
 */
static Key keys[] = {
	/* modifier          key            function                argument */

	{  MOD1|SHIFT,       XK_g,          tile_layout_grid,       { 0 }},
	{  MOD1|SHIFT,       XK_m,          tile_layout_master,     { 0 }},
	{  MOD1,             XK_t,          tile_current,           { 0 }},

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


/*****************\
* Mouse Shortcuts *
\*****************/

/**
 * Array containing all mouse events that splitwm supports.
 */
static Button buttons[] = {
	/* event mask        button        function           argument */

	{  MOD1,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD1,             Button3,       mousemove,         { .i = RESIZE }},
	{  MOD4,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD4,             Button3,       mousemove,         { .i = RESIZE }}
};


#endif /* _CONFIG_H */

/* vim: set ts=8 sts=8 sw=8 : */
