#include "uds.h"

int main(void)
{
	struct uds_context *uds_context;

	uds_context = uds_context_alloc();

	uds_service_start(uds_context);
}
