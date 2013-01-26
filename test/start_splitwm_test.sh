#!/bin/bash

# path to test_splitwm.py
test_splitwm="/home/exo/Github/splitwm/test/test_splitwm/test_splitwm.py"

# number of tests
tests=1000

# delay (in ms)
delay=0

# log filename
log="test_splitwm.log"

$test_splitwm $tests $delay > $log
