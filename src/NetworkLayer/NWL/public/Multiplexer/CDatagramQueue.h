//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CDatagramQueue.h
///
///  Declares and implements a thread-unsafe queue
///  
///  @author Alexander Novak @date 25.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <NWL/Streaming/CStreamException.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename ElmQueue> class CDatagramQueue
{
	unsigned int	m_head;
	unsigned int	m_tail;
	unsigned int	m_size_queue;
	ElmQueue*		m_queue;

	CDatagramQueue(const CDatagramQueue&);
	CDatagramQueue& operator=(const CDatagramQueue&);
public:

	/// Create queue with specified item count
	/// @param countItem Count of items in queue
	/// @remarks Will throw an exception, if free memory not available
	CDatagramQueue(unsigned int countItem);

	virtual ~CDatagramQueue();

	/// Push item into queue
	/// @param data		Const pointer to data
	/// @return		[true] - if item was pushed, [false] in otherwise
	bool Push(const ElmQueue* data);

	/// Gets pointer to an item on top of the queue
	/// @return		[pointer to an item] - if data exists or [NULL] in otherwise
	const ElmQueue* Top();

	/// Remove item out of queue
	/// @param data		Pointer to an item for storing
	/// @return		[true] - if item was removed, [false] in otherwise
	/// @remarks	If input parameter is NULL then item just removes
	bool Pop(ElmQueue* data = NULL);

	/// Clear the queue
	void Clear();

	/// Check the queue if it empty
	/// @return		[true] - if queue is empty, [false] in otherwise
	bool IsEmpty() const;
};
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> CDatagramQueue<ElmQueue>::CDatagramQueue(unsigned int countItem)
	:	m_head(0),
		m_tail(0),
		m_size_queue(countItem)
{
TRY_CATCH

	m_queue=static_cast<ElmQueue*>( VirtualAlloc(	NULL,
													countItem*sizeof(ElmQueue),
													MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,PAGE_READWRITE) );
	if ( !m_queue )
		throw MCStreamException_Win(_T("Can't alloc memory for a queue"),GetLastError());

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> CDatagramQueue<ElmQueue>::~CDatagramQueue()
{
TRY_CATCH

	if ( !VirtualFree(m_queue,0,MEM_RELEASE) )
		throw MCStreamException_Win(_T("Can't free memory for a queue"),GetLastError());

CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool CDatagramQueue<ElmQueue>::Push(const ElmQueue* data)
{
	unsigned int next_tail = (m_tail+1)%m_size_queue;

	if ( next_tail == m_head )
		return false;					// Queue is full

	m_queue[m_tail] = *data;
	m_tail = next_tail;

	return true;						// Item was added
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> inline const ElmQueue* CDatagramQueue<ElmQueue>::Top()
{
	return ( IsEmpty() ) ? NULL : &m_queue[m_head];
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> bool CDatagramQueue<ElmQueue>::Pop(ElmQueue* data)
{
	if ( IsEmpty() )
		return false;

	if ( data )
		*data = m_queue[m_head];

	m_head = (m_head+1)%m_size_queue;

	return true;
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> inline void CDatagramQueue<ElmQueue>::Clear()
{
	m_head = m_tail;
}
//--------------------------------------------------------------------------------------------------------

template<typename ElmQueue> inline bool CDatagramQueue<ElmQueue>::IsEmpty() const
{
	return ( m_head == m_tail );
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
