#ifndef __UDS_SERVICE_10_H_INCLUDED__
#define __UDS_SERVICE_10_H_INCLUDED__

struct uds_context;

typedef enum {
	UDS_DEFAULT_SESSION = 0x01,
	UDS_PROGRAMMING_SESSION,
	UDS_EXTEND_SESSION,
	UDS_SESSION_MAX,
} UDS_Session_E;

typedef struct uds_service_10 {
	UDS_Session_E session;
#define MAX_S3SERVER_MS (5000)
	int S3Server;
	struct uds_timer *s3_timer;
} uds_service_10_t;

UDS_Session_E uds_diagnostic_session(struct uds_context *uds_context);

void maintain_diagnostic_mode(struct uds_context *uds_context);

void uds_service_10_init(struct uds_context *uds_context);

int uds_service_10_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
