/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SThreadInfo.h
///
///  Declares SThreadInfo structure, responsible for staring thread information
///
///  @author Dmitry Netrebenko @date 27.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>

/// SThreadInfo structure, responsible for staring thread information
struct SThreadInfo
{
	int		m_threadId;				/// Id of thread
	tstring	m_connectId;			/// Connection Id
	tstring	m_localPeer;			/// Local peer name
	tstring	m_remotePeer;			/// Remote peer name
	bool	m_master;				/// Is master
/// Default constructor
	SThreadInfo()
		:	m_master(false)
		,	m_threadId(0)
		,	m_connectId(_T(""))
		,	m_localPeer(_T(""))
		,	m_remotePeer(_T(""))
	{};
/// Initial constructor
	SThreadInfo(const bool master, const int& threadId, const tstring& connectId, const tstring& localPeer, const tstring& remotePeer)
		:	m_master(master)
		,	m_threadId(threadId)
		,	m_connectId(connectId)
		,	m_localPeer(localPeer)
		,	m_remotePeer(remotePeer)
	{};

};

