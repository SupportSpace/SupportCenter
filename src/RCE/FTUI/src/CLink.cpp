//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CLink.cpp
///
///  Implements CLink class
///  Performs operations with files path
///  
///  @author Alexander Novak @date 20.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CLink.h"
#include <AidLib/CException/CException.h>

// CLink [BEGIN] /////////////////////////////////////////////////////////////////////////////////////////

CLink::CLink(const tstring& path, bool local)
	:	m_local(local)
{
TRY_CATCH

	#ifdef CLINK_USE_UNICODE
		if (path.find(_T("\\\\?\\")) == tstring::npos)
			m_pathRef += _T("\\\\?\\");					//Add the UNC prefix
	#endif

	m_pathRef+=path;

	// Remove the termination slash
	if ( path.size() && *m_pathRef.rbegin() == _T('\\') )
		m_pathRef.erase( m_pathRef.size()-1 );

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CLink::CLink(const CLink& link)
{
TRY_CATCH

	m_local		= link.m_local;
	m_pathRef	= link.m_pathRef;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CLink& CLink::operator= (const CLink& link)
{
TRY_CATCH

	if ( this != &link )
	{
		m_local		= link.m_local;
		m_pathRef	= link.m_pathRef;
	}
	return *this;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CLink& CLink::operator+= (const tstring& path)
{
TRY_CATCH

	if ( !path.size() )
		return *this;

	if ( !IsEmpty() )
		m_pathRef += _T("\\");

	m_pathRef += path;

	// Remove the termination slash
	if ( m_pathRef.size() && *m_pathRef.rbegin() == _T('\\') )
		m_pathRef.erase( m_pathRef.size()-1 );
		
	return *this;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

TCHAR CLink::GetDriveLetter() const
{
TRY_CATCH

	#ifdef CLINK_USE_UNICODE					//[0][1][2][3][4][5]
		if ( m_pathRef.size() >= 6 )			// \  \  ?  \  X  :
			return m_pathRef[4];				// UNC prefix  Drive letter
	#else										//[0][1][2][3][4][5]
		if ( m_pathRef.size() >= 2 )			// X  :
			return m_pathRef[0];				// Drive letter without UNC prefix
	#endif

	throw MCException(_T("Invalid path reference"));
	
	return _T('\0');

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

tstring CLink::GetTargetName() const
{
TRY_CATCH

	tstring::size_type indxPos = m_pathRef.rfind(_T('\\'));
	
	#ifdef CLINK_USE_UNICODE
		if ( m_pathRef.size() == 4 && m_pathRef[indxPos-1]==_T('?') )
			return _T("");
	#else
		if ( indxPos == tstring::npos )
			return m_pathRef;
	#endif

	return m_pathRef.substr(indxPos+1);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CLink::CutTargetName()
{
TRY_CATCH

	if (IsEmpty())
		return;

	tstring::size_type indxPos = m_pathRef.rfind(_T('\\'));
	
	#ifdef CLINK_USE_UNICODE
		if ( m_pathRef[indxPos-1]==_T('?') )
			m_pathRef.erase(indxPos+1);
		else
			m_pathRef.erase(indxPos);
	#else
		if ( indxPos == tstring::npos )
			m_pathRef.clear();
		else
			m_pathRef.erase(indxPos);
	#endif

CATCH_THROW()
}
// CLink [END] ///////////////////////////////////////////////////////////////////////////////////////////
