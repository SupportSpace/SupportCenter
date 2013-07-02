/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSafeEvents.h
///
///  Safe connection point events invoking
///
///  @author "Archer Software" Sogin M. @date 10.01.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

/// Safe connection point events invoking
template<class ComClass, class EventsInterface> class CSafeEvents
{
private:
	/// Setup to true when object is deleted
	/// This variable could cause 1 byte memory leak, but this is deliberate
	/// solution, cause there isn't other way to determinate if object was deleted from within object
	bool *m_deleted;
public:
	/// Class ctor
	CSafeEvents()
		:	m_deleted(new bool)
	{
		*m_deleted = false;
	}

	~CSafeEvents()
	{
		*m_deleted = true;
	}

	/// Invokes event whithout params by it's id
	/// @param id event id
	/// @returns S_OK in case of success
	HRESULT InvokeEvent(int id)
	{
	TRY_CATCH
		VARIANT args[1];
		VARIANT rgvars[1];
		::VariantInit(&rgvars[0]);
		rgvars[0].vt = VT_EMPTY;
		return InvokeEvent(id, rgvars);
	CATCH_THROW("CSafeConnectionPoint::InvokeEvent")
	}

	/// Invokes event with one int param by it's id
	/// @param id event id
	/// @param param int parameter
	/// @returns S_OK in case of success
	HRESULT InvokeEvent(int id, int param)
	{
	TRY_CATCH
		VARIANT rgvars[1];
		::VariantInit(&rgvars[0]);
		rgvars[0].vt = VT_I4;
		V_I4(&rgvars[0])= param;
		return InvokeEvent(id, rgvars);
	CATCH_THROW("CSafeConnectionPoint::InvokeEvent")
	}

	/// Invokes event by it's id
	/// @param id event id
	/// @param args event arguments
	/// @returns S_OK in case of success
	HRESULT InvokeEvent(int id, VARIANT args[1])
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
                DISPPARAMS disp = { args, NULL, 1, 0 };
                VARIANT ret_val;
                hr = __ComInvokeEventHandler(pDispatch, id, 1, &disp, &ret_val);
                if (FAILED(hr)) 
				{
                    break;
                }
            }
		}
		return hr;
	CATCH_THROW("CSafeConnectionPoint::InvokeEvent")
	}
};
