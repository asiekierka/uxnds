#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <tex3ds.h>

#include "../util.h"
#include "ctr_util.h"
#include "ctr_keyboard.h"

/*
Copyright (c) 2018, 2023 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef uint8_t keycode_t;

typedef struct {
	u16 x, y, w, h;
	keycode_t keycode, keycode_shifted;
	uint8_t flags;
} key_area_t;

#define KEYS_HELD_MAX_COUNT 6
#define KEY_SIZE 10
#define KEY_YOFS (240 - (KEY_SIZE*2)*5)
#define KEY_POS(ix, iy, iw) ((int)(ix*2)*(KEY_SIZE)+1), (KEY_YOFS)+((int)(iy*2)*(KEY_SIZE)+1), ((int)(iw*2)*(KEY_SIZE)-2), ((KEY_SIZE)*2-2)

#define KF_MODIFIER		0x01
#define KF_HELD			0x02

#define KF_RUNTIME_HELD			0x40
#define KF_RUNTIME_HOVERED		0x80
#define KF_RUNTIME_ANY_HIGHLIGHT	0xC0
#define KF_RUNTIME_ALL			0xC0

static key_area_t keyboard_areas[] = {
	{ KEY_POS(0, 0, 1),	K_ESC,	K_ESC,	0 },
	{ KEY_POS(1.5, 0, 1),	'1',	'!',	0 },
	{ KEY_POS(2.5, 0, 1),	'2',	'@',	0 },
	{ KEY_POS(3.5, 0, 1),	'3',	'#',	0 },
	{ KEY_POS(4.5, 0, 1),	'4',	'$',	0 },
	{ KEY_POS(5.5, 0, 1),	'5',	'%',	0 },
	{ KEY_POS(6.5, 0, 1),	'6',	'^',	0 },
	{ KEY_POS(7.5, 0, 1),	'7',	'&',	0 },
	{ KEY_POS(8.5, 0, 1),	'8',	'*',	0 },
	{ KEY_POS(9.5, 0, 1),	'9',	'(',	0 },
	{ KEY_POS(10.5, 0, 1),	'0',	')',	0 },
	{ KEY_POS(11.5, 0, 1),	'-',	'_',	0 },
	{ KEY_POS(12.5, 0, 1),	'=',	'+',	0 },
	{ KEY_POS(13.5, 0, 1),	'\\',	'|',	0 },
	{ KEY_POS(14.5, 0, 1),	'`',	'~',	0 },

	{ KEY_POS(0, 1, 2),	K_TAB,	K_TAB,	0 },
	{ KEY_POS(2, 1, 1),	'q',	'Q',	0 },
	{ KEY_POS(3, 1, 1),	'w',	'W',	0 },
	{ KEY_POS(4, 1, 1),	'e',	'E',	0 },
	{ KEY_POS(5, 1, 1),	'r',	'R',	0 },
	{ KEY_POS(6, 1, 1),	't',	'T',	0 },
	{ KEY_POS(7, 1, 1),	'y',	'Y',	0 },
	{ KEY_POS(8, 1, 1),	'u',	'U',	0 },
	{ KEY_POS(9, 1, 1),	'i',	'I',	0 },
	{ KEY_POS(10, 1, 1),	'o',	'O',	0 },
	{ KEY_POS(11, 1, 1),	'p',	'P',	0 },
	{ KEY_POS(12, 1, 1),	'[',	'{',	0 },
	{ KEY_POS(13, 1, 1),	']',	'}',	0 },
	{ KEY_POS(14, 1, 2),	K_BKSP,	K_BKSP,	0 },

	{ KEY_POS(0, 2, 2.5),	K_CTRL,	K_CTRL,	KF_MODIFIER },
	{ KEY_POS(2.5, 2, 1),	'a',	'A',	0 },
	{ KEY_POS(3.5, 2, 1),	's',	'S',	0 },
	{ KEY_POS(4.5, 2, 1),	'd',	'D',	0 },
	{ KEY_POS(5.5, 2, 1),	'f',	'F',	0 },
	{ KEY_POS(6.5, 2, 1),	'g',	'G',	0 },
	{ KEY_POS(7.5, 2, 1),	'h',	'H',	0 },
	{ KEY_POS(8.5, 2, 1),	'j',	'J',	0 },
	{ KEY_POS(9.5, 2, 1),	'k',	'K',	0 },
	{ KEY_POS(10.5, 2, 1),	'l',	'L',	0 },
	{ KEY_POS(11.5, 2, 1),	';',	':',	0 },
	{ KEY_POS(12.5, 2, 1),	'\'',	'"',	0 },
	{ KEY_POS(13.5, 2, 2.5),	K_RET,	K_RET,	0 },

	{ KEY_POS(0, 3, 3),	K_SHIFT,	K_SHIFT,	KF_MODIFIER },
	{ KEY_POS(3, 3, 1),	'z',	'Z',	0 },
	{ KEY_POS(4, 3, 1),	'x',	'X',	0 },
	{ KEY_POS(5, 3, 1),	'c',	'C',	0 },
	{ KEY_POS(6, 3, 1),	'v',	'V',	0 },
	{ KEY_POS(7, 3, 1),	'b',	'B',	0 },
	{ KEY_POS(8, 3, 1),	'n',	'N',	0 },
	{ KEY_POS(9, 3, 1),	'm',	'M',	0 },
	{ KEY_POS(10, 3, 1),	',',	'<',	0 },
	{ KEY_POS(11, 3, 1),	'.',	'>',	0 },
	{ KEY_POS(12, 3, 1),	'/',	'?',	0 },
	{ KEY_POS(13, 3, 3),	K_SHIFT,	K_SHIFT,	KF_MODIFIER },

	{ KEY_POS(0, 4, 1),	K_SYSTEM,	K_SYSTEM,	0 },
	{ KEY_POS(1.5, 4, 2),	K_ALT,	K_ALT,	KF_MODIFIER },
	{ KEY_POS(3.5, 4, 8),	' ',	' ', 0 },
	{ KEY_POS(11.5, 4, 2),	K_ALT,	K_ALT,	KF_MODIFIER },
	{ KEY_POS(13.5, 4, 1),	K_HOME,	K_HOME,	KF_HELD },
	{ KEY_POS(15, 4, 1),	K_DEL,	K_DEL,	0 }
};
#define KEYBOARD_AREAS_COUNT (sizeof(keyboard_areas) / sizeof(key_area_t))

#define KR_REDRAW	0x01
#define KR_CLEAR_HELD	0x02
#define KR_CAPS_LOCK	0x04

static keycode_t keys_held[KEYS_HELD_MAX_COUNT];
static uint8_t keys_held_count;
static uint8_t keyboard_reqs;
static C3D_Tex gfx_keyboard_tex;

static bool key_area_touched(touchPosition* pos, const key_area_t* area) {
	return pos->px >= area->x && pos->py >= area->y
		&& pos->px < (area->x + area->w)
		&& pos->py < (area->y + area->h);
}

bool keyboard_is_held(int key) {
	for (int i = 0; i < keys_held_count; i++) {
		if (keys_held[i] == key)
			return true;
	}
	return false;
}

static void keyboard_unhold(int key) {
	for (int i = 0; i < keys_held_count; i++) {
		if (keys_held[i] == key) {
			for (int j = i; j < keys_held_count - 1; j++) {
				keys_held[j] = keys_held[j + 1];
			}
			i--;
			keys_held_count--;
		}
	}
}

static bool keyboard_is_shifted(void) {
	return (keyboard_reqs & KR_CAPS_LOCK) || keyboard_is_held(K_SHIFT);
}

bool keyboard_init(void) {
	if (!ctr_load_t3x(&gfx_keyboard_tex, "romfs:/gfx_keyboard.t3x", TEXTURE_TARGET_VRAM)) {
		return false;
	}

	keyboard_clear();
	return true;
}

void keyboard_exit(void) {
	C3D_TexDelete(&gfx_keyboard_tex);
}

void keyboard_draw(void) {
	bool shifted = keyboard_is_shifted();

	C2D_Image kbd_image;
	Tex3DS_SubTexture kbd_subtex;

	kbd_subtex.width = 320;
	kbd_subtex.height = 100;
	kbd_subtex.left = 0.0f;
	kbd_subtex.top = 1.0f - ((shifted ? 100.0f : 200.0f) / 256.0f);
	kbd_subtex.right = 320.0f / 512.0f;
	kbd_subtex.bottom = kbd_subtex.top + (100.0f / 256.0f);
	kbd_image.tex = &gfx_keyboard_tex;
	kbd_image.subtex = &kbd_subtex;

	C2D_DrawImageAt(kbd_image, 0, KEY_YOFS, 0.0f, NULL, 1.0f, 1.0f);

	for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
		const key_area_t* area = &keyboard_areas[i];
		keycode_t keycode = shifted ? area->keycode_shifted : area->keycode;
		if (keycode == K_SHIFT && (keyboard_reqs & KR_CAPS_LOCK)) {
			C2D_DrawRectSolid(area->x, area->y, 0.0f, area->w, area->h, 0x70000000);
		} else if (area->flags & KF_RUNTIME_ANY_HIGHLIGHT) {
			C2D_DrawRectSolid(area->x, area->y, 1.0f, area->w, area->h, 0x38000000);
		}
	}

	keyboard_reqs &= ~KR_REDRAW;
}

bool keyboard_needs_draw(void) {
	return keyboard_reqs & KR_REDRAW;
}

void keyboard_clear(void) {
	for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
		key_area_t* area = &keyboard_areas[i];
		area->flags &= ~KF_RUNTIME_ALL;
	}
	keys_held_count = 0;
	keyboard_reqs = KR_REDRAW;
}

int keyboard_update(void) {
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();
	u32 kUp = hidKeysUp();
	touchPosition pos;
	int retval = -1;

	if (keyboard_reqs & KR_CLEAR_HELD) {
		// no keypress ongoing - pop all held keys
		if (keys_held_count > 0) {
			keys_held_count = 0;
			keyboard_reqs |= KR_REDRAW;
		}
		for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
			key_area_t* area = &keyboard_areas[i];
			area->flags &= ~KF_RUNTIME_HELD;
		}
		keyboard_reqs &= ~KR_CLEAR_HELD;
	}

	bool touching = ((kDown | kHeld) & KEY_TOUCH) != 0;
	bool shifted = keyboard_is_shifted();
	if (touching) {
		hidTouchRead(&pos);

		// handle key hovering
		for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
			key_area_t* area = &keyboard_areas[i];
			if (key_area_touched(&pos, area)) {
				if (!(area->flags & KF_RUNTIME_HOVERED))
					keyboard_reqs |= KR_REDRAW;
				keycode_t keycode = shifted ? area->keycode_shifted : area->keycode;

				area->flags |= KF_RUNTIME_HOVERED;
				if (area->flags & KF_HELD) {
					retval = keycode;
				}
			} else {
				area->flags &= ~KF_RUNTIME_HOVERED;
			}
		}
	} else if (kUp & KEY_TOUCH) {
		// actually press key
		for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
			key_area_t* area = &keyboard_areas[i];
			if (area->flags & KF_RUNTIME_HOVERED) {
				keyboard_reqs |= KR_REDRAW;
				area->flags &= ~KF_RUNTIME_HOVERED;
				keycode_t keycode = shifted ? area->keycode_shifted : area->keycode;

				if ((area->flags & KF_RUNTIME_HELD) && keycode == K_SHIFT) {
					if (keyboard_reqs & KR_CAPS_LOCK) {
						keyboard_reqs &= ~KR_CAPS_LOCK;
						keyboard_reqs |= KR_CLEAR_HELD;
					} else {
						keyboard_reqs |= KR_CAPS_LOCK;
					}
				} else if (area->flags & KF_MODIFIER) {
					if (area->flags & KF_RUNTIME_HELD) {
						keyboard_unhold(keycode);
						area->flags &= ~KF_RUNTIME_HELD;
					} else if (keys_held_count < KEYS_HELD_MAX_COUNT && !keyboard_is_held(keycode)) {
						keys_held[keys_held_count++] = keycode;
						area->flags |= KF_RUNTIME_HELD;
					}
				} else {
					if (!(area->flags & KF_HELD)) {
						retval = keycode;
					}
					keyboard_reqs |= KR_CLEAR_HELD;
				}
			}
		}
	}

	return retval;
}
