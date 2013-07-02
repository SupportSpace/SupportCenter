/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SLogMsg.h
///
///  Declares SLogMsg structure, responsible for netlog message
///
///  @author Dmitry Netrebenko @date 21.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>
#include <AidLib/Logging/cLog.h>

#define NETLOG_MESSAGE_HEADER_SIZE (sizeof(SLogMsg) - 1)

#pragma pack(push)
#pragma pack(1)
///  SLogMsg structure, responsible for netlog message
struct SLogMsg
{
	int					m_size;
	eVerbosity			m_verbosity;
	cLog::eSeverity		m_severity;
	char				m_data[1];
};
#pragma pack(pop)

///  Shared pointer to SLogMsg strucure
typedef boost::shared_ptr<SLogMsg> SPLogMsg;
