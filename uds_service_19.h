#ifndef __UDS_SERVICE_19_H_INCLUDED__
#define __UDS_SERVICE_19_H_INCLUDED__

typedef enum {
	REPORT_NUMBER_OF_DTC_BY_STATUS_MASK = 0x01,
	REPORT_DTC_BY_STATUS_MASK = 0x02,
	REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER = 0x04,
	REPORT_DTC_EXTENDED_DATA_BY_DTC_NUMBER = 0x06,
	REPORT_SUPPORTED_DTC = 0x0a,
} DTC_Report_Type_E;

struct uds_context;

typedef struct uds_service_19 {

} uds_service_19_t;

void uds_service_19_init(struct uds_context *uds_context);

int uds_service_19_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
