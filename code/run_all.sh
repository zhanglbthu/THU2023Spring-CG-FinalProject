#!/usr/bin/env bash

# If project not ready, generate cmake file.
if [[ ! -d build ]]; then
    echo "good"
else
    rm -rf build
fi
cmake -B build
cmake --build build

# Run all testcases. 
# You can comment some lines to disable the run of specific examples.

mkdir -p output/rt
mkdir -p output/pt
mkdir -p output/sppm
# build/PA1 testcases/room.txt output/rt/room
# build/PA1 testcases/rabbit.txt output/rt/rabbit
# build/PA1 testcases/room.txt output/pt/room
# build/PA1 testcases/cornell.txt output/pt/cornelltest3
# build/PA1 testcases/skyplane.txt output/pt/skyplane
build/PA1 testcases/china.txt output/pt/chinatest03
# build/PA1 testcases/texture_ez.txt output/pt/ez
# build/PA1 testcases/curve.txt output/pt/curve
# build/PA1 testcases/result01.txt output/pt/result03
# build/PA1 testcases/refrac.txt output/pt/refrac
# build/PA1 testcases/smallpt1.txt output/pt/smallpt1test
# build/PA1 testcases/s.txt output/pt/s
# build/PA1 testcases/obj.txt output/pt/obj
# build/PA1 testcases/texture.txt output/pt/texture
# build/PA1 testcases/sky_dragon_clear.txt output/pt/sky_dragon_clear
# build/PA1 testcases/sky_dragon.txt output/pt/sky_dragon_head
# build/PA1 testcases/sky_texture.txt output/pt/sky_texture
# build/PA1 testcases/night_heart.txt output/pt/night_heart_new
# build/PA1 testcases/objtest.txt output/pt/objtest
# build/PA1 testcases/combat.txt output/pt/comba
# build/PA1 testcases/depth.txt output/pt/depthtest
# build/PA1 testcases/diamond.txt output/pt/diamond
# build/PA1 testcases/rabbit.txt output/pt/rabbit
