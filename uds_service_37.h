#ifndef __UDS_SERVICE_37_H_INCLUDED__
#define __UDS_SERVICE_37_H_INCLUDED__

struct uds_context;

typedef struct uds_service_37 {

} uds_service_37_t;

void uds_service_37_init(struct uds_context *uds_context);

int uds_service_37_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
