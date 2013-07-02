//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CAsyncFileManager.h
///
///  Declares CAsyncFileManager class
///  Performs asynchronous file operations with local and remote system
///  
///  @author Alexander Novak @date 29.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CFileManager.h"
#include <AidLib/CThread/CThread.h>
#include <boost/bind.hpp>
#include <list>
//========================================================================================================

enum EManageOperation
{
	op_Create,
	op_Rename,
	op_Delete
};

#define AFM_ERR_OVERWRITE_ACCEPTED		0x80000001
#define AFM_ERR_OVERWRITE_DECLINED		0x80000002

typedef boost::function <void (	const CLink& path,
								const CFileData& fileData)> cbScanHandler_t;
typedef boost::function <void (	const CLink& directoryName,
								const DWORD winErr,
								const DWORD internalErr)> cbCreateHandler_t;
typedef boost::function <void (	const CLink& newItemName,
								const CLink& oldItemName,
								const DWORD winErr,
								const DWORD internalErr)> cbRenameHandler_t;
typedef boost::function <void (	const CLink& targetItem,
								const DWORD winErr,
								const DWORD internalErr)> cbDeleteHandler_t;
typedef boost::function <void (	const CLink& destinationItem,
								const CLink& sourceItem,
								const CFileData& fileData,
								const bool copyOperation,
								const DWORD winErr,
								const DWORD internalErr)> cbTransferCompletionHandler_t;
typedef boost::function <void (	const CFileData& fileData)> cbTransferCompressingHandler_t;
typedef boost::function <void (	EManageOperation operation,
								const CLink& newItemName,
								const CLink& oldItemName,
								DWORD winErrCode,
								DWORD internalErrCode)> cbThunkManageHandler_t;
typedef boost::function <bool (	const CLink& destinationItem,
								const CLink& sourceItem,
								const CFileData& fileData,
								const bool copyOperation)> cbAskForOverwriteHandler_t;
typedef boost::function <void (	const CLink& destinationItem,
								const CLink& sourceItem,
								const CFileData& fileData)> cbTransferMoveDeleteHandler_t;
typedef std::vector<CLink> link_vec_t;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration for AsyncFileManager operations
class CScanningThread;
class CManagingThread;
class CTransferQueuedThread;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAsyncFileManager
{
	boost::shared_ptr<CFileManager> m_fileManager;

	boost::shared_ptr<CTransferQueuedThread> m_localTransferThread;
	boost::shared_ptr<CTransferQueuedThread> m_remoteTransferThread;
	boost::shared_ptr<CManagingThread> m_localManagingThread;
	boost::shared_ptr<CManagingThread> m_remoteManagingThread;
	boost::shared_ptr<CScanningThread> m_localScanningThread;
	boost::shared_ptr<CScanningThread> m_remoteScanningThread;

	cbScanHandler_t					m_scanHandler;
	cbCreateHandler_t				m_createHandler;
	cbRenameHandler_t				m_renameHandler;
	cbDeleteHandler_t				m_deleteHandler;
	cbDeleteHandler_t				m_preDeleteHandler;
	cbTransferCompletionHandler_t	m_transferCompletionHandler;
	cbAskForOverwriteHandler_t		m_askForOverwriteHandler;
	cbTransferCompressingHandler_t	m_transferCompressingHandler;
	cbTransferMoveDeleteHandler_t	m_transferMoveDeleteHandler;
	
	/// Variable for cancel local scan operation
	volatile LONG m_abortLocalScanning;

	/// Variable for cancel remote scan operation
	volatile LONG m_abortRemoteScanning;

	/// Thunk callback method
	unsigned int ScanHandlerThunk(const CLink& path, const CFileData& fileData);

	/// Thunk callback method for managing operations
	void ManageHandlerThunk(EManageOperation operation,
							const CLink& newItemName,
							const CLink& oldItemName,
							DWORD winErrCode,
							DWORD internalErrCode);
										
