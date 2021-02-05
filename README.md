# Uxn

A stack-based VM, written in ANSI C.

## Build

```
cc uxn.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o uxn
```

## Assembly Syntax

### Write

- `;variable`, set a label to an assigned address
- `:const`, set a label to a constant short
- `@label`, set a label to an address

### Read

- `,literal`, push label value to stack
- `.pointer`, read label value

### Special

- `( comment )`, toggle parsing on/off
- `|0010`, move to position in the program
- `"hello`, push literal bytes for word "hello"

```
( hello world )

;iterator
:dev1r FFF0
:dev1w FFF1

|0100 @RESET

"hello

@loop
	,dev1w STR
	,iterator LDR
	,01 ADD
	,iterator STR 
	,iterator LDR
	,05 NEQ ,loop ROT JSC

BRK ( RESET )

|c000 @FRAME BRK 
|d000 @ERROR BRK 
|FFFA .RESET .FRAME .ERROR

```

## Mission

### Assembler

- Catch overflow/underflow
- Jumps should be relative

### CPU

- Pointers/Literals
- A Three-Way Decision Routine(http://www.6502.org/tutorials/compare_instructions.html)
- Print word to stdout
- Draw pixel to screen
- Detect mouse click
- SDL Layer Emulator
- Build PPU
- Add flags..

### Devices

- Devices each have an input byte, an output byte and two request bytes.

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/