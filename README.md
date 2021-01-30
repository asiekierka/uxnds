# Uxn

A stack-based VM, written in ANSI C.

## Build

```
cc uxn.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o uxn
```

## Assembly Syntax

```
: 	starting a definition
& 	obtaining pointers
( 	stack comments
` 	inlining bytecodes
' 	strings
# 	numbers
$ 	characters
~   vector
[ 12 34 ] real values
< 12 34 > relative values
( 12 34 ) deadzone
```

```
;add-two JSR

BRK

:add-two
	[ 2 ] ADD RTS
```

## Design

### CPU

- Build stack with pointer
- Print stack
- Build memory

### PPU

### Assembly

#### Addressing 

- `label`, a named offset[TODO]
- `literal`, a numeric value
- `pointer`, pointer to an address[TODO]

### Assembler


### Emulator

- SDL Layer
