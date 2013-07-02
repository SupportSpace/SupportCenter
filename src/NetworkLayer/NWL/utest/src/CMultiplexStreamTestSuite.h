//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CMultiplexStreamTestSuite.h
///
///  Test suite for CStreamMultiplexerBase
///
///  @author Alexander Novak @date 04.10.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Streaming/CSocketStream.h>
#include <NWL/Multiplexer/CSubStream.h>
#include <NWL/Multiplexer/CStreamMultiplexerBase.h>
#include <boostTestLib/boostTestLib.h>
//========================================================================================================

#define MULTIPLEXSTREAM_PORT_CORRECTDATA		5020
#define MULTIPLEXSTREAM_PORT_EVENTTEST			5021
#define MULTIPLEXSTREAM_PORT_OVERFLOWTEST		5022
#define MULTIPLEXSTREAM_PORT_SUBSUBSTREAMTEST	5023
#define TIMEOUT_FOR_ACCEPT						5000
#define NUMBER_SETOFSTREAM						50				// Max number - 64
#define NUMBER_SETOFSTREAM_FORSUBSTREAM			5				// Max number - 64
#define TIMEOUT_FOR_SETOFSTREAM_ACCEPT			15000
#define NUMBER_EXCHANGE_OPERATIONS				500
#define SUBSTREAM_ID_FOR_CORRECTDATA			1
#define SUBSTREAM_PL_FOR_CORRECTDATA			1				// Priority Level
#define SUBSTREAM_ID_FOR_OVERFLOW				1
#define SUBSTREAM_PL_FOR_OVERFLOW				1				// Priority Level
#define SUBSTREAM_ID_FOR_CARRIERSTREAM			1
#define SUBSTREAM_PL_FOR_CARRIERSTREAM			1				// Priority Level
#define SUBSTREAM_OVERFLOWREFRESHTIME			1000

typedef boost::function<void (unsigned int, void*)> ServiceIDEvent;
typedef boost::function<void (boost::shared_ptr<CAbstractStream>, void*)> TransportEvent;
// namespace CMultiplexStreamTestSuite [BEGIN] ===========================================================

namespace CMultiplexStreamTestSuite
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Class CStreamMultiplexer with events for client side
// CStreamMultiplexer [BEGIN] ////////////////////////////////////////////////////////////////////////////

class CStreamMultiplexer
	:	public CStreamMultiplexerBase
{
	int m_connectedCounter;
	int m_disconnectedCounter;
	int m_brokeCounter;
	ServiceIDEvent m_handlerConnect;
	ServiceIDEvent m_handlerDisconnect;
	TransportEvent m_handlerBroke;
	void* m_connectUserData;
	void* m_disconnectUserData;
	void* m_brokeUserData;
	virtual int OnSubStreamConnected(unsigned int serviceID);
	virtual int OnSubStreamDisconnected(unsigned int serviceID);
	virtual int OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream);
public:
	CStreamMultiplexer(boost::shared_ptr<CAbstractStream> transport);
	~CStreamMultiplexer();
	void SetSubStreamConnectedHandler(ServiceIDEvent handler, void* userData);
	void SetSubStreamDisconnectedHandler(ServiceIDEvent handler, void* userData);
	void SetConnectionBrokeHandler(TransportEvent handler, void* userData);
	int GetConnectedCounter() const;
	int GetDisconnectedCounter() const;
	int GetBrokeCounter() const;
};
//--------------------------------------------------------------------------------------------------------

int CStreamMultiplexer::OnSubStreamConnected(unsigned int serviceID)
{
	m_connectedCounter++;

	if ( m_handlerConnect )
		m_handlerConnect(serviceID,m_connectUserData);
	
	return 0;
}
//--------------------------------------------------------------------------------------------------------

int CStreamMultiplexer::OnSubStreamDisconnected(unsigned int serviceID)
{
	m_disconnectedCounter++;

	if ( m_handlerDisconnect )
		m_handlerDisconnect(serviceID,m_disconnectUserData);

	return 0;
}
//--------------------------------------------------------------------------------------------------------

int CStreamMultiplexer::OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream)
{
	m_brokeCounter++;

	if ( m_handlerBroke )
		m_handlerBroke(transportStream,m_brokeUserData);

	return 0;
}
//--------------------------------------------------------------------------------------------------------

CStreamMultiplexer::CStreamMultiplexer(boost::shared_ptr<CAbstractStream> transport)
		:	CStreamMultiplexerBase(transport),
			m_connectedCounter(0),
			m_disconnectedCounter(0),
			m_brokeCounter(0),
			m_handlerConnect(NULL),
			m_handlerDisconnect(NULL),
			m_handlerBroke(NULL),
			m_connectUserData(NULL),
			m_disconnectUserData(NULL),
			m_brokeUserData(NULL)
{
	Log.Add(_MESSAGE_,_T("CStreamMultiplexer::CStreamMultiplexer()"));
}
//--------------------------------------------------------------------------------------------------------

CStreamMultiplexer::~CStreamMultiplexer()
{
	Log.Add(_MESSAGE_,_T("CStreamMultiplexer::~CStreamMultiplexer()"));
}
//--------------------------------------------------------------------------------------------------------

inline void CStreamMultiplexer::SetSubStreamConnectedHandler(ServiceIDEvent handler, void* userData)
{
	m_connectUserData	= userData;
	m_handlerConnect	= handler;
}
//--------------------------------------------------------------------------------------------------------

inline void CStreamMultiplexer::SetSubStreamDisconnectedHandler(ServiceIDEvent handler, void* userData)
{
	m_disconnectUserData	= userData;
	m_handlerDisconnect		= handler;
}
//--------------------------------------------------------------------------------------------------------

inline void CStreamMultiplexer::SetConnectionBrokeHandler(TransportEvent handler, void* userData)
{
	m_brokeUserData	= userData;
	m_handlerBroke	= handler;
}
//--------------------------------------------------------------------------------------------------------

inline int CStreamMultiplexer::GetConnectedCounter() const
{
	return m_connectedCounter;
}
//--------------------------------------------------------------------------------------------------------

inline int CStreamMultiplexer::GetDisconnectedCounter() const
{
	return m_disconnectedCounter;
}
//--------------------------------------------------------------------------------------------------------

inline int CStreamMultiplexer::GetBrokeCounter() const
{
	return m_brokeCounter;
}
// CStreamMultiplexer [END] //////////////////////////////////////////////////////////////////////////////

// CCheckForCorrectData_Server [BEGIN] ///////////////////////////////////////////////////////////////////

bool PassedCCheckForCorrectData_Server = false;

