#ifndef __UDS_SERVICE_11_H_INCLUDED__
#define __UDS_SERVICE_11_H_INCLUDED__

struct uds_context;

typedef enum {
	ECURESET_HARDRESET = 0x01,
	ECURESET_KEYOFFRESET,
	ECURESET_SOFTRESET,
	ECURESET_MAX,
} ECU_ResetType_E;

typedef struct uds_service_11 {
	ECU_ResetType_E reset_type;
	struct uds_timer *delay_reset_timer;
} uds_service_11_t;

void uds_service_11_init(struct uds_context *uds_context);

int uds_service_11_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
