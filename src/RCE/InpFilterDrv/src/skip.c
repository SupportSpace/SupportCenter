#include "skip.h"
#include "common.h"

NTSTATUS
DispatchPassThrough(
        __in PDEVICE_OBJECT DeviceObject,
        __in PIRP Irp
        )
/*++
Routine Description:

    Passes a request on to the lower driver.
     
Considerations:
     
    If you are creating another device object (to communicate with user mode
    via IOCTLs), then this function must act differently based on the intended 
    device object.  If the IRP is being sent to the solitary device object, then
    this function should just complete the IRP (becuase there is no more stack
    locations below it).  If the IRP is being sent to the PnP built stack, then
    the IRP should be passed down the stack. 
    
    These changes must also be propagated to all the other IRP_MJ dispatch
    functions (such as create, close, cleanup, etc.) as well!

--*/
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Pass the IRP to the target
    //
    IoSkipCurrentIrpStackLocation(Irp);
        
	if(DeviceObject == ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->HookMouseDeviceObject)
	{
		
		return IoCallDriver(((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->Top_of_Mouse_Stack, Irp);
	} 
	if(DeviceObject == ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->HookKbrdDeviceObject)
	{
		return IoCallDriver(((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->Top_of_KBD_Stack, Irp);
	} 

    
	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;

}           