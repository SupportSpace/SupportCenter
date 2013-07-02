#include <ntddk.h>

#include "kbdmou.h"
#include <ntddmou.h>
#include <ntdd8042.h>

NTSTATUS
OnInternIoCtl(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
);

NTSTATUS
OnPendingComplete(
    __in PDEVICE_OBJECT   DeviceObject,
    __in PIRP             Irp,
    __in_opt PVOID        Context
);
