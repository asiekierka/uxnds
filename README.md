# Uxn

A [stack-based VM](https://wiki.xxiivv.com/site/uxn.html), written in ANSI C. 

## Build

To build the Uxn emulator, you must have [SDL2](https://wiki.libsdl.org/).

```sh
./build.sh 
	--debug # Add debug flags to compiler
	--cli # Run rom without graphics
```

## Emulator Controls

- `ctrl+h` toggle debugger
- `alt+h` toggle zoom

## Uxambly

Read more in the [Uxambly Guide](https://wiki.xxiivv.com/site/uxambly.html).

```
( hello world )

%RTN { JMP2r }

( devices )

|0100 ;Console { pad 8 char 1 byte 1 short 2 }
|01F0 .RESET .FRAME .ERROR ( vectors )
|01F8 [ 13fd 1ef3 1bf2 ] ( palette )

( program )

|0200 @RESET 
	
	,text1 ,print-label JSR2
	,text2 ,print-label JSR2
	#ab =Console.byte
	#cdef =Console.short

BRK

@print-label ( text )
	
	$loop
		( send ) DUP2 PEK2 =Console.char
		( incr ) #0001 ADD2
		( loop ) DUP2 PEK2 #00 NEQ ^$loop JNZ
	POP2

RTN    

@text1 [ Welcome 20 to 20 UxnVM 0a00 ]
@text2 [ Hello 20 World 0a00 ] 

@FRAME BRK
@ERROR BRK 
```

## TODOs

### OS Boot Disk

- Load external disk in disk2

### Assembler

- Includes
- Defines
- Jump helpers
- Implement Peek/Pook to helpers
- Create a theme designer application
- DateTime device
- Document controller.player2

## Notes

## Palettes

- `[ 6a03 4a0d aa0c ]`, purple/cyan
- `[ a1f3 a14d a16c ]`, grey-pink/teal

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
