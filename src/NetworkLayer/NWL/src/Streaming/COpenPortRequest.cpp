
#include <NWL/Streaming/COpenPortRequest.h>
#include <AidLib/CException/CException.h>
#include <AidLib/CCrypto/CCrypto.h>

// COpenPortRequest [BEGIN] //////////////////////////////////////////////////////////////

COpenPortRequest::COpenPortRequest(const tstring& serverAddress, unsigned short serverPort)
	:	m_server_address(serverAddress),
		m_server_port(serverPort),
		m_thread_listen_port()
{
TRY_CATCH
CATCH_THROW()
}
//----------------------------------------------------------------------------------------

COpenPortRequest::~COpenPortRequest()
{
TRY_CATCH
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

void COpenPortRequest::CheckPortAvailability(	const tstring& userID,
												const tstring& password,
												unsigned short inPort,
												unsigned short exPort)
{
TRY_CATCH
	Log.Add(_MESSAGE_,"Connecting to server...");
	m_socket_authentication.Connect(m_server_address, m_server_port, true);
	Log.Add(_MESSAGE_,"Connected");

	SRelayMessage msg;

	Log.Add(_MESSAGE_,"Sending authentication request...");
	msg.type = rmtAuthRequest;
	msg.size = static_cast<int>(RELAY_MSG_HEAD_SIZE + (userID.length() + 1)*sizeof(TCHAR));
	_tcscpy_s(msg.data,MAX_MSG_SIZE,userID.c_str()); //ATTENTION: Unicode problem

	m_socket_authentication.Send(reinterpret_cast<char*>(&msg),msg.size);

	Log.Add(_MESSAGE_,"Receiving challenge request...");
	m_socket_authentication.Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);

	if (!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_CHALLENGE_SIZE)
		throw MCStreamException("Invalid message size");
	if (msg.type != rmtChallengeRequest)
		throw MCStreamException("Invalid message type");

	m_socket_authentication.Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);

	// Calculating hash
	std::auto_ptr<TCHAR> buffer;
	buffer.reset(new TCHAR[RELAY_CHALLENGE_SIZE + password.length()]);

	memcpy(buffer.get(),password.c_str(),password.length()*sizeof(TCHAR));
	memcpy(buffer.get()+password.length(),msg.data,RELAY_CHALLENGE_SIZE);

	char hash[RELAY_HASH_SIZE];
	CRYPTO_INSTANCE.MakeHash(	buffer.get(),
												(int)(RELAY_CHALLENGE_SIZE + password.length())*sizeof(TCHAR),
												hash,
												RELAY_HASH_SIZE);

	Log.Add(_MESSAGE_,"Sending challenge response...");
	msg.type = rmtChallengeResponceNoSession;
	msg.size = RELAY_MSG_HEAD_SIZE + RELAY_HASH_SIZE;
	memcpy(msg.data,hash,RELAY_HASH_SIZE);

	m_socket_authentication.Send(reinterpret_cast<char*>(&msg),msg.size);

	// Validate authentication
	msg.size = RELAY_MSG_HEAD_SIZE;
	try
	{
		Log.Add(_MESSAGE_,"Receiving authentication reply...");
		m_socket_authentication.Receive(reinterpret_cast<char*>(&msg),msg.size);
		Log.Add(_MESSAGE_,"Authentication reply received");
	}
	catch(CStreamException&)
	{
		throw MCStreamException("Authentication failed");
	}
	if (msg.type != rmtAuthSuccessfull)
		throw MCStreamException("Relay authentication failed");
	Log.Add(_MESSAGE_,"Authentication was successful");

	// Starting port checking. Create thread and port for listening
	m_thread_listen_port.StartForCommunication(inPort);
	
	Log.Add(_MESSAGE_,"Send port number (%u) for checking by server...",exPort);
	msg.type = rmtCheckPort;
	msg.size = RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE;
	*(reinterpret_cast<int*>(msg.data)) = exPort;
	m_socket_authentication.Send(reinterpret_cast<char*>(&msg),msg.size);
	
	Log.Add(_MESSAGE_,"Waiting for server respond...");
	
	if ( !m_thread_listen_port.WaitForCommunication(NWL_INSTANCE.GetTimeoutPortListen()) )
		throw MCStreamException("Server doesn't respond");

	Log.Add(_MESSAGE_,"Receiving confirmation data...");
	m_socket_authentication.Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);

	if (!msg.size || msg.size != RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE)
		throw MCStreamException("Invalid message size");
	if (msg.type != rmtPortOpened)
		throw MCStreamException("Invalid message type");

	m_socket_authentication.Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);

	//Check received and sent data on an equality
	if ( static_cast<int>(exPort) != *(reinterpret_cast<int*>(msg.data)) )
		throw MCStreamException("Wrong answer from server side");
	
	Log.Add(_MESSAGE_,"All operation have done. The port is available...");
CATCH_THROW()
}
// COpenPortRequest [END] ////////////////////////////////////////////////////////////////

// CListenPortThread [BEGIN] /////////////////////////////////////////////////////////////

void CListenPortThread::Execute(void*)
{
TRY_CATCH
	m_badCommunicationSequence = true;

	if ( !m_socket.Bind(m_port) || !m_socket.Listen() )
	{
		SetEvent(m_hEvntDone.get());				// Unlock waiting for comunication
		SetEvent(m_hEvntBind.get());				// Unlock waiting for binding
		return;										// Proplems with socket binding
	}
	SetEvent(m_hEvntBind.get());					// Socket was binded

	boost::shared_ptr<CSSocket> acceptSocket(m_socket.Accept());	// Waiting for connect

	if (acceptSocket.get()!=NULL)
	{
		SRelayMessage msg;
		
		acceptSocket->Receive(reinterpret_cast<char*>(&msg), RELAY_MSG_HEAD_SIZE);

		if (	msg.size &&
				msg.size == RELAY_MSG_HEAD_SIZE + RELAY_PORTCHECK_SIZE &&
				msg.type == rmtCheckPort )
		{
			acceptSocket->Receive(msg.data,msg.size - RELAY_MSG_HEAD_SIZE);

			msg.type=rmtPortOpened;
			acceptSocket->Send(reinterpret_cast<char*>(&msg),msg.size);
			
			m_badCommunicationSequence = false;
		}
	}
	SetEvent(m_hEvntDone.get());
	
	return;
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

CListenPortThread::CListenPortThread()
	:	CThread()
{
TRY_CATCH
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

void CListenPortThread::StartForCommunication(unsigned short portNumber)
{
TRY_CATCH
	m_port = portNumber;
	m_socket.Create();
	
	m_hEvntDone.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	m_hEvntBind.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);

	Start();
	
	WaitForSingleObject(m_hEvntBind.get(),INFINITE);		//Wait for binding socket
	
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

bool CListenPortThread::WaitForCommunication(int timeout)
{
TRY_CATCH
	DWORD waitResult = WaitForSingleObject(m_hEvntDone.get(),timeout);
	
	m_socket.Close();			// Forced thread termination, if it still works
	Stop(false);				// Wait for thread termination

	return waitResult == WAIT_OBJECT_0 && !m_badCommunicationSequence;
CATCH_LOG()
	return false;
}
// CListenPortThread [END] ///////////////////////////////////////////////////////////////
