/* See LICENSE file for copyright and license details */

#ifndef _TILE_H
#define _TILE_H

#include "desktop.h"


/** Functions **/

extern void tile(Desktop *d);
extern void tile_layout_master(const Arg *arg);
extern void tile_layout_grid(const Arg *arg);
extern void tile_current(const Arg *arg);


#endif /* _TILE_H */


/* vim: set ts=8 sts=8 sw=8 : */
