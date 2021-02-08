#include <stdio.h>

typedef unsigned char Uint8;
typedef unsigned short Uint16;

typedef struct {
	Uint8 ptr;
	Uint8 dat[256];
} Stack8;

typedef struct {
	Uint8 ptr;
	Uint16 dat[256];
} Stack16;

typedef struct {
	Uint16 ptr;
	Uint8 dat[65536];
} Memory;

typedef struct {
	Uint8 literal, status;
	Uint16 counter, vreset, vframe, verror;
	Stack8 wst;
	Stack16 rst;
	Memory ram;
} Computer;

int error(char *name, int id);
int load(FILE *f);
int boot(void);
void echof(void);
void echom(Memory *m, Uint8 len, char *name);
void echos(Stack8 *s, Uint8 len, char *name);
