#include <stdio.h>
#include "../../include/uxn.h"

#ifndef CPU_ERROR_CHECKING
#define NO_STACK_CHECKS
#endif

/*
Copyright (u) 2021 Devine Lu Linvega
Copyright (u) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

static inline void   devpoke8(Device *d, Uint8 a, Uint8 b) { d->dat[a & 0xf] = b; d->talk(d, a & 0x0f, 1); }
static inline Uint8  devpeek8(Device *d, Uint8 a) { d->talk(d, a & 0x0f, 0); return d->dat[a & 0xf];  }
static inline void   devpoke16(Device *d, Uint8 a, Uint16 b) { devpoke8(d, a, b >> 8); devpoke8(d, a + 1, b); }
static inline Uint16 devpeek16(Device *d, Uint16 a) { return (devpeek8(d, a) << 8) + devpeek8(d, a + 1); }

ITCM_ARM_CODE
int
evaluxn(Uxn *u, Uint16 vec)
{
	Uint8 instr;
	u->ram.ptr = vec;
	while(u->ram.ptr) {
		instr = u->ram.dat[u->ram.ptr++];
		switch(instr) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
		case 0x00: /* op_brk */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0x80: /* op_brk + keep */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0xc0: /* op_brk + keep */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0x40: /* op_brk */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0x01: /* op_lit */
		{
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x81: /* op_lit + keep */
		{
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xc1: /* op_lit + keep */
		{
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x41: /* op_lit */
		{
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x02: /* op_nop */
		{
			(void)u;
			break;
		}
		case 0x82: /* op_nop + keep */
		{
			(void)u;
			break;
		}
		case 0xc2: /* op_nop + keep */
		{
			(void)u;
			break;
		}
		case 0x42: /* op_nop */
		{
			(void)u;
			break;
		}
		case 0x03: /* op_pop */
		{
			u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x83: /* op_pop + keep */
		{
			u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xc3: /* op_pop + keep */
		{
			u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x43: /* op_pop */
		{
			u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x04: /* op_dup */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = a;
			u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x84: /* op_dup + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = a;
			u->wst.dat[u->wst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xc4: /* op_dup + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = a;
			u->rst.dat[u->rst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x44: /* op_dup */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = a;
			u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x05: /* op_swp */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = a;
			u->wst.dat[u->wst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x85: /* op_swp + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = a;
			u->wst.dat[u->wst.ptr + 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xc5: /* op_swp + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = a;
			u->rst.dat[u->rst.ptr + 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x45: /* op_swp */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = a;
			u->rst.dat[u->rst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x06: /* op_ovr */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b;
			u->wst.dat[u->wst.ptr - 1] = a;
			u->wst.dat[u->wst.ptr] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x86: /* op_ovr + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b;
			u->wst.dat[u->wst.ptr + 1] = a;
			u->wst.dat[u->wst.ptr + 2] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 252) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 3;
			break;
		}
		case 0xc6: /* op_ovr + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b;
			u->rst.dat[u->rst.ptr + 1] = a;
			u->rst.dat[u->rst.ptr + 2] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 252) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 3;
			break;
		}
		case 0x46: /* op_ovr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b;
			u->rst.dat[u->rst.ptr - 1] = a;
			u->rst.dat[u->rst.ptr] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x07: /* op_rot */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3];
			u->wst.dat[u->wst.ptr - 3] = b;
			u->wst.dat[u->wst.ptr - 2] = a;
			u->wst.dat[u->wst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x87: /* op_rot + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3];
			u->wst.dat[u->wst.ptr] = b;
			u->wst.dat[u->wst.ptr + 1] = a;
			u->wst.dat[u->wst.ptr + 2] = c;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 252) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 3;
			break;
		}
		case 0xc7: /* op_rot + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3];
			u->rst.dat[u->rst.ptr] = b;
			u->rst.dat[u->rst.ptr + 1] = a;
			u->rst.dat[u->rst.ptr + 2] = c;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 252) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 3;
			break;
		}
		case 0x47: /* op_rot */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3];
			u->rst.dat[u->rst.ptr - 3] = b;
			u->rst.dat[u->rst.ptr - 2] = a;
			u->rst.dat[u->rst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x08: /* op_equ */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x88: /* op_equ + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xc8: /* op_equ + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x48: /* op_equ */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x09: /* op_neq */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x89: /* op_neq + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xc9: /* op_neq + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x49: /* op_neq */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x0a: /* op_gth */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x8a: /* op_gth + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xca: /* op_gth + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x4a: /* op_gth */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x0b: /* op_lth */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x8b: /* op_lth + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xcb: /* op_lth + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x4b: /* op_lth */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x0c: /* op_jmp */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x8c: /* op_jmp + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xcc: /* op_jmp + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x4c: /* op_jmp */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x0d: /* op_jnz */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			if(u->wst.dat[u->wst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x8d: /* op_jnz + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			if(u->wst.dat[u->wst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xcd: /* op_jnz + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			if(u->rst.dat[u->rst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x4d: /* op_jnz */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			if(u->rst.dat[u->rst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x0e: /* op_jsr */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x8e: /* op_jsr + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xce: /* op_jsr + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x4e: /* op_jsr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x0f: /* op_sth */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x8f: /* op_sth + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xcf: /* op_sth + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x4f: /* op_sth */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x10: /* op_pek */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x90: /* op_pek + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xd0: /* op_pek + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x50: /* op_pek */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x11: /* op_pok */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x91: /* op_pok + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd1: /* op_pok + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x51: /* op_pok */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x12: /* op_ldr */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x92: /* op_ldr + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xd2: /* op_ldr + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x52: /* op_ldr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x13: /* op_str */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x93: /* op_str + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd3: /* op_str + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x53: /* op_str */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x14: /* op_lda */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr - 2] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x94: /* op_lda + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xd4: /* op_lda + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x54: /* op_lda */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr - 2] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x15: /* op_sta */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint8 b = u->wst.dat[u->wst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x95: /* op_sta + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint8 b = u->wst.dat[u->wst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd5: /* op_sta + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint8 b = u->rst.dat[u->rst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x55: /* op_sta */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint8 b = u->rst.dat[u->rst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x16: /* op_dei */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x96: /* op_dei + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xd6: /* op_dei + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x56: /* op_dei */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x17: /* op_deo */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x97: /* op_deo + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd7: /* op_deo + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x57: /* op_deo */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x18: /* op_add */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x98: /* op_add + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xd8: /* op_add + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x58: /* op_add */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x19: /* op_sub */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x99: /* op_sub + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xd9: /* op_sub + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x59: /* op_sub */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x1a: /* op_mul */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x9a: /* op_mul + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xda: /* op_mul + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x5a: /* op_mul */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x1b: /* op_div */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x9b: /* op_div + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xdb: /* op_div + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x5b: /* op_div */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x1c: /* op_and */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x9c: /* op_and + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xdc: /* op_and + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x5c: /* op_and */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x1d: /* op_ora */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x9d: /* op_ora + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xdd: /* op_ora + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x5d: /* op_ora */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x1e: /* op_eor */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x9e: /* op_eor + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xde: /* op_eor + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x5e: /* op_eor */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x1f: /* op_sft */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x9f: /* op_sft + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xdf: /* op_sft + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x5f: /* op_sft */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x20: /* op_brk */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0xa0: /* op_brk + keep */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0xe0: /* op_brk + keep */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0x60: /* op_brk */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0x21: /* op_lit16 */
		{
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xa1: /* op_lit16 + keep */
		{
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xe1: /* op_lit16 + keep */
		{
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x61: /* op_lit16 */
		{
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x22: /* op_nop */
		{
			(void)u;
			break;
		}
		case 0xa2: /* op_nop + keep */
		{
			(void)u;
			break;
		}
		case 0xe2: /* op_nop + keep */
		{
			(void)u;
			break;
		}
		case 0x62: /* op_nop */
		{
			(void)u;
			break;
		}
		case 0x23: /* op_pop16 */
		{
			(u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xa3: /* op_pop16 + keep */
		{
			(u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xe3: /* op_pop16 + keep */
		{
			(u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x63: /* op_pop16 */
		{
			(u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x24: /* op_dup16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr - 2] = a >> 8;
			u->wst.dat[u->wst.ptr - 1] = a & 0xff;
			u->wst.dat[u->wst.ptr] = a >> 8;
			u->wst.dat[u->wst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xa4: /* op_dup16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr] = a >> 8;
			u->wst.dat[u->wst.ptr + 1] = a & 0xff;
			u->wst.dat[u->wst.ptr + 2] = a >> 8;
			u->wst.dat[u->wst.ptr + 3] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 251) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 4;
			break;
		}
		case 0xe4: /* op_dup16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr] = a >> 8;
			u->rst.dat[u->rst.ptr + 1] = a & 0xff;
			u->rst.dat[u->rst.ptr + 2] = a >> 8;
			u->rst.dat[u->rst.ptr + 3] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 251) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 4;
			break;
		}
		case 0x64: /* op_dup16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr - 2] = a >> 8;
			u->rst.dat[u->rst.ptr - 1] = a & 0xff;
			u->rst.dat[u->rst.ptr] = a >> 8;
			u->rst.dat[u->rst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x25: /* op_swp16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = a >> 8;
			u->wst.dat[u->wst.ptr - 3] = a & 0xff;
			u->wst.dat[u->wst.ptr - 2] = b >> 8;
			u->wst.dat[u->wst.ptr - 1] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xa5: /* op_swp16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = a >> 8;
			u->wst.dat[u->wst.ptr + 1] = a & 0xff;
			u->wst.dat[u->wst.ptr + 2] = b >> 8;
			u->wst.dat[u->wst.ptr + 3] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 251) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 4;
			break;
		}
		case 0xe5: /* op_swp16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = a >> 8;
			u->rst.dat[u->rst.ptr + 1] = a & 0xff;
			u->rst.dat[u->rst.ptr + 2] = b >> 8;
			u->rst.dat[u->rst.ptr + 3] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 251) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 4;
			break;
		}
		case 0x65: /* op_swp16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = a >> 8;
			u->rst.dat[u->rst.ptr - 3] = a & 0xff;
			u->rst.dat[u->rst.ptr - 2] = b >> 8;
			u->rst.dat[u->rst.ptr - 1] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x26: /* op_ovr16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b >> 8;
			u->wst.dat[u->wst.ptr - 3] = b & 0xff;
			u->wst.dat[u->wst.ptr - 2] = a >> 8;
			u->wst.dat[u->wst.ptr - 1] = a & 0xff;
			u->wst.dat[u->wst.ptr] = b >> 8;
			u->wst.dat[u->wst.ptr + 1] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xa6: /* op_ovr16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b >> 8;
			u->wst.dat[u->wst.ptr + 1] = b & 0xff;
			u->wst.dat[u->wst.ptr + 2] = a >> 8;
			u->wst.dat[u->wst.ptr + 3] = a & 0xff;
			u->wst.dat[u->wst.ptr + 4] = b >> 8;
			u->wst.dat[u->wst.ptr + 5] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 249) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 6;
			break;
		}
		case 0xe6: /* op_ovr16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b >> 8;
			u->rst.dat[u->rst.ptr + 1] = b & 0xff;
			u->rst.dat[u->rst.ptr + 2] = a >> 8;
			u->rst.dat[u->rst.ptr + 3] = a & 0xff;
			u->rst.dat[u->rst.ptr + 4] = b >> 8;
			u->rst.dat[u->rst.ptr + 5] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 249) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 6;
			break;
		}
		case 0x66: /* op_ovr16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b >> 8;
			u->rst.dat[u->rst.ptr - 3] = b & 0xff;
			u->rst.dat[u->rst.ptr - 2] = a >> 8;
			u->rst.dat[u->rst.ptr - 1] = a & 0xff;
			u->rst.dat[u->rst.ptr] = b >> 8;
			u->rst.dat[u->rst.ptr + 1] = b & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x27: /* op_rot16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8)), c = (u->wst.dat[u->wst.ptr - 5] | (u->wst.dat[u->wst.ptr - 6] << 8));
			u->wst.dat[u->wst.ptr - 6] = b >> 8;
			u->wst.dat[u->wst.ptr - 5] = b & 0xff;
			u->wst.dat[u->wst.ptr - 4] = a >> 8;
			u->wst.dat[u->wst.ptr - 3] = a & 0xff;
			u->wst.dat[u->wst.ptr - 2] = c >> 8;
			u->wst.dat[u->wst.ptr - 1] = c & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 6) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xa7: /* op_rot16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8)), c = (u->wst.dat[u->wst.ptr - 5] | (u->wst.dat[u->wst.ptr - 6] << 8));
			u->wst.dat[u->wst.ptr] = b >> 8;
			u->wst.dat[u->wst.ptr + 1] = b & 0xff;
			u->wst.dat[u->wst.ptr + 2] = a >> 8;
			u->wst.dat[u->wst.ptr + 3] = a & 0xff;
			u->wst.dat[u->wst.ptr + 4] = c >> 8;
			u->wst.dat[u->wst.ptr + 5] = c & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 6) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 249) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 6;
			break;
		}
		case 0xe7: /* op_rot16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8)), c = (u->rst.dat[u->rst.ptr - 5] | (u->rst.dat[u->rst.ptr - 6] << 8));
			u->rst.dat[u->rst.ptr] = b >> 8;
			u->rst.dat[u->rst.ptr + 1] = b & 0xff;
			u->rst.dat[u->rst.ptr + 2] = a >> 8;
			u->rst.dat[u->rst.ptr + 3] = a & 0xff;
			u->rst.dat[u->rst.ptr + 4] = c >> 8;
			u->rst.dat[u->rst.ptr + 5] = c & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 6) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 249) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 6;
			break;
		}
		case 0x67: /* op_rot16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8)), c = (u->rst.dat[u->rst.ptr - 5] | (u->rst.dat[u->rst.ptr - 6] << 8));
			u->rst.dat[u->rst.ptr - 6] = b >> 8;
			u->rst.dat[u->rst.ptr - 5] = b & 0xff;
			u->rst.dat[u->rst.ptr - 4] = a >> 8;
			u->rst.dat[u->rst.ptr - 3] = a & 0xff;
			u->rst.dat[u->rst.ptr - 2] = c >> 8;
			u->rst.dat[u->rst.ptr - 1] = c & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 6) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x28: /* op_equ16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xa8: /* op_equ16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xe8: /* op_equ16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x68: /* op_equ16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x29: /* op_neq16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xa9: /* op_neq16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xe9: /* op_neq16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x69: /* op_neq16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x2a: /* op_gth16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xaa: /* op_gth16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xea: /* op_gth16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x6a: /* op_gth16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x2b: /* op_lth16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xab: /* op_lth16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xeb: /* op_lth16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x6b: /* op_lth16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x2c: /* op_jmp16 */
		{
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xac: /* op_jmp16 + keep */
		{
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xec: /* op_jmp16 + keep */
		{
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x6c: /* op_jmp16 */
		{
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x2d: /* op_jnz16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			if(u->wst.dat[u->wst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xad: /* op_jnz16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			if(u->wst.dat[u->wst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xed: /* op_jnz16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			if(u->rst.dat[u->rst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x6d: /* op_jnz16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			if(u->rst.dat[u->rst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x2e: /* op_jsr16 */
		{
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xae: /* op_jsr16 + keep */
		{
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xee: /* op_jsr16 + keep */
		{
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x6e: /* op_jsr16 */
		{
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x2f: /* op_sth16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr] = a >> 8;
			u->rst.dat[u->rst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xaf: /* op_sth16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr] = a >> 8;
			u->rst.dat[u->rst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xef: /* op_sth16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr] = a >> 8;
			u->wst.dat[u->wst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x6f: /* op_sth16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr] = a >> 8;
			u->wst.dat[u->wst.ptr + 1] = a & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x30: /* op_pek16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xb0: /* op_pek16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xf0: /* op_pek16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x70: /* op_pek16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x31: /* op_pok16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xb1: /* op_pok16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf1: /* op_pok16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x71: /* op_pok16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x32: /* op_ldr16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xb2: /* op_ldr16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xf2: /* op_ldr16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x72: /* op_ldr16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x33: /* op_str16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xb3: /* op_str16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf3: /* op_str16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x73: /* op_str16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x34: /* op_lda16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr - 2] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xb4: /* op_lda16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xf4: /* op_lda16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x74: /* op_lda16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr - 2] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x35: /* op_sta16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 4;
			break;
		}
		case 0xb5: /* op_sta16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf5: /* op_sta16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x75: /* op_sta16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 4;
			break;
		}
		case 0x36: /* op_dei16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
			u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xb6: /* op_dei16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a);
			u->wst.dat[u->wst.ptr + 1] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xf6: /* op_dei16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a);
			u->rst.dat[u->rst.ptr + 1] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x76: /* op_dei16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
			u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x37: /* op_deo16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0xb7: /* op_deo16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf7: /* op_deo16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x77: /* op_deo16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x38: /* op_add16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b + a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xb8: /* op_add16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b + a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xf8: /* op_add16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b + a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x78: /* op_add16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b + a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x39: /* op_sub16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b - a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xb9: /* op_sub16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b - a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xf9: /* op_sub16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b - a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x79: /* op_sub16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b - a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x3a: /* op_mul16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b * a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xba: /* op_mul16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b * a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xfa: /* op_mul16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b * a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x7a: /* op_mul16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b * a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x3b: /* op_div16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b / a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xbb: /* op_div16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b / a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xfb: /* op_div16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b / a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x7b: /* op_div16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b / a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x3c: /* op_and16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr - 4] = d & b;
			u->wst.dat[u->wst.ptr - 3] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xbc: /* op_and16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr] = d & b;
			u->wst.dat[u->wst.ptr + 1] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xfc: /* op_and16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr] = d & b;
			u->rst.dat[u->rst.ptr + 1] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x7c: /* op_and16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr - 4] = d & b;
			u->rst.dat[u->rst.ptr - 3] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x3d: /* op_ora16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr - 4] = d | b;
			u->wst.dat[u->wst.ptr - 3] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xbd: /* op_ora16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr] = d | b;
			u->wst.dat[u->wst.ptr + 1] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xfd: /* op_ora16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr] = d | b;
			u->rst.dat[u->rst.ptr + 1] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x7d: /* op_ora16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr - 4] = d | b;
			u->rst.dat[u->rst.ptr - 3] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x3e: /* op_eor16 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr - 4] = d ^ b;
			u->wst.dat[u->wst.ptr - 3] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xbe: /* op_eor16 + keep */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr] = d ^ b;
			u->wst.dat[u->wst.ptr + 1] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xfe: /* op_eor16 + keep */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr] = d ^ b;
			u->rst.dat[u->rst.ptr + 1] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x7e: /* op_eor16 */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr - 4] = d ^ b;
			u->rst.dat[u->rst.ptr - 3] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x3f: /* op_sft16 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0xbf: /* op_sft16 + keep */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xff: /* op_sft16 + keep */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x7f: /* op_sft16 */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
#pragma GCC diagnostic pop
		}
	}
	return 1;
#ifndef NO_STACK_CHECKS
error:
	dprintf("Halted: %s-stack %sflow#%04x, at 0x%04x\n",
		u->wst.error ? "Working" : "Return",
		((u->wst.error | u->rst.error) & 2) ? "over" : "under",
		instr,
		u->ram.ptr);
	return 0;
#endif
}

int
bootuxn(Uxn *u)
{
	memset(u, 0, sizeof(*u));
	u->ram.dat = malloc(65536);
	memset(u->ram.dat, 0, 65536);
	return 1;
}

int
loaduxn(Uxn *u, char *filepath)
{
	FILE *f;
	if(!(f = fopen(filepath, "rb"))) {
		dprintf("Halted: Missing input rom.\n");
		return 0;
	}
	fread(u->ram.dat + PAGE_PROGRAM, sizeof(u->ram.dat) - PAGE_PROGRAM, 1, f);
	dprintf("Uxn loaded[%s].\n", filepath);
	return 1;
}

Device *
portuxn(Uxn *u, Uint8 id, char *name, void (*talkfn)(Device *d, Uint8 b0, Uint8 w))
{
	Device *d = &u->dev[id];
	d->addr = id * 0x10;
	d->u = u;
	d->mem = u->ram.dat;
	d->talk = talkfn;
	dprintf("Device added #%02x: %s, at 0x%04x \n", id, name, d->addr);
	return d;
}
