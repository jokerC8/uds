#ifndef __UDS_SERVICE_22_H_INCLUDED__
#define __UDS_SERVICE_22_H_INCLUDED__

struct uds_context;

typedef struct uds_service_22_identifier {
	unsigned short did;
	unsigned char session_nrc;
	unsigned char security_access_nrc;
	unsigned int len;
	char desc[64];
	struct {
		unsigned char session;
		unsigned char attribute[3];
	} sessions[3];
	struct {
		unsigned char level;
		unsigned char attribute[3];
	} security_access_levels[3];
} uds_service_22_identifier_t;

typedef struct uds_service_22 {
	int count;
	struct uds_service_22_identifier *identifiers;
} uds_service_22_t;

void uds_service_22_init(struct uds_context *uds_context);

int uds_service_22_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
