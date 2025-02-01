#!/bin/sh

echo '
4d2j2g2d2s2d2i2t2i3q 2t2i4i
2 2i2d2i2p2t2y2e2i2i8q6 2d2d
2d2d2d2i3i 2i2i2i2i2i2i3q 2d
2j2g2d2s2d2i2t2i3q 2d2q4i2 2i
2d2i2p2t2y2e2i2i8q' | piano -t triangle -d 150 -a 20 &

echo '
4d2j2g2d2s2d2i2d2g3j 2d2j4g
2 2d2j2g2d2s2p2g2d2s8d6 2j2j
2j2d2j2j3g 2g2g2g2s2g2g3d 2d
2j2g2d2s2d2i2d2g3j 2d2j4g2 2d
2j2g2d2s2p2g2d2s8d' | piano -t sawtooth -d 150 -a 20 &

echo '
4d2j2g2d2s2d2g2j2z3z 2j2z4z
2 2j2z2k2j2j2k2k2j2g8j6 2j2z
2z2j2z2z3k 2k2k2k2g2k2k3j 2d
2j2g2d2s2d2g2j2z3z 2j2z4z2 2j
2z2k2j2j2k2c2z2k8j' | piano -t sine -d 150 -a 20 &
