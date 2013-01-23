/* See LICENSE file for copyright and license details */

#ifndef _VIEW_H
#define _VIEW_H

#include "desktop.h"


/** Structures **/

typedef struct {
	Desktop ld[10];		/* left desktops */
	Desktop rd[10];		/* right desktops */
	int curr_left_id;
	int prev_left_id;
	int curr_right_id;
	int prev_right_id;
	int curr_desk;		/* current view, LEFT/RIGHT */
	float split_width_x;
	float split_height_y;
	Bool both_views_activated;
	Bool left_view_activated;
	Bool right_view_activated;
} View;


/** Variables **/

extern int cv_id;
extern int pv_id;
extern View views[13];


/* Functions **/

extern void activate_left_view(const Arg *arg);
extern void activate_right_view(const Arg *arg);
extern void activate_both_views(const Arg *arg);
extern void client_to_view(const Arg *arg);
extern void change_view(const Arg *arg);
extern void nextview(const Arg *arg);
extern void previous_view(const Arg *arg);
extern void separator_increase(const Arg *arg);
extern void separator_decrease(const Arg *arg);
extern void update_geometry(void);



#endif /* _VIEW_H */


/* vim: set ts=8 sts=8 sw=8 : */
