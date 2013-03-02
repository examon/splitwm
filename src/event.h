/* See LICENSE file for copyright and license details */

#ifndef _EVENT_H
#define _EVENT_H


/**
 * Generated when user uses mouse buttons.
 * Handles all available mouse buttons.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void buttonpress(XEvent *e);

/**
 * Updates screen resolution & updates graphics elements (bar, separator, etc.).
 */
extern void configurenotify(XEvent *e);

/**
 * Configures window's size.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 */
extern void configurerequest(XEvent *e);

/**
 * Generated whenever a window is destroyed.
 *
 * Returns without any effect when:
 *  - get_current_desktop() returned NULL
 *  - when there is no window to delete
 */
extern void destroynotify(XEvent *e);

/**
 * Generated whenever mouse pointer enters window's borders.
 *
 * Returns without any effect when:
 *  - entered window was root
 *  - get_current_desktop() returned NULL
 */
extern void enternotify(XEvent *e);

/**
 * Generated whenever server reports a GraphicsExpose event.
 */
extern void expose(XEvent *e);

/**
 * Generated whenever user presses any key.
 * Handles all available key shortcuts.
 */
extern void keypress(XEvent *e);

/**
 * Generated whenever window wants to display itself.
 *
 * Returns without any effect when:
 *  - get_current_desktop()
 */
extern void maprequest(XEvent *e);

/**
 * Handler used for mapping appropriate event to event function.
 */
extern void (*events[LASTEvent]) (XEvent *e);


#endif /* _EVENT_H */

/* vim: set ts=8 sts=8 sw=8 : */
