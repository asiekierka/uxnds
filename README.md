# Uxn

A [stack-based VM](https://wiki.xxiivv.com/site/uxn.html), written in ANSI C.

## Setup

If you wish to build your own emulator, you can create a new instance of Uxn like:

```
#include "uxn.h"

Uxn u;

if(!bootuxn(&u))
	return error("Boot", "Failed");
if(!loaduxn(&u, argv[1]))
	return error("Load", "Failed");
if(!init())
	return error("Init", "Failed");

evaluxn(u, u->vreset); /* Once on start */
evaluxn(u, u->vframe); /* Each frame
```

## Assembly Syntax

- `ADD`, an opcode.
- `@label`, assign the current address to a label.
- `;variable 2`, assign an address to a label automatically.
- `:const 1a2b`, assign an address to a label manually.
- `&macro { x 2 y 2 }`, define a macro named `macro`.
- `#1a`, a literal byte/short.
- `+1a`, a literal signed byte/short.
- `-1a`, a literal signed byte/short(negative).
- `.ab`, a raw byte/short in memory.
- `,literal`, push label address to stack, prefixed with `LIT LEN`.
- `=label`, helper to STR, equivalent to `,label STR`, or `label STR2`.
- `~label`, helper to LDR, equivalent to `,label LDR2`, or `,label LDR2`.
- `|0010`, move to position in the program.
- `<23`, move the program position `23` bytes backward.
- `>12`, move the program position `12` bytes forward.
- `( comment )`, toggle parsing on/off.
- `[ 0123 abcd ]`, write shorts to memory.
- `[ Hello World ]`, write text to memory.

### Operator modes

- `#1234 #0001 ADD2`, 16-bits operators have the short flag `2`.
- `#12 #11 GTH JMP?`, conditional operators have the cond flag `?`.
- `+21 -03 MULS`, signed operators have the cond flag `S`.
- `ADDS2?`, modes can be combined.

```
( hello world )

:dev/w fff9 ( const write port )

|0100 @RESET 
	
	#00 =dev/w ( set dev/write to console ) 
	,text1 ,print-label JSR ( print to console )

BRK

@print-label ( text )

	@cliloop
		DUP2 LDR IOW                             ( write pointer value to console )
		#0001 ADD2                               ( increment string pointer )
		DUP2 LDR #00 NEQ ,cliloop ROT JMP? POP2  ( while *ptr!=0 goto loop )
	POP2
		
RTS                 

@text1 [ Hello World ] <1 .00 ( add text to memory, return 1 byte, add null byte )

|c000 @FRAME
|d000 @ERROR 

|FFF0 [ f3f0 f30b f30a ] ( palette )
|FFFA .RESET .FRAME .ERROR
```

## Emulator

### Controller(dev/ctrl)

A device that works like a NES controller, each button is a bit from a single byte. Press `h` to toggle debugger.

- `0x01` Ctrl
- `0x02` Alt
- `0x04` Escape
- `0x08` Return
- `0x10` Up
- `0x20` Down
- `0x40` Left
- `0x80` Right

## TODOs

### OS Boot Disk

- Load external disk in disk2
- Build hex editor
- Build sprite editor

### Examples

- Basics:
	- Simple drag/drop redraw
	- Window basics with open/close drag/drop redraw
	- Example of button pointing to a subroutine
- GUI:
	- Line routine

### Devices redesign

- Possibly remove

### Assembler

- Includes
- Defines
- Print unused labels

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
