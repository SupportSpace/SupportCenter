#pragma once

#define FILE_DEVICE_SSINPUT	0x00008400
#define INPUT_DEVICE_NAME _T("\\\\.\\SSInputDevice")
#define INPUT_DRIVER_NAME _T("SSInputDriver")
#define INPUT_DRIVER_PATH _T("\\SystemRoot\\system32\\drivers\\ssinput.sys")

#define CTRL_SSINPUT_ENABLE				CTL_CODE(FILE_DEVICE_SSINPUT, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTRL_SSINPUT_DISABLE			CTL_CODE(FILE_DEVICE_SSINPUT, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTRL_SSINPUT_GET_MOUSE_DATA		CTL_CODE(FILE_DEVICE_SSINPUT, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define CTRL_SSINPUT_GET_KBRD_DATA		CTL_CODE(FILE_DEVICE_SSINPUT, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct MOUSE_INPUT_DATA
{
	USHORT	UnitId;
	USHORT	Flags;
	union 
	{
		ULONG	Buttons;
		struct
		{
			USHORT	ButtonFlags;
			USHORT	ButtonData;
		};
	};
	ULONG	RawButtons;
	LONG	LastX;
	LONG	LastY;
	ULONG	ExtraInformation;
} MOUSE_INPUT_DATA, *PMOUSE_INPUT_DATA;

//
// Define the mouse button state indicators.
//

#define MOUSE_LEFT_BUTTON_DOWN   0x0001  // Left Button changed to down.
#define MOUSE_LEFT_BUTTON_UP     0x0002  // Left Button changed to up.
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define MOUSE_RIGHT_BUTTON_UP    0x0008  // Right Button changed to up.
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define MOUSE_MIDDLE_BUTTON_UP   0x0020  // Middle Button changed to up.

#define MOUSE_BUTTON_1_DOWN     MOUSE_LEFT_BUTTON_DOWN
#define MOUSE_BUTTON_1_UP       MOUSE_LEFT_BUTTON_UP
#define MOUSE_BUTTON_2_DOWN     MOUSE_RIGHT_BUTTON_DOWN
#define MOUSE_BUTTON_2_UP       MOUSE_RIGHT_BUTTON_UP
#define MOUSE_BUTTON_3_DOWN     MOUSE_MIDDLE_BUTTON_DOWN
#define MOUSE_BUTTON_3_UP       MOUSE_MIDDLE_BUTTON_UP

#define MOUSE_BUTTON_4_DOWN     0x0040
#define MOUSE_BUTTON_4_UP       0x0080
#define MOUSE_BUTTON_5_DOWN     0x0100
#define MOUSE_BUTTON_5_UP       0x0200

#define MOUSE_WHEEL             0x0400
#define MOUSE_HWHEEL		0x0800

//
// Define the mouse indicator flags.
//

#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_VIRTUAL_DESKTOP    0x02  // the coordinates are mapped to the virtual desktop
#define MOUSE_ATTRIBUTES_CHANGED 0x04  // requery for mouse attributes
#if(_WIN32_WINNT >= 0x0600)
#define MOUSE_MOVE_NOCOALESCE    0x08  // do not coalesce WM_MOUSEMOVEs
#endif /* _WIN32_WINNT >= 0x0600 */


#define MAGIC 0xFFFF

typedef struct _KEYBOARD_INPUT_DATA
{
	USHORT	UnitId;
	USHORT	MakeCode;
	USHORT	Flags;
	USHORT	Reserved;
	ULONG	ExtraInformation;
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;

#define KEY_MAKE  0
#define KEY_BREAK 1
#define KEY_E0    2
#define KEY_E1    4
#define KEY_TERMSRV_SET_LED 8
#define KEY_TERMSRV_SHADOW  0x10
#define KEY_TERMSRV_VKPACKET 0x20
