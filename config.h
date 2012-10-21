/* see LICENSE for copyright and license */

#ifndef CONFIG_H
#define CONFIG_H

/** modifiers **/
#define MOD1            Mod1Mask	/* ALT key */
#define DESKTOPS	5		/* number of desktops */	
#define NEW_IS_MASTER	True		/* new window is master */

/** commands **/
static const char *spawn_terminal[] = { "urxvt", NULL };
static const char *spawn_dmenu[]    = { "dmenu_run", NULL };

/** keyboard shortcuts **/
static Key keys[] = {
	/* modifier          key            function           argument */
	{  MOD1,             XK_q,          quit,              { 0 }},
	{  MOD1,             XK_Return,     spawn,             { .com = spawn_terminal }},
	{  MOD1,             XK_p,          spawn,             { .com = spawn_dmenu }}

};

/** mouse shortcuts **/
static Button buttons[] = {
	/* event mask        buttoon        function           argument */
	{  MOD1,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD1,             Button3,       mousemove,         { .i = RESIZE }}
};

#endif
