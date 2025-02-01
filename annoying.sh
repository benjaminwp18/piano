#!/bin/sh

DUR=201
FREQ=200
while true
do
	piano -d "$DUR" < songs/gladiators -t sawtooth -b "$FREQ"
	[ "$DUR" -gt 1 ] && let DUR-=20 FREQ+=30
done
