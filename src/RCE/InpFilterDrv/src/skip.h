#include <ntddk.h>

NTSTATUS DispatchPassThrough(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
);

