#!/usr/bin/env python2

""" test_splitwm - perform set of random operations to test splitwm
    Copyright (C) 2013  Tomas Meszaros [exo at tty dot com]

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

from random import choice
from os import system, popen
from sys import argv, exit
from min_set import alphabet, butns, mods


# can change this
path = "/usr/local/bin/xfakeevent"
apps = [ "Terminal", "xterm", "gedit" ]


# dont change this ------------------------------------------------------------#
if not len(argv) == 3:
	exit("Usage: %s tests delay(ms)" % argv[0])
tests = int(argv[1])
sleep = int(argv[2])
screen = popen("xrandr -q -d :0").readlines()[0]
screen_width = int(screen.split()[7])
screen_height = int(screen.split()[9][:-1])
cnt = 0

def main():
	for i in range(tests):
		global cnt
		cnt += 1
		fcn = choice(range(5))
		if fcn == 0: key_solo()
		elif fcn == 1: key_double()
		elif fcn == 2: key_tripple()
		elif fcn == 3: motion()
		elif fcn == 4: button()

		if cnt % 10 == 0:
			cnt += 1
			app()
		if cnt % 30 == 0:
			cnt += 1
			kill_app()

def app():
	""" launch application
	"""
	from subprocess import Popen
	a = choice(apps)
	Popen(a)

	print "%d\t         app:" % cnt, a

def kill_app():
	""" kill application
	"""
	m1 = "Shift_L"
	m2 = "Alt_L"
	a = "c"

	print "%d\t    kill_app:" % cnt
	xfakeevent("-k", m1, 1, 0)
	xfakeevent("-k", m2, 1, 0)
	xfakeevent("-k", a, 1, 0)
	xfakeevent("-k", a, 0, sleep)
	xfakeevent("-k", m2, 0, sleep)
	xfakeevent("-k", m1, 0, sleep)


def button():
	""" mouse button click
	"""
	b = choice(range(1, 4))

	print "%d\t      button:" % cnt, b
	xfakeevent("-b", b, 1, sleep)
	xfakeevent("-b", b, 0, sleep)

def motion():
	""" mouse pointer motion
	"""
	x = choice(range(screen_width))
	y = choice(range(screen_height))

	print "%d\t      motion:" % cnt, x, y
	xfakeevent("-m", x, y, sleep)


def key_solo():
	""" one key press
	"""
	a = choice(range(2))
	if a == 0: a = choice(alphabet)
	else: a = choice(butns)
	
	print "%d\t    key_solo:" % cnt, a
	xfakeevent("-k", a, 1, 0)
	xfakeevent("-k", a, 0, sleep)

def key_double():
	""" combination of two keys
	"""
	m = choice(mods)
	a = choice(range(2))
	if a == 0: a = choice(alphabet)
	else: a = choice(butns)

	print "%d\t  key_double:" % cnt, m, a
	xfakeevent("-k", m, 1, 0)
	xfakeevent("-k", a, 1, 0)
	xfakeevent("-k", a, 0, sleep)
	xfakeevent("-k", m, 0, sleep)

def key_tripple():
	""" combination of three keys
	"""
	m1 = choice(mods)
	m2 = choice(mods)
	a = choice(range(2))
	if a == 0: a = choice(alphabet)
	else: a = choice(butns)

	if (a in ["F1", "F2", "F3", "F4", "F5", "F6",
		"F7", "F8", "F9", "F10", "F11", "F12"]):
		return

	print "%d\t key_tripple:" % cnt, m1, m2, a
	xfakeevent("-k", m1, 1, 0)
	xfakeevent("-k", m2, 1, 0)
	xfakeevent("-k", a, 1, 0)
	xfakeevent("-k", a, 0, sleep)
	xfakeevent("-k", m2, 0, sleep)
	xfakeevent("-k", m1, 0, sleep)

def xfakeevent(option, a, b, delay):
	system(path + " %s %s %d %d" % (option, a, b, delay))

if __name__ == "__main__":
	main()
