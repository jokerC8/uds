#include "uds.h"
#include "uds_timer.h"
#include <stdlib.h>
#include <json-c/json.h>

typedef struct uds_dtc_handler {
	uint32_t dtc;
	int (*monitor)(uds_context_t *uds_context, uds_dtc_t *dtc);
} uds_dtc_handler_t;

/* 1. DTC_Confirmed时只保存一次Global Snapshot
 * 2. DTC_TestFailed时每次都更新Local Snapshot
 */
static void update_global_dtc_record(uds_dtc_t *dtc)
{
	if (!dtc->global_snapshot.valid) {
		bzero(&dtc->global_snapshot, sizeof(dtc->global_snapshot));

		dtc->global_snapshot.valid = 1;
		dtc->global_snapshot.record_num = GLOBAL_RECORD_NUMBER;
		dtc->global_snapshot.power_supply_vol_did = 0xB000;
		dtc->global_snapshot.power_supply_vol = 0;
		dtc->global_snapshot.vehile_speed_did = 0xB001;
		dtc->global_snapshot.vehile_speed = 0;
		dtc->global_snapshot.odometer_id = 0xB004;
		dtc->global_snapshot.odometer = 0;
		dtc->global_snapshot.timestamp_id = 0xB005;
	}
}

static void update_local_dtc_record(uds_dtc_t *dtc)
{
	bzero(&dtc->local_snapshot, sizeof(dtc->local_snapshot));
	dtc->local_snapshot.valid = 1;
	dtc->local_snapshot.record_num = LOCAL_RECORD_NUMBER;
	dtc->local_snapshot.power_supply_vol_did = 0xB000;
	dtc->local_snapshot.power_supply_vol = 0;
	dtc->local_snapshot.vehile_speed_did = 0xB001;
	dtc->local_snapshot.vehile_speed = 0;
	dtc->local_snapshot.odometer_id = 0xB004;
	dtc->local_snapshot.odometer = 0;
	dtc->local_snapshot.timestamp_id = 0xB005;
}

static void test_failed_confirmed(uds_dtc_t *dtc)
{
	if (!(dtc->statusOfDTC & DTC_ConfirmedDTC)) {
		update_global_dtc_record(dtc);
		dtc->statusOfDTC |= DTC_ConfirmedDTC;
	}
}

static void test_pass(uds_dtc_t *dtc)
{
	dtc->statusOfDTC &= ~(DTC_TestFailed);
}

static void test_failed(uds_dtc_t *dtc)
{
	dtc->statusOfDTC |= DTC_TestFailed;
	++dtc->extended_data.occurrence_counter;
	update_local_dtc_record(dtc);
}

int __uds_dtc_monitor(uds_context_t *uds_context, uds_dtc_t *dtc)
{
	/* 未到检测时间 */
	if (++dtc->counter < dtc->count) {
		return 0;
	}

	dtc->counter = 0;

	/* 检测到故障码 */
	if (dtc->monitor(uds_context, dtc)) {
		test_failed(dtc);
		dtc->monitor_counter++;
		if (++dtc->monitor_counter == dtc->monitor_count) {
			dtc->monitor_counter = 0;
			test_failed_confirmed(dtc);
		}
	}
	else {
		dtc->monitor_counter = 0;
		test_pass(dtc);
	}

	return 0;
}

static int uds_dtc_monitor_stub(uds_context_t *uds_context, uds_dtc_t *dtc)
{
	return FALSE;
}

/* 输入电压过低 */
int uds_dtc_monitor_A20016(uds_context_t *uds_context, uds_dtc_t *dtc)
{
	return FALSE;
}

/* 输入电压过高 */
int uds_dtc_monitor_A20017(uds_context_t *uds_context, uds_dtc_t *dtc)
{
	return FALSE;
}

static uds_dtc_handler_t __gs_dtc_handlers__[] = {
	{0xA20016, uds_dtc_monitor_A20016},
	{0xA20017, uds_dtc_monitor_A20017},
};

static void monitor_timer_callback(struct timer_loop *loop, struct uds_timer *timer)
{
	uds_context_t *uds_context = uds_timer_userdata(timer);
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;

	/* DTC检测已关闭参考(uds_service_85.c) */
	if (uds_context->uds_service_85.type == DTC_SETTING_TYPE_OFF) {
		return;
	}

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		__uds_dtc_monitor(uds_context, &uds_dtc_monitor->dtcs[i]);
	}
}

