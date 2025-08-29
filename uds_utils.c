#include "uds_utils.h"
#include <stdint.h>

unsigned long byte_array2_uint64(unsigned char *data, int len)
{
	uint64_t val = 0;

	for (int i = 0; i < len; i++) {
		val |= data[i];
		if (i < (len - 1)) {
			val <<= 8;
		}
	}

	return val;
}
