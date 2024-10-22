#ifndef __UDS_H_INCLUDED__
#define __UDS_H_INCLUDED__

#include <stdint.h>
#include <sys/un.h>

#ifndef UDS_MIN
#define UDS_MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define UDS_RECEIVER_SOCKFILE                    "/tmp/doip2uds"
#define UDS_SENDER_SOCKFILE                      "/tmp/uds2doip"

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

#define doip_assert(expr, format, args...) do {\
	if (!!!expr) { \
		loge(format, ##args); \
		assert(!!expr); \
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
	offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "[line:%d, func:%s] ", __LINE__, __FUNCTION__); \
	for (int i = 0; i < len; ++i) { \
		offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "%02x ", data[i]); \
	} \
	offset += snprintf(__buffer__ + offset, sizeof(__buffer__) - offset, "\033[0m"); \
	fprintf(stdout, "%s\n", __buffer__); \
} while (0)

typedef struct uds_indication {
	int status;
	int handler;
#define MAX_UDS_PDU_SIZE (0x4000)
	int cap;
	int len;
	uint8_t *buffer;
	const char *sockfile;
} uds_indication_t; /* uds_indication */

typedef struct uds_request {
	int status;
	int handler;
	uint8_t buffer[4096];
	struct sockaddr_un target;
	const char *sockfile;
} uds_request_t; /* uds_request */

typedef struct uds_context {
	int quit;
	int busy;
	int start;
	int status;
	uint16_t sa;
	uint16_t ta;
	uint8_t ta_type;
	uint8_t reserved_1;
	uint8_t reserved_2;
	uint8_t reserved_3;
	uds_request_t uds_request;
	uds_indication_t uds_indication;
	/* timer looper */
	struct timer_loop *loop;
	/* timer for debug*/
	struct uds_timer *heartbeat_timer;
} uds_context_t; /* uds_context */

struct uds_context;

struct uds_context *uds_context_init(const char *conf);

void uds_service_start(struct uds_context *uds_context);

void uds_service_stop(struct uds_context *uds_context);

void uds_context_destroy(struct uds_context *uds_context);

#endif
