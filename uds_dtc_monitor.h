#ifndef __UDS_DTC_MONITOR_H_INCLUDED__
#define __UDS_DTC_MONITOR_H_INCLUDED__

#define MONITOR_BASE_TIMER (100) /* DTC检测基准定时器周期 */

#define DTC_TestFailed (1 << 0)
#define DTC_TestFailedThisOperationCycle (1 << 1)
#define DTC_PendingDTC (1 << 2)
#define DTC_ConfirmedDTC (1 << 3)
#define DTC_TestNotCompletedSinceLastClear (1 << 4)
#define DTC_TestFailedSinceLastClear (1 << 5)
#define DTC_TestNotCompletedThisOperationCycle (1 << 6)
#define DTC_WarningIndicatorRequested (1 << 7)

struct uds_context;

#define EXTENDED_DATA_RECORD_NUMBER_1 (0x01)
#define EXTENDED_DATA_RECORD_NUMBER_2 (0x02)

typedef struct dtc_extended_data {
	unsigned char record_num; /* 记录ID */
	unsigned char occurrence_counter; /* DTC_TestFailed计数器 */
	unsigned char failure_forget_counter; /* 老化计数器 */
} dtc_extended_data_t;

typedef struct global_snapshot {
	unsigned char valid; /* 有效标志 */
#define GLOBAL_RECORD_NUMBER (0x20) /* 记录号,通常是这个值 */
	unsigned char record_num; /* 记录id */
	unsigned short power_supply_vol_did; /* 电池电压did */
	unsigned short power_supply_vol; /* 电池电压值 */
	unsigned short odometer_id; /* 里程did  */
	unsigned int odometer; /* 里程 */
	unsigned short vehile_speed_did; /* 车速did */
	unsigned short vehile_speed; /* 车速值 */
	unsigned short timestamp_id; /* 时间did */
	unsigned char timestamp[6]; /* 时间 */
} global_snapshot_t;

typedef struct local_snapshot {
	unsigned char valid; /* 有效标志 */
#define LOCAL_RECORD_NUMBER (0x21) /* 记录号, 通常是这个值 */
	unsigned char record_num; /* 记录id */
	unsigned short power_supply_vol_did; /* 电池电压did */
	unsigned short power_supply_vol; /* 电池电压值 */
	unsigned short odometer_id; /* 里程did  */
	unsigned int odometer; /* 里程 */
	unsigned short vehile_speed_did; /* 车速did */
	unsigned short vehile_speed; /* 车速值 */
	unsigned short timestamp_id; /* 时间did */
	unsigned char timestamp[6]; /* 时间 */
} local_snapshot_t;

typedef struct uds_dtc {
	unsigned int DTC_Num; /* DTC */
	unsigned char statusOfDTC; /* DTC状态码 */
	struct {
		unsigned char DTCHighByte;
		unsigned char DTCMiddleByte;
		unsigned char DTCLowByte;
	} dtc;
	unsigned char count; /* 以基准定时器多少次超时检测一次 */
	unsigned char counter; /* 计数器 */
	unsigned char monitor_count; /* DTC Confirmed需要连续检测到故障次数 */
	unsigned char monitor_counter; /* 已连续检测到故障次数 */
	char desc[64]; /* DTC说明 */
	int monitor_rate; /* 检测周期(ms) */
	global_snapshot_t global_snapshot; /* 全局snapshot */
	local_snapshot_t local_snapshot; /* 私有snapshot */
	dtc_extended_data_t extended_data; /* 拓展数据 */
	int (*monitor)(struct uds_context *uds_context, struct uds_dtc *uds_dtc);
} uds_dtc_t;

typedef struct uds_dtc_monitor {
#define DTC_STATUS_AVAILABILITY_MASK (0x09)
	unsigned char DTCStatusAvailabilityMask;
	int count;
	uds_dtc_t *dtcs;
	struct uds_timer *monitor_timer;
} uds_dtc_monitor_t;

void uds_dtc_monitor_init(struct uds_context *uds_context);

#endif
