#ifndef __UDS_DTC_MONITOR_H_INCLUDED__
#define __UDS_DTC_MONITOR_H_INCLUDED__

#define DTC_NUMBER(data,len) ((data)[0] << 16 | (data)[1] << 8 | (data)[0])

struct uds_context;

typedef struct uds_dtc {
	unsigned char status;
	struct {
		unsigned char DTCHighByte;
		unsigned char DTCMiddleByte;
		unsigned char DTCLowByte;
	} dtc;
	unsigned int dtc_num;
} uds_dtc_t;

typedef struct uds_dtc_monitor {
	int count;
	uds_dtc_t *dtcs;
	struct uds_timer *monitor_timer;
} uds_dtc_monitor_t;

void uds_dtc_monitor_init(struct uds_context *uds_context);

#endif
