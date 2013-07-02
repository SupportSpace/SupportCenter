#include "power.h"
#include "common.h"
#include "internalIOctl.h"
#include "skip.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, OnPnPRequest)
#pragma alloc_text (PAGE, OnPowerRequest)
#endif

NTSTATUS
OnPnPRequest(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
/*++

Routine Description:

    This routine is the dispatch routine for plug and play irps 

Arguments:

    DeviceObject - Pointer to the device object.

    Irp - Pointer to the request packet.

Return Value:

    Status is returned.

--*/
{
    PDEVICE_EXTENSION           devExt; 
    PIO_STACK_LOCATION          irpStack;
    NTSTATUS                    status = STATUS_SUCCESS;
    KIRQL                       oldIrql;
    KEVENT                      event;        

    PAGED_CODE();

	DbgPrint("OnPnPRequest: Enter\n");

	devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (irpStack->MinorFunction) {
    case IRP_MN_START_DEVICE: {

        //
        // The device is starting.
        //
        // We cannot touch the device (send it any non pnp irps) until a
        // start device has been passed down to the lower drivers.
        //
        IoCopyCurrentIrpStackLocationToNext(Irp);
        KeInitializeEvent(&event,
                          NotificationEvent,
                          FALSE
                          );

        IoSetCompletionRoutine(Irp,
                               (PIO_COMPLETION_ROUTINE) OnPendingComplete, 
                               &event,
                               TRUE,
                               TRUE,
                               TRUE); // No need for Cancel

  		if(DeviceObject == devExt->HookMouseDeviceObject)
		{
			status = IoCallDriver(devExt->Top_of_Mouse_Stack, Irp);
		} 
		if(DeviceObject == devExt->HookKbrdDeviceObject)
		{
			status = IoCallDriver(devExt->Top_of_KBD_Stack, Irp);
		} 

        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &event,
               Executive, // Waiting for reason of a driver
               KernelMode, // Waiting in kernel mode
               FALSE, // No allert
               NULL); // No timeout

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {
            //
            // As we are successfully now back from our start device
            // we can do work.
            //
            devExt->Started = TRUE;
            devExt->Removed = FALSE;
            devExt->SurpriseRemoved = FALSE;
        }

		if (!NT_SUCCESS(IoSetDeviceInterfaceState(&devExt->DeviceLinkName, TRUE))) 
		{
			DbgPrint("OnPnPRequest: failed to Enable Device Interface: \n");
			IoDeleteDevice(devExt->HookMouseDeviceObject);
	    }
		
		//
        // We must now complete the IRP, since we stopped it in the
        // completetion routine with MORE_PROCESSING_REQUIRED.
        //
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

		break;
    }

    case IRP_MN_SURPRISE_REMOVAL:
        //
        // Same as a remove device, but don't call IoDetach or IoDeleteDevice
        //
        devExt->SurpriseRemoved = TRUE;

        // Remove code here

		if(DeviceObject == devExt->HookMouseDeviceObject || DeviceObject == devExt->HookKbrdDeviceObject)
		{
			status = DispatchPassThrough(DeviceObject, Irp);
	        break;
		} 
        break;

    case IRP_MN_REMOVE_DEVICE:
        
        devExt->Removed = TRUE;

        // remove code here
        Irp->IoStatus.Status = STATUS_SUCCESS;

		status = IoSetDeviceInterfaceState(&devExt->DeviceLinkName, FALSE);
		if (!NT_SUCCESS(status)) 
		{
			DbgPrint("OnPnPRequest: failed to Enable Device Interface: %d\n", status);
	    }
		RtlFreeUnicodeString(&devExt->DeviceLinkName);


		if(DeviceObject == devExt->HookMouseDeviceObject || DeviceObject == devExt->HookKbrdDeviceObject)
		{
			status = DispatchPassThrough(DeviceObject, Irp);
		}

		IoDeleteDevice(DeviceObject);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: 
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_QUERY_DEVICE_TEXT:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    default:
        //
        // Here the filter driver might modify the behavior of these IRPS
        // Please see PlugPlay documentation for use of these IRPs.
        //
        status = DispatchPassThrough( DeviceObject ,Irp);
        break;
    }
	DbgPrint("OnPnPRequest: Leave\n");

    return status;
}

NTSTATUS
OnPowerRequest(
    __in PDEVICE_OBJECT    DeviceObject,
    __in PIRP              Irp
    )
/*++

Routine Description:

    This routine is the dispatch routine for power irps   Does nothing except
    record the state of the device.

Arguments:

    DeviceObject - Pointer to the device object.

    Irp - Pointer to the request packet.

Return Value:

    Status is returned.

--*/
{
    PIO_STACK_LOCATION  irpStack;
    PDEVICE_EXTENSION   devExt;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
	NTSTATUS            status = STATUS_SUCCESS;

    PAGED_CODE();

	DbgPrint("OnPowerRequest: Enter\n");
  
	devExt = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    powerType = irpStack->Parameters.Power.Type;
    powerState = irpStack->Parameters.Power.State;

   switch (irpStack->MinorFunction) {
    case IRP_MN_SET_POWER:
        if (powerType  == DevicePowerState) {
            devExt->DeviceState = powerState.DeviceState;
        }

    case IRP_MN_QUERY_POWER:
    case IRP_MN_WAIT_WAKE:
    case IRP_MN_POWER_SEQUENCE:
    default:
        break;
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);


 	if(DeviceObject == devExt->HookMouseDeviceObject)
	{
		status = PoCallDriver(DeviceObject, Irp);
	} 
	if(DeviceObject == devExt->HookKbrdDeviceObject)
	{
		status = PoCallDriver(DeviceObject, Irp);
	} 

	DbgPrint("OnPowerRequest: Leave\n");

	return status;
}