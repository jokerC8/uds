#ifndef __UDS_SERVICE_85_H_INCLUDED__
#define __UDS_SERVICE_85_H_INCLUDED__

struct uds_context;

typedef enum {
	DTC_SETTING_TYPE_ON = 0x01,
	DTC_SETTING_TYPE_OFF,
} DTC_Setting_Type_E;

typedef struct uds_service_85 {
	DTC_Setting_Type_E type;
} uds_service_85_t;

void uds_service_85_init(struct uds_context *uds_context);

void uds_service_85_dtc_setting_on(struct uds_context *uds_context);

void uds_service_85_dtc_setting_off(struct uds_context *uds_context);

int uds_service_85_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
