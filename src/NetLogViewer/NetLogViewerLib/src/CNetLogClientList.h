#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogClientList.h
///
///  NetLogClientList COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "resource.h"       // main symbols
#include "CNetLogClient.h"
#include <vector>

// INetLogClientList
[
	object,
	uuid("9FC0E995-8BCB-4309-ADED-ECEF23C0B7A5"),
	dual,	helpstring("INetLogClientList Interface"),
	pointer_default(unique)
]
__interface INetLogClientList : IDispatch
{
	[propget, id(DISPID_NEWENUM), helpstring("property _NewEnum"), restricted] HRESULT _NewEnum([out, retval] LPUNKNOWN *pVal);
	[propget, id(DISPID_VALUE), helpstring("property Item")] HRESULT Item([in] long lIndex, [out, retval] VARIANT *pVal);
	[id(1), helpstring("method AddClient")] HRESULT AddClient([in] INetLogClient* client);
};

// Simulate Deep copy semantics for the elements in our collection 
class _CopyPolicyNetLogClient
{
public:

  static HRESULT copy(VARIANT* pVarDest,INetLogClient *const *__w64 netLogClient)
  {
    // Assign to a CComVariant 
    CComVariant varNetLogClient(*netLogClient); 

    // Perform a deep copy
    return ::VariantCopy(pVarDest,&varNetLogClient);
	  return S_OK;
  }

  static void init(VARIANT* pVar) 
  {
    pVar->vt = VT_EMPTY;
  }

  static void destroy(VARIANT* pVar) 
  {
    VariantClear(pVar);
  }

};

// Define an STL vector to hold all clients
typedef std::vector<INetLogClient*> NetLogClientVector;

// Define a COM Enumerator based on our NetLogClientVector
typedef CComEnumOnSTL< IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyPolicyNetLogClient, NetLogClientVector > VarEnum;

// Collection Class Helper for STL based containers
//typedef ICollectionOnSTLImpl< IUnknown*,  NetLogClientVector, VARIANT, _CopyPolicyNetLogClient, VarEnum> INetLogClientsCollection;

namespace NetLogClientList
{

/// Com class for NetLog clients enumeration
typedef NetLogClientVector CollType;
typedef VARIANT ItemType;
typedef _CopyPolicyNetLogClient CopyItem;
typedef VarEnum EnumType;
[
	coclass,
	default(INetLogClientList),
	threading(free),
	aggregatable(never),
	vi_progid("NetLogViewerLib.NetLogClientList"),
	progid("NetLogViewerLib.NetLogClientList.1"),
	version(1.0),
	uuid("F844B23B-DE07-4CD7-850B-211E715E064A"),
	helpstring("NetLogClientList Class")
]
class ATL_NO_VTABLE CNetLogClientList : public INetLogClientList
{
public:
	CNetLogClientList()
	{
	}

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	//-------ICollectionOnSTLImpl--------------------------------------------------------------
public:
	STDMETHOD(get_Count)(long* pcount)
	{
		if (pcount == NULL)
			return E_POINTER;
		ATLASSUME(m_coll.size()<=LONG_MAX);

		*pcount = (long)m_coll.size();

		return S_OK;
	}
	STDMETHOD(get_Item)(long Index, ItemType* pvar)
	{
		//Index is 1-based
		if (pvar == NULL)
			return E_POINTER;
		if (Index < 1)
			return E_INVALIDARG;
		HRESULT hr = E_FAIL;
		Index--;
		CollType::const_iterator iter = m_coll.begin();
		while (iter != m_coll.end() && Index > 0)
		{
			iter++;
			Index--;
		}
		if (iter != m_coll.end())
			hr = CopyItem::copy(pvar, &*iter);
		return hr;
	}
	STDMETHOD(get__NewEnum)(IUnknown** ppUnk)
	{
		if (ppUnk == NULL)
			return E_POINTER;
		*ppUnk = NULL;
		HRESULT hRes = S_OK;
		CComObject<EnumType>* p;
		hRes = CComObject<EnumType>::CreateInstance(&p);
		if (SUCCEEDED(hRes))
		{
			IUnknown *pUnk;
			hRes == QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&pUnk));
			if (hRes != S_OK)
				return hRes; 
			hRes = p->Init(pUnk, m_coll);
			if (hRes == S_OK)
				hRes = p->QueryInterface(__uuidof(IUnknown), (void**)ppUnk);
		}
		if (hRes != S_OK)
			delete p;
		return hRes;
	}
	CollType m_coll;
	//-------ICollectionOnSTLImpl--------------------------------------------------------------
public:
	STDMETHOD(AddClient)(INetLogClient* client);
};

};//namespace NetLogClientList