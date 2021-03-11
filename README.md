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

&Console { pad 8 char 1 byte 1 short 2 }

|0100 @RESET 
	
	,text1 ,print-label JSR
	,text2 ,print-label JSR
	#ab =dev/console.byte
	#cdef =dev/console.short

BRK

@print-label ( text )

	@print-label-loop
		DUP2 LDR =dev/console.char               ( write pointer value to console )
		#0001 ADD2                               ( increment string pointer )
		DUP2 LDR #00 NEQ ,print-label-loop ROT JMP? POP2  ( while *ptr!=0 goto loop )
	POP2

RTS                 

@text1 [ Hello World 0a00 ] ( store text with a linebreak and null byte )
@text2 [ Welcome to UxnVM 0a00 ]

|c000 @FRAME
|d000 @ERROR 

|FF00 ;dev/console Console

|FFF0 .RESET .FRAME .ERROR ( vectors )
|FFF8 [ 13fd 1ef3 1bf2 ] ( palette )
```

## TODOs

### OS Boot Disk

- Load external disk in disk2
- Build hex editor

### Assembler

- Includes
- Defines
- Jump relative
- Local loops
- Jump helpers

## Palettes

- `[ 6a03 4a0d aa0c ]`, purple/cyan
- `[ a1f3 a14d a16c ]`, grey-pink/teal

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
