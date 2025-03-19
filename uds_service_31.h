#ifndef __UDS_SERVICE_31_H_INCLUDED__
#define __UDS_SERVICE_31_H_INCLUDED__

struct uds_context;

typedef struct uds_service_31_routine {
	unsigned short rid;
	struct {
		char desc[32]; /* RID描述 */
		unsigned int len; /* 31服务参数长度, 不汗SID(1) + type(1) + RID(2) */
		unsigned char type; /* 01 - 执行, 03 - 读取结果 */
		unsigned char sessions[3]; /* 在该会话模式下可执行routine */
		unsigned char security_access_levels[3]; /* 在该安全级别下可执行routine */
		unsigned char session_nrc; /* 会话模式不通过返回该nrc */
		unsigned char security_access_nrc; /* 安全级别不通过返回该nrc */
	} control_type[2];
} uds_service_31_routine_t;

typedef struct uds_service_31 {
	int count;
	uds_service_31_routine_t *routines;
} uds_service_31_t;

void uds_service_31_init(struct uds_context *uds_context);

int uds_service_31_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
