#ifndef __UDS_SERVICE_19_H_INCLUDED__
#define __UDS_SERVICE_19_H_INCLUDED__

struct uds_context;

struct uds_service_19 {

};

void uds_service_19_init(struct uds_context *uds_context);

int uds_service_19_handler(struct uds_context *uds_context, unsigned char *data, int len);

#endif
