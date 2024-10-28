#ifndef __UDS_SERVICE_FILTER_H_INCLUDED__
#define __UDS_SERVICE_FILTER_H_INCLUDED__

struct uds_context;

int uds_service_verify(struct uds_context *uds_context);

int uds_service_session_verify(struct uds_context *uds_context);

#endif
