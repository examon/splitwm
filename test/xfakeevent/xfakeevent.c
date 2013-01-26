/*
 * xfakeevent - send fake events to the X
 * Copyright (C) 2013  Tomas Meszaros [exo at tty dot com]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>


Display *dpy;

void fake_button(unsigned int button, int is_press, unsigned long delay);
void fake_motion(int x, int y, unsigned long delay);
void fake_key(const char *key, int is_press, unsigned long delay);


int main(int argc, char *argv[])
{
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "%s error: cannot open display!\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
		printf("Usage: %s [options]\n\n", argv[0]);
		printf("Options:\n"
		       " -h [--help]\n"
		       " -k [--key] key pressed delay\n"
		       " -m [--motion] x y delay\n"
		       " -b [--button] button pressed delay\n"
		       " note: delay is in milliseconds\n\n");
		printf("Examples:\n"
		       " %s -k Return 1 0\n"
		       " %s --key Super_L 0 2000\n"
		       " %s -m 150 300 1000\n"
		       " %s --motion 0 1000 0\n"
		       " %s -b 1 1 0\n"
		       " %s --button 3 0 1500\n",
		       argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
		return EXIT_FAILURE;
	} else if (argc == 5 && (!strcmp(argv[1], "--key") || !strcmp(argv[1], "-k"))) {
		fake_key(argv[2], atoi(argv[3]), (unsigned long)atoi(argv[4]));
	} else if (argc == 5 && (!strcmp(argv[1], "--motion") || !strcmp(argv[1], "-m"))) {
		fake_motion(atoi(argv[2]), atoi(argv[3]), (unsigned long)atoi(argv[4]));
	} else if (argc == 5 && (!strcmp(argv[1], "--button") || !strcmp(argv[1], "-b"))) {
		fake_button((unsigned int)atoi(argv[2]), atoi(argv[3]), (unsigned long)atoi(argv[4]));
	} else {
		fprintf(stderr, "%s error: invalid arguments, use %s -h [--help] for help\n", argv[0], argv[0]);
		XCloseDisplay(dpy);
		return EXIT_FAILURE;
	}

	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}

void fake_button(unsigned int button, int is_press, unsigned long delay)
{
	/*   mouse left button: button = 1
	 * mouse middle button: button = 2
	 *  mouse right button: button = 3
	 */
	XTestFakeButtonEvent(dpy, button, is_press, delay);
}

void fake_motion(int x, int y, unsigned long delay)
{
	int screen = DefaultScreen(dpy);
	XTestFakeMotionEvent(dpy, screen, x, y, delay);
}

void fake_key(const char *key, int is_press, unsigned long delay)
{
	XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XStringToKeysym(key)), is_press, delay);

}


/* vim: set ts=8 sts=8 sw=8 : */
