#include "unload.h"
#include "common.h"

void UnloadDriver(
	IN PDRIVER_OBJECT DriverObject
)
{
	UNICODE_STRING		LinkName;
//	PDEVICE_EXTENSION	Extension;
//	PDEVICE_OBJECT		DeviceObject;
	
	RtlInitUnicodeString(&LinkName, LINK_NAME);
	IoDeleteSymbolicLink(&LinkName);
	RtlFreeUnicodeString(&LinkName);

/*
	DeviceObject = DriverObject->DeviceObject;
	
	if(DeviceObject)
	{
		Extension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
		if(Extension)
		{
			if(Extension->TopOfStack)
			{
				IoDetachDevice(Extension->TopOfStack);
				DbgPrint("MegaDriver: Device detached\n");
			}
		}
		IoDeleteDevice(DeviceObject);
		DbgPrint("MegaDriver: Device detleted\n");
	}
*/
	DbgPrint("MegaDriver: ----Stopped!!!----\n");
}

