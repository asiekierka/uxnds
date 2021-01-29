# Uxn

A stack-based VM, written in ANSI C.

## Build

```
cc uxn.c -std=c89 -Os -DNDEBUG -g0 -s -Wall -Wno-unknown-pragmas -o uxn
```

## OP Codes

```
VALUE       OPCODE  EXPLANATION
0x00000000  NOP     do nothing
0x00000001  ADD     pop a, pop b, push a + b
0x00000002  SUB     pop a, pop b, push a - b
0x00000003  AND     pop a, pop b, push a & b
0x00000004  OR      pop a, pop b, push a | b
0x00000005  XOR     pop a, pop b, push a ^ b
0x00000006  NOT     pop a, push !a
0x00000007  IN      read one byte from stdin, push as word on stack
0x00000008  OUT     pop one word and write to stream as one byte
0x00000009  LOAD    pop a, push word read from address a
0x0000000A  STOR    pop a, pop b, write b to address a
0x0000000B  JMP     pop a, goto a
0x0000000C  JZ      pop a, pop b, if a == 0 goto b
0x0000000D  PUSH    push next word
0x0000000E  DUP     duplicate word on stack
0x0000000F  SWAP    swap top two words on stack
0x00000010  ROL3    rotate top three words on stack once left, (a b c) -> (b c a)
0x00000011  OUTNUM  pop one word and write to stream as number
0x00000012  JNZ     pop a, pop b, if a != 0 goto b
0x00000013  DROP    remove top of stack
0x00000014  PUSHIP  push a in IP stack
0x00000015  POPIP   pop IP stack to current IP, effectively performing a jump
0x00000016  DROPIP  pop IP, but do not jump
0x00000017  COMPL   pop a, push the complement of a
```

## Design

### CPU

- Build stack with pointer
- Print stack
- Build memory

### PPU

### Assembly

- `%25`, decimal
- `#25`, hex

```
2 2 + $ef
```

### Assembler


### Emulator

- SDL Layer
