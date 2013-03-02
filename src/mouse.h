/* See LICENSE file for copyright and license details */

#ifndef _MOUSE_H
#define _MOUSE_H


/**
 * Handles all mouse moves (resizing window, moving window, etc.).
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 *  - current window does not exist
 *  - unable to get pointer
 */
extern void mousemove(const Arg *arg);


#endif /* _MOUSE_H */

/* vim: set ts=8 sts=8 sw=8 : */
