/* See LICENSE file for copyright and license details */

#ifndef CONFIG_H
#define CONFIG_H

/** Font **/
static const char font[] = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-*-*";

/** Modifiers **/
#define MOD1		Mod1Mask	/* ALT key */
#define MOD4		Mod4Mask	/* Super/Win key */
#define SHIFT		ShiftMask	/* Shift key */

/** Cursor **/
#define CURSOR		XC_left_ptr	/* default cursor */

/** Settings **/
#define DESKTOPS		5		/* number of desktops */	
#define DEFAULT_LEFT_DESKTOP	1		/* defualt left desktop */
#define DEFAULT_RIGHT_DESKTOP	1		/* defualt right desktop */
#define VIEWS			10		/* number of views */
#define DEFAULT_VIEW		1		/* default view */
#define DEFAULT_FOCUSED_VIEW	LEFT		/* LEFT/RIGHT */
#define BORDER_WIDTH		2		/* window border width */
#define BORDER_OFFSET		0		/* spaces betweed windows borders */
#define MASTER_SIZE		0		/* master window size, if 0 then MASTER_SIZE = screen_width / 2 */
#define MIN_WINDOW_SIZE 	10		/* minimum window size in pixels */
#define SPLIT_SEPARATOR_WIDTH	2		/* width of the split seperator */
#define DEFAULT_WIDTH_SPLIT_COEFFICIENT	  2	/* screen width split coefficient, 2 == 1/2 screen width */
#define DEFAULT_HEIGHT_SPLIT_COEFFICIENT  1    	/* screen height split coefficient, 2 == 1/2 screen heigth */
#define SEPARATOR_INCREASE	20		/* separator increase motion distance */
#define SEPARATOR_DECREASE	20		/* separator decrease motion distance */
#define VIEWS_ACTIVATED		False		/* True if you want to activate views at start */
#define SHOW_BAR		True		/* True if you want to show bar */

/** Colors **/
static const char bar_bg_color[]       = "#222222";	/* status bar background color */
static const char bar_fg_color[]       = "#ffffff";	/* status bar foreground color */
static const char bar_fg_focus_color[] = "#ff5555";	/* status bar foreground focus color */
#define FOCUS_COLOR		"#ff5555"	/* focused window border color (any desktop) */
#define LEFT_UNFOCUS_COLOR	"#55ff55"	/* unfocused window border color (left desktop) */
#define RIGHT_UNFOCUS_COLOR	"#ffff55"	/* unfocused window border color (right desktop) */
#define SEPARATOR_COLOR		"#ff0000"	/* screen separator color */

/** Commands **/
static const char *spawn_terminal[] = { "urxvt", NULL };
static const char *spawn_dmenu[]    = { "dmenu_run", NULL };

/** Anti-multiple paste macro **/
#define CLIENT_TO_DESKTOP(K, N) \
	{  MOD4|SHIFT,       K,             client_to_desktop,      { .i = N }}, \
	{  MOD1|SHIFT,       K,             client_to_desktop,      { .i = N }},

/** Keyboard shortcuts **/
static Key keys[] = {
	/* modifier          key            function                argument */
	{  MOD1,             XK_q,          quit,                   { 0 }},
	{  MOD1,             XK_j,          nextwindow,             { 0 }},
	{  MOD4,             XK_j,          nextview,               { 0 }},
	{  MOD4|SHIFT,       XK_j,          client_to_view,         { 0 }},
	{  MOD4,             XK_v,          activate_left_view,     { 0 }},
	{  MOD1,             XK_v,          activate_right_view,    { 0 }},
	{  MOD1,             XK_b,          activate_both_views,    { 0 }},
	{  MOD4,             XK_b,          activate_both_views,    { 0 }},
	{  MOD1,             XK_x,          kill_client,            { 0 }},
	{  MOD1,             XK_f,          fullscreen,             { 0 }},
	{  MOD1,             XK_t,          tile_current,           { 0 }},
	{  MOD1,             XK_space,      maximize_current,       { 0 }},
	{  MOD1,             XK_Tab,        previous_desktop,       { 0 }},
	{  MOD4,             XK_Tab,        previous_desktop,       { 0 }},
	{  MOD1,             XK_i,          status,                 { 0 }},
	{  MOD1|SHIFT,	     XK_h,	    separator_decrease,	    { 0 }},
	{  MOD1|SHIFT,	     XK_l,	    separator_increase,     { 0 }},
	{  MOD1,             XK_Return,     spawn,                  { .com = spawn_terminal }},
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
	{  MOD1,             Button3,       mousemove,         { .i = RESIZE }}
};

#endif
