#include "uds.h"
#include "uds_stream.h"
#include <json-c/json.h>

typedef struct identifier_read_handler {
	int did;
	int (*read)(uds_context_t *uds_context, uint16_t identifier);
} identifier_read_handler_t;


static int fill_uds_response(uds_context_t *uds_context, uint16_t identifier, uint8_t *data, int len)
{
	uds_stream_t strm = {0};
	uds_response_t *uds_response = &uds_context->uds_response;

	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_be16(&strm, identifier);
	uds_stream_write_data(&strm, data, len);
	uds_response->len = uds_stream_len(&strm);
	return uds_response->len;
}

static int identifier_0xf187_read_handler(uds_context_t *uds_context, uint16_t identifier)
{
	uint8_t record[15] = {"123456789012345"};
	return fill_uds_response(uds_context, identifier, record, ARRAYSIZE(record));
}

static int identifier_0xf18a_read_handler(uds_context_t *uds_context, uint16_t identifier)
{
	uint8_t record[10] = {"1234567890"};
	return fill_uds_response(uds_context, identifier, record, ARRAYSIZE(record));
}

static int identifier_0xf197_read_handler(uds_context_t *uds_context, uint16_t identifier)
{
	uint8_t record[10] = {"1234567890"};
	return fill_uds_response(uds_context, identifier, record, ARRAYSIZE(record));
}

static int identifier_0xf193_read_handler(uds_context_t *uds_context, uint16_t identifier)
{
	uint8_t record[2] = {"12"};
	return fill_uds_response(uds_context, identifier, record, ARRAYSIZE(record));
}

static int identifier_0xf195_read_handler(uds_context_t *uds_context, uint16_t identifier)
{
	uint8_t record[2] = {"34"};
	return fill_uds_response(uds_context, identifier, record, ARRAYSIZE(record));
}

static int identifier_0xf18c_read_handler(uds_context_t *uds_context, uint16_t identifier)
{
	uint8_t record[14] = {"12345678901234"};
	return fill_uds_response(uds_context, identifier, record, ARRAYSIZE(record));
}

static int identifier_0xf190_read_handler(uds_context_t *uds_context, uint16_t identifier)
{
	uint8_t record[17] = {"12345678901234567"};
	return fill_uds_response(uds_context, identifier, record, ARRAYSIZE(record));
}

static struct identifier_read_handler __gs_read_handler_table__[] = {
	{0xf187, identifier_0xf187_read_handler},
	{0xf18a, identifier_0xf18a_read_handler},
	{0xf197, identifier_0xf197_read_handler},
	{0xf193, identifier_0xf193_read_handler},
	{0xf195, identifier_0xf195_read_handler},
	{0xf18c, identifier_0xf18c_read_handler},
	{0xf190, identifier_0xf190_read_handler},
};

static void uds_service_22_identifier_dump(uds_context_t *uds_context)
{
	uds_service_22_identifier_t *identifiers = uds_context->uds_service_22.identifiers;

	for (int i = 0; i < uds_context->uds_service_22.count; i++) {
		logd("did:0x%04x, len:%d, desc:%s\n", identifiers[i].did, identifiers[i].len, identifiers[i].desc);
		for (int j = 0; j < 3; j++) {
			logd("session:%d attr:%s\n", identifiers[i].sessions[j].session, identifiers[i].sessions[j].attribute);
		}
		for (int j = 0; j < 3; j++) {
			logd("security level:%d attr:%s\n", identifiers[i].security_access_levels[j].level, \
					identifiers[i].security_access_levels[j].attribute);
		}
		logd("session_nrc:0x%02x, security_access_nrc:0x%02x\n", identifiers[i].session_nrc, identifiers[i].security_access_nrc);
		logd("-------------------------------------------------------------------\n");
	}
}

int uds_service_22_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_service_22_identifier_t *identifier = NULL;
	uds_service_22_t *uds_service_22 = &uds_context->uds_service_22;
	identifier_read_handler_t *identifier_handler = NULL;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len < 3) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	uds_stream_forward(&strm, 1);
	uint16_t did = uds_stream_read_be16(&strm);
	for (int i = 0; i < uds_service_22->count; i++) {
		if (uds_service_22->identifiers[i].did == did) {
			identifier = &uds_service_22->identifiers[i];
			break;
		}
	}

	/* did不支持 */
	if (!identifier) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 子功能在当前会话模式下不支持 */
	for (size_t i = 0; i < ARRAYSIZE(identifier->sessions); i++) {
		if (identifier->sessions[i].session == uds_diagnostic_session(uds_context)) {
			if (strcmp((char *)identifier->sessions[i].attribute, "rd") != 0 &&
				strcmp((char *)identifier->sessions[i].attribute, "rw") != 0) {
				nrc = identifier->session_nrc;
				goto finish;
			}
			break;
		}
	}

	/* 当前安全级别下不支持 */
	for (size_t i = 0; i < ARRAYSIZE(identifier->security_access_levels); i++) {
		if (identifier->security_access_levels[i].level == uds_security_access_level(uds_context)) {
			if (strcmp((char *)identifier->security_access_levels[i].attribute, "rd") != 0 &&
				strcmp((char *)identifier->security_access_levels[i].attribute, "rw") != 0) {
				nrc = identifier->security_access_nrc;
				goto finish;
			}
			break;
		}
	}

	for (size_t i = 0; i < ARRAYSIZE(__gs_read_handler_table__); i++) {
		if (__gs_read_handler_table__[i].did == identifier->did) {
			identifier_handler = &__gs_read_handler_table__[i];
			break;
		}
	}

	/* did读取未实现或者读取失败, 返回全0数据 */
	if (!(identifier_handler && identifier_handler->read) || !identifier_handler->read(uds_context, identifier->did)) {
		uds_stream_init(&strm, uds_response->pos, uds_response->cap);
		uds_stream_write_be16(&strm, identifier->did);
		uds_assert(identifier->len < uds_stream_left_len(&strm), "did len(%d) too long\n", identifier->len);
		memset(uds_stream_ptr(&strm), 0x00, identifier->len);
		uds_stream_forward(&strm, identifier->len);
		uds_response->len = uds_stream_len(&strm);
		goto finish;
	}

	/* 读取成功, 在did的处理函数中填充 */

finish:
	uds_context->nrc = nrc;
	return nrc;
}

static void uds_service_22_identifier_parse(uds_context_t *uds_context, const char *filename)
{
	struct json_object *obj = json_object_from_file(filename);
	uds_service_22_t *uds_service_22 = &uds_context->uds_service_22;

	uds_assert(obj, "%s not valid json", filename);
	struct json_object *identifiers_obj = json_object_object_get(obj, "sid_22");
	if (!identifiers_obj) {
		loge("can not find sid_22 in %s\n", filename);
		return;
	}

	uds_assert(json_object_is_type(identifiers_obj, json_type_array), "%s is not array", filename);

	int count = json_object_array_length(identifiers_obj);
	uds_service_22->count = count;
	uds_service_22->identifiers = calloc(count, sizeof(*uds_service_22->identifiers));
	uds_assert(uds_service_22->identifiers, "malloc failed");

	for (int i = 0; i < count; i++) {

		struct json_object *item = json_object_array_get_idx(identifiers_obj, i);
		uds_assert(json_object_is_type(item, json_type_object), "array item not valid json object");

		struct json_object *did_obj = json_object_object_get(item, "did");
		struct json_object *len_obj = json_object_object_get(item, "len");
		struct json_object *desc_obj = json_object_object_get(item, "desc");
		struct json_object *sessions_obj = json_object_object_get(item, "sessions");
		struct json_object *security_access_levels_obj = json_object_object_get(item, "security_access_levels");
		struct json_object *session_nrc_obj = json_object_object_get(item, "session_nrc");
		struct json_object *security_access_nrc_obj = json_object_object_get(item, "security_access_nrc");

		uds_assert(json_object_is_type(did_obj, json_type_string), "did is not valid string");
		uds_assert(json_object_is_type(len_obj, json_type_int), "len is not valid int");
		uds_assert(json_object_is_type(desc_obj, json_type_string), "desc is not valid string");
		uds_assert(json_object_is_type(sessions_obj, json_type_array), "sessions is not valid array");
		uds_assert(json_object_is_type(session_nrc_obj, json_type_string), "session_nrc is not valid string");
		uds_assert(json_object_is_type(security_access_nrc_obj, json_type_string), "security_access_nrc is not valid string");

		uds_service_22->identifiers[i].len = json_object_get_int(len_obj);
		uds_service_22->identifiers[i].did = strtoul(json_object_get_string(did_obj), NULL, 0x10);
		memcpy(uds_service_22->identifiers[i].desc, json_object_get_string(desc_obj), \
				sizeof(uds_service_22->identifiers[i].desc) - 1);

		uds_service_22->identifiers[i].session_nrc = strtoul(json_object_get_string(session_nrc_obj), NULL, 0x10);
		uds_service_22->identifiers[i].security_access_nrc = strtoul(json_object_get_string(security_access_nrc_obj), NULL, 0x10);

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

			uds_service_22->identifiers[i].sessions[j].session = json_object_get_int(session_obj);
			memcpy(uds_service_22->identifiers[i].sessions[j].attribute, json_object_get_string(attr_obj), \
					sizeof(uds_service_22->identifiers[i].sessions[j].attribute) - 1);
		}

		for (int k = 0; k < security_count; k++) {

			struct json_object *security_item = json_object_array_get_idx(security_access_levels_obj, k);
			uds_assert(json_object_is_type(security_item, json_type_object), "security_item not valid json object");

			struct json_object *level_obj = json_object_object_get(security_item, "security_access_level");
			struct json_object *attr_obj = json_object_object_get(security_item, "attr");

			uds_assert(json_object_is_type(level_obj, json_type_int), "level_obj not valid int");
			uds_assert(json_object_is_type(attr_obj, json_type_string), "attr_obj not valid string");

			uds_service_22->identifiers[i].security_access_levels[k].level= json_object_get_int(level_obj);
			memcpy(uds_service_22->identifiers[i].security_access_levels[k].attribute, \
				json_object_get_string(attr_obj), sizeof(uds_service_22->identifiers[i].security_access_levels[k].attribute) - 1);
		}
	}

	uds_service_22_identifier_dump(uds_context);
	json_object_put(obj);
}

void uds_service_22_init(struct uds_context *uds_context)
{
	logd("uds_service_22_init\n");
	uds_service_22_identifier_parse(uds_context, UDS_CONFIG_FILE);
}

