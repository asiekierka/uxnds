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

## Uxambly

Read more in the [Uxambly Guide](https://wiki.xxiivv.com/site/uxambly.html).

```
( hello world )

@RESET 
	
	,text1 ,print-label JSR2
	,text2 ,print-label JSR2
	#ab =Console.byte
	#cdef =Console.short

BRK

@print-label ( text )
	
	$loop NOP
		( send ) DUP2 LDR =Console.char
		( incr ) #0001 ADD2
		( loop ) DUP2 LDR #00 NEQ ^$loop MUL JMPS 
	POP2

RTN    

@text1 [ Welcome 20 to 20 UxnVM 0a00 ]
@text2 [ Hello 20 World 0a00 ] 

|c000 @FRAME
|d000 @ERROR 

|FF00 ;Console { pad 8 char 1 byte 1 short 2 }

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
- Jump helpers
- Don't brk when return stack is not zeroed
- LDRS should load from the zeropage?
- Keep ref counts in macros

### Macros

```
&RTS { RSW JMP } 
&JSR { PRG WSR JMP }
```

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
