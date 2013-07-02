/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSafeEventsRaiser.h
///
///  Safe connection point events invoking
///
///  @author "Archer Software" Sogin M. @date 10.01.2007
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/CException/CException.h>
#include <boost/scoped_array.hpp>
#include <stdio.h>
#include <stdarg.h>

/// Safe connection point events invoking
template<class ComClass, class EventsInterface> 
class CSafeEventsRaiser
{
private:
	/// Setup to true when object is deleted
	/// This variable could cause 1 byte memory leak, but this is deliberate
	/// solution, cause there isn't other way to determinate if object was deleted from within object
	bool *m_deleted;
public:
	/// Class ctor
	CSafeEventsRaiser()
		:	m_deleted(new bool)
	{
		*m_deleted = false;
	}
	/// Destructor
	~CSafeEventsRaiser()
	{
		*m_deleted = true;
	}

/// Invokes event by it's id
/// @param id event id
/// @param args event arguments
/// @param count count of arguments
/// @returns S_OK in case of success
	HRESULT SafeInvokeEvent(int id, VARIANT* args, int count);
/// Invokes event by it's id
/// @param id event id
/// @returns S_OK in case of success
/// @remarks Event's parameters should be necessarily wrapped up in CComVariant, last parameter should be NULL
	HRESULT SafeInvokeEvent(int id, ...);

/// Connection point implementation
	inline HRESULT __ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult);
	STDMETHOD(Advise)(IUnknown* pUnkSink,DWORD* pdwCookie);
	STDMETHOD(Unadvise)(DWORD dwCookie);
};

template<class ComClass, class EventsInterface> 
inline HRESULT CSafeEventsRaiser<ComClass,EventsInterface>::SafeInvokeEvent(int id, VARIANT* args, int count)
{
TRY_CATCH
	HRESULT hr = S_OK;
	ComClass* _this = static_cast<ComClass*>(this);
	IConnectionPointImpl<ComClass, &__uuidof(EventsInterface), CComDynamicUnkArray>* p = _this;
	_this->Lock();
	std::vector<IUnknown**> vec;
	for(IUnknown** pp = p->m_vec.begin();
		pp < p->m_vec.end();
		++pp)
	{
		vec.push_back(pp);
	}
	_this->Unlock();
	bool* deleted = m_deleted;
	for(std::vector<IUnknown**>::iterator pp = vec.begin();
		pp != vec.end();
		++pp)
	{
		if (*deleted)
		{
			/// Object was deleted, events doesn't handled further
			delete deleted;
			break;
		}
		if (**pp != NULL) 
		{
			IDispatch* pDispatch = (IDispatch*) **pp;
			DISPPARAMS disp = { args, NULL, count, 0 };
			VARIANT ret_val;
			hr = this->__ComInvokeEventHandler(pDispatch, id, 1, &disp, &ret_val);
			if (FAILED(hr)) 
			{
				break;
			}
		}
	}
	return hr;
CATCH_THROW()
}

template<class ComClass, class EventsInterface> 
inline HRESULT CSafeEventsRaiser<ComClass,EventsInterface>::SafeInvokeEvent(int id, ...)
{
TRY_CATCH
	HRESULT hr = S_OK;
	ComClass* _this = static_cast<ComClass*>(this);
	IConnectionPointImpl<ComClass, &__uuidof(EventsInterface), CComDynamicUnkArray>* p = _this;
	_this->Lock();
	std::vector<IUnknown**> vec;
	for(IUnknown** pp = p->m_vec.begin();
		pp < p->m_vec.end();
		++pp)
	{
		vec.push_back(pp);
	}
	_this->Unlock();
	bool* deleted = m_deleted;

	va_list argptr;
	va_start(argptr, id);
	int count = 0;
	while(*argptr)
	{
		CComVariant var = va_arg(argptr, CComVariant);
		count++;
	}
	va_start(argptr, id);
	boost::scoped_array<CComVariant> args;
	if(count)
		args.reset(new CComVariant[count]);
	for(int i = count - 1; i >= 0; --i)
	{
		args[i] = va_arg(argptr, CComVariant);
	}
	va_end(argptr);

	for(std::vector<IUnknown**>::iterator pp = vec.begin();
		pp != vec.end();
		++pp)
	{
		if (*deleted)
		{
			/// Object was deleted, events doesn't handled further
			delete deleted;
			break;
		}
		if (**pp != NULL) 
		{
			IDispatch* pDispatch = (IDispatch*) **pp;
			DISPPARAMS disp = { args.get(), NULL, count, 0 };
			VARIANT ret_val;
			hr = this->__ComInvokeEventHandler(pDispatch, id, 1, &disp, &ret_val);
			if (FAILED(hr)) 
			{
				break;
			}
		}
	}
	return hr;
CATCH_THROW()
}

