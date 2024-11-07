#include "uds.h"
#include "uds_timer.h"
#include "uds_stream.h"

Security_Level_E uds_security_access_level(struct uds_context *uds_context)
{
	uds_assert(uds_context, "uds_context is NULL");
	return uds_context->uds_service_27.security_access_level;
}

static int generate_security_access_seed_1(uds_context_t *uds_context)
{
	memcpy(uds_context->uds_service_27.seed, SECURITY_ACCESS_SEED, sizeof(uds_context->uds_service_27.seed));
	return TRUE;
}

static int generate_security_access_seed_2(uds_context_t *uds_context)
{
	memcpy(uds_context->uds_service_27.seed, SECURITY_ACCESS_SEED, sizeof(uds_context->uds_service_27.seed));
	return TRUE;
}

static void security_access_request_seed_handler(uds_context_t *uds_context, uint8_t sub)
{
	/* 已经在对应的安全访问级别中, 返回seed 0x00000000 */
	if ((Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_1 && \
		uds_security_access_level(uds_context) == SECURITY_ACCESS_LEVEL_1) || \
		(Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_2 && \
		 uds_security_access_level(uds_context) == SECURITY_ACCESS_LEVEL_2)) {
		bzero(uds_context->uds_service_27.seed, sizeof(uds_context->uds_service_27.seed));
		return;
	}

	if (Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_1) {
		generate_security_access_seed_1(uds_context);
	}
	else {
		generate_security_access_seed_2(uds_context);
	}
}

static void generate_security_access_key_1(uds_context_t *uds_context)
{
	memcpy(uds_context->uds_service_27.key, SECURITY_ACCESS_KEY, sizeof(uds_context->uds_service_27.key));
}

static void generate_security_access_key_2(uds_context_t *uds_context)
{
	memcpy(uds_context->uds_service_27.key, SECURITY_ACCESS_KEY, sizeof(uds_context->uds_service_27.key));
}

static void security_access_send_key_handler(uds_context_t *uds_context, uint8_t sub, uint8_t *data, int len)
{
	uds_service_27_t *uds_service_27 = &uds_context->uds_service_27;

	if (sub == SECURITY_ACCESS_SEND_KEY_1) {
		generate_security_access_key_1(uds_context);
	}
	else {
		generate_security_access_key_2(uds_context);
	}

	if (!memcmp(uds_service_27->key, data, sizeof(uds_service_27->key))) {
		uds_context->nrc = NRC_InvalidKey_35;
		/* 连续发送无效key超过一定次数, 需要等待一段时间才能访问27服务 */
		if (++uds_service_27->fail_count >= MAX_INVALID_KEY_RETRY_TIMES) {
			uds_service_27->delay_timer->timeout = REFUSE_REQUEST_TIME;
			uds_timer_start(uds_context->loop, uds_service_27->delay_timer);
		}
	}
	else {
		uds_context->uds_service_27.fail_count = 0;
		if (uds_timer_running(uds_service_27->delay_timer)) {
			uds_timer_stop(uds_context->loop, uds_service_27->delay_timer);
		}
	}
}

static void security_access_level_set(uds_context_t *uds_context, uint8_t sub)
{
	if (sub == SECURITY_ACCESS_SEND_KEY_1) {
		uds_context->uds_service_27.security_access_level = SECURITY_ACCESS_LEVEL_1;
	}

	else if (sub == SECURITY_ACCESS_SEND_KEY_2) {
		uds_context->uds_service_27.security_access_level = SECURITY_ACCESS_LEVEL_2;
	}
}

