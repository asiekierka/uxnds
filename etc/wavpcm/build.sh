#!/bin/bash

echo "Formatting.."
clang-format -i wavpcm.c

echo "Cleaning.."
rm -f ../../bin/wavpcm

echo "Building.."
mkdir -p ../../bin
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -lm wavpcm.c -o ../../bin/wavpcm

echo "Running.."
../../bin/wavpcm

echo "Done."
