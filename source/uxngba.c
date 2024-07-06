#ifdef __NDS__
#include <nds.h>
#else
#include <3ds.h>
#define DTCM_BSS
#endif

#include "uxn.h"

extern void uxn_eval_asm(Uint32 pc);

DTCM_BSS u8 wst[256];
DTCM_BSS u8 rst[256];
extern uintptr_t wst_ptr;
extern uintptr_t rst_ptr;

extern uxn_deo_t deo_map[16];
extern uxn_dei_t dei_map[16];
DTCM_BSS u8 device_data[256];

// JSI and similar depend on a 64K alignment (!)
__attribute__((aligned(65536)))
u8 uxn_ram[64 * 1024 * RAM_PAGES];

void
deo_stub(u8 *dev, u8 port) {
    (void)dev;
    (void)port;
}

void
deo2_wrap(u8 *dev, u8 port, uxn_deo_t deo1) {
    deo1(dev,port);
    deo1(dev,port+1);
}

Uint8
dei_stub(u8 *dev, u8 port) {
    return dev[port];
}

int
resetuxn(void)
{
	// Reset the stacks
	memset(wst, 0, sizeof(wst));
	wst_ptr = (uintptr_t) &wst;
	memset(rst, 0, sizeof(rst));
	rst_ptr = (uintptr_t) &rst;

	memset(device_data, 0, 16 * 16);

	// Reset RAM
	memset(uxn_ram, 0, sizeof(uxn_ram));
	return 1;
}

int
uxn_boot(void)
{
	// Emulate legacy API
	u.wst.dat = wst;
	u.rst.dat = rst;
	u.dev = device_data;
	u.ram.dat = uxn_ram;

	return resetuxn();
}

int
uxn_eval(Uxn *u, Uint32 vec)
{
	uxn_eval_asm(vec);
	return 1;
}

int
uxn_get_wst_ptr(void)
{
	return wst_ptr - ((uintptr_t) &wst);
}

int
uxn_get_rst_ptr(void)
{
	return rst_ptr - ((uintptr_t) &rst);
}

void
uxn_set_wst_ptr(int value)
{
	rst_ptr = ((uintptr_t) &rst) + value;
}

void
uxn_set_rst_ptr(int value)
{
	rst_ptr = ((uintptr_t) &rst) + value;
}

void
uxn_register_device(int id, uxn_dei_t dei, uxn_deo_t deo)
{
	if (dei != NULL)
		dei_map[id] = dei;
	if (deo != NULL)
		deo_map[id] = deo;
}