#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSrvComm.h
///
///  CRCHost -> CService communicators
///
///  @author "Archer Software" Sogin M. @date 03.10.2007
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/CException/CException.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <map>

#define DEF_WND_CLS_NAME _T("ServiceCommunicatorWindow")

typedef boost::function<DWORD (WORD code, char* msg, WORD msgSize)> MsgHandler;

/// Message codes
#define SRVCOMM_BOUND_PROCESS		0
#define SRVCOMM_SEND_MOUSE_INPUT	1
#define SRVCOMM_SEND_KBD_INPUT		2
#define SRVCOMM_GET_DESKTOP			3
#define SRVCOMM_SET_VNCHOOKS		4
#define SRVCOMM_SEND_CAD			5
#define SRVCOMM_RESET_WALLPAPER		6
#define SRVCOMM_TOGGLE_TASKBAR		7
#define SRVCOMM_ALLOW_CONNECTIONS	8
#define SRVCOMM_START_BROKER		9

/// Message structures

/// Send mouse event message
typedef struct _SSendMouseEventMsg
{
	DWORD flags;
	DWORD x,y;
	DWORD wheelMovement;
	DWORD extraInfo;
	bool  toWinlogon;
	INPUT evt;
	bool useSendInput;
} SSendMouseEventMsg;

/// Send keyboard event message
typedef struct _SSendKeyEventMsg
{
	BYTE vkCode;
	DWORD flags;
	DWORD extraInfo;
	bool  toWinlogon;
} SSendKeyEventMsg;

/// Set / reser vnc hooks message
typedef struct _SSetVNCHooksMsg
{
	DWORD tid;
	bool start;
	UINT rfbScreenUpdate;
	UINT rfbCopyRectUpdate;
	UINT rfbMouseUpdate;
	bool ddihook;
	bool localInputsDisabled;
} SSetVNCHooksMsg;

/// Set / reset wallpaper
typedef struct _SHideWallpaper
{
	DWORD pid;
	bool hide;
} SHideWallpaper;

/// Show hide taskbar
typedef struct _SToggleTaskbar
{
	DWORD pid;
	bool show;
} SToggleTaskbar;

/// Allow / disable socket connections
typedef struct _SAllowConnections
{
	DWORD pid;
	bool enabled;
} SAllowConnections;

/// Start broker object
typedef struct _SStartBroker
{
	/// Size of local buffer
	unsigned int bufSize;
	/// Pointer to local buffer
	char* buf;
} SStartBroker;

/// Base class for service communication
class CSrvComm
{
private:
	/// Map of message handlers
	std::map<WORD, MsgHandler> m_handlersMap;

public:
	/// Sends message to service
	virtual DWORD SendMsg(WORD code, char* msg, const WORD msgSize) = NULL;

	/// Should be called by some proxy class
	inline DWORD OnMsgReceived(WORD code, char* msg, const WORD msgSize)
	{
	TRY_CATCH
		std::map<WORD, MsgHandler>::iterator handler = m_handlersMap.find(code);
		if (m_handlersMap.end() != handler)
		{
			return handler->second(code, msg, msgSize);
		} else
		{
			Log.Add(_WARNING_,_T("Message handler for %d not found"),code);
			return FALSE;
		}
	CATCH_THROW()
	}

	/// Registers new
	inline void RegisterMsgHandler(WORD code, MsgHandler msgHandler)
	{
	TRY_CATCH
		m_handlersMap[code] = msgHandler;
	CATCH_THROW()
	}
};

/// Communication with service through system queue (SendMessage/GetMessage)
class CSrvSTDQueueComm : public CSrvComm
{
private:
	/// Server window for communication
	HWND m_hWnd;
	/// Client process handle
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type> m_clientProcess;
	/// name of window class to search
	tstring m_windowClassName;
public:

	/// Communication message code
	const unsigned int m_commMsg;

	/// default ctor
	CSrvSTDQueueComm() 
		:	m_hWnd(NULL),
			m_commMsg(RegisterWindowMessage(_T("CSrvSTDQueueComm::m_commMsg")))
	{}

