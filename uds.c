#include "uds.h"
#include "uds_timer.h"
#include "uds_stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>

enum {
	Connection_Socket_Uninitialized,
	Connection_Socket_Initialized,
	Connection_Finalization,
};  /* DoIP Connection Status */

enum {
	UDS_Context_Uninitialized,
	UDS_Context_Initialized,
};

static char * connection_des(int status)
{
	switch (status) {
		case Connection_Socket_Uninitialized:
			return "Connection_Socket_Uninitialized";
		case Connection_Socket_Initialized:
			return "Connection_Socket_Initialized";
		case Connection_Finalization:
			return "Connection_Finalization";
		default:
			return "unknow";
	}
}

static const char *uds_service_desc(uds_context_t *uds_context)
{
	switch (uds_context->sid) {
		case UDS_Service_Session_Control_10:
			return "Session Control";
		case UDS_Service_ECU_Reset_11:
			return "ECU Reset";
		case UDS_Service_Clear_DTC_14:
			return "Clear DTC";
		case UDS_Service_Read_DTC_19:
			return "Read DTC";
		case UDS_Service_Read_Identifier_22:
			return "Read Identifier";
		case UDS_Service_Security_Control_27:
			return "Security Control";
		case UDS_Service_Communication_Control_28:
			return "Communication Control";
		case UDS_Service_Write_Identifier_2e:
			return "Write Identifier";
		case UDS_Service_Input_Output_Control_2f:
			return "Input Output Control";
		case UDS_Service_Routine_Control_31:
			return "Routine Control";
		case UDS_Service_Request_Download_34:
			return "Request Download";
		case UDS_Service_Transfer_Data_36:
			return "Transfer Data";
		case UDS_Service_Request_Data_Exit_37:
			return "Request Data Exit";
		case UDS_Service_Request_File_Transfer_38:
			return "Request File Transfer";
		case UDS_Service_Tester_Present_3e:
			return "Tester Present";
		case UDS_Service_DTC_Setting_Control_85:
			return "DTC Setting Control";
		default:
			return "not supported";
	}
}

/* 定时打印一些调试信息 */
static void heartbeat_timer_cb(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);

	logd("%.3fs timeout\n", timer->timeout * 1e-3);
	logd("uds_indication->handler:%d, status:%s\n", uds_context->uds_indication.handler, \
			connection_des(uds_context->uds_indication.status));
	logd("uds_response->handler:%d, status:%s\n", uds_context->uds_response.handler, \
			connection_des(uds_context->uds_response.status));
}

static void nrc78_timer_callback(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);

	uds_timer_stop(loop, timer);
	logd("nrc78_timer timeout(%dms)\n", uds_context->p2xserver);
	if (uds_context->busy) {
		uds_context->busy = UDS_IDLE;
	}
}

struct uds_context *uds_context_alloc()
{
	uds_context_t *uds_context = NULL;

	uds_context = malloc(sizeof(*uds_context));
	if (!uds_context) {
		return NULL;
	}

	uds_context->uds_indication.status = Connection_Socket_Uninitialized;
	uds_context->uds_response.status = Connection_Socket_Uninitialized;
	uds_context->uds_indication.sockfile = UDS_RECEIVER_SOCKFILE;
	uds_context->uds_response.sockfile = UDS_SENDER_SOCKFILE;
	uds_context->status = UDS_Context_Uninitialized;

	uds_context->uds_indication.cap = MAX_UDS_PDU_SIZE;
	uds_context->uds_indication.buffer = malloc(uds_context->uds_indication.cap);
	if (!uds_context->uds_indication.buffer) {
		goto nomem;
	}

	/* 预留首部5字节, 用来保存SA,TA,TA_type */
	uds_context->uds_response.pos = uds_context->uds_response.buffer + 5;
	uds_context->uds_response.cap = sizeof(uds_context->uds_response.buffer) - 5;

