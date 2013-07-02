//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CLink.h
///
///  Declares CLink class
///  Performs operations with files path
///  If module compiled in UNICODE version, the CLink uses UNC paths
///  
///  @author Alexander Novak @date 20.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
//========================================================================================================

#ifdef UNICODE
	#define		CLINK_USE_UNICODE	// If CLINK_USE_UNICODE is define then CLink uses the UNC file names
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLink
{
	bool m_local;
	tstring m_pathRef;
public:
	/// Creates link based on path
	/// @param path					Path to file or directory
	/// @param local				True - if link is local, otherwise - false
	CLink(const tstring& path, bool local = true);

	/// Creates link based on another link object
	/// @param link					Constructed link object
	CLink(const CLink& link);

	/// Assigns one link to another
	/// @param link					Constructed link object
	/// @return				Newly assigned link object
	CLink& operator= (const CLink& link);

	/// Concatenates path to link object
	/// @param path					Part of path
	/// @return				Reference to link object
	CLink& operator+= (const tstring& path);

	/// Casts link to const TCHAR*
	/// @return				Constant pointer to path-string
	operator const TCHAR*() const;

	/// Casts link to const tstring&
	/// @return				Constant reference to path-string
	operator const tstring&() const;

	/// Checks if link object is local or remote
	/// @return				True if link is local, otherwise - false
	bool IsLocal() const;

	/// Checks if link object is empty
	/// @return				True if link is empty, otherwise - false
	bool IsEmpty() const;

	/// Return length of the link
	/// @return				Length of the link in symbols
	unsigned int GetLength() const;

	/// Return drive letter of the link object
	/// @return				Letter of the path reference
	TCHAR GetDriveLetter() const;

	/// Gets target name of the link
	/// @return				Target name for file or directory
	tstring GetTargetName() const;
	
	/// Remove target name from the link
	void CutTargetName();
};
//--------------------------------------------------------------------------------------------------------

inline CLink::operator const TCHAR*() const
{
	return m_pathRef.c_str();
}
//--------------------------------------------------------------------------------------------------------

inline CLink::operator const tstring&() const
{
	return m_pathRef;
}
//--------------------------------------------------------------------------------------------------------

inline bool CLink::IsLocal() const
{
	return m_local;
}
//--------------------------------------------------------------------------------------------------------

inline bool CLink::IsEmpty() const
{
	#ifdef CLINK_USE_UNICODE				//[0][1][2][3][4][5]
		return m_pathRef.size() <= 4 ; 		// \  \  ?  \  X  :
	#else									// UNC prefix  Drive letter
		return m_pathRef.size() == 0;
	#endif
}
//--------------------------------------------------------------------------------------------------------

inline unsigned int CLink::GetLength() const
{
	return static_cast<unsigned int>(m_pathRef.size());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool operator== (const CLink& link1, const CLink& link2)
{
	return _tcsicoll(link1,link2) == 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
