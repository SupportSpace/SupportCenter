/*--         

Module Name:

    ssinput.c

Abstract:

Environment:

    Kernel mode only.

Notes:


--*/
#include "inputdriver.h"
#include "..\public\user.h"
#include "common.h"


NTSTATUS DriverEntry (PDRIVER_OBJECT, PUNICODE_STRING);
UNICODE_STRING		DeviceName;
PDEVICE_OBJECT		DeviceObject;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, OnAddDevice)
#endif

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT  DriverObject,
	IN PUNICODE_STRING RegistryPath
)
{	
	ULONG i;
	PDEVICE_EXTENSION   drvExt= NULL;
	NTSTATUS			Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER (RegistryPath);

	DbgPrint("DriverEntry: Enter\n");
	
	// 
    // Fill in all the dispatch entry points with the pass through function
    // and the explicitly fill in the functions we are going to intercept
    // 
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		DriverObject->MajorFunction[i] = DispatchPassThrough;
	}

	DriverObject->MajorFunction[IRP_MJ_READ]		   = OnReadRequest;
	DriverObject->MajorFunction[IRP_MJ_CREATE]         = OnCreateRequest;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]          = OnCloseRequest;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnControlRequest;
	DriverObject->MajorFunction [IRP_MJ_PNP]		   = OnPnPRequest;
	DriverObject->MajorFunction [IRP_MJ_POWER]		   = OnPowerRequest;
	DriverObject->MajorFunction [IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                                         OnInternIoCtl;
	DriverObject->DriverUnload						   = UnloadDriver;
	DriverObject->DriverExtension->AddDevice		   = OnAddDevice;


	do
	{
	

	} while (FALSE);

	DbgPrint("DriverEntry: Leave\n");

	return Status;
}