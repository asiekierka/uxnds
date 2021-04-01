#!/bin/bash

echo "Formatting.."
clang-format -i tables.c

echo "Cleaning.."
rm -f ../../bin/tables

echo "Building.."
mkdir -p ../../bin
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined -lm tables.c -o ../../bin/tables

echo "Assembling.."
../../bin/tables 

echo "Done."
