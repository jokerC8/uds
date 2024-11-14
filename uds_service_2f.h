#ifndef __UDS_SERVICE_2f_H_INCLUDED__
#define __UDS_SERVICE_2f_H_INCLUDED__

struct uds_context;

typedef struct uds_service_2f {

} uds_service_2f_t;

void uds_service_2f_init(struct uds_context *uds_context);

void uds_service_2f_io_reset(struct uds_context *uds_context);

int uds_service_2f_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
