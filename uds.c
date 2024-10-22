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

/* for debug */
static void heartbeat_timer_cb(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);

	logd("%.3fs timeout\n", timer->timeout * 1e-3);
	logd("uds_indication->handler:%d, status:%s\n", uds_context->uds_indication.handler, \
			connection_des(uds_context->uds_indication.status));
	logd("uds_request->handler:%d, status:%s\n", uds_context->uds_request.handler, \
			connection_des(uds_context->uds_request.status));
}

struct uds_context *uds_context_init(const char *conf)
{
	uds_context_t *uds_context = NULL;

	uds_context = malloc(sizeof(*uds_context));
	if (!uds_context) {
		return NULL;
	}

	uds_context->uds_indication.status = Connection_Socket_Uninitialized;
	uds_context->uds_request.status = Connection_Socket_Uninitialized;
	uds_context->uds_indication.sockfile = UDS_RECEIVER_SOCKFILE;
	uds_context->uds_request.sockfile = UDS_SENDER_SOCKFILE;
	uds_context->status = UDS_Context_Uninitialized;

	uds_context->uds_indication.cap = MAX_UDS_PDU_SIZE;
	uds_context->uds_indication.buffer = malloc(uds_context->uds_indication.cap);
	if (!uds_context->uds_indication.buffer) {
		goto nomem;
	}

	uds_context->loop = timer_loop_alloc(16);
	uds_context->heartbeat_timer = uds_timer_alloc(heartbeat_timer_cb, 3000, 3000, 0);
	uds_timer_set_userdata(uds_context->heartbeat_timer, uds_context);
	uds_timer_start(uds_context->loop, uds_context->heartbeat_timer);

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
	memcpy(server.sun_path, uds_indication->sockfile, UDS_MIN(sizeof(server.sun_path) - 1, strlen(uds_indication->sockfile)));

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

static int uds_request_init(uds_context_t *uds_context)
{
	uds_request_t *uds_request = &uds_context->uds_request;

	if (uds_request->status == Connection_Socket_Initialized) {
		return uds_request->handler;
	}

	logd("uds_request_init\n");
	if ((uds_request->handler = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	bzero(&uds_request->target, sizeof(uds_request->target));
	uds_request->target.sun_family = AF_UNIX;
	memcpy(uds_request->target.sun_path, uds_request->sockfile, \
			UDS_MIN(sizeof(uds_request->target.sun_path), strlen(uds_request->sockfile)));

	uds_request->status = Connection_Socket_Initialized;

	return uds_request->handler;
}

static void uds_indication_dispatch(uds_context_t *uds_context)
{
	uds_stream_t strm = {0};
	uds_indication_t *uds_indication = &uds_context->uds_indication;

	uds_stream_init(&strm, uds_indication->buffer, uds_indication->len);
	uds_context->sa = uds_stream_read_be16(&strm);
	uds_context->ta = uds_stream_read_be16(&strm);
	uds_context->ta_type = uds_stream_read_byte(&strm);

	uint8_t sid = uds_stream_read_byte(&strm);

	switch (sid) {
		case 0x10:
			break;
		case 0x11:
			break;
		case 0x14:
			break;
		case 0x19:
			break;
		case 0x22:
			break;
		case 0x27:
			break;
		case 0x28:
			break;
		case 0x2e:
			break;
		case 0x31:
			break;
		case 0x34:
			break;
		case 0x35:
			break;
		case 0x36:
			break;
		case 0x37:
			break;
		case 0x38:
			break;
		case 0x85:
			break;
	}

	uds_hexdump(uds_indication->buffer, uds_indication->len);
}

static void uds_indication_handler(uds_context_t *uds_context)
{
	struct pollfd pollfd;
	uds_indication_t *uds_indication = &uds_context->uds_indication;

	pollfd.fd = uds_indication->handler;
	pollfd.events = POLLIN;

	int count = poll(&pollfd, 1, 100);
	/* timeout */
	if (count == 0) {
		return;
	}
	/* error */
	if (count < 0) {
		uds_indication->status = Connection_Finalization;
		return;
	}
	/* readable */
	count = recv(uds_indication->handler, uds_indication->buffer, uds_indication->cap, 0);
	if (count == 0) {
		return;
	}
	if (count < 0) {
		uds_indication->status = Connection_Finalization;
		return;
	}

	/* SA + TA + TY_type */
	if (count <= 5) {
		return;
	}

	uds_indication->len = count;
	/* recv uds request */
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

static void uds_request_cleanup(uds_context_t *uds_context)
{
	uds_request_t *uds_request = &uds_context->uds_request;

	if (uds_request->status != Connection_Finalization) {
		return;
	}

	if (uds_request->handler > 0) {
		close(uds_request->handler);
	}
	uds_request->handler = -1;
	uds_request->status = Connection_Socket_Uninitialized;
}

/* start uds servers */
void uds_service_start(struct uds_context *uds_context)
{
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
			logd("uds_requestication_init failed(%s)\n", strerror(errno));
			poll(0, 0, 3000);
			continue;
		}

		if (uds_request_init(uds_context) < 0) {
			loge("uds_request_init failed(%s)\n", strerror(errno));
			poll(0, 0, 3000);
			continue;
		}

		/* process uds request */
		uds_indication_handler(uds_context);

		/* do some socket cleanup if needed */
		uds_indication_cleanup(uds_context);

		/* do some socket cleanup if needed */
		uds_request_cleanup(uds_context);

		/* uds timer main loop */
		uds_timer_loop(uds_context->loop);
	}

	uds_context->start = 0;
}

void uds_service_stop(struct uds_context *uds_context)
{
	if (uds_context) {
		uds_context->quit = 1;
	}
}

void uds_context_destroy(struct uds_context *uds_context)
{
	/* TODO */
}
