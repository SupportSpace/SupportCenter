#include "complete.h"
#include "common.h"
#include <ntddmou.h>
#include <ntddkbd.h>

NTSTATUS OnReadComplete(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP				Irp,
	IN PVOID			Context
)
{
	PIO_STACK_LOCATION		IOStack = IoGetCurrentIrpStackLocation(Irp);
	PKEYBOARD_INPUT_DATA	KbrdData;
	PMOUSE_INPUT_DATA		MouseData;
	KIRQL					OldIrql;
	PKBRD_QUEUE_ENTRY		KbrdEntry;
	PMOUSE_QUEUE_ENTRY		MouseEntry;
	PLIST_ENTRY				ListEntry;
	KIRQL					NewIrql = DISPATCH_LEVEL;
	BOOLEAN					QueueEntryAdded = FALSE;

//	DbgPrint("OnReadComplete\n");

	if(NT_SUCCESS(Irp->IoStatus.Status))
	{
//		DbgPrint("STATUS is OK\n");
		if(DeviceObject == HookMouseDeviceObject)
		{
	  		MouseData = Irp->AssociatedIrp.SystemBuffer;
/*
			DbgPrint("Mouse position is (%d, %d).\n", MouseData->LastX, MouseData->LastY);
			if((MouseData->Flags & MOUSE_MOVE_RELATIVE) == MOUSE_MOVE_RELATIVE)
				DbgPrint("MOUSE_MOVE_RELATIVE\n");
			if((MouseData->Flags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
				DbgPrint("MOUSE_MOVE_ABSOLUTE\n");
			if((MouseData->Flags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP)
				DbgPrint("MOUSE_VIRTUAL_DESKTOP\n");
			if((MouseData->Flags & MOUSE_ATTRIBUTES_CHANGED) == MOUSE_ATTRIBUTES_CHANGED)
				DbgPrint("MOUSE_ATTRIBUTES_CHANGED\n");
*/
			if(TRUE == InputDisabled)
			{

				// add data to queue

/*
				if((MOUSE_MOVE_RELATIVE == (MouseData->Flags & 1)) && (FALSE == MouseFirstPacket))
				{
					MouseRelX += MouseData->LastX;
					MouseRelY += MouseData->LastY;
					MouseBufferCount++;
					if((MouseBufferCount < MouseBufferSize) && (MouseButtons == MouseData->ButtonFlags))
					{
						// soft canceling mouse movement
						MouseData->Flags = MOUSE_MOVE_RELATIVE;
						MouseData->LastX = 0;
						MouseData->LastY = 0;
						MouseData->Buttons = 0;
						MouseData->RawButtons = 0;
						MouseData->ExtraInformation = 0;

						if(Irp->PendingReturned)
							IoMarkIrpPending(Irp);
						return Irp->IoStatus.Status;
					}
					else
					{
						MouseData->LastX = MouseRelX;
						MouseData->LastY = MouseRelY;
					}
				}
				MouseBufferCount = 0;
				MouseRelX = 0;
				MouseRelY = 0;
				MouseButtons = MouseData->ButtonFlags;
				MouseFirstPacket = FALSE;
*/

				QueueEntryAdded = FALSE;
				MouseEntry = (PMOUSE_QUEUE_ENTRY)ExAllocatePool(NonPagedPool, sizeof(MOUSE_QUEUE_ENTRY));
				if(MouseEntry)
				{
					RtlCopyMemory(&MouseEntry->MouseData, MouseData, sizeof(MOUSE_INPUT_DATA));
					KeAcquireSpinLock(&MouseQueueLock, &OldIrql);
					if(TRUE == InputDisabled)
					{
//						PushEntryList(&MouseQueue, &MouseEntry->Entry);
						InsertTailList(&MouseQueue, &MouseEntry->Entry);

						QueueEntryAdded = TRUE;
//						DbgPrint("List entry is added\n");
						if(MouseQueueCount >= MouseQueueSize)
						{
							//ListEntry = PopEntryList(&MouseQueue);
							if(FALSE == IsListEmpty(&MouseQueue))
							{
								ListEntry = RemoveHeadList(&MouseQueue);
								MouseEntry = CONTAINING_RECORD(ListEntry, MOUSE_QUEUE_ENTRY, Entry);
//								DbgPrint("First entry is extracted\n");
								ExFreePool(MouseEntry);
							}
							else
								DbgPrint("Cannot extract entry from list\n");
						}
						else
						{
							MouseQueueCount++;
						}
					}
					else
					{
						DbgPrint("Surprise stopping\n");
						ExFreePool(MouseEntry);
					}
					KeReleaseSpinLock(&MouseQueueLock,OldIrql);
				}
				else
					DbgPrint("Cannot allocate memory for queue entry\n");

				if(TRUE == QueueEntryAdded)
					KeSetEvent(&MouseEvent, IO_NO_INCREMENT, FALSE);


				// soft canceling mouse movement
				MouseData->Flags = MOUSE_MOVE_RELATIVE;
				MouseData->LastX = 0;
				MouseData->LastY = 0;
				MouseData->Buttons = 0;
				MouseData->RawButtons = 0;
				MouseData->ExtraInformation = 0;

			}
		} 
		if(DeviceObject == HookKbrdDeviceObject)
		{
	  		KbrdData = Irp->AssociatedIrp.SystemBuffer;
//			DbgPrint("Key pressed (%d).\n", KbrdData->MakeCode);
			if(TRUE == InputDisabled)
			{
				// cancel keyboard request
				Irp->IoStatus.Status = STATUS_CANCELLED;
				Irp->IoStatus.Information = 0;
				Irp->Cancel = TRUE;

				// add data to queue
				QueueEntryAdded = FALSE;
				KbrdEntry = (PKBRD_QUEUE_ENTRY)ExAllocatePool(NonPagedPool, sizeof(KBRD_QUEUE_ENTRY));
				if(KbrdEntry)
				{
					RtlCopyMemory(&KbrdEntry->KbrdData, KbrdData, sizeof(KEYBOARD_INPUT_DATA));
					KeAcquireSpinLock(&KbrdQueueLock, &OldIrql);
					if(TRUE == InputDisabled)
					{
						//PushEntryList(&KbrdQueue, &KbrdEntry->Entry);
						InsertTailList(&KbrdQueue, &KbrdEntry->Entry);
						QueueEntryAdded = TRUE;
//						DbgPrint("List entry is added\n");
						if(KbrdQueueCount >= KbrdQueueSize)
						{
//							ListEntry = PopEntryList(&KbrdQueue);
							if(FALSE == IsListEmpty(&KbrdQueue))
							{
								ListEntry = RemoveHeadList(&KbrdQueue);
								KbrdEntry = CONTAINING_RECORD(ListEntry, KBRD_QUEUE_ENTRY, Entry);
//								DbgPrint("First entry is extracted\n");
								ExFreePool(KbrdEntry);
							}
							else
								DbgPrint("Cannot extract entry from list\n");
						}
						else
						{
							KbrdQueueCount++;
						}
					}
					else
					{
						DbgPrint("Surprise stopping\n");
						ExFreePool(MouseEntry);
					}
					KeReleaseSpinLock(&KbrdQueueLock,OldIrql);

/*
					DbgPrint("Raising IRQL\n");
					KeRaiseIrql(NewIrql,&OldIrql);
					IoStartNextPacket(SelfDeviceObject, FALSE);
					KeLowerIrql(OldIrql);
					DbgPrint("IRQL lowered\n");
*/

				}
				else
					DbgPrint("Cannot allocate memory for queue entry\n");

				if(TRUE == QueueEntryAdded)
					KeSetEvent(&KbrdEvent, IO_NO_INCREMENT, FALSE);

			}
		} 
	}

	if(Irp->PendingReturned)
	{
//		DbgPrint("IoMarkIrpPending\n");
		IoMarkIrpPending(Irp);
	}

	return Irp->IoStatus.Status;
}

