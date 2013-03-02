/* See LICENSE file for copyright and license details */

#ifndef _VIEW_H
#define _VIEW_H

#include "desktop.h"


/**
 * View structure, storing all information about each view.
 */
typedef struct {
	
	/**
	 * Left desktops.
	 */
	Desktop ld[10];

	/**
	 * Right desktops.
	 */
	Desktop rd[10];

	/**
	 * Currently active left desktop ID.
	 */
	int curr_left_id;

	/**
	 * Previously active left desktop ID.
	 */
	int prev_left_id;

	/**
	 * Currently active right desktop ID.
	 */
	int curr_right_id;

	/**
	 * Previously active right desktop ID.
	 */
	int prev_right_id;

	/**
	 * Currently active view side (LEFT or RIGHT).
	 */
	int curr_desk;

	/**
	 * Distance in the x-axis where the screen is split (in pixels).
	 */
	float split_width_x;

	/**
	 * Distance in the y-axis where the screen is split (in pixels).
	 */
	float split_height_y;

	/**
	 * True when both view sides are activated at the same time.
	 */
	Bool both_views_activated;

	/**
	 * True when only LEFT view side is activated.
	 */
	Bool left_view_activated;

	/**
	 * True when only RIGHT view side is activated.
	 */
	Bool right_view_activated;
} View;


/**
 * Currently active view ID number.
 */
extern int cv_id;

/**
 * Previously active view ID number.
 */
extern int pv_id;

/**
 * Array of all views.
 */
extern View views[13];


/**
 * Activates only LEFT view side.
 *
 * Returns without any effect when:
 *  - both views are not activated before activate_left_view() has been called
 */
extern void activate_left_view(const Arg *arg);

/**
 * Activates only RIGHT view side.
 *
 * Returns without any effect when:
 *  - both views are not activated before activate_right_view() has been called
 */
extern void activate_right_view(const Arg *arg);

/**
 * Activates both views.
 *
 * Returns without any effect when:
 *  - both views are already activated
 */
extern void activate_both_views(const Arg *arg);

/**
 * Sends currently active client to the opposite view (e.g. client is in the LEFT
 * side so client_to_view() sends it to the RIGHT side.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 *  - current client has not been found
 *  - LEFT/RIGHT view is activated
 */
extern void client_to_view(const Arg *arg);

/**
 * Changes currently active view to the targeted view (arg->i).
 *
 * Returns without any effect when:
 *  - targeted view ID is the same as the currently active view ID
 */
extern void change_view(const Arg *arg);

/**
 * Switches focus between LEFT & RIGHT view side.
 *
 * Returns without any effect when:
 *  - both views are not activated
 */
extern void nextview(const Arg *arg);

/**
 * Switches to the previous activated view.
 */
extern void previous_view(const Arg *arg);

/**
 * Increases split screen separator position in the x-axis.
 *
 * Returns without any effect when:
 *  - both views are not activated
 */
extern void separator_increase(const Arg *arg);

/**
 * Decreases split screen separator position in the x-axis.
 *
 * Returns without any effect when:
 *  - both views are not activated
 */
extern void separator_decrease(const Arg *arg);

/**
 * Updates all splitwm elements (separator, bar, etc.) when resolution changes.
 */
extern void update_geometry(void);


#endif /* _VIEW_H */

/* vim: set ts=8 sts=8 sw=8 : */
