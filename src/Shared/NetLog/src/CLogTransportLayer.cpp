/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogTransportLayer.cpp
///
///  Implements CLogTransportLayer class, base class for logging transport layer
///
///  @author Dmitry Netrebenko @date 22.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <NetLog/CLogTransportLayer.h>
#include <NetLog/CNetworkLog.h>

CLogTransportLayer::CLogTransportLayer(const tstring& name, CNetworkLog& netLog)
	:	m_name(name)
	,	m_netLog(netLog)
{

}

CLogTransportLayer::~CLogTransportLayer()
{

}