	/// ctor.
	/// @param windowClassName name of window class to search
	CSrvSTDQueueComm(const tstring& windowClassName)
		:	m_commMsg(RegisterWindowMessage(_T("CSrvSTDQueueComm::m_commMsg"))),
			m_windowClassName(windowClassName),
			m_hWnd(NULL)
	{
	TRY_CATCH
		Bind();
	CATCH_THROW()
	}

	/// Binds communicator to service
	/// Bind to window by window class. First window with matching window class is taken
	void Bind()
	{
	TRY_CATCH
		/// Looking for window
		m_hWnd = FindWindow(m_windowClassName.c_str() ,NULL);
		if (NULL == m_hWnd)
			throw MCException_Win(Format("Failed to find window with class %s ",m_windowClassName.c_str()));
		/// Binding process
		if (FALSE != SendMsg(SRVCOMM_BOUND_PROCESS, reinterpret_cast<char*>(GetCurrentProcessId()), 0))
			Log.Add(_MESSAGE_,_T("CSrvSTDQueueComm: Bound to service"));
	CATCH_THROW()
	}

	/// Almost the same as bind, but without superfluous logging if binding wasn't broken
	void ReBind()
	{
	TRY_CATCH
		if (NULL == m_hWnd || FALSE == SendMsg(SRVCOMM_BOUND_PROCESS, reinterpret_cast<char*>(GetCurrentProcessId()), 0))
			Bind();
	CATCH_THROW()
	}

	boost::shared_ptr<boost::remove_pointer<HANDLE>::type> GetClientProcess()
	{
	TRY_CATCH
		return m_clientProcess;
	CATCH_THROW()
	}

	/// Sends message to service
	virtual DWORD SendMsg(WORD code, char* msg, const WORD msgSize)
	{
	TRY_CATCH
		return static_cast<DWORD>(SendMessage(m_hWnd, m_commMsg, reinterpret_cast<WPARAM>(msg), MAKELONG(code,msgSize)));
	CATCH_THROW()
	}

	/// Callback for owner class
	/// Should be called on message with m_commMsg code reception
	DWORD OnMsgReceived(WPARAM wParam, LPARAM lParam)
	{
	TRY_CATCH

		WORD code = LOWORD(lParam);
		WORD msgSize = HIWORD(lParam);

		if (SRVCOMM_BOUND_PROCESS == code) /// Bound client process message received
		{
			unsigned int pid = static_cast<unsigned int>(wParam);
			m_clientProcess.reset(OpenProcess(PROCESS_DUP_HANDLE | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION , NULL, pid), CloseHandle);
			if (NULL == m_clientProcess.get())
			{
				Log.WinError(_ERROR_,_T("Failed to open client process pid(%d) "),pid);
				return FALSE;
			}
			Log.Add(_MESSAGE_,_T("CSrvSTDQueueComm: Connected to client"));
			return TRUE;
		}

		if (NULL == m_clientProcess.get())
		{
			Log.Add(_WARNING_,_T("Message code(%d) received, while client process still not bound"),code);
			return FALSE;
		}

		if (0 == msgSize)
		{
			return CSrvComm::OnMsgReceived(code, NULL, 0);
		} else
		{
			/// Reading client process memory
			boost::scoped_ptr<char> msg;
			msg.reset(new char[msgSize]);
			if (NULL == msg.get())
			{
				Log.WinError(_ERROR_,_T("Failed to allocate %d bytes "),msgSize);
				return FALSE;
			}
			SIZE_T read;
			if (0 == ReadProcessMemory(m_clientProcess.get(), reinterpret_cast<LPCVOID>(wParam), msg.get(), msgSize, &read))
			{
				Log.WinError(_ERROR_,_T("Failed to ReadProcessMemory"));
				return FALSE;
			}
			return CSrvComm::OnMsgReceived(code, msg.get(), msgSize);
		}

	CATCH_LOG()
		return 0;
	}
};

