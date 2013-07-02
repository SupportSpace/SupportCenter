/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBSimplePersistentStorageFile.h
///
///  CBSimplePersistentStorageFile simple persistent storage via file class declaration 
///
///  @author Kirill Solovyov @date 13.05.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include <AidLib/Strings/tstring.h>
#include <AidLib/CException/CException.h>


class CBSimplePersistentStorageFile
{
protected:
	/// path to storage
	tstring m_path;
public:
	CBSimplePersistentStorageFile(const tstring& path);
	virtual ~CBSimplePersistentStorageFile(void);
	/// The method load value from storage
	/// @param name name of needed property
	/// @return value of the property
	tstring Load(const tstring& name);

	/// The method save value to storage
	/// @param name name of saved property
	/// @param value value of saved property
	void Save(const tstring& name, const tstring& value);

};
