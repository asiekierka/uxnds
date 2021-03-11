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
- `.address`, push label address to memory.
- `,literal`, push label address to stack, prefixed with `LIT LEN`.
- `#1a`, a literal byte/short.
- `+1a`, a literal signed byte/short.
- `-1a`, a literal signed byte/short(negative).
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
	
	,text1 ,print-label JSR2
	,text2 ,print-label JSR2
	#ab =CNSL.byte
	#cdef =CNSL.short

BRK

@print-label ( text )
	
	@print-label-loop NOP
		( send ) DUP2 LDR =CNSL.char
		( incr ) #0001 ADD2
		DUP2 LDR #00 NEQ ^print-label-loop MUL JMPS 
	POP2

RTS                 

@text1 [ Hello 20 World 0a00 ] ( store text with a linebreak and null byte )
@text2 [ Welcome 20 to 20 UxnVM 0a00 ]

|c000 @FRAME
|d000 @ERROR 

|FF00 ;CNSL Console

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

NOTE: OPCODES should not be relative, but there should be a relative accessor for addresses, like:

$relative_name JMP

## Notes

### Conditional Jumping

I've considered automatically popping an amount of items from the stack equal to the offset between the opcode's push/pop to make the stack length more predictable, and making the pattern JMP? POP2 unecessary, but that idea would make DUP? unusable. That change was reverted.

## Palettes

- `[ 6a03 4a0d aa0c ]`, purple/cyan
- `[ a1f3 a14d a16c ]`, grey-pink/teal

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
