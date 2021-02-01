# Uxn

A stack-based VM, written in ANSI C.

## Build

```
cc uxn.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o uxn
```

## Assembly Syntax

- `:label`, a named offset
- `+literal`, a numeric value
- `.pointer`, pointer to a label

```
< comment >

+01 < literal >

[ 01 02 03 04 ] < block of literals >

$01 < pointer8 >

{ 01 02 03 04 } < block of pointer8 >

~ff0f < pointer16 >

( ff00 ff01 ff02 ff03 ) < block of pointer16 >

=const +ff

:label ADD RTS
```

## Mission

- Carry flag?
- Loop
- Pointers/Literals
- Print word to stdout
- Draw pixel to screen
- Detect mouse click
- 16 bits addressing
- jumping to subroutine should be relative

## TODOs

- Implement addressing
- Implement 16 bits operations
- Jumps should be relative
- Catch overflow/underflow
- Audo-detect literals length.
- SDL Layer Emulator
- Build PPU
- Interrupts

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/