#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#include <libiberty/libiberty.h>
#define realpath(s, dummy) lrealpath(s)
#define DIR_SEP_CHAR '\\'
#define DIR_SEP_STR "\\"
#define pathcmp(path1, path2, length) strncasecmp(path1, path2, length) /* strncasecmp provided by libiberty */
#define notdriveroot(file_name) (file_name[0] != DIR_SEP_CHAR && ((strlen(file_name) > 2 && file_name[1] != ':') || strlen(file_name) <= 2))
#define mkdir(file_name) (_mkdir(file_name) == 0)
#else
#define DIR_SEP_CHAR '/'
#define DIR_SEP_STR "/"
#define pathcmp(path1, path2, length) strncmp(path1, path2, length)
#define notdriveroot(file_name) (file_name[0] != DIR_SEP_CHAR)
#define mkdir(file_name) (mkdir(file_name, 0755) == 0)
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "../uxn.h"
#include "file.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

typedef struct {
	FILE *f;
	DIR *dir;
	char current_filename[4096];
	struct dirent *de;
	enum { IDLE,
		FILE_READ,
		FILE_WRITE,
		DIR_READ,
		DIR_WRITE
	} state;
	int outside_sandbox;
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
	c->outside_sandbox = 0;
}

static Uint16
get_entry(char *p, Uint16 len, const char *pathname, const char *basename, int fail_nonzero)
{
	struct stat st;
	if(len < strlen(basename) + 8)
		return 0;
	if(stat(pathname, &st))
		return fail_nonzero ? sniprintf(p, len, "!!!! %s\n", basename) : 0;
	else if(S_ISDIR(st.st_mode))
		return sniprintf(p, len, "---- %s/\n", basename);
	else if(st.st_size < 0x10000)
		return sniprintf(p, len, "%04x %s\n", (unsigned int)st.st_size, basename);
	else
		return sniprintf(p, len, "???? %s\n", basename);
}

static Uint16
file_read_dir(UxnFile *c, char *dest, Uint16 len)
{
	static char pathname[4352];
	char *p = dest;
	if(c->de == NULL) c->de = readdir(c->dir);
	for(; c->de != NULL; c->de = readdir(c->dir)) {
		Uint16 n;
		if(c->de->d_name[0] == '.' && c->de->d_name[1] == '\0')
			continue;
		if(strcmp(c->de->d_name, "..") == 0) {
			/* hide "sandbox/.." */
			char cwd[PATH_MAX] = {'\0'}, *t;
			/* Note there's [currently] no way of chdir()ing from uxn, so $PWD
			 * is always the sandbox top level. */
			getcwd(cwd, sizeof(cwd));
			/* We already checked that c->current_filename exists so don't need a wrapper. */
			t = realpath(c->current_filename, NULL);
			if(strcmp(cwd, t) == 0) {
				free(t);
				continue;
			}
			free(t);
		}
		if(strlen(c->current_filename) + 1 + strlen(c->de->d_name) < sizeof(pathname))
			sniprintf(pathname, sizeof(pathname), "%s/%s", c->current_filename, c->de->d_name);
		else
			pathname[0] = '\0';
		n = get_entry(p, len, pathname, c->de->d_name, 1);
		if(!n) break;
		p += n;
		len -= n;
	}
	return p - dest;
}

static char *
retry_realpath(const char *file_name)
{
	char *r, p[PATH_MAX] = {'\0'}, *x;
	int fnlen;
	if(file_name == NULL) {
		errno = EINVAL;
		return NULL;
	} else if((fnlen = strlen(file_name)) >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return NULL;
	}
	if(notdriveroot(file_name)) {
		/* TODO: use a macro instead of '/' for absolute path first character so that other systems can work */
		/* if a relative path, prepend cwd */
		getcwd(p, sizeof(p));
		if(strlen(p) + strlen(DIR_SEP_STR) + fnlen >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return NULL;
		}
		strcat(p, DIR_SEP_STR); /* TODO: use a macro instead of '/' for the path delimiter */
	}
	strcat(p, file_name);
	while((r = realpath(p, NULL)) == NULL) {
		if(errno != ENOENT)
			return NULL;
		x = strrchr(p, DIR_SEP_CHAR); /* TODO: path delimiter macro */
		if(x)
			*x = '\0';
		else
			return NULL;
	}
	return r;
}

static void
file_check_sandbox(UxnFile *c)
{
	char *x, *rp, cwd[PATH_MAX] = {'\0'};
	x = getcwd(cwd, sizeof(cwd));
	rp = retry_realpath(c->current_filename);
	if(rp == NULL || (x && pathcmp(cwd, rp, strlen(cwd)) != 0)) {
		c->outside_sandbox = 1;
		fiprintf(stderr, "file warning: blocked attempt to access %s outside of sandbox\n", c->current_filename);
	}
	free(rp);
}

static Uint16
file_init(UxnFile *c, char *filename, size_t max_len, int override_sandbox)
{
	char *p = c->current_filename;
	size_t len = sizeof(c->current_filename);
	reset(c);
	if(len > max_len) len = max_len;
	while(len) {
		if((*p++ = *filename++) == '\0') {
			if(!override_sandbox) /* override sandbox for loading roms */
				file_check_sandbox(c);
			return 0;
		}
		len--;
	}
	c->current_filename[0] = '\0';
	return 0;
}

static Uint16
file_read(UxnFile *c, void *dest, int len)
{
	if(c->outside_sandbox) return 0;
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

static int
is_dir_path(char *p)
{
	char c;
	int saw_slash = 0;
	while((c = *p++))
		saw_slash = c == DIR_SEP_CHAR;
	return saw_slash;
}

int
dir_exists(char *p)
{
	struct stat st;
	return stat(p, &st) == 0 && S_ISDIR(st.st_mode);
}

int
ensure_parent_dirs(char *p)
{
	int ok = 1;
	char c, *s = p;
	for(; ok && (c = *p); p++) {
		if(c == DIR_SEP_CHAR) {
			*p = '\0';
			ok = dir_exists(s) || mkdir(s);
			*p = c;
		}
	}
	return ok;
}

static Uint16
file_write(UxnFile *c, void *src, Uint16 len, Uint8 flags)
{
	Uint16 ret = 0;
	if(c->outside_sandbox) return 0;
	ensure_parent_dirs(c->current_filename);
	if(c->state != FILE_WRITE && c->state != DIR_WRITE) {
		reset(c);
		if(is_dir_path(c->current_filename))
			c->state = DIR_WRITE;
		else if((c->f = fopen(c->current_filename, (flags & 0x01) ? "ab" : "wb")) != NULL)
			c->state = FILE_WRITE;
	}
	if(c->state == FILE_WRITE) {
		if((ret = fwrite(src, 1, len, c->f)) > 0 && fflush(c->f) != 0)
			ret = 0;
	}
	if(c->state == DIR_WRITE) {
		ret = dir_exists(c->current_filename);
	}
	return ret;
}

static Uint16
stat_fill(Uint8 *dest, Uint16 len, char c)
{
	Uint16 i;
	for(i = 0; i < len; i++)
		*(dest++) = c;
	return len;
}

static Uint16
stat_size(Uint8 *dest, Uint16 len, off_t size)
{
	Uint16 i;
	dest += len - 1;
	for(i = 0; i < len; i++) {
		char c = '0' + (Uint8)(size & 0xf);
		if(c > '9') c += 39;
		*(dest--) = c;
		size = size >> 4;
	}
	return size == 0 ? len : stat_fill(dest, len, '?');
}

static Uint16
file_stat(UxnFile *c, void *dest, Uint16 len)
{
	struct stat st;
	if(c->outside_sandbox)
		return 0;
	else if(stat(c->current_filename, &st))
		return stat_fill(dest, len, '!');
	else if(S_ISDIR(st.st_mode))
		return stat_fill(dest, len, '-');
	else
		return stat_size(dest, len, st.st_size);
}

static Uint16
file_delete(UxnFile *c)
{
	return c->outside_sandbox ? 0 : unlink(c->current_filename);
}

/* IO */

void
file_deo(Uxn *u, Uint8 port)
{
	Uint16 addr, len, res;
	switch(port) {
	case 0xa5:
		addr = PEEK2(&u->dev[0xa4]);
		len = PEEK2(&u->dev[0xaa]);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_stat(&uxn_file[0], &u->ram.dat[addr], len);
		POKE2(&u->dev[0xa2], res);
		break;
	case 0xa6:
		res = file_delete(&uxn_file[0]);
		POKE2(&u->dev[0xa2], res);
		break;
	case 0xa9:
		addr = PEEK2(&u->dev[0xa8]);
		res = file_init(&uxn_file[0], (char *)&u->ram.dat[addr], 0x10000 - addr, 0);
		POKE2(&u->dev[0xa2], res);
		break;
	case 0xad:
		addr = PEEK2(&u->dev[0xac]);
		len = PEEK2(&u->dev[0xaa]);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_read(&uxn_file[0], &u->ram.dat[addr], len);
		POKE2(&u->dev[0xa2], res);
		break;
	case 0xaf:
		addr = PEEK2(&u->dev[0xae]);
		len = PEEK2(&u->dev[0xaa]);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_write(&uxn_file[0], &u->ram.dat[addr], len, u->dev[0xa7]);
		POKE2(&u->dev[0xa2], res);
		break;
	/* File 2 */
	case 0xb5:
		addr = PEEK2(&u->dev[0xb4]);
		len = PEEK2(&u->dev[0xba]);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_stat(&uxn_file[1], &u->ram.dat[addr], len);
		POKE2(&u->dev[0xb2], res);
		break;
	case 0xb6:
		res = file_delete(&uxn_file[1]);
		POKE2(&u->dev[0xb2], res);
		break;
	case 0xb9:
		addr = PEEK2(&u->dev[0xb8]);
		res = file_init(&uxn_file[1], (char *)&u->ram.dat[addr], 0x10000 - addr, 0);
		POKE2(&u->dev[0xb2], res);
		break;
	case 0xbd:
		addr = PEEK2(&u->dev[0xbc]);
		len = PEEK2(&u->dev[0xba]);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_read(&uxn_file[1], &u->ram.dat[addr], len);
		POKE2(&u->dev[0xb2], res);
		break;
	case 0xbf:
		addr = PEEK2(&u->dev[0xbe]);
		len = PEEK2(&u->dev[0xba]);
		if(len > 0x10000 - addr)
			len = 0x10000 - addr;
		res = file_write(&uxn_file[1], &u->ram.dat[addr], len, u->dev[0xb7]);
		POKE2(&u->dev[0xb2], res);
		break;
	}
}
