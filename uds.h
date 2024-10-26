#ifndef __UDS_H_INCLUDED__
#define __UDS_H_INCLUDED__

#include <stdint.h>
#include <sys/un.h>
#include "list.h"
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

#define Acquire_Sub_Function(a) ((a) & 0x7f)
#define Supress_Positive_Response(a) (((a) >> 7) & 0xff)

#define UDS_RECEIVER_SOCKFILE                    "/tmp/doip2uds"
#define UDS_SENDER_SOCKFILE                      "/tmp/uds2doip"

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
	SEND_RESPONSE,
	IGNORE_RESPONSE,
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

typedef struct uds_request {
	int status;
	int handler;
	int spr;
	int len;
	uint32_t cap;
	uint8_t *pos;
	uint8_t buffer[4096];
	struct sockaddr_un target;
	const char *sockfile;
} uds_request_t; /* uds_request */

typedef struct uds_context {
	uint8_t quit;
	uint8_t busy;
	uint8_t start;
	uint8_t status;
	uint16_t sa;
	uint16_t ta;
	uint8_t ta_type;
	uint8_t sid;
	uint8_t nrc;
	uint8_t reserved;
#define MAX_P2SERVER (50)
	uint16_t p2server;
#define MAX_P2XSERVER (5000)
	uint16_t p2xserver;
	uds_request_t uds_request;
	uds_indication_t uds_indication;

	struct uds_service_10 uds_service_10;
	struct uds_service_11 uds_service_11;

	/* services list head */
	struct list_head head;
	/* timer looper */
	struct timer_loop *loop;
	/* timer for debug*/
	struct uds_timer *heartbeat_timer;
	/* n78 timer */
	struct uds_timer *nrc78_timer;
} uds_context_t; /* uds_context */

struct uds_context *uds_context_alloc();

void uds_service_start(struct uds_context *uds_context);

void uds_service_stop(struct uds_context *uds_context);

void uds_context_destroy(struct uds_context *uds_context);

#endif
