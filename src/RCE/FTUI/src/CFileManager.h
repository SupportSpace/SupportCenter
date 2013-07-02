//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileManager.h
///
///  Declares CFileManager class
///  Performs file operations with local and remote system
///  
///  @author Alexander Novak @date 20.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAttributesFilter.h"
#include "CFileSystem.h"
#include "CLink.h"
#include <FTUI/FileTransfer/CFileAccessClient.h>
#include <boost/shared_ptr.hpp>
//========================================================================================================

typedef boost::function <unsigned int (	const CLink& path,
										const CFileData& fileData)> cbScanResultHandler_t;
typedef boost::function <void (const CLink& path)> cbScanCompletionHandler_t;
typedef boost::function <void (	const CLink& destinationItem,
								const CLink& sourceItem,
								const ULARGE_INTEGER totalSize,
								const ULARGE_INTEGER transferedSize)> cbTransferHandler_t;
typedef boost::function <void (const CLink& uncompressLink)> cbUncompressHandler_t;

typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> auto_handle_t;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileManager
	:	protected CFileSystem,
		protected CFileAccessClient
{
	FILEOP_FLAGS m_fileOperationFlags;
	CLink m_currentTransferDestinationLink;
	CLink m_currentTransferSourceLink;
	
	/// Returns drive types
	CDriveType m_getDriveType;
	
	/// Event for suspending or resuming transfer operations
	auto_handle_t m_transferSuspender;
		
	/// Stream for control the remote file operations
	boost::shared_ptr<CAbstractStream> m_stream;

	/// Filtering files and directories during a scan process
	boost::shared_ptr<CAttributesFilter> m_attributesFilter;

	/// Event for cancel transfer operation with remote host
	HANDLE			m_rawEventHandle;
	auto_handle_t	m_abortTransferEvent;

	/// Variable for cancel scan operation
	volatile LONG m_abortScanEvent;

	/// Handler for scan operations
	cbScanResultHandler_t m_scanHandler;

	/// Handler for scan completion operations
	cbScanCompletionHandler_t m_scanCompletionHandler;

	/// Handler for transfer operations
	cbTransferHandler_t m_transferHandler;
	
	/// Handler for uncompress operations
	cbUncompressHandler_t m_uncompressHandler;
	
protected:
	/// Notification method for transfer operations
	/// @param status				Status of the transfer progress
	virtual void NotifyTransferStatus(Status& status);

	/// Notification method for uncompressing operations
	virtual void NotifyUncompressStatus();
	
	/// Notification method for scan operations
	/// @param path					Current directory
	/// @param fileData				Current file
	/// @param userData				User defined data
	/// @return				For stopping the scan process zero must be returned
	virtual unsigned int OnFindItem(const tstring& path, const CFileData& fileData, const void* userData);

	/// Notification method for scan completion	operations
	/// @param path					Requested path for scan operations
	/// @param userData				User defined data
	virtual void OnFindComplete(const tstring& path, const void* userData);
public:
	/// Creates file manager object
	/// @param stream				Stream for control remote file operations
	/// @param showOperateProgress	Show progress and confirmation dialog for local file operations
	CFileManager(const bool showOperateProgress = true);

	virtual ~CFileManager();
	
	/// Sets stream for remote operations
	/// @param stream				Stream for control remote file operations
	void SetRemoteStream(boost::shared_ptr<CAbstractStream> stream);
	
	/// Sets filter for the file scan operations
	/// @param attributesFilter		Filter object for scanning
	void SetAttributesFilter(boost::shared_ptr<CAttributesFilter> attributesFilter);

	/// Creates directory on local or remote host
	/// @param directoryName		Directory name
	void CreateDirectory(const CLink& directoryName);

	/// Copies file or directory between local and remote hosts, or does this operation one host
	/// @param destinationItem		Destination path
	/// @param sourceItem			Source path
	/// @param overwriteFile		Overwrite file if it exists
	/// @remarks			Copy operation on remote host doesn't provided by CFileAccessClient
	///						Bad current issue it's receive and then sending file
	void CopyItem(const CLink& destinationItem, const CLink& sourceItem, bool overwriteFile = false);

	/// Rename file or directory on local or remote host
	/// @param newItemName			New name
	/// @param oldItemName			Old name
	/// @remarks			Rename operation on remote host available only for single file or
	///						directory on same volume
	///						//TODO: Fix this in future on CoFAHost. Propose issue is change MoveFile to SHFileOperation
	void RenameItem(const CLink& newItemName, const CLink& oldItemName);

	/// Moves file or directory between local and remote hosts, or does this operation on one host
	/// @param destinationItem		Destination path
	/// @param sourceItem			Source path
	/// @param overwriteFile		Overwrite file if it exists
	void MoveItem(const CLink& destinationItem, const CLink& sourceItem, bool overwriteFile = false);

	/// Deletes file or directory on local or remote host
	/// @param targetItem			Path to deletion
	void DeleteItem(const CLink& targetItem);

	/// Browse for files and directories
	/// @param path					Path for browsing
	/// @remarks			Doesn't browse in sub directories
	///						Calls the OnFindItem method
	void ScanItems(const CLink& path);

	/// Cancels scan operation
	void CancelScanOperation();

	/// Cancel transfer or current suspended operation
	void CancelTransferOperation();

	/// Returns true if current transfer operation is canceled
	bool TransferOperationCanceled();

	/// Suspend transfer operation
	void SuspendTransferOperation();

	/// Resume suspended transfer operation
	void ResumeTransferOperation();

	/// Sets handler for scan operation
	/// @param handler				Callback handler for scan operations
	void SetScanHandler(cbScanResultHandler_t handler);

	/// Sets handler for scan completion operation
	/// @param handler				Callback handler for scan completion operations
	void SetScanCompletionHandler(cbScanCompletionHandler_t handler);

	/// Sets handler for transfer operation
	/// @param handler				Callback handler for transfer operations
	void SetTransferHandler(cbTransferHandler_t handler);

	/// Sets handler for uncompress operation
	/// @param handler				Callback handler for uncompress operations
	void SetUncompressHandler(cbUncompressHandler_t handler);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
