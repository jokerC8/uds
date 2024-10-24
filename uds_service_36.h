#ifndef __UDS_SERVICE_36_H_INCLUDED__
#define __UDS_SERVICE_36_H_INCLUDED__

struct uds_context;

struct uds_service_36 {

};

void uds_service_36_init(struct uds_context *uds_context);

int uds_service_36_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
