/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SServerMessage.h
///
///  Declares SServerMessage structure, responsible for netlog server message
///
///  @author Dmitry Netrebenko @date 21.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>
#include "EServerMessageCode.h"


#define LOG_SERVER_MESSAGE_HEADER_SIZE (sizeof(SServerMessage) - 1)

#pragma pack(push)
#pragma pack(1)
///  SServerMessage structure, responsible for netlog server message
struct SServerMessage
{
	unsigned int		m_size;
	EServerMessageCode	m_code;
	char				m_data[1];
};
#pragma pack(pop)

///  Shared pointer to SServerMessage strucure
typedef boost::shared_ptr<SServerMessage> SPServerMessage;
