/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CIMStub.h
///
///  Instant messaging stub
///
///  @author "Archer Software" Sogin M. @date 22.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include <NWL/Streaming/CSocketStream.h>
#include <NWL/NetworkLayer.h>
#include <AidLib/Strings/tstring.h>
#include <windows.h>

/// IM messages codes
typedef enum _EIMtype
{
	IMT_AddMessage 		= 1,
	IMT_GetMessage		= 2,
	IMT_ClearMessages	= 3
} EIMtype;

#define MAX_MESSAGE_SIZE 4096
#define MAX_PEERID_SIZE 255

#pragma pack (push)
#pragma pack (1)
typedef struct _sIMessage
{
	/// overall message size
	unsigned int size;
	/// message code
	EIMtype code;
	/// peedId
	char peerID[MAX_PEERID_SIZE];
	/// message body
	char data[1];
} sIMessage;
#pragma pack(pop)

#define IMESSAGE_HEADSIZE (sizeof(sIMessage) - 1)


/// Instant messaging stub
class NWL_API CIMStub
{
	tstring m_peerID;
	HANDLE m_hEvent;
public:
	/// Init class object
	CIMStub(const tstring &peerId);
	virtual ~CIMStub(void);

	/// Sends a message to the specified destination peer (user ID).  
	/// @param messageData Message data is an arbitrary string.
	void SendMsg(const tstring& messageData);

	/// Handles an incoming message from a specified source peer (user ID).
	/// @param messageData Message data is an arbitrary string.
	void HandleMsg(tstring& messageData);

	/// Returns incoming message event handle
	HANDLE GetMsgEvent();

	/// Removes all my messages from server
	void RemoveAllMyMessagesFromServer();

	/// Resets server. This cause all pending messages to be removed from server
	void ResetServer();
};
