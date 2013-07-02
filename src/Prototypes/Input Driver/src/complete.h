#include <ntddk.h>

NTSTATUS OnReadComplete(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp,
	IN PVOID			Context
);
