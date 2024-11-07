#ifndef __UDS_SERVICE_27_H_INCLUDED__
#define __UDS_SERVICE_27_H_INCLUDED__

struct uds_context;

#define SECURITY_ACCESS_KEY "\x11\x22\x33\x44"
#define SECURITY_ACCESS_SEED "\x11\x22\x33\x44"

#define MAX_INVALID_KEY_RETRY_TIMES (3)

typedef enum {
	SECURITY_ACCESS_LEVEL_LOCK = 0x00,
	SECURITY_ACCESS_LEVEL_1,
	SECURITY_ACCESS_LEVEL_2,
	SECURITY_ACCESS_LEVEL_MAX,
} Security_Level_E;

enum {
	SECURITY_ACCESS_REQUEST_SEED_1 = 0x01,
	SECURITY_ACCESS_REQUEST_SEED_2,
	SECURITY_ACCESS_SEND_KEY_1 = 0x09,
	SECURITY_ACCESS_SEND_KEY_2,
} Security_Access_Operation_E;

typedef struct uds_service_27 {
	int status;
	int fail_count;
	unsigned char seed[4];
	unsigned char key[4];
	Security_Level_E security_access_level;
#define REFUSE_REQUEST_TIME (10000)
	/* 连续3次发送无效key, 需等待10s才能再次请求 */
	struct uds_timer *delay_timer;
} uds_service_27_t;

void uds_service_27_init(struct uds_context *uds_context);

void uds_service_27_lock_ecu(struct uds_context *uds_context);

Security_Level_E uds_security_access_level(struct uds_context *uds_context);

int uds_service_27_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
