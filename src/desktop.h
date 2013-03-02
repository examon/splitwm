/* See LICENSE file for copyright and license details */

#ifndef _DESKTOP_H
#define _DESKTOP_H

#include "client.h"


/**
 * Structure for storing info about desktop.
 * Each desktop got its own Desktop structure.
 */
typedef struct {

	/**
	 * First client in the list.
	 */
	Client *head;
	
	/**
	 * Currently active client in the list.
	 */
	Client *curr;

	/**
	 * Master window size (in pixels).
	 */
	int master_size;

	/**
	 * Layout type for the Desktop.
	 */
	int layout;

	/**
	 * Tiling of floating mode currently active for the Desktop.
	 */
	int tile_or_float;
} Desktop;

/**
 * Gets current active desktop.
 *
 * Return:
 *  - NULL: when there is no current active desktop
 *  - Desktop *: pointer to Desktop structure of current active desktop
 */
extern Desktop *get_current_desktop(void);

/**
 * Changes currently active left desktop to the targeted desktop.
 *
 * Returns without any effect when:
 *  - currently active desktop is the same as the targeted desktop
 *  - targeted desktop number is higher than maximum allowed desktop number
 */
extern void change_left_desktop(const Arg *arg);

/**
 * Changes currently active right desktop to the targeted desktop.
 *
 * Returns without any effect when:
 *  - currently active desktop is the same as the targeted desktop
 *  - targeted desktop number is higher than maximum allowed desktop number
 */
extern void change_right_desktop(const Arg *arg);

/**
 * Switches to the previous active desktop. 
 */
extern void previous_desktop(const Arg *arg);

/**
 * Increases master window size.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 *  - there is no space to increase master window
 */
extern void master_size_increase(const Arg *arg);

/**
 * Decreases master window size.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 *  - there is no space to decrease master window
 */
extern void master_size_decrease(const Arg *arg);

/**
 * Switch currently active desktop to the floating mode 
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void toggle_float(const Arg *arg);


#endif /* _DESKTOP_H */

/* vim: set ts=8 sts=8 sw=8 : */
