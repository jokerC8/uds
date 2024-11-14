#include "uds.h"
#include "uds_stream.h"
#include <json-c/json.h>

typedef struct routine_handler {
	uint16_t rid;
	uint8_t type;
	int (*fp)(uds_context_t *uds_context, uint8_t control_type);
} routine_handler_t;

static int routine_handler_fe01_01(uds_context_t *uds_context, uint8_t control_type)
{
	uds_stream_t strm = {0};
	uds_response_t *uds_response = &uds_context->uds_response;

	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_byte(&strm, 0x00);
	return NRC_PositiveRespon_00;
}

static int routine_handler_fe01_03(uds_context_t *uds_context, uint8_t control_type)
{
	uds_stream_t strm = {0};
	uds_response_t *uds_response = &uds_context->uds_response;

	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_byte(&strm, 0x00);
	return NRC_PositiveRespon_00;
}

static routine_handler_t __gs_routine_handler_table__[] = {
	{0xfe01, 0x01, routine_handler_fe01_01},
	{0xfe01, 0x03, routine_handler_fe01_03},
};

int uds_service_31_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint16_t rid;
	uint8_t sid, control_type;
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_service_31_routine_t *routine = NULL;
	uds_service_31_t *uds_service_31 = &uds_context->uds_service_31;

	if (len < 4) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	control_type = uds_stream_read_byte(&strm);
	rid = uds_stream_read_be16(&strm);
	for (int i = 0; i < uds_service_31->count; i++) {
		if (uds_service_31->routines[i].rid == rid) {
			routine = &uds_service_31->routines[i];
			break;
		}
	}

	/* RID不支持 */
	if (!routine) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	size_t idx = 0;
	for (; idx < ARRAYSIZE(routine->control_type); idx++) {
		if (routine->control_type[idx].type == control_type) {
			break;
		}
	}

	/* control_type不支持 */
	if (idx == ARRAYSIZE(routine->control_type)) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 长度不合法 */
	if ((int)routine->control_type[idx].len != (len - 4)) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uint8_t pass = FALSE;
	for (size_t i = 0; i < ARRAYSIZE(routine->control_type[idx].sessions); i++) {
		if (uds_diagnostic_session(uds_context) == routine->control_type[idx].sessions[i]) {
			pass = TRUE;
			break;
		}
	}
	/* 当前会话模式下不支持 */
	if (!pass) {
		nrc = routine->control_type[idx].session_nrc;
		goto finish;
	}

	pass = FALSE;
	for (size_t i = 0; i < ARRAYSIZE(routine->control_type[idx].security_access_levels); i++) {
		if (uds_diagnostic_session(uds_context) == routine->control_type[idx].security_access_levels[i]) {
			pass = TRUE;
			break;
		}
	}
	/* 当前安全级别下不支持 */
	if (!pass) {
		nrc = routine->control_type[idx].security_access_nrc;
		goto finish;
	}

	for (size_t i = 0; i < ARRAYSIZE(__gs_routine_handler_table__); i++) {
		if (__gs_routine_handler_table__[i].rid == routine->rid && __gs_routine_handler_table__[i].type == control_type) {
			if (__gs_routine_handler_table__[i].fp) {
				nrc = __gs_routine_handler_table__[i].fp(uds_context, control_type);
			}
			else {
				nrc = NRC_ConditionsNotCorrect_22;
			}
			break;
		}
	}

finish:
	uds_context->nrc = nrc;
	return nrc;
}

static void uds_service_31_routine_dump(uds_context_t *uds_context)
{
	char buffer[64] = {0};
	uds_service_31_t *uds_service_31 = &uds_context->uds_service_31;

	for (int i = 0; i < uds_service_31->count; i++) {
		logd("RID:0x%x\n", uds_service_31->routines[i].rid);
		for (size_t j = 0; j < ARRAYSIZE(uds_service_31->routines[i].control_type); j++) {
			logd("desc:%s\n", uds_service_31->routines[i].control_type[j].desc);
			logd("type:%d\n", uds_service_31->routines[i].control_type[j].type);
			logd("len:%d\n", uds_service_31->routines[i].control_type[j].len);
			for (size_t k = 0; k < ARRAYSIZE(uds_service_31->routines[i].control_type[j].sessions); k++) {
				snprintf(buffer, ARRAYSIZE(buffer), "%d ", uds_service_31->routines[i].control_type[j].sessions[k]);
			}
			logd("sessions:[ %s]\n", buffer);
			for (size_t k = 0; k < ARRAYSIZE(uds_service_31->routines[i].control_type[j].security_access_levels); k++) {
				snprintf(buffer, ARRAYSIZE(buffer), "%d ", uds_service_31->routines[i].control_type[j].security_access_levels[k]);
			}
			logd("security_access_levels:[ %s]\n", buffer);
			logd("session_nrc:0x%02x\n", uds_service_31->routines[i].control_type[j].session_nrc);
			logd("security_access_nrc:0x%02x\n", uds_service_31->routines[i].control_type[j].security_access_nrc);
		}
		logd("-------------------------------------------------------------------\n");
	}
}

