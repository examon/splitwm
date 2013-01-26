#!/usr/bin/env python2

from random import choice
from os import system
import small_key_set

path = "/home/exo/Github/splitwm/test/test"
sleep = 0
tests = 10000
cnt = 1

def main():
	for i in range(tests):
		fcn = choice(range(3))
		if fcn == 0: solo()
		elif fcn == 1: double()
		else: tripple()
		global cnt
		cnt += 1

def solo():
	a = choice(range(2))
	if a == 0: a = choice(small_key_set.alphabet)
	else: a = choice(small_key_set.butns)
	
	print "%d\t    solo:" % cnt, a

	test_key(a, 1, 0)
	test_key(a, 0, sleep)

def double():
	m = choice(small_key_set.mods)
	a = choice(range(2))
	if a == 0: a = choice(small_key_set.alphabet)
	else: a = choice(small_key_set.butns)

	print "%d\t  double:" % cnt, m, a

	test_key(m, 1, 0)
	test_key(a, 1, 0)
	test_key(a, 0, sleep)
	test_key(m, 0, sleep)

def tripple():
	m1 = choice(small_key_set.mods)
	m2 = choice(small_key_set.mods)
	a = choice(range(2))
	if a == 0: a = choice(small_key_set.alphabet)
	else: a = choice(small_key_set.butns)

	if (a in ["F1", "F2", "F3", "F4", "F5", "F6",
		"F7", "F8", "F9", "F10", "F11", "F12"]):
		return

	print "%d\t tripple:" % cnt, m1, m2, a

	test_key(m1, 1, 0)
	test_key(m2, 1, 0)
	test_key(a, 1, 0)
	test_key(a, 0, sleep)
	test_key(m2, 0, sleep)
	test_key(m1, 0, sleep)

def test_key(key, key_type, delay):
	system(path + " %s %d %d" % (key, key_type, delay))

if __name__ == "__main__":
	main()
