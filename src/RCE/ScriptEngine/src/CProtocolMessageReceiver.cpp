/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CProtocolMessageReceiver.h
///
///  Implements CProtocolMessageReceiver class, responsible for thread for 
///    receiving protocol messages
///
///  @author Dmitry Netrebenko @date 16.11.2007
///
////////////////////////////////////////////////////////////////////////

#include "CProtocolMessageReceiver.h"
#include <AidLib/CException/CException.h>
#include <AidLib/CThread/CThreadLS.h>

CProtocolMessageReceiver::CProtocolMessageReceiver()
	:	CThread()
	,	m_inited(false)
{
TRY_CATCH
CATCH_THROW()
}

CProtocolMessageReceiver::~CProtocolMessageReceiver()
{
TRY_CATCH
	StopReceiver();
CATCH_LOG()
}

void CProtocolMessageReceiver::InitReceiver(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH
	if (NULL != m_stream)
		throw MCException("Already initialized");
	/// Store stream and start thread
	m_stream = stream;
	Log.Add(_MESSAGE_,_T("CProtocolMessageReceiver::InitReceiver"));
	Start();
	m_inited = true;
CATCH_THROW()
}

void CProtocolMessageReceiver::Execute(void *Params)
{
	::CoInitialize(NULL);
	SET_THREAD_LS;
TRY_CATCH
	while(!Terminated())
	{
		if(!m_stream.get())
			throw MCException(_T("Transport stream is not created."));
		SScriptEngineMsg msg;
		/// Receive message header
		m_stream->Receive(reinterpret_cast<char*>(&msg), SCRIPTENGINE_MSG_HEAD_SIZE);
		boost::shared_array<char> buf;
		if(msg.m_size - SCRIPTENGINE_MSG_HEAD_SIZE > 0)
		{
			/// Allocate buffer and receive data
			buf.reset(new char[msg.m_size - SCRIPTENGINE_MSG_HEAD_SIZE]);
			m_stream->Receive(buf.get(), msg.m_size - SCRIPTENGINE_MSG_HEAD_SIZE);
		}
		TRY_CATCH
			/// Handle received message
			HandleProtocolMessage(msg.m_id, msg.m_type, buf, msg.m_size - SCRIPTENGINE_MSG_HEAD_SIZE);
		CATCH_LOG()
	}
CATCH_LOG()
TRY_CATCH
	OnReceiverStopped();
CATCH_LOG()
	::CoUninitialize();
}

void CProtocolMessageReceiver::StopReceiver()
{
TRY_CATCH
	if(m_inited)
	{
		/// Terminate thread
		Terminate();
		/// Close substream
		if(m_stream.get())
			m_stream->CancelReceiveOperation();
		/// Wait for thread termination
		WaitForSingleObject(hTerminatedEvent.get(), INFINITE);
		/// Destroy stream
		m_stream.reset();
		m_inited = false;
	}
CATCH_THROW()
}

