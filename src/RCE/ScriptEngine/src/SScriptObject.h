/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SScriptObject.h
///
///  Declares SScriptObject structure, stores data for registration objects
///    in ScriptControl
///
///  @author Dmitry Netrebenko @date 20.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>

///  SScriptObject structure, stores data for registration objects
///    in ScriptControl
struct SScriptObject
{
	CComBSTR					m_name;			/// Variable name
	CComPtr<IDispatch>			m_object;		/// Pointer to IDispatch interface of object
	VARIANT_BOOL				m_addMemmber;	/// Variable should be added to script
	SScriptObject(const CComBSTR name, const bool addMember)
		:	m_object(NULL)
		,	m_addMemmber(addMember ? VARIANT_TRUE : VARIANT_FALSE)
		,	m_name(name)
	{}
};

/// Shared pointer to SScriptObject object
typedef boost::shared_ptr<SScriptObject> SPScriptObject;
