///////////////////////////////////////////////////////////
//  CShcheduler.cpp
//  Implementation of the Class CShcheduler
//  Created on:      11-Dec-2006 3:41:01 PM
///////////////////////////////////////////////////////////
#include "../stdafx.h"
#include "CScheduler.h"
#include <AidLib/Logging/cLog.h>


CScheduler::CScheduler(SPHandle h):m_hStopEvent(h)
{
	m_sphWorkerThread = SPHandle( (HANDLE)_beginthreadex( 0 , 0 , &CScheduler::ThreadEntry, this , 0 , 0 ) , CloseHandle);
	InitializeCriticalSection( &m_cs );
}



CScheduler::~CScheduler()
{
	DeleteCriticalSection( &m_cs );
}

unsigned CScheduler::ThreadEntry( void* ptr)
{
	CScheduler* _this = reinterpret_cast<CScheduler*>( ptr );
	return _this->Dispatch();
}

unsigned CScheduler::Dispatch()
{
	try
	{
		while( true )
		{
			if( !m_activeQueue.empty() )
			{
				TMethod job;
				{
					//TODO add critical section
					CCritSection lock( &m_cs );
					job = m_activeQueue.front();
					m_activeQueue.pop();
				}
				job->callMethod();
			}
			if( WaitForSingleObject( m_hStopEvent.get() , 2 ) == WAIT_OBJECT_0 )
				return 0;
		}
	}
	catch( CStreamException& ex )
	{
		Log.Add(_MESSAGE_,ex.What().c_str())	;	
	}
	return 0;
}

void CScheduler::AddMethod(TMethod method)
{
	//TODO add critical section
	CCritSection lock( &m_cs );
	m_activeQueue.push( method );
}
