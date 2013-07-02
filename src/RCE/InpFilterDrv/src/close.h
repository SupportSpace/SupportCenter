#include <ntddk.h>

NTSTATUS OnCloseRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
);

