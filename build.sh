#!/bin/bash

echo "Formatting.."
clang-format -i src/uxn.h
clang-format -i src/uxn.c
clang-format -i src/ppu.h
clang-format -i src/ppu.c
clang-format -i src/apu.h
clang-format -i src/apu.c
clang-format -i src/assembler.c
clang-format -i src/emulator.c
clang-format -i src/debugger.c

echo "Cleaning.."
rm -f ./bin/assembler
rm -f ./bin/emulator
rm -f ./bin/debugger
rm -f ./bin/boot.rom

echo "Building.."
mkdir -p bin
if [ "${1}" = '--debug' ]; 
then
	echo "[debug]"
    cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined src/assembler.c -o bin/assembler
	cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined src/uxn.c src/ppu.c src/emulator.c src/apu.c -L/usr/local/lib -lSDL2 -o bin/emulator
    cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined src/uxn.c src/debugger.c -o bin/debugger
else
	cc src/assembler.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o bin/assembler
	cc src/uxn.c src/debugger.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o bin/debugger
	cc src/uxn.c src/ppu.c src/emulator.c src/apu.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -L/usr/local/lib -lSDL2 -o bin/emulator
fi

echo "Assembling.."
./bin/assembler projects/examples/dev.mouse.usm bin/boot.rom

echo "Running.."
if [ "${2}" = '--cli' ]; 
then
	echo "[cli]"
	./bin/debugger bin/boot.rom
else
	./bin/emulator bin/boot.rom
fi

echo "Done."