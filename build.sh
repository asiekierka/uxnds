#!/bin/bash

# Create bin folder
mkdir -p bin

# Assembler
clang-format -i assembler.c
rm -f ./assembler
rm -f ./boot.rom
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined assembler.c -o bin/assembler
./bin/assembler examples/hello.usm bin/boot.rom

# Emulator
clang-format -i emulator.c
clang-format -i uxn.h
clang-format -i uxn.c
rm -f ./uxn
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined uxn.c emulator.c -o bin/emulator

# run
./bin/emulator bin/boot.rom
