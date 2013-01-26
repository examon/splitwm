/* See LICENSE file for copyright and license details */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>


Display *dpy;

void test_key(const char *key, int type, unsigned long delay);

int main(int argc, char *argv[])
{
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "%s error: cannot open display!\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (argc == 2 && !strcmp(argv[1], "-h")) {
		printf("%s [-h] keycode key_type delay\n", argv[0]);
		return EXIT_FAILURE;
	} else if (argc != 4) {
		fprintf(stderr, "%s error: invalid arguments, use %s -h for help\n", argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	test_key(argv[1], atoi(argv[2]), (unsigned long)atoi(argv[3]));
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}

void test_key(const char *key, int type, unsigned long delay)
{
	XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XStringToKeysym(key)), type, delay);

}


/* vim: set ts=8 sts=8 sw=8 : */
