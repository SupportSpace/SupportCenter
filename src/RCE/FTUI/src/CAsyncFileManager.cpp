//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CAsyncFileManager.cpp
///
///  Implements CAsyncFileManager class
///  Performs asynchronous file operations with local and remote system
///  
///  @author Alexander Novak @date 30.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <atlbase.h>
#include "CAsyncFileManager.h"
#include "CSkinnedMsgBox.h"

// CAsyncFileManager [BEGIN] /////////////////////////////////////////////////////////////////////////////

unsigned int CAsyncFileManager::ScanHandlerThunk(const CLink& path, const CFileData& fileData)
{
TRY_CATCH

	if ( m_scanHandler )
		m_scanHandler(path, fileData);

	if ( path.IsLocal() )
		return !m_abortLocalScanning;
	else
		return !m_abortRemoteScanning;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::ManageHandlerThunk(	EManageOperation operation,
											const CLink& newItemName,
											const CLink& oldItemName,
											DWORD winErrCode,
											DWORD internalErrCode)
{
TRY_CATCH

	switch ( operation )
	{
		case op_Create:
			if ( m_createHandler )
				m_createHandler(newItemName,winErrCode,internalErrCode);
			break;
		case op_Rename:
			if ( m_renameHandler )
				m_renameHandler(newItemName,oldItemName,winErrCode,internalErrCode);
			break;
		case op_Delete:
			if ( m_deleteHandler )
				m_deleteHandler(newItemName,winErrCode,internalErrCode);
			break;
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::TransferCompletionHandlerThunk(	const CLink& destinationItem,
														const CLink& sourceItem,
														const CFileData& fileData,
														const bool copyOperation,
														DWORD winErrCode,
														DWORD internalErrCode)
{
TRY_CATCH

	if ( m_transferCompletionHandler )
		m_transferCompletionHandler(destinationItem,sourceItem,fileData,copyOperation,winErrCode,internalErrCode);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CAsyncFileManager::AskForOverwriteHandlerThunk(const CLink& destinationItem,
													const CLink& sourceItem,
													const CFileData& fileData,
													const bool copyOperation)
{
TRY_CATCH

	if ( m_askForOverwriteHandler )
		return m_askForOverwriteHandler(destinationItem,sourceItem,fileData,copyOperation);
	
	return false;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::TransferCompressingHandlerThunk(const CFileData& fileData)
{
TRY_CATCH

	if ( m_transferCompressingHandler )
		m_transferCompressingHandler(fileData);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::TransferMoveDeleteHandlerThunk(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData)
{
TRY_CATCH

	if ( m_transferMoveDeleteHandler )
		m_transferMoveDeleteHandler(destinationItem,sourceItem,fileData);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CAsyncFileManager::CAsyncFileManager(bool showOperateProgress)
	:	m_scanHandler(NULL),
		m_createHandler(NULL),
		m_renameHandler(NULL),
		m_deleteHandler(NULL),
		m_preDeleteHandler(NULL),
		m_transferCompletionHandler(NULL),
		m_transferMoveDeleteHandler(NULL),
		m_askForOverwriteHandler(NULL),
		m_transferCompressingHandler(NULL),
		m_abortLocalScanning(FALSE),
		m_abortRemoteScanning(FALSE)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CAsyncFileManager::CAsyncFileManager"));
	
	m_fileManager.reset(new CFileManager(showOperateProgress));
	m_fileManager->SetScanHandler(boost::bind(&CAsyncFileManager::ScanHandlerThunk,this,_1,_2));

	m_localTransferThread.reset(new CTransferQueuedThread(	m_fileManager,
															boost::bind(&CAsyncFileManager::TransferCompletionHandlerThunk,this,_1,_2,_3,_4,_5,_6),
															boost::bind(&CAsyncFileManager::AskForOverwriteHandlerThunk,this,_1,_2,_3,_4),
															boost::bind(&CAsyncFileManager::TransferCompressingHandlerThunk,this,_1),
															boost::bind(&CAsyncFileManager::TransferMoveDeleteHandlerThunk,this,_1,_2,_3)));
	m_remoteTransferThread.reset(new CTransferQueuedThread(	m_fileManager,
															boost::bind(&CAsyncFileManager::TransferCompletionHandlerThunk,this,_1,_2,_3,_4,_5,_6),
															boost::bind(&CAsyncFileManager::AskForOverwriteHandlerThunk,this,_1,_2,_3,_4),
															boost::bind(&CAsyncFileManager::TransferCompressingHandlerThunk,this,_1),
															boost::bind(&CAsyncFileManager::TransferMoveDeleteHandlerThunk,this,_1,_2,_3)));
	m_localManagingThread.reset(new CManagingThread(	m_fileManager,
														boost::bind(&CAsyncFileManager::ManageHandlerThunk,this,_1,_2,_3,_4,_5)));
	m_remoteManagingThread.reset(new CManagingThread(	m_fileManager,
														boost::bind(&CAsyncFileManager::ManageHandlerThunk,this,_1,_2,_3,_4,_5)));
	m_localScanningThread.reset(new CScanningThread(m_fileManager));
	m_remoteScanningThread.reset(new CScanningThread(m_fileManager));

	Log.Add(_MESSAGE_,_T("END CAsyncFileManager::CAsyncFileManager"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CAsyncFileManager::~CAsyncFileManager()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CAsyncFileManager::~CAsyncFileManager"));

	CancelAllTransferOperations();

	m_localTransferThread->StopTransferThread();
	m_remoteTransferThread->StopTransferThread();
	AtlWaitWithMessageLoop(m_localTransferThread->hTerminatedEvent.get());
	AtlWaitWithMessageLoop(m_remoteTransferThread->hTerminatedEvent.get());

	InterlockedExchange(&m_abortLocalScanning,TRUE);
	AtlWaitWithMessageLoop(m_localScanningThread->hTerminatedEvent.get());
	
	InterlockedExchange(&m_abortRemoteScanning,TRUE);
	AtlWaitWithMessageLoop(m_remoteScanningThread->hTerminatedEvent.get());

	m_localManagingThread->Stop(false);
	m_remoteManagingThread->Stop(false);

	Log.Add(_MESSAGE_,_T("END CAsyncFileManager::~CAsyncFileManager"));

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetRemoteOperationsStream(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH

	m_fileManager->SetRemoteStream(stream);

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetScanAttributesFilter(boost::shared_ptr<CAttributesFilter> attributesFilter)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CAsyncFileManager::SetScanAttributesFilter"));

	InterlockedExchange(&m_abortLocalScanning,TRUE);
	InterlockedExchange(&m_abortRemoteScanning,TRUE);
	AtlWaitWithMessageLoop(m_localScanningThread->hTerminatedEvent.get());
	AtlWaitWithMessageLoop(m_remoteScanningThread->hTerminatedEvent.get());

	m_fileManager->SetAttributesFilter(attributesFilter);

	InterlockedExchange(&m_abortLocalScanning,FALSE);
	InterlockedExchange(&m_abortRemoteScanning,FALSE);

	Log.Add(_MESSAGE_,_T("END CAsyncFileManager::SetScanAttributesFilter"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::CopyItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData)
{
TRY_CATCH

	if ( destinationItem.IsLocal() && sourceItem.IsLocal() )
		m_localTransferThread->CopyItem(destinationItem,sourceItem,fileData);
	else
		m_remoteTransferThread->CopyItem(destinationItem,sourceItem,fileData);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::MoveItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData)
{
TRY_CATCH

	if ( destinationItem.IsLocal() && sourceItem.IsLocal() )
		m_localTransferThread->MoveItem(destinationItem,sourceItem,fileData);
	else
		m_remoteTransferThread->MoveItem(destinationItem,sourceItem,fileData);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetTransferHandler(cbTransferHandler_t handler)
{
TRY_CATCH

	m_fileManager->SetTransferHandler(handler);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetUncompressHandler(cbUncompressHandler_t handler)
{
TRY_CATCH

	m_fileManager->SetUncompressHandler(handler);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetTransferCompletionHandler(cbTransferCompletionHandler_t handler)
{
TRY_CATCH

	m_transferCompletionHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetTransferMoveDeleteHandler(cbTransferMoveDeleteHandler_t handler)
{
TRY_CATCH

	m_transferMoveDeleteHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::CancelTransferOperation()
{
TRY_CATCH

	m_fileManager->CancelTransferOperation();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::CancelAllTransferOperations()
{
TRY_CATCH

	m_remoteTransferThread->ClearTransferQueue();
	m_fileManager->CancelTransferOperation();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CAsyncFileManager::TransferOperationCanceled()
{
TRY_CATCH

	return m_fileManager->TransferOperationCanceled();

CATCH_THROW()
}

//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SuspendTransferOperation()
{
TRY_CATCH

	m_fileManager->SuspendTransferOperation();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::ResumeTransferOperation()
{
TRY_CATCH

	m_fileManager->ResumeTransferOperation();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::CreateDirectory(const CLink& directoryName)
{
TRY_CATCH

	if ( directoryName.IsLocal() )
		m_localManagingThread->CreateDirectory(directoryName);
	else
		m_remoteManagingThread->CreateDirectory(directoryName);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetCreateOperationHandler(cbCreateHandler_t handler)
{
TRY_CATCH

	m_createHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::RenameItem(const CLink& newItemName, const CLink& oldItemName)
{
TRY_CATCH

	if ( newItemName.IsLocal() && oldItemName.IsLocal() )
		m_localManagingThread->RenameItem(newItemName,oldItemName);
	else
		m_remoteManagingThread->RenameItem(newItemName,oldItemName);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetRenameOperationHandler(cbRenameHandler_t handler)
{
TRY_CATCH

	m_renameHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::DeleteItem(boost::shared_ptr<link_vec_t> targetItems)
{
TRY_CATCH

	if ( targetItems->size() )
	{
		if ( targetItems->begin()->IsLocal() )
			m_localManagingThread->DeleteItem(targetItems);
		else
			m_remoteManagingThread->DeleteItem(targetItems);
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetDeleteOperationHandler(cbDeleteHandler_t handler)
{
TRY_CATCH

	m_deleteHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::ScanItems(const CLink& path)
{
TRY_CATCH

	if ( path.IsLocal() )
		m_localScanningThread->ScanItems(path);
	else
		m_remoteScanningThread->ScanItems(path);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetScanHandler(cbScanHandler_t handler)
{
TRY_CATCH

	m_scanHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetScanCompletionHandler(cbScanCompletionHandler_t handler)
{
TRY_CATCH

	m_fileManager->SetScanCompletionHandler(handler);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::CancelScanOperation(const CLink& path)
{
TRY_CATCH

	if ( path.IsLocal() )
	{
		InterlockedExchange(&m_abortLocalScanning,TRUE);
		AtlWaitWithMessageLoop(m_localScanningThread->hTerminatedEvent.get());
		InterlockedExchange(&m_abortLocalScanning,FALSE);
	}
	else
	{
		InterlockedExchange(&m_abortRemoteScanning,TRUE);
		AtlWaitWithMessageLoop(m_remoteScanningThread->hTerminatedEvent.get());
		InterlockedExchange(&m_abortRemoteScanning,FALSE);
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetAskForOverwriteHandler(cbAskForOverwriteHandler_t handler)
{
TRY_CATCH

	m_askForOverwriteHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CAsyncFileManager::SetTransferCompressingHandler(cbTransferCompressingHandler_t handler)
{
TRY_CATCH

	m_transferCompressingHandler = handler;

CATCH_THROW()
}
// CAsyncFileManager [END] ///////////////////////////////////////////////////////////////////////////////

// CScanningThread [BEGIN] ///////////////////////////////////////////////////////////////////////////////

void CScanningThread::Execute(void *Params)
{
TRY_CATCH

	CCritSection cs(&m_scanningGuard);

	m_fileManager->ScanItems(m_scanPath);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CScanningThread::CScanningThread(boost::shared_ptr<CFileManager> fileManager)
	:	m_fileManager(fileManager),
		m_scanPath(_T(""))
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CScanningThread::CScanningThread"));
	
	Log.Add(_MESSAGE_,_T("END CScanningThread::CScanningThread"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CScanningThread::ScanItems(const CLink& path)
{
TRY_CATCH

	if ( TryEnterCriticalSection(&m_scanningGuard) )
	{
		m_scanPath = path;
		LeaveCriticalSection(&m_scanningGuard);

		Start();
	}

CATCH_THROW()
}
// CScanningThread [END] /////////////////////////////////////////////////////////////////////////////////

// CManagingThread [BEGIN] ///////////////////////////////////////////////////////////////////////////////

void CManagingThread::Execute(void *Params)
{
TRY_CATCH

	CCritSection cs(&m_managingGuard);

	DWORD errWinResultCode = 0, errInternalResultCode = 0;

	if ( m_manageOperation != op_Delete )
	{
		try
		{
			switch ( m_manageOperation )
			{
				case op_Create:
					m_fileManager->CreateDirectory(m_link1);
					break;
				case op_Rename:
					m_fileManager->RenameItem(m_link1,m_link2);
					break;
			}
		}
		catch (CExceptionBase& exception)
		{
			Log.Add(exception);
			
			errWinResultCode		= exception.GetWindowsErrorCode();
			errInternalResultCode	= exception.GetInternalErrorCode();
		}
		catch (...)				// Because CThread doesn't throw exceptions
		{
			Log.Add(_MESSAGE_,_T("        UNKNOW EXCEPTION"));
			errWinResultCode = errInternalResultCode = -1;
		}
		m_manageHandler(m_manageOperation,m_link1,m_link2,errWinResultCode,errInternalResultCode);
	}
	else
	{
		for (link_vec_t::iterator iLnk = m_linksToDelete->begin(); iLnk!=m_linksToDelete->end(); iLnk++)
		{
			try
			{
				m_link1 = *iLnk;
				m_fileManager->DeleteItem(m_link1);
			}
			catch (CExceptionBase& exception)
			{
				Log.Add(exception);
				
				errWinResultCode		= exception.GetWindowsErrorCode();
				errInternalResultCode	= exception.GetInternalErrorCode();
			}
			catch (...)				// Because CThread doesn't throw exceptions
			{
				Log.Add(_MESSAGE_,_T("        UNKNOW EXCEPTION"));
				errWinResultCode = errInternalResultCode = -1;
			}
			m_manageHandler(m_manageOperation,m_link1,m_link2,errWinResultCode,errInternalResultCode);
		}
		m_linksToDelete.reset();				
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CManagingThread::CManagingThread(boost::shared_ptr<CFileManager> fileManager, cbThunkManageHandler_t manageHandler)
	:	m_fileManager(fileManager),
		m_manageHandler(manageHandler),
		m_link1(_T("")),
		m_link2(_T(""))
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CManagingThread::CManagingThread"));


	Log.Add(_MESSAGE_,_T("END CManagingThread::CManagingThread"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CManagingThread::CreateDirectory(const CLink& directoryName)
{
TRY_CATCH

	if ( TryEnterCriticalSection(&m_managingGuard) )
	{
		m_link1 = directoryName;
		m_manageOperation = op_Create;
		LeaveCriticalSection(&m_managingGuard);
		
		Start();
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CManagingThread::RenameItem(const CLink& newItemName, const CLink& oldItemName)
{
TRY_CATCH

	if ( TryEnterCriticalSection(&m_managingGuard) )
	{
		m_link1 = newItemName;
		m_link2 = oldItemName;
		m_manageOperation = op_Rename;
		LeaveCriticalSection(&m_managingGuard);

		Start();
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CManagingThread::DeleteItem(boost::shared_ptr<link_vec_t> targetItems)
{
TRY_CATCH

	if ( TryEnterCriticalSection(&m_managingGuard) )
	{
		m_linksToDelete = targetItems;
		m_manageOperation = op_Delete;
		LeaveCriticalSection(&m_managingGuard);

		Start();
	}

CATCH_THROW()
}
// CManagingThread [END] /////////////////////////////////////////////////////////////////////////////////

// CTransferQueuedThread [BEGIN] /////////////////////////////////////////////////////////////////////////

void CTransferQueuedThread::Execute(void *Params)
{
TRY_CATCH

	for (;;)
	{
		DWORD waitResult = WaitForSingleObject(m_transferAdded.get(),INFINITE);

		if ( !m_finishTransfer && waitResult == WAIT_OBJECT_0 )
		{
			CCritSection cs(&m_transferGuard);

			bool overwriteFile = false;
			DWORD errWinResultCode = 0, errInternalResultCode = 0;

			while ( !m_listTransfer.empty() )
			{
				std::list<SListItem>::const_iterator topList = m_listTransfer.begin();

				SListItem transferTask(topList->m_destinationItem,topList->m_sourceItem,topList->m_fileData,topList->m_moveOperation);

				cs.Unlock();

				// Callback for status bar while folder is compressing
				if ( transferTask.m_fileData.IsDirectory() )
					m_transferCompressingHandler(transferTask.m_fileData);
					
				try
				{
					if ( transferTask.m_moveOperation )
					{
						//m_fileManager->MoveItem(transferTask.m_destinationItem, transferTask.m_sourceItem, overwriteFile);
						m_fileManager->CopyItem(transferTask.m_destinationItem, transferTask.m_sourceItem, overwriteFile);
						m_transferMoveDeleteHandler(transferTask.m_destinationItem, transferTask.m_sourceItem,transferTask.m_fileData);
						m_fileManager->DeleteItem(transferTask.m_sourceItem);
					}
					else
						m_fileManager->CopyItem(transferTask.m_destinationItem, transferTask.m_sourceItem, overwriteFile);
				}
				catch (CExceptionBase& exception)
				{
					Log.Add(exception);

					errWinResultCode		= exception.GetWindowsErrorCode();
					errInternalResultCode	= exception.GetInternalErrorCode();
				}
				catch (...)				// Because CThread doesn't throw exceptions
				{
					Log.Add(_MESSAGE_,_T("        UNKNOW EXCEPTION"));
					errWinResultCode = errInternalResultCode = -1;
				}
				if ( errInternalResultCode == ERROR_FILE_EXISTS && m_askForOverwriteHandler )// Ask user for overwriting
				{
					if ( m_askForOverwriteHandler(	transferTask.m_destinationItem,
													transferTask.m_sourceItem,
													transferTask.m_fileData,
													!transferTask.m_moveOperation) )
					{
						cs.Lock();
						overwriteFile = true;
						errInternalResultCode = AFM_ERR_OVERWRITE_ACCEPTED;

						continue;
					}
					else
						errInternalResultCode = AFM_ERR_OVERWRITE_DECLINED;
				}
				m_transferCompletionHandler(transferTask.m_destinationItem,
											transferTask.m_sourceItem,
											transferTask.m_fileData,
											!transferTask.m_moveOperation,
											errWinResultCode,
											errInternalResultCode);

				cs.Lock();

				overwriteFile = false;
				errWinResultCode = errInternalResultCode = 0;

				if ( !m_listTransfer.empty() )
					m_listTransfer.pop_front();
			}
		}
		else
			break;
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CTransferQueuedThread::CTransferQueuedThread(	boost::shared_ptr<CFileManager> fileManager,
												cbTransferCompletionHandler_t transferCompletionHandler,
												cbAskForOverwriteHandler_t askForOverwriteHandler,
												cbTransferCompressingHandler_t transferCompressingHandler,
												cbTransferMoveDeleteHandler_t transferMoveDeleteHandler)
	:	m_fileManager(fileManager),
		m_transferCompletionHandler(transferCompletionHandler),
		m_askForOverwriteHandler(askForOverwriteHandler),
		m_transferCompressingHandler(transferCompressingHandler),
		m_transferMoveDeleteHandler(transferMoveDeleteHandler),
		m_destinationLink(_T("")),
		m_sourceLink(_T("")),
		m_finishTransfer(false)
{
TRY_CATCH

	m_transferAdded.reset(CreateEvent(NULL,FALSE,FALSE,NULL),CloseHandle);
	Start();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CTransferQueuedThread::~CTransferQueuedThread()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CTransferQueuedThread::CopyItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData)
{
TRY_CATCH

	CCritSection cs(&m_transferGuard);

	m_listTransfer.push_back(SListItem(destinationItem,sourceItem,fileData,false));
	SetEvent(m_transferAdded.get());
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CTransferQueuedThread::MoveItem(const CLink& destinationItem, const CLink& sourceItem, const CFileData& fileData)
{
TRY_CATCH

	CCritSection cs(&m_transferGuard);

	m_listTransfer.push_back(SListItem(destinationItem,sourceItem,fileData,true));
	SetEvent(m_transferAdded.get());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CTransferQueuedThread::ClearTransferQueue()
{
TRY_CATCH

	CCritSection cs(&m_transferGuard);

	m_listTransfer.clear();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CTransferQueuedThread::StopTransferThread()
{
TRY_CATCH

	m_finishTransfer = true;
	SetEvent(m_transferAdded.get());

CATCH_THROW()
}
// CTransferQueuedThread [END] ///////////////////////////////////////////////////////////////////////////
