#include "uds.h"
#include "uds_stream.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

static int download_firmware(uds_context_t *uds_context, uint8_t *data, int len)
{
	size_t count = 0;
	uds_service_36_t *uds_service_36 = &uds_context->uds_service_36;

	if (uds_service_36->fp == NULL) {
		uds_service_36->fp = fopen(uds_service_36->filepath, "w");
		if (!uds_service_36->fp) {
			loge("fopen(%s) failed (%s)\n", uds_service_36->filepath, strerror(errno));
			return -1;
		}
	}

	count = fwrite(data, len, 1, uds_service_36->fp);
	if (count != (size_t)len) {
		fclose(uds_service_36->fp);
		uds_service_36->fp = NULL;
		return -1;
	}

	return count;
}

int uds_service_36_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uds_stream_t strm = {0};
	uint8_t sid, blockSequenceCounter;
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;
	uds_service_36_t *uds_service_36 = &uds_context->uds_service_36;

	if (len < 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	/* 当前会话模式不支持 */
	if (uds_diagnostic_session(uds_context) != UDS_PROGRAMMING_SESSION) {
		nrc = NRC_SubFunctionNotSupportedInActiveSession_7e;
		goto finish;
	}

	/* 当前安全级别下不支持 */
	if (uds_security_access_level(uds_context) != SECURITY_ACCESS_LEVEL_2) {
		nrc = NRC_SecurityAccessDenied_33;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	blockSequenceCounter = uds_stream_read_byte(&strm);

	/* 请求序列错误 */
	if (blockSequenceCounter != uds_service_36->block_num) {
		nrc = NRC_WrongBlockSequenceCounter_73;
		goto finish;
	}
	uds_service_36->block_num++;

	download_firmware(uds_context, uds + 2, len - 2);

finish:
	uds_context->nrc = nrc;
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, blockSequenceCounter);
	}
	else {
		uds_service_36_prepare(uds_context);
	}

	uds_response->len = uds_stream_len(&strm);

	return nrc;
}

void uds_service_36_prepare(uds_context_t *uds_context)
{
	uds_assert(uds_context, "uds_context is NULL");
	uds_context->uds_service_36.block_num = 1;
	if (uds_context->uds_service_36.fp) {
		fflush(uds_context->uds_service_36.fp);
		fclose(uds_context->uds_service_36.fp);
		uds_context->uds_service_36.fp = NULL;
	}
	if (access(SOC_FIRMWARE_DIR, F_OK) != 0) {
		system("mkdir -p "SOC_FIRMWARE_DIR);
	}
}

void uds_service_36_init(struct uds_context *uds_context)
{
	logd("uds_service_36_init\n");

	uds_context->uds_service_36.block_num = 1;
	uds_context->uds_service_36.filepath = SOC_FIRMWARE_FILEPATH;
}
