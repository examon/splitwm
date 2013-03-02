/* See LICENSE file for copyright and license details */

#ifndef _GRAB_H
#define _GRAB_H


/**
 * Grabs all defined keys & shortcuts defined in the config.h.
 */
extern void grabkeys(void);

/**
 * Grabs all mouse buttons & shortcuts defined in the config.h.
 */
extern void grabbuttons(void);

/**
 * Grabs all colors defined in the config.h.
 */
extern unsigned long grabcolor(const char *color);


#endif /* _GRAB_H */

/* vim: set ts=8 sts=8 sw=8 : */
