/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CVariantSerializer.h
///
///  Declares CVariantSerializer class, responsible for serialization of variants
///
///  @author Dmitry Netrebenko @date 14.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <AidLib/Com/CComVariantEx.h>
#include <AidLib/AidLib.h>

///  CVariantSerializer class, responsible for serialization of variants
class AIDLIB_API CVariantSerializer
{
private:
/// Prevents making copies of CVariantSerializer objects
	CVariantSerializer(const CVariantSerializer&);
	CVariantSerializer& operator=(const CVariantSerializer&);
/// Prevents making instances
	CVariantSerializer();
public:
/// Defines shared pointer type for CComVariantEx
	typedef boost::shared_ptr<CComVariantEx> SPVariant;
/// Defines shared array type for buffer of chars
	typedef boost::shared_array<char> SPCharBuf;

/// Converts variant to buffer of chars
/// @param var - [in] shared pointer to CComVariantEx
/// @param size - [out] pointer to size of created buffer
/// @return buffer with variant's data
	static SPCharBuf VariantToBuf(SPVariant var, unsigned int* size);

/// Converts buffer of chars to variant
/// @param buf - [in] buffer
/// @param size - [in] buffer size in bytes
/// @return shared pointer to CComVariantEx object
	static SPVariant BufToVariant(SPCharBuf buf, const unsigned int size);
};
