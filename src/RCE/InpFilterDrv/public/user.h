#ifndef SSINPUT_USER_H
#define SSINPUT_USER_H
#include "initguid.h"

#define DEVICE_NAME		L"\\Device\\SSInputDevice"
#define LINK_NAME		L"\\DosDevices\\SSInputDevice"

#define FILE_DEVICE_SSINPUT	0x13119075

// {13119075-4CA2-4a62-8E7F-02171A9E3B20}
DEFINE_GUID(InterfaceClassGuidConstant, FILE_DEVICE_SSINPUT, 0x4ca2, 0x4a62, 0x8e, 0x7f, 0x2, 0x17, 0x1a, 0x9e, 0x3b, 0x20);


#define CTRL_SSINPUT_ENABLE				CTL_CODE(FILE_DEVICE_SSINPUT, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTRL_SSINPUT_DISABLE			CTL_CODE(FILE_DEVICE_SSINPUT, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTRL_SSINPUT_GET_MOUSE_DATA		CTL_CODE(FILE_DEVICE_SSINPUT, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTRL_SSINPUT_GET_KBRD_DATA		CTL_CODE(FILE_DEVICE_SSINPUT, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#endif
