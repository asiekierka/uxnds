# Uxn

A stack-based VM, written in ANSI C.

## Build

```
cc uxn.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o uxn
```

## Assembly Syntax

- `:label`, a named address
- `+literal`, a numeric value
- `.pointer`, pointer to a label

```
< conditionals >

0302	ADD 
05		EQU

.there	JMQ

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

- constants
- variables
- A Three-Way Decision Routine(http://www.6502.org/tutorials/compare_instructions.html)
- Carry flag?
- Loop
- Pointers/Literals
- Print word to stdout
- Draw pixel to screen
- Detect mouse click
- Jumps should be relative
- Catch overflow/underflow
- Audo-detect literals length.
- SDL Layer Emulator
- Build PPU
- Interrupts

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