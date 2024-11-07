#include "uds.h"
#include "uds_timer.h"

static void detect_timer_callback(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);

	/* DTC检测已关闭 */
	if (uds_context->uds_service_85.type == DTC_SETTING_TYPE_OFF) {
		return;
	}
}

void uds_dtc_detect_init(struct uds_context *uds_context)
{
	uds_context->uds_dtc_detection.detect_timer = uds_timer_alloc(detect_timer_callback, 100, 0, 0);
	uds_timer_set_userdata(uds_context->uds_dtc_detection.detect_timer, uds_context);
	uds_timer_start(uds_context->loop, uds_context->uds_dtc_detection.detect_timer);
}
