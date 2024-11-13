#ifndef __UDS_SERVICE_31_H_INCLUDED__
#define __UDS_SERVICE_31_H_INCLUDED__

struct uds_context;

typedef struct uds_service_31_routine {
	unsigned short rid;
	struct {
		unsigned char type;
		unsigned int len;
		char desc[32];
		unsigned char sessions[3];
		unsigned char security_access_levels[3];
	} control_type[2];
} uds_service_31_routine_t;

typedef struct uds_service_31 {
	int count;
	uds_service_31_routine_t *routines;
} uds_service_31_t;

void uds_service_31_init(struct uds_context *uds_context);

int uds_service_31_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
