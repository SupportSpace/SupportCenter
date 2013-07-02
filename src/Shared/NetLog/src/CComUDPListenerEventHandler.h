/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CComUDPListenerEventHandler.h
///
///  Declares CComUDPListenerEventHandler class, event handler for
///    OnDatagramReceived event from ComUDPListener object
///
///  @author Dmitry Netrebenko @date 02.04.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include "NetLogLib.tlh"
//#import "D:\Users\Max\WORK\SupportSpace\roots\console_static_debug\bin\NetLogLib.dll" no_namespace

///  CComUDPListenerEventHandler class, event handler for
///    OnDatagramReceived event from ComUDPListener object
///  Base class - IDispEventImpl
class CComUDPListenerEventHandler
	:	public IDispEventImpl<0, CComUDPListenerEventHandler, &__uuidof(_IComUDPListenerEvents), &__uuidof(__NetLogLib), 1, 0>
{
private:
///  Prevents making copies of CComUDPListenerEventHandler objects.
	CComUDPListenerEventHandler( const CComUDPListenerEventHandler& );
	CComUDPListenerEventHandler& operator=( const CComUDPListenerEventHandler& );

public:
///  Constructor
	CComUDPListenerEventHandler();

///  Destructor
	~CComUDPListenerEventHandler();

///  DatagramReceived event handler
	virtual HRESULT __stdcall OnDatagramReceived ( BSTR addr, long port ) = NULL;

public:
///  Sink map
	BEGIN_SINK_MAP(CComUDPListenerEventHandler)
		SINK_ENTRY_EX(0, __uuidof(_IComUDPListenerEvents), 1, OnDatagramReceived)
	END_SINK_MAP()

};
