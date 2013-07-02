/////////////////////////////////////////////////////////////////////////
///
///  CScheduler.h
///
///  This is a scheduler class which consists Active Queue
///
///
///  @author Dmiry S. Golub @date 2/6/2007
///
////////////////////////////////////////////////////////////////////////
#if !defined(EA_8CA084DE_769C_44ea_B502_4511367CB1E1__INCLUDED_)
#define EA_8CA084DE_769C_44ea_B502_4511367CB1E1__INCLUDED_

#include "IMethodRequest.h"
#include <FTUI/FileTransfer/filetransferdata.h>
#include <process.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <windows.h>
#include <queue>
#include <AidLib/CCritSection/CCritSection.h>

/// This is a scheduler class which consists Active Queue
class CScheduler
{
public:
	/// type of the methods which are put into Active Queue
	typedef boost::shared_ptr<IMethodRequest> TMethod;
	/// .ctor	
	CScheduler(SPHandle);
	/// .dtor
	virtual ~CScheduler();
private:
	/// thread entry function
	static unsigned __stdcall ThreadEntry( void* );
	/// dispatches messages
	unsigned Dispatch();
private:
	/// event which stops messages dispatcing
	SPHandle m_sphWorkerThread;
	/// active queue 
	std::queue<TMethod> m_activeQueue;
	/// critical section
	CRITICAL_SECTION	m_cs;

	SPHandle	m_hStopEvent;
public:
	/// puts method to active queue
	void AddMethod(TMethod method);
};
#endif // !defined(EA_8CA084DE_769C_44ea_B502_4511367CB1E1__INCLUDED_)
