#include "read.h"
#include "complete.h"
#include "common.h"
#include "apc.h"
#include "read.h"

NTSTATUS OnReadRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{
	PDEVICE_EXTENSION   devExt;
	PIO_STACK_LOCATION	currentIrpStack	= IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION	nextIrpStack	= IoGetNextIrpStackLocation(Irp);
	KIRQL				irql			= KeGetCurrentIrql();

	DbgPrint("Read request received, IRQL = %d\n", irql);

	devExt =  (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

	if(DeviceObject == devExt->HookMouseDeviceObject)
	{
		DbgPrint("This is mouse hook device. Set completion routine and Calling lower device.\n");
		*nextIrpStack = *currentIrpStack;
		IoSetCompletionRoutine(Irp, OnReadComplete, DeviceObject, TRUE, TRUE, TRUE);
		return IoCallDriver(devExt->Top_of_Mouse_Stack, Irp);
	} 
	if(DeviceObject == devExt->HookKbrdDeviceObject)
	{
		DbgPrint("This is keyboard hook device. Set completion routine and Calling lower device.\n");
		*nextIrpStack = *currentIrpStack;
		IoSetCompletionRoutine(Irp, OnReadComplete, DeviceObject, TRUE, TRUE, TRUE);
		return IoCallDriver(devExt->Top_of_KBD_Stack, Irp);
	} 

	DbgPrint("Unknown device. IoCompleteRequest calling....\n");
	Irp->IoStatus.Status      = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID
Mouse_ServiceCallback(
		IN PDEVICE_OBJECT DeviceObject,
		IN PMOUSE_INPUT_DATA InputDataStart,
		IN PMOUSE_INPUT_DATA InputDataEnd,
		IN OUT PULONG InputDataConsumed
		)
		/*++

  Routine Description:

  Called when there are mouse packets to report to the RIT.  You can do
  anything you like to the packets.  For instance:

  o Drop a packet altogether
  o Mutate the contents of a packet
  o Insert packets into the stream

  Arguments:

  DeviceObject - Context passed during the connect IOCTL

  InputDataStart - First packet to be reported

  InputDataEnd - One past the last packet to be reported.  Total number of
  packets is equal to InputDataEnd - InputDataStart

  InputDataConsumed - Set to the total number of packets consumed by the RIT
  (via the function pointer we replaced in the connect
  IOCTL)

  Return Value:

  Status is returned.

  --*/
{
	PDEVICE_EXTENSION   devExt;
	PKAPC apc;

	DbgPrint("Mouse_ServiceCallback: Enter\n");

	devExt =  (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
	
	if(TRUE == InputDisabled)
	{
	
//		if (devExt->Hooked) {
		void *p, *b;
		LARGE_INTEGER TimeOut;
		PKAPC apc;
		/*TimeOut.QuadPart = 0;
		for (i=InputDataStart; i!=InputDataEnd; i++) 
		{
			p = (void *)(unsigned int)((i->LastX << 16) | (((unsigned int) i->LastY) & 0xffff));
			b = (void *)(unsigned int)((i->ButtonFlags << 16) | 0x1);
			apc = ExAllocatePool(NonPagedPool, sizeof(struct _KAPC));
			KeInitializeApc(apc, devExt->client, 0, (PKKERNEL_ROUTINE) &KeAPCRoutine, 0, devExt->routine, UserMode, (PVOID) devExt->id);
			KeInsertQueueApc(apc, p, b, 0 ); 
#ifdef DEBUG
//			DebugPrint("--- %x %x %x %x %x ---", p, b, devExt->client, devExt->routine, devExt->id);
#endif
		}*/
	} else
	{
		(*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)(
												   devExt->UpperConnectData.ClassDeviceObject,
												   InputDataStart,
												   InputDataEnd,
												   InputDataConsumed
												  );
	}

	DbgPrint("Mouse_ServiceCallback: Leave\n");
}

