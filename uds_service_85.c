#include "uds.h"
#include "uds_stream.h"

void uds_service_85_dtc_setting_on(uds_context_t *uds_context)
{
	uds_context->uds_service_85.type = DTC_SETTING_TYPE_ON;
}

void uds_service_85_dtc_setting_off(uds_context_t *uds_context)
{
	uds_context->uds_service_85.type = DTC_SETTING_TYPE_OFF;
}

/* 1. DTC_SETTING_TYPE_ON
 * 2. ECUReset
 * 3. 从非默认会话模式退回默认会话
 * 以上三种情况下需要重新打开DTC设置,参考(uds_service_10.c, uds_service_11.c, uds_service_85.c)
 */
int uds_service_85_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint8_t sid, sub;
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	if (uds_diagnostic_session(uds_context) != UDS_EXTEND_SESSION) {
		nrc = NRC_ServiceNotSupportedInActiveSession_7f;
		goto finish;
	}

	sid = uds[0];
	sub = uds[1];
	switch (Acquire_Sub_Function(sub)) {
		case DTC_SETTING_TYPE_ON:
			uds_service_85_dtc_setting_on(uds_context);
			break;
		case DTC_SETTING_TYPE_OFF:
			uds_service_85_dtc_setting_off(uds_context);
			break;
		default:
			nrc = NRC_SubFunctionNotSupported_12;
			break;
	}

finish:
	uds_context->nrc = nrc;
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, sub);
	}

	uds_response->len = uds_stream_len(&strm);
	return nrc;
}

void uds_service_85_init(struct uds_context *uds_context)
{
	logd("uds_service_85_init\n");

	uds_context->uds_service_85.type = DTC_SETTING_TYPE_ON;
}
