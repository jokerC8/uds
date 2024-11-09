#include "uds.h"

static const char *uds_service_14_desc(uint32_t group)
{
	switch (group) {
		case GROUP_OF_DTC_EMISSION:
			return "clear DTC GROUP_OF_DTC_EMISSION";
		case GROUP_OF_DTC_POWER:
			return "clear DTC GROUP_OF_DTC_POWER";
		case GROUP_OF_DTC_CHASSIS:
			return "clear DTC GROUP_OF_DTC_CHASSIS";
		case GROUP_OF_DTC_BODY:
			return "clear DTC GROUP_OF_DTC_BODY";
		case GROUP_OF_DTC_NETWORK:
			return "clear DTC GROUP_OF_DTC_NETWORK";
		case GROUP_OF_DTC_ALL:
			return "clear DTC GROUP_OF_DTC_ALL";
		default:
			return "unkonw dtc group";
	}
}

/* 删除与DTC相关记录 */
static int del_dtc_record(uds_dtc_t *dtc)
{
	/* TODO */
	return 0;
}

static int uds_service_14_clear_dtc(uds_context_t *uds_context, uint32_t group)
{
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		if (group == GROUP_OF_DTC_ALL) {
			uds_dtc_monitor->dtcs[i].statusOfDTC = 0;
			del_dtc_record(&uds_dtc_monitor->dtcs[i]);
		}
		else if ((uds_dtc_monitor->dtcs[i].DTC_Num & 0x3fffff) == (group & 0x3fffff)) {
			uds_dtc_monitor->dtcs[i].statusOfDTC = 0;
			del_dtc_record(&uds_dtc_monitor->dtcs[i]);
		}
	}

	return NRC_PositiveRespon_00;
}

int uds_service_14_handler(struct uds_context *uds_context, unsigned char *data, int len)
{
	uint32_t groupOfDTC = 0;
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 4) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	if (uds_diagnostic_session(uds_context) == UDS_PROGRAMMING_SESSION) {
		nrc = NRC_ServiceNotSupportedInActiveSession_7f;
		goto finish;
	}

	groupOfDTC = (uint32_t)(data[1] << 16 | data[2] << 8 | data[3]);

	switch (groupOfDTC) {
		case GROUP_OF_DTC_EMISSION:
			nrc = uds_service_14_clear_dtc(uds_context, GROUP_OF_DTC_EMISSION);
			break;
		case GROUP_OF_DTC_POWER:
			nrc = uds_service_14_clear_dtc(uds_context, GROUP_OF_DTC_POWER);
			break;
		case GROUP_OF_DTC_CHASSIS:
			nrc = uds_service_14_clear_dtc(uds_context, GROUP_OF_DTC_CHASSIS);
			break;
		case GROUP_OF_DTC_BODY:
			nrc = uds_service_14_clear_dtc(uds_context, GROUP_OF_DTC_BODY);
			break;
		case GROUP_OF_DTC_NETWORK:
			nrc = uds_service_14_clear_dtc(uds_context, GROUP_OF_DTC_NETWORK);
			break;
		case GROUP_OF_DTC_ALL:
			nrc = uds_service_14_clear_dtc(uds_context, GROUP_OF_DTC_ALL);
			break;
		default:
			nrc = NRC_RequestOutOfRange_31;
			break;
	}

finish:
	logd("%s\n", uds_service_14_desc(groupOfDTC));
	uds_response->len = 0;
	uds_context->nrc = nrc;
	return nrc;
}

void uds_service_14_init(struct uds_context *uds_context)
{
	logd("uds_service_14_init\n");
}