int uds_service_27_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint8_t sid, sub;
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;
	uds_service_27_t *uds_service_27 = &uds_context->uds_service_27;

	if (len < 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	sub = uds_stream_read_byte(&strm);

	/* 长度或者格式不正确 */
	if (Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_1 || \
		Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_2) {
		if (len != 2) {
			nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
			goto finish;
		}
	}
	if (Acquire_Sub_Function(sub) == SECURITY_ACCESS_SEND_KEY_1 || \
		Acquire_Sub_Function(sub) == SECURITY_ACCESS_SEND_KEY_2) {
		if (len != 6) {
			nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
			goto finish;
		}
	}

	/* 子功能不支持 */
	if (!(Acquire_Sub_Function(sub) == SECURITY_ACCESS_SEND_KEY_1 || \
		Acquire_Sub_Function(sub) == SECURITY_ACCESS_SEND_KEY_2 || \
		Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_1 || \
		Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_2)) {
		nrc = NRC_SubFunctionNotSupported_12;
	}

	/* 子功能在当前会话模式下不支持 */
	if (Acquire_Sub_Function(sub) == SECURITY_ACCESS_SEND_KEY_1 || \
		Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_1) {
		if (uds_diagnostic_session(uds_context) != UDS_EXTEND_SESSION) {
			nrc = NRC_SubFunctionNotSupportedInActiveSession_7e;
			goto finish;
		}
	}
	if (Acquire_Sub_Function(sub) == SECURITY_ACCESS_SEND_KEY_2 || \
		Acquire_Sub_Function(sub) == SECURITY_ACCESS_REQUEST_SEED_2) {
		if (uds_diagnostic_session(uds_context) != UDS_PROGRAMMING_SESSION) {
			nrc = NRC_SubFunctionNotSupportedInActiveSession_7e;
			goto finish;
		}
	}

	/* 超时时间未到 */
	if (uds_timer_running(uds_service_27->delay_timer)) {
		nrc = NRC_RequiredTimeDelayNotExpired_37;
		goto finish;
	}

	switch (Acquire_Sub_Function(sub)) {
		case SECURITY_ACCESS_REQUEST_SEED_1:
		case SECURITY_ACCESS_REQUEST_SEED_2:
			security_access_request_seed_handler(uds_context, Acquire_Sub_Function(sub));
			break;
		case SECURITY_ACCESS_SEND_KEY_1:
		case SECURITY_ACCESS_SEND_KEY_2:
			security_access_send_key_handler(uds_context, Acquire_Sub_Function(sub), uds + 2, len - 2);
			if (uds_context->nrc == NRC_PositiveRespon_00) {
				security_access_level_set(uds_context, Acquire_Sub_Function(sub));
			}
			break;
	}

finish:
	uds_context->nrc = nrc;
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (uds_context->nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, sub);
		uds_stream_write_data(&strm, uds_service_27->seed, sizeof(uds_service_27->seed));
	}

	uds_response->len = uds_stream_len(&strm);
	return nrc;
}

void uds_service_27_lock_ecu(struct uds_context *uds_context)
{
	uds_context->uds_service_27.security_access_level = SECURITY_ACCESS_LEVEL_LOCK;
}

static void delay_timer_callback(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);
	uds_service_27_t *uds_service_27 = &uds_context->uds_service_27;

	uds_timer_stop(loop, timer);

	logd("uds_service_27->delay_timer timeout(%f)ms, fail_count:%d\n", timer->timeout, uds_service_27->fail_count);
	if (uds_service_27->fail_count > 0) {
		--uds_service_27->fail_count;
	}
}

void uds_service_27_init(struct uds_context *uds_context)
{
	logd("uds_service_27_init\n");

	uds_context->uds_service_27.fail_count = 0;
	uds_context->uds_service_27.security_access_level = SECURITY_ACCESS_LEVEL_LOCK;
	/* 0xffffffff为无效种子 */
	memset(uds_context->uds_service_27.seed, 0xff, sizeof(uds_context->uds_service_27.seed));
	uds_context->uds_service_27.delay_timer = uds_timer_alloc(delay_timer_callback, REFUSE_REQUEST_TIME, 0, 0);
	uds_timer_set_userdata(uds_context->uds_service_27.delay_timer, uds_context);
}