class CCheckForCorrectData_Server
	:	public CThread
{
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_eventTransportConnected;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_subStreamConnected;

	void OnTransportConnected(void*);
	void OnSubStreamConnected(unsigned int serviceID, void* userData);

	virtual void Execute(void *Params);
public:
	CCheckForCorrectData_Server();
	~CCheckForCorrectData_Server();
};
//--------------------------------------------------------------------------------------------------------

void CCheckForCorrectData_Server::OnTransportConnected(void*)
{
	SetEvent(m_eventTransportConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckForCorrectData_Server::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
	SetEvent(m_subStreamConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckForCorrectData_Server::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);
	
	transport->SetConnectedEvent(boost::bind(&CCheckForCorrectData_Server::OnTransportConnected,this,_1));
	transport->Accept(MULTIPLEXSTREAM_PORT_CORRECTDATA);
	
	if ( WaitForSingleObject(m_eventTransportConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SoketStream Accept Timeout"));
		
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);

	multiplexer->SetSubStreamConnectedHandler(boost::bind(&CCheckForCorrectData_Server::OnSubStreamConnected,this,_1,_2),NULL);
	
	boost::shared_ptr<CAbstractStream> substream = multiplexer->GetSubStream(SUBSTREAM_ID_FOR_CORRECTDATA,SUBSTREAM_PL_FOR_CORRECTDATA);
	
	if ( WaitForSingleObject(m_subStreamConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SubStream Connect Timeout"));

	Log.Add(_MESSAGE_,_T("CCheckForCorrectData_Server Conneced"));
		
	// So strange size needs for check datagram splitting
	int sz_buffer = MAX_DATAGRAM_SIZE + MAX_DATAGRAM_SIZE/2;
	boost::shared_ptr<char> buffer(new char[sz_buffer]);
	
	try
	{
		for (;;)
		{
			substream->Receive(buffer.get(),sz_buffer*sizeof(char));
			substream->Send(buffer.get(),sz_buffer*sizeof(char));
		}
	}
	catch (CStreamException& exception)
	{
		Log.Add(exception);
	}
	PassedCCheckForCorrectData_Server = true;
}
//--------------------------------------------------------------------------------------------------------

CCheckForCorrectData_Server::CCheckForCorrectData_Server()
	:	CThread()
{
	m_eventTransportConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	m_subStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
}
//--------------------------------------------------------------------------------------------------------

CCheckForCorrectData_Server::~CCheckForCorrectData_Server()
{
}
// CCheckForCorrectData_Server [END] /////////////////////////////////////////////////////////////////////

// CCheckForCorrectData_Client [BEGIN] ///////////////////////////////////////////////////////////////////

bool PassedCCheckForCorrectData_Client = false;

class CCheckForCorrectData_Client
	:	public CThread
{
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_subStreamConnected;

	void OnSubStreamConnected(unsigned int serviceID, void* userData);

	virtual void Execute(void *Params);
public:
	CCheckForCorrectData_Client();
	~CCheckForCorrectData_Client();
};
//--------------------------------------------------------------------------------------------------------

void CCheckForCorrectData_Client::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
	SetEvent(m_subStreamConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckForCorrectData_Client::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);

	transport->Connect(_T("localhost"),MULTIPLEXSTREAM_PORT_CORRECTDATA,true);
	
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);
	
	m_subStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	multiplexer->SetSubStreamConnectedHandler(boost::bind(&CCheckForCorrectData_Client::OnSubStreamConnected,this,_1,_2),NULL);
	
	boost::shared_ptr<CAbstractStream> substream = multiplexer->GetSubStream(SUBSTREAM_ID_FOR_CORRECTDATA,SUBSTREAM_PL_FOR_CORRECTDATA);
	
	if ( WaitForSingleObject(m_subStreamConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SubStream Connect Timeout"));

	Log.Add(_MESSAGE_,_T("CCheckForCorrectData_Client Conneced"));
		
	// So strange size needs for check datagram splitting
	int sz_buffer = MAX_DATAGRAM_SIZE + MAX_DATAGRAM_SIZE/2;
	boost::shared_ptr<char> buffer(new char[sz_buffer]);
	boost::shared_ptr<char> bufferRcv(new char[sz_buffer]);
	
	srand(GetTickCount());
	for (int i=0;  i < NUMBER_EXCHANGE_OPERATIONS; i++)
	{
		for (unsigned int j=0; j < sz_buffer/sizeof(char); j++)
			buffer.get()[j]=rand();
		
		substream->Send(buffer.get(),sz_buffer*sizeof(char));
		substream->Receive(bufferRcv.get(),sz_buffer*sizeof(char));
		
		if ( memcmp(buffer.get(),bufferRcv.get(),sz_buffer*sizeof(char)) )
			throw MCStreamException(_T("Data transfer was broken"));
	}
	Log.Add(_MESSAGE_,_T("Data exchange completed successfully"));
	
	PassedCCheckForCorrectData_Client = true;
}
//--------------------------------------------------------------------------------------------------------

CCheckForCorrectData_Client::CCheckForCorrectData_Client()
	:	CThread()
{
	m_subStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
}
//--------------------------------------------------------------------------------------------------------

CCheckForCorrectData_Client::~CCheckForCorrectData_Client()
{
}
// CCheckForCorrectData_Client [END] /////////////////////////////////////////////////////////////////////

// CCheckSetOfStreamsAndEventPassing_Server [BEGIN] //////////////////////////////////////////////////////

bool PassedCCheckSetOfStreamsAndEventPassing_Server = false;

class CCheckSetOfStreamsAndEventPassing_Server
	:	public CThread
{
	struct SSubStreamList
	{
		unsigned int m_subStreamID;
		bool m_eventOnConnected;
		bool m_eventOnDisconnected;
		boost::shared_ptr<CAbstractStream> m_subStream;
	};
	HANDLE m_arr_hThreadTask[NUMBER_SETOFSTREAM];
	SSubStreamList m_arSubStreams[NUMBER_SETOFSTREAM];
	
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_eventTransportConnected;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_allSubStreamsConnected;
	
	void OnTransportConnected(void*);
	void OnSubStreamConnected(unsigned int serviceID, void* userData);
	void OnSubStreamDisconnected(unsigned int serviceID, void* userData);

	static DWORD WINAPI ThreadSubStreamTask(LPVOID lpParameter);

	virtual void Execute(void *Params);
public:
	CCheckSetOfStreamsAndEventPassing_Server();
	~CCheckSetOfStreamsAndEventPassing_Server();
};
//--------------------------------------------------------------------------------------------------------

void CCheckSetOfStreamsAndEventPassing_Server::OnTransportConnected(void*)
{
	SetEvent(m_eventTransportConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckSetOfStreamsAndEventPassing_Server::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Server::OnSubStreamConnected ServiceID %u"),serviceID);

	int i=0;
	while ( i < NUMBER_SETOFSTREAM && m_arSubStreams[i].m_subStreamID != serviceID )
		i++;
	
	if ( i != NUMBER_SETOFSTREAM )
		m_arSubStreams[i].m_eventOnConnected = true;
	else
		throw MCStreamException(_T("SERVER OnSubStreamConnected: SubStream with this service identifier doesn't exist"));
		
	i=0;
	while ( i < NUMBER_SETOFSTREAM && m_arSubStreams[i].m_eventOnConnected )
		i++;
		
	if ( i ==  NUMBER_SETOFSTREAM)
	{
		Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Server::OnSubStreamConnected Fire event for start"));
		
		SetEvent(m_allSubStreamsConnected.get());
	}
	
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

void CCheckSetOfStreamsAndEventPassing_Server::OnSubStreamDisconnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Server::OnSubStreamDisconnected ServiceID %u"),serviceID);

	int i=0;
	while ( i < NUMBER_SETOFSTREAM && m_arSubStreams[i].m_subStreamID != serviceID )
		i++;
	
	if ( i != NUMBER_SETOFSTREAM )
		m_arSubStreams[i].m_eventOnDisconnected = true;
	else
		throw MCStreamException(_T("SERVER OnSubStreamDisconnected: SubStream with this service identifier doesn't exist"));

CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

DWORD WINAPI CCheckSetOfStreamsAndEventPassing_Server::ThreadSubStreamTask(LPVOID lpParameter)
{
	CSubStream* stream = static_cast<CSubStream*>(lpParameter);

	char buffer[MAX_DATAGRAM_SIZE*5];
	try
	{
		for (;;)
		{
			stream->Receive(buffer,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
			stream->Send(buffer,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
		}
	}
	catch (CStreamException& exception)
	{
		Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Server::ThreadSubStreamTask ServiceID %u Detail: %s"),stream->GetServiceID(),exception.What().c_str());
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------------

void CCheckSetOfStreamsAndEventPassing_Server::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);
	
	transport->SetConnectedEvent(boost::bind(&CCheckSetOfStreamsAndEventPassing_Server::OnTransportConnected,this,_1));
	transport->Accept(MULTIPLEXSTREAM_PORT_EVENTTEST);
	
	if ( WaitForSingleObject(m_eventTransportConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SoketStream Accept Timeout"));
		
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);

	multiplexer->SetSubStreamConnectedHandler(boost::bind(&CCheckSetOfStreamsAndEventPassing_Server::OnSubStreamConnected,this,_1,_2),NULL);
	multiplexer->SetSubStreamDisconnectedHandler(boost::bind(&CCheckSetOfStreamsAndEventPassing_Server::OnSubStreamDisconnected,this,_1,_2),NULL);
	
	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
	{
		m_arSubStreams[i].m_subStreamID	= i+1;					// (ServiceID,PriorityLevel)
		m_arSubStreams[i].m_subStream	= multiplexer->GetSubStream(m_arSubStreams[i].m_subStreamID,i*i+1);
	}
	if ( WaitForSingleObject(m_allSubStreamsConnected.get(), TIMEOUT_FOR_SETOFSTREAM_ACCEPT)!=WAIT_OBJECT_0 )
	{
		for (int i=0; i < NUMBER_SETOFSTREAM; i++)
			m_arSubStreams[i].m_subStream.reset();

		throw MCStreamException(_T("Server: SubStream Connect Timeout"));
	}

	Log.Add(_MESSAGE_,_T("CheckSetOfStreamsAndEventPassing_Server All SubStreams Connected"));
		
	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
		m_arr_hThreadTask[i] = CreateThread(NULL,0,ThreadSubStreamTask,m_arSubStreams[i].m_subStream.get(),CREATE_SUSPENDED,NULL);
	
	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
		ResumeThread(m_arr_hThreadTask[i]);

	if ( WaitForMultipleObjects(NUMBER_SETOFSTREAM,m_arr_hThreadTask,TRUE,INFINITE)!=WAIT_OBJECT_0 )
		throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());
		
	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
	{
		CloseHandle(m_arr_hThreadTask[i]);
		m_arSubStreams[i].m_subStream.reset();
	}
	//Check correct number of the disconnect events
	if ( multiplexer->GetDisconnectedCounter()!=NUMBER_SETOFSTREAM )
		throw MCStreamException(_T("Server: Number of OnConnect and OnDisconnect events are different"));
	
	//Check OnTransportBroken event
	m_arSubStreams[0].m_eventOnConnected = false;
	m_arSubStreams[0].m_subStreamID = NUMBER_SETOFSTREAM+1;
	m_arSubStreams[0].m_subStream = multiplexer->GetSubStream(NUMBER_SETOFSTREAM+1,1); // (ServiceID,PriorityLevel)

	if ( WaitForSingleObject(m_allSubStreamsConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SubStream Connect Timeout for the OnTransportBroken test"));
		
	Log.Add(_MESSAGE_,_T("CheckSetOfStreamsAndEventPassing_Server SubStream for Transport Broke Connected"));

	char buffer[MAX_DATAGRAM_SIZE/2];
	try
	{
		for (;;)
		{
			m_arSubStreams[0].m_subStream->Receive(buffer,(MAX_DATAGRAM_SIZE/2)*sizeof(char));
			m_arSubStreams[0].m_subStream->Send(buffer,(MAX_DATAGRAM_SIZE/2)*sizeof(char));
		}
	}
	catch (CStreamException& exception)
	{
		if ( multiplexer->GetBrokeCounter()!=1 )
		{
			Log.Add(_MESSAGE_,_T("Server BrokeCounter = %u"),multiplexer->GetBrokeCounter());
			
			throw exception; // SubStream was disconnected in a wrong way
		}
	}
	m_arSubStreams[0].m_subStream.reset();

	PassedCCheckSetOfStreamsAndEventPassing_Server = true;

	Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Server passed"));
}
//--------------------------------------------------------------------------------------------------------

CCheckSetOfStreamsAndEventPassing_Server::CCheckSetOfStreamsAndEventPassing_Server()
	:	CThread()
{
	m_eventTransportConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	m_allSubStreamsConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	
	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
	{
		m_arSubStreams[i].m_eventOnConnected	= false;
		m_arSubStreams[i].m_eventOnDisconnected	= false;
		m_arSubStreams[i].m_subStreamID			= 0;
	}
}
//--------------------------------------------------------------------------------------------------------

CCheckSetOfStreamsAndEventPassing_Server::~CCheckSetOfStreamsAndEventPassing_Server()
{
}
// CCheckSetOfStreamsAndEventPassing_Server [END] ////////////////////////////////////////////////////////

// CCheckSetOfStreamsAndEventPassing_Client [BEGIN] //////////////////////////////////////////////////////

bool PassedCCheckSetOfStreamsAndEventPassing_Client = false;

class CCheckSetOfStreamsAndEventPassing_Client
	:	public CThread
{
	struct SSubStreamList
	{
		unsigned int m_subStreamID;
		bool m_eventOnConnected;
		boost::shared_ptr<CAbstractStream> m_subStream;
	};
	HANDLE m_arr_hThreadTask[NUMBER_SETOFSTREAM];
	SSubStreamList m_arSubStreams[NUMBER_SETOFSTREAM];
	
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_allSubStreamsConnected;
	
	void OnSubStreamConnected(unsigned int serviceID, void* userData);

	static DWORD WINAPI ThreadSubStreamTask(LPVOID lpParameter);

	virtual void Execute(void *Params);
public:
	CCheckSetOfStreamsAndEventPassing_Client();
	~CCheckSetOfStreamsAndEventPassing_Client();
};
//--------------------------------------------------------------------------------------------------------

void CCheckSetOfStreamsAndEventPassing_Client::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Client::OnSubStreamConnected ServiceID %u"),serviceID);

	int i=0;
	while ( i < NUMBER_SETOFSTREAM && m_arSubStreams[i].m_subStreamID != serviceID )
		i++;
	
	if ( i != NUMBER_SETOFSTREAM )
		m_arSubStreams[i].m_eventOnConnected = true;
	else
		throw MCStreamException(_T("CLIENT OnSubStreamConnected: SubStream with this service identifier doesn't exist"));
		
	i=0;
	while ( i < NUMBER_SETOFSTREAM && m_arSubStreams[i].m_eventOnConnected )
		i++;
		
	if ( i ==  NUMBER_SETOFSTREAM)
	{
		Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Client::OnSubStreamConnected Fire event for start"));

		SetEvent(m_allSubStreamsConnected.get());
	}

CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

DWORD WINAPI CCheckSetOfStreamsAndEventPassing_Client::ThreadSubStreamTask(LPVOID lpParameter)
{
	CSubStream* stream = static_cast<CSubStream*>(lpParameter);

	char buffer[MAX_DATAGRAM_SIZE*5];
	char bufferRcv[MAX_DATAGRAM_SIZE*5];
	
	for (int i=0;  i < NUMBER_EXCHANGE_OPERATIONS; i++)
	{
		for (unsigned int j=0; j < (MAX_DATAGRAM_SIZE*5)/sizeof(char); j++)
			buffer[j]=j+i;
		
		stream->Send(buffer,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
		stream->Receive(bufferRcv,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
		
		if ( memcmp(buffer,bufferRcv,(MAX_DATAGRAM_SIZE*5)*sizeof(char)) )
			throw MCStreamException(_T("Data transfer was broken"));
	}
	Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Client: All operations have been done for serviceID %u"),stream->GetServiceID());

	return 0;
}
//--------------------------------------------------------------------------------------------------------

void CCheckSetOfStreamsAndEventPassing_Client::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);

	transport->Connect(_T("localhost"),MULTIPLEXSTREAM_PORT_EVENTTEST,true);
	
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);

	multiplexer->SetSubStreamConnectedHandler(boost::bind(&CCheckSetOfStreamsAndEventPassing_Client::OnSubStreamConnected,this,_1,_2),NULL);

	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
	{
		m_arSubStreams[i].m_subStreamID			= i+1;					// (ServiceID,PriorityLevel)
		m_arSubStreams[i].m_subStream			= multiplexer->GetSubStream(m_arSubStreams[i].m_subStreamID,i*i+1);
	}

	if ( WaitForSingleObject(m_allSubStreamsConnected.get(), TIMEOUT_FOR_SETOFSTREAM_ACCEPT)!=WAIT_OBJECT_0 )
	{
		for (int i=0; i < NUMBER_SETOFSTREAM; i++)
			m_arSubStreams[i].m_subStream.reset();

		throw MCStreamException(_T("SubStream Connect Timeout"));
	}

	Log.Add(_MESSAGE_,_T("CheckSetOfStreamsAndEventPassing_Client All SubStreams Connected"));

	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
		m_arr_hThreadTask[i] = CreateThread(NULL,0,ThreadSubStreamTask,m_arSubStreams[i].m_subStream.get(),CREATE_SUSPENDED,NULL);

	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
		ResumeThread(m_arr_hThreadTask[i]);

	if ( WaitForMultipleObjects(NUMBER_SETOFSTREAM,m_arr_hThreadTask,TRUE,INFINITE)!=WAIT_OBJECT_0 )
		throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());

	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
	{
		CloseHandle(m_arr_hThreadTask[i]);
		m_arSubStreams[i].m_subStream.reset();
	}
	//Check OnTransportBroken event
	m_arSubStreams[0].m_eventOnConnected = false;
	m_arSubStreams[0].m_subStreamID = NUMBER_SETOFSTREAM+1;
	m_arSubStreams[0].m_subStream = multiplexer->GetSubStream(NUMBER_SETOFSTREAM+1,1); // (ServiceID,PriorityLevel)

	if ( WaitForSingleObject(m_allSubStreamsConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SubStream Connect Timeout for the OnTransportBroken test"));

	Log.Add(_MESSAGE_,_T("CheckSetOfStreamsAndEventPassing_Client SubStream for Transport Broke Connected"));

	char buffer[MAX_DATAGRAM_SIZE/2];
	char bufferRcv[MAX_DATAGRAM_SIZE/2];
	
	for (int i=0;  i < NUMBER_EXCHANGE_OPERATIONS; i++)
	{
		for (unsigned int j=0; j < (MAX_DATAGRAM_SIZE/2)/sizeof(char); j++)
			buffer[j]=j+i;
		
		m_arSubStreams[0].m_subStream->Send(buffer,(MAX_DATAGRAM_SIZE/2)*sizeof(char));
		m_arSubStreams[0].m_subStream->Receive(bufferRcv,(MAX_DATAGRAM_SIZE/2)*sizeof(char));
		
		if ( memcmp(buffer,bufferRcv,(MAX_DATAGRAM_SIZE/2)*sizeof(char)) )
			throw MCStreamException(_T("Data transfer was broken"));
	}
	Log.Add(_MESSAGE_,_T("Client: Data exchange was successful"));
	Log.Add(_MESSAGE_,_T("Try to break the transport"));

	transport->Disconnect();
	//Try to do transfer operation for event firing
	bool receivePassed = false;
	try
	{
		m_arSubStreams[0].m_subStream->Send(buffer,(MAX_DATAGRAM_SIZE/2)*sizeof(char));
		m_arSubStreams[0].m_subStream->Receive(buffer,(MAX_DATAGRAM_SIZE/2)*sizeof(char));
		receivePassed = true;
	}
	catch (CStreamException& exception)
	{
		if ( multiplexer->GetBrokeCounter()!=1 )
		{
			Log.Add(_MESSAGE_,_T("Client BrokeCounter = %u"),multiplexer->GetBrokeCounter());
		
			throw exception; // SubStream was disconnected in a wrong way
		}
	}
	if ( receivePassed )
		throw MCStreamException(_T("Receive operation had passed after transport was closed"));
	
	m_arSubStreams[0].m_subStream.reset();
	multiplexer.reset();

	PassedCCheckSetOfStreamsAndEventPassing_Client = true;
	
	Log.Add(_MESSAGE_,_T("CCheckSetOfStreamsAndEventPassing_Client passed"));
}
//--------------------------------------------------------------------------------------------------------

CCheckSetOfStreamsAndEventPassing_Client::CCheckSetOfStreamsAndEventPassing_Client()
	:	CThread()
{
	m_allSubStreamsConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);

	for (int i=0; i < NUMBER_SETOFSTREAM; i++)
	{
		m_arSubStreams[i].m_eventOnConnected	= false;
		m_arSubStreams[i].m_subStreamID			= 0;
	}
}
//--------------------------------------------------------------------------------------------------------

CCheckSetOfStreamsAndEventPassing_Client::~CCheckSetOfStreamsAndEventPassing_Client()
{
}
// CCheckSetOfStreamsAndEventPassing_Client [END] ////////////////////////////////////////////////////////

// CCheckForOverflow_Server [BEGIN] //////////////////////////////////////////////////////////////////////

bool PassedCCheckForOverflow_Server = false;

class CCheckForOverflow_Server
	:	public CThread
{
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_eventTransportConnected;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_subStreamConnected;

	void OnTransportConnected(void*);
	void OnSubStreamConnected(unsigned int serviceID, void* userData);

	virtual void Execute(void *Params);
public:
	CCheckForOverflow_Server();
	~CCheckForOverflow_Server();
};
//--------------------------------------------------------------------------------------------------------

void CCheckForOverflow_Server::OnTransportConnected(void*)
{
	SetEvent(m_eventTransportConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckForOverflow_Server::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
	SetEvent(m_subStreamConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckForOverflow_Server::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);
	
	transport->SetConnectedEvent(boost::bind(&CCheckForOverflow_Server::OnTransportConnected,this,_1));
	transport->Accept(MULTIPLEXSTREAM_PORT_OVERFLOWTEST);
	
	if ( WaitForSingleObject(m_eventTransportConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SoketStream Accept Timeout"));
		
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);

	multiplexer->SetSubStreamConnectedHandler(boost::bind(&CCheckForOverflow_Server::OnSubStreamConnected,this,_1,_2),NULL);
	
	boost::shared_ptr<CAbstractStream> substream = multiplexer->GetSubStream(SUBSTREAM_ID_FOR_OVERFLOW,SUBSTREAM_PL_FOR_OVERFLOW);
	
	if ( WaitForSingleObject(m_subStreamConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SubStream Connect Timeout"));

	Log.Add(_MESSAGE_,_T("CCheckForOverflow_Server Connected"));
		
	char buffer[MAX_DATAGRAM_SIZE];
	try
	{
		for (;;)
		{
			Sleep(SUBSTREAM_OVERFLOWREFRESHTIME);
			substream->Receive(buffer,MAX_DATAGRAM_SIZE*sizeof(char));
		}
	}
	catch (CStreamException& exception)
	{
		PassedCCheckForOverflow_Server = true;
	
		Log.Add(	_MESSAGE_,
					_T("Server disconnected. CountConnect: %u, CountDisconnect: %u, CountBroken: %u"),
					multiplexer->GetConnectedCounter(),
					multiplexer->GetDisconnectedCounter(),
					multiplexer->GetBrokeCounter());

		throw exception;
	}
}
//--------------------------------------------------------------------------------------------------------

CCheckForOverflow_Server::CCheckForOverflow_Server()
	:	CThread()
{
	m_eventTransportConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	m_subStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
}
//--------------------------------------------------------------------------------------------------------

CCheckForOverflow_Server::~CCheckForOverflow_Server()
{
}
// CCheckForOverflow_Server [END] ////////////////////////////////////////////////////////////////////////

// CCheckForOverflow_Client [BEGIN] //////////////////////////////////////////////////////////////////////

bool PassedCCheckForOverflow_Client = false;

class CCheckForOverflow_Client
	:	public CThread
{
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_subStreamConnected;

	void OnSubStreamConnected(unsigned int serviceID, void* userData);

	virtual void Execute(void *Params);
public:
	CCheckForOverflow_Client();
	~CCheckForOverflow_Client();
};
//--------------------------------------------------------------------------------------------------------

void CCheckForOverflow_Client::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
	SetEvent(m_subStreamConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckForOverflow_Client::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);

	transport->Connect(_T("localhost"),MULTIPLEXSTREAM_PORT_OVERFLOWTEST,true);
	
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);
	
	m_subStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	multiplexer->SetSubStreamConnectedHandler(boost::bind(&CCheckForOverflow_Client::OnSubStreamConnected,this,_1,_2),NULL);
	
	boost::shared_ptr<CAbstractStream> substream = multiplexer->GetSubStream(SUBSTREAM_ID_FOR_OVERFLOW,SUBSTREAM_PL_FOR_OVERFLOW);
	
	if ( WaitForSingleObject(m_subStreamConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SubStream Connect Timeout"));

	Log.Add(_MESSAGE_,_T("CCheckForOverflow_Client Connected"));
		
	char buffer[MAX_DATAGRAM_SIZE];
	
	srand(GetTickCount());
	try
	{
		for (int i=0;  i < (SUBSTREAM_BUFFERSIZE/MAX_DATAGRAM_SIZE)*2; i++)
			substream->Send(buffer,MAX_DATAGRAM_SIZE*sizeof(char));
	}
	catch (CStreamException& exception)
	{
		PassedCCheckForOverflow_Client = true;

		Log.Add(	_MESSAGE_,
					_T("Client disconnected. CountConnect: %u, CountDisconnect: %u, CountBroken: %u"),
					multiplexer->GetConnectedCounter(),
					multiplexer->GetDisconnectedCounter(),
					multiplexer->GetBrokeCounter());
	
		throw exception;
	}
}
//--------------------------------------------------------------------------------------------------------

CCheckForOverflow_Client::CCheckForOverflow_Client()
	:	CThread()
{
	m_subStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
}
//--------------------------------------------------------------------------------------------------------

CCheckForOverflow_Client::~CCheckForOverflow_Client()
{
}
// CCheckForOverflow_Client [END] ////////////////////////////////////////////////////////////////////////

// CCheckSubstreamAsTransport_Server [BEGIN] /////////////////////////////////////////////////////////////

bool PassedCCheckSubstreamAsTransport_Server = false;

class CCheckSubstreamAsTransport_Server
	:	public CThread
{
	struct SSubStreamList
	{
		unsigned int m_subStreamID;
		bool m_eventOnConnected;
		bool m_eventOnDisconnected;
		boost::shared_ptr<CAbstractStream> m_subStream;
	};
	HANDLE m_arr_hThreadTask[NUMBER_SETOFSTREAM_FORSUBSTREAM];
	SSubStreamList m_arSubStreams[NUMBER_SETOFSTREAM_FORSUBSTREAM];
	
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_eventTransportConnected;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_CarrierStreamConnected;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_allSubStreamsConnected;
	
	void OnTransportConnected(void*);
	void OnCarrierStreamConnected(unsigned int serviceID, void* userData);
	void OnCarrierStreamDisconnected(unsigned int serviceID, void* userData);
	void OnSubStreamConnected(unsigned int serviceID, void* userData);
	void OnSubStreamDisconnected(unsigned int serviceID, void* userData);

	static DWORD WINAPI ThreadSubStreamTask(LPVOID lpParameter);

	virtual void Execute(void *Params);
public:
	CCheckSubstreamAsTransport_Server();
	~CCheckSubstreamAsTransport_Server();
};
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Server::OnTransportConnected(void*)
{
	SetEvent(m_eventTransportConnected.get());
}
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Server::OnCarrierStreamConnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Server::OnCarrierStreamConnected ServiceID %u"),serviceID);

	SetEvent(m_CarrierStreamConnected.get());
	
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Server::OnCarrierStreamDisconnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Server::OnCarrierStreamDisconnected ServiceID %u"),serviceID);

CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Server::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Server::OnSubStreamConnected ServiceID %u"),serviceID);

	int i=0;
	while ( i < NUMBER_SETOFSTREAM_FORSUBSTREAM && m_arSubStreams[i].m_subStreamID != serviceID )
		i++;
	
	if ( i != NUMBER_SETOFSTREAM_FORSUBSTREAM )
		m_arSubStreams[i].m_eventOnConnected = true;
	else
		throw MCStreamException(_T("SERVER OnSubStreamConnected: SubStream with this service identifier doesn't exist"));
		
	i=0;
	while ( i < NUMBER_SETOFSTREAM_FORSUBSTREAM && m_arSubStreams[i].m_eventOnConnected )
		i++;
		
	if ( i ==  NUMBER_SETOFSTREAM_FORSUBSTREAM)
	{
		Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Server::OnSubStreamConnected Fire event for start"));
		
		SetEvent(m_allSubStreamsConnected.get());
	}
	
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Server::OnSubStreamDisconnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Server::OnSubStreamDisconnected ServiceID %u"),serviceID);

	int i=0;
	while ( i < NUMBER_SETOFSTREAM_FORSUBSTREAM && m_arSubStreams[i].m_subStreamID != serviceID )
		i++;
	
	if ( i != NUMBER_SETOFSTREAM_FORSUBSTREAM )
		m_arSubStreams[i].m_eventOnDisconnected = true;
	else
		throw MCStreamException(_T("SERVER OnSubStreamDisconnected: SubStream with this service identifier doesn't exist"));

CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

DWORD WINAPI CCheckSubstreamAsTransport_Server::ThreadSubStreamTask(LPVOID lpParameter)
{
	CSubStream* stream = static_cast<CSubStream*>(lpParameter);

	char buffer[MAX_DATAGRAM_SIZE*5];
	try
	{
		for (;;)
		{
			stream->Receive(buffer,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
			stream->Send(buffer,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
		}
	}
	catch (CStreamException& exception)
	{
		Log.Add(	_MESSAGE_,
					_T("CCheckSubstreamAsTransport_Server::ThreadSubStreamTask ServiceID %u Detail: %s"),
					stream->GetServiceID(),
					exception.What().c_str());
	}
	return 0;
}
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Server::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);
	
	transport->SetConnectedEvent(boost::bind(&CCheckSubstreamAsTransport_Server::OnTransportConnected,this,_1));
	transport->Accept(MULTIPLEXSTREAM_PORT_SUBSUBSTREAMTEST);
	
	if ( WaitForSingleObject(m_eventTransportConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
		throw MCStreamException(_T("SoketStream Accept Timeout"));
		
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);

	multiplexer->SetSubStreamConnectedHandler(	boost::bind(&CCheckSubstreamAsTransport_Server::OnCarrierStreamConnected,this,_1,_2),
												NULL);
	multiplexer->SetSubStreamDisconnectedHandler(	boost::bind(&CCheckSubstreamAsTransport_Server::OnCarrierStreamDisconnected,this,_1,_2),
													NULL);
	
	boost::shared_ptr<CAbstractStream> carrierStream = multiplexer->GetSubStream(	SUBSTREAM_ID_FOR_CARRIERSTREAM,
																					SUBSTREAM_PL_FOR_CARRIERSTREAM);

	if ( WaitForSingleObject(m_CarrierStreamConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
	{
		carrierStream.reset();

		throw MCStreamException(_T("Server: CarrierStream Connect Timeout"));
	}
	//Create Multiplexer based on substream
	boost::shared_ptr<CStreamMultiplexer> multiplexerOnSubStream = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(carrierStream);
	
	multiplexerOnSubStream->SetSubStreamConnectedHandler(	boost::bind(&CCheckSubstreamAsTransport_Server::OnSubStreamConnected,this,_1,_2),
															NULL);
	multiplexerOnSubStream->SetSubStreamDisconnectedHandler(	boost::bind(&CCheckSubstreamAsTransport_Server::OnSubStreamDisconnected,this,_1,_2),
																NULL);

	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
	{
		m_arSubStreams[i].m_subStreamID	= i+1;								//(ServiceID,PriorityLevel)
		m_arSubStreams[i].m_subStream	= multiplexerOnSubStream->GetSubStream(m_arSubStreams[i].m_subStreamID,i*i+1);
	}
	if ( WaitForSingleObject(m_allSubStreamsConnected.get(), TIMEOUT_FOR_SETOFSTREAM_ACCEPT)!=WAIT_OBJECT_0 )
	{
		for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
			m_arSubStreams[i].m_subStream.reset();

		throw MCStreamException(_T("Server: SubStream Connect Timeout"));
	}
	Log.Add(_MESSAGE_,_T("CheckSetOfStreamsAndEventPassing_Server All SubStreams Connected"));

	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
		m_arr_hThreadTask[i] = CreateThread(NULL,0,ThreadSubStreamTask,m_arSubStreams[i].m_subStream.get(),CREATE_SUSPENDED,NULL);
	
	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
		ResumeThread(m_arr_hThreadTask[i]);

	if ( WaitForMultipleObjects(NUMBER_SETOFSTREAM_FORSUBSTREAM,m_arr_hThreadTask,TRUE,INFINITE)!=WAIT_OBJECT_0 )
		throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());
		
	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
	{
		CloseHandle(m_arr_hThreadTask[i]);
		m_arSubStreams[i].m_subStream.reset();
	}
	//Check correct number of the disconnect events
	Log.Add(_MESSAGE_,
			_T("CheckSetOfStreamsAndEventPassing_Server multiplexerOnSubStream's State: ConnectedCounter: %u, DisconnectedCounter: %u, BrokeCounter: %u"),
			multiplexerOnSubStream->GetConnectedCounter(),
			multiplexerOnSubStream->GetDisconnectedCounter(),
			multiplexerOnSubStream->GetBrokeCounter());

	if (	multiplexerOnSubStream->GetBrokeCounter()!=1 && 
			multiplexerOnSubStream->GetDisconnectedCounter()!=NUMBER_SETOFSTREAM_FORSUBSTREAM )
		throw MCStreamException(_T("Server: Eventing model has been failed"));

	PassedCCheckSubstreamAsTransport_Server = true;

	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Server passed"));
}
//--------------------------------------------------------------------------------------------------------

CCheckSubstreamAsTransport_Server::CCheckSubstreamAsTransport_Server()
	:	CThread()
{
	m_eventTransportConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	m_CarrierStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	m_allSubStreamsConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);

	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
	{
		m_arSubStreams[i].m_eventOnConnected	= false;
		m_arSubStreams[i].m_eventOnDisconnected	= false;
		m_arSubStreams[i].m_subStreamID			= 0;
	}
}
//--------------------------------------------------------------------------------------------------------

CCheckSubstreamAsTransport_Server::~CCheckSubstreamAsTransport_Server()
{
}
// CCheckSubstreamAsTransport_Server [END] ///////////////////////////////////////////////////////////////

// CCheckSubstreamAsTransport_Client [BEGIN] /////////////////////////////////////////////////////////////

bool PassedCCheckSubstreamAsTransport_Client = false;

class CCheckSubstreamAsTransport_Client
	:	public CThread
{
	struct SSubStreamList
	{
		unsigned int m_subStreamID;
		bool m_eventOnConnected;
		boost::shared_ptr<CAbstractStream> m_subStream;
	};
	HANDLE m_arr_hThreadTask[NUMBER_SETOFSTREAM_FORSUBSTREAM];
	SSubStreamList m_arSubStreams[NUMBER_SETOFSTREAM_FORSUBSTREAM];
	
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_CarrierStreamConnected;
	boost::shared_ptr< boost::remove_pointer<HANDLE>::type > m_allSubStreamsConnected;
	
	void OnCarrierStreamConnected(unsigned int serviceID, void* userData);
	void OnSubStreamConnected(unsigned int serviceID, void* userData);

	static DWORD WINAPI ThreadSubStreamTask(LPVOID lpParameter);

	virtual void Execute(void *Params);
public:
	CCheckSubstreamAsTransport_Client();
	~CCheckSubstreamAsTransport_Client();
};
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Client::OnCarrierStreamConnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Client::OnCarrierStreamConnected ServiceID %u"),serviceID);

	SetEvent(m_CarrierStreamConnected.get());
	
CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Client::OnSubStreamConnected(unsigned int serviceID, void* userData)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Client::OnSubStreamConnected ServiceID %u"),serviceID);

	int i=0;
	while ( i < NUMBER_SETOFSTREAM_FORSUBSTREAM && m_arSubStreams[i].m_subStreamID != serviceID )
		i++;
	
	if ( i != NUMBER_SETOFSTREAM_FORSUBSTREAM )
		m_arSubStreams[i].m_eventOnConnected = true;
	else
		throw MCStreamException(_T("CLIENT OnSubStreamConnected: SubStream with this service identifier doesn't exist"));
		
	i=0;
	while ( i < NUMBER_SETOFSTREAM_FORSUBSTREAM && m_arSubStreams[i].m_eventOnConnected )
		i++;
		
	if ( i ==  NUMBER_SETOFSTREAM_FORSUBSTREAM)
	{
		Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Client::OnSubStreamConnected Fire event for start"));

		SetEvent(m_allSubStreamsConnected.get());
	}

CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

DWORD WINAPI CCheckSubstreamAsTransport_Client::ThreadSubStreamTask(LPVOID lpParameter)
{
	CSubStream* stream = static_cast<CSubStream*>(lpParameter);

	char buffer[MAX_DATAGRAM_SIZE*5];
	char bufferRcv[MAX_DATAGRAM_SIZE*5];
	
	for (int i=0;  i < NUMBER_EXCHANGE_OPERATIONS; i++)
	{
		for (unsigned int j=0; j < (MAX_DATAGRAM_SIZE*5)/sizeof(char); j++)
			buffer[j]=j+i;
		
		stream->Send(buffer,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
		stream->Receive(bufferRcv,(MAX_DATAGRAM_SIZE*5)*sizeof(char));
		
		if ( memcmp(buffer,bufferRcv,(MAX_DATAGRAM_SIZE*5)*sizeof(char)) )
			throw MCStreamException(_T("Data transfer was broken"));
	}
	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Client: All operations have been done for serviceID %u"),stream->GetServiceID());

	return 0;
}
//--------------------------------------------------------------------------------------------------------

void CCheckSubstreamAsTransport_Client::Execute(void *Params)
{
	boost::shared_ptr<CSocketStream> transport(new CSocketStream);

	transport->Connect(_T("localhost"),MULTIPLEXSTREAM_PORT_SUBSUBSTREAMTEST,true);
	
	boost::shared_ptr<CStreamMultiplexer> multiplexer = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(transport);

	multiplexer->SetSubStreamConnectedHandler(boost::bind(&CCheckSubstreamAsTransport_Client::OnCarrierStreamConnected,this,_1,_2),NULL);

	boost::shared_ptr<CAbstractStream> carrierStream = multiplexer->GetSubStream(	SUBSTREAM_ID_FOR_CARRIERSTREAM,
																					SUBSTREAM_PL_FOR_CARRIERSTREAM);

	if ( WaitForSingleObject(m_CarrierStreamConnected.get(), TIMEOUT_FOR_ACCEPT)!=WAIT_OBJECT_0 )
	{
		carrierStream.reset();

		throw MCStreamException(_T("Client: CarrierStream Connect Timeout"));
	}
	//Create Multiplexer based on substream
	boost::shared_ptr<CStreamMultiplexer> multiplexerOnSubStream = CStreamMultiplexer::GetInstance<CStreamMultiplexer>(carrierStream);
	
	multiplexerOnSubStream->SetSubStreamConnectedHandler(boost::bind(&CCheckSubstreamAsTransport_Client::OnSubStreamConnected,this,_1,_2),NULL);

	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
	{
		m_arSubStreams[i].m_subStreamID	= i+1;								//(ServiceID,PriorityLevel)
		m_arSubStreams[i].m_subStream	= multiplexerOnSubStream->GetSubStream(m_arSubStreams[i].m_subStreamID,i*i+1);
	}
	if ( WaitForSingleObject(m_allSubStreamsConnected.get(), TIMEOUT_FOR_SETOFSTREAM_ACCEPT)!=WAIT_OBJECT_0 )
	{
		for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
			m_arSubStreams[i].m_subStream.reset();

		throw MCStreamException(_T("Client: SubStream Connect Timeout"));
	}
	Log.Add(_MESSAGE_,_T("CheckSetOfStreamsAndEventPassing_Client All SubStreams Connected"));

	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
		m_arr_hThreadTask[i] = CreateThread(NULL,0,ThreadSubStreamTask,m_arSubStreams[i].m_subStream.get(),CREATE_SUSPENDED,NULL);
	
	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
		ResumeThread(m_arr_hThreadTask[i]);

	if ( WaitForMultipleObjects(NUMBER_SETOFSTREAM_FORSUBSTREAM,m_arr_hThreadTask,TRUE,INFINITE)!=WAIT_OBJECT_0 )
		throw MCStreamException_Win(_T("Wait operation failed"),GetLastError());

	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
	{
		CloseHandle(m_arr_hThreadTask[i]);
		m_arSubStreams[i].m_subStream.reset();
	}
	PassedCCheckSubstreamAsTransport_Client = true;
	
	Log.Add(_MESSAGE_,_T("CCheckSubstreamAsTransport_Client passed"));
}
//--------------------------------------------------------------------------------------------------------

CCheckSubstreamAsTransport_Client::CCheckSubstreamAsTransport_Client()
	:	CThread()
{
	m_allSubStreamsConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	m_CarrierStreamConnected.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);

	for (int i=0; i < NUMBER_SETOFSTREAM_FORSUBSTREAM; i++)
	{
		m_arSubStreams[i].m_eventOnConnected	= false;
		m_arSubStreams[i].m_subStreamID			= 0;
	}
}
//--------------------------------------------------------------------------------------------------------

CCheckSubstreamAsTransport_Client::~CCheckSubstreamAsTransport_Client()
{
}
// CCheckSubstreamAsTransport_Client [END] ///////////////////////////////////////////////////////////////

// Functions for testing [BEGIN] /////////////////////////////////////////////////////////////////////////

void TestCheckCorrectDataTransfer()
{
	CCheckForCorrectData_Server server;
	CCheckForCorrectData_Client client;

	server.Start();
	Sleep(1000);		//Wait for initialization of server
	client.Start();
	
	client.Stop(false);
	server.Stop(false);
	
	BOOST_CHECK( PassedCCheckForCorrectData_Client && PassedCCheckForCorrectData_Server );
}
//--------------------------------------------------------------------------------------------------------

void TestCheckSetOfStreamsAndEventPassing()
{
	CCheckSetOfStreamsAndEventPassing_Server server;
	CCheckSetOfStreamsAndEventPassing_Client client;

	server.Start();
	Sleep(1000);		//Wait for initialization of server
	client.Start();

	client.Stop(false);
	server.Stop(false);
	
	BOOST_CHECK( PassedCCheckSetOfStreamsAndEventPassing_Client && PassedCCheckSetOfStreamsAndEventPassing_Server );
}
//--------------------------------------------------------------------------------------------------------

void TestCheckSubStreamOverflow()
{
	CCheckForOverflow_Server server;
	CCheckForOverflow_Client client;

	server.Start();
	Sleep(1000);		//Wait for initialization of server
	client.Start();
	
	client.Stop(false);
	server.Stop(false);
	
	BOOST_CHECK( PassedCCheckForOverflow_Client && PassedCCheckForOverflow_Server );
}
//--------------------------------------------------------------------------------------------------------

void TestCheckSubstreamAsTransport()
{
	CCheckSubstreamAsTransport_Server server;
	CCheckSubstreamAsTransport_Client client;

	server.Start();
	Sleep(1000);		//Wait for initialization of server
	client.Start();

	client.Stop(false);
	server.Stop(false);
	
	BOOST_CHECK( PassedCCheckSubstreamAsTransport_Client && PassedCCheckSubstreamAsTransport_Server );
}
// Functions for testing [END] ///////////////////////////////////////////////////////////////////////////

}
// namespace CMultiplexStreamTestSuite [END] =============================================================

test_suite* getCMultiplexStreamTestSuite()
{
	test_suite* suite = BOOST_TEST_SUITE( "CMultiplexStreamTestSuite" );

	suite->add( BOOST_TEST_CASE(CMultiplexStreamTestSuite::TestCheckCorrectDataTransfer) );
	suite->add( BOOST_TEST_CASE(CMultiplexStreamTestSuite::TestCheckSetOfStreamsAndEventPassing) );
	suite->add( BOOST_TEST_CASE(CMultiplexStreamTestSuite::TestCheckSubStreamOverflow) );
	suite->add( BOOST_TEST_CASE(CMultiplexStreamTestSuite::TestCheckSubstreamAsTransport) );

	return suite;
}