static const char *dtc_group_type(uds_dtc_t *dtc)
{
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
		logd("desc:%s\n", uds_dtc_monitor->dtcs[i].desc);
		logd("DTC_Num: 0x%X, DTC:%s%x\n", uds_dtc_monitor->dtcs[i].DTC_Num, \
				dtc_group_type(&uds_dtc_monitor->dtcs[i]), uds_dtc_monitor->dtcs[i].DTC_Num & 0x003fffff);
		logd("status:0x%02x\n", uds_dtc_monitor->dtcs[i].statusOfDTC);
		logd("monitor_rate:%d\n", uds_dtc_monitor->dtcs[i].monitor_rate);
		logd("times:%d\n", uds_dtc_monitor->dtcs[i].monitor_count);
		logd("-------------------------------------------------------------------\n");
	}
}

static void uds_dtc_parse(uds_context_t *uds_context, const char *filename)
{
	struct json_object *obj = json_object_from_file(filename);
	uds_assert(obj, "%s is not valid json", filename);

	struct json_object *dtcs_obj = json_object_object_get(obj, "dtcs");
	if (!dtcs_obj) {
		loge("can not find dtcs in %s\n", filename);
		return;
	}

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
		struct json_object *monitor_count_obj = json_object_object_get(item, "monitor_count");

		uds_assert(json_object_is_type(dtc_obj, json_type_string), "dtc is not string");
		uds_assert(json_object_is_type(desc_obj, json_type_string), "desc is not string");
		uds_assert(json_object_is_type(monitor_obj, json_type_int), "monitor_rate is not int");
		uds_assert(json_object_is_type(monitor_count_obj, json_type_int), "monitor_count is not int");

		uint32_t dtc = strtoul(json_object_get_string(dtc_obj), NULL, 0x10);
		uds_dtc_monitor->dtcs[i].dtc.DTCHighByte = (dtc >> 16) & 0xff;
		uds_dtc_monitor->dtcs[i].dtc.DTCMiddleByte = (dtc >> 8) & 0xff;
		uds_dtc_monitor->dtcs[i].dtc.DTCLowByte = dtc & 0xff;
		uds_dtc_monitor->dtcs[i].DTC_Num = dtc;
		uds_dtc_monitor->dtcs[i].monitor_count = json_object_get_int(monitor_count_obj);
		uds_dtc_monitor->dtcs[i].monitor_rate = json_object_get_int(monitor_obj);
		strncpy(uds_dtc_monitor->dtcs[i].desc, json_object_get_string(desc_obj), \
				MIN((int)ARRAYSIZE(uds_dtc_monitor->dtcs[i].desc), json_object_get_string_len(desc_obj)));

		/* 放个桩函数,防止空函数指针 */
		uds_dtc_monitor->dtcs[i].monitor = uds_dtc_monitor_stub;
		/* 以基准定时器为周期,计算超时几次需要检测一次DTC */
		uds_dtc_monitor->dtcs[i].count = uds_dtc_monitor->dtcs[i].monitor_rate / MONITOR_BASE_TIMER;
		if (uds_dtc_monitor->dtcs[i].count == 0) {
			++uds_dtc_monitor->dtcs[i].count;
		}
	}

	/* 为每个DTC映射对应的检测函数 */
	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		for (size_t k = 0; k < ARRAYSIZE(__gs_dtc_handlers__); k++) {
			if (__gs_dtc_handlers__[k].dtc == uds_dtc_monitor->dtcs[i].DTC_Num) {
				uds_dtc_monitor->dtcs[i].monitor = __gs_dtc_handlers__[k].monitor;
			}
		}
	}

	uds_dtc_dump(uds_context);

finish:
	json_object_put(obj);
}

void uds_dtc_monitor_init(struct uds_context *uds_context)
{
	logd("uds_dtc_monitor_init\n");

	uds_dtc_parse(uds_context, UDS_CONFIG_FILE);
	uds_context->uds_dtc_monitor.DTCStatusAvailabilityMask = DTC_STATUS_AVAILABILITY_MASK;
	uds_context->uds_dtc_monitor.monitor_timer = uds_timer_alloc(monitor_timer_callback, MONITOR_BASE_TIMER, 0, 0);
	uds_timer_set_userdata(uds_context->uds_dtc_monitor.monitor_timer, uds_context);
	uds_timer_start(uds_context->loop, uds_context->uds_dtc_monitor.monitor_timer);
}
