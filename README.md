# Uxn

An [8-bit stack-based computer](https://wiki.xxiivv.com/site/uxn.html), written in ANSI C. 

## Build

To build the Uxn emulator on Linux, you must have [SDL2](https://wiki.libsdl.org/) and [Portmidi](http://portmedia.sourceforge.net/portmidi/).

```sh
./build.sh 
	--debug # Add debug flags to compiler
	--cli # Run rom without graphics
```

To build the Uxn emulator on [9front](http://9front.org/), via [npe](https://git.sr.ht/~ft/npe):

```rc
mk
```

If the build fails on 9front because of missing headers or functions,
try again after `rm -r /sys/include/npe`.

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
	
	,hello-word 

	&loop
		( send ) LDRk .Console/char DEO
		( incr ) #01 ADD
		( loop ) DUP ,&loop JCN
	POP
	
BRK

@hello-word "hello 20 "World!
```

## TODOs

- Shortcut to export/import disk state

## Palettes

- `#6a03` `#4a0d` `#aa0c`, purple/cyan
- `#a1f3` `#a14d` `#a16c`, grey-pink/teal
- `#8c4b` `#884b` `#e8bb`, commodore64

## Convert audio for Unx

```sox sub202_C.wav -b 8 -c 1 -e signed output.raw```

## Refs

https://code.9front.org/hg/plan9front/file/a7f9946e238f/sys/src/games/nes/cpu.c
http://www.w3group.de/stable_glossar.html
http://www.emulator101.com/6502-addressing-modes.html
http://forth.works/8f0c04f616b6c34496eb2141785b4454
https://justinmeiners.github.io/lc3-vm/
