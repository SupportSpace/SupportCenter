//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileManager.cpp
///
///  Implements CFileManager class
///  Performs file operations with local and remote system
///  
///  @author Alexander Novak @date 21.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CFileManager.h"
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

// CFileManager [BEGIN] //////////////////////////////////////////////////////////////////////////////////

void CFileManager::NotifyTransferStatus(Status& status)
{
TRY_CATCH

	if ( m_transferHandler )
	{
		WaitForSingleObject(m_transferSuspender.get(),INFINITE);
		
		ULARGE_INTEGER totalSize;
		ULARGE_INTEGER transferedSize;
		totalSize.QuadPart		= status.size;
		transferedSize.QuadPart	= status.transferred;
		
		m_transferHandler(	m_currentTransferDestinationLink,
							m_currentTransferSourceLink,
							totalSize,
							transferedSize);
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::NotifyUncompressStatus()
{
TRY_CATCH

	if ( m_uncompressHandler )
		m_uncompressHandler(m_currentTransferDestinationLink);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

unsigned int CFileManager::OnFindItem(const tstring& path, const CFileData& fileData, const void* userData)
{
TRY_CATCH

	unsigned int callbackResult = TRUE;
	if ( m_scanHandler && m_attributesFilter->IsValid(fileData) )
		callbackResult = m_scanHandler(*static_cast<const CLink*>(userData), fileData);

	return !m_abortScanEvent && callbackResult;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::OnFindComplete(const tstring& path, const void* userData)
{
TRY_CATCH

	if ( m_scanCompletionHandler )
		m_scanCompletionHandler(*static_cast<const CLink*>(userData));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CFileManager::CFileManager(const bool showOperateProgress)
	:	CFileAccessClient((m_rawEventHandle = CreateEvent(NULL,FALSE,FALSE,NULL),m_rawEventHandle)),
		m_abortScanEvent(FALSE),
		m_scanHandler(NULL),
		m_scanCompletionHandler(NULL),
		m_transferHandler(NULL),
		m_uncompressHandler(NULL),
		m_currentTransferDestinationLink(_T("")),
		m_currentTransferSourceLink(_T(""))
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CFileManager::CFileManager"));
	
	m_transferSuspender.reset(CreateEvent(NULL,TRUE,TRUE,NULL),CloseHandle);
	m_abortTransferEvent.reset(m_rawEventHandle,CloseHandle);
	m_fileOperationFlags = FOF_NOERRORUI|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR;
	m_fileOperationFlags |= ( showOperateProgress ?  0 : FOF_SILENT );
	m_attributesFilter.reset(new CAttributesFilter(0));

	Log.Add(_MESSAGE_,_T("END CFileManager::CFileManager"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CFileManager::~CFileManager()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CFileManager::~CFileManager"));

	Log.Add(_MESSAGE_,_T("END CFileManager::~CFileManager"));

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::SetRemoteStream(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CFileManager::SetRemoteStream"));
	
	SetStream(stream);

	Log.Add(_MESSAGE_,_T("END CFileManager::SetRemoteStream"));

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::SetAttributesFilter(boost::shared_ptr<CAttributesFilter> attributesFilter)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CFileManager::SetAttributesFilter"));

	m_attributesFilter = attributesFilter;

	Log.Add(_MESSAGE_,_T("END CFileManager::SetAttributesFilter"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::CreateDirectory(const CLink& directoryName)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CFileManager::CreateDirectory"));

	if ( directoryName.IsLocal() )
		CFileSystem::CreateDirectory(directoryName);
	else
	{
		ResponseCode result = CFileAccessClient::CreateDirectory(directoryName);
		
		if ( result != ResponseCode(0) )
			throw MCException_ErrCode(_T("Can't create directory"),result);
	}

	Log.Add(_MESSAGE_,_T("END CFileManager::CreateDirectory"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::CopyItem(const CLink& destinationItem, const CLink& sourceItem, bool overwriteFile)
{
TRY_CATCH

	Log.Add(	_MESSAGE_,
				_T("BGN CFileManager::CopyItem: destination: %s; source: %s"),
				(const TCHAR*)destinationItem,
				(const TCHAR*)sourceItem);

	if ( destinationItem.IsLocal() && sourceItem.IsLocal() )		// Do local host operations
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::CopyItem: Local host operations"));
		
		//Alloc memory for a trailing zero + trailing zero for function requirement
		size_t sz_alloc = (sourceItem.GetLength() + 1 + 1) * sizeof(TCHAR);
		boost::scoped_array<TCHAR> copyFrom(new TCHAR[sz_alloc]);
		_tcscpy_s(copyFrom.get(), sz_alloc, sourceItem);
		copyFrom[sourceItem.GetLength()+1] = _T('\0');

		SHFILEOPSTRUCT fos;
		fos.hwnd					= 0;
		fos.wFunc					= FO_COPY;
		fos.pFrom					= copyFrom.get();
		fos.pTo						= NULL;
		fos.fFlags					= m_fileOperationFlags;
		fos.hNameMappings			= NULL;
		fos.lpszProgressTitle		= NULL;

		int resultCode = SHFileOperation(&fos);
		Log.Add(	_MESSAGE_,
					_T("        CFileManager::CopyItem: SHFileOperation returned %d, the fAnyOperationsAborted flag is %d"),
					resultCode,
					fos.fAnyOperationsAborted);

		if ( resultCode && !fos.fAnyOperationsAborted )
			throw MCException_ErrCode(_T("Can't copy the file or directory"),resultCode);

	}
	else if ( destinationItem.IsLocal() && !sourceItem.IsLocal() )	// Receive from remote host
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::CopyItem: Receive from remote host"));

		m_currentTransferDestinationLink	= destinationItem;
		m_currentTransferSourceLink			= sourceItem;
		ResponseCode result = ReceiveFile(sourceItem,destinationItem,overwriteFile);

		if ( result != ResponseCode(0) )
			throw MCException_ErrCode(_T("Can't copy file or directory"),result);
	}
	else if ( !destinationItem.IsLocal() && sourceItem.IsLocal() )	// Send to remote host
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::CopyItem: Send to remote host"));

		m_currentTransferDestinationLink	= destinationItem;
		m_currentTransferSourceLink			= sourceItem;
		ResponseCode result = SendFile(destinationItem,sourceItem,overwriteFile);

		if ( result != ResponseCode(0) )
			throw MCException_ErrCode(_T("Can't copy file or directory"),result);
	}
	else															// Do remote host operations
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::CopyItem: Remote host operations"));
		
		// Getting temporary path using the TEMP enviroment variable
		boost::scoped_ptr<TCHAR> tempPath(new TCHAR[MAX_PATH]);
		GetTempPath(MAX_PATH, tempPath.get());

		// Create name for a temp file
		boost::scoped_ptr<TCHAR> tempName(new TCHAR[MAX_PATH + 1]);
		if ( !GetTempFileName(tempPath.get(),_T("~rcft"),0,tempName.get()) )
			throw MCException_Win(_T("Fail to create temporary file name"));
		
		tstring tempFileName = tempPath.get();

		Log.Add(_MESSAGE_,_T("        CFileManager::CopyItem Temporary file name is: %s"),tempPath.get());

		m_currentTransferDestinationLink	= destinationItem;
		m_currentTransferSourceLink			= sourceItem;
		ResponseCode result = ReceiveFile(sourceItem,tempFileName,overwriteFile);

		if ( result != ResponseCode(0) )
			throw MCException_ErrCode(_T("Can't receive file or directory in the temporary storage"),result);

		m_currentTransferDestinationLink	= destinationItem;
		m_currentTransferSourceLink			= sourceItem;
		result = SendFile(destinationItem,tempFileName,overwriteFile);

		if ( result != ResponseCode(0) )
			throw MCException_ErrCode(_T("Can't send file or directory form the temporary storage"),result);
	}

	Log.Add(_MESSAGE_,_T("END CFileManager::CopyItem"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::RenameItem(const CLink& newItemName, const CLink& oldItemName)
{
TRY_CATCH

	Log.Add(	_MESSAGE_,
				_T("BGN CFileManager::RenameItem: newName: %s; oldName: %s"),
				(const TCHAR*)newItemName,
				(const TCHAR*)oldItemName);

	if ( newItemName.IsLocal() && oldItemName.IsLocal() )					// Do local host operations
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::RenameItem: Local host operations"));

		//Alloc memory for a trailing zero + trailing zero for function requirement
		size_t sz_alloc = (oldItemName.GetLength() + 1 + 1) * sizeof(TCHAR);
		boost::scoped_array<TCHAR> oldName(new TCHAR[sz_alloc]);
		_tcscpy_s(oldName.get(), sz_alloc, oldItemName);
		oldName[oldItemName.GetLength()+1] = _T('\0');

		sz_alloc = (newItemName.GetLength() + 1 + 1) * sizeof(TCHAR);
		boost::scoped_array<TCHAR> newName(new TCHAR[sz_alloc]);
		_tcscpy_s(newName.get(), sz_alloc, newItemName);
		newName[newItemName.GetLength()+1] = _T('\0');

		SHFILEOPSTRUCT fos;
		fos.hwnd					= 0;
		fos.wFunc					= FO_RENAME;
		fos.pFrom					= oldName.get();
		fos.pTo						= newName.get();
		fos.fFlags					= m_fileOperationFlags|FOF_NOERRORUI;
		fos.hNameMappings			= NULL;
		fos.lpszProgressTitle		= NULL;

		int resultCode = SHFileOperation(&fos);
		Log.Add(	_MESSAGE_,
					_T("        CFileManager::RenameItem: SHFileOperation returned %d, the fAnyOperationsAborted flag is %d"),
					resultCode,
					fos.fAnyOperationsAborted);

		if ( resultCode && !fos.fAnyOperationsAborted )
			throw MCException_ErrCode(_T("Can't rename file or directory"),resultCode);
	}
	else if ( !newItemName.IsLocal() && !oldItemName.IsLocal() )			// Do remote host operations
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::RenameItem: Remote host operations"));

		ResponseCode result = RenameFile(newItemName,oldItemName);

		if ( result != ResponseCode(0) )
			throw MCException_ErrCode(_T("Can't rename file or directory"),result);
	}
	else
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::RenameItem: New and old names have the different host"));

		throw MCException(_T("Can't rename file or directory placed on different hosts"));
	}

	Log.Add(_MESSAGE_,_T("END CFileManager::RenameItem"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::MoveItem(const CLink& destinationItem, const CLink& sourceItem, bool overwriteFile)
{
TRY_CATCH

	Log.Add(	_MESSAGE_,
				_T("BGN CFileManager::MoveItem: destination: %s; source: %s"),
				(const TCHAR*)destinationItem,
				(const TCHAR*)sourceItem);

	if ( destinationItem.IsLocal() && sourceItem.IsLocal() )		// Do local host operations
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Local host operations"));
		Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Redirect to RenameItem method"));

		RenameItem(destinationItem,sourceItem);
	}
	else if ( destinationItem.IsLocal() && !sourceItem.IsLocal() )	// Receive from remote host
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Receive from the remote host"));
		Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Redirect to CopyItem method"));

		CopyItem(destinationItem,sourceItem,overwriteFile);

		// Coping was successed, so delete file or directory
		Log.Add(	_MESSAGE_,
					_T("        CFileManager::MoveItem: Receiving from the remote host has been completed. Redirect to DeleteItem method"));
		
		DeleteItem(sourceItem);
	}
	else if ( !destinationItem.IsLocal() && sourceItem.IsLocal() )	// Send to remote host
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Send to the remote host"));
		Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Redirect to CopyItem method"));

		CopyItem(destinationItem,sourceItem,overwriteFile);
		
		// Coping was successed, so delete file or directory
		Log.Add(	_MESSAGE_,
					_T("        CFileManager::MoveItem: Sending to the remote host has been completed. Redirect to DeleteItem method"));

		DeleteItem(sourceItem);
	}
	else															// Do remote host operations
	{
		Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Remote host operations"));
		
		//TODO: Fix code below (Fix CFileAccesHost::RemaneFile method in future) BWT: rename it :).
		// Problem was decribing in RenameItem method declaration 
		if ( destinationItem.GetDriveLetter() == sourceItem.GetDriveLetter() )
		{
			Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Redirect to RenameItem method"));

			RenameItem(destinationItem,sourceItem);
		}
		else
		{
			Log.Add(_MESSAGE_,_T("        CFileManager::MoveItem: Redirect to CopyItem method"));
			
			CopyItem(destinationItem,sourceItem,overwriteFile);
			
			// Coping was successed, so delete file or directory
			Log.Add(	_MESSAGE_,
						_T("        CFileManager::MoveItem: Coping operation has been completed. Redirect to DeleteItem method"));

			DeleteItem(sourceItem);
		}
	}

	Log.Add(_MESSAGE_,_T("END CFileManager::MoveItem"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::DeleteItem(const CLink& targetItem)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CFileManager::DeleteItem"));

	if ( targetItem.IsLocal() )
	{
		//Alloc memory for a trailing zero + trailing zero for function requirement
		size_t sz_alloc = (targetItem.GetLength() + 1 + 1) * sizeof(TCHAR);
		boost::scoped_array<TCHAR> deleteName(new TCHAR[sz_alloc]);
		_tcscpy_s(deleteName.get(), sz_alloc, targetItem);
		deleteName[targetItem.GetLength()+1] = _T('\0');

		SHFILEOPSTRUCT fos;
		fos.hwnd					= 0;
		fos.wFunc					= FO_DELETE;
		fos.pFrom					= deleteName.get();
		fos.pTo						= NULL;	
		fos.fFlags					= m_fileOperationFlags;
		fos.hNameMappings			= NULL;
		fos.lpszProgressTitle		= NULL;

		int resultCode = SHFileOperation(&fos);
		Log.Add(	_MESSAGE_,
					_T("        CFileManager::DeleteItem: SHFileOperation returned %d, the fAnyOperationsAborted flag is %d"),
					resultCode,
					fos.fAnyOperationsAborted);

		if ( resultCode && !fos.fAnyOperationsAborted )
			throw MCException_ErrCode(_T("Can't delete file or directory"),resultCode);
	}
	else
	{
		ResponseCode result = DeleteDirectory(targetItem);

		if ( result != ResponseCode(0) )
			throw MCException_ErrCode(_T("Can't delete file or directory on the remote host"),result);
	}

	Log.Add(_MESSAGE_,_T("END CFileManager::DeleteItem"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::ScanItems(const CLink& path)
{
TRY_CATCH
	
	Log.Add(_MESSAGE_,_T("BGN CFileManager::ScanItems: path=%s"),(const TCHAR*)path);

	InterlockedExchange(&m_abortScanEvent,FALSE);
	
	if ( !path.IsEmpty() )		// Browse for directory content
	{
		CFileData levelUpFolder;
		ZeroMemory(&levelUpFolder,sizeof(levelUpFolder));
		levelUpFolder.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		_tcscpy(levelUpFolder.cFileName,_T(".."));
		
		if ( path.IsLocal() )
		{
			Log.Add(_MESSAGE_,_T("    CFileManager::ScanItems: Getting local file list"));

			// Push a level up folder into the searching context
			OnFindItem(path,levelUpFolder,&path);

			CFileSystem::ScanItems(path,static_cast<const void*>(&path));
		}
		else
		{
			Log.Add(_MESSAGE_,_T("    CFileManager::ScanItems: Getting remote file list"));

			TFileInfo fileInfo;
			tstring browsePath = path;
			browsePath += _T("\\");

			ListFiles(browsePath,fileInfo);

			// Push a level up folder into the searching context
			OnFindItem(path,levelUpFolder,&path);

			for (unsigned int i=0; i < fileInfo.size(); i++)
			{
				CFileData fileData(fileInfo[i]);
				if ( !fileData.IsRootDirectory() )
					if ( !OnFindItem(path,fileData,&path) )
						break;
			}
			OnFindComplete(path,&path);
		}
	}
	else						// Browse for drives
	{
		if ( path.IsLocal() )
		{
			Log.Add(_MESSAGE_,_T("    CFileManager::ScanItems: Getting local drives list"));

			DWORD needSize = GetLogicalDriveStrings(0,NULL);
			boost::scoped_ptr<TCHAR> buffer(new TCHAR[needSize+1]);
			
			GetLogicalDriveStrings(needSize+1,buffer.get());
			TCHAR* nextDrive = buffer.get();
			
			UINT prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
			
			while ( *nextDrive )
			{
				CFileData fileData;
				fileData.m_drive = true;
				_tcscpy_s(fileData.cFileName,sizeof(fileData.cFileName)*sizeof(TCHAR),nextDrive);
				fileData.dwFileAttributes = m_getDriveType(nextDrive);

				fileData.nFileSizeHigh = fileData.nFileSizeLow = 0;
				
				if ( fileData.dwFileAttributes != DT_FLOPPY )
				{
					ULARGE_INTEGER driveSize;

					if ( GetDiskFreeSpaceEx(fileData.cFileName,NULL,&driveSize,NULL) )
					{
						fileData.nFileSizeHigh	= driveSize.HighPart;
						fileData.nFileSizeLow	= driveSize.LowPart;
					}
				}
				
				if ( !OnFindItem(path,fileData,&path) )
					break;
				
				nextDrive = nextDrive + _tcslen(nextDrive)+1;
			}
			OnFindComplete(path,&path);

			SetErrorMode(prevErrorMode);
		}
		else
		{
			Log.Add(_MESSAGE_,_T("    CFileManager::ScanItems: Getting remote drives list"));

			TDriveInfo driveInfo;
			ListDrives(driveInfo);

			for (unsigned int i=0; i < driveInfo.size(); i++)
			{
				CFileData fileData;
				fileData.m_drive = true;
				driveInfo[i].drive += _T("\\");
				_tcscpy_s(fileData.cFileName,sizeof(fileData.cFileName)*sizeof(TCHAR),driveInfo[i].drive.c_str());
				
				ULARGE_INTEGER driveSize;
				driveSize = driveInfo[i].space;
				fileData.dwFileAttributes	= driveInfo[i].type;
				fileData.nFileSizeHigh		= driveSize.HighPart;
				fileData.nFileSizeLow		= driveSize.LowPart;

				//Log.Add(_MESSAGE_,_T("     CFileManager::ScanItems: fileName=%s"),fileData.GetFileName());

				if ( !OnFindItem(path,fileData,&path) )
					break;
			}
			OnFindComplete(path,&path);
		}
	}
	Log.Add(_MESSAGE_,_T("END CFileManager::ScanItems"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::CancelScanOperation()
{
TRY_CATCH

	InterlockedExchange(&m_abortScanEvent,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::CancelTransferOperation()
{
TRY_CATCH

	SetEvent(m_abortTransferEvent.get());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CFileManager::TransferOperationCanceled()
{
TRY_CATCH
	DWORD dwAbort = WaitForSingleObject( m_abortTransferEvent.get(), 2 );
	return WAIT_OBJECT_0==dwAbort?true:false;
CATCH_THROW()
}

//--------------------------------------------------------------------------------------------------------

void CFileManager::SuspendTransferOperation()
{
TRY_CATCH

	ResetEvent(m_transferSuspender.get());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::ResumeTransferOperation()
{
TRY_CATCH

	SetEvent(m_transferSuspender.get());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::SetScanHandler(cbScanResultHandler_t handler)
{
TRY_CATCH

	m_scanHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::SetScanCompletionHandler(cbScanCompletionHandler_t handler)
{
TRY_CATCH

	m_scanCompletionHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::SetTransferHandler(cbTransferHandler_t handler)
{
TRY_CATCH

	m_transferHandler = handler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManager::SetUncompressHandler(cbUncompressHandler_t handler)
{
TRY_CATCH

	m_uncompressHandler = handler;

CATCH_THROW()
}
// CFileManager [END] ////////////////////////////////////////////////////////////////////////////////////
