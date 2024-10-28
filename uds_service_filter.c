#include "uds.h"

/* 根据客户提供诊断调查表修改 */
int uds_service_session_verify(struct uds_context *uds_context)
{
	UDS_Session_E session = uds_diagnostic_session(uds_context);

	if (uds_context->sid == 0x14 || uds_context->sid == 0x19) {
		return (session != UDS_PROGRAMMING_SESSION);
	}

	if (uds_context->sid == 0x27) {
		return (session != UDS_DEFAULT_SESSION);
	}

	if (uds_context->sid == 0x28) {
		return (session == UDS_EXTEND_SESSION);
	}

	if (uds_context->sid == 0x2e) {
		return (session != UDS_DEFAULT_SESSION);
	}

	if (uds_context->sid == 0x2f) {
		return (session == UDS_EXTEND_SESSION);
	}

	if (uds_context->sid == 0x31) {
		return (session != UDS_DEFAULT_SESSION);
	}

	if (uds_context->sid == 0x34 || \
		uds_context->sid == 0x36 || \
		uds_context->sid == 0x37 || \
		uds_context->sid == 0x38) {
		return (session == UDS_PROGRAMMING_SESSION);
	}

	if (uds_context->sid == 0x85) {
		return (session == UDS_EXTEND_SESSION);
	}

	return TRUE;
}

/* 根据ecu实际uds实现修改 */
int uds_service_verify(uds_context_t *uds_context)
{
	if (uds_context->sid == 0x10 || \
		uds_context->sid == 0x11 || \
		uds_context->sid == 0x14 || \
		uds_context->sid == 0x19 || \
		uds_context->sid == 0x22 || \
		uds_context->sid == 0x27 || \
		uds_context->sid == 0x28 || \
		uds_context->sid == 0x2e || \
		uds_context->sid == 0x2f || \
		uds_context->sid == 0x31 || \
		uds_context->sid == 0x34 || \
		uds_context->sid == 0x36 || \
		uds_context->sid == 0x37 || \
		uds_context->sid == 0x38 || \
		uds_context->sid == 0x3e || \
		uds_context->sid == 0x85) {
			return TRUE;
		}

	return FALSE;
}
