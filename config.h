#ifndef CONFIG_H
#define CONFIG_H

/** modifiers **/
#define MOD1			Mod1Mask	/* ALT key */
#define DESKTOPS		4		/* number of desktops */	
#define DEFAULT_DESKTOP		0
#define NEW_IS_MASTER		True		/* new window is master */	/* UNUSED */
#define BORDER_WIDTH		5		/* window border width */
#define BORDER_OFFSET		10		/* spaces betweed windows borders */
#define MIN_WINDOW_SIZE 	50		/* minimum window size in pixels */

/** colors **/
#define FOCUS_COLOR		"#00ff22"	/* focused window border color */
#define UNFOCUS_COLOR		"#d2d2d2"	/* unfocused window border color */

/** commands **/
static const char *spawn_terminal[] = { "urxvt", NULL };
static const char *spawn_dmenu[]    = { "dmenu_run", NULL };

/** keyboard shortcuts **/
static Key keys[] = {
	/* modifier          key            function           argument */
	{  MOD1,             XK_q,          quit,              { 0 }},
	{  MOD1,             XK_j,          nextwindow,        { 0 }},
	{  MOD1,             XK_x,          killcurrent,       { 0 }},
	{  MOD1,             XK_f,          fullscreen,        { 0 }},
	{  MOD1,             XK_Tab,        previous_desktop,  { 0 }},
	{  MOD1,             XK_i,          status,            { 0 }},
	{  MOD1,             XK_Return,     spawn,             { .com = spawn_terminal }},
	{  MOD1,             XK_p,          spawn,             { .com = spawn_dmenu }},

	{  MOD1,             XK_0,          change_desktop,    { .i = 0 }},
	{  MOD1,             XK_1,          change_desktop,    { .i = 1 }},
	{  MOD1,             XK_2,          change_desktop,    { .i = 2 }},
	{  MOD1,             XK_3,          change_desktop,    { .i = 3 }}
};

/** mouse shortcuts **/
static Button buttons[] = {
	/* event mask        buttoon        function           argument */
	{  MOD1,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD1,             Button3,       mousemove,         { .i = RESIZE }}
};

#endif
