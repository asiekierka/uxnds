#!/bin/bash
set -e
EMULATOR=./bin/emulator
if [ "${1}" = '--no-sdl' ]; then
    EMULATOR=./bin/emulator-nosdl
    shift
fi
if [ -z "${1}" ]; then
    printf 'usage: %s [--no-sdl] USM_FILE\n' "${0}" >&2
    exit 2
fi
./bin/assembler "${1}" bin/boot.rom
"${EMULATOR}" bin/boot.rom