template<class ComClass, class EventsInterface> 
HRESULT CSafeEventsRaiser<ComClass,EventsInterface>::__ComInvokeEventHandler(IDispatch* pDispatch, DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult)
{
TRY_CATCH
	CComPtr<IGlobalInterfaceTable> git;
	HRESULT hr;
	hr = ::CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IGlobalInterfaceTable),(void**)&git);
	CComPtr<IUnknown> unkn;
	git->GetInterfaceFromGlobal(reinterpret_cast<DWORD>(pDispatch),IID_IUnknown,(void**)&unkn);
	CComPtr<IDispatch> disp;
	unkn->QueryInterface(&disp);
	return ::__ComInvokeEventHandler(disp, id, wFlags, pDispParams, pVarResult);
CATCH_THROW()
}

template<class ComClass, class EventsInterface> 
STDMETHODIMP CSafeEventsRaiser<ComClass,EventsInterface>::Advise(IUnknown* pUnkSink,DWORD* pdwCookie)
{
TRY_CATCH
	ComClass* pT = static_cast<ComClass*>(this);
	IUnknown* p;
	HRESULT hRes = S_OK;
	if (pdwCookie != NULL)
		*pdwCookie = 0;
	if (pUnkSink == NULL || pdwCookie == NULL)
		return E_POINTER;
	IID iid;
	pT->IConnectionPointImpl<ComClass, &__uuidof(EventsInterface), CComDynamicUnkArray>::GetConnectionInterface(&iid);
	hRes = pUnkSink->QueryInterface(iid, (void**)&p);
	if (SUCCEEDED(hRes))
	{
		pT->Lock();
		*pdwCookie = pT->IConnectionPointImpl<ComClass, &__uuidof(EventsInterface), CComDynamicUnkArray>::m_vec.Add(reinterpret_cast<IUnknown*>(CComGITPtr<IUnknown>(p).Detach()));
		hRes = (*pdwCookie != NULL) ? S_OK : CONNECT_E_ADVISELIMIT;
		pT->Unlock();
		p->Release();
	}
	else if (hRes == E_NOINTERFACE)
		hRes = CONNECT_E_CANNOTCONNECT;
	if (FAILED(hRes))
		*pdwCookie = 0;
	return hRes;
CATCH_THROW()
}

template<class ComClass, class EventsInterface> 
STDMETHODIMP CSafeEventsRaiser<ComClass,EventsInterface>::Unadvise(DWORD dwCookie)
{
TRY_CATCH
	ComClass* pT = static_cast<ComClass*>(this);
	pT->Lock();
	IUnknown* p = pT->IConnectionPointImpl<ComClass, &__uuidof(EventsInterface), CComDynamicUnkArray>::m_vec.GetUnknown(dwCookie);
	CComGITPtr<IUnknown>(reinterpret_cast<DWORD>(p));
	HRESULT hRes = pT->IConnectionPointImpl<ComClass, &__uuidof(EventsInterface), CComDynamicUnkArray>::m_vec.Remove(dwCookie) ? S_OK : CONNECT_E_NOCONNECTION;
	pT->Unlock();
	return hRes;
CATCH_THROW()
}


#define SAFEEVENTS_CONNECTIONPOINT_IMPL(ComClass,EventsInterface)	public: \
	STDMETHOD(Advise)(IUnknown* pUnkSink,DWORD* pdwCookie) \
	{ \
		return CSafeEventsRaiser<ComClass,EventsInterface>::Advise(pUnkSink,pdwCookie); \
	}; \
	STDMETHOD(Unadvise)(DWORD dwCookie) \
	{ \
		return CSafeEventsRaiser<ComClass,EventsInterface>::Unadvise(dwCookie); \
	};
