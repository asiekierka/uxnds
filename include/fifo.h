/*
Copyright (c) 2021 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#include <nds.h>

#define UXNDS_AUDIO_BUFFER_SIZE 512
#define UXNDS_FIFO_CHANNEL FIFO_USER_01
#define UXNDS_FIFO_CMD_SET_RATE	0x10000000
#define UXNDS_FIFO_CMD_SET_ADDR	0x20000000
#define UXNDS_FIFO_CMD_APU0	0x80000000
#define UXNDS_FIFO_CMD_APU1	0x90000000
#define UXNDS_FIFO_CMD_APU2	0xA0000000
#define UXNDS_FIFO_CMD_APU3	0xB0000000
#define UXNDS_FIFO_CMD_MASK	0xF0000000

