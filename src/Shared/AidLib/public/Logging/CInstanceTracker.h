/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInstanceTracker.h
///
///  simple class to log objects creation deletion
///
///  @author "Archer Software" Sogin M. @date 08.08.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <AidLib/Logging/cLog.h>

/// Simple class to log objects creation deletion
class CInstanceTracker
{
private:
	/// Name of object
	tstring m_objectName;
public:
	/// ctor
	/// Since we don't use RTTI to reduce binaries size
	/// objectName is required parameter
	CInstanceTracker(const tstring &objectName)
		: m_objectName(objectName)
	{
		Log.Add(_MESSAGE_,_T("Object %s this(%X) created"),objectName.c_str(),this);
	}

	/// dtor
	~CInstanceTracker()
	{
		Log.Add(_MESSAGE_,_T("Object %s this(%X) deleted"),m_objectName.c_str(),this);
	}
};
