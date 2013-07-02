/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SUDPListenerMsg.h
///
///  Declares SUDPListenerMsg structure, UDP listener's response
///
///  @author Dmitry Netrebenko @date 21.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>

#define NETLOG_UDP_MESSAGE_HEADER_SIZE (sizeof(SUDPListenerMsg) - 1)

#pragma pack(push)
#pragma pack(1)
///  SUDPListenerMsg structure, UDP listener's response
struct SUDPListenerMsg
{
	unsigned int	m_size;
	unsigned int	m_process;
	unsigned int	m_tcpPort;
	char			m_data[1];
};
#pragma pack(pop)

///  Shared pointer to SLogMsg strucure
typedef boost::shared_ptr<SUDPListenerMsg> SPUDPListenerMsg;
