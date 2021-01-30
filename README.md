# Uxn

A stack-based VM, written in ANSI C.

## Build

```
cc uxn.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o uxn
```

## Assembly Syntax

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

## Design

### CPU

- Build stack with pointer
- Print stack
- Build memory

### PPU

### Assembly

#### Addressing 

- `label`, a named offset[TODO]
- `literal`, a numeric value
- `pointer`, pointer to an address[TODO]

### Assembler


### Emulator

- SDL Layer


## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/