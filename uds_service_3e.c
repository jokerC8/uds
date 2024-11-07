#include "uds.h"
#include "uds_stream.h"

/* 通常以功能寻址方式发送3e 80, 可根据车厂实际规范修改 */
int uds_service_3e_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint8_t sub;
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	uds_stream_forward(&strm, 1);
	sub = uds_stream_read_byte(&strm);

	if (sub != 0x80) {
		nrc = NRC_SubFunctionNotSupported_12;
		goto finish;
	}

	maintain_diagnostic_mode(uds_context);

finish:
	uds_context->nrc = nrc;
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, sub);
	}
	uds_response->len = uds_stream_len(&strm);
	return nrc;
}

void uds_service_3e_init(struct uds_context *uds_context)
{
	logd("uds_service_3e_init\n");
}
