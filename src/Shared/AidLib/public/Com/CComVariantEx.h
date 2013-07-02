/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CComVariantEx.h
///
///  Implements CComVariantEx class, CComVariant with supporting of SAFEARRAYs
///    serialization
///
///  @author Dmitry Netrebenko @date 14.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <boost/scoped_array.hpp>
#include <AidLib/CException/CException.h>
#include <AidLib/AidLib.h>

///  CComVariantEx class, CComVariant with supporting of SAFEARRAYs serialization
///  Base class - CComVariant
class AIDLIB_API CComVariantEx
	:	public CComVariant
{
public:
/// Default constructor
	CComVariantEx()
		:	CComVariant()
	{};
/// Constructor
/// @param varSrc - variant
	CComVariantEx(const VARIANT& varSrc)
		:	CComVariant(varSrc)
	{};
/// Constructor
/// @param varSrc - safe array
	CComVariantEx(const SAFEARRAY *pSrc)
		:	CComVariant(pSrc)
	{};
/// Writes variant to stream
/// @param stream - pointer to IStream
/// @return HRESULT
	HRESULT WriteToStream(IStream* stream);
/// Extracts variant from stream
/// @param stream - pointer to IStream
/// @return HRESULT
	HRESULT ReadFromStream(IStream* stream);
private:
/// Writes SAFEARRAY to stream
/// @param arr - pointer to SAFEARRAY
/// @param stream - pointer to IStream
/// @return HRESULT
	HRESULT WriteArrayToStream(SAFEARRAY* arr, IStream* stream);
/// Extracts SAFEARRAY from stream
/// @param stream - pointer to IStream
/// @return HRESULT
	HRESULT ReadArrayFromStream(IStream* stream);
};

inline HRESULT CComVariantEx::WriteToStream(IStream* stream)
{
TRY_CATCH
	/// Check up pointer ti IStream
	if(!stream)
		return E_POINTER;
	HRESULT hr(E_FAIL);
	if((VT_NULL == vt) || (VT_EMPTY == vt))
	{
		/// Wite type of variant only
		hr = stream->Write(&vt, sizeof(VARTYPE), NULL);
	}
	else
	{
		if((vt & VT_TYPEMASK) == vt)
		{
			/// Write simple type variants to stream
			hr = CComVariant::WriteToStream(stream);
		}
		else 
		{
			if((VT_ARRAY | VT_VARIANT) == vt)
			{
				/// Write SAFEARRAY to stream
				hr = WriteArrayToStream(parray, stream);
			}
		}
	}

	return hr;
CATCH_THROW()
};

inline HRESULT CComVariantEx::ReadFromStream(IStream* stream)
{
TRY_CATCH
	/// Check up pointer ti IStream
	if(!stream)
		return E_POINTER;
	HRESULT hr(E_FAIL);

	/// Clear self
	VariantClear(this);

	VARTYPE vtRead(VT_NULL);
	DWORD read(0);

	/// Read variant type from stream
	hr = stream->Read(&vtRead, sizeof(VARTYPE), &read);
	if((S_OK != hr) || (0 == read))
		return E_FAIL;

	if((VT_NULL == vtRead) || (VT_EMPTY == vtRead))
	{
		/// Redaing completed
		vt = vtRead;
	}
	else
	{
		if((vtRead & VT_TYPEMASK) == vtRead)
		{
			/// Jump back
			LARGE_INTEGER move;
			move.QuadPart = -(LONGLONG)sizeof(VARTYPE);
			hr = stream->Seek(move, STREAM_SEEK_CUR, NULL);
			if(S_OK != hr)
				return hr;
			/// Read simple types variant from stream
			hr = CComVariant::ReadFromStream(stream);
		}
		else
		{
			if((VT_ARRAY | VT_VARIANT) == vtRead)
			{
				/// Read SAFEARRAY from stream
				vt = vtRead;
				hr = ReadArrayFromStream(stream);
			}
		}
	}

	return hr;
CATCH_THROW()
};

inline HRESULT CComVariantEx::WriteArrayToStream(SAFEARRAY* arr, IStream* stream)
{
TRY_CATCH
	/// Check up pointers
	if(!arr || !stream)
		return E_POINTER;

	HRESULT hr(E_FAIL);

	/// Write type of variant to stream
	hr = stream->Write(&vt, sizeof(VARTYPE), NULL);
	if(S_OK != hr)
		return hr;

	/// Get dimensions count and write to stream
	unsigned short dimension = arr->cDims;
	hr = stream->Write(&dimension, sizeof(dimension), NULL);
	if(S_OK != hr)
		return hr;

	/// Get pointer to bounds array and write to stream
	SAFEARRAYBOUND* bounds = arr->rgsabound;
	hr = stream->Write(bounds, sizeof(SAFEARRAYBOUND) * dimension, NULL);
	if(S_OK != hr)
		return hr;

	/// Create array of indexes (for all dimensions) and fill by LBound
	boost::scoped_array<LONG> indexes(new LONG[dimension]);
	for(int i = 0; i < dimension; ++i)
		indexes[i] = arr->rgsabound[i].lLbound;

	/// Get pointer to current index and current bounds
	LONG* curIndex = &indexes[dimension - 1];
	SAFEARRAYBOUND* curBound = &arr->rgsabound[0];

	while(true)
	{
		/// Extract variant from array
		CComVariantEx comVar;
		hr = SafeArrayGetElement(arr, indexes.get(), &comVar);
		if(S_OK != hr)
			return hr;
		
		/// Write variant to stream
		hr = comVar.WriteToStream(stream);
		if(S_OK != hr)
			return hr;

		/// Calculate next set of indexes
		while(true)
		{
			(*curIndex)++;
			if(*curIndex >= (curBound->lLbound + (LONG)curBound->cElements))
			{
				*curIndex = curBound->lLbound;
				if(curIndex == &indexes[0])
					return S_OK;
				curIndex--;
				curBound++;
			}
			else
			{
				curIndex = &indexes[dimension - 1];
				curBound = &arr->rgsabound[0];
				break;
			}
		}
	}

	return S_OK;
CATCH_THROW()
};

inline HRESULT CComVariantEx::ReadArrayFromStream(IStream* stream)
{
TRY_CATCH
	/// Check up pointer
	if(!stream)
		return E_POINTER;

	HRESULT hr(E_FAIL);

	/// Read array's dimension from stream
	unsigned short dimension = 0;
	hr = stream->Read(&dimension, sizeof(dimension), NULL);
	if(S_OK != hr)
		return hr;
	if(0 >= dimension)
		return E_FAIL;

	/// Allocate memory for bounds and read from stream
	DWORD read(0);
	boost::scoped_array<SAFEARRAYBOUND> tmp(new SAFEARRAYBOUND[dimension]);
	hr = stream->Read(tmp.get(), sizeof(SAFEARRAYBOUND) * dimension, &read);
	if((S_OK != hr) || (read != sizeof(SAFEARRAYBOUND) * dimension))
		return E_FAIL;

	/// Allocate memory and create reverted array with bounds
	boost::scoped_array<SAFEARRAYBOUND> bounds(new SAFEARRAYBOUND[dimension]);
	for(int i = 0; i < dimension; ++i)
		bounds[i] = tmp[dimension - i - 1];

	/// Create SAFEARRAY
	parray = SafeArrayCreate(VT_VARIANT, dimension, bounds.get());
	if(!parray)
		return E_FAIL;

	/// Create array of indexes (for all dimensions) and fill by LBound
	boost::scoped_array<LONG> indexes(new LONG[dimension]);
	for(int i = 0; i < dimension; ++i)
		indexes[i] = bounds[i].lLbound;

	/// Get pointer to current index and current bounds
	LONG* curIndex = &indexes[dimension - 1];
	SAFEARRAYBOUND* curBound = &parray->rgsabound[0];

	while(true)
	{
		/// Extract variant from stream
		CComVariantEx comVar;
		hr = comVar.ReadFromStream(stream);
		if(S_OK != hr)
			return hr;

		/// Add element into new array
		hr = SafeArrayPutElement(parray, indexes.get(), &comVar);
		if(S_OK != hr)
			return hr;

		/// Calculate next set of indexes
		while(true)
		{
			(*curIndex)++;
			if(*curIndex >= (curBound->lLbound + (LONG)curBound->cElements))
			{
				*curIndex = curBound->lLbound;
				if(curIndex == &indexes[0])
					return S_OK;
				curIndex--;
				curBound++;
			}
			else
			{
				curIndex = &indexes[dimension - 1];
				curBound = &parray->rgsabound[0];
				break;
			}
		}
	}
	return S_OK;
CATCH_THROW()
};

