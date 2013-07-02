/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  cLogLight.h
///
///  cLogLight, light cLog class
///
///  @author "Archer Software" Kirill Solovyov. @date 21.05.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <AidLib/Strings/tstring.h>
/// light cLog class
class cLogLight
{
public:	
	/// The method Adds formated record
	/// @param error win32 error code
	/// @param format string like in printf
	void Add(const DWORD error,const TCHAR*const format, ...) throw();
protected:
	/// The method output logging message
	/// @param tstring logging message
	virtual void AddList(const tstring)throw()=0;
};
