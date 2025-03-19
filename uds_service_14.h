#ifndef __UDS_SERVICE_14_H_INCLUDED__
#define __UDS_SERVICE_14_H_INCLUDED__

struct uds_context;

typedef enum {
	GROUP_OF_DTC_EMISSION = 0x000000, /* 排放相关系统 */
	GROUP_OF_DTC_POWER = 0x100000, /* 动力组 */
	GROUP_OF_DTC_CHASSIS = 0x400000, /* 底盘组 */
	GROUP_OF_DTC_BODY = 0x800000, /* 车身组 */
	GROUP_OF_DTC_NETWORK = 0xc00000, /* 网络通讯组 */
	GROUP_OF_DTC_ALL = 0xffffff, /* 所有组(所有DTC) */
} Group_Of_DTC_E;

typedef struct uds_service_14 {

} uds_service_14_t;

void uds_service_14_init(struct uds_context *uds_context);

int uds_service_14_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
