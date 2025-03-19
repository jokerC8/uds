#include "uds.h"
#include "uds_stream.h"

void uds_service_28_communicate_control_reset(uds_context_t *uds_context)
{
	logd("uds_service_28_communicate_control_reset\n");
}

static void communication_control_EnableRxAndTx(uds_context_t *uds_context, uint8_t type)
{
	logd("communication_control_EnableRxAndTx\n");
}

static void communication_control_EnableRxAndDisableTx(uds_context_t *uds_context, uint8_t type)
{
	logd("communication_control_EnableRxAndDisableTx\n");
}

static void communication_control_DisableRxAndEnableTx(uds_context_t *uds_context, uint8_t type)
{
	logd("communication_control_DisableRxAndEnableTx\n");
}

static void communication_control_DisableRxAndTx(uds_context_t *uds_context, uint8_t type)
{
	logd("communication_control_DisableRxAndTx\n");
}

int uds_service_28_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;

	/* 长度或者格式不正确 */
	if (len != 3) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	/* 子功能在当前会话模式下不支持 */
	if (uds_diagnostic_session(uds_context) != UDS_EXTEND_SESSION) {
		nrc = NRC_SubFunctionNotSupportedInActiveSession_7e;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	uds_stream_forward(&strm, 1);
	uint8_t sub = uds_stream_read_byte(&strm);
	uint8_t type = uds_stream_read_byte(&strm);
	switch (Acquire_Sub_Function(sub)) {
		case CommunicationControl_EnableRxAndTx:
			communication_control_EnableRxAndTx(uds_context, type);
			break;
		case CommunicationControl_EnableRxAndDisableTx:
			communication_control_EnableRxAndDisableTx(uds_context, type);
			break;
		case CommunicationControl_DisableRxAndEnableTx:
			communication_control_DisableRxAndEnableTx(uds_context, type);
			break;
		case CommunicationControl_DisableRxAndTx:
			communication_control_DisableRxAndTx(uds_context, type);
			break;
		default:
			nrc = NRC_SubFunctionNotSupported_12;
			break;
	}

finish:
	uds_stream_init(&strm, uds_response->buffer, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, sub);
	}

	uds_response->len = uds_stream_len(&strm);
	return nrc;
}

void uds_service_28_init(struct uds_context *uds_context)
{
	logd("uds_service_28_init\n");
}
