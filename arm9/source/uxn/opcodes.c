/*
Copyright (u) 2021 Devine Lu Linvega
Copyright (c) 2021 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

/* 8-BIT OPCODES */

case UXN_OPC(0x00):
case UXN_OPC(0x20): { // brk
	u->ram.ptr = 0;
} break;
case UXN_OPC(0x01): { // lit
	push8(UXN_SRC, mempeek8(u->ram.dat, u->ram.ptr++));
} break;
case UXN_OPC(0x02):
case UXN_OPC(0x22): { // nop
} break;
case UXN_OPC(0x03): { // pop
	UXN_KEEP_SYNC;
	pop8(UXN_SRC);
} break;
case UXN_OPC(0x04): { // dup
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, a);
	push8(UXN_SRC, a);
} break;
case UXN_OPC(0x05): { // swp
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, a);
	push8(UXN_SRC, b);
} break;
case UXN_OPC(0x06): { // ovr
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b);
	push8(UXN_SRC, a);
	push8(UXN_SRC, b);
} break;
case UXN_OPC(0x07): { // rot
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	Uint8 c = pop8(UXN_SRC);
	push8(UXN_SRC, b);
	push8(UXN_SRC, a);
	push8(UXN_SRC, c);
} break;
case UXN_OPC(0x08): { // equ
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b == a);
} break;
case UXN_OPC(0x09): { // neg
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b != a);
} break;
case UXN_OPC(0x0A): { // gth
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b > a);
} break;
case UXN_OPC(0x0B): { // lth
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b < a);
} break;
case UXN_OPC(0x0C): { // jmp
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	u->ram.ptr += (Sint8)a;
} break;
case UXN_OPC(0x0D): { // jnz
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	if (pop8(UXN_SRC))
		u->ram.ptr += (Sint8)a;
} break;
case UXN_OPC(0x0E): { // jsr
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_DST, u->ram.ptr);
	u->ram.ptr += (Sint8)a;
} break;
case UXN_OPC(0x0F): { // sth
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_DST, a);
} break;
case UXN_OPC(0x10): { // pek
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, mempeek8(u->ram.dat, a));
} break;
case UXN_OPC(0x11): { // pok
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	mempoke8(u->ram.dat, a, b);
} break;
case UXN_OPC(0x12): { // ldr
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a));
} break;
case UXN_OPC(0x13): { // str
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
} break;
case UXN_OPC(0x14): { // lda
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	push8(UXN_SRC, mempeek8(u->ram.dat, a));
} break;
case UXN_OPC(0x15): { // sta
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	mempoke8(u->ram.dat, a, b);
} break;
case UXN_OPC(0x16): { // dei
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, devpeek8(&u->dev[a >> 4], a));
} break;
case UXN_OPC(0x17): { // deo
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	devpoke8(&u->dev[a >> 4], a, b);
} break;
case UXN_OPC(0x18): { // add
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b + a);
} break;
case UXN_OPC(0x19): { // sub
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b - a);
} break;
case UXN_OPC(0x1A): { // mul
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b * a);
} break;
case UXN_OPC(0x1B): { // div
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b / a);
} break;
case UXN_OPC(0x1C): { // and
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b & a);
} break;
case UXN_OPC(0x1D): { // ora
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b | a);
} break;
case UXN_OPC(0x1E): { // eor
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b ^ a);
} break;
case UXN_OPC(0x1F): { // sft
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b >> (a & 0x07) << ((a & 0x70) >> 4));
} break;

/* 16-BIT OPCODES */

case UXN_OPC(0x21): { // lit
	push16(UXN_SRC, mempeek16_i(u->ram.dat, u->ram.ptr));
	u->ram.ptr += 2;
} break;
case UXN_OPC(0x23): { // pop
	UXN_KEEP_SYNC;
	pop16(UXN_SRC);
} break;
case UXN_OPC(0x24): { // dup
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	push16(UXN_SRC, a);
	push16(UXN_SRC, a);
} break;
case UXN_OPC(0x25): { // swp
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, a);
	push16(UXN_SRC, b);
} break;
case UXN_OPC(0x26): { // ovr
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b);
	push16(UXN_SRC, a);
	push16(UXN_SRC, b);
} break;
case UXN_OPC(0x27): { // rot
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	Uint16 c = pop16(UXN_SRC);
	push16(UXN_SRC, b);
	push16(UXN_SRC, a);
	push16(UXN_SRC, c);
} break;
case UXN_OPC(0x28): { // equ
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b == a);
} break;
case UXN_OPC(0x29): { // neg
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b != a);
} break;
case UXN_OPC(0x2A): { // gth
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b > a);
} break;
case UXN_OPC(0x2B): { // lth
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b < a);
} break;
case UXN_OPC(0x2C): { // jmp
	UXN_KEEP_SYNC;
	u->ram.ptr = pop16(UXN_SRC);
} break;
case UXN_OPC(0x2D): { // jnz
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	if (pop8(UXN_SRC))
		u->ram.ptr = a;
} break;
case UXN_OPC(0x2E): { // jsr
	UXN_KEEP_SYNC;
	push16(UXN_DST, u->ram.ptr);
	u->ram.ptr = pop16(UXN_SRC);
} break;
case UXN_OPC(0x2F): { // sth
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	push16(UXN_DST, a);
} break;
case UXN_OPC(0x30): { // pek
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_SRC, mempeek16_i(u->ram.dat, a));
} break;
case UXN_OPC(0x31): { // pok
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	mempoke16_i(u->ram.dat, a, b);
} break;
case UXN_OPC(0x32): { // ldr
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_SRC, mempeek16_i(u->ram.dat, u->ram.ptr + (Sint8)a));
} break;
case UXN_OPC(0x33): { // str
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	mempoke16_i(u->ram.dat, u->ram.ptr + (Sint8)a, b);
} break;
case UXN_OPC(0x34): { // lda
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	push16(UXN_SRC, mempeek16_i(u->ram.dat, a));
} break;
case UXN_OPC(0x35): { // sta
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	mempoke16_i(u->ram.dat, a, b);
} break;
case UXN_OPC(0x36): { // dei
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_SRC, devpeek16(&u->dev[a >> 4], a));
} break;
case UXN_OPC(0x37): { // deo
	UXN_KEEP_SYNC;
	Uint8 a = pop8(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	devpoke16(&u->dev[a >> 4], a, b);
} break;
case UXN_OPC(0x38): { // add
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b + a);
} break;
case UXN_OPC(0x39): { // sub
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b - a);
} break;
case UXN_OPC(0x3A): { // mul
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b * a);
} break;
case UXN_OPC(0x3B): { // div
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b / a);
} break;
case UXN_OPC(0x3C): { // and
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b & a);
} break;
case UXN_OPC(0x3D): { // ora
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b | a);
} break;
case UXN_OPC(0x3E): { // eor
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b ^ a);
} break;
case UXN_OPC(0x3F): { // sft
	UXN_KEEP_SYNC;
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b >> (a & 0x000f) << ((a & 0x00f0) >> 4));
} break;
