#ifndef __UDS_SERVICE_36_H_INCLUDED__
#define __UDS_SERVICE_36_H_INCLUDED__

struct uds_context;

typedef struct uds_service_36 {
	void *fp;
#define SOC_FIRMWARE_DIR "/mnt/sdisk/firmware_update"
#define SOC_FIRMWARE_FILEPATH SOC_FIRMWARE_DIR "/soc.zip"
	char *filepath;
	unsigned char block_num;
} uds_service_36_t;

void uds_service_36_prepare(struct uds_context *uds_context);

void uds_service_36_init(struct uds_context *uds_context);

int uds_service_36_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
