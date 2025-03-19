#include "uds.h"
#include "uds_stream.h"

int uds_service_37_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 1) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_service_36_prepare(uds_context);

finish:
	uds_context->nrc = nrc;
	uds_response->len = 0;

	return nrc;
}

void uds_service_37_init(struct uds_context *uds_context)
{
	logd("uds_service_37_init\n");
}
