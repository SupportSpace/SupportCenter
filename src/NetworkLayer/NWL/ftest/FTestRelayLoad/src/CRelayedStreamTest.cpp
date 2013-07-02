/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayedStreamTest.cpp
///
///  Implements CRelayedStreamTest class, test for CRelayedNetworkStream connection
///
///  @author Dmitry Netrebenko @date 27.02.2008
///
////////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include "CRelayedStreamTest.h"
#include <AidLib/CException/CException.h>
#include <NWL/Streaming/CRelayedNetworkStream.h>
#include <AidLib/Strings/tstring.h>

CRelayedStreamTest::CRelayedStreamTest(boost::shared_ptr<CSettings> settings)
	:	CStreamTest(settings)
{
TRY_CATCH
CATCH_THROW()
}

CRelayedStreamTest::~CRelayedStreamTest()
{
TRY_CATCH
CATCH_LOG()
}

boost::shared_ptr<CAbstractNetworkStream> CRelayedStreamTest::CreateStream(boost::shared_ptr<SThreadInfo> threadInfo)
{
TRY_CATCH
	CRelayedNetworkStream<>* stream = new CRelayedNetworkStream<>();
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