	/* 创建定时器主loop */
	uds_context->loop = timer_loop_alloc(16);
	uds_context->heartbeat_timer = uds_timer_alloc(heartbeat_timer_cb, 3000, 0, 0);
	uds_timer_set_userdata(uds_context->heartbeat_timer, uds_context);
	uds_timer_start(uds_context->loop, uds_context->heartbeat_timer);

	/* 创建NRC78 Pending定时器 */
	uds_context->nrc78_timer = uds_timer_alloc(nrc78_timer_callback, uds_context->p2xserver, 0, 0);
	uds_timer_set_userdata(uds_context->nrc78_timer, uds_context);

	/* 初始化P2Server, P2*Server */
	uds_context->p2server = MAX_P2SERVER;
	uds_context->p2xserver = MAX_P2XSERVER;

	/* uds服务初始化放这里 */
	uds_service_10_init(uds_context);
	uds_service_11_init(uds_context);
	uds_service_14_init(uds_context);
	uds_service_19_init(uds_context);
	uds_service_22_init(uds_context);
	uds_service_27_init(uds_context);
	uds_service_28_init(uds_context);
	uds_service_2e_init(uds_context);
	uds_service_31_init(uds_context);
	uds_service_34_init(uds_context);
	uds_service_36_init(uds_context);
	uds_service_37_init(uds_context);
	uds_service_38_init(uds_context);
	uds_service_3e_init(uds_context);
	uds_service_85_init(uds_context);

	uds_context->status = UDS_Context_Initialized;

	return uds_context;

nomem:
	free(uds_context);
	return NULL;
}

static int uds_indication_init(uds_context_t *uds_context)
{
	struct sockaddr_un server;
	uds_indication_t *uds_indication = &uds_context->uds_indication;

	if (uds_indication->status == Connection_Socket_Initialized) {
		return uds_indication->handler;
	}
	logd("uds_indication init\n");

	unlink(uds_indication->sockfile);
	if ((uds_indication->handler = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	bzero(&server, sizeof(server));
	server.sun_family = AF_UNIX;
	memcpy(server.sun_path, uds_indication->sockfile, MIN(sizeof(server.sun_path) - 1, strlen(uds_indication->sockfile)));

	if (bind(uds_indication->handler, (struct sockaddr *)&server, sizeof(server)) < 0) {
		goto finish;
	}

	uds_indication->status = Connection_Socket_Initialized;
	return uds_indication->handler;

finish:
	close(uds_indication->handler);
	uds_indication->handler = -1;
	return -1;
}

static int uds_response_init(uds_context_t *uds_context)
{
	uds_response_t *uds_response = &uds_context->uds_response;

	if (uds_response->status == Connection_Socket_Initialized) {
		return uds_response->handler;
	}

	logd("uds_response_init\n");
	if ((uds_response->handler = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	bzero(&uds_response->target, sizeof(uds_response->target));
	uds_response->target.sun_family = AF_UNIX;
	memcpy(uds_response->target.sun_path, uds_response->sockfile, \
			MIN(sizeof(uds_response->target.sun_path), strlen(uds_response->sockfile)));

	uds_response->status = Connection_Socket_Initialized;

	return uds_response->handler;
}

static size_t uds_service_respon(uds_context_t *uds_context)
{
	uds_stream_t strm = {0};
	uds_response_t *uds_response = &uds_context->uds_response;

	if (uds_response->status != Connection_Socket_Initialized) {
		return 0;
	}

	uds_stream_init(&strm, uds_response->buffer, sizeof(uds_response->buffer));
	uds_stream_write_be16(&strm, uds_context->sa);
	uds_stream_write_be16(&strm, uds_context->ta);
	uds_stream_write_byte(&strm, uds_context->ta_type);

	/* 负响应 */
	if (uds_context->nrc != NRC_PositiveRespon_00){
		uds_stream_write_byte(&strm, 0x7f);
		uds_stream_write_byte(&strm, uds_context->sid);
		uds_stream_write_byte(&strm, uds_context->nrc);
		uds_response->len = uds_stream_len(&strm);
	}
	/* 正响应 */
	else if (!uds_response->spr) {
		uds_response->len += uds_stream_len(&strm);
	}
	/* 抑制正响应 */
	else {
		return 0;
	}

	uds_hexdump(uds_stream_start_ptr(&strm), uds_response->len);
	return sendto(uds_response->handler, uds_response->buffer, uds_response->len, 0, \
			(struct sockaddr *)&uds_response->target, sizeof(uds_response->target));
}

/* 处理uds新请求前清空response长度和spr标志 */
static void reset_uds_response(uds_context_t *uds_context)
{
	uds_context->uds_response.len = 0;
	uds_context->uds_response.spr = 0;
}

static void uds_indication_dispatch(uds_context_t *uds_context)
{
	uds_stream_t strm = {0};
	uds_indication_t *uds_indication = &uds_context->uds_indication;

	uds_stream_init(&strm, uds_indication->buffer, uds_indication->len);
	uds_context->sa = uds_stream_read_be16(&strm);
	uds_context->ta = uds_stream_read_be16(&strm);
	uds_context->ta_type = uds_stream_read_byte(&strm);

	uds_context->sid = uds_stream_read_byte(&strm);

	if (uds_context->busy) {
		logd("uds busy\n");
		return;
	}

	reset_uds_response(uds_context);

	uds_context->busy = UDS_BUSY;

	/* uds服务推荐NRC优先级顺序如下,如车厂有自己要求则需要更改
	 * -> NRC_ServiceNotSupported_11
	 * -> NRC_ServiceNotSupportedInActiveSession_7f
	 * -> NRC_IncorrectMessageLengthOrInvalidFormat_13
	 * -> NRC_SubFunctionNotSupported_12
	 * -> NRC_SubFunctionNotSupportedInActiveSession_7e
	 * -> NRC_SecurityAccessDenied_33
	 * -> NRC_RequestSequenceError_24
	 * -> NRC_RequestOutOfRange_31
	 * ->NRC_ConditionsNotCorrect_22
	 * -> NRC_RequestCorrentlyReceivedResponPending_78
	 */

	/* NRC11,NRC7f在uds请求分发前可统一处理, 剩余NRC在各自服务中处理比较好 */
	if (!uds_service_verify(uds_context)) {
		uds_context->nrc = NRC_ServiceNotSupported_11;
		goto finish;
	}
	if (!uds_service_session_verify(uds_context)) {
		uds_context->nrc = NRC_ServiceNotSupportedInActiveSession_7f;
		goto finish;
	}

	/* 检查服务是否支持功能地址 */
	if (!uds_service_TAtype_filter(uds_context)) {
		uds_context->nrc = NRC_ConditionsNotCorrect_22;
		goto finish;
	}

	switch (uds_context->sid) {
		case UDS_Service_Session_Control_10:
			uds_service_10_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_ECU_Reset_11:
			uds_service_11_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Clear_DTC_14:
			uds_service_14_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Read_DTC_19:
			uds_service_19_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Read_Identifier_22:
			uds_service_22_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Security_Control_27:
			uds_service_27_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Communication_Control_28:
			uds_service_28_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Write_Identifier_2e:
			uds_service_2e_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Input_Output_Control_2f:
			uds_service_2f_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Routine_Control_31:
			uds_service_31_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Request_Download_34:
			uds_service_34_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Transfer_Data_36:
			uds_service_36_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Request_Data_Exit_37:
			uds_service_37_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Request_File_Transfer_38:
			uds_service_38_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_Tester_Present_3e:
			uds_service_3e_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		case UDS_Service_DTC_Setting_Control_85:
			uds_service_85_handler(uds_context, uds_indication->buffer + 5, uds_indication->len - 5);
			break;
		default:
			/* 服务不支持 */
			logd("sid(0x%02x) not supported\n", uds_context->sid);
			uds_context->nrc = NRC_ServiceNotSupported_11;
			break;
	}

finish:
	logd("sid(0x%02x) %s\n", uds_context->sid, uds_service_desc(uds_context));
	uds_hexdump(uds_indication->buffer, uds_indication->len);

	uds_service_respon(uds_context);

	/* 非NRC78, uds处理流程结束 */
	if (uds_context->nrc != NRC_RequestCorrentlyReceivedResponPending_78) {
		uds_context->busy = UDS_IDLE;
		return;
	}

	/* NRC78期间不允许处理新的uds请求, 除非ecu处理完uds请求后主动停止NRC78定时器或者NRC78定时器自己超时 */
	if (!uds_timer_running(uds_context->nrc78_timer)) {
		uds_context->nrc78_timer->timeout = uds_context->p2xserver;
		uds_timer_start(uds_context->loop, uds_context->nrc78_timer);
	}
}

static void uds_indication_handler(uds_context_t *uds_context)
{
	struct pollfd pollfd;
	uds_indication_t *uds_indication = &uds_context->uds_indication;

	pollfd.fd = uds_indication->handler;
	pollfd.events = POLLIN;

	int count = poll(&pollfd, 1, 100);
	/* 超时 */
	if (count == 0) {
		return;
	}
	/* 系统错误 */
	if (count < 0) {
		uds_indication->status = Connection_Finalization;
		return;
	}
	/* 数据可读 */
	count = recv(uds_indication->handler, uds_indication->buffer, uds_indication->cap, 0);
	if (count == 0) {
		return;
	}
	if (count < 0) {
		uds_indication->status = Connection_Finalization;
		return;
	}

	/* SA + TA + TY_type + UDS(至少1字节sid), 后续解析至少可以先解析出sid */
	if (count < 6) {
		return;
	}

	uds_indication->len = count;

	/* uds请求解析并分发 */
	uds_indication_dispatch(uds_context);
}

static void uds_indication_cleanup(uds_context_t *uds_context)
{
	uds_indication_t *uds_indication = &uds_context->uds_indication;

	if (uds_indication->status != Connection_Finalization) {
		return;
	}

	if (uds_indication->handler > 0) {
		close(uds_indication->handler);
	}
	uds_indication->handler = -1;
	uds_indication->status = Connection_Socket_Uninitialized;
}

static void uds_response_cleanup(uds_context_t *uds_context)
{
	uds_response_t *uds_response = &uds_context->uds_response;

	if (uds_response->status != Connection_Finalization) {
		return;
	}

	if (uds_response->handler > 0) {
		close(uds_response->handler);
	}
	uds_response->handler = -1;
	uds_response->status = Connection_Socket_Uninitialized;
}

/* 启动uds服务 */
void uds_service_start(struct uds_context *uds_context)
{
	uds_assert(uds_context, "uds_context is NULL");

	if (uds_context->status == UDS_Context_Uninitialized) {
		loge("uds context uninitialized\n");
		return;
	}

	if (uds_context->start) {
		logd("uds service already started\n");
		return;
	}

	while (!uds_context->quit) {

		uds_context->start = 1;

		if (uds_indication_init(uds_context) < 0) {
			logd("uds_responseication_init failed(%s)\n", strerror(errno));
			poll(0, 0, 3000);
			continue;
		}

		if (uds_response_init(uds_context) < 0) {
			loge("uds_response_init failed(%s)\n", strerror(errno));
			poll(0, 0, 3000);
			continue;
		}

		/* 接收并处理uds请求 */
		uds_indication_handler(uds_context);

		/* 做一些清理操作, 如socket异常清理 */
		uds_indication_cleanup(uds_context);

		/* 做一些清理操作, 如socket异常清理 */
		uds_response_cleanup(uds_context);

		/* 定时器loop */
		uds_timer_loop(uds_context->loop);
	}

	uds_context->start = 0;
}

/* 停住uds服务 */
void uds_service_stop(struct uds_context *uds_context)
{
	if (uds_context) {
		uds_context->quit = 1;
	}
}

/* 做一些清理 */
void uds_context_destroy(struct uds_context *uds_context)
{
	/* TODO */
}
