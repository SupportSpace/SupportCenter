// MouseMonitor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include <conio.h>
#include "strsafe.h"
#include "setupapi.h"
#include <stddef.h>

#include "..\public\user.h"

typedef struct _DeviceData {
  TCHAR  *HardwareId;
  TCHAR  *Path; // symbolic link
  TCHAR  *FriendlyName;
  DWORD  DeviceInstance;
} DeviceData, *DeviceList;


HANDLE GetDeviceHandle();

int _tmain(int argc, _TCHAR* argv[])
{
	printf("Starting Mouse monitor \n");


	// Open SSInput Driver for access;
	HANDLE hFile =GetDeviceHandle();
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf(" Cannot open access to driver error %d \n",GetLastError());
		return 0;
	}

	DWORD dResultExec = 0;
	DWORD dResult = 0;
	DWORD in,out;
	OVERLAPPED overl;

	ZeroMemory (&overl, sizeof(OVERLAPPED));
	overl.hEvent = CreateEvent( NULL, TRUE,FALSE, NULL);

	//Enable Mouse redirection
	dResult = DeviceIoControl ( hFile, CTRL_SSINPUT_ENABLE, &in, sizeof(DWORD),&out, sizeof(DWORD), &dResult,  &overl);

	if (dResult == 0)
	{
		printf("Driver refused to monitor mouse \n");
		CloseHandle (hFile);
		return 0;
	}

	printf ("Monitoring ... \n");
	while (!_getch_nolock())
	{
		Sleep(1000);
		printf ("Mouse set is \n");
	}

		//Enable Mouse redirection
	DeviceIoControl ( hFile, CTRL_SSINPUT_DISABLE, NULL, 0,NULL, 0, &dResult, NULL);

	if (dResult != 0)
		printf("Driver refused to stop monitor \n");
		CloseHandle (hFile);
	return 0;
}

