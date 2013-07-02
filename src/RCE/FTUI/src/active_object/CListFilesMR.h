/////////////////////////////////////////////////////////////////////////
///
///  clistfilesmr.h
///
///  Part of the active object realization
///
///  @author Dmiry S. Golub @date 12/12/2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "IMethodRequest.h"


///  This is a method request for list files in remote machine
///  Base class IMethodRequest define main interface for all method request classes
class CListFilesMR : public IMethodRequest
{
public:
	typedef boost::function<void(TFileInfo&)> TCallback;
	/// .ctor
	CListFilesMR(CFileAccessClient*,TCallback,const s_param& par);
	/// .dtor
	~CListFilesMR();
	/// virtual method which makes all work
	void callMethod();
private:
	/// data for callMethod
	s_param   m_param;
	/// callback method which will be called after finishing callMethod
	TCallback m_futureResult;
};

