/* See LICENSE file for copyright and license details */

#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "splitwm.h"
#include "view.h"
#include "event.h"
#include "draw.h"
#include "dbg.h"
#include "client.h"
#include "desktop.h"
#include "tile.h"
#include "mouse.h"
#include "config.h"


/** Event handlers **/

void (*events[LASTEvent]) (XEvent *e) = {
	[ButtonPress]      = buttonpress,
	[ConfigureNotify]  = configurenotify,
	[ConfigureRequest] = configurerequest,
	[DestroyNotify]    = destroynotify,
	[EnterNotify]      = enternotify,
	[Expose]           = expose,
	[KeyPress]         = keypress,
	[MapRequest]       = maprequest
};


/** Functions **/

void buttonpress(XEvent *e)
{
	dbg("buttonpress(): IN\n");
	unsigned int i;
	Bool client_found = False;
	Desktop *d = NULL;
	Client *c = NULL;
	XButtonEvent ev = e->xbutton;

	if (e->xbutton.state == MOD1 || e->xbutton.state == MOD4) {
		if (!(d = get_current_desktop()))
			return;
		for (c = d->head; c; c = c->next) {
			if (c->win == ev.subwindow) {
				d->curr = c;
				client_found = True;
				focuscurrent();
				XRaiseWindow(dpy, c->win);
			}
		}
		if (!client_found && ev.subwindow != root) {
			nextview(0);
			if (!(d = get_current_desktop()))
				return;
			for (c = d->head; c; c = c->next) {
				if (c->win == ev.subwindow) {
					d->curr = c;
					focuscurrent();
					XRaiseWindow(dpy, c->win);
					break;
				}
			}
		}
	}
	draw();
	
	for (i = 0; i < LENGTH(buttons); i++) {
		if ((buttons[i].mask == e->xbutton.state) &&
		    (buttons[i].button == e->xbutton.button) && buttons[i].func)
			buttons[i].func(&(buttons[i].arg));
	}
	dbg("buttonpress(): OUT\n");
}

void configurenotify(XEvent *e)
{
	dbg("configurenotify(): IN\n");
	/* update screen width/height on root geom change */
	XConfigureEvent *ev = &e->xconfigure;
	if (ev->window == root) {
		dbg("configurenotify(): \tROOT CHANGED\n");
		sw = ev->width;
		sh = ev->height;
		update_geometry();
		create_bar();
		map_bar();
		draw();
		tile_current(0);
	}
	dbg("configurenotify(): OUT\n");
}

void configurerequest(XEvent *e)
{
	dbg("cofigurerequest(): IN\n");
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;
	wc.x = ev->x;
	wc.y = ev->y;
	wc.width = ev->width;
	wc.height = ev->height;
	wc.border_width = ev->border_width;
	wc.sibling = ev->above;
	wc.stack_mode = ev->detail;
	XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	XSync(dpy, False);
	Desktop *d = NULL;
	if (!(d = get_current_desktop()))
		return;
	d->layout = FLOAT;
	tile(d);
	dbg("cofigurerequest(): OUT\n");
}

void destroynotify(XEvent *e)
{
	dbg("destroynotify(): IN\n");
	unsigned int i = 0;
	Client *c = NULL;
	Desktop *d = NULL;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next) {
		if (ev->window == c->win)
			i++;
	}
	if (i == 0)
		return;
	if (ev->window != root)
		removewindow(ev->window);
	tile(d);
	focuscurrent();
	draw();
	dbg("destroynotify(): OUT\n");
}

void keypress(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ke;
	ke = &e->xkey;
	keysym = XkbKeycodeToKeysym(dpy, (KeyCode)ke->keycode, 0, 0);

	for (i = 0; i < LENGTH(keys); i++) {
		if (keysym == keys[i].keysym && keys[i].mod == ke->state && keys[i].func)
			keys[i].func(&(keys[i].arg));
	}
}

void maprequest(XEvent *e)
{
	dbg("maprequest(): IN\n");
	XMapRequestEvent *ev = &e->xmaprequest;
	XWindowAttributes wa;
	Desktop *d = NULL;
	Client *c = NULL;

	if (!(d = get_current_desktop()))
		return;
	for (c = d->head; c; c = c->next) {
		if (ev->window == c->win) {
			XMapWindow(dpy, ev->window);
			return;
		}
	}
	addwindow(ev->window);
	XMapWindow(dpy, ev->window);
	if (views[cv_id].both_views_activated)
		d->layout = TILE;
	tile(d);
	focuscurrent();
	XGetWindowAttributes(dpy, d->curr->win, &wa);
	XWarpPointer(dpy, root, ev->window, 0, 0, 0, 0, --wa.width/2, --wa.height/2);
	draw();
	dbg("maprequest(): OUT\n");
}

void enternotify(XEvent *e)
{
	dbg("enternotify(): IN\n");
	if (!FOLLOW_MOUSE_FOCUS)
		return;
	Client *c = NULL;
	Desktop *d = NULL;
	Bool client_found = False;
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	if (!(d = get_current_desktop()))
		return;

	for (c = d->head; c; c = c->next) {
		if (c->win == ev->window) {
			dbg("enternotify(): FOCUSCURRENT()\n");
			client_found = True;
			d->curr = c;
			focuscurrent();
			return;
		}
	}

	if (!client_found && ev->window != root) {
		dbg("enternotify(): NEXTVIEW()\n");
		nextview(0);
		if (!(d = get_current_desktop()))
			return;
		for (c = d->head; c; c = c->next) {
			if (c->win == ev->window) {
				d->curr = c;
				focuscurrent();
				return;
			}
		}
	}
	dbg("enternotify(): OUT\n");
}

void expose(XEvent *e)
{
	dbg("expose(): IN\n");
	if (BAR_POSITION == NONE)
		return;
	XExposeEvent *ex = &e->xexpose;
	if (ex->window == bar || (ex->window != bar && ex->window != root)) {
		dbg("expose(): \tBAR\n");
		XCopyArea(dpy, bar_buffer, bar, gc, 0, 0, sw, bar_height, 0, 0);
		XRaiseWindow(dpy, bar);
		XFlush(dpy);
	}
	dbg("expose(): OUT\n");
}


/* vim: set ts=8 sts=8 sw=8 : */
