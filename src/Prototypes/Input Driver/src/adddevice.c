#include "adddevice.h"
#include "common.h"
#include "attach.h"

NTSTATUS OnAddDevice(
	IN PDRIVER_OBJECT  DriverObject,
	IN PDEVICE_OBJECT  pdo 
)
{
	NTSTATUS			Status;
	UNICODE_STRING		DeviceName;
	UNICODE_STRING		LinkName;
	PDEVICE_EXTENSION	Extension;

	DbgPrint("AddDevice: start\n");

	RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
	RtlInitUnicodeString(&LinkName, LINK_NAME);

	Status = IoCreateDevice(
		DriverObject,
		0,
		&DeviceName,
		FILE_DEVICE_SSINPUT,
		0,
		FALSE,
		&SelfDeviceObject);

	if(!NT_SUCCESS(Status))
	{
		DbgPrint("MegaDriver: failed to create device\n");
		RtlFreeUnicodeString(&DeviceName);
		RtlFreeUnicodeString(&LinkName);
		return Status;
	}

	Status = IoCreateUnprotectedSymbolicLink(
		&LinkName,
		&DeviceName);

	if (NT_SUCCESS(Status))
		DbgPrint("MegaDriver: Symbolic link created\n");
	else
		DbgPrint("MegaDriver: Create symbolic link failed\n");

//	Extension = (PDEVICE_EXTENSION)fdo->DeviceExtension;
//	RtlZeroMemory(Extension, sizeof(DEVICE_EXTENSION));

    SelfDeviceObject->Flags |= DO_BUFFERED_IO;

	Status = AttachDevice(DriverObject);
	DbgPrint("MegaDriver: Attach device end\n");

	if(!NT_SUCCESS(Status))
	{
		DbgPrint("MegaDriver: failed to attach to mouse:\n");
		IoDeleteDevice(SelfDeviceObject);
		RtlFreeUnicodeString(&DeviceName);
		RtlFreeUnicodeString(&LinkName);
		DbgPrint("MegaDriver: RtlFreeUnicodeString\n");
		return Status;
	}

	DbgPrint("MegaDriver: ----Started!!!----\n");

	return STATUS_SUCCESS;
}

