
#include <iostream>
#include <string>
#include <vector>

#include <Windows.h>

#include "gloox.h"
#include "client.h"
#include "connectionlistener.h"
#include "presencehandler.h"
#include "messagesessionhandler.h"
#include "messagesession.h"
#include "disco.h"

using namespace gloox;
using namespace std;

std::string mesasage_subject;

class IMClient : public ConnectionListener, MessageHandler, LogHandler, MessageSessionHandler
{
private:
	Client *client;
	MessageSession *m_session;
	JID *m_session_to;
	std::string m_server;
public:
	IMClient(std::string username, std::string password, std::string server, std::string server_addr);
	void connect();
	void handle_input(std::string &input);
	ConnectionError idle(int timeout);
	virtual void handleMessageSession(MessageSession *session );
	virtual void handleMessage(Stanza *stanza );
	virtual void handleLog(LogLevel level, LogArea area, const std::string& message);
	virtual void onConnect();
	virtual void onDisconnect(ConnectionError e);
	virtual bool onTLSConnect( const CertInfo& info );
};


IMClient::IMClient(std::string username, std::string password, 
				   std::string server, std::string server_addr)
				   : m_session(NULL), m_session_to(NULL)
{
	JID jid;

	jid.setUsername(username);
	jid.setResource("IMClient");
	jid.setServer(server);
	m_server = server;

	client = new Client(jid, password);
	client->registerConnectionListener( this );
	client->setAutoPresence(true);
	//client->setAutoMessageSession(true, this);//Was NOT commented provide session mehanism
	client->setInitialPriority(4);
	client->disco()->setVersion( "messageTest", GLOOX_VERSION, "Linux" );
	client->disco()->setIdentity( "client", "bot" );

	client->registerMessageHandler( this ); //was commented
	//client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);//was commented

	client->setServer(server_addr);
	client->setTls(false);
}

void IMClient::connect()
{
	try {
		client->connect(false);//False for Unblocked
	} catch (std::exception& e) {
		cout << "connect failed: " << e.what() << endl;
	}
}

void IMClient::onConnect()
{
	cout << "=== Connected to server." << endl;
}

void IMClient::onDisconnect(ConnectionError e)
{
	cout << "=== Connection closed; reason: " << e << endl;
}

bool IMClient::onTLSConnect( const CertInfo& info )
{
	cout << "=== TLS connection established." << endl;
	cout << "===	Server Certificate: " << info.server << endl;
	cout << "===	Certificate Issuer: " << info.issuer << endl;
	cout << "===	Protocol details: " << info.protocol << ", " << info.cipher << ", " << info.mac << endl;
	return true;
}

void IMClient::handleMessageSession(MessageSession *session )
{
	if ( m_session )//Close exists session - not good
	{
		delete m_session;
	}
	if ( m_session_to ) 
	{
		delete m_session_to;
		m_session_to = 0;
	}

	m_session = session;
	session->registerMessageHandler(this);

	cout << "New chat session form " << session->target().full() << endl;
	m_session = session;
}
//
//	http://camaya.net/glooxdoc Example
//  
void IMClient::handleMessage( Stanza *stanza )
{
	cout << "Blocked handleMessage started"  << endl;
	client->disconnect();
	
	if (stanza->error() == StanzaErrorUndefined)
	{
		cout << "Message from [" << stanza->from().full() << 
		"]: "  << stanza->body() <<  "Subject [" << stanza->subject() << "]: " << std::endl;

		cout << "Attribute UUID:" << stanza->findAttribute("UUID")  << endl;
		cout << "Attribute userId:" << stanza->findAttribute("userId")  << endl;
		cout << "XML:" << stanza->xml() << endl;
		
		gloox::Tag::TagList   tagList = stanza->children();
		size_t size = tagList.size();

		//StringMap* map = stanza->attributes();

		Tag* tagReceived = stanza->findChild("UserCreatedFlags");
		std::string attValue;
		
		if( tagReceived != NULL )
		{
			attValue = tagReceived->findAttribute( "test" );
			cout << "FoundChildStanza and attribute "  << attValue << endl;
		}
	}
	else
	{
		cout << "Message delivery failed: (" << stanza->error() << ") " << stanza->errorText() << endl;
	}

	cout << "handleMessage ended"  << endl;
	Stanza *parentStanza = Stanza::createMessageStanza( stanza->from().full(), "hello world" );
	
	parentStanza->addAttribute("UUID", "Anatoly");
	parentStanza->addAttribute("userId", "Gutnick");
	
	Tag*	testTag1 = new Tag("UserCreatedFlags");
	testTag1->addAttribute("test", "STAM TEST");

	Tag*	childOfTag1 = new Tag("Flag", "Some data");

	childOfTag1->addAttribute("Reply11","Reply11 Value");
	childOfTag1->addAttribute("Reply12","Reply12 Value");
	childOfTag1->addAttribute("Reply13","Reply13 Value");

	Tag*	childOfTag2 = new Tag("Flag");

	childOfTag2->addAttribute("Flag21","Reply11 Value");
	childOfTag2->addAttribute("Flag22","Reply12 Value");
	childOfTag2->addAttribute("Flag23","Reply13 Value");

	testTag1->addChild(childOfTag1);
	testTag1->addChild(childOfTag2);

	parentStanza->addChild(testTag1);

    client->send( parentStanza );
}

ConnectionError IMClient::idle(int timeout)
{
	return client->recv(timeout);
}

void IMClient::handle_input(std::string &input)
{
	// New session?
	if (input[0] == '.') {
		if (m_session) {
			delete m_session;
			m_session = NULL;
		}
		if (m_session_to) {
			delete m_session_to;
			m_session_to = NULL;
		}
		cout << "Session closed." << endl;
		return;
	}

	if (input[0] == '>') {
		if (m_session) {
			delete m_session;
			m_session = NULL;
		}
		if (m_session_to) {
			delete m_session_to;
			m_session_to = NULL;
		}
		std::string addr = input.substr(1, input.length()-1) + "@" + m_server;
		m_session_to = new JID(addr);

		try {
			m_session = new MessageSession(client, *m_session_to);
		} catch (std::exception &e) {
			cout << "Failed to initiate session.";
			return;
		}

		m_session->registerMessageHandler(this);
		cout << "Established message session with " << addr << endl;
		return;
	}

	if (!m_session) {
		cout << "No active session; use >[username] to create session." << endl;
		return;
	}
	try {
		m_session->send(input, mesasage_subject);
		cout << "Message sent." << endl;
	} catch (std::exception& e) {
		cout << "Message could not be delivered: " << client->streamError() << endl;
	}
}

void IMClient::handleLog(LogLevel level, LogArea area, const std::string& message)
{
	cout << message << endl;
}

void init_winsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		cout << "WSAStartup failed" << endl;
		exit(1);
	}
}

vector<std::string> input;
CRITICAL_SECTION input_lock;

DWORD WINAPI input_thread(LPVOID param)
{
	IMClient *client = reinterpret_cast<IMClient *>(param);

	while (!cin.eof()) {
		std::string line;
		std::getline(cin, line);
	
		EnterCriticalSection(&input_lock);
		input.push_back(line);
		LeaveCriticalSection(&input_lock);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	init_winsock();
	InitializeCriticalSection(&input_lock);

	std::string username;
	std::string server;
	std::string password;
	std::string server_addr;

	cout << "Username [u1]\t\t: ";
	std::getline(cin, username);
	if (username.length() == 0)
		username = "u1";

	cout << "Server [supportspace.com]\t: ";
	std::getline(cin, server);
	if (server.length() == 0)
		server = "supportspace.com";

	cout << "Password [1234]\t\t\t: ";
	std::getline(cin, password);
	if (password.length()   == 0)
		password = "1234";

	cout << "Server Address [192.168.146.129]\t: ";
	std::getline(cin, server_addr);
	if (server_addr.length() == 0)
		server_addr = "192.168.146.129";
	
	cout << "Message Subject [new]\t: ";
	std::getline(cin, mesasage_subject);
	if (mesasage_subject.length() == 0)
		mesasage_subject = "new";

	IMClient client(username, password, server, server_addr);

	// Start input thread
	DWORD input_thread_id;
	HANDLE input_thread_handle = CreateThread(NULL, 64*1024, input_thread, &client, 0, &input_thread_id);
	if (!input_thread_handle) {
		cout << "failed to create input thread." << endl;
		exit(0);
	}

	cout << "=== Connecting to server..." << endl;
	client.connect();
	
	while (client.idle(1000) == ConnNoError) {
		EnterCriticalSection(&input_lock);
		while (input.size() > 0) {
			std::string line = input.back();
			input.pop_back();
			client.handle_input(line);
		}
		LeaveCriticalSection(&input_lock);
	}
	cout << "=== Client terminated" << endl;
	cout << "=== Connect again" << endl;
	client.connect();
}