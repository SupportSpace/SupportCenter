#include <ntddk.h>
#include <ntddmou.h>

NTSTATUS OnReadRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp
);

VOID
Mouse_ServiceCallback(
		IN PDEVICE_OBJECT DeviceObject,
		IN PMOUSE_INPUT_DATA InputDataStart,
		IN PMOUSE_INPUT_DATA InputDataEnd,
		IN OUT PULONG InputDataConsumed
);
