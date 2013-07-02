#include "close.h"
#include "common.h"
#include "clear.h"

NTSTATUS OnCloseRequest(
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

	if(FALSE == DriverOpened)
	{
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		DbgPrint("Driver already closed.\n");
	}
	else
	{
		DriverOpened = FALSE;
		InputDisabled = FALSE;
		KeSetEvent(&MouseEvent, IO_NO_INCREMENT, FALSE);
		KeSetEvent(&KbrdEvent, IO_NO_INCREMENT, FALSE);
		ClearQueues();
		DbgPrint("Driver is closed.\n");
	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
