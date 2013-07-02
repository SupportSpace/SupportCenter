#include <ntddk.h>

NTSTATUS OnCreateRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
);

