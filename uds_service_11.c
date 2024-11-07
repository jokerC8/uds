#include "uds.h"
#include "uds_stream.h"
#include "uds_timer.h"

#include <stdlib.h>

static void hard_reset(struct uds_context *uds_context)
{
	UNUSED(uds_context);
	system("reboot");
}

static void keyoff_reset(struct uds_context *uds_context)
{
	UNUSED(uds_context);
	system("reboot");
}

static void soft_reset(struct uds_context *uds_context)
{
	UNUSED(uds_context);
	system("reboot");
}

static const char *uds_service_sub_desc(unsigned char sub)
{
	switch (sub) {
		case ECURESET_HARDRESET:
			return "ECUReset Hard Reset";
		case ECURESET_KEYOFFRESET:
			return "ECUReset Keyoff Reset";
		case ECURESET_SOFTRESET:
			return "ECUReset Soft Reset";
	}

	return "unknow";
}

int uds_service_11_handler(struct uds_context *uds_context, uint8_t *uds, int len)
{
	uint8_t sid, sub, reset_type;
	uds_stream_t strm = {0};
	int nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;
	uds_service_11_t *uds_service_11 = &uds_context->uds_service_11;

	if (len != 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	sub = uds_stream_read_byte(&strm);
	reset_type = Acquire_Sub_Function(sub);

	uds_service_11->reset_type = reset_type;

	logd("%s\n", uds_service_sub_desc(reset_type));

	if (!(reset_type == ECURESET_HARDRESET || reset_type == ECURESET_KEYOFFRESET || reset_type == ECURESET_SOFTRESET)) {
		nrc = NRC_SubFunctionNotSupported_12;
		goto finish;
	}

	/* 恢复DTC设置 */
	uds_service_85_dtc_setting_on(uds_context);

	if (!uds_timer_running(uds_service_11->delay_reset_timer)) {
		uds_timer_start(uds_context->loop, uds_service_11->delay_reset_timer);
	}

finish:
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, sub);
	}

	uds_context->nrc = nrc;
	uds_response->len = uds_stream_len(&strm);
	return nrc;
}

static void delay_reset_timer_callback(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);
	uds_service_11_t *uds_service_11 = &uds_context->uds_service_11;

	if (uds_service_11->reset_type == ECURESET_HARDRESET) {
		hard_reset(uds_context);
	}
	else if (uds_service_11->reset_type == ECURESET_KEYOFFRESET) {
		keyoff_reset(uds_context);
	}
	else if (uds_service_11->reset_type == ECURESET_SOFTRESET) {
		soft_reset(uds_context);
	}
}

void uds_service_11_init(struct uds_context *uds_context)
{
	logd("uds_service_11_init\n");
	uds_context->uds_service_11.delay_reset_timer = uds_timer_alloc(delay_reset_timer_callback, 1000, 0, 0);
	uds_timer_set_userdata(uds_context->uds_service_11.delay_reset_timer, uds_context);
}
