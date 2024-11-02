#include "uds.h"
#include <json-c/json.h>

static const char *attribute_desc(int attribute)
{
	switch (attribute) {
		case 1: return "rd";
		case 2: return "wr";
		case 3: return "rw";
	}
	return "unknow";
}

static void uds_service_22_identifier_dump(uds_context_t *uds_context)
{
	struct uds_service_22_identifier *identifiers = uds_context->uds_service_22.identifiers;

	for (int i = 0; i < uds_context->uds_service_22.count; i++) {
		logd("did:0x%04x, len:%d, desc:%s\n", identifiers[i].did, identifiers[i].len, identifiers[i].desc);
		for (int j = 0; j < 3; j++) {
			logd("session:%d attr:%s\n", identifiers[i].sessions[j].session, identifiers[i].sessions[j].attribute);
		}
		for (int j = 0; j < 3; j++) {
			logd("security level:%d attr:%s\n", identifiers[i].security_access_levels[j].level, \
					identifiers[i].security_access_levels[j].attribute);
		}
		logd("-------------------------------------------------------------------\n");
	}
}

int uds_service_22_handler(struct uds_context *uds_context, unsigned char *data, int len)
{
	/* TODO */
	return 0;
}

static void uds_service_22_identifier_parse(uds_context_t *uds_context, const char *filename)
{
	struct json_object *identifiers_obj = json_object_from_file(filename);

	uds_assert(identifiers_obj, "%s not valid json", filename);
	uds_assert(json_object_is_type(identifiers_obj, json_type_array), "%s is not array", filename);

	int count = json_object_array_length(identifiers_obj);
	uds_context->uds_service_22.count = count;
	uds_context->uds_service_22.identifiers = calloc(count, sizeof(*uds_context->uds_service_22.identifiers));
	uds_assert(uds_context->uds_service_22.identifiers, "malloc failed");

	for (int i = 0; i < count; i++) {

		struct json_object *item = json_object_array_get_idx(identifiers_obj, i);
		uds_assert(json_object_is_type(item, json_type_object), "array item not valid json object");

		struct json_object *did_obj = json_object_object_get(item, "did");
		struct json_object *len_obj = json_object_object_get(item, "len");
		struct json_object *desc_obj = json_object_object_get(item, "desc");
		struct json_object *sessions_obj = json_object_object_get(item, "sessions");
		struct json_object *security_access_levels_obj = json_object_object_get(item, "security_access_levels");

		uds_assert(json_object_is_type(did_obj, json_type_string), "did is not valid string");
		uds_assert(json_object_is_type(len_obj, json_type_int), "len is not valid int");
		uds_assert(json_object_is_type(desc_obj, json_type_string), "desc is not valid string");
		uds_assert(json_object_is_type(sessions_obj, json_type_array), "sessions is not valid array");
		uds_assert(json_object_is_type(security_access_levels_obj, json_type_array), "security_access_levels is not valid array");

		uds_context->uds_service_22.identifiers[i].len = json_object_get_int(len_obj);
		uds_context->uds_service_22.identifiers[i].did = strtoul(json_object_get_string(did_obj), NULL, 0x10);
		memcpy(uds_context->uds_service_22.identifiers[i].desc, json_object_get_string(desc_obj), \
				sizeof(uds_context->uds_service_22.identifiers[i].desc) - 1);

		int sessions_count = json_object_array_length(sessions_obj);
		int security_count = json_object_array_length(security_access_levels_obj);
		uds_assert(sessions_count <= 3, "too many sessions");
		uds_assert(security_count <= 3, "too many security levels");

		for (int j = 0; j < sessions_count; j++) {

			struct json_object *session_item = json_object_array_get_idx(sessions_obj, j);
			uds_assert(json_object_is_type(session_item, json_type_object), "session item not valid json object");

			struct json_object *session_obj = json_object_object_get(session_item, "session");
			struct json_object *attr_obj = json_object_object_get(session_item, "attr");

			uds_assert(json_object_is_type(session_obj, json_type_int), "session_obj item not valid int");
			uds_assert(json_object_is_type(attr_obj, json_type_string), "attr_obj not valid string");

			uds_context->uds_service_22.identifiers[i].sessions[j].session = json_object_get_int(session_obj);
			memcpy(uds_context->uds_service_22.identifiers[i].sessions[j].attribute, json_object_get_string(attr_obj), \
					sizeof(uds_context->uds_service_22.identifiers[i].sessions[j].attribute) - 1);
		}

		for (int k = 0; k < security_count; k++) {

			struct json_object *security_item = json_object_array_get_idx(security_access_levels_obj, k);
			uds_assert(json_object_is_type(security_item, json_type_object), "security_item not valid json object");

			struct json_object *level_obj = json_object_object_get(security_item, "security_access_level");
			struct json_object *attr_obj = json_object_object_get(security_item, "attr");

			uds_assert(json_object_is_type(level_obj, json_type_int), "level_obj not valid int");
			uds_assert(json_object_is_type(attr_obj, json_type_string), "attr_obj not valid string");

			uds_context->uds_service_22.identifiers[i].security_access_levels[k].level= json_object_get_int(level_obj);
			memcpy(uds_context->uds_service_22.identifiers[i].security_access_levels[k].attribute, \
				json_object_get_string(attr_obj), sizeof(uds_context->uds_service_22.identifiers[i].security_access_levels[k].attribute) - 1);
		}
	}

	uds_service_22_identifier_dump(uds_context);
	json_object_put(identifiers_obj);
}

void uds_service_22_init(struct uds_context *uds_context)
{
	logd("uds_service_22_init\n");
	uds_service_22_identifier_parse(uds_context, UDS_IDENTIFIERS_CONF);
}

