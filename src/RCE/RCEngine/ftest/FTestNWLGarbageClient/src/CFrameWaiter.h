/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrameWaiter.h
///
///  Declares CFrameWaiter class, responsible for waiting of changing frames 
///
///  @author Dmitry Netrebenko @date 11.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/CThread/CThread.h>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

///  Shared pointer to HANDLE type
typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > SPHandle;
///  Waiter's event
typedef boost::function<void (void*)> WaiterEvent;

///  CFrameWaiter class, responsible for waiting of changing frames 
///  Base class - CThread from AidLib
class CFrameWaiter
	:	public CThread
{
private:
///  Prevents making copies of CFrameWaiter objects.
	CFrameWaiter( const CFrameWaiter& );
	CFrameWaiter& operator=( const CFrameWaiter& );

public:
///  Constructor
///  @param - event handlers
	CFrameWaiter( WaiterEvent sessionEventHandler, WaiterEvent frameEventHandler );
///  Destructor
	~CFrameWaiter();
///  Thread's entry point
///  @param Params - thread's parameters
	virtual void Execute( void* Params );

private:
///  Event for session
	SPHandle		m_sessionEvent;
///  Event for frames
	SPHandle		m_frameEvent;
///  Event handler
	WaiterEvent		m_sessionEventHandler;
///  Event handler
	WaiterEvent		m_frameEventHandler;

public:
///  Returns event object for session
	SPHandle GetSessionEvent() const;
///  Returns event object for frame
	SPHandle GetFrameEvent() const;
};

///  Shared pointer to CFrameWaiter class
typedef boost::shared_ptr<CFrameWaiter> SPFrameWaiter;
