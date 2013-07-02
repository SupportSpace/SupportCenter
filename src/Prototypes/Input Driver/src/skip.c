#include "skip.h"
#include "common.h"

NTSTATUS OnUnhandledRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{
	PIO_STACK_LOCATION	currentIrpStack	= IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION	nextIrpStack	= IoGetNextIrpStackLocation(Irp);
	KIRQL				irql			= KeGetCurrentIrql();

	DbgPrint("IRP request received: %d(%d), IRQL = %d\n", currentIrpStack->MajorFunction, currentIrpStack->MinorFunction, irql);

	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	if(DeviceObject == HookMouseDeviceObject)
	{
//		DbgPrint("This is mouse hook device. Calling lower device.\n");
		*nextIrpStack = *currentIrpStack;
		return IoCallDriver(LowerMouseDeviceObject, Irp);
	} 
	if(DeviceObject == HookKbrdDeviceObject)
	{
//		DbgPrint("This is keyboard hook device. Calling lower device.\n");
		*nextIrpStack = *currentIrpStack;
		return IoCallDriver(LowerKbrdDeviceObject, Irp);
	} 

//	DbgPrint("Unknown device. IoCompleteRequest calling....\n");
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

