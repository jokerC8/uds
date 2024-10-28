#ifndef __UDS_SERVICE_14_H_INCLUDED__
#define __UDS_SERVICE_14_H_INCLUDED__

struct uds_context;

struct uds_service_14 {

};

void uds_service_14_init(struct uds_context *uds_context);

int uds_service_14_handler(struct uds_context *uds_context, unsigned char *data, int len);

#endif
