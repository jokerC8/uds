#include "uds.h"
#include "uds_stream.h"

enum {
	ADDFILE = 0x01,
	DELFILE,
	REPLACEFILE,
	READFILE,
	READDIR,
} ModeOfOperaion_E;

static int add_file_handler(uds_context_t *uds_context, uint8_t *uds, int len)
{
	uint8_t buffer[8] = {0};
	uint8_t dataFormatIdentifer;
	uint8_t fileSizeParamLength;
	uint64_t fileSizeCompressed = 0;
	uint64_t fileSizeUnCompressed = 0;
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_stream_t strm = {0};
	uds_response_t *uds_response = &uds_context->uds_response;
	uds_service_38_t *uds_service_38 = &uds_context->uds_service_38;

	uds_stream_init(&strm, uds, len);
	/* sid(1) + mode(1) + filename_len(2) + filename(filename_len) */
	uds_stream_forward(&strm, uds_service_38->filename_len + 4);

	/* 长度不匹配 */
	if (!(len > (uds_service_38->filename_len + 6))) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	dataFormatIdentifer = uds_stream_read_byte(&strm);
	fileSizeParamLength = uds_stream_read_byte(&strm);

	/* 4字节能表示4G大小的文件,一般来说足够了 */
	if (!(fileSizeParamLength > 0 && fileSizeParamLength <= 4)) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 长度不匹配 */
	if (!(len == (uds_service_38->filename_len + 6 + 2 * fileSizeParamLength))) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	/* 未压缩文件大小 */
	uds_stream_read_data(&strm, buffer, fileSizeParamLength);
	fileSizeUnCompressed = byte_array2_uint64(buffer, fileSizeParamLength);

	/* 压缩文件大小 */
	uds_stream_read_data(&strm, buffer, fileSizeParamLength);
	fileSizeCompressed = byte_array2_uint64(buffer, fileSizeParamLength);

	uds_service_38->fileSizeCompressed = fileSizeCompressed;
	uds_service_38->fileSizeUnCompressed = fileSizeUnCompressed;

	/* 压缩和未压缩文件长度为0 */
	if (fileSizeCompressed == 0 || fileSizeUnCompressed == 0) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 当前会话模式不支持 */
	if (uds_diagnostic_session(uds_context) != UDS_PROGRAMMING_SESSION) {
		nrc = NRC_SubFunctionNotSupportedInActiveSession_7e;
		goto finish;
	}

	/* 当前安全访问级别不支持 */
	if (uds_security_access_level(uds_context) != SECURITY_ACCESS_LEVEL_2) {
		nrc = NRC_SecurityAccessDenied_33;
		goto finish;
	}

	/* 36服务准备 */
	uds_service_36_prepare(uds_context);

finish:
	uds_context->nrc = nrc;
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	if (nrc == NRC_PositiveRespon_00) {
		uds_stream_write_byte(&strm, ADDFILE);
		uds_stream_write_byte(&strm, 2);
		uds_stream_write_be16(&strm, Max_Number_Of_Block_Length);
		uds_stream_write_byte(&strm, 0x00);
	}

	uds_response->len = uds_stream_len(&strm);
	return nrc;
}

static int delete_file_handler(uds_context_t *uds_context, uint8_t *uds, int len)
{
	return NRC_RequestOutOfRange_31;
}

static int replace_file_handler(uds_context_t *uds_context, uint8_t *data, int len)
{
	return NRC_RequestOutOfRange_31;
}

static int read_file_handler(uds_context_t *uds_context, uint8_t *data, int len)
{
	return NRC_RequestOutOfRange_31;
}

static int read_dir_handler(uds_context_t *uds_context, uint8_t *data, int len)
{
	return NRC_RequestOutOfRange_31;
}

int uds_service_38_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint8_t sid, sub;
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_service_38_t *uds_service_38 = &uds_context->uds_service_38;

	/* 确保sid, sub, filename len可以正常解析 */
	if (len < 4) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	sub = uds_stream_read_byte(&strm);
	uds_service_38->filename_len = uds_stream_read_be16(&strm);

	/* 确保uds请求长度大于文件名长度 */
	if (len < (uds_service_38->filename_len + 4)) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	if (!(uds_service_38->filename_len > 0 && uds_service_38->filename_len <= (int)ARRAYSIZE(uds_service_38->filename))) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	bzero(uds_service_38->filename, ARRAYSIZE(uds_service_38->filename));
	uds_stream_read_data(&strm, (uint8_t *)uds_service_38->filename, uds_service_38->filename_len);

	switch (Acquire_Sub_Function(sub)) {
		case ADDFILE:
			nrc = add_file_handler(uds_context, uds, len);
			break;
		case DELFILE:
			nrc = delete_file_handler(uds_context, uds, len);
			break;
		case REPLACEFILE:
			nrc = replace_file_handler(uds_context, uds, len);
			break;
		case READFILE:
			nrc = read_file_handler(uds_context, uds, len);
			break;
		case READDIR:
			nrc = read_dir_handler(uds_context, uds, len);
			break;
		default:
			nrc = NRC_RequestOutOfRange_31;
			break;
	}

finish:
	uds_context->nrc = nrc;
	return nrc;
}

void uds_service_38_init(struct uds_context *uds_context)
{
	logd("uds_service_38_init\n");
}
