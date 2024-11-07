#ifndef __UDS_SERVICE_38_H_INCLUDED__
#define __UDS_SERVICE_38_H_INCLUDED__

#define Max_Number_Of_Block_Length (10240)

struct uds_context;

typedef struct uds_service_38 {
	char filename[256];
	int filename_len;
	unsigned long fileSizeCompressed;
	unsigned long fileSizeUnCompressed;
} uds_service_38_t;

void uds_service_38_init(struct uds_context *uds_context);

int uds_service_38_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
