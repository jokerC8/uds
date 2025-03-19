#ifndef __UDS_SERVICE_UTILS_H_INCLUDED__
#define __UDS_SERVICE_UTILS_H_INCLUDED__

#include <stdio.h>
#include <time.h>
#include <assert.h>

#define UDS_DEBUG_ENABLE

#ifndef TRUE
#define TRUE (0==0)
#endif

#ifndef FALSE
#define FALSE !(TRUE)
#endif

#ifndef UNUSED
#define UNUSED (void)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef UDS_DEBUG_ENABLE

#define logd(format, args...) do {\
	int offset = 0; \
	char __buffer__[1024] = {0}; \
	time_t cur; \
	struct tm tm; \
	time(&cur); \
	localtime_r(&cur, &tm); \
	offset += strftime(__buffer__ + offset, sizeof(__buffer__) - offset, "\033[32m%Y-%m-%d %H:%M:%S ", &tm); \
	offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "[func:%s, line:%d] " format "\033[0m", __FUNCTION__, __LINE__, ##args); \
	fprintf(stdout, "%s", __buffer__); \
} while (0)

#define loge(format, args...) do {\
	int offset = 0; \
	char __buffer__[1024] = {0}; \
	time_t cur; \
	struct tm tm; \
	time(&cur); \
	localtime_r(&cur, &tm); \
	offset += strftime(__buffer__ + offset, sizeof(__buffer__) - offset, "\033[31m%Y-%m-%d %H:%M:%S ", &tm); \
	offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "[func:%s, line:%d] " format "\033[0m", __FUNCTION__, __LINE__, ##args); \
	fprintf(stderr, "%s", __buffer__); \
} while (0)

#define uds_assert(expr, format, args...) do {\
	if (!!!(expr)) { \
		loge(format, ##args); \
		assert(!!(expr)); \
	} \
} while (0)

#define uds_hexdump(data, len) do { \
	int offset = 0; \
	char __buffer__[4096] = {0}; \
	time_t cur; \
	struct tm tm; \
	time(&cur); \
	localtime_r(&cur, &tm); \
	offset += strftime(__buffer__ + offset, sizeof(__buffer__) - offset, "\033[32m%Y-%m-%d %H:%M:%S ", &tm); \
	offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "[func:%s, line:%d] ", __FUNCTION__, __LINE__); \
	for (int i = 0; i < len; ++i) { \
		offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "%02x ", data[i]); \
	} \
	offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "\033[0m"); \
	fprintf(stdout, "%s\n", __buffer__); \
} while (0)

#else
#define logd(format, args...)
#define loge(format, args...)
#define uds_hexdump(data, len)
#endif

unsigned long byte_array2_uint64(unsigned char *data, int len);

#endif
