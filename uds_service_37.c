#include "uds.h"
#include "uds_stream.h"

int uds_service_37_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 1) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

finish:
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_context->nrc = nrc;
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, uds_context->sid + 0x40);
	}
	uds_response->len = uds_stream_len(&strm);

	return nrc;
}

void uds_service_37_init(struct uds_context *uds_context)
{
	logd("uds_service_37_init\n");
}
