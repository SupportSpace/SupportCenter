#include "attach.h"
#include "common.h"

NTSTATUS AttachDevice(
	IN PDRIVER_OBJECT  DriverObject
)
{
    UNICODE_STRING       MouseDeviceName;
    UNICODE_STRING       KbrdDeviceName;
    NTSTATUS             Status;

	DbgPrint("MegaDriver: Attach start\n");

	RtlInitUnicodeString(&MouseDeviceName, MOUSE_DEVICE);

    Status = IoCreateDevice(
		DriverObject,
		0,
		NULL,
		FILE_DEVICE_MOUSE,
		0,
		FALSE,
		&HookMouseDeviceObject);

    if(!NT_SUCCESS(Status))
	{
		DbgPrint("MegaDriver: failed to create mouse device: %d\n", Status);
		RtlFreeUnicodeString(&MouseDeviceName);
		return Status;
    }
   
    HookMouseDeviceObject->Flags |= DO_BUFFERED_IO;

	DbgPrint("MegaDriver: mouse device created\n");

	Status = IoAttachDevice(HookMouseDeviceObject, &MouseDeviceName, &LowerMouseDeviceObject);

    if(!NT_SUCCESS(Status))
	{
		DbgPrint("MegaDriver: failed to attach device: %d\n", Status);
		IoDeleteDevice(HookMouseDeviceObject);
		RtlFreeUnicodeString(&MouseDeviceName);
		return Status;
    }

//	RtlFreeUnicodeString(&DeviceName);

	DbgPrint("MegaDriver: Device is attached to mouse device\n");


	
	RtlInitUnicodeString(&KbrdDeviceName, KEYBOARD_DEVICE);

    Status = IoCreateDevice(
		DriverObject,
		0,
		NULL,
		FILE_DEVICE_KEYBOARD,
		0,
		FALSE,
		&HookKbrdDeviceObject);

    if(!NT_SUCCESS(Status))
	{
		DbgPrint("MegaDriver: failed to create keyboard device: %d\n", Status);
		RtlFreeUnicodeString(&KbrdDeviceName);
		IoDeleteDevice(HookMouseDeviceObject);
		RtlFreeUnicodeString(&MouseDeviceName);
		return Status;
    }
   
    HookKbrdDeviceObject->Flags |= DO_BUFFERED_IO;

	DbgPrint("MegaDriver: keyboard device created\n");

	Status = IoAttachDevice(HookKbrdDeviceObject, &KbrdDeviceName, &LowerKbrdDeviceObject);

    if(!NT_SUCCESS(Status))
	{
		DbgPrint("MegaDriver: failed to attach device: %d\n", Status);
		IoDeleteDevice(HookMouseDeviceObject);
		RtlFreeUnicodeString(&MouseDeviceName);
		IoDeleteDevice(HookKbrdDeviceObject);
		RtlFreeUnicodeString(&KbrdDeviceName);
		return Status;
    }

//	RtlFreeUnicodeString(&DeviceName);

	DbgPrint("MegaDriver: Device is attached to keyboard device\n");
	
	
	
	return STATUS_SUCCESS;

/*
	NTSTATUS			Status;
	OBJECT_ATTRIBUTES	Attributes;
	HANDLE             	FileHandle;   
	IO_STATUS_BLOCK    	IOStatus;
	PDEVICE_OBJECT		DeviceObject;
	PDEVICE_OBJECT		MouseDeviceObject;
	PFILE_OBJECT		FileObject;
	UNICODE_STRING		DeviceName;

	DbgPrint("MegaDriver: Attach start\n");

	RtlInitUnicodeString(&DeviceName, MOUSE_DEVICE);
	
	InitializeObjectAttributes(&Attributes, &DeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL);
	DbgPrint("MegaDriver: InitializeObjectAttributes\n");

	Status = ZwCreateFile(
		&FileHandle, 
		SYNCHRONIZE | FILE_ANY_ACCESS, 
		&Attributes, 
		&IOStatus, 
		NULL, 
		0, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN, 
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL, 
		0);

	DbgPrint("MegaDriver: ZwCreateFile\n");

//	RtlFreeUnicodeString(&DeviceName);

	DbgPrint("MegaDriver: RtlFreeUnicodeString\n");

	if(!NT_SUCCESS(Status))
	{
		DbgPrint("MegaDriver: Cannot ZwCreateFile: %d\n", Status);
		return Status;
	}

	Status = ObReferenceObjectByHandle(
		FileHandle,
		FILE_READ_DATA, 
		NULL, 
		KernelMode, 
		&FileObject, 
		NULL);

	if(!NT_SUCCESS(Status)) 
	{
		DbgPrint("MegaDriver: Failed ObReferenceObjectByHandle\n");
		ZwClose(FileHandle);
		return Status;
	}

	MouseDeviceObject = IoGetRelatedDeviceObject(FileObject);

	if(!MouseDeviceObject)
	{
		DbgPrint("MegaDriver: Failed IoGetRelatedDeviceObject\n");
		ZwClose(FileHandle);
		return STATUS_OBJECT_TYPE_MISMATCH;
	}

	DeviceObject = IoAttachDeviceToDeviceStack(fdo, MouseDeviceObject);

	((PDEVICE_EXTENSION)fdo->DeviceExtension)->TopOfStack = DeviceObject;
	DbgPrint("MegaDriver: TopOfStack established: %d\n", ((PDEVICE_EXTENSION)fdo->DeviceExtension)->TopOfStack);

	if(!DeviceObject) 
	{
		DbgPrint("MegaDriver: Failed IoAttachDeviceByPointer\n");
		ZwClose(FileHandle);
		return Status;
	}

	return STATUS_SUCCESS;
	*/
}

