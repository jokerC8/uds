#ifndef __UDS_SERVICE_85_H_INCLUDED__
#define __UDS_SERVICE_85_H_INCLUDED__

struct uds_context;

struct uds_service_85 {

};

void uds_service_85_init(struct uds_context *uds_context);

int uds_service_85_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
