#include "uds.h"
#include "uds_timer.h"
#include "uds_stream.h"

static void s3_timer_callback(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);

	logd("S3Server timeout (%d)ms, switch to default Session\n", uds_context->uds_service_10.S3Server);
	uds_context->uds_service_10.session = UDS_DEFAULT_SESSION;
	uds_timer_stop(loop, timer);

	/* 1- 恢复DTC设置
	 * 2- 重新锁定ECU
	 * 3- 清空IO控制
	 * 4- 清楚升级状态
	 * 5- 报文收发控制
	 */
	uds_service_27_lock_ecu(uds_context);
	uds_service_2f_io_reset(uds_context);
	uds_service_85_dtc_setting_on(uds_context);
	uds_service_28_communicate_control_reset(uds_context);
}

UDS_Session_E uds_diagnostic_session(struct uds_context *uds_context)
{
	uds_assert(uds_context, "uds_context is NULL");
	return uds_context->uds_service_10.session;
}

void maintain_diagnostic_mode(struct uds_context *uds_context)
{
	uds_service_10_t *uds_service_10 = &uds_context->uds_service_10;
	uds_assert(uds_context, "uds_context is NULL");

	/* 默认会话下无需维持 */
	if (uds_diagnostic_session(uds_context) == UDS_DEFAULT_SESSION) {
		return;
	}

	uds_assert(uds_timer_running(uds_service_10->s3_timer), "s3_timer not running");
	uds_service_10->s3_timer->timeout = uds_service_10->S3Server;
	uds_timer_start(uds_context->loop, uds_service_10->s3_timer);
}

int uds_service_10_handler(struct uds_context *uds_context, uint8_t *uds, int len)
{
	uds_stream_t strm = {0};
	uint8_t sid, sub, session, nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;
	uds_service_10_t *uds_service_10 = &uds_context->uds_service_10;

	if (len != 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	sub = uds_stream_read_byte(&strm);
	session = Acquire_Sub_Function(sub);

	/* session非变化, 直接返回正响应, 非默认会话模式需要更新S3定时器 */
	if (uds_service_10->session == session) {
		if ((session == UDS_PROGRAMMING_SESSION) || (session == UDS_EXTEND_SESSION)) {
			uds_service_10->s3_timer->timeout = uds_service_10->S3Server;
			uds_timer_start(uds_context->loop, uds_service_10->s3_timer);
		}
		goto finish;
	}

	/* 会话模式改变, 重新锁定ECU */
	uds_service_27_lock_ecu(uds_context);

	/* 默认会话模式 -> 默认会话模式或拓展会话模式
	 * 编程会话模式 -> 编程会话模式或默认会话模式
	 * 拓展会话模式 -> 拓展会话,编程会话,默认会话模式
	 */
	if ((session == UDS_PROGRAMMING_SESSION && uds_service_10->session == UDS_DEFAULT_SESSION) || \
		(session == UDS_EXTEND_SESSION && uds_service_10->session == UDS_PROGRAMMING_SESSION)) {
		nrc = NRC_ConditionsNotCorrect_22;
		goto finish;
	}

	uds_service_10->session = session;

	/* 切换回默认会话, 需要重新锁定ECU, 解除DTC设置, 解除输出输出设置等等...
	 * 切到编程模式或拓展会话模式, 需启动S3定时器, tester可通过3e服务让ECU
	 * 维持在某个非默认会话模式
	 */
	if (session == UDS_DEFAULT_SESSION) {
		uds_service_10->s3_timer->timeout = 100;
		uds_timer_start(uds_context->loop, uds_service_10->s3_timer);
	}
	else {
		uds_service_10->s3_timer->timeout = uds_service_10->S3Server;
		uds_timer_start(uds_context->loop, uds_service_10->s3_timer);
	}

finish:
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, session);
		uds_stream_write_be16(&strm, uds_context->p2server);
		uds_stream_write_be16(&strm, uds_context->p2xserver / 10);
	}

	uds_response->len = uds_stream_len(&strm);
	uds_context->nrc = nrc;
	return nrc;
}

void uds_service_10_init(struct uds_context *uds_context)
{
	logd("uds_service_10_init\n");

	uds_context->uds_service_10.session = UDS_DEFAULT_SESSION;
	uds_context->uds_service_10.S3Server = MAX_S3SERVER_MS;
	uds_context->uds_service_10.s3_timer = uds_timer_alloc(s3_timer_callback, uds_context->uds_service_10.S3Server, 0, 0);
	uds_timer_set_userdata(uds_context->uds_service_10.s3_timer, uds_context);
	uds_assert(uds_context->uds_service_10.s3_timer, "uds_timer_alloc failed");
}
