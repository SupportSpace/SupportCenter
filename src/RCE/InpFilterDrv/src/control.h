#include <ntddk.h>

NTSTATUS OnControlRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
);

