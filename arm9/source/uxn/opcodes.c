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
	pop8(UXN_SRC);
} break;
case UXN_OPC(0x04): { // dup
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, a);
	push8(UXN_SRC, a);
} break;
case UXN_OPC(0x05): { // swp
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, a);
	push8(UXN_SRC, b);
} break;
case UXN_OPC(0x06): { // ovr
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b);
	push8(UXN_SRC, a);
	push8(UXN_SRC, b);
} break;
case UXN_OPC(0x07): { // rot
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	Uint8 c = pop8(UXN_SRC);
	push8(UXN_SRC, b);
	push8(UXN_SRC, a);
	push8(UXN_SRC, c);
} break;
case UXN_OPC(0x08): { // equ
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b == a);
} break;
case UXN_OPC(0x09): { // neg
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b != a);
} break;
case UXN_OPC(0x0A): { // gth
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b > a);
} break;
case UXN_OPC(0x0B): { // lth
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b < a);
} break;
case UXN_OPC(0x0C): { // jmp
	Uint8 a = pop8(UXN_SRC);
	u->ram.ptr += (Sint8)a;
} break;
case UXN_OPC(0x0D): { // jnz
	Uint8 a = pop8(UXN_SRC);
	if (pop8(UXN_SRC))
		u->ram.ptr += (Sint8)a;
} break;
case UXN_OPC(0x0E): { // jsr
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_DST, u->ram.ptr);
	u->ram.ptr += (Sint8)a;
} break;
case UXN_OPC(0x0F): { // sth
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_DST, a);
} break;
case UXN_OPC(0x10): { // pek
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, mempeek8(u->ram.dat, a));
} break;
case UXN_OPC(0x11): { // pok
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	mempoke8(u->ram.dat, a, b);
} break;
case UXN_OPC(0x12): { // ldr
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a));
} break;
case UXN_OPC(0x13): { // str
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
} break;
case UXN_OPC(0x14): { // lda
	Uint16 a = pop16(UXN_SRC);
	push8(UXN_SRC, mempeek8(u->ram.dat, a));
} break;
case UXN_OPC(0x15): { // sta
	Uint16 a = pop16(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	mempoke8(u->ram.dat, a, b);
} break;
case UXN_OPC(0x16): { // dei
	Uint8 a = pop8(UXN_SRC);
	push8(UXN_SRC, devpeek8(&u->dev[a >> 4], a));
} break;
case UXN_OPC(0x17): { // deo
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	devpoke8(&u->dev[a >> 4], a, b);
} break;
case UXN_OPC(0x18): { // add
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b + a);
} break;
case UXN_OPC(0x19): { // sub
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b - a);
} break;
case UXN_OPC(0x1A): { // mul
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b * a);
} break;
case UXN_OPC(0x1B): { // div
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b / a);
} break;
case UXN_OPC(0x1C): { // and
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b & a);
} break;
case UXN_OPC(0x1D): { // ora
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b | a);
} break;
case UXN_OPC(0x1E): { // eor
	Uint8 a = pop8(UXN_SRC);
	Uint8 b = pop8(UXN_SRC);
	push8(UXN_SRC, b ^ a);
} break;
case UXN_OPC(0x1F): { // sft
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
	pop16(UXN_SRC);
} break;
case UXN_OPC(0x24): { // dup
	Uint16 a = pop16(UXN_SRC);
	push16(UXN_SRC, a);
	push16(UXN_SRC, a);
} break;
case UXN_OPC(0x25): { // swp
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, a);
	push16(UXN_SRC, b);
} break;
case UXN_OPC(0x26): { // ovr
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b);
	push16(UXN_SRC, a);
	push16(UXN_SRC, b);
} break;
case UXN_OPC(0x27): { // rot
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	Uint16 c = pop16(UXN_SRC);
	push16(UXN_SRC, b);
	push16(UXN_SRC, a);
	push16(UXN_SRC, c);
} break;
case UXN_OPC(0x28): { // equ
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b == a);
} break;
case UXN_OPC(0x29): { // neg
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b != a);
} break;
case UXN_OPC(0x2A): { // gth
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b > a);
} break;
case UXN_OPC(0x2B): { // lth
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push8(UXN_SRC, b < a);
} break;
case UXN_OPC(0x2C): { // jmp
	u->ram.ptr = pop16(UXN_SRC);
} break;
case UXN_OPC(0x2D): { // jnz
	Uint16 a = pop16(UXN_SRC);
	if (pop8(UXN_SRC))
		u->ram.ptr = a;
} break;
case UXN_OPC(0x2E): { // jsr
	push16(UXN_DST, u->ram.ptr);
	u->ram.ptr = pop16(UXN_SRC);
} break;
case UXN_OPC(0x2F): { // sth
	Uint16 a = pop16(UXN_SRC);
	push16(UXN_DST, a);
} break;
case UXN_OPC(0x30): { // pek
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_SRC, mempeek16_i(u->ram.dat, a));
} break;
case UXN_OPC(0x31): { // pok
	Uint8 a = pop8(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	mempoke16_i(u->ram.dat, a, b);
} break;
case UXN_OPC(0x32): { // ldr
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_SRC, mempeek16_i(u->ram.dat, u->ram.ptr + (Sint8)a));
} break;
case UXN_OPC(0x33): { // str
	Uint8 a = pop8(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	mempoke16_i(u->ram.dat, u->ram.ptr + (Sint8)a, b);
} break;
case UXN_OPC(0x34): { // lda
	Uint16 a = pop16(UXN_SRC);
	push16(UXN_SRC, mempeek16_i(u->ram.dat, a));
} break;
case UXN_OPC(0x35): { // sta
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	mempoke16_i(u->ram.dat, a, b);
} break;
case UXN_OPC(0x36): { // dei
	Uint8 a = pop8(UXN_SRC);
	push16(UXN_SRC, devpeek16(&u->dev[a >> 4], a));
} break;
case UXN_OPC(0x37): { // deo
	Uint8 a = pop8(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	devpoke16(&u->dev[a >> 4], a, b);
} break;
case UXN_OPC(0x38): { // add
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b + a);
} break;
case UXN_OPC(0x39): { // sub
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b - a);
} break;
case UXN_OPC(0x3A): { // mul
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b * a);
} break;
case UXN_OPC(0x3B): { // div
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b / a);
} break;
case UXN_OPC(0x3C): { // and
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b & a);
} break;
case UXN_OPC(0x3D): { // ora
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b | a);
} break;
case UXN_OPC(0x3E): { // eor
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b ^ a);
} break;
case UXN_OPC(0x3F): { // sft
	Uint16 a = pop16(UXN_SRC);
	Uint16 b = pop16(UXN_SRC);
	push16(UXN_SRC, b >> (a & 0x000f) << ((a & 0x00f0) >> 4));
} break;
