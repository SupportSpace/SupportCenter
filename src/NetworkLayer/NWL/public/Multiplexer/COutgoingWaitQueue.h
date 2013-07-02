//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  COutgoingWaitQueue.h
///
///  Declares and implements a thread-safe queue
///  Queue does the block operation on the push method if it is full
///  
///  @author Alexander Novak @date 26.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/CDatagramQueue.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename ElmQueue> class COutgoingWaitQueue
	:	CDatagramQueue<ElmQueue>
{
	bool m_waitForPush;
	bool m_waitDropped;
	CCritSectionSimpleObject m_csDataGuard;
	CCritSectionSimpleObject m_csOncePushEnter;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_eventWait;

	COutgoingWaitQueue(const COutgoingWaitQueue&);
	COutgoingWaitQueue& operator=(const COutgoingWaitQueue&);
public:

	/// Create queue with specified item count
	/// @param countItem	Count of items in queue
	/// @remarks	Will throw an exception, if free memory not available
	COutgoingWaitQueue(unsigned int itemCount);

	virtual ~COutgoingWaitQueue();

	/// Push item into queue
	/// @param data			Const pointer to data
	/// @return		[true] - if item was pushed, [false] if DropOutgoingWaiting method was called
	/// @remarks	Will lock, if queue is full
	bool Push(const ElmQueue* data);

	/// Gets pointer to an item on top of the queue
	/// @return		[pointer to an item] - if data exists or [NULL] in otherwise
	const ElmQueue* Top();

	/// Remove item out of queue
	/// @param data			Pointer to an item for storing
	/// @return		[true] - if item was removed, [false] if no data in queue
	/// @remarks	If input parameter is NULL then item just removes
	bool Pop(ElmQueue* data = NULL);

	/// Clear the queue
	void Clear();

	/// Check the queue if it empty
	/// @return		[true] - if queue is empty, [false] in otherwise
	bool IsEmpty();

	/// Cancel wait operation
	/// @remarks	If the push method is waiting it will return [false]
	void DropOutgoingWaiting();
};
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> COutgoingWaitQueue<ElmQueue>::COutgoingWaitQueue(unsigned int itemCount)
	:	m_waitForPush(false),
		m_waitDropped(false),
		CDatagramQueue<ElmQueue>(itemCount)
{
TRY_CATCH

	m_eventWait.reset(CreateEvent(NULL,FALSE,FALSE,NULL), CloseHandle);

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> COutgoingWaitQueue<ElmQueue>::~COutgoingWaitQueue()
{
TRY_CATCH
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool COutgoingWaitQueue<ElmQueue>::Push(const ElmQueue* data)
{
TRY_CATCH

	bool result = true;

	CCritSection oncePushEnter(&m_csOncePushEnter);
	CCritSection dataGuard(&m_csDataGuard);

	if ( !CDatagramQueue::Push(data) )
	{
		m_waitForPush = true;

		dataGuard.Unlock();

		if ( WaitForSingleObject(m_eventWait.get(),INFINITE)!=WAIT_OBJECT_0 )
			throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());

		dataGuard.Lock();

		if ( !m_waitDropped )
		{
			if ( !CDatagramQueue::Push(data) )
				// If me got here, other operations don't have a sense
				throw MCStreamException(_T("Queue have done fatal a push operation. The wrong algorithm implementation!"));
		}
		else
			result = m_waitDropped = false;
	}
	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> const ElmQueue* COutgoingWaitQueue<ElmQueue>::Top()
{
TRY_CATCH

	const ElmQueue* result;

	CCritSection dataGuard(&m_csDataGuard);

	result = CDatagramQueue::Top();

	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool COutgoingWaitQueue<ElmQueue>::Pop(ElmQueue* data)
{
TRY_CATCH

	CCritSection dataGuard(&m_csDataGuard);

	bool result;
	if ( result = CDatagramQueue::Pop(data) )
	{
		if ( m_waitForPush )
		{
			m_waitForPush = false;
			SetEvent(m_eventWait.get());
		}
	}
	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> void COutgoingWaitQueue<ElmQueue>::Clear()
{
TRY_CATCH

	CCritSection dataGuard(&m_csDataGuard);

	CDatagramQueue::Clear();

	if ( m_waitForPush )
	{
		m_waitForPush = false;
		SetEvent(m_eventWait.get());
	}

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool COutgoingWaitQueue<ElmQueue>::IsEmpty()
{
TRY_CATCH

	bool result;

	CCritSection dataGuard(&m_csDataGuard);

	result=CDatagramQueue::IsEmpty();

	return result;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> void COutgoingWaitQueue<ElmQueue>::DropOutgoingWaiting()
{
TRY_CATCH

	CCritSection dataGuard(&m_csDataGuard);

	if ( m_waitForPush )
	{
		m_waitForPush = false;
		m_waitDropped = true;
		SetEvent(m_eventWait.get());
	}

CATCH_THROW();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
