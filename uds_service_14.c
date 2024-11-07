#include "uds.h"

static int read_dtc_group_of_emission(uds_context_t *uds_context)
{
	return 0;
}

static int read_dtc_group_of_power(uds_context_t *uds_context)
{
	return 0;
}

static int read_dtc_group_of_chassis(uds_context_t *uds_context)
{
	return 0;
}

static int read_dtc_group_of_body(uds_context_t *uds_context)
{
	return 0;
}

static int read_dtc_group_of_network(uds_context_t *uds_context)
{
	return 0;
}

static int read_dtc_group_of_all(uds_context_t *uds_context)
{
	return 0;
}

int uds_service_14_handler(struct uds_context *uds_context, unsigned char *data, int len)
{
	uint32_t groupOfDTC = 0;
	uint8_t nrc = NRC_PositiveRespon_00;

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
			read_dtc_group_of_emission(uds_context);
			break;
		case GROUP_OF_DTC_POWER:
			read_dtc_group_of_power(uds_context);
			break;
		case GROUP_OF_DTC_CHASSIS:
			read_dtc_group_of_chassis(uds_context);
			break;
		case GROUP_OF_DTC_BODY:
			read_dtc_group_of_body(uds_context);
			break;
		case GROUP_OF_DTC_NETWORK:
			read_dtc_group_of_network(uds_context);
			break;
		case GROUP_OF_DTC_ALL:
			read_dtc_group_of_all(uds_context);
			break;
		default:
			nrc = NRC_RequestOutOfRange_31;
			break;
	}

finish:
	uds_context->nrc = nrc;
	return nrc;
}

void uds_service_14_init(struct uds_context *uds_context)
{
	logd("uds_service_14_init\n");
}
