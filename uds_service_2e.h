#ifndef __UDS_SERVICE_2e_H_INCLUDED__
#define __UDS_SERVICE_2e_H_INCLUDED__

struct uds_context;

typedef struct uds_service_2e {

} uds_service_2e_t;

void uds_service_2e_init(struct uds_context *uds_context);

int uds_service_2e_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
