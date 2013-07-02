#include <ntddk.h>

NTSTATUS
OnPnPRequest(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
);

NTSTATUS
OnPowerRequest(
    __in PDEVICE_OBJECT    DeviceObject,
    __in PIRP              Irp
);