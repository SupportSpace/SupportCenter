/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SExecEnvironment.h
///
///  Declares SExecEnvironment structure, storage for remote execution environment
///
///  @author Dmitry Netrebenko @date 06.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>
#include <atlbase.h>
#include <atlcomcli.h>

///  SExecEnvironment structure, storage for remote execution environment
struct SExecEnvironment
{
	unsigned int	m_id;				/// Environment Id
	CComBSTR		m_completion;		/// Name of completion procedure
	CComBSTR		m_lang;				/// Name of engine (VBScript, JavaScript, etc)
	CComBSTR		m_proc;				/// Name of remote procedure to execute
	CComVariant		m_params1;			/// Completion parameter
	CComVariant		m_params2;			/// Completion parameter
	bool			m_params1Object;	/// Is parameter 1 is object
	bool			m_params2Object;	/// Is parameter 2 is object
	CComBSTR		m_file;				/// Script file name
	CComBSTR		m_code;				/// Script code
	CComVariant		m_inputParam1;		/// Input parameter for remote procedure
	CComVariant		m_inputParam2;		/// Input parameter for remote procedure
	SExecEnvironment()
		:	m_id(0)
		,	m_completion(L"")
		,	m_lang(L"")
		,	m_proc(L"")
		,	m_params1()
		,	m_params2()
		,	m_params1Object(false)
		,	m_params2Object(false)
		,	m_file(L"")
		,	m_code(L"")
		,	m_inputParam1()
		,	m_inputParam2()
	{};
};

/// Shared pointer to SExecEnvironment structure
typedef boost::shared_ptr<SExecEnvironment> SPExecEnvironment;
