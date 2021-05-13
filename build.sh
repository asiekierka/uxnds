#!/bin/bash

echo "Formatting.."
clang-format -i src/uxn.h
clang-format -i src/uxn.c
clang-format -i src/devices/ppu.h
clang-format -i src/devices/ppu.c
clang-format -i src/devices/apu.h
clang-format -i src/devices/apu.c
clang-format -i src/devices/mpu.h
clang-format -i src/devices/mpu.c
clang-format -i src/assembler.c
clang-format -i src/emulator.c
clang-format -i src/debugger.c

echo "Cleaning.."
rm -f ./bin/uxnasm
rm -f ./bin/uxnemu
rm -f ./bin/debugger
rm -f ./bin/boot.rom

echo "Building.."
mkdir -p bin
if [ "${1}" = '--debug' ]; 
then
	echo "[debug]"
    cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined src/assembler.c -o bin/uxnasm
	cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined src/uxn.c src/devices/ppu.c src/devices/apu.c src/devices/mpu.c src/emulator.c -L/usr/local/lib -lSDL2 -lportmidi -o bin/uxnemu
    cc -std=c89 -DDEBUG -Wall -Wno-unknown-pragmas -Wpedantic -Wshadow -Wextra -Werror=implicit-int -Werror=incompatible-pointer-types -Werror=int-conversion -Wvla -g -Og -fsanitize=address -fsanitize=undefined src/uxn.c src/debugger.c -o bin/debugger
else
	cc src/assembler.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o bin/uxnasm
	cc src/uxn.c src/debugger.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o bin/debugger
	cc src/uxn.c src/devices/ppu.c src/devices/apu.c src/devices/mpu.c src/emulator.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -L/usr/local/lib -lSDL2 -lportmidi -o bin/uxnemu
fi

echo "Installing.."
if [ -d "$HOME/bin" ] && [ -e ./bin/uxnemu ] && [ -e ./bin/uxnasm ]
then
	cp ./bin/uxnemu $HOME/bin
	cp ./bin/uxnasm $HOME/bin
    echo "Installed in $HOME/bin" 
fi

echo "Assembling.."
./bin/uxnasm projects/demos/polycat.usm bin/boot.rom

echo "Running.."
if [ "${2}" = '--cli' ]; 
then
	echo "[cli]"
	./bin/debugger bin/boot.rom
else
	./bin/uxnemu bin/boot.rom
fi

echo "Done."