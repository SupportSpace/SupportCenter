/////////////////////////////////////////////////////////////////////////
///
///  CFileAccessServer.h
///
///  CFileAccessServer class
///
///  @author Dmiry S. Golub @date 12/21/2006
///
////////////////////////////////////////////////////////////////////////

#if !defined(EA_A4B8B177_9588_445c_8A9E_772EC8D7EF01__INCLUDED_)
#define EA_A4B8B177_9588_445c_8A9E_772EC8D7EF01__INCLUDED_

#include <boost/shared_ptr.hpp>
#include <NWL/Streaming/CAbstractStream.h>
#include "filetransferdata.h"
#include <bitset>
#include "CFileTransferLog.h"

typedef StatusInfo Status;
typedef int Operation;
typedef std::bitset<8> Auth;


class CServerThread;

///  CFileAccessServer class
class CFileAccessServer
{
	CFileTransferLog m_transferLog;
public:
	/// .ctor
	CFileAccessServer(CAbstractStream* stream);
	/// .dtor
	virtual ~CFileAccessServer();

	/// Activity has changed delegate type
	typedef boost::function<void(const tstring&)> ActivityChangedHandler;

	/// Set new handler for activity changed event
	void SetActivityChangedHandler(ActivityChangedHandler activityChangedHandler);

	/// A virtual method that is called for every file access operation, before it executes.
	/// @param
	virtual ResponseCode NotifyOperation(BYTE status, TransferOpearation operation);
	///A virtual method used to notify the status of a file transfer process
	/// @param 
	/// - File details: file name, location, size.
	///	- Transfer details: send/receive, status (starting, in-progress, completed).
	///	- Progress: number of bytes transferred.
	void NotifyTransferStatus(Status status);
	///  Updates the authorization profile.  
	///  @param  Type specifies the operation referred to (e.g., get file, send file, rename, delete, etc.).
	///  @param  Type is either grant or revoke. 
	void SetAuthorization(const TransferOpearation& operation, bool auth);
	///  Start the file access session
	void Start();
	/// Stops an on-going file access session
	void Stop();
	friend CServerThread;

	/// Initializes transfer logging
	/// @param sid session id
	/// @param customerName - customer user name
	/// @param expertName - expert user name
	void InitTransferLogging(const tstring& sid, const tstring& customerName, const tstring& expertName);
private:
	/// network stream
	CAbstractStream *m_pStream;
	/// worker thread handle
	boost::shared_ptr<CServerThread> m_spWorkerThread;
	/// client permission
	Auth  m_userPermisiions;

	/// Invokes activity changed delegate
	/// @param description - activity description
	void NotifyActivity(const tstring description);

protected:
	///  Activity has changed delegate
	ActivityChangedHandler m_activityChangedHandler;
};
#endif // !defined(EA_A4B8B177_9588_445c_8A9E_772EC8D7EF01__INCLUDED_)
