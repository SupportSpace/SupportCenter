#include "inputdriver.h"
#include "common.h"

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT  DriverObject, 
	IN PUNICODE_STRING RegistryPath
)
{
	unsigned int i;
	NTSTATUS Status;
	
	DbgPrint("DriverEntry: start\n");
/*
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i) 
	{
		DriverObject->MajorFunction[i] = OnUnhandledRequest;
	}
*/

	Status = OnAddDevice(DriverObject, NULL);

	if(NT_SUCCESS(Status))
	{
		InputDisabled = FALSE;
		DriverOpened = FALSE;
		WaitTimeout.QuadPart = -300000000;
		
		KeInitializeSpinLock(&KbrdQueueLock);
		InitializeListHead(&KbrdQueue);
		KbrdQueueSize = KBRD_QUEUE_SIZE;
		KbrdQueueCount = 0;
		KeInitializeEvent(&KbrdEvent, SynchronizationEvent, FALSE);

		
		KeInitializeSpinLock(&MouseQueueLock);
		InitializeListHead(&MouseQueue);
		MouseQueueSize = MOUSE_QUEUE_SIZE;
		MouseQueueCount = 0;
		KeInitializeEvent(&MouseEvent, SynchronizationEvent, FALSE);

		MouseBufferSize = MOUSE_BUFFER_SIZE;
		MouseBufferCount = 0;
		MouseRelX = 0;
		MouseRelY = 0;
		MouseButtons = 0;
		MouseFirstPacket = TRUE;


		DriverObject->MajorFunction[IRP_MJ_READ]		   = OnReadRequest;
		DriverObject->MajorFunction[IRP_MJ_CREATE]         = OnCreateRequest;
		DriverObject->MajorFunction[IRP_MJ_CLOSE]          = OnCloseRequest;
		DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]  =
		DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = OnUnhandledRequest;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnControlRequest;

//		DriverObject->DriverStartIo = OnStartIo;

		if(LowerKbrdDeviceObject->CurrentIrp)
		{
			DbgPrint("Cancelling previous request........\n");
			IoCancelIrp(LowerKbrdDeviceObject->CurrentIrp);
		}
	}

//	DriverObject->DriverUnload = UnloadDriver;

	return Status;
}
