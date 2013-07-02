/////////////////////////////////////////////////////////////////////////
///
///  CFAProxy.h
///
///  proxy object for Active Object pattern
///
///  @author Dmiry S. Golub @date 2/6/2007
///
////////////////////////////////////////////////////////////////////////
#if !defined(EA_5CC55C80_7BCA_4bd1_ACB9_6716AA2E92D6__INCLUDED_)
#define EA_5CC55C80_7BCA_4bd1_ACB9_6716AA2E92D6__INCLUDED_

#include <windows.h>
#include <FTUI/FileTransfer/filetransferdata.h>
#include "CScheduler.h"

class CFAManager;
class CAbstractStream;

/// proxy object class
class CFAProxy
{
	/// event which stops Active Queue
	SPHandle m_hEvent;
	/// Scheduler object
	CScheduler m_scheduler;
	/// pointer to servant object
	CFileAccessClient* m_pFAClient;
	/// pointer to manager object 
	CFAManager* m_pManager;
public:
	/// .ctor
	CFAProxy(CFileAccessClient*,SPHandle);
	/// .dtor
	virtual ~CFAProxy();
	/// \sa CFileAccessClient::CreateDirectory()
	void CreateDirectory(const tstring& fn);
	/// \sa CFileAccessClient::DeleteDirectory()
	void DeleteDirectory(const tstring& fn);
	/// \sa CFileAccessClient::DeleteFile()
	void DeleteFile(const tstring& fn);
	/// \sa CFileAccessClient::ListDrives()
	void ListDrives();
	/// \sa CFileAccessClient::ListFiles()
	void ListFiles(const tstring& path);
	/// \sa CFileAccessClient::RenameFile()
	void RenameFile(const tstring& oldName, const tstring& newName);
	/// \sa CFileAccessClient::RetrieveFile()
	void RetrieveFile(const tstring& remoteFile, const tstring& localFile, bool permission= false);
	/// \sa CFileAccessClient::SendFile()
	void SendFile(const tstring& remoteFile, const tstring& localFile, bool permission= false);
	/// sets manager class
	void SetManager( CFAManager* pm)
	{
		m_pManager=pm;
	}
};
#endif // !defined(EA_5CC55C80_7BCA_4bd1_ACB9_6716AA2E92D6__INCLUDED_)
