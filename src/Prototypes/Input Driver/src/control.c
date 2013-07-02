#include "control.h"
#include "common.h"
#include "user.h"
#include "clear.h"

NTSTATUS OnControlRequest(
	IN PDEVICE_OBJECT	DeviceObject,
	IN PIRP			Irp
)
{
	NTSTATUS			Status			= STATUS_SUCCESS;
	PIO_STACK_LOCATION	currentIrpStack	= IoGetCurrentIrpStackLocation(Irp);
	PIO_STACK_LOCATION	nextIrpStack	= IoGetNextIrpStackLocation(Irp);
	ULONG				ControlCode		= currentIrpStack->Parameters.DeviceIoControl.IoControlCode;
	ULONG				Method			= ControlCode & 0x03;
	ULONG				OutputLength	= currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	KIRQL				OldIrql;
	PKBRD_QUEUE_ENTRY	KbrdEntry;
	PMOUSE_QUEUE_ENTRY	MouseEntry;
	PLIST_ENTRY			ListEntry;
	UCHAR*				Buffer;
	NTSTATUS			WaitStatus;

//	DbgPrint("OnControlRequest: %d\n", ControlCode);

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

	Irp->IoStatus.Information = 0;

	switch(ControlCode)
	{
	case CTRL_SSINPUT_ENABLE: 
		{
			if(FALSE == InputDisabled)
			{
				ClearQueues();
				MouseBufferCount = 0;
				MouseRelX = 0;
				MouseRelY = 0;
				MouseFirstPacket = TRUE;

				InputDisabled = TRUE;
			}
			else
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				DbgPrint("Invalid control code for current state\n");
			}
			break;
		}
	case CTRL_SSINPUT_DISABLE: 
		{
			if(TRUE == InputDisabled)
			{
				InputDisabled = FALSE;
				KeSetEvent(&MouseEvent, IO_NO_INCREMENT, FALSE);
				KeSetEvent(&KbrdEvent, IO_NO_INCREMENT, FALSE);
				ClearQueues();

				MouseBufferCount = 0;
				MouseRelX = 0;
				MouseRelY = 0;
				MouseFirstPacket = TRUE;
			}
			else
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				DbgPrint("Invalid control code for current state\n");
			}
			break;
		}
	case CTRL_SSINPUT_GET_MOUSE_DATA: 
		{
			if(TRUE == InputDisabled)
			{
//				DbgPrint("CTRL_SSINPUT_GET_MOUSE_DATA\n");
				if(OutputLength < sizeof(MOUSE_INPUT_DATA))
				{
					DbgPrint("Invalid buffer size\n");
					Status = STATUS_INVALID_PARAMETER;
					break;
				}
				if(METHOD_BUFFERED != Method)
				{
					DbgPrint("Invalid method\n");
					Status = STATUS_INVALID_DEVICE_REQUEST;
					break;
				}
				//TODO: implement

				KeAcquireSpinLock(&MouseQueueLock, &OldIrql);
//				ListEntry = PopEntryList(&MouseQueue);
				if(FALSE == IsListEmpty(&MouseQueue))
				{
					ListEntry = RemoveHeadList(&MouseQueue);
					MouseEntry = CONTAINING_RECORD(ListEntry, MOUSE_QUEUE_ENTRY, Entry);
					Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
//					DbgPrint("Bytes in buffer %d\n", currentIrpStack->Parameters.Read.Length);
					RtlCopyMemory(Buffer, &MouseEntry->MouseData, sizeof(MOUSE_INPUT_DATA));
					ExFreePool(MouseEntry);
					MouseQueueCount--;
					Irp->IoStatus.Information = sizeof(MOUSE_INPUT_DATA);
					KeReleaseSpinLock(&MouseQueueLock,OldIrql);
				}
				else
				{
//					DbgPrint("There is no data in mouse queue. Waiting...\n");
					KeReleaseSpinLock(&MouseQueueLock,OldIrql);

					//TODO: implement
/*
					WaitStatus = KeWaitForSingleObject(&MouseEvent, Executive, KernelMode, FALSE, &WaitTimeout);
					if(STATUS_SUCCESS == WaitStatus)
					{
						DbgPrint("Mouse event raised.\n");
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
							DbgPrint("There is no data in mouse queue again. Exit...\n");
							Status = STATUS_TIMEOUT;
						}
						KeReleaseSpinLock(&MouseQueueLock,OldIrql);
					}
					else
					{
						DbgPrint("TIMEOUT or some error.\n");
						Status = WaitStatus;
					}
*/
				}
//				KeReleaseSpinLock(&MouseQueueLock,OldIrql);
			}
			else
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				DbgPrint("Invalid control code for current state\n");
			}
			break;
		}
	case CTRL_SSINPUT_GET_KBRD_DATA: 
		{
			if(TRUE == InputDisabled)
			{
//				DbgPrint("CTRL_SSINPUT_GET_KBRD_DATA\n");
				if(OutputLength < sizeof(KEYBOARD_INPUT_DATA))
				{
					DbgPrint("Invalid buffer size\n");
					Status = STATUS_INVALID_PARAMETER;
					break;
				}
				if(METHOD_BUFFERED != Method)
				{
					DbgPrint("Invalid method\n");
					Status = STATUS_INVALID_DEVICE_REQUEST;
					break;
				}

				//TODO: implement
				KeAcquireSpinLock(&KbrdQueueLock, &OldIrql);
//				ListEntry = PopEntryList(&KbrdQueue);
				if(FALSE == IsListEmpty(&KbrdQueue))
				{
					ListEntry = RemoveHeadList(&KbrdQueue);
					KbrdEntry = CONTAINING_RECORD(ListEntry, KBRD_QUEUE_ENTRY, Entry);
					Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
					RtlCopyMemory(Buffer, &KbrdEntry->KbrdData, sizeof(KEYBOARD_INPUT_DATA));
					ExFreePool(KbrdEntry);
					KbrdQueueCount--;
					Irp->IoStatus.Information = sizeof(KEYBOARD_INPUT_DATA);
					KeReleaseSpinLock(&KbrdQueueLock,OldIrql);
				}
				else
				{
//					DbgPrint("There is no data in keyboard queue. Waiting...\n");
					KeReleaseSpinLock(&KbrdQueueLock,OldIrql);
					/*
					Irp->IoStatus.Information = sizeof(KEYBOARD_INPUT_DATA);
					IoMarkIrpPending(Irp);
					IoStartPacket(DeviceObject, Irp, NULL, NULL);
					DbgPrint("IoStartPacket\n");
					return STATUS_PENDING;
					*/
					//TODO: implement
/*
					WaitStatus = KeWaitForSingleObject(&KbrdEvent, Executive, KernelMode, FALSE, &WaitTimeout);
					if(STATUS_SUCCESS == WaitStatus)
					{
						DbgPrint("Keyboard event raised.\n");
						KeAcquireSpinLock(&KbrdQueueLock, &OldIrql);
						ListEntry = PopEntryList(&KbrdQueue);
						if(ListEntry)
						{
							KbrdEntry = CONTAINING_RECORD(ListEntry, KBRD_QUEUE_ENTRY, Entry);
							Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
							RtlCopyMemory(Buffer, &KbrdEntry->KbrdData, sizeof(KEYBOARD_INPUT_DATA));
							ExFreePool(KbrdEntry);
							Irp->IoStatus.Information = sizeof(KEYBOARD_INPUT_DATA);
						}
						else
						{
							DbgPrint("There is no data in keyboard queue again. Exit...\n");
							Status = STATUS_TIMEOUT;
						}
						KeReleaseSpinLock(&KbrdQueueLock,OldIrql);
					}
					else
					{
						DbgPrint("TIMEOUT or some error.\n");
						Status = WaitStatus;
					}
*/
				}
//				KeReleaseSpinLock(&KbrdQueueLock,OldIrql);
			}
			else
			{
				Status = STATUS_INVALID_DEVICE_REQUEST;
				DbgPrint("Invalid control code for current state\n");
			}
			break;
		}
	default:
		{
			Status = STATUS_INVALID_DEVICE_REQUEST;
			DbgPrint("Invalid control code\n");
		}
	}

	Irp->IoStatus.Status = Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

