/* See LICENSE file for copyright and license details */

#ifndef CONFIG_H
#define CONFIG_H

/** Font **/
static const char font[] = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-*";

/** Bar **/
static const Bool show_bar = True;		/* False to hide bar */
static const char *tile_tag = "[T]";		/* Tile tag shown in bar */
static const char *float_tag = "[F]";		/* Float tag shown in bar */
static const unsigned int char_space = 5; 	/* Spaces between bar tags */

/** Views **/
static const char *tags_views[] = { "1", "2", "3", "4", "5" };	/* max 9 */
static const unsigned int default_view = 1;	/* default view */

/** Desktops **/
static const char *tags_left[]  = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };	/* max 9 */
static const char *tags_right[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };	/* max 9 */
static const unsigned int master_size = 0;	/* master window size, if 0 then master_size = screen_width / 2 */
static const unsigned int master_size_inc = 20; /* master_size increase distance */
static const unsigned int master_size_dec = 20; /* master_size decrease distance */
static const unsigned int default_left_desktop  = 1;
static const unsigned int default_right_desktop = 1;

/** Window **/
static const Bool follow_mouse_focus      = False;
static const unsigned int border_width    = 2;		/* window border width */
static const unsigned int border_offset   = 2;		/* spaces betweed windows borders */

/** Separator **/
static const unsigned int separator_width = 10;		/* width of the split seperator */
static const unsigned int separator_inc   = 20;		/* separator increase distance */
static const unsigned int separator_dec   = 20;		/* separator decrease distance */

/** Colors **/
static const char focus_color[]           = "#d87a16";
static const char left_unfocus_color[]    = "#005577";
static const char right_unfocus_color[]   = "#288428";
static const char bar_bg_color[]          = "#111111";
static const char bar_title_color[]       = "#bbbbbb";

static const char left_tag_focus_bg[]     = "#005577";
static const char left_tag_focus_fg[]     = "#eeeeee";
static const char left_tag_normal_bg[]    = "#111111";
static const char left_tag_normal_fg[]    = "#bbbbbb";
static const char left_tag_occupied_bg[]  = "#bbbbbb";
static const char left_tag_occupied_fg[]  = "#bbbbbb";

static const char right_tag_focus_bg[]    = "#288428";
static const char right_tag_focus_fg[]    = "#eeeeee";
static const char right_tag_normal_bg[]   = "#111111";
static const char right_tag_normal_fg[]   = "#bbbbbb";
static const char right_tag_occupied_bg[] = "#bbbbbb";
static const char right_tag_occupied_fg[] = "#bbbbbb";

static const char view_tag_focus_bg[]     = "#686868";
static const char view_tag_focus_fg[]     = "#eeeeee";
static const char view_tag_normal_bg[]    = "#111111";
static const char view_tag_normal_fg[]    = "#bbbbbb";
static const char view_tag_occupied_bg[]  = "#bbbbbb";
static const char view_tag_occupied_fg[]  = "#bbbbbb";

/** Commands **/
static const char *spawn_terminal[] = { "urxvt", NULL };
static const char *spawn_dmenu[]    = { "dmenu_run", "-fn", font, "-nb", bar_bg_color, "-nf", view_tag_normal_fg, "-sb", focus_color, "-sf", bar_bg_color, NULL };

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

#endif
