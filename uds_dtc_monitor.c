#include "uds.h"
#include "uds_timer.h"
#include <stdlib.h>
#include <json-c/json.h>

typedef struct uds_dtc_handler {
	uint32_t dtc;
	int (*monitor)(uds_context_t *uds_context, uds_dtc_t *dtc);
} uds_dtc_handler_t;

/* 输入电压过低 */
int uds_dtc_monitor_A20016(uds_context_t *uds_context, uds_dtc_t *dtc)
{
	return 0;
}

/* 输入电压过高 */
int uds_dtc_monitor_A20017(uds_context_t *uds_context, uds_dtc_t *dtc)
{
	return 0;
}

static uds_dtc_handler_t __gs_dtc_handlers__[] = {
	{0xA20016, uds_dtc_monitor_A20016},
	{0xA20017, uds_dtc_monitor_A20017},
};

static void detect_timer_callback(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;

	/* DTC检测已关闭 */
	if (uds_context->uds_service_85.type == DTC_SETTING_TYPE_OFF) {
		return;
	}

	/* 检测 */
	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		for (size_t j = 0; j < ARRAYSIZE(__gs_dtc_handlers__); j++) {
			if (__gs_dtc_handlers__[j].dtc == uds_dtc_monitor->dtcs[i].dtc_num && __gs_dtc_handlers__[j].monitor) {
				__gs_dtc_handlers__[j].monitor(uds_context, &uds_dtc_monitor->dtcs[i]);
			}
		}
	}
}

static const char *dtc_group_type(uds_dtc_t *dtc)
{
	logd("dtc.dtc.DTCHighByte:%02x\n", dtc->dtc.DTCHighByte >> 6);
	switch (dtc->dtc.DTCHighByte >> 6) {
		case 0x00: return "P";
		case 0x01: return "C";
		case 0x02: return "B";
		case 0x03: return "U";
	}

	return " ";
}

static void uds_dtc_dump(uds_context_t *uds_context)
{
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		logd("DTC Number:0x%X, DTC:%s%x, status:0x%02x\n", \
				uds_dtc_monitor->dtcs[i].dtc_num, \
				dtc_group_type(&uds_dtc_monitor->dtcs[i]), \
				uds_dtc_monitor->dtcs[i].dtc_num & 0x003fffff, \
				uds_dtc_monitor->dtcs[i].status);
		logd("-------------------------------------------------------------------\n");
	}
}

static void uds_dtc_parse(uds_context_t *uds_context, const char *filename)
{
	struct json_object *obj = json_object_from_file(filename);
	uds_assert(obj, "%s is not valid json", filename);

	struct json_object *dtcs_obj = json_object_object_get(obj, "dtcs");
	uds_assert(dtcs_obj, "can not find dtcs in %s", filename);
	uds_assert(json_object_is_type(dtcs_obj, json_type_array), "dtcs is not array");

	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;
	uds_dtc_monitor->count = json_object_array_length(dtcs_obj);
	if (uds_dtc_monitor->count == 0) {
		goto finish;
	}

	uds_dtc_monitor->dtcs = calloc(uds_dtc_monitor->count, sizeof(*uds_dtc_monitor->dtcs));
	if (!uds_dtc_monitor->dtcs) {
		goto finish;
	}

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		struct json_object *item = json_object_array_get_idx(dtcs_obj, i);
		uds_assert(json_object_is_type(item, json_type_object), "item is not valid json");

		struct json_object *dtc_obj = json_object_object_get(item, "dtc");
		struct json_object *desc_obj = json_object_object_get(item, "desc");
		struct json_object *monitor_obj = json_object_object_get(item, "monitor_rate");

		uds_assert(json_object_is_type(dtc_obj, json_type_string), "dtc is not string");
		uds_assert(json_object_is_type(desc_obj, json_type_string), "desc is not string");
		uds_assert(json_object_is_type(monitor_obj, json_type_int), "monitor_rate is not int");

		uint32_t dtc = strtoul(json_object_get_string(dtc_obj), NULL, 0x10);
		uds_dtc_monitor->dtcs[i].dtc.DTCHighByte = (dtc >> 16) & 0xff;
		uds_dtc_monitor->dtcs[i].dtc.DTCMiddleByte = (dtc >> 8) & 0xff;
		uds_dtc_monitor->dtcs[i].dtc.DTCLowByte = dtc & 0xff;
		uds_dtc_monitor->dtcs[i].dtc_num = dtc;
	}

	uds_dtc_dump(uds_context);

finish:
	json_object_put(obj);
}

void uds_dtc_monitor_init(struct uds_context *uds_context)
{
	logd("uds_dtc_monitor_init\n");

	uds_dtc_parse(uds_context, UDS_CONFIG_FILE);
	uds_context->uds_dtc_monitor.monitor_timer = uds_timer_alloc(detect_timer_callback, 100, 0, 0);
	uds_timer_set_userdata(uds_context->uds_dtc_monitor.monitor_timer, uds_context);
	uds_timer_start(uds_context->loop, uds_context->uds_dtc_monitor.monitor_timer);
}
