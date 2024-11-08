#ifndef __UDS_DTC_MONITOR_H_INCLUDED__
#define __UDS_DTC_MONITOR_H_INCLUDED__

#define MONITOR_BASE_TIMER (100) /* DTC检测基准定时器周期 */

struct uds_context;

typedef struct uds_dtc {
	unsigned char statusOfDTC; /* DTC状态码 */
	unsigned char DTCStatusMask; /* DTC状态掩码 */
	unsigned int DTC_Num;
	struct {
		unsigned char DTCHighByte;
		unsigned char DTCMiddleByte;
		unsigned char DTCLowByte;
	} dtc;
	unsigned char count; /* 以基准定时器多少次超时检测一次 */
	unsigned char counter; /* 计数器 */
	unsigned char monitor_count; /* DTC Confirmed需要连续检测到故障次数 */
	unsigned char monitor_counter; /* 已连续检测到故障次数 */
	char desc[63]; /* DTC说明 */
	int monitor_rate; /* 检测周期(ms) */
	int (*fp)(struct uds_context *uds_context, struct uds_dtc *uds_dtc);
} uds_dtc_t;

typedef struct uds_dtc_monitor {
	int count;
	uds_dtc_t *dtcs;
	struct uds_timer *monitor_timer;
} uds_dtc_monitor_t;

void uds_dtc_monitor_init(struct uds_context *uds_context);

#endif
