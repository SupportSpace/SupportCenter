/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SScriptEngineRequest.h
///
///  Declares SScriptEngineRequest structure, storage for ScriptEngine request
///
///  @author Dmitry Netrebenko @date 16.01.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <windows.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>

/// SScriptEngineRequest structure, storage for ScriptEngine request
struct SScriptEngineRequest
{
	unsigned int											m_id;				/// Request id
	bool													m_result;			/// Request result
	tstring													m_error;			/// Error description
	bool													m_async;			/// Async request
	boost::shared_ptr<boost::remove_pointer<HANDLE>::type>	m_completeEvent;	/// Completion event
/// Default constructor
	SScriptEngineRequest()
		:	m_id(0)
		,	m_result(false)
		,	m_error(_T(""))
		,	m_async(true)
	{
		if(!m_async)
		{
			/// Create event
			m_completeEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle);
		}
	};
/// Constructor
/// @param requestId - id of request
/// @param async - async request
	SScriptEngineRequest(unsigned int requestId, bool async = true)
		:	m_id(requestId)
		,	m_result(false)
		,	m_error(_T(""))
		,	m_async(async)
	{
		if(!m_async)
		{
			/// Create event
			m_completeEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle);
		}
	};
/// Changes type of request
/// @param async - new async mode
	void ChangeType(bool async)
	{
		if(m_async != async)
		{
			m_async = async;
			if(m_async)
				m_completeEvent.reset();
			else
				m_completeEvent.reset(CreateEvent(NULL, TRUE, FALSE, NULL), CloseHandle);
		}
	}
};

/// Defines shared pointer to SScriptEngineRequest
typedef boost::shared_ptr<SScriptEngineRequest> SPScriptEngineRequest;
