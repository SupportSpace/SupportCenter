
#ifndef SSINPUT_COMMON_H
#define SSINPUT_COMMON_H

#include <ntddmou.h>
#include <ntddkbd.h>
#include "kbdmou.h"

#define MOUFILTER_POOL_TAG (ULONG) 'flSS'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, MOUFILTER_POOL_TAG)

#if DBG

#define TRAP()                      DbgBreakPoint()
#define DbgRaiseIrql(_x_,_y_)       KeRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)           KeLowerIrql(_x_)

#else   // DBG

#define TRAP()
#define DbgRaiseIrql(_x_,_y_)
#define DbgLowerIrql(_x_)

#endif


#define KEYBOARD_DEVICE	L"\\Device\\KeyboardClass0"

#define KBRD_QUEUE_SIZE ((ULONG)512)
#define MOUSE_QUEUE_SIZE ((ULONG)512)
#define MOUSE_BUFFER_SIZE ((ULONG)1)


typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT	Self;
    PDEVICE_OBJECT	Top_of_Mouse_Stack;
    PDEVICE_OBJECT	Top_of_KBD_Stack;
	PDEVICE_OBJECT	HookMouseDeviceObject;
	PDEVICE_OBJECT	HookKbrdDeviceObject;

	//User Mode communication Device
	PDEVICE_OBJECT	UserDeviceObject;
	UNICODE_STRING  DeviceLinkName;

	//
	// The real connect data that this driver reports to
	//
	CONNECT_DATA	UpperConnectData;

	BOOLEAN			Started;
	BOOLEAN			Removed;
	BOOLEAN			SurpriseRemoved;
	ULONG			DeviceState;

    //
	// Device Create Close counter
	//
	ULONG			EnableCount;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

extern UNICODE_STRING	DeviceName;
extern PDEVICE_OBJECT	DeviceObject;
BOOLEAN			InputDisabled;
BOOLEAN			DriverOpened;
LARGE_INTEGER	WaitTimeout;

// Keyboard queue ---------------------
LIST_ENTRY KbrdQueue;

typedef struct _KBRD_QUEUE_ENTRY
{
	KEYBOARD_INPUT_DATA	KbrdData;
	LIST_ENTRY			Entry;
} KBRD_QUEUE_ENTRY, *PKBRD_QUEUE_ENTRY;

ULONG		KbrdQueueSize;
ULONG		KbrdQueueCount;
KSPIN_LOCK	KbrdQueueLock;
KEVENT		KbrdEvent;
// ------------------------------------

// Mouse queue ------------------------
LIST_ENTRY MouseQueue;

typedef struct _MOUSE_QUEUE_ENTRY
{
	MOUSE_INPUT_DATA	MouseData;
	LIST_ENTRY			Entry;
} MOUSE_QUEUE_ENTRY, *PMOUSE_QUEUE_ENTRY;

ULONG		MouseQueueSize;
ULONG		MouseQueueCount;
KSPIN_LOCK	MouseQueueLock;
KEVENT		MouseEvent;

ULONG		MouseBufferSize;
ULONG		MouseBufferCount;
LONG		MouseRelX;
LONG		MouseRelY;
USHORT		MouseButtons;
BOOLEAN		MouseFirstPacket;
// ------------------------------------
#endif
