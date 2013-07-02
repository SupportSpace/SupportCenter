/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInterfaceMarshaler.h
///
///  Implements CInterfaceMarshaler class, responsible for interface marshaling
///    through GIT
///
///  @author Dmitry Netrebenko @date 10.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <AidLib/CException/CException.h>

/// CInterfaceMarshaler class, responsible for interface marshaling
///    through GIT
template<class Interface>
class CInterfaceMarshaler
{
private:
/// Prevents making copies of CInterfaceMarshaler objects
	CInterfaceMarshaler(const CInterfaceMarshaler&);
	CInterfaceMarshaler& operator=(const CInterfaceMarshaler&);
public:
/// Default constructor
	CInterfaceMarshaler() {};
/// Constructor
/// @param intf - interface for marshaling
	CInterfaceMarshaler(CComPtr<Interface> intf)
	{
	TRY_CATCH
		m_intf.Attach(intf);
	CATCH_THROW()
	};
/// Destructor
	~CInterfaceMarshaler() {};
/// Sets new marshaled interface
/// @param intf - interface for marshaling
	void SetInterface(CComPtr<Interface> intf)
	{
	TRY_CATCH
		m_intf.Attach(intf);
	CATCH_THROW()
	};
/// Returns marshaled interface
	CComPtr<Interface> GetInterface() const
	{
	TRY_CATCH
		CComPtr<Interface> intf;
		HRESULT res = m_intf.CopyTo(&intf);
		if((S_OK != res) || !intf)
		{
			SetLastError(res);
			throw MCException_Win(_T("Error at obtaining marshaled interface"));
		}
		return intf;
	CATCH_THROW()
	};
private:
/// Pointer to interface in Global Interface Table
	CComGITPtr<Interface> m_intf;
};

