/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScriptsEnumerator.h
///
///  Declares CScriptsEnumerator class, responsible for ScriptsEnumerator ActiveX
///
///  @author Dmitry Netrebenko @date 11.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"       // main symbols
#include <AidLib/Strings/tstring.h>
#include "CScriptDirectories.h"

#define SCRIPT_FILES_MASK			_T("*.zip")
#define SCRIPT_FILES_EXT			_T("zip")
#define SCRIPT_ALL_FILES_MASK		_T("*.*")

// IScriptsEnumerator
[
	object,
	uuid("0490220A-85BC-4B01-B4B6-CE33EE3D575F"),
	pointer_default(unique)
]
__interface IScriptsEnumerator : IUnknown
{
	[id(1), helpstring("method GetScriptList. Returns array of script names. "
		"Caller is responsible for freeing up strings memory and array memory")]	HRESULT GetScriptList([out] BSTR** names, [out] ULONG* count);
};

/// CScriptsEnumerator class, responsible for ScriptsEnumerator ActiveX
[
	coclass,
	default(IScriptsEnumerator),
	threading(both),
	support_error_info("IScriptsEnumerator"),
	vi_progid("ScriptEngine.ScriptsEnumerator"),
	progid("ScriptEngine.ScriptsEnumerator.1"),
	version(1.0),
	uuid("6AAD3340-D94A-4A0C-B314-8E2B9994859D"),
	helpstring("ScriptsEnumerator Class")
]
class ATL_NO_VTABLE CScriptsEnumerator :
	public IScriptsEnumerator
{
public:
	CScriptsEnumerator();
	~CScriptsEnumerator();

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

private:
/// Directoris of scripts
	CScriptDirectories m_dirs;
/// Extracts file name from find data
	tstring GetArchiveName(const WIN32_FIND_DATA* fileData);
public:
/// IScriptsEnumerator interface realization
	STDMETHOD(GetScriptList)(BSTR** names, ULONG* count);
};

