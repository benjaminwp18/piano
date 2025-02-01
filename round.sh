#!/bin/sh

cat songs/round | piano -d 150 &
(echo "6 6 " && cat songs/round) | piano -d 150 &
(echo "6 6 6 6 " && cat songs/round) | piano -d 150 &
(echo "6 6 6 6 6 6 " && cat songs/round) | piano -d 150 &
(echo "6 6 6 6 6 6 6 6 " && cat songs/round) | piano -d 150 &
