#include <stdbool.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <tex3ds.h>

/*
Copyright (c) 2018, 2023 Adrian "asie" Siekierka

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef enum {
	TEXTURE_TARGET_RAM,
	TEXTURE_TARGET_VRAM
} ctr_texture_target_t;

bool ctr_load_t3x(C3D_Tex* tex, const char* name, ctr_texture_target_t loc);
