/* see LICENSE for copyright and license */

#ifndef CONFIG_H
#define CONFIG_H

/** modifiers **/
#define MOD1            Mod1Mask    /* ALT key */

/** commands **/
static const char *terminal[] = { "urxvt", NULL };

/** keyboard shortcuts **/
static Key keys[] = {
	/* modifier          key            function           argument */
	{  MOD1,             XK_q,          quit,              { 0 }},
	{  MOD1,             XK_Return,     spawn,             { .com = terminal }}
};

/** mouse shortcuts **/
static Button buttons[] = {
	/* event mask        buttoon        function           argument */
	{  MOD1,             Button1,       mousemove,         { .i = MOVE }},
	{  MOD1,             Button3,       mousemove,         { .i = RESIZE }}
};

#endif
