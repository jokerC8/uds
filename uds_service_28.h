#ifndef __UDS_SERVICE_28_H_INCLUDED__
#define __UDS_SERVICE_28_H_INCLUDED__

struct uds_context;

struct uds_service_28 {

};

void uds_service_28_init(struct uds_context *uds_context);

int uds_service_28_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
