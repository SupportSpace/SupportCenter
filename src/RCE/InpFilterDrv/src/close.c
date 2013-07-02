#include "close.h"
#include "common.h"
#include "clear.h"
#include "skip.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, OnCloseRequest)
#endif

NTSTATUS OnCloseRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{
	PIO_STACK_LOCATION	currentIrpStack;
	PIO_STACK_LOCATION	nextIrpStack;	
	PDEVICE_EXTENSION   devExt;	
	NTSTATUS            status = STATUS_SUCCESS; 
 

	PAGED_CODE();

	DbgPrint("OnCloseRequest:  Enter\n");

	currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	DbgPrint("OnCloseRequest:  Close request from %x\n",DeviceObject);

	do 
	{
		devExt =  (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

	
		if(DeviceObject == devExt->HookMouseDeviceObject)
		{
			status = DispatchPassThrough(DeviceObject, Irp);
			break;
		}
		if(DeviceObject == devExt->HookKbrdDeviceObject)
		{
			status = DispatchPassThrough(DeviceObject, Irp);
			break;
		}


		if(FALSE == DriverOpened)
		{
			Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
			DbgPrint("OnCloseRequest: Driver already closed.\n");
		}
		else
		{	
			DriverOpened = FALSE;
			InputDisabled = FALSE;

			KeSetEvent(&MouseEvent, IO_NO_INCREMENT, FALSE);
			KeSetEvent(&KbrdEvent, IO_NO_INCREMENT, FALSE);
			ClearQueues();
			DbgPrint("OnCloseRequest: Driver is closed.\n");
		}

		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	} while (FALSE);
	
	DbgPrint("OnCloseRequest:  Leave\n");
	return status;
}
