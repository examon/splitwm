#ifndef CONFIG_H
#define CONFIG_H

/** modifiers **/
#define MOD1			Mod1Mask	/* ALT key */
#define MOD4			Mod4Mask	/* Super/Win key */
#define DESKTOPS		10		/* number of desktops */	
#define DEFAULT_LEFT_DESKTOP	1		/* defualt left desktop */
#define DEFAULT_RIGHT_DESKTOP	1		/* defualt right desktop */
#define VIEWS			10		/* number of views */
#define DEFAULT_VIEW		1		/* default view */
#define DEFAULT_FOCUSED_VIEW	LEFT		/* LEFT/RIGHT */
#define BORDER_WIDTH		5		/* window border width */
#define BORDER_OFFSET		10		/* spaces betweed windows borders */
#define MIN_WINDOW_SIZE 	50		/* minimum window size in pixels */
#define W_SPLIT_COEFFICIENT	2		/* screen width split coefficient, 2 == 1/2 screen width */
#define H_SPLIT_COEFFICIENT	1        	/* screen height split coefficient, 2 == 1/2 screen heigth */
#define SPLIT_SEPARATOR_WIDTH	0		/* width of the split seperator */

/** colors **/
#define FOCUS_COLOR		"#ff0000"	/* focused window border color (any desktop) */
#define LEFT_UNFOCUS_COLOR	"#8ab3d5"	/* unfocused window border color (left desktop) */
#define RIGHT_UNFOCUS_COLOR	"#76d27b"	/* unfocused window border color (right desktop) */

/** commands **/
static const char *spawn_terminal[] = { "urxvt", NULL };
static const char *spawn_dmenu[]    = { "dmenu_run", NULL };

/** keyboard shortcuts **/
static Key keys[] = {
	/* modifier          key            function                argument */
	{  MOD1,             XK_q,          quit,                   { 0 }},
	{  MOD1,             XK_j,          nextwindow,             { 0 }},
	{  MOD4,             XK_j,          nextview,               { 0 }},
	{  MOD1,             XK_x,          killcurrent,            { 0 }},
	{  MOD1,             XK_f,          fullscreen,             { 0 }},
	{  MOD1,             XK_space,      maximize_current,       { 0 }},
	{  MOD1,             XK_Tab,        previous_desktop,       { 0 }},
	{  MOD1,             XK_i,          status,                 { 0 }},
	{  MOD1,             XK_Return,     spawn,                  { .com = spawn_terminal }},
	{  MOD1,             XK_p,          spawn,                  { .com = spawn_dmenu }},

	{  MOD4,             XK_0,          change_left_desktop,    { .i = 0 }},
	{  MOD4,             XK_1,          change_left_desktop,    { .i = 1 }},
	{  MOD4,             XK_2,          change_left_desktop,    { .i = 2 }},
	{  MOD4,             XK_3,          change_left_desktop,    { .i = 3 }},
	{  MOD4,             XK_4,          change_left_desktop,    { .i = 4 }},
	{  MOD4,             XK_5,          change_left_desktop,    { .i = 5 }},
	{  MOD4,             XK_6,          change_left_desktop,    { .i = 6 }},
	{  MOD4,             XK_7,          change_left_desktop,    { .i = 7 }},
	{  MOD4,             XK_8,          change_left_desktop,    { .i = 8 }},
	{  MOD4,             XK_9,          change_left_desktop,    { .i = 9 }},

	{  MOD1,             XK_0,          change_right_desktop,   { .i = 0 }},
	{  MOD1,             XK_1,          change_right_desktop,   { .i = 1 }},
	{  MOD1,             XK_2,          change_right_desktop,   { .i = 2 }},
	{  MOD1,             XK_3,          change_right_desktop,   { .i = 3 }},
	{  MOD1,             XK_4,          change_right_desktop,   { .i = 4 }},
	{  MOD1,             XK_5,          change_right_desktop,   { .i = 5 }},
	{  MOD1,             XK_6,          change_right_desktop,   { .i = 6 }},
	{  MOD1,             XK_7,          change_right_desktop,   { .i = 7 }},
	{  MOD1,             XK_8,          change_right_desktop,   { .i = 8 }},
	{  MOD1,             XK_9,          change_right_desktop,   { .i = 9 }}

};

/** mouse shortcuts **/
static Button buttons[] = {
	/* event mask        buttoon        function           argument */
	{  MOD1,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD1,             Button3,       mousemove,         { .i = RESIZE }}
};

#endif
