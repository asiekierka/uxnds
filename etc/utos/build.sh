#!/bin/bash

echo "Formatting.."
clang-format -i utos.c

echo "Cleaning.."
rm -f ../../bin/utos

echo "Building.."
mkdir -p ../../bin
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -lm utos.c -o ../../bin/utos

echo "Running.."
../../bin/utos ../../projects/sounds/pad1.ss8 ../../projects/sounds/pad1.pcm

echo "Done."
