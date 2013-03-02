/* See LICENSE file for copyright and license details */

#ifndef _DBG_H
#define _DBG_H


/**
 * Prints passed argument string to the stdout as a debug message.
 */
extern void dbg(const char *errstr, ...);

/**
 * Prints status of the left & right resktop client list.
 */
extern void printstatus(void);


#endif /* _DBG_H */

/* vim: set ts=8 sts=8 sw=8 : */
