#ifndef __UDS_SERVICE_28_H_INCLUDED__
#define __UDS_SERVICE_28_H_INCLUDED__

#define CommunicationControl_EnableRxAndTx (0x00)
#define CommunicationControl_EnableRxAndDisableTx (0x01)
#define CommunicationControl_DisableRxAndEnableTx (0x02)
#define CommunicationControl_DisableRxAndTx (0x03)

struct uds_context;

typedef struct uds_service_28 {

} uds_service_28_t;

void uds_service_28_init(struct uds_context *uds_context);

void uds_service_28_communicate_control_reset(struct uds_context *uds_context);

int uds_service_28_handler(struct uds_context *uds_context, unsigned char *uds, int len);

#endif
