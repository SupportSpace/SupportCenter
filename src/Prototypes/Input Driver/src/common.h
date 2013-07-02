
#ifndef SSINPUT_COMMON_H
#define SSINPUT_COMMON_H

#include <ntddmou.h>
#include <ntddkbd.h>

#define DEVICE_NAME		L"\\Device\\SSInputDevice"
#define LINK_NAME		L"\\DosDevices\\SSInputDevice"
#define MOUSE_DEVICE	L"\\Device\\PointerClass0"
#define KEYBOARD_DEVICE	L"\\Device\\KeyboardClass0"

#define KBRD_QUEUE_SIZE ((ULONG)512)
#define MOUSE_QUEUE_SIZE ((ULONG)512)
#define MOUSE_BUFFER_SIZE ((ULONG)1)

#define FILE_DEVICE_SSINPUT	0x00008400

typedef struct _DEVICE_EXTENSION
{
    PDEVICE_OBJECT	TopOfStack;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


PDEVICE_OBJECT	HookMouseDeviceObject;
PDEVICE_OBJECT	LowerMouseDeviceObject;
PDEVICE_OBJECT	HookKbrdDeviceObject;
PDEVICE_OBJECT	LowerKbrdDeviceObject;
PDEVICE_OBJECT	SelfDeviceObject;

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
