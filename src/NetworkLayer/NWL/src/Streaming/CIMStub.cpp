#include <NWL/Streaming/CIMStub.h>
#include <AidLib/Logging/cLog.h>
#include <NWL/Streaming/CStreamException.h>
#include <NWL/Streaming/CNetworkLayer.h>
#include <AidLib/CSingleton/CSingleton.h>

CIMStub::CIMStub(const tstring &peerId)
	: m_peerID(peerId)
{
TRY_CATCH
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
CATCH_THROW("CIMStub::CIMStub")
}

CIMStub::~CIMStub(void)
{
TRY_CATCH
	if (m_hEvent) CloseHandle(m_hEvent);
CATCH_LOG("CIMStub::~CIMStub")
}

void CIMStub::SendMsg(const tstring& messageData)
{
TRY_CATCH
	CSocketStream sstream;
	sstream.Connect(NWL_INSTANCE.GetRelayHost(), NWL_INSTANCE.GetIMStubPort(),true);
	std::auto_ptr<sIMessage> msg;
	msg.reset(reinterpret_cast<sIMessage*>(new char[MAX_MESSAGE_SIZE]));
	msg->code = IMT_AddMessage;
	strcpy_s(msg->data,MAX_MESSAGE_SIZE-IMESSAGE_HEADSIZE,messageData.c_str());
	strcpy_s(msg->peerID,MAX_PEERID_SIZE,m_peerID.c_str());
	msg->size = static_cast<unsigned int>(IMESSAGE_HEADSIZE + messageData.length() + 1);
	sstream.Send(reinterpret_cast<char*>(msg.get()),msg->size);
CATCH_THROW("CIMStub::SendMsg")
}

void CIMStub::HandleMsg(tstring& messageData)
{
TRY_CATCH
	for(int i=0;i<20;++i) //try only 20 times
	{
		CSocketStream sstream;
		sstream.Connect(NWL_INSTANCE.GetRelayHost(), NWL_INSTANCE.GetIMStubPort(),true);
		std::auto_ptr<sIMessage> msg;
		msg.reset(reinterpret_cast<sIMessage*>(new char[MAX_MESSAGE_SIZE]));
		msg->code = IMT_GetMessage;
		strcpy_s(msg->peerID,MAX_PEERID_SIZE,m_peerID.c_str());
		msg->size = IMESSAGE_HEADSIZE;
		sstream.Send(reinterpret_cast<char*>(msg.get()),msg->size);
		int n=0;
		char buf[MAX_MESSAGE_SIZE];
		try
		{
			for(;n<MAX_MESSAGE_SIZE;++n)
			{
				sstream.Receive(buf+n,1);
			}
		}
		catch(CStreamException&)
		{
			if (n)
			{
				messageData = buf;
				return;
			}
			//Log.Add(_MESSAGE_,"CIMStub::HandleMsg try(%d): No messages on server for this peer %s",i,m_peerID.c_str());
			Sleep(1000);
		}
	}
	throw MCStreamException("Failed to handle message");
CATCH_THROW("CIMStub::HandleMsg")
}

void CIMStub::ResetServer()
{
TRY_CATCH
	CSocketStream sstream;
	sstream.Connect(NWL_INSTANCE.GetRelayHost(), NWL_INSTANCE.GetIMStubPort(),true);
	std::auto_ptr<sIMessage> msg;
	msg.reset(reinterpret_cast<sIMessage*>(new char[MAX_MESSAGE_SIZE]));
	msg->code = IMT_ClearMessages;
	strcpy_s(msg->peerID,MAX_PEERID_SIZE,m_peerID.c_str());
	msg->size = IMESSAGE_HEADSIZE;
	sstream.Send(reinterpret_cast<char*>(msg.get()),msg->size);
CATCH_LOG("CIMStub::ResetServer")
}

void CIMStub::RemoveAllMyMessagesFromServer()
{
TRY_CATCH
	while(true)
	{
		CSocketStream sstream;
		sstream.Connect(NWL_INSTANCE.GetRelayHost(), NWL_INSTANCE.GetIMStubPort(),true);
		std::auto_ptr<sIMessage> msg;
		msg.reset(reinterpret_cast<sIMessage*>(new char[MAX_MESSAGE_SIZE]));
		msg->code = IMT_GetMessage;
		strcpy_s(msg->peerID,MAX_PEERID_SIZE,m_peerID.c_str());
		msg->size = IMESSAGE_HEADSIZE;
		sstream.Send(reinterpret_cast<char*>(msg.get()),msg->size);
		int n=0;
		char buf[MAX_MESSAGE_SIZE];
		try
		{
			for(;n<MAX_MESSAGE_SIZE;++n)
			{
				sstream.Receive(buf+n,1);
			}
		}
		catch(CStreamException&)
		{
			break;
		}
	}
CATCH_LOG("CIMStub::RemoveAllMyMessagesFromServer")
}

HANDLE CIMStub::GetMsgEvent()
{
TRY_CATCH
	SetEvent(m_hEvent);
	return m_hEvent;
CATCH_THROW("CIMStub::GetMsgEvent")
}