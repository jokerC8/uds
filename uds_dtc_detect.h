#ifndef __UDS_DTC_DETECT_H_INCLUDED__
#define __UDS_DTC_DETECT_H_INCLUDED__

struct uds_context;

typedef struct uds_dtc_detect {
	struct uds_timer *detect_timer;
} uds_dtc_detect_t;

void uds_dtc_detect_init(struct uds_context *uds_context);

#endif
