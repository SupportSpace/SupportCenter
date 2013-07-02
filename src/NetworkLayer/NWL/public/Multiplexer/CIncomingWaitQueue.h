//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CIncomingWaitQueue.h
///
///  Declares and implements a thread-safe queue
///  Queue does the block operation on the pop/top method if no data exists
///  
///  @author Alexander Novak @date 27.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/CDatagramQueue.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename ElmQueue> class CIncomingWaitQueue
	:	CDatagramQueue<ElmQueue>
{
	bool m_waitForPop;
	bool m_waitDropped;
	CCritSectionSimpleObject m_csDataGuard;
	CCritSectionSimpleObject m_csOncePopEnter;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_eventWait;

	CIncomingWaitQueue(const CIncomingWaitQueue&);
	CIncomingWaitQueue& operator=(const CIncomingWaitQueue&);
public:

	/// Create queue with specified item count
	/// @param countItem	Count of items in queue
	/// @remarks	Will throw an exception, if free memory not available
	CIncomingWaitQueue(unsigned int itemCount);

	virtual ~CIncomingWaitQueue();

	/// Push item into queue
	/// @param data			Const pointer to data
	/// @return		[true] - if item was pushed, [false] if queue is full
	bool Push(const ElmQueue* data);

	/// Gets pointer to an item on top of the queue
	/// @return		[pointer to an item] - if data exists or [NULL] if DropIncomingWaiting method was called
	/// @remarks	Will lock, if queue is empty
	const ElmQueue* Top();

	/// Remove item out of queue
	/// @param data			Pointer to an item for storing
	/// @return		[true] - if item was removed, [false] if DropIncomingWaiting method was called
	/// @remarks	If input parameter is NULL then item just removes
	/// @remarks	Will lock, if queue is empty
	bool Pop(ElmQueue* data = NULL);

	/// Clear the queue
	void Clear();

	/// Check the queue if it empty
	/// @return		[true] - if queue is empty, [false] in otherwise
	bool IsEmpty();

	/// Cancel wait operation
	/// @remarks	If the pop/top method is waiting it will return [false/NULL]
	void DropIncomingWaiting();

	/// Returns true if wait was dropped
	inline bool WaitDropped()
	{
		return m_waitDropped;
	}
};
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> CIncomingWaitQueue<ElmQueue>::CIncomingWaitQueue(unsigned int itemCount)
	:	m_waitForPop(false),
		m_waitDropped(false),
		CDatagramQueue<ElmQueue>(itemCount)
{
TRY_CATCH

	m_eventWait.reset(CreateEvent(NULL,FALSE,FALSE,NULL), CloseHandle);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> CIncomingWaitQueue<ElmQueue>::~CIncomingWaitQueue()
{
TRY_CATCH
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool CIncomingWaitQueue<ElmQueue>::Push(const ElmQueue* data)
{
TRY_CATCH
	bool result;

	CCritSection dataGuard(&m_csDataGuard);

	if ( m_waitForPop )
	{
		m_waitForPop = false;
		SetEvent(m_eventWait.get());
	}
	result = CDatagramQueue::Push(data);

	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> const ElmQueue* CIncomingWaitQueue<ElmQueue>::Top()
{
TRY_CATCH

	CCritSection oncePopEnter(&m_csOncePopEnter);
	CCritSection dataGuard(&m_csDataGuard);

	const ElmQueue* result = CDatagramQueue::Top();

	while ( !result )
	{
		m_waitForPop = true;

		dataGuard.Unlock();

		if ( WaitForSingleObject(m_eventWait.get(),INFINITE)!=WAIT_OBJECT_0 )
			throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());

		dataGuard.Lock();

		if ( !m_waitDropped )
			result = CDatagramQueue::Top();
		else
		{
			m_waitDropped = false;
			break;
		}
	}				// If result is false then Clear occured during wait operation. Wait for another data.
	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool CIncomingWaitQueue<ElmQueue>::Pop(ElmQueue* data)
{
TRY_CATCH

	CCritSection oncePopEnter(&m_csOncePopEnter);
	CCritSection dataGuard(&m_csDataGuard);

	bool result = CDatagramQueue::Pop(data);

	while ( !result )
	{
		m_waitForPop = true;

		dataGuard.Unlock();

		if ( WaitForSingleObject(m_eventWait.get(),INFINITE)!=WAIT_OBJECT_0 )
			throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());

		dataGuard.Lock();

		if ( !m_waitDropped )
			result = CDatagramQueue::Pop(data);
		else
		{
			result = m_waitDropped = false;
			break;
		}
	}				// If result is false then Clear occured during wait operation. Wait for another data.
	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> void CIncomingWaitQueue<ElmQueue>::Clear()
{
TRY_CATCH

	CCritSection dataGuard(&m_csDataGuard);

	CDatagramQueue::Clear();

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool CIncomingWaitQueue<ElmQueue>::IsEmpty()
{
TRY_CATCH

	bool result;

	CCritSection dataGuard(&m_csDataGuard);

	result=CDatagramQueue::IsEmpty();

	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> void CIncomingWaitQueue<ElmQueue>::DropIncomingWaiting()
{
TRY_CATCH

	CCritSection dataGuard(&m_csDataGuard);

	m_waitDropped = true;
	if ( m_waitForPop )
	{
		m_waitForPop = false;
		SetEvent(m_eventWait.get());
	}

CATCH_THROW();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
