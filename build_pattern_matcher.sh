#!/bin/bash

make && echo -n "$1" | ./regex2c >pattern.c && make pattern_matcher && ./pattern_matcher
