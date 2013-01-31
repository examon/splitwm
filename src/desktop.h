/* See LICENSE file for copyright and license details */

#ifndef _DESKTOP_H
#define _DESKTOP_H

#include "client.h"


/** Structures **/

typedef struct {
	Client *head;
	Client *curr;
	int master_size;
	int layout;
	int tile_or_float;
} Desktop;


/** Functions **/

extern Desktop *get_current_desktop(void);
extern void change_left_desktop(const Arg *arg);
extern void change_right_desktop(const Arg *arg);
extern void previous_desktop(const Arg *arg);
extern void master_size_increase(const Arg *arg);
extern void master_size_decrease(const Arg *arg);
extern void toggle_float(const Arg *arg);


#endif /* _DESKTOP_H */


/* vim: set ts=8 sts=8 sw=8 : */
