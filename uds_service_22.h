#ifndef __UDS_SERVICE_22_H_INCLUDED__
#define __UDS_SERVICE_22_H_INCLUDED__

#define UDS_IDENTIFIERS_CONF "./uds_service_22.json"

struct uds_context;

struct uds_service_22_identifier {
	unsigned short did;
	unsigned int len;
	char desc[64];
	struct {
		unsigned char session;
		unsigned char attribute;
	} sessions[3];
	struct {
		unsigned char level;
		unsigned char attribute;
	} security_access_levels[3];
};

struct uds_service_22 {
	int count;
	struct uds_service_22_identifier *identifiers;
};

void uds_service_22_init(struct uds_context *uds_context);

int uds_service_22_handler(struct uds_context *uds_context, unsigned char *data, int len);

#endif