HANDLE GetDeviceHandle()
{

	GUID InterfaceClassGuid = InterfaceClassGuidConstant;
	const DWORD nMemberIndex = 5; //Up to five inerfaces
	DWORD nSize=0;
	DWORD  nStatus,i;
	// Define a handle to a device information set.
	HDEVINFO  hdevClassInfo; 
	// Allocate array that will hold information about the devices that
	// are associated with this interface class.
	SP_DEVICE_INTERFACE_DATA  DeviceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA  DeviceDataDetails =NULL;
	SP_DEVINFO_DATA DevInfoData;

	// Retrieve a device information set.
	hdevClassInfo = SetupDiGetClassDevs( &InterfaceClassGuid, NULL, NULL,
											DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hdevClassInfo == INVALID_HANDLE_VALUE) {
		printf ("SetupDiGetClassDevs : Cannot Setup Interface error %Xh  \n",GetLastError());
        return INVALID_HANDLE_VALUE;
	}


	DeviceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

	DeviceList DevList[nMemberIndex + 1];
    
	// Enumerate devices that are associated with the interface.
	for (i = 0; i < nMemberIndex; i++)
	{
		nStatus = SetupDiEnumDeviceInterfaces
			         (hdevClassInfo, NULL,
				     (LPGUID)&InterfaceClassGuid, i,
					 &DeviceData);
		if (nStatus != TRUE) 
			break;

		// Retrieve the size of the device data.
		nStatus = SetupDiGetDeviceInterfaceDetail(hdevClassInfo, &DeviceData, 
												  NULL, 0, &nSize, NULL);                  

		// Allocate memory for the device detail buffer. 
		DeviceDataDetails = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new TCHAR[offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA, DevicePath) +nSize];
		

		// Initialize variables.
		DeviceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
		DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		DeviceDataDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

	DevList[i] = new _DeviceData();	
		nStatus = SetupDiGetDeviceInterfaceDetail(hdevClassInfo, &DeviceData,
												DeviceDataDetails, nSize, NULL,&DevInfoData);
		if (nStatus = FALSE) {
			printf ("SetupDiGetDeviceInterfaceDetail : Cannot Setup Interface error %Xh  \n",GetLastError());
			return INVALID_HANDLE_VALUE;
		}


		//****************************************
		// Save the device interface path: 
		//           This path can be used to open 
		//           the interface with CreateFile.
		//****************************************

	    // Calculate the length of the path string. 
		// Add 1 for the terminating NULL character.
		size_t nLen = strlen(DeviceDataDetails->DevicePath) + 1; 
		DevList[i]->Path = new TCHAR[nLen];
		StringCchCopy(DevList[i]->Path, nLen, DeviceDataDetails->DevicePath);


		//****************************************
		// Retrieve registry values.
		//****************************************

		// Initialize variables that are used in registry
		// operations.
		DWORD dwRegType =0 ,dwRegSize =0;

		//****************************************
		// Retrieve the device friendly name.
		//****************************************

		// Query for the size of the friendly name.
/*		nStatus = SetupDiGetDeviceRegistryProperty(hdevClassInfo, &DevInfoData,
													SPDRP_FRIENDLYNAME, &dwRegType,
													NULL, 0, &dwRegSize);
		if (nStatus == FALSE) {
			printf ("SetupDiGetDeviceRegistryProperty : Cannot Setup Interface error %Xh  \n",GetLastError());
			return INVALID_HANDLE_VALUE;
		}
		// Save the device instance.
		DevList[i]->DeviceInstance = DevInfoData.DevInst;


		// Allocate buffer for the friendly name.
		TCHAR* pBuffer = new TCHAR[dwRegSize];
		// Retrieve the friendly name.
		nStatus = SetupDiGetDeviceRegistryProperty(hdevClassInfo, &DevInfoData,
												   SPDRP_FRIENDLYNAME, NULL,
												   (PBYTE) *pBuffer, dwRegSize, NULL);
		if (nStatus == FALSE) {
			printf ("SetupDiGetDeviceRegistryProperty : Cannot Setup Interface error %Xh  \n",GetLastError());
			return INVALID_HANDLE_VALUE;
		}

		// Store the friendly name for this device.
		DevList[i]->FriendlyName = pBuffer;

		// ************************************
		// Retrieve the hardware ID.
		// ************************************

		// Query for the size of the hardware ID.
		nStatus = SetupDiGetDeviceRegistryProperty(hdevClassInfo, &DevInfoData,
												   SPDRP_HARDWAREID, &dwRegType,
												    NULL, 0, &dwRegSize);
		if (nStatus == FALSE) {
			printf ("SetupDiGetDeviceRegistryProperty : Cannot Setup Interface error %Xh  \n",GetLastError());
			return INVALID_HANDLE_VALUE;
		} 

		// Allocate a buffer for the hardware ID.
		pBuffer = new TCHAR[dwRegSize];
		// Retrieve the hardware ID.
		nStatus = SetupDiGetDeviceRegistryProperty(hdevClassInfo, &DevInfoData,
													SPDRP_HARDWAREID, NULL,
													(PBYTE) *pBuffer, dwRegSize, NULL);
		if (nStatus == FALSE) {
			printf ("SetupDiGetDeviceRegistryProperty : Cannot Setup Interface error %Xh  \n",GetLastError());
			return INVALID_HANDLE_VALUE;
		} 
		// Store the hardware ID for this device.
		DevList[i]->HardwareId = pBuffer; */
	}

	if (!i)
	{
		printf ("No Driver found \n");
		return INVALID_HANDLE_VALUE; 
	}
	// Create a symbolic link to device interface
	LPCTSTR Path; 
	HANDLE DeviceInterfaceHandle;

	
	Path = DevList[0]->Path;
	DeviceInterfaceHandle = CreateFile (Path,
					                    0,
									    FILE_SHARE_READ |
										FILE_SHARE_WRITE,
										NULL,
										OPEN_EXISTING,
										FILE_FLAG_OVERLAPPED,
										NULL);
	if (DeviceInterfaceHandle == INVALID_HANDLE_VALUE) {
		printf ("SetupDiGetDeviceRegistryProperty : Cannot Setup Interface error %Xh  \n",GetLastError());
		return INVALID_HANDLE_VALUE;
	} 

	return DeviceInterfaceHandle;
}