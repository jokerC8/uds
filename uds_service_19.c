#include "uds.h"
#include "uds_stream.h"

static int report_dtc_number_by_status_mask(uds_context_t *uds_context, uint8_t *uds, int len)
{
	int dtc_num = 0;
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_stream_t strm = {0};
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 3) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uint8_t status_mask = uds[2];
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_byte(&strm, uds[1]);
	uds_stream_write_byte(&strm, uds_dtc_monitor->DTCStatusAvailabilityMask);
	uds_stream_write_byte(&strm, 0); /* DTCFormatIdentify */
	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		if (uds_dtc_monitor->dtcs[i].statusOfDTC & status_mask) {
			++dtc_num;
		}
	}
	uds_stream_write_be16(&strm, dtc_num);
	uds_response->len = uds_stream_len(&strm);

finish:
	return nrc;
}

static int report_dtc_by_status_mask(uds_context_t *uds_context, uint8_t *uds, int len)
{
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_stream_t strm = {0};
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 3) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uint8_t status_mask = uds[2];
	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_byte(&strm, uds[1]);
	uds_stream_write_byte(&strm, uds_dtc_monitor->DTCStatusAvailabilityMask);

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		if (uds_dtc_monitor->dtcs[i].statusOfDTC & status_mask) {
			uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].dtc.DTCHighByte);
			uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].dtc.DTCMiddleByte);
			uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].dtc.DTCLowByte);
			uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].statusOfDTC);
		}
	}

	uds_response->len = uds_stream_len(&strm);

finish:
	return nrc;
}

static int report_dtc_snapshot_record_by_dtc_number(uds_context_t *uds_context, uint8_t *uds, int len)
{
	uint32_t DTC_Num;
	uds_stream_t strm = {0};
	uint8_t snapshot_record_num;
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_dtc_t *dtc = NULL;
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 6) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	DTC_Num = (uds[2] << 16 | uds[3] << 8 | uds[4]);
	snapshot_record_num = uds[5];

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		if (uds_dtc_monitor->dtcs[i].DTC_Num == DTC_Num) {
			dtc = &uds_dtc_monitor->dtcs[i];
			break;
		}
	}

	/* DTC不支持 */
	if (!dtc) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 快照ID不支持 */
	if (!(snapshot_record_num == GLOBAL_RECORD_NUMBER || snapshot_record_num == LOCAL_RECORD_NUMBER)) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_byte(&strm, uds[1]);
	uds_stream_write_byte(&strm, dtc->dtc.DTCHighByte);
	uds_stream_write_byte(&strm, dtc->dtc.DTCMiddleByte);
	uds_stream_write_byte(&strm, dtc->dtc.DTCLowByte);
	uds_stream_write_byte(&strm, dtc->statusOfDTC);
	if (snapshot_record_num == GLOBAL_RECORD_NUMBER) {
		uds_stream_write_byte(&strm, snapshot_record_num);
		uds_stream_write_byte(&strm, 4);
		uds_stream_write_be16(&strm, dtc->global_snapshot.power_supply_vol_did);
		uds_stream_write_be16(&strm, dtc->global_snapshot.power_supply_vol);
		uds_stream_write_be16(&strm, dtc->global_snapshot.odometer_id);
		uds_stream_write_be32(&strm, dtc->global_snapshot.odometer);
		uds_stream_write_be16(&strm, dtc->global_snapshot.vehile_speed_did);
		uds_stream_write_be16(&strm, dtc->global_snapshot.vehile_speed);
		uds_stream_write_be16(&strm, dtc->global_snapshot.timestamp_id);
		uds_stream_write_data(&strm, dtc->global_snapshot.timestamp, ARRAYSIZE(dtc->global_snapshot.timestamp));
	}
	else {
		uds_stream_write_byte(&strm, snapshot_record_num);
		uds_stream_write_byte(&strm, 4);
		uds_stream_write_be16(&strm, dtc->local_snapshot.power_supply_vol_did);
		uds_stream_write_be16(&strm, dtc->local_snapshot.power_supply_vol);
		uds_stream_write_be16(&strm, dtc->local_snapshot.odometer_id);
		uds_stream_write_be32(&strm, dtc->local_snapshot.odometer);
		uds_stream_write_be16(&strm, dtc->local_snapshot.vehile_speed_did);
		uds_stream_write_be16(&strm, dtc->local_snapshot.vehile_speed);
		uds_stream_write_be16(&strm, dtc->local_snapshot.timestamp_id);
		uds_stream_write_data(&strm, dtc->local_snapshot.timestamp, ARRAYSIZE(dtc->local_snapshot.timestamp));
	}

	uds_response->len = uds_stream_len(&strm);

