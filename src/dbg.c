/* See LICENSE file for copyright and license details */

#include <stdarg.h>
#include <stdio.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "splitwm.h"
#include "dbg.h"
#include "client.h"
#include "desktop.h"
#include "tile.h"
#include "mouse.h"
#include "config.h"
#include "view.h"


/** Functions **/

void dbg(const char *errstr, ...)
{
	if (!USE_DBG)
		return;
	va_list ap;
	va_start(ap, errstr);
	vfprintf(stdout, errstr, ap);
	va_end(ap);
}

void printstatus(void)
{
	if (!USE_PRINTSTATUS)
		return;
	unsigned int i;

	for (i = 0; i < DESKTOPS_LEFT; i++) {
		dbg("LEFT DESKTOP %d: ", i);
		Desktop *d = &views[cv_id].ld[i];
		Client *c = d->head;
		
		if (!c) {
			dbg(".\n");
			continue;
		}

		for (; c; c = c->next)
			if (c == d->curr && d->curr) {
				dbg("M ");
			} else {
				dbg("C ");
			}
		dbg("\n");
	}

	for (i = 0; i < DESKTOPS_RIGHT; i++) {
		dbg("RIGHT DESKTOP %d: ", i);
		Desktop *d = &views[cv_id].rd[i];
		Client *c = d->head;
		
		if (!c) {
			dbg(".\n");
			continue;
		}

		for (; c; c = c->next)
			if (c == d->curr && d->curr) {
				dbg("M ");
			} else {
				dbg("C ");
			}
		dbg("\n");
	}
}


/* vim: set ts=8 sts=8 sw=8 : */
