#!/bin/bash

# format code
clang-format -i uxnasm.c
clang-format -i uxn.c
clang-format -i cpu.h
clang-format -i cpu.c

# remove old
rm -f ./uxnasm
rm -f ./uxn
rm -f ./boot.rom

# debug(slow)
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined uxnasm.c -o uxnasm
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined cpu.c uxn.c -o uxn

# run
./uxnasm examples/hello.usm boot.rom
./uxn boot.rom
