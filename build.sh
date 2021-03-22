#!/bin/bash

# Create bin folder
mkdir -p bin

# Assembler
clang-format -i assembler.c
rm -f ./bin/assembler
rm -f ./bin/boot.rom
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined assembler.c -o bin/assembler

# Core
clang-format -i uxn.h
clang-format -i uxn.c

# Emulator
clang-format -i emulator.c
rm -f ./bin/emulator
cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined uxn.c emulator.c -L/usr/local/lib -lSDL2 -o bin/emulator
# cc uxn.c emulator.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -L/usr/local/lib -lSDL2 -o bin/emulator

# Emulator(CLI)
clang-format -i emulator-cli.c
rm -f ./bin/emulator-cli

# run
./bin/assembler projects/software/nasu.usm bin/boot.rom
./bin/emulator bin/boot.rom
