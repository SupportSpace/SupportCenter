#include <ntddk.h>

NTSTATUS OnUnhandledRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
);

