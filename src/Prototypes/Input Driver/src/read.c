#include "read.h"
#include "complete.h"
#include "common.h"

NTSTATUS OnReadRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{
	PIO_STACK_LOCATION	currentIrpStack	= IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION	nextIrpStack	= IoGetNextIrpStackLocation(Irp);
	KIRQL				irql			= KeGetCurrentIrql();

//	DbgPrint("Read request received, IRQL = %d\n", irql);

	if(DeviceObject == HookMouseDeviceObject)
	{
//		DbgPrint("This is mouse hook device. Set competeon routine and Calling lower device.\n");
		*nextIrpStack = *currentIrpStack;
		IoSetCompletionRoutine(Irp, OnReadComplete, DeviceObject, TRUE, TRUE, TRUE);
		return IoCallDriver(LowerMouseDeviceObject, Irp);
	} 
	if(DeviceObject == HookKbrdDeviceObject)
	{
//		DbgPrint("This is keyboard hook device. Set competeon routine and Calling lower device.\n");
		*nextIrpStack = *currentIrpStack;
		IoSetCompletionRoutine(Irp, OnReadComplete, DeviceObject, TRUE, TRUE, TRUE);
		return IoCallDriver(LowerKbrdDeviceObject, Irp);
	} 

//	DbgPrint("Unknown device. IoCompleteRequest calling....\n");
	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

