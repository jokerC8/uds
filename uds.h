#ifndef __UDS_H_INCLUDED__
#define __UDS_H_INCLUDED__

#include <stdint.h>
#include <sys/un.h>
#include "uds_utils.h"
#include "uds_service_10.h"
#include "uds_service_11.h"
#include "uds_service_14.h"
#include "uds_service_19.h"
#include "uds_service_22.h"
#include "uds_service_27.h"
#include "uds_service_28.h"
#include "uds_service_2e.h"
#include "uds_service_2f.h"
#include "uds_service_31.h"
#include "uds_service_34.h"
#include "uds_service_36.h"
#include "uds_service_37.h"
#include "uds_service_38.h"
#include "uds_service_3e.h"
#include "uds_service_85.h"
#include "uds_dtc_monitor.h"
#include "uds_service_filter.h"

#define Acquire_Sub_Function(a) ((a) & 0x7f)
#define Supress_Positive_Response(a) (((a) >> 7) & 0xff)

/* 从doip模块读取uds请求 */
#define UDS_RECEIVER_SOCKFILE                    "/tmp/doip2uds"

/* 返回结果给doip模块 */
#define UDS_SENDER_SOCKFILE                      "/tmp/uds2doip"

#define UDS_CONFIG_FILE                          "uds.json"

/* uds services */
#define UDS_Service_Session_Control_10 (0x10)
#define UDS_Service_ECU_Reset_11 (0x11)
#define UDS_Service_Clear_DTC_14 (0x14)
#define UDS_Service_Read_DTC_19 (0x19)
#define UDS_Service_Read_Identifier_22 (0x22)
#define UDS_Service_Security_Control_27 (0x27)
#define UDS_Service_Communication_Control_28 (0x28)
#define UDS_Service_Write_Identifier_2e (0x2e)
#define UDS_Service_Input_Output_Control_2f (0x2f)
#define UDS_Service_Routine_Control_31 (0x31)
#define UDS_Service_Request_Download_34 (0x34)
#define UDS_Service_Transfer_Data_36 (0x36)
#define UDS_Service_Request_Data_Exit_37 (0x37)
#define UDS_Service_Request_File_Transfer_38 (0x38)
#define UDS_Service_Tester_Present_3e (0x3e)
#define UDS_Service_DTC_Setting_Control_85 (0x85)

/* uds NRC */
#define NRC_PositiveRespon_00 (0x00)
#define NRC_GeneralReject_10 (0x10)
#define NRC_ServiceNotSupported_11 (0x11)
#define NRC_SubFunctionNotSupported_12 (0x12)
#define NRC_IncorrectMessageLengthOrInvalidFormat_13 (0x13)
#define NRC_ResponTooLong_14 (0x14)
#define NRC_ConditionsNotCorrect_22 (0x22)
#define NRC_RequestSequenceError_24 (0x24)
#define NRC_NoResponseFromSubnetComponent_25 (0x25)
#define NRC_FailurePreventsExecutionOfRequestedAction_26 (0x26)
#define NRC_RequestOutOfRange_31 (0x31)
#define NRC_SecurityAccessDenied_33 (0x33)
#define NRC_InvalidKey_35 (0x35)
#define NRC_ExceedNumberOfAttempts_36 (0x36)
#define NRC_RequiredTimeDelayNotExpired_37 (0x37)
#define NRC_UploadDownloadNotAccepted_70 (0x70)
#define NRC_TransferDataSuspended_71 (0x71)
#define NRC_GeneralProgrammingFailure_72 (0x72)
#define NRC_WrongBlockSequenceCounter_73 (0x73)
#define NRC_RequestCorrentlyReceivedResponPending_78 (0x78)
#define NRC_SubFunctionNotSupportedInActiveSession_7e (0x7e)
#define NRC_ServiceNotSupportedInActiveSession_7f (0x7f)

enum {
	TATYPE_PHY_ADDR = 0x00,
	TATYPE_FUNC_ADDR,
};

typedef struct uds_indication {
	int status;
	int handler;
#define MAX_UDS_PDU_SIZE (0x4000)
	int cap;
	int len;
	uint8_t *buffer;
	const char *sockfile;
} uds_indication_t; /* uds_indication */

typedef struct uds_response {
	int status;
	int handler;
	int len;
	uint32_t cap;
	uint8_t *pos;
	uint8_t buffer[4096];
	struct sockaddr_un target;
	const char *sockfile;
} uds_response_t; /* uds_response */

typedef struct uds_context {
	uint8_t quit;
	uint8_t busy;
	uint8_t start;
	uint8_t status;
	uint16_t SA;
	uint16_t TA;
	uint8_t TA_type;
	uint8_t sid;
	uint8_t nrc;
	uint8_t reserved;
#define MAX_P2SERVER (50)
	uint16_t p2server;
#define MAX_P2XSERVER (5000)
	uint16_t p2xserver;
	uds_response_t uds_response;
	uds_indication_t uds_indication;

	/* uds服务 */
	uds_service_10_t uds_service_10;
	uds_service_11_t uds_service_11;
	uds_service_14_t uds_service_14;
	uds_service_19_t uds_service_19;
	uds_service_22_t uds_service_22;
	uds_service_27_t uds_service_27;
	uds_service_28_t uds_service_28;
	uds_service_2e_t uds_service_2e;
	uds_service_2f_t uds_service_2f;
	uds_service_31_t uds_service_31;
	uds_service_34_t uds_service_34;
	uds_service_36_t uds_service_36;
	uds_service_37_t uds_service_37;
	uds_service_38_t uds_service_38;
	uds_service_3e_t uds_service_3e;
	uds_service_85_t uds_service_85;
	uds_dtc_monitor_t uds_dtc_monitor;

	/* 定时器loop */
	struct timer_loop *loop;
	/* 调试 */
	struct uds_timer *heartbeat_timer;
#define MAX_NRC78_RUN_TIMERS (10)
	int nrc78_timer_count;
	/* nrc78相关定时器 */
	struct uds_timer *nrc78_timer;
} uds_context_t; /* uds_context */

struct uds_context *uds_context_alloc();

void uds_service_start(struct uds_context *uds_context);

void uds_service_stop(struct uds_context *uds_context);

void uds_context_destroy(struct uds_context *uds_context);

#endif
