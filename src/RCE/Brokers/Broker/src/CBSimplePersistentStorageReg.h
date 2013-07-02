/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBSimplePersistentStorageReg.h
///
///  CBSimplePersistentStorageReg simple persistent storage via registry class declaration 
///
///  @author Kirill Solovyov @date 13.05.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <AidLib/CException/CException.h>
#include <atlbase.h>
/// Simple persistent storage via registry
class CBSimplePersistentStorageReg
{
protected:
	/// persistent storage key
	CRegKey m_key;
public:
	CBSimplePersistentStorageReg(HKEY keyParent, const tstring& keyName);
	virtual ~CBSimplePersistentStorageReg(void);
	
	/// The method load value from storage
	/// @param name name of needed property
	/// @return value of the property
	tstring Load(const tstring& name);

	/// The method save value to storage
	/// @param name name of saved property
	/// @param value value of saved property
	void Save(const tstring& name, const tstring& value);
};
