/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNATTraversingStreamTest.cpp
///
///  Implements CNATTraversingStreamTest class, test for CNATTraversingUDPNetworkStream connection
///
///  @author Dmitry Netrebenko @date 29.02.2008
///
////////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include "CNATTraversingStreamTest.h"
#include <AidLib/CException/CException.h>
#include <NWL/Streaming/CNATTraversingUDPNetworkStream.h>
#include <AidLib/Strings/tstring.h>

CNATTraversingStreamTest::CNATTraversingStreamTest(boost::shared_ptr<CSettings> settings)
	:	CStreamTest(settings)
{
TRY_CATCH
CATCH_THROW()
}

CNATTraversingStreamTest::~CNATTraversingStreamTest()
{
TRY_CATCH
CATCH_LOG()
}

boost::shared_ptr<CAbstractNetworkStream> CNATTraversingStreamTest::CreateStream(boost::shared_ptr<SThreadInfo> threadInfo)
{
TRY_CATCH
	CNATTraversingUDPNetworkStream* stream = new CNATTraversingUDPNetworkStream();
	tstring userId = Format(_T("%s%d"), m_settings->GetUser().c_str(), threadInfo->m_threadId);
	stream->SetIsMaster(threadInfo->m_master);
	stream->SetRelayServer( 
		m_settings->GetRelayHost(), 
		m_settings->GetRelayPort(), 
		userId, 
		m_settings->GetPassword());
	stream->SetConnectionId(threadInfo->m_connectId, threadInfo->m_localPeer, threadInfo->m_remotePeer);
	stream->GetCredentials().Key = _T("TestUser");
	stream->GetCredentials().UserID = _T("TestUser");
	stream->SetConnectTimeout(30000); 
	return boost::shared_ptr<CAbstractNetworkStream>(stream);
CATCH_THROW()
}
