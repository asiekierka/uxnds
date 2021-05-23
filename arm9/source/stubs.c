#include <nds.h>
#include <time.h>

// Stubbing out unused functionality, in order to optimize the uxnds filesize.

// libfat source/filetime.c

uint16_t _FAT_filetime_getTimeFromRTC (void) {
	return 0;
}

uint16_t _FAT_filetime_getDateFromRTC (void) {
	return 0;

}
time_t _FAT_filetime_to_time_t (uint16_t t, uint16_t d) {
	return 0;
}


