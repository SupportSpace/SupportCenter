#include <ntddk.h>

NTSTATUS OnReadRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp
);

