#include "startio.h"
#include "common.h"
#include "..\public\user.h"

void OnStartIo(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{

	NTSTATUS			Status			= STATUS_SUCCESS;
	PIO_STACK_LOCATION	currentIrpStack	= IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION	nextIrpStack	= IoGetNextIrpStackLocation(Irp);
	ULONG				ControlCode		= currentIrpStack->Parameters.DeviceIoControl.IoControlCode;
	KIRQL				OldIrql;
	PKBRD_QUEUE_ENTRY	KbrdEntry;
	PMOUSE_QUEUE_ENTRY	MouseEntry;
	PSINGLE_LIST_ENTRY	ListEntry;
	UCHAR*				Buffer;

	DbgPrint("OnStartIo: start\n");

	Irp->IoStatus.Information = 0;

	switch(ControlCode)
	{
	case CTRL_SSINPUT_GET_MOUSE_DATA: 
		{
			KeAcquireSpinLock(&MouseQueueLock, &OldIrql);
			ListEntry = PopEntryList(&MouseQueue);
			if(ListEntry)
			{
				MouseEntry = CONTAINING_RECORD(ListEntry, MOUSE_QUEUE_ENTRY, Entry);
				Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
				RtlCopyMemory(Buffer, &MouseEntry->MouseData, sizeof(MOUSE_INPUT_DATA));
				ExFreePool(MouseEntry);
				Irp->IoStatus.Information = sizeof(MOUSE_INPUT_DATA);
			}
			else
			{
				DbgPrint("There is no data in mouse queue\n");
				//TODO: implement
			}
			KeReleaseSpinLock(&MouseQueueLock,OldIrql);
			break;
		}
	case CTRL_SSINPUT_GET_KBRD_DATA: 
		{
			KeAcquireSpinLock(&KbrdQueueLock, &OldIrql);
			ListEntry = PopEntryList(&KbrdQueue);
			if(ListEntry)
			{
				KbrdEntry = CONTAINING_RECORD(ListEntry, KBRD_QUEUE_ENTRY, Entry);
				Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
				RtlCopyMemory(Buffer, &KbrdEntry->KbrdData, sizeof(KEYBOARD_INPUT_DATA));
				ExFreePool(KbrdEntry);
				Irp->IoStatus.Information = sizeof(KEYBOARD_INPUT_DATA);
				Irp->IoStatus.Status = Status;
				IoCompleteRequest(Irp, IO_NO_INCREMENT);
			}
			else
			{
				DbgPrint("There is no data in keyboard queue\n");
				//TODO: implement
			}
			KeReleaseSpinLock(&KbrdQueueLock,OldIrql);
			break;
		}
	default:
		{
			Status = STATUS_INVALID_DEVICE_REQUEST;
			DbgPrint("Invalid control code\n");
		}
	}



	DbgPrint("OnStartIo: end\n");
}
