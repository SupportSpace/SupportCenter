//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileFilter.h
///
///  Declares and implements CFileFilter class
///  Filter files with ones of specified attributes
///  
///  @author Alexander Novak @date 20.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAttributesFilter.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileFilter
	:	public CAttributesFilter
{
	unsigned int m_driveType;
public:
	/// Creates CFileFilter object
	/// @param driveType			Allowed drive type
	/// @param attributes			Allowed file atributes
	CFileFilter(const unsigned int driveType, const unsigned int attributes)
		:	CAttributesFilter(attributes),
			m_driveType(driveType){};

	/// Sets new attributes
	/// @param driveType			Allowed drive type
	/// @param attributes			Allowed file atributes
	void SetAttibutes(const unsigned int driveType, const unsigned int attributes);

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

inline void CFileFilter::SetAttibutes(const unsigned int driveType, const unsigned int attributes)
{
	m_driveType		= driveType;
	m_attributes	= attributes;
}
//--------------------------------------------------------------------------------------------------------

inline bool CFileFilter::IsValid(const CFileData& fileData)
{
	if ( fileData.IsDrive() )
		return !(m_driveType == fileData.GetAttributes());

	return !(m_attributes & fileData.GetAttributes());
}
//--------------------------------------------------------------------------------------------------------

inline bool CFileFilter::IsValid(const CFileData* fileData)
{
	return IsValid(*fileData);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
