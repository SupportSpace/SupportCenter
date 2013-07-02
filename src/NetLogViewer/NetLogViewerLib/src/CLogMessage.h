#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogMessage.h
///
///  NetLogMessage COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "resource.h"       // main symbols
#include <NetLog/CNetLogRuntimeStruct.h>
#include <map>

class CNetLogFieldsMap
{
private:
	std::map<int, tstring> m_fieldMap;
public:
	CNetLogFieldsMap()
	{
	TRY_CATCH
		for(int i=0; i<MAX_INFORM_FIELD_INDEX; ++i)
		{
			m_fieldMap[cLog::m_informFields[i].m_type] = cLog::m_informFields[i].m_informString;
		}
	CATCH_LOG()
	}

	tstring GetFieldIndex(cLog::eInformFieldType type)
	{
	TRY_CATCH
		return m_fieldMap[type];
	CATCH_THROW()
	}
};


// ILogMessage
[
	object,
	uuid("ADDE7CED-19C0-436F-8A98-25FDDB0EF282"),
	dual,	helpstring("ILogMessage Interface"),
	pointer_default(unique)
]
__interface ILogMessage : IDispatch
{
	[propget, id(1), helpstring("property Message")] HRESULT Message([out, retval] BSTR* pVal);
	[propput, id(1), helpstring("property Message")] HRESULT Message([in] BSTR newVal);
	[propget, id(2), helpstring("property PID")] HRESULT PID([out, retval] BSTR* pVal);
	[propput, id(2), helpstring("property PID")] HRESULT PID([in] BSTR newVal);
	[propget, id(3), helpstring("property TID")] HRESULT TID([out, retval] BSTR* pVal);
	[propput, id(3), helpstring("property TID")] HRESULT TID([in] BSTR newVal);
	[propget, id(4), helpstring("property AddedDate")] HRESULT AddedDate([out, retval] DATE* pVal);
	[propput, id(4), helpstring("property AddedDate")] HRESULT AddedDate([in] DATE newVal);
	[propget, id(5), helpstring("property Severity")] HRESULT Severity([out, retval] SHORT* pVal);
	[propput, id(5), helpstring("property Severity")] HRESULT Severity([in] SHORT newVal);
	[propget, id(6), helpstring("property CallStack")] HRESULT CallStack([out, retval] BSTR* pVal);
	[propput, id(6), helpstring("property CallStack")] HRESULT CallStack([in] BSTR newVal);
	[propget, id(7), helpstring("property FileName")] HRESULT FileName([out, retval] BSTR* pVal);
	[propput, id(7), helpstring("property FileName")] HRESULT FileName([in] BSTR newVal);
	[propget, id(8), helpstring("property CompileDate")] HRESULT CompileDate([out, retval] BSTR* pVal);
	[propput, id(8), helpstring("property CompileDate")] HRESULT CompileDate([in] BSTR newVal);
	[propget, id(9), helpstring("property FileLine")] HRESULT FileLine([out, retval] SHORT* pVal);
	[propput, id(9), helpstring("property FileLine")] HRESULT FileLine([in] SHORT newVal);
	[propget, id(10), helpstring("property ModuleName")] HRESULT ModuleName([out, retval] BSTR* pVal);
	[propput, id(10), helpstring("property ModuleName")] HRESULT ModuleName([in] BSTR newVal);
	[propget, id(11), helpstring("property SysError")] HRESULT SysError([out, retval] BSTR* pVal);
	[propput, id(11), helpstring("property SysError")] HRESULT SysError([in] BSTR newVal);
	[propget, id(12), helpstring("property TestCase")] HRESULT TestCase([out, retval] BSTR* pVal);
	[propput, id(12), helpstring("property TestCase")] HRESULT TestCase([in] BSTR newVal);
	[propget, id(13), helpstring("property TestSuite")] HRESULT TestSuite([out, retval] BSTR* pVal);
	[propput, id(13), helpstring("property TestSuite")] HRESULT TestSuite([in] BSTR newVal);
	[id(14), helpstring("method Decode")] HRESULT Decode([in] BYTE* buffer, [in] LONG size);
	[propget, id(15), helpstring("property ReceivedDate")] HRESULT ReceivedDate([out, retval] DATE* pVal);
	[propput, id(15), helpstring("property ReceivedDate")] HRESULT ReceivedDate([in] DATE newVal);
	[id(16), helpstring("method Serialize2String")] HRESULT Serialize2String([out] BSTR* str);
	[id(17), helpstring("method DeserializeFromString")] HRESULT DeserializeFromString(BSTR str);
};



// CLogMessage

[
	coclass,
	default(ILogMessage),
	threading(free),
	support_error_info("ILogMessage"),
	aggregatable(never),
	vi_progid("NetLogViewerLib.LogMessage"),
	progid("NetLogViewerLib.LogMessage.1"),
	version(1.0),
	uuid("F9515289-DD30-4850-8E0E-E2897E794837"),
	helpstring("LogMessage Class")
]
class ATL_NO_VTABLE CLogMessage :
	public ILogMessage
{
	/// Internal log message structure
	CNetLogRuntimeStruct m_logMessage;

	/// Message received date
	cDate m_receivedDate;

	/// Message severity
	SHORT m_severity;

public:
	
	/// Initializes object instance
	CLogMessage();

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

public:
	STDMETHOD(get_Message)(BSTR* pVal);
public:
	STDMETHOD(put_Message)(BSTR newVal);
public:
	STDMETHOD(get_PID)(BSTR* pVal);
public:
	STDMETHOD(put_PID)(BSTR newVal);
public:
	STDMETHOD(get_TID)(BSTR* pVal);
public:
	STDMETHOD(put_TID)(BSTR newVal);
public:
	STDMETHOD(get_AddedDate)(DATE* pVal);
public:
	STDMETHOD(put_AddedDate)(DATE newVal);
public:
	STDMETHOD(get_Severity)(SHORT* pVal);
public:
	STDMETHOD(put_Severity)(SHORT newVal);
public:
	STDMETHOD(get_CallStack)(BSTR* pVal);
public:
	STDMETHOD(put_CallStack)(BSTR newVal);
public:
	STDMETHOD(get_FileName)(BSTR* pVal);
public:
	STDMETHOD(put_FileName)(BSTR newVal);
public:
	STDMETHOD(get_CompileDate)(BSTR* pVal);
public:
	STDMETHOD(put_CompileDate)(BSTR newVal);
public:
	STDMETHOD(get_FileLine)(SHORT* pVal);
public:
	STDMETHOD(put_FileLine)(SHORT newVal);
public:
	STDMETHOD(get_ModuleName)(BSTR* pVal);
public:
	STDMETHOD(put_ModuleName)(BSTR newVal);
public:
	STDMETHOD(get_SysError)(BSTR* pVal);
public:
	STDMETHOD(put_SysError)(BSTR newVal);
public:
	STDMETHOD(get_TestCase)(BSTR* pVal);
public:
	STDMETHOD(put_TestCase)(BSTR newVal);
public:
	STDMETHOD(get_TestSuite)(BSTR* pVal);
public:
	STDMETHOD(put_TestSuite)(BSTR newVal);
public:
	STDMETHOD(Decode)(BYTE* buffer, LONG size);
public:
	STDMETHOD(get_ReceivedDate)(DATE* pVal);
public:
	STDMETHOD(put_ReceivedDate)(DATE newVal);
public:
	STDMETHOD(Serialize2String)(BSTR* str);
public:
	STDMETHOD(DeserializeFromString)(BSTR str);
};

