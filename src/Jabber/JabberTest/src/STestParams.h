/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  STestParams.h
///
///  Declares STestParams structure, responsible for set of test
///    parameters
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "EClientType.h"

#define DEFAULT_MSG_COUNT		50
#define DEFAULT_SEND_DELAY		100
#define DEFAULT_WAIT_TIMEOUT	10000

///  STestParams structure, responsible for set of test
///    parameters
struct STestParams
{
	bool			m_server;		/// server side
	EClientType		m_clientType;	/// type of client connection
	unsigned int	m_msgCount;		/// count of messages
	unsigned int	m_sendDelay;	/// delay at sending messages
	unsigned int	m_waitTimeout;	/// timeout to wait answer in msecs
	unsigned int	m_bulkSize;		/// count of messages to send

/// Default constructor
	STestParams()
		:	m_server(false)
		,	m_clientType(ctGloox)
		,	m_msgCount(DEFAULT_MSG_COUNT)
		,	m_sendDelay(DEFAULT_SEND_DELAY)
		,	m_waitTimeout(DEFAULT_WAIT_TIMEOUT)
		,	m_bulkSize(1)
	{};
};
