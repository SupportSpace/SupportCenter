/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptControlEventHandler.h
///
///  Implements CScriptControlEventHandler class, responsible for handling
///    events of DcriptControl object
///
///  @author Dmitry Netrebenko @date 19.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "msscript.tlh"
#include <atlbase.h>
#include <atlcom.h>

class CScriptControlEventHandler
	:	public IDispEventImpl<0, CScriptControlEventHandler, &__uuidof(DScriptControlSource), &__uuidof(__MSScriptControl), 1, 0>
{
private:
/// Prevents making copies of CScriptControlEventHandler objects.
	CScriptControlEventHandler( const CScriptControlEventHandler& );
	CScriptControlEventHandler& operator=( const CScriptControlEventHandler& );
public:
/// Constructor
	CScriptControlEventHandler() 
	{
	};
/// Destructor
	~CScriptControlEventHandler()
	{
	};
protected:
/// OnError event handler
	virtual HRESULT __stdcall OnScriptControlError() = NULL;
/// OnTimeout event handler
	virtual HRESULT __stdcall OnScriptControlTimeout() = NULL;
public:
///  Sink map
	BEGIN_SINK_MAP(CScriptControlEventHandler)
		SINK_ENTRY_EX(0, __uuidof(DScriptControlSource), 3000, OnScriptControlError)
		SINK_ENTRY_EX(0, __uuidof(DScriptControlSource), 3001, OnScriptControlTimeout)
	END_SINK_MAP()
};
