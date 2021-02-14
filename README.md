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

### Assign

- `@label`, assign the current address to a label.
- `;variable 2`, assign an address to a label automatically.
- `:const 1a2b`, assign an address to a label manually.

### Read

- `,literal`, push label value to stack, prefixed with `LIT LEN`.
- `.pointer`, push label value to stack.

### Write

- `ADD`, an opcode.
- `#1a`, a literal byte.
- `#12ef`, a literal short.
- `+1a`, a literal signed byte.
- `+12ef`, a literal signed short.
- `-1a`, a literal signed byte(negative).
- `-12ef`, a literal signed short(negative).
- `.ab`, a raw byte in memory.
- `.abcd`, a raw short in memory.

### Special

- `( comment )`, toggle parsing on/off.
- `|0010`, move to position in the program.
- `"hello`, push literal bytes for word "hello".

### Operator modes

- `#1234 #0001 ADD2`, 16-bits operators have the short flag `2`.
- `#12 #11 GTH JMP?`, conditional operators have the cond flag `?`.
- `+21 -03 MULS`, signed operators have the cond flag `S`.

```
( comment )

:dev/w fff9 ( const write port )

|0100 @RESET 

	#00 ,dev/w STR                        ( set dev/write to console ) 
	,string                               ( add string pointer to stack )
	@loop
		DUP2 LDR IOW                         ( write pointer value to console )
		#0001 ADD2                           ( increment string pointer )
		DUP2 LDR #00 NEQ ,loop ROT JMP? POP2 ( while *ptr!=0 goto loop )

BRK

@string " Hello World "                ( add string to memory )

|c000 @FRAME BRK
|d000 @ERROR BRK 

|FFFA .RESET .FRAME .ERROR
```

## Emulator

### Controller(dev/ctrl)

A device that works like a NES controller, each button is a bit from a single byte.

- Ctrl
- Alt
- Escape
- Return
- Up
- Down
- Left
- Right

## TODOs

- Defines?
- LDR/STR helpers
- Keyboard example
- PPU chr device
- Line routine
- On-screen debugger.
- Auto-advance ldr?
- Getting rid of IOR/IOW would be nice..
- Sending from the wst to the rst, balance mode/flag?
- Device that works like an extra memory bank
- [debug]Print unused labels

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
