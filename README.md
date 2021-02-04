# Uxn

A stack-based VM, written in ANSI C.

## Build

```
cc uxn.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o uxn
```

## Assembly Syntax

- `;variable`, a named address(zero-page)
- `:label`, a named address
- `.pointer`, a pointer to a label
- `@0010`, a position in the program

```
< conditionals >

.there ( 0a 05 GTH ) JMC

:here
	< when not equal >
	ee
	BRK

:there
	< when is equal >
	ff
	BRK
```

## Mission

### Assembler

- Crash on missing label
- Catch overflow/underflow
- Constants
- Jumps should be relative

### CPU

- Pointers/Literals
- A Three-Way Decision Routine(http://www.6502.org/tutorials/compare_instructions.html)
- Carry flag?
- Print word to stdout
- Draw pixel to screen
- Detect mouse click
- SDL Layer Emulator
- Build PPU
- Interrupts, vectors

### 16 Bit Missions

- 16 bits addressing
- jumping to subroutine should be relative
- Implement addressing
- Implement 16 bits operations

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/