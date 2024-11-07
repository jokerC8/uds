#include "uds.h"
#include "uds_stream.h"

typedef struct identifier_write_handler {
	int did;
	int (*write)(uds_context_t *uds_context);
} identifier_write_handler_t;


static int identifier_0xf187_write_handler(uds_context_t *uds_context)
{
	return FALSE;
}

static int identifier_0xf18a_write_handler(uds_context_t *uds_context)
{
	return FALSE;
}

static int identifier_0xf197_write_handler(uds_context_t *uds_context)
{
	return FALSE;
}

static int identifier_0xf193_write_handler(uds_context_t *uds_context)
{
	return FALSE;
}

static int identifier_0xf195_write_handler(uds_context_t *uds_context)
{
	return FALSE;
}

static int identifier_0xf18c_write_handler(uds_context_t *uds_context)
{
	return FALSE;
}

static int identifier_0xf190_write_handler(uds_context_t *uds_context)
{
	return FALSE;
}

static struct identifier_write_handler __gs_write_handler_table__[] = {
	{0xf187, identifier_0xf187_write_handler},
	{0xf18a, identifier_0xf18a_write_handler},
	{0xf197, identifier_0xf197_write_handler},
	{0xf193, identifier_0xf193_write_handler},
	{0xf195, identifier_0xf195_write_handler},
	{0xf18c, identifier_0xf18c_write_handler},
	{0xf190, identifier_0xf190_write_handler},
};

int uds_service_2e_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint16_t did;
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_service_22_identifier_t *identifier = NULL;
	identifier_write_handler_t *identifier_handler = NULL;
	uds_service_22_t *uds_service_22 = &uds_context->uds_service_22;

	if (len < 3) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	uds_stream_forward(&strm, 1);
	did = uds_stream_read_be16(&strm);
	for (int i = 0; i < uds_service_22->count; i++) {
		if (uds_service_22->identifiers[i].did == did) {
			identifier = &uds_service_22->identifiers[i];
		}
	}

	/* did不支持 */
	if (!identifier) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 长度或者格式不正确 */
	if ((int)identifier->len != (len - 3)) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	/* 子功能在当前会话模式下不支持 */
	for (size_t i = 0; i < ARRAYSIZE(identifier->sessions); i++) {
		if (identifier->sessions[i].session == uds_diagnostic_session(uds_context)) {
			if (strcmp((char *)identifier->sessions[i].attribute, "wr") != 0 &&
				strcmp((char *)identifier->sessions[i].attribute, "rw") != 0) {
				nrc = NRC_ConditionsNotCorrect_22;
				goto finish;
			}
			break;
		}
	}

	/* 当前安全级别下不支持 */
	for (size_t i = 0; i < ARRAYSIZE(identifier->security_access_levels); i++) {
		if (identifier->security_access_levels[i].level == uds_security_access_level(uds_context)) {
			if (strcmp((char *)identifier->security_access_levels[i].attribute, "wr") != 0 &&
				strcmp((char *)identifier->security_access_levels[i].attribute, "rw") != 0) {
				nrc = NRC_ConditionsNotCorrect_22;
				goto finish;
			}
			break;
		}
	}

	for (size_t i = 0; i < ARRAYSIZE(__gs_write_handler_table__); i++) {
		if (__gs_write_handler_table__[i].did == identifier->did) {
			identifier_handler = &__gs_write_handler_table__[i];
			break;
		}
	}

	/* did写入未实现或者写入失败 */
	if (!(identifier_handler && identifier_handler->write) || !identifier_handler->write(uds_context)) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 写入成功, 在did的处理函数中填充 */

finish:
	uds_context->nrc = nrc;
	return nrc;
}

void uds_service_2e_init(struct uds_context *uds_context)
{
	logd("uds_service_2e_init\n");
}