	/// Thunk method for transfer callbacks
	void TransferCompletionHandlerThunk(	const CLink& destinationItem,
											const CLink& sourceItem,
											const CFileData& fileData,
											const bool copyOperation,
											DWORD winErrCode,
											DWORD internalErrCode);

	/// Thunk method for user interaction
	bool AskForOverwriteHandlerThunk(	const CLink& destinationItem,
										const CLink& sourceItem,
										const CFileData& fileData,
										const bool copyOperation);

	/// Thunk method status messages
	void TransferCompressingHandlerThunk(const CFileData& fileData);

	/// Thunk method before delete operation during move processing
	void TransferMoveDeleteHandlerThunk(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData);
public:
	/// Creates file manager object	with threads for asynchronous operations
	/// @param showOperateProgress	Show progress and confirmation dialog for local file operations
	CAsyncFileManager(bool showOperateProgress = true);

	virtual ~CAsyncFileManager();
	
	/// Sets stream for remote operations
	/// @param stream				Stream for control remote file operations
	void SetRemoteOperationsStream(boost::shared_ptr<CAbstractStream> stream);

	/// Sets filter for the file scan operations
	/// @param attributesFilter		Filter object for scanning
	void SetScanAttributesFilter(boost::shared_ptr<CAttributesFilter> attributesFilter);

	/// Copies file or directory between local and remote hosts, or does this operation one host
	/// @param destinationItem		Destination path
	/// @param sourceItem			Source path
	/// @param fileData				File size, attributes and so on
	void CopyItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData);

	/// Moves file or directory between local and remote hosts, or does this operation on one host
	/// @param destinationItem		Destination path
	/// @param sourceItem			Source path
	/// @param fileData				File size, attributes and so on
	void MoveItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData);

	/// Sets handler for transfer operation
	/// @param handler				Callback handler for transfer operations
	void SetTransferHandler(cbTransferHandler_t handler);

	/// Sets handler for uncompress operation
	/// @param handler				Callback handler for uncompress operations
	void SetUncompressHandler(cbUncompressHandler_t handler);

	/// Sets handler for transfer completion operation
	/// @param handler				Callback handler for transfer completion operation
	void SetTransferCompletionHandler(cbTransferCompletionHandler_t handler);

	/// Sets handler for move transfer operation, calls before deletion and after copy
	/// @param handler				Callback handler for move operation
	void SetTransferMoveDeleteHandler(cbTransferMoveDeleteHandler_t handler);

	/// Cancel transfer operation
	void CancelTransferOperation();

	/// Cancel all queued transfer operation
	void CancelAllTransferOperations();

	/// Return true if current transfer operation is canceled
	bool TransferOperationCanceled();

	/// Suspend transfer operation
	void SuspendTransferOperation();

	/// Resume suspended transfer operation
	void ResumeTransferOperation();

	/// Creates directory on local or remote host
	/// @param directoryName		Directory name
	void CreateDirectory(const CLink& directoryName);

	/// Sets handler for create operation
	/// @param handler				Callback handler for create operations
	void SetCreateOperationHandler(cbCreateHandler_t handler);

	/// Rename file or directory on local or remote host
	/// @param newItemName			New name
	/// @param oldItemName			Old name
	void RenameItem(const CLink& newItemName, const CLink& oldItemName);

	/// Sets handler for rename operation
	/// @param handler				Callback handler for rename operations
	void SetRenameOperationHandler(cbRenameHandler_t handler);

	/// Deletes file or directory on local or remote host
	/// @param targetItems			Paths to deletion
	void DeleteItem(boost::shared_ptr<link_vec_t> targetItems);
	
	/// Sets handler for delete operation
	/// @param handler				Callback handler for delete operations
	void SetDeleteOperationHandler(cbDeleteHandler_t handler);

	/// Browse for files and directories
	/// @param path					Path for browsing
	/// @remarks			Doesn't browse in sub directories
	///						Calls the ScanHandler callback function
	void ScanItems(const CLink& path);

	/// Sets handler for scan operation
	/// @param handler				Callback handler for scan operations
	void SetScanHandler(cbScanHandler_t handler);

	/// Sets handler for scan completion operation
	/// @param handler				Callback handler for scan operations
	void SetScanCompletionHandler(cbScanCompletionHandler_t handler);

	/// Cancels scan operation
	/// @param path					Cancel scanning for path on local, or remote host
	/// @remarks			path used only for determination local or remote host
	void CancelScanOperation(const CLink& path);

	/// Sets handler for user interation
	/// @param handler				Callback handler for confirm overwrite operation
	void SetAskForOverwriteHandler(cbAskForOverwriteHandler_t handler);

	/// Sets handler for status messages during folder is compressing
	void SetTransferCompressingHandler(cbTransferCompressingHandler_t handler);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///  Declares CScanningThread class
