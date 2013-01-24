/* See LICENSE file for copyright and license details */

#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include "test.h"

Display *dpy;

int main(void)
{
	if (!(dpy = XOpenDisplay(NULL)))
		return EXIT_FAILURE;

	/*
	unsigned int i;
	for (i = 0; i < 10; i++) {
		test_key("Super_L", 1, 0);
		test_key("1", 1, 500);
		test_key("Super_L", 0, 0);
		test_key("1", 0, 0);

		test_key("Super_L", 1, 0);
		test_key("3", 1, 500);
		test_key("Super_L", 0, 0);
		test_key("3", 0, 0);
	}
	*/

	unsigned int i;

	for (i = 0; i < LENGTH(alphabet); i++) {
		printf("%s\t%d\n", alphabet[i], XKeysymToKeycode(dpy, XStringToKeysym(alphabet[i])));
	}
	for (i = 0; i < LENGTH(mods); i++) {
		printf("%s\t\t%d\n", mods[i], XKeysymToKeycode(dpy, XStringToKeysym(mods[i])));
	}

	XCloseDisplay(dpy);
	return 0;
}

void test_key(const char *key, int type, unsigned long delay)
{
	XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XStringToKeysym(key)), type, delay);

}


/* vim: set ts=8 sts=8 sw=8 : */
