/* See LICENSE file for copyright and license details */

#ifndef _EVENT_H
#define _EVENT_H


/* Functions */

extern void buttonpress(XEvent *e);
extern void configurenotify(XEvent *e);
extern void configurerequest(XEvent *e);
extern void destroynotify(XEvent *e);
extern void enternotify(XEvent *e);
extern void expose(XEvent *e);
extern void keypress(XEvent *e);
extern void maprequest(XEvent *e);

/* Event handlers */
extern void (*events[LASTEvent]) (XEvent *e);


#endif /* _EVENT_H */


/* vim: set expandtab ts=8 sts=8 sw=8 : */