static void uds_service_31_routine_parse(uds_context_t *uds_context, const char *filename)
{
	struct json_object *obj = json_object_from_file(filename);
	uds_assert(obj, "%s not valid json", filename);

	struct json_object *routines_obj = json_object_object_get(obj, "sid_31");
	if (!routines_obj) {
		loge("can not find sid_31 is %s\n", filename);
		return;
	}

	int count = json_object_array_length(routines_obj);
	uds_context->uds_service_31.count = count;
	uds_context->uds_service_31.routines = calloc(count, sizeof(*uds_context->uds_service_31.routines));
	uds_assert(uds_context->uds_service_31.routines, "malloc failed");

	for (int i = 0; i < count; i++) {
		struct json_object *item = json_object_array_get_idx(routines_obj, i);
		struct json_object *rid_obj = json_object_object_get(item, "rid");
		struct json_object *control_type_obj = json_object_object_get(item, "control_type");
		uds_assert(json_object_is_type(rid_obj, json_type_string), "rid is not string");
		uds_assert(json_object_is_type(control_type_obj, json_type_array), "control_type is not array");

		uds_context->uds_service_31.routines[i].rid = strtoul(json_object_get_string(rid_obj), NULL, 0x10);
		int control_type_cnt = json_object_array_length(control_type_obj);
		uds_assert(control_type_cnt <= 2, "control_type_cnt > 2 is invalid");

		for (int k = 0; k < control_type_cnt; k++) {
			struct json_object *control_type_item = json_object_array_get_idx(control_type_obj, k);
			uds_assert(json_object_is_type(control_type_item, json_type_object), "type is not obj");

			struct json_object *type_obj = json_object_object_get(control_type_item, "type");
			struct json_object *len_obj = json_object_object_get(control_type_item, "len");
			struct json_object *desc_obj = json_object_object_get(control_type_item, "desc");
			struct json_object *session_nrc_obj = json_object_object_get(control_type_item, "session_nrc");
			struct json_object *security_access_nrc_obj = json_object_object_get(control_type_item, "security_access_nrc");
			struct json_object *sessions_obj = json_object_object_get(control_type_item, "sessions");
			struct json_object *security_access_levels_obj = json_object_object_get(control_type_item, "security_access_levels");
			uds_assert(json_object_is_type(type_obj, json_type_int), "type is not int");
			uds_assert(json_object_is_type(len_obj, json_type_int), "len is not int");
			uds_assert(json_object_is_type(desc_obj, json_type_string), "desc is not string");
			uds_assert(json_object_is_type(session_nrc_obj, json_type_string), "session_nrc is not string");
			uds_assert(json_object_is_type(security_access_nrc_obj, json_type_string), "security_access_nrc is not string");
			uds_assert(json_object_is_type(sessions_obj, json_type_array), "sessions is not array");
			uds_assert(json_object_is_type(security_access_levels_obj, json_type_array), "security_access_levels is not array");

			uds_context->uds_service_31.routines[i].control_type[k].type = json_object_get_int(type_obj);
			uds_context->uds_service_31.routines[i].control_type[k].len = json_object_get_int(len_obj);
			uds_context->uds_service_31.routines[i].control_type[k].session_nrc =
				strtoul(json_object_get_string(session_nrc_obj), NULL, 0x10);
			uds_context->uds_service_31.routines[i].control_type[k].security_access_nrc =
				strtoul(json_object_get_string(security_access_nrc_obj), NULL, 0x10);
			memcpy(uds_context->uds_service_31.routines[i].control_type[k].desc, json_object_get_string(desc_obj), \
					MIN((int)ARRAYSIZE(uds_context->uds_service_31.routines[i].control_type[k].desc), \
						json_object_get_string_len(desc_obj)));

			int sessions_cnt = json_object_array_length(sessions_obj);
			int security_access_levels_cnt = json_object_array_length(security_access_levels_obj);
			for (int j = 0; j < sessions_cnt; j++) {
				struct json_object *session_item = json_object_array_get_idx(sessions_obj, j);
				uds_assert(json_object_is_type(session_item, json_type_int), "session is not int");
				uds_context->uds_service_31.routines[i].control_type[k].sessions[j] = json_object_get_int(session_item);
			}
			for (int j = 0; j < security_access_levels_cnt; j++) {
				struct json_object *security_access_level_item = json_object_array_get_idx(security_access_levels_obj, j);
				uds_assert(json_object_is_type(security_access_level_item, json_type_int), "security_access_level is not int");
				uds_context->uds_service_31.routines[i].control_type[k].security_access_levels[j] = json_object_get_int(security_access_level_item);
			}
		}
	}
	uds_service_31_routine_dump(uds_context);
	json_object_put(obj);
}

void uds_service_31_init(struct uds_context *uds_context)
{
	logd("uds_service_31_init\n");

	uds_service_31_routine_parse(uds_context, UDS_CONFIG_FILE);
}
