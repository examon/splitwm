/* See LICENSE file for copyright and license details */

#ifndef _TILE_H
#define _TILE_H

#include "desktop.h"


/**
 * Tile desktop passed in as an argument (uses desktop's defined tiling layout).
 *
 * Returns without any effect when:
 *  - current desktop is in the floating mode
 */
extern void tile(Desktop *d);

/**
 * Tile currently active desktop (using master layout).
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void tile_layout_master(const Arg *arg);

/**
 * Tile currently active desktop (using grid layout).
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void tile_layout_grid(const Arg *arg);

/**
 * Tile currentyl active desktop (uses desktop's define tiling layout).
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void tile_current(const Arg *arg);


#endif /* _TILE_H */

/* vim: set ts=8 sts=8 sw=8 : */
