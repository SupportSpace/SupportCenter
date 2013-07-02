/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStreamFactoryRelayedImpl.cpp
///
///  Implements CStreamFactoryRelayedImpl class, responsible for steam factory
///    with initial message exchange through relay server
///
///  @author Dmitry Netrebenko @date 25.09.2005
///
////////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <NWL/Streaming/CStreamFactoryRelayedImpl.h>
#include <NWL/Streaming/CStreamException.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <boost/scoped_array.hpp>
#include <AidLib/Localization/CLocale.h>

CStreamFactoryRelayedImpl::CStreamFactoryRelayedImpl()
	:	CStreamFactory()
	,	m_initialStream()
{
TRY_CATCH
CATCH_THROW()
}

CStreamFactoryRelayedImpl::~CStreamFactoryRelayedImpl()
{
TRY_CATCH
CATCH_LOG()
}

void CStreamFactoryRelayedImpl::SendMsg(const tstring& peerId, const tstring& messageData)
{
TRY_CATCH
	if(!m_initialStream.Connected())
		throw MCStreamException(_T("Can not send message in disconnected state"));
	m_initialStream.Send(messageData.c_str(), static_cast<unsigned int>(messageData.length()));
CATCH_THROW()
}

void CStreamFactoryRelayedImpl::HandleMsg(const tstring& peerId, tstring& messageData)
{
TRY_CATCH
	if(!m_initialStream.Connected())
		throw MCStreamException(_T("Can not receive message from disconnected state"));
	messageData = _T("");
	boost::scoped_array<char> buf(new char[MAX_INITIAL_MSG_SIZE]);
	memset(buf.get(), 0, MAX_INITIAL_MSG_SIZE);
	m_initialStream.Receive(buf.get(), 1);
	unsigned int received = 1;
	bool bigMessage = false;
	do
	{
		unsigned int ret = m_initialStream.GetInBuffer(buf.get() + received, MAX_INITIAL_MSG_SIZE - received);
		messageData += buf.get();
		bigMessage = (ret == MAX_INITIAL_MSG_SIZE - received);
		if(bigMessage)
		{
			memset(buf.get(), 0, MAX_INITIAL_MSG_SIZE);
			received = 0;
		}
	} while(bigMessage);
CATCH_THROW()
}

HANDLE CStreamFactoryRelayedImpl::GetHandleMsgEvent()
{
TRY_CATCH
	return NULL;
CATCH_THROW()
}

void CStreamFactoryRelayedImpl::InitExchanger(const tstring& sourcePeerId, const tstring& destPeerId, int timeOut, bool masterRole)
{
TRY_CATCH
	NotifyProgress(25,MLocalize(_T("Initialization of exchanger")));
	Log.Add(_MESSAGE_,_T("Initialization of exchanger"));
	if(m_initialStream.Connected())
		m_initialStream.Disconnect();
	tstring connectId(_T(""));
	if(masterRole)
		connectId = sourcePeerId + destPeerId;
	else
		connectId = destPeerId + sourcePeerId;
	m_initialStream.SetConnectTimeout(timeOut);
	m_initialStream.SetRelayServer( 
		NWL_INSTANCE.GetRelayHost(),
		NWL_INSTANCE.GetRelayTCPPort(),
		m_strServerUserId,
		NWL_INSTANCE.GetRelayPasswd() );
	m_initialStream.SetConnectionId( 
		connectId, 
		sourcePeerId,
		destPeerId );
	m_initialStream.Connect();
	Log.Add(_MESSAGE_,_T("Exchanger is initialized"));
CATCH_THROW()
}

void CStreamFactoryRelayedImpl::CloseExchanger(bool masterRole)
{
TRY_CATCH
	if(!masterRole && m_initialStream.Connected())
	{
		TRY_CATCH
			char buf;
			m_initialStream.Receive(&buf, 1);
		CATCH_LOG()
	}
	if(m_initialStream.Connected())
		m_initialStream.Disconnect();
	Log.Add(_MESSAGE_,_T("Exchanger is closed"));
CATCH_THROW()
}

void CStreamFactoryRelayedImpl::AbortExchanger()
{
TRY_CATCH
	m_initialStream.Close();
	Log.Add(_MESSAGE_,_T("Exchanger is aborted"));
CATCH_THROW()
}