///  Performs asynchronous browsing operations
class CScanningThread
	:	public CThread
{
	boost::shared_ptr<CFileManager>	m_fileManager;
	cbScanResultHandler_t m_scanHandler;

	CCritSectionSimpleObject m_scanningGuard;
	CLink m_scanPath;
	virtual void Execute(void *Params);
public:
	CScanningThread(boost::shared_ptr<CFileManager> fileManager);
	void ScanItems(const CLink& path);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///  Declares CManagingThread class
///  Performs asynchronous renaming, deleting and creating operations
class CManagingThread
	:	public CThread
{
	boost::shared_ptr<CFileManager>	m_fileManager;
	cbThunkManageHandler_t m_manageHandler;

	CCritSectionSimpleObject m_managingGuard;
	EManageOperation m_manageOperation;
	CLink m_link1;
	CLink m_link2;
	boost::shared_ptr<link_vec_t> m_linksToDelete;
	virtual void Execute(void *Params);
public:
	CManagingThread(boost::shared_ptr<CFileManager> fileManager, cbThunkManageHandler_t manageHandler);
	void CreateDirectory(const CLink& directoryName);
	void RenameItem(const CLink& newItemName, const CLink& oldItemName);
	void DeleteItem(boost::shared_ptr<link_vec_t> targetItems);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

///  Declares CTransferThread class
///  Performs asynchronous file transfer operations
class CTransferQueuedThread
	:	public CThread
{
	struct SListItem
	{
		CLink m_destinationItem;
		CLink m_sourceItem;
		CFileData m_fileData;
		bool m_moveOperation;
		SListItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData, bool moveOperation)
			:	m_destinationItem(destinationItem),
				m_sourceItem(sourceItem),
				m_fileData(fileData),
				m_moveOperation(moveOperation){}
	};
	auto_handle_t m_transferAdded;
	
	volatile bool m_finishTransfer;

	boost::shared_ptr<CFileManager>	m_fileManager;
	cbTransferCompletionHandler_t m_transferCompletionHandler;
	cbTransferMoveDeleteHandler_t m_transferMoveDeleteHandler;
	cbTransferCompressingHandler_t m_transferCompressingHandler;
	cbAskForOverwriteHandler_t m_askForOverwriteHandler;

	CLink m_destinationLink;
	CLink m_sourceLink;
	CCritSectionSimpleObject m_transferGuard;
	std::list<SListItem> m_listTransfer;
	virtual void Execute(void *Params);
public:
	CTransferQueuedThread(	boost::shared_ptr<CFileManager> fileManager,
							cbTransferCompletionHandler_t transferCompletionHandler,
							cbAskForOverwriteHandler_t askForOverwriteHandler,
							cbTransferCompressingHandler_t transferCompressingHandler,
							cbTransferMoveDeleteHandler_t transferMoveDeleteHandler);
	~CTransferQueuedThread();
	void CopyItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData);
	void MoveItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData);
	void ClearTransferQueue();
	void StopTransferThread();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
