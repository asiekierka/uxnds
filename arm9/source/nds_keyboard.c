#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <nds.h>

#include "util.h"
#include "gfx_keyboard.h"
#include "nds_keyboard.h"

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
#define KEY_SIZE 8
#define KEY_YOFS (192 - (KEY_SIZE*2)*5)
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

#define KR_CLEAR_HELD	0x02
#define KR_CAPS_LOCK	0x04

#define KBD_MAP_BASE 20
#define KBD_TILE_BASE 2

static keycode_t keys_held[KEYS_HELD_MAX_COUNT];
static uint8_t keys_held_count;
static uint8_t keyboard_reqs;

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

static void keyboard_update_map(void) {
	bool shifted = keyboard_is_shifted();
	uint32_t *src = (uint32_t*) (gfx_keyboardMap + (shifted ? 32*10 : 0));
	uint32_t *dst = (uint32_t*) (BG_MAP_RAM_SUB(KBD_MAP_BASE) + (32*14));

	for (int i = 0; i < 32*10/2; i++, src++, dst++) {
		*dst = (*dst & 0xF000F000) | (*src & 0x0FFF0FFF);
	}
}

static void keyboard_update_palette(key_area_t *area) {
	uint16_t palette_idx = 0;
	bool shifted = keyboard_is_shifted();
	keycode_t keycode = shifted ? area->keycode_shifted : area->keycode;

	if (keycode == K_SHIFT && (keyboard_reqs & KR_CAPS_LOCK)) {
		palette_idx = 2 << 12;
	} else if (area->flags & KF_RUNTIME_ANY_HIGHLIGHT) {
		palette_idx = 1 << 12;
	}

	for (int ty = (area->y >> 3); ty <= ((area->y + area->h - 1) >> 3); ty++) {
		for (int tx = (area->x >> 3); tx <= ((area->x + area->w - 1) >> 3); tx++) {
			uint16_t tile = BG_MAP_RAM_SUB(KBD_MAP_BASE)[(ty << 5) | tx];
			BG_MAP_RAM_SUB(KBD_MAP_BASE)[(ty << 5) | tx] = (tile & 0xFFF) | palette_idx;
		}
	}
}

static void keyboard_update_palettes(int target_keycode) {
	bool shifted = keyboard_is_shifted();
	for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
		key_area_t* area = &keyboard_areas[i];
		keycode_t keycode = shifted ? area->keycode_shifted : area->keycode;
		if (target_keycode == -1 || target_keycode == keycode) {
			keyboard_update_palette(area);
		}
	}
}

__attribute__((optimize("-Os")))
static void keyboard_set_palette_colors(uint16_t *pal, uint8_t mul /* 0-15 */) {
	for (int i = 0; i < 14; i++) {
		uint16_t col = gfx_keyboardPal[i];
		pal[i] =
			((col & 0x1F) * mul / 15)
			| ((((col >> 5) & 0x1F) * mul / 15) << 5)
			| ((((col >> 10) & 0x1F) * mul / 15) << 10)
			| (col & 0x8000);
	}
}

__attribute__((optimize("-Os")))
bool keyboard_init(void) {
	decompress(gfx_keyboardTiles, BG_TILE_RAM_SUB(KBD_TILE_BASE), LZ77Vram);
	memset(BG_MAP_RAM_SUB(KBD_MAP_BASE), 0, 2*(32*14));

	memcpy(BG_PALETTE_SUB, gfx_keyboardPal, 2*14);
	keyboard_set_palette_colors(BG_PALETTE_SUB + 16, 12);
	keyboard_set_palette_colors(BG_PALETTE_SUB + 32, 10);

	keyboard_clear();
	swiWaitForVBlank();
	REG_BG3CNT_SUB = BG_32x32 | BG_COLOR_16 | BG_PRIORITY_3 | BG_TILE_BASE(KBD_TILE_BASE) | BG_MAP_BASE(KBD_MAP_BASE);
	REG_BG3HOFS_SUB = 0;
	REG_BG3VOFS_SUB = 0;
	videoBgEnableSub(3);
	return true;
}

void keyboard_exit(void) {

}

void keyboard_clear(void) {
	for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
		key_area_t* area = &keyboard_areas[i];
		area->flags &= ~KF_RUNTIME_ALL;
	}
	memcpy(BG_MAP_RAM_SUB(KBD_MAP_BASE) + (32*14), gfx_keyboardMap, 2*(32*10));
	keys_held_count = 0;
}

int keyboard_update(void) {
	u32 kDown = keysDown();
	u32 kHeld = keysHeld();
	u32 kUp = keysUp();
	touchPosition pos;
	int retval = -1;

	if (keyboard_reqs & KR_CLEAR_HELD) {
		bool was_shifted = keyboard_is_shifted();
		// no keypress ongoing - pop all held keys
		keys_held_count = 0;
		for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
			key_area_t* area = &keyboard_areas[i];
			if (area->flags & KF_RUNTIME_HELD) {
				area->flags &= ~KF_RUNTIME_HELD;
				keyboard_update_palette(area);
			}
		}
		keyboard_reqs &= ~KR_CLEAR_HELD;
		if (was_shifted) {
			keyboard_update_map();
		}
	}

	bool touching = ((kDown | kHeld) & KEY_TOUCH) != 0;
	bool shifted = keyboard_is_shifted();
	if (touching) {
		touchRead(&pos);

		// handle key hovering
		for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
			key_area_t* area = &keyboard_areas[i];
			if (key_area_touched(&pos, area)) {
				keycode_t keycode = shifted ? area->keycode_shifted : area->keycode;

				if (!(area->flags & KF_RUNTIME_HOVERED)) {
					area->flags |= KF_RUNTIME_HOVERED;
					keyboard_update_palette(area);
				}
				if (area->flags & KF_HELD) {
					retval = keycode;
				}
			} else {
				if (area->flags & KF_RUNTIME_HOVERED) {
					area->flags &= ~KF_RUNTIME_HOVERED;
					keyboard_update_palette(area);
				}
			}
		}
	} else if (kUp & KEY_TOUCH) {
		// actually press key
		for (int i = 0; i < KEYBOARD_AREAS_COUNT; i++) {
			key_area_t* area = &keyboard_areas[i];
			if (area->flags & KF_RUNTIME_HOVERED) {
				area->flags &= ~KF_RUNTIME_HOVERED;
				keycode_t keycode = shifted ? area->keycode_shifted : area->keycode;

				if ((keyboard_reqs & KR_CAPS_LOCK) && area->keycode_shifted == area->keycode) {
					keyboard_reqs &= ~KR_CAPS_LOCK;
					keyboard_reqs |= KR_CLEAR_HELD;
				} else if ((area->flags & KF_RUNTIME_HELD) && keycode == K_SHIFT) {
					keyboard_reqs |= KR_CAPS_LOCK;
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

				if (keycode == K_SHIFT) {
					keyboard_update_map();
				}
				if (area->flags & KF_MODIFIER) {
					keyboard_update_palettes(keycode);
				} else {
					keyboard_update_palette(area);
				}
			}
		}
	}

	return retval;
}
