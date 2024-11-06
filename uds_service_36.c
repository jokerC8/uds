#include "uds.h"
#include "uds_stream.h"

int uds_service_36_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uds_stream_t strm = {0};
	uint8_t sid, blockSequenceCounter;
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;
	struct uds_service_36 *uds_service_36 = &uds_context->uds_service_36;

	if (len < 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	/* 当前会话模式不支持 */
	if (uds_diagnostic_session(uds_context) != UDS_PROGRAMMING_SESSION) {
		nrc = NRC_SubFunctionNotSupportedInActiveSession_7e;
		goto finish;
	}

	/* 当前安全级别下不支持 */
	if (uds_security_access_level(uds_context) != SECURITY_ACCESS_LEVEL_2) {
		nrc = NRC_SecurityAccessDenied_33;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	blockSequenceCounter = uds_stream_read_byte(&strm);

	/* 序列号不匹配 */
	if (blockSequenceCounter != uds_service_36->block_num) {
		nrc = NRC_RequestSequenceError_24;
		goto finish;
	}
	uds_service_36->block_num++;

	if (uds_service_36->fp == NULL) {

	}

finish:
	uds_context->nrc = nrc;
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, uds_context->sid + 0x40);
		uds_stream_write_byte(&strm, blockSequenceCounter);
	}
	uds_response->len = uds_stream_len(&strm);

	return nrc;
}

void uds_service_36_init(struct uds_context *uds_context)
{
	uds_context->uds_service_36.block_num = 1;
	logd("uds_service_36_init\n");
}
