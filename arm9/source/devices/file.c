#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../../include/uxn.h"
#include "file.h"

/*
Copyright (c) 2021 Devine Lu Linvega
Copyright (c) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef struct {
	FILE *f;
	DIR *dir;
	char current_filename[MAX_PATH];
	struct dirent *de;
	enum { IDLE,
		FILE_READ,
		FILE_WRITE,
		DIR_READ } state;
} UxnFile;

static UxnFile uxn_file[POLYFILEY];

static void
reset(UxnFile *c)
{
	if(c->f != NULL) {
		fclose(c->f);
		c->f = NULL;
	}
	if(c->dir != NULL) {
		closedir(c->dir);
		c->dir = NULL;
	}
	c->de = NULL;
	c->state = IDLE;
}

static Uint16
get_entry(char *p, Uint16 len, const char *pathname, const char *basename, int fail_nonzero)
{
	struct stat st;
	if(len < strlen(basename) + 8)
		return 0;
	if(stat(pathname, &st))
		return fail_nonzero ? siprintf(p, "!!!! %s\n", basename) : 0;
	else if(S_ISDIR(st.st_mode))
		return siprintf(p, "---- %s/\n", basename);
	else if(st.st_size < 0x10000)
		return siprintf(p, "%04x %s\n", (unsigned int)st.st_size, basename);
	else
		return siprintf(p, "???? %s\n", basename);
}

static Uint16
file_read_dir(UxnFile *c, char *dest, Uint16 len)
{
	static char pathname[MAX_PATH + 256];
	char *p = dest;
	if(c->de == NULL) c->de = readdir(c->dir);
	for(; c->de != NULL; c->de = readdir(c->dir)) {
		Uint16 n;
		if(c->de->d_name[0] == '.' && c->de->d_name[1] == '\0')
			continue;
		if(strlen(c->current_filename) + 1 + strlen(c->de->d_name) < sizeof(pathname))
			siprintf(pathname, "%s/%s", c->current_filename, c->de->d_name);
		else
			pathname[0] = '\0';
		n = get_entry(p, len, pathname, c->de->d_name, 1);
		if(!n) break;
		p += n;
		len -= n;
	}
	return p - dest;
}

static Uint16
file_init(UxnFile *c, char *filename, size_t max_len)
{
	char *p = c->current_filename;
	size_t len = sizeof(c->current_filename);
	reset(c);
	if(len > max_len) len = max_len;
	while(len) {
		if((*p++ = *filename++) == '\0')
			return 0;
		len--;
	}
	c->current_filename[0] = '\0';
	return 0;
}

static Uint16
file_read(UxnFile *c, void *dest, Uint16 len)
{
	if(c->state != FILE_READ && c->state != DIR_READ) {
		reset(c);
		if((c->dir = opendir(c->current_filename)) != NULL)
			c->state = DIR_READ;
		else if((c->f = fopen(c->current_filename, "rb")) != NULL)
			c->state = FILE_READ;
	}
	if(c->state == FILE_READ)
		return fread(dest, 1, len, c->f);
	if(c->state == DIR_READ)
		return file_read_dir(c, dest, len);
	return 0;
}

static Uint16
file_write(UxnFile *c, void *src, Uint16 len, Uint8 flags)
{
	Uint16 ret = 0;
	if(c->state != FILE_WRITE) {
		reset(c);
		if((c->f = fopen(c->current_filename, (flags & 0x01) ? "ab" : "wb")) != NULL)
			c->state = FILE_WRITE;
	}
	if(c->state == FILE_WRITE) {
		if((ret = fwrite(src, 1, len, c->f)) > 0 && fflush(c->f) != 0)
			ret = 0;
	}
	return ret;
}

static Uint16
file_stat(UxnFile *c, void *dest, Uint16 len)
{
	char *basename = strrchr(c->current_filename, '/');
	if(basename != NULL)
		basename++;
	else
		basename = c->current_filename;
	return get_entry(dest, len, c->current_filename, basename, 0);
}

static Uint16
file_delete(UxnFile *c)
{
	return unlink(c->current_filename);
}

/* IO */

void
file_deo(Uint8 id, Uint8 *ram, Uint8 *d, Uint8 port)
{
	UxnFile *c = &uxn_file[id];
	Uint16 addr, len, res;
	switch(port) {
	case 0x5:
		PEKDEV(addr, 0x4);
		PEKDEV(len, 0xa);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_stat(c, &ram[addr], len);
		POKDEV(0x2, res);
		break;
	case 0x6:
		res = file_delete(c);
		POKDEV(0x2, res);
		break;
	case 0x9:
		PEKDEV(addr, 0x8);
		res = file_init(c, (char *)&ram[addr], 0x10000 - addr);
		POKDEV(0x2, res);
		break;
	case 0xd:
		PEKDEV(addr, 0xc);
		PEKDEV(len, 0xa);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_read(c, &ram[addr], len);
		POKDEV(0x2, res);
		break;
	case 0xf:
		PEKDEV(addr, 0xe);
		PEKDEV(len, 0xa);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_write(c, &ram[addr], len, d[0x7]);
		POKDEV(0x2, res);
		break;
	}
}

Uint8
file_dei(Uint8 id, Uint8 *d, Uint8 port)
{
	UxnFile *c = &uxn_file[id];
	Uint16 res;
	switch(port) {
	case 0xc:
	case 0xd:
		res = file_read(c, &d[port], 1);
		POKDEV(0x2, res);
		break;
	}
	return d[port];
}
