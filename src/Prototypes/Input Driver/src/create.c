#include "create.h"
#include "common.h"

NTSTATUS OnCreateRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{
	PIO_STACK_LOCATION	currentIrpStack	= IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION	nextIrpStack	= IoGetNextIrpStackLocation(Irp);

	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	if(DeviceObject == HookMouseDeviceObject)
	{
		*nextIrpStack = *currentIrpStack;
		return IoCallDriver(LowerMouseDeviceObject, Irp);
	} 
	if(DeviceObject == HookKbrdDeviceObject)
	{
		*nextIrpStack = *currentIrpStack;
		return IoCallDriver(LowerKbrdDeviceObject, Irp);
	} 

	if(TRUE == DriverOpened)
	{
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		DbgPrint("Driver already opened.\n");
	}
	else
	{
		DriverOpened = TRUE;
		DbgPrint("Driver is opened.\n");
	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

