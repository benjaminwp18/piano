#!/bin/sh

for i in $(seq 100 -10 40)
do
	yes qwerpoiuasdflkjh | head -n 3 | piano -d "$i" -t sawtooth
done
