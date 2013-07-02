//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CAttributesFilter.h
///
///  Declares and implements CAttributesFilter class
///  Performs file filtering operations. Does no filtering operation.
///  
///  @author Alexander Novak @date 20.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CFileData.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAttributesFilter
{
protected:
	unsigned int m_attributes;
public:
	CAttributesFilter(const unsigned int attributes)
		:	m_attributes(attributes){};
	virtual ~CAttributesFilter(){};

	/// Checks if attributes are valid
	/// @param fileData				File data for checking
	/// @return				Returns true if condition holds, otherwise - false
	virtual bool IsValid(const CFileData& fileData);

	/// Checks if attributes are valid
	/// @param fileData				File data for checking
	/// @return				Returns true if condition holds, otherwise - false
	virtual bool IsValid(const CFileData* fileData);
};
//--------------------------------------------------------------------------------------------------------

inline bool CAttributesFilter::IsValid(const CFileData& fileData)
{
	return true;
}
//--------------------------------------------------------------------------------------------------------

inline bool CAttributesFilter::IsValid(const CFileData* fileData)
{
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
