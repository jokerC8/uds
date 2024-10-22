#include "uds.h"

int main(void)
{
	struct uds_context *uds_context;

	uds_context = uds_context_init(0);

	uds_service_start(uds_context);
}
