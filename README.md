# Uxn

An [8-bit stack-based computer](https://wiki.xxiivv.com/site/uxn.html), written in ANSI C. 

## Build

To build the Uxn emulator, you must have [SDL2](https://wiki.libsdl.org/) and [Portmidi](http://portmedia.sourceforge.net/portmidi/).

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
( dev/console )

%RTN { JMP2r }

( devices )

|10 @Console    [ &pad $8 &char $1 ]

( init )

|0100 ( -> )
	
	;hello-word ,print JSR
	
BRK

@print ( addr* -- )
	
	&loop
		( send ) DUP2 GET .Console/char DEO
		( incr ) #0001 ADD2
		( loop ) DUP2 GET #00 NEQ ,&loop JNZ
	POP2

RTN

@hello-word "hello 20 "World!
```

## TODOs

- Shortcut to export/import disk state

## Palettes

- `#6a03` `#4a0d` `#aa0c`, purple/cyan
- `#a1f3` `#a14d` `#a16c`, grey-pink/teal
- `#8c4b` `#884b` `#e8bb`, commodore64

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
