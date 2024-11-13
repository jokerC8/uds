#include "uds.h"
#include "uds_stream.h"
#include <json-c/json.h>

int uds_service_31_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	return 0;
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
			struct json_object *sessions_obj = json_object_object_get(control_type_item, "sessions");
			struct json_object *security_access_levels_obj = json_object_object_get(control_type_item, "security_access_levels");
			uds_assert(json_object_is_type(type_obj, json_type_int), "type is not int");
			uds_assert(json_object_is_type(len_obj, json_type_int), "len is not int");
			uds_assert(json_object_is_type(desc_obj, json_type_string), "desc is not string");
			uds_assert(json_object_is_type(sessions_obj, json_type_array), "sessions is not array");
			uds_assert(json_object_is_type(security_access_levels_obj, json_type_array), "security_access_levels is not array");

			uds_context->uds_service_31.routines[i].control_type[k].type = json_object_get_int(type_obj);
			uds_context->uds_service_31.routines[i].control_type[k].len = json_object_get_int(len_obj);
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

	json_object_put(obj);
}

void uds_service_31_init(struct uds_context *uds_context)
{
	logd("uds_service_31_init\n");

	uds_service_31_routine_parse(uds_context, UDS_CONFIG_FILE);
}
