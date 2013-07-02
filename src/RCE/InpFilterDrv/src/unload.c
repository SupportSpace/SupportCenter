#include "unload.h"
#include "..\public\user.h"
#include "common.h"

void UnloadDriver(
	IN PDRIVER_OBJECT DriverObject);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, UnloadDriver)
#endif

void UnloadDriver(
	IN PDRIVER_OBJECT DriverObject
)
{
	UNICODE_STRING		LinkName;
	PDEVICE_EXTENSION	Extension;
	PDEVICE_OBJECT		DeviceObject;

	PAGED_CODE();

	DbgPrint("OnUnLoad: Enter\n");

	RtlInitUnicodeString(&LinkName, LINK_NAME);
	IoDeleteSymbolicLink(&LinkName);


	DeviceObject = DriverObject->DeviceObject;

	if(DeviceObject)
	{
		Extension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
		if(Extension)
		{
			if(Extension->Top_of_Mouse_Stack)
			{
				IoDetachDevice(Extension->Top_of_Mouse_Stack);
				DbgPrint("OnUnLoad: Mouse device detached\n");
			}
			if(Extension->Top_of_KBD_Stack)
			{
				IoDetachDevice(Extension->Top_of_KBD_Stack);
				DbgPrint("OnUnLoad: Keyboard device detached\n");
			}
		}
		IoDeleteDevice(DeviceObject);
		DbgPrint("OnUnLoad: Device deleted\n");
	}

	DbgPrint("OnUnLoad: ----Stopped!!!----\n");
	DbgPrint("OnUnLoad: Leave\n");

}