finish:
	return nrc;
}

static int report_dtc_extened_data_by_dtc_number(uds_context_t *uds_context, uint8_t *uds, int len)
{
	uint32_t DTC_Num;
	uds_stream_t strm = {0};
	uint8_t extended_data_record_num;
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_dtc_t *dtc = NULL;
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 6) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	DTC_Num = (uds[2] << 16 | uds[3] << 8 | uds[4]);
	extended_data_record_num = uds[5];

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		if (uds_dtc_monitor->dtcs[i].DTC_Num == DTC_Num) {
			dtc = &uds_dtc_monitor->dtcs[i];
			break;
		}
	}

	/* DTC不支持 */
	if (!dtc) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	/* 快照ID不支持 */
	if (!(extended_data_record_num == EXTENDED_DATA_RECORD_NUMBER_1 || \
		extended_data_record_num == EXTENDED_DATA_RECORD_NUMBER_2)) {
		nrc = NRC_RequestOutOfRange_31;
		goto finish;
	}

	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_byte(&strm, uds[1]);
	uds_stream_write_byte(&strm, dtc->dtc.DTCHighByte);
	uds_stream_write_byte(&strm, dtc->dtc.DTCMiddleByte);
	uds_stream_write_byte(&strm, dtc->dtc.DTCLowByte);
	uds_stream_write_byte(&strm, dtc->statusOfDTC);
	if (extended_data_record_num == EXTENDED_DATA_RECORD_NUMBER_1) {
		uds_stream_write_byte(&strm, extended_data_record_num);
		uds_stream_write_byte(&strm, 1);
		uds_stream_write_byte(&strm, 0);
	}
	else {
		uds_stream_write_byte(&strm, extended_data_record_num);
		uds_stream_write_byte(&strm, 1);
		uds_stream_write_byte(&strm, 0);
	}

	uds_response->len = uds_stream_len(&strm);

finish:
	return nrc;
}

static int report_supported_dtc(uds_context_t *uds_context, uint8_t *uds, int len)
{
	uint8_t nrc = NRC_PositiveRespon_00;
	uds_stream_t strm = {0};
	uds_dtc_monitor_t *uds_dtc_monitor = &uds_context->uds_dtc_monitor;
	uds_response_t *uds_response = &uds_context->uds_response;

	if (len != 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds_response->pos, uds_response->cap);
	uds_stream_write_byte(&strm, uds[1]);
	uds_stream_write_byte(&strm, uds_dtc_monitor->DTCStatusAvailabilityMask);

	for (int i = 0; i < uds_dtc_monitor->count; i++) {
		uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].dtc.DTCHighByte);
		uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].dtc.DTCMiddleByte);
		uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].dtc.DTCLowByte);
		uds_stream_write_byte(&strm, uds_dtc_monitor->dtcs[i].statusOfDTC);
	}

	uds_response->len = uds_stream_len(&strm);

finish:
	return nrc;
}

int uds_service_19_handler(struct uds_context *uds_context, unsigned char *uds, int len)
{
	uint8_t sid, sub;
	uds_stream_t strm = {0};
	uint8_t nrc = NRC_PositiveRespon_00;

	if (len < 2) {
		nrc = NRC_IncorrectMessageLengthOrInvalidFormat_13;
		goto finish;
	}

	uds_stream_init(&strm, uds, len);
	sid = uds_stream_read_byte(&strm);
	sub = uds_stream_read_byte(&strm);

	switch (sub) {
		case REPORT_NUMBER_OF_DTC_BY_STATUS_MASK:
			nrc = report_dtc_number_by_status_mask(uds_context, uds, len);
			break;
		case REPORT_DTC_BY_STATUS_MASK:
			nrc = report_dtc_by_status_mask(uds_context, uds, len);
			break;
		case REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER:
			nrc = report_dtc_snapshot_record_by_dtc_number(uds_context, uds, len);
			break;
		case REPORT_DTC_EXTENDED_DATA_BY_DTC_NUMBER:
			nrc = report_dtc_extened_data_by_dtc_number(uds_context, uds, len);
			break;
		case REPORT_SUPPORTED_DTC:
			nrc = report_supported_dtc(uds_context, uds, len);
			break;
		default:
			nrc = NRC_RequestOutOfRange_31;
			break;
	}

finish:
	uds_context->nrc = nrc;
	return nrc;
}

void uds_service_19_init(struct uds_context *uds_context)
{
	logd("uds_service_19_init\n");
}
