/////////////////////////////////////////////////////////////////////////
///
///  CFileAccessClient.h
///
///  Implementation of file transfer test
///
///  @remarks <TODO: insert remarks here>
///
///  @author Dmiry S. Golub @date 11/7/2006
///
////////////////////////////////////////////////////////////////////////

#if !defined(EA_3D02042A_62FF_49e2_9685_43CE3BA4FED1__INCLUDED_)
#define EA_3D02042A_62FF_49e2_9685_43CE3BA4FED1__INCLUDED_

#include <NWL/Streaming/CAbstractStream.h>
#include "filetransferdata.h"
#include "utils.h"
#include <AidLib/Strings/tstring.h>
typedef StatusInfo Status;
typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> auto_handle_t;


///  Implementation of file transfer test
class CFileAccessClient
{
public:
	/// .ctor
	CFileAccessClient(HANDLE hAbortEvent);
	/// .dtor
	virtual ~CFileAccessClient();

	/// Sets stream for remote operations
	/// @param stream				Stream for control remote file operations
	void SetStream(boost::shared_ptr<CAbstractStream> stream);

	///Attempt to delete a remote directory
	///  @param  remoteDirectory remote directory name
	///  @return ResponseCode
	ResponseCode DeleteDirectory(const tstring& remoteDirectory);
	///Attempt to delete a remote file
	///  @param  remoteFile remote file name
	ResponseCode DeleteFile(const tstring& remoteFile);
	/// Request a list of all drives.
	ResponseCode ListDrives(TDriveInfo& di);
	/// Request a list of files and directories at the specified path
	void ListFiles(const tstring& path,TFileInfo& fi);
	/// Attempt to rename/move remote directory
	///  @return ResponseCode
	ResponseCode RenameFile(const tstring& newName,const tstring& oldName);
	/// Attempt to retrieve a remote file and store it locally.
	///  @return ResponseCode
	ResponseCode ReceiveFile(const tstring& remoteFile, const tstring& localFile, bool = false);
	/// Attempt to send a local file to the File Access Server
	///  @return ResponseCode
	ResponseCode SendFile(const tstring& remotefile, const tstring& localFile , bool = false);
	/// Attempt to create new directory
	///  @return ResponseCode
	ResponseCode CreateDirectory(const tstring& remotefile);
	/// A virtual method used to notify the status of a file transfer process
	virtual void NotifyTransferStatus(Status& status);

	/// A virtual method used to notify then folder is being started to decompress
	virtual void NotifyUncompressStatus();

protected:
	/// tokanies drives string
	/// @see ListFiles
	void TokenizeDrives(TDriveInfo& di, unsigned int len, char* drives);
	void TokenizeInfo(TDriveInfo::value_type& di, char* info);
private:
	/// pointer to network stream
	boost::shared_ptr<CAbstractStream> m_pStream;
	/// status of the operation
	Status			m_curFileStatus;
	/// 
	HANDLE	m_hAbortEvent;
protected:
	/// check if it's a folder or file
	 BOOL CheckFolder(const tstring& fn,tstring&);
};
#endif // !defined(EA_3D02042A_62FF_49e2_9685_43CE3BA4FED1__INCLUDED_)
