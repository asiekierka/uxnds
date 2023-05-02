#include <nds.h>

/*
Copyright (c) 2018, 2023 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define K_BKSP 8
#define K_TAB 9
#define K_RET 13
#define K_ESC 27
#define K_DEL 127
#define K_CTRL 128
#define K_ALT 129
#define K_SHIFT 130
#define K_HOME 131
#define K_SYSTEM 132

bool keyboard_is_held(int key);
bool keyboard_init(void);
void keyboard_exit(void);
void keyboard_clear(void);
// Returns pressed keycode, or -1 if no key pressed.
int keyboard_update(void);
