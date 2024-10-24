#ifndef __UDS_SERVICE_22_H_INCLUDED__
#define __UDS_SERVICE_22_H_INCLUDED__

struct uds_context;

void uds_service_22_init(struct uds_context *uds_context);

int uds_service_22_handler(struct uds_context *uds_context, unsigned char *data, int len);

#endif
