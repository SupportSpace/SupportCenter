//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CCommandManager.cpp
///
///  Implements CCommandManager class
///  Mediator class for interaction between UI and FileManager object
///  
///  @author Alexander Novak @date 22.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "CCommandManager.h"
#include <boost/bind.hpp>
#include "CSkinnedMsgBox.h"
#include "CSkinnedWaitMsgBox.h"
#include "..\..\Brokers\Shared\BrokersTypes.h"
//#include <uxtheme.h>

//#pragma comment (lib,"UxTheme.lib")

// CCommandManager [BEGIN] ///////////////////////////////////////////////////////////////////////////////

void CCommandManager::TransferHandler(	const CLink& destinationItem,
										const CLink& sourceItem,
										const ULARGE_INTEGER totalSize,
										const ULARGE_INTEGER transferedSize)
{
TRY_CATCH

	tstring strStatus(( sourceItem.IsLocal() ? _T("Sending: ") : _T("Receiving: ") ));
	strStatus += sourceItem.GetTargetName();
	strStatus += _T("...");
	ShowStatusMessage(strStatus);

	bool rearrange = !m_wdtProgressBar->IsVisible();
	
	m_wdtProgressBar->ShowWidget(true);
	m_wdtCancelTransferButton->ShowWidget(true);
	m_wdtProgressBar->SetProgressState(totalSize,transferedSize);
	
	if ( rearrange )
		CmdRearrangeWidgets(m_wdtStatusPanel.get());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::UncompressHandler(const CLink& uncompressLink)
{
TRY_CATCH

	tstring strStatus = _T("Decompressing ");
	strStatus += uncompressLink.GetTargetName();
	strStatus += _T(" folder...");
	SetDefaultStatusMessage(_T(""),0);	// Turn off text changing
	ShowStatusMessage(strStatus);

	m_wdtProgressBar->ShowWidget(false);
	m_wdtCancelTransferButton->ShowWidget(false);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::TransferCompletionHandler(const CLink& destinationItem,
												const CLink& sourceItem,
												const CFileData& fileData,
												const bool copyOperation,
												const DWORD winErr,
												const DWORD internalErr)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::TransferCompletionHandler: WinErr: %u, IntErr: %u"),winErr,internalErr);
	
	m_wdtProgressBar->ShowWidget(false);
	m_wdtCancelTransferButton->ShowWidget(false);

	CmdRearrangeWidgets(m_wdtStatusPanel.get());
	ChangeWidgetState(CMST_TRANSFER_COMPLETE);

	CLink destinationPath = destinationItem;
	destinationPath.CutTargetName();
	CLink sourcePath = sourceItem;
	sourcePath.CutTargetName();

	if ( m_lockCounter == 0 && m_customIsConnected )
		SetDefaultStatusMessage(_T("File manager session is ON"),CM_DEFAULTTEXT_ELAPSE,EFMS_ON);	// Turn on text changing

	if ( internalErr == AFM_ERR_OVERWRITE_DECLINED )
		return;

	if ( internalErr == operation_was_aborted )
	{
		if ( copyOperation )
			m_transferLog.AddMessage(Format(_T("%s customer %s was canceled"),
											( sourceItem.IsLocal() ? _T("Sending to") : _T("Receiving from") ),
											(const TCHAR*)( sourceItem.IsLocal() ? destinationItem : sourceItem )).c_str());
		else
			m_transferLog.AddMessage(Format(_T("%s customer %s was canceled"),
											( sourceItem.IsLocal() ? _T("Moving to") : _T("Moving from") ),
											(const TCHAR*)( sourceItem.IsLocal() ? destinationItem : sourceItem )).c_str());

		if ( destinationItem.IsLocal() )
		{
			if ( m_currentExpertLink == destinationPath && m_lockCounter == 0)
				m_wdtExpertRefresh->PostCommand();
		}
		else
		{
			if ( m_currentCustomerLink == destinationPath && m_lockCounter == 0)
				m_wdtCustomerRefresh->PostCommand();
		}
		if ( !copyOperation )
		{
			if ( sourceItem.IsLocal() )
			{
				if ( m_currentExpertLink == sourcePath && m_lockCounter == 0 )
					m_wdtExpertRefresh->PostCommand();
			}
			else
			{
				if ( m_currentCustomerLink == sourcePath && m_lockCounter == 0 )
					m_wdtCustomerRefresh->PostCommand();
			}			
		}
		if ( m_customIsConnected )
			ShowStatusMessage(_T("File transfer was canceled"),EFMS_SESSION_FAILED);
		
		Log.Add(_MESSAGE_,_T("END CCommandManager::TransferCompletionHandler"));
		return;
	}
	else if ( internalErr && internalErr != AFM_ERR_OVERWRITE_ACCEPTED )
	{
		tstring errorMessage(_T("Can't complete transfer operation.\n"));

		TCHAR* lpMsgBuf;
		if ( FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							internalErr,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US),
							(LPTSTR)&lpMsgBuf,
							0,
							NULL) )
		{
			errorMessage += lpMsgBuf;
			LocalFree(lpMsgBuf);
		}
		else
			errorMessage += _T("Unknow error.");

		CSkinnedMsgBox(this, false, false).Show(0,errorMessage,true);
		
		if ( destinationItem.IsLocal() )
		{
			if ( m_currentExpertLink == destinationPath && m_lockCounter == 0)
				m_wdtExpertRefresh->PostCommand();
		}
		else
		{
			if ( m_currentCustomerLink == destinationPath && m_lockCounter == 0)
				m_wdtCustomerRefresh->PostCommand();
		}
		if ( !copyOperation )
		{
			if ( sourceItem.IsLocal() )
			{
				if ( m_currentExpertLink == sourcePath && m_lockCounter == 0 )
					m_wdtExpertRefresh->PostCommand();
			}
			else
			{
				if ( m_currentCustomerLink == sourcePath && m_lockCounter == 0 )
					m_wdtCustomerRefresh->PostCommand();
			}			
		}
		tstring strStatus(_T("File "));
		strStatus += sourceItem.GetTargetName();
		strStatus += ( sourceItem.IsLocal() ? _T(" wasn't sent") : _T(" wasn't received") );
		ShowStatusMessage(strStatus);
	}
	else
	{
		if ( copyOperation )
		{
			m_transferLog.AddMessage(Format(_T("%s customer %s"),
											( sourceItem.IsLocal() ? _T("Sent to") : _T("Received from") ),
											(const TCHAR*)( sourceItem.IsLocal() ? sourceItem : destinationItem )).c_str());
			if ( fileData.IsDirectory() )
			{
				m_dirLogging->ContentToLog(	(sourceItem.IsLocal()?sourceItem:destinationItem),
											(sourceItem.IsLocal()?_T("Sent to customer "):_T("Received from customer ")));
			}
		}
		else
		{
			if ( !fileData.IsDirectory() )
				m_transferLog.AddMessage(Format(_T("%s customer %s"),
												( sourceItem.IsLocal() ? _T("Moved to") : _T("Moved from") ),
												(const TCHAR*)( sourceItem.IsLocal() ? sourceItem : destinationItem )).c_str());
		}

		m_wdtHistoryListView->AddTransferInfo(fileData,destinationItem.IsLocal());

		if ( destinationItem.IsLocal() )
		{
			if ( m_currentExpertLink == destinationPath && m_lockCounter == 0)
				m_wdtExpertRefresh->PostCommand();
		}
		else
		{
			if ( m_currentCustomerLink == destinationPath && m_lockCounter == 0)
				m_wdtCustomerRefresh->PostCommand();
		}
		if ( !copyOperation )
		{
			if ( sourceItem.IsLocal() )
			{
				if ( m_currentExpertLink == sourcePath && m_lockCounter == 0 )
					m_wdtExpertRefresh->PostCommand();
			}
			else
			{
				if ( m_currentCustomerLink == sourcePath && m_lockCounter == 0 )
					m_wdtCustomerRefresh->PostCommand();
			}			
		}
		tstring strStatus( fileData.IsDirectory() ? _T("Folder ") : _T("File ") );
		strStatus += sourceItem.GetTargetName();
		strStatus += ( sourceItem.IsLocal() ? _T(" was sent") : _T(" was received") );
		ShowStatusMessage(strStatus);
	}

	Log.Add(_MESSAGE_,_T("END CCommandManager::TransferCompletionHandler"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::TransferMoveDeleteHandler(const CLink& destinationItem,
												const CLink& sourceItem,
												const CFileData& fileData)
{
TRY_CATCH

	if ( fileData.IsDirectory() )
	{
		m_transferLog.AddMessage(Format(_T("%s customer %s"),
										( sourceItem.IsLocal() ? _T("Moved to") : _T("Moved from") ),
										(const TCHAR*)( sourceItem.IsLocal() ? sourceItem : destinationItem )).c_str());

		m_dirLogging->ContentToLog(	(sourceItem.IsLocal()?sourceItem:destinationItem),
									(sourceItem.IsLocal()?_T("Moved to customer "):_T("Moved from customer ")));
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CCommandManager::AskForOverwriteHandler(	const CLink& destinationItem,
												const CLink& sourceItem,
												const CFileData& fileData,
												const bool copyOperation)
{
TRY_CATCH

	if ( !m_asyncFileManagerAvailable )
		return false;
	
	tstring strMsg(_T("File "));
	strMsg += destinationItem.GetTargetName();
	strMsg += _T(" already exists. Would you like to overwrite it?");

	INT_PTR result = CSkinnedMsgBox(this, true, true).Show(CM_DLGID_CONFIRM_OVERWRITE,strMsg,true);

	if (m_asyncFileManager->TransferOperationCanceled())
		return false;	

	return IDOK==result?true:false;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::TransferCompressingHandler(const CFileData& fileData)
{
TRY_CATCH

	tstring strStatus = _T("Compressing ");
	strStatus += fileData.GetFileName();
	strStatus += _T(" folder...");
	SetDefaultStatusMessage(_T(""),0);	// Turn off text changing
	ShowStatusMessage(strStatus);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CreateOperationHandler(const CLink& directoryName, const DWORD winErr, const DWORD internalErr)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CreateOperationHandler: WinErr: %u, IntErr: %u"),winErr,internalErr);

	boost::shared_ptr<CWidgetFileListView> wdtActive;
	
	if ( directoryName.IsLocal() )
		wdtActive = m_wdtExpertFileListView;
	else
	{
		wdtActive = m_wdtCustomerFileListView;
		ChangeWidgetState(CMST_CREATE_DIRECTORY_COMPLETE);
	}
	if ( internalErr == ERROR_ALREADY_EXISTS )
	{
		CSkinnedMsgBox(this, false, false).Show(0,_T("Can't create directory.\nDirectory already exists."));
		wdtActive->CancelDirectoryName();

		ShowStatusMessage(_T("Folder already exists"));
	}
	else if ( internalErr )
	{
		tstring errorMessage(_T("Can't create directory.\n"));

		TCHAR* lpMsgBuf;
		if ( FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							internalErr,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US),
							(LPTSTR)&lpMsgBuf,
							0,
							NULL) )
		{
			errorMessage += lpMsgBuf;
			LocalFree(lpMsgBuf);
		}
		else
			errorMessage += _T("Unknow error.");

		CSkinnedMsgBox(this, false, false).Show(0,errorMessage,true);
		wdtActive->UndoCreateDirectory();

		ShowStatusMessage(_T("Folder wasn't created"));
	}
	else
	{
		if ( !directoryName.IsLocal() )
			m_transferLog.AddMessage(Format(_T("Created in customer %s"),(const TCHAR*)directoryName).c_str());

		wdtActive->ApplyDirectoryName();

		tstring strStatus(_T("Folder "));
		strStatus += directoryName.GetTargetName();
		strStatus += _T(" was created");
		ShowStatusMessage(strStatus);
	}

	Log.Add(_MESSAGE_,_T("END CCommandManager::CreateOperationHandler"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::RenameOperationHandler(const CLink& newItemName, const CLink& oldItemName, const DWORD winErr, const DWORD internalErr)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::RenameOperationHandler: WinErr: %u, IntErr: %u"),winErr,internalErr);

	boost::shared_ptr<CWidgetFileListView> wdtActive;
	
	if ( newItemName.IsLocal() )
		wdtActive = m_wdtExpertFileListView;
	else
	{
		wdtActive = m_wdtCustomerFileListView;
		ChangeWidgetState(CMST_RENAME_FILE_COMPLETE);
	}
	if ( internalErr )
	{
		tstring errorMessage(_T("Can't rename file or directory.\n"));

		TCHAR* lpMsgBuf;
		if ( FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							internalErr,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US),
							(LPTSTR)&lpMsgBuf,
							0,
							NULL) )
		{
			errorMessage += lpMsgBuf;
			LocalFree(lpMsgBuf);
		}
		else
			errorMessage += _T("Unknow error.");

		CSkinnedMsgBox(this, false, false).Show(0,errorMessage,true);
		wdtActive->RestoreEditedItem();

		tstring strStatus(_T("File "));
		strStatus += oldItemName.GetTargetName();
		strStatus += _T(" wasn't renamed");
		ShowStatusMessage(strStatus);
	}
	else
	{
		if ( !oldItemName.IsLocal() && !newItemName.IsLocal() ) 
			m_transferLog.AddMessage(Format(_T("Renamed in customer %s to %s"),
											(const TCHAR*)oldItemName,
											(const TCHAR*)newItemName).c_str());

		wdtActive->RenameEditedItem();

		tstring strStatus(_T("File "));
		strStatus += oldItemName.GetTargetName();
		strStatus += _T(" was renamed to ");
		strStatus += newItemName.GetTargetName();
		ShowStatusMessage(strStatus);
	}
	
	Log.Add(_MESSAGE_,_T("END CCommandManager::RenameOperationHandler"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::DeleteOperationHandler(const CLink& targetItem, const DWORD winErr, const DWORD internalErr)
{
TRY_CATCH

	const CLink* linkActive;
	boost::shared_ptr<CWidgetFileListView> wdtListViewActive;
	boost::shared_ptr<CWidgetBitmapButton> wdtButtonActive;
	
	if ( targetItem.IsLocal() )
	{
		linkActive			= &m_currentExpertLink;
		wdtListViewActive	= m_wdtExpertFileListView;
		wdtButtonActive		= m_wdtExpertRefresh;
		ChangeWidgetState(CMST_DELETE_EXPERT_COMPLETE);
	}
	else
	{
		linkActive			= &m_currentCustomerLink;
		wdtListViewActive	= m_wdtCustomerFileListView;
		wdtButtonActive		= m_wdtCustomerRefresh;
		ChangeWidgetState(CMST_DELETE_CUSTOM_COMPLETE);
	}
	if ( internalErr )
	{
		tstring errorMessage(_T("Can't delete "));
		errorMessage += targetItem.GetTargetName();
		errorMessage += _T(".\n");

		TCHAR* lpMsgBuf;
		if ( FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							internalErr,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US),
							(LPTSTR)&lpMsgBuf,
							0,
							NULL) )
		{
			errorMessage += lpMsgBuf;
			LocalFree(lpMsgBuf);
		}
		else
			errorMessage += _T("Unknow error.");

		CSkinnedMsgBox(this, false, false).Show(0,errorMessage,true);

		tstring strStatus(_T("Can't delete "));
		strStatus += targetItem.GetTargetName();
		strStatus += _T(".");
		ShowStatusMessage(strStatus);
	}
	else
	{
		if ( !targetItem.IsLocal() )
			m_transferLog.AddMessage(Format(_T("Deleted from customer %s"),(const TCHAR*)targetItem).c_str());

		CLink targetPath = targetItem;
		targetPath.CutTargetName();
		if ( *linkActive == targetPath && m_lockCounter == 0 )
			wdtButtonActive->PostCommand();

		tstring strStatus(_T("File "));
		strStatus += targetItem.GetTargetName();
		strStatus += _T(" was deleted");
		ShowStatusMessage(strStatus);
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::ScanHandler(const CLink& path, const CFileData& fileData)
{
TRY_CATCH

//	Log.Add(_MESSAGE_,_T("BGN CCommandManager::ScanHandler file=%s"),fileData.GetFileName());

	if ( path.IsLocal() )
	{
		m_wdtExpertFileListView->InsertFileInfo(fileData);
		if ( m_expertPanelIsActive )
			ChangeWidgetState(CMST_EXPERT_BROWSE);
	}
	else
	{
		m_wdtCustomerFileListView->InsertFileInfo(fileData);
		if ( !m_expertPanelIsActive )
			ChangeWidgetState(CMST_CUSTOM_BROWSE);
	}

//	Log.Add(_MESSAGE_,_T("END CCommandManager::ScanHandler"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::ScanCompletionHandler(const CLink& path)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::ScanCompletionHandler path=%s"),(const TCHAR*)path);

	if ( path.IsLocal() )
	{
		ChangeWidgetState(CMST_EXPERT_BROWSE_COMPLETE);
		m_wdtExpertFileListView->EnableSorting(true);
	}
	else
	{
		ChangeWidgetState(CMST_CUSTOM_BROWSE_COMPLETE);
		m_wdtCustomerFileListView->EnableSorting(true);
	}
	//tstring strStatus(_T("Entered into "));
	//if ( path.IsEmpty() )
	//	strStatus += _T("root");
	//else
	//	strStatus += (const TCHAR*)path;
	//strStatus += _T(" folder");
	//ShowStatusMessage(strStatus);

	Log.Add(_MESSAGE_,_T("END CCommandManager::ScanCompletionHandler"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::ExecuteCommand(const CCommandProxy* sender, EWidgetCommand command, const void* commandData)
{
TRY_CATCH

	//switch ( command )
	//{
	//	case cmd_NullCommand:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_NullCommandcommand: %u"),command);
	//		break;
	//	case cmd_Size:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_Size command: %u"),command);
	//		break;
	//	case cmd_ButtonClick:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_ButtonClick command: %u"),command);
	//		break;
	//	case cmd_BrowseForFiles:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_BrowseForFiles command: %u"),command);
	//		break;
	//	case cmd_UpdateFileStatus:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_UpdateFileStatus command: %u"),command);
	//		break;
	//	case cmd_CreateDirectory:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_CreateDirectory command: %u"),command);
	//		break;
	//	case cmd_RenameFile:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_RenameFile command: %u"),command);
	//		break;
	//	case cmd_DeleteFile:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_DeleteFile command: %u"),command);
	//		break;
	//	case cmd_CopyFile:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_CopyFile command: %u"),command);
	//		break;
	//	case cmd_MoveFile:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_MoveFile command: %u"),command);
	//		break;
	//	case cmd_ChangePanel:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_ChangePanel command: %u"),command);
	//		break;
	//	case cmd_StartRenaming:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_StartRenaming command: %u"),command);
	//		break;
	//	case cmd_EndRenaming:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_EndRenaming command: %u"),command);
	//		break;
	//	case cmd_StartDirCreate:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_StartDirCreate command: %u"),command);
	//		break;
	//	case cmd_EndDirCreate:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_EndDirCreate command: %u"),command);
	//		break;
	//	case cmd_FileSelected:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_FileSelected command: %u"),command);
	//		break;
	//	case cmd_DragDropCopy:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_DragDropCopy command: %u"),command);
	//		break;
	//	case cmd_BeginDragDrop:
	//		Log.Add(_MESSAGE_,_T("CCommandManager::ExecuteCommand: cmd_BeginDragDrop command: %u"),command);
	//		break;
	//}

	switch ( command )
	{
		case cmd_ButtonClick:
			
			if ( sender == m_wdtExpertRefresh.get() )
				CmdContentBrowse(m_currentExpertLink, false);
			else if ( sender == m_wdtCustomerRefresh.get() )
				CmdContentBrowse(m_currentCustomerLink, false);
			else if ( sender == m_wdtExpertUp.get() )
			{
				m_currentExpertLink.CutTargetName();
				CmdContentBrowse(m_currentExpertLink);
			}
			else if ( sender == m_wdtCustomerUp.get() )
			{
				m_currentCustomerLink.CutTargetName();
				CmdContentBrowse(m_currentCustomerLink);
			}
			else if ( sender == m_wdtRequestButton.get() && m_requestHandler )
			{
				SetDefaultStatusMessage(_T(""),0,EFMS_INIFIAL);		// Turn off default text appearing
				m_requestHandler();
			}
			else if ( sender == m_wdtExpandHistoryButton.get() )
			{
				m_historyWidgetExpanded = !m_historyWidgetExpanded;
				CmdResizeHistoryWidget(m_historyWidgetExpanded);
			}
			else if ( sender == m_wdtCancelTransferButton.get() )
			{
				m_asyncFileManager->SuspendTransferOperation();

				if ( CSkinnedMsgBox(this, true, false).Show(0,_T("Are you sure you would like to cancel the file transferring?"),true) == IDOK )
				{
					ChangeWidgetState(CMST_TRANSFER_CANCELED);
					m_asyncFileManager->CancelAllTransferOperations();
				}

				m_asyncFileManager->ResumeTransferOperation();
			}
			break;
		case cmd_BrowseForFilesUp:
			if ( sender == m_wdtExpertFileListView.get() && m_lockCounter == 0 && !m_currentExpertLink.IsEmpty() )
			{
				m_currentExpertLink.CutTargetName();
				CmdContentBrowse(m_currentExpertLink);
			}
			else if ( sender == m_wdtCustomerFileListView.get() && m_lockCounter == 0 && !m_currentCustomerLink.IsEmpty() )
			{
				m_currentCustomerLink.CutTargetName();
				CmdContentBrowse(m_currentCustomerLink);
			}
			break;
		case cmd_BrowseForFiles:
			if ( sender == m_wdtExpertFileListView.get() )
			{
				m_currentExpertLink += m_wdtExpertFileListView->GetItemName(commandData);
				CmdContentBrowse(m_currentExpertLink);
			}
			else if ( sender == m_wdtCustomerFileListView.get() )
			{
				m_currentCustomerLink += m_wdtCustomerFileListView->GetItemName(commandData);
				CmdContentBrowse(m_currentCustomerLink);
			}
			break;
		case cmd_UpdateFileStatus:
			if ( sender == m_wdtExpertFileListView.get() )
				m_wdtExpertFileInfo->SetWindowText(m_wdtExpertFileListView->GetCurrentStatusString().c_str());
			else if ( sender == m_wdtCustomerFileListView.get() )
				m_wdtCustomerFileInfo->SetWindowText(m_wdtCustomerFileListView->GetCurrentStatusString().c_str());
			break;
		case cmd_FileSelected:
			if ( sender == m_wdtExpertFileListView.get() )
				ChangeWidgetState(CMST_EXPERT_FILE_SELECTED);
			else if ( sender == m_wdtCustomerFileListView.get() )
				ChangeWidgetState(CMST_CUSTOM_FILE_SELECTED);
			break;
		case cmd_RenameFile:
			CmdRenameFile();
			break;
		case cmd_StartRenaming:
			CmdRenameFileStart();
		case cmd_EndRenaming:
			CmdRenameFileComplete( sender == m_wdtExpertFileListView.get(),commandData);
			break;
		case cmd_CreateDirectory:
			CmdCreateDirectory();
			break;
		case cmd_EndDirCreate:
			CmdCreateDirectoryComplete( sender == m_wdtExpertFileListView.get() );
			break;
		case cmd_DeleteFile:
			CmdDeleteFile();
			break;
		case cmd_CopyFile:
		case cmd_MoveFile:
			CmdTransferOperations( command == cmd_MoveFile );
			break;
		case cmd_BeginDragDrop:
			if ( sender == m_wdtExpertFileListView.get() )
				m_wdtCustomerFileListView->AllowDragAndDrop(IsAllowTransferFile());

			if ( sender == m_wdtCustomerFileListView.get() )
				m_wdtExpertFileListView->AllowDragAndDrop(IsAllowTransferFile());

			break;
		case cmd_DragDropCopy:
		{
			bool prevActivePanel = m_expertPanelIsActive;

			if ( sender == m_wdtExpertFileListView.get() )
				m_expertPanelIsActive = false;
			else if ( sender == m_wdtCustomerFileListView.get() )
				m_expertPanelIsActive = true;
			else
				break;

			CmdTransferOperations(false);
			m_expertPanelIsActive = prevActivePanel;
			break;
		}
		case cmd_ChangePanel:
			CmdActivatePanel(sender);
			break;
		case cmd_Size:
			CmdRearrangeWidgets(sender);
			break;
		case cmd_FilterChanged:
			CmdFilterChanged();
			break;
		case cmd_DefaultTextAppeared:
			if ( EFMS_STATES_COUNT != m_defaultMsgState )
			{
				m_wdtStatusImage->m_currentImage = m_uiStatusIcons[m_defaultMsgState - 1];
				m_wdtStatusImage->RedrawWindow();
				
				m_wdtDescriptionLabel->FontColor1 = m_wdtStatusImage->m_currentImage->GetPixel(m_wdtStatusImage->m_currentImage->GetWidth()/2,1);
				m_wdtDescriptionLabel->RedrawWindow();
			}
			break;
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdContentBrowse(const CLink& link, bool changeStatus)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CmdContentBrowse: link: %s"),(const TCHAR*)link);
	
	if ( !m_asyncFileManagerAvailable )
		return;
	
	m_asyncFileManager->CancelScanOperation(link);
	
	if ( link.IsLocal() )
	{
		m_wdtExpertFileListView->ClearWidgetItems();
		m_wdtExpertAddress->SetAddress(link);
		m_wdtExpertFileListView->EnableSorting(false);
	}
	else
	{
		m_wdtCustomerFileListView->ClearWidgetItems();
		m_wdtCustomerAddress->SetAddress(link);
		m_wdtCustomerFileListView->EnableSorting(false);
	}
	if ( link.IsLocal() )
		ChangeWidgetState(CMST_EXPERT_START_BROWSE);
	else
		ChangeWidgetState(CMST_CUSTOM_START_BROWSE);

	if ( changeStatus )
	{
		tstring strStatus(_T("Entered into "));
		if ( link.IsEmpty() )
			strStatus += _T("root");
		else
			strStatus += (const TCHAR*)link;
		strStatus += _T(" folder");
		ShowStatusMessage(strStatus,EFMS_ON);
	}

	m_asyncFileManager->ScanItems(link);
	
	Log.Add(_MESSAGE_,_T("END CCommandManager::CmdContentBrowse"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdRenameFile()
{
TRY_CATCH

	if ( m_expertPanelIsActive )
		m_wdtExpertFileListView->StartRenameCurrentItem();
	else
		m_wdtCustomerFileListView->StartRenameCurrentItem();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdRenameFileStart()
{
TRY_CATCH

	if ( m_expertPanelIsActive )
		m_wdtExpertFileListView->AllowItemRenaming(!m_currentExpertLink.IsEmpty());
	else
		m_wdtCustomerFileListView->AllowItemRenaming(!m_currentCustomerLink.IsEmpty());

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdRenameFileComplete(const bool expertSize, const void* itemId)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CmdRenameFileComplete"));

	tstring strStatus(_T("Renaming "));

	if ( expertSize )
	{
		tstring newName = m_wdtExpertFileListView->GetEnteredText();
		if ( !newName.size() )
			return;

		CLink oldLink = m_currentExpertLink;
		oldLink += m_wdtExpertFileListView->GetItemName(itemId);

		CLink newLink = m_currentExpertLink;
		newLink += newName;

		strStatus += oldLink.GetTargetName();
		ShowStatusMessage(strStatus,EFMS_ON);

		m_asyncFileManager->RenameItem(newLink,oldLink);
	}
	else
	{
		tstring newName = m_wdtCustomerFileListView->GetEnteredText();
		if ( !newName.size() )
			return;

		CLink oldLink = m_currentCustomerLink;
		oldLink += m_wdtCustomerFileListView->GetItemName(itemId);

		CLink newLink = m_currentCustomerLink;
		newLink += newName;

		ChangeWidgetState(CMST_RENAME_FILE);
		CmdActivatePanel(m_wdtExpertFileListView.get());

		strStatus += oldLink.GetTargetName();
		ShowStatusMessage(strStatus,EFMS_ON);

		m_asyncFileManager->RenameItem(newLink,oldLink);
	}

	Log.Add(_MESSAGE_,_T("END CCommandManager::CmdRenameFileComplete"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdCreateDirectory()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CmdCreateDirectory"));

	if ( m_expertPanelIsActive )
	{
		if ( !m_currentExpertLink.IsEmpty() )
			m_wdtExpertFileListView->StartCreateDirectory();
	}
	else
	{
		if ( !m_currentCustomerLink.IsEmpty() )
			m_wdtCustomerFileListView->StartCreateDirectory();
	}

	Log.Add(_MESSAGE_,_T("END CCommandManager::CmdCreateDirectory"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdCreateDirectoryComplete(const bool expertSize)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CmdCreateDirectoryComplete"));

	if ( expertSize )
	{
		tstring newName = m_wdtExpertFileListView->GetEnteredText();
		if ( !newName.size() )
			return;

		CLink drectoryName = m_currentExpertLink;
		drectoryName += newName;
		
		m_asyncFileManager->CreateDirectory(drectoryName);
	}
	else
	{
		tstring newName = m_wdtCustomerFileListView->GetEnteredText();

		if ( !newName.size() )
			return;

		CLink drectoryName = m_currentCustomerLink;
		drectoryName += newName;

		ChangeWidgetState(CMST_CREATE_DIRECTORY);
		CmdActivatePanel(m_wdtExpertFileListView.get());

		m_asyncFileManager->CreateDirectory(drectoryName);
	}
	tstring strStatus(_T("Creating new folder"));
	ShowStatusMessage(strStatus,EFMS_ON);

	Log.Add(_MESSAGE_,_T("END CCommandManager::CmdCreateDirectoryComplete"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdDeleteFile()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CmdDeleteFile"));

	const CLink* activeParentLink;
	boost::shared_ptr<filedata_vec_t> selItems;
	boost::shared_ptr<link_vec_t> linksToDelete(new link_vec_t);

	tstring strAlertText(_T("Are you sure you would like to delete\n"));

	if ( m_expertPanelIsActive )
	{
		selItems			= m_wdtExpertFileListView->GetSelectedItems();
		activeParentLink	= &m_currentExpertLink;

		if ( selItems->size() == 1 )
			strAlertText += selItems->begin()->GetFileName();
		else if ( selItems->size() > 1 )
			strAlertText += _T("the selected items");
		else
			return;
		strAlertText += _T(" from your computer?");
	}
	else
	{
		selItems			= m_wdtCustomerFileListView->GetSelectedItems();
		activeParentLink	= &m_currentCustomerLink;

		if ( selItems->size() == 1 )
			strAlertText += selItems->begin()->GetFileName();
		else if ( selItems->size() > 1 )
			strAlertText += _T("the selected items");
		else
			return;
		strAlertText += _T(" from the customer's computer?");
	}
	if ( CSkinnedMsgBox(this).Show(	( m_expertPanelIsActive ? CM_DLGID_DELETE_EXPERT : CM_DLGID_DELETE_CUSTOM ),
									strAlertText,
									true) == IDCANCEL )
		return;
	
	for ( filedata_vec_t::iterator iFD = selItems->begin(); iFD != selItems->end(); iFD++ )
	{
		CLink target = *activeParentLink;
		target += iFD->GetFileName();
		linksToDelete->push_back(target);

		ChangeWidgetState( (m_expertPanelIsActive ? CMST_DELETE_EXPERT_FILE : CMST_DELETE_CUSTOM_FILE) );
	}
	CmdActivatePanel(m_wdtExpertFileListView.get());

	tstring strStatus;
	if ( selItems->size() > 1 )
		strStatus = _T("Deleting files");
	else
	{
		strStatus = _T("Deleting ");
		strStatus += selItems->begin()->GetFileName();
		strStatus += _T(" file");
	}
	ShowStatusMessage(strStatus,EFMS_ON);

	m_asyncFileManager->DeleteItem(linksToDelete);

	Log.Add(_MESSAGE_,_T("END CCommandManager::CmdDeleteFile"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdTransferOperations(const bool moveOperations)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CmdTransferOperations: command - %s"),(moveOperations)?_T("Move"):_T("Copy"));

	const CLink* sourceParentLink;
	const CLink* destinationParentLink;
	boost::shared_ptr<filedata_vec_t> selItems;
	
	if ( m_expertPanelIsActive )
	{
		selItems				= m_wdtExpertFileListView->GetSelectedItems();
		sourceParentLink		= &m_currentExpertLink;
		destinationParentLink	= &m_currentCustomerLink;
	}
	else
	{
		selItems				= m_wdtCustomerFileListView->GetSelectedItems();
		sourceParentLink		= &m_currentCustomerLink;
		destinationParentLink	= &m_currentExpertLink;
	}
	if ( moveOperations )
	{
		tstring strAlertText(_T("Are you sure you would like to move\n"));

		if ( m_expertPanelIsActive )
		{
			if ( selItems->size() == 1 )
				strAlertText += selItems->begin()->GetFileName();
			else if ( selItems->size() > 1 )
				strAlertText += _T("the selected items");
			else
				return;
			strAlertText += _T(" to the customer's computer?");
		}
		else
		{
			if ( selItems->size() == 1 )
				strAlertText += selItems->begin()->GetFileName();
			else if ( selItems->size() > 1 )
				strAlertText += _T("the selected items");
			else
				return;
			strAlertText += _T(" to your computer?");
		}

		if ( CSkinnedMsgBox(this).Show(	( m_expertPanelIsActive ? CM_DLGID_MOVE_CUSTOM : CM_DLGID_MOVE_EXPERT ),
										strAlertText,
										true) == IDCANCEL )
			return;
	}

	for ( filedata_vec_t::iterator iSel = selItems->begin(); iSel != selItems->end(); iSel++ )
	{
		CLink source = *sourceParentLink;
		source += iSel->GetFileName();

		CLink destination = *destinationParentLink;
		destination += iSel->GetFileName();

		if ( moveOperations )
		{
			tstring strStatus(_T("Preparing to move "));
			if ( selItems->size() > 1 )
				strStatus += _T("selected files");
			else
				strStatus += selItems->begin()->GetFileName();
			ShowStatusMessage(strStatus,EFMS_ON);
			
			m_asyncFileManager->MoveItem(destination,source,*iSel);
		}
		else
		{
			tstring strStatus(_T("Preparing to copy "));
			if ( selItems->size() > 1 )
				strStatus += _T("selected files");
			else
				strStatus += selItems->begin()->GetFileName();
			ShowStatusMessage(strStatus,EFMS_ON);

			m_asyncFileManager->CopyItem(destination,source,*iSel);
		}
		ChangeWidgetState(CMST_TRANSFER);
	}
	CmdActivatePanel(m_wdtExpertFileListView.get());

	Log.Add(_MESSAGE_,_T("END CCommandManager::CmdTransferOperations"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdActivatePanel(const CCommandProxy* sender)
{
TRY_CATCH

	if ( sender == m_wdtExpertFileListView.get() )
	{
		m_expertPanelIsActive = true;
		ChangeWidgetState(CMST_EXPERT_FILE_SELECTED);

		return;
	}
	if ( sender == m_wdtCustomerFileListView.get() )
	{
		m_expertPanelIsActive = false;
		ChangeWidgetState(CMST_CUSTOM_FILE_SELECTED);

		return;
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdRearrangeWidgets(const CCommandProxy* sender)
{
TRY_CATCH

	if ( m_wdtToolbarPanel.get() && sender == m_wdtToolbarPanel.get() )
	{
		RECT rc;
		m_wdtToolbarPanel->GetWidgetRect(&rc);
		rc.left+=2;
		rc.top+=2;
		rc.bottom+=2;
		m_wdtToolBar->ResizeWidget(&rc);
	}
	if ( m_wdtFileViewExpertContainer.get() && sender == m_wdtFileViewExpertContainer.get() )
	{
		RECT rc;
		m_wdtFileViewExpertContainer->GetWidgetRect(&rc);
		
		RECT wrc;
		wrc = rc;
		wrc.top += 4;
		wrc.bottom	= 15;
		m_wdtExpertLabel->ResizeWidget(&wrc);

#define LEFT_CTRL_MARGIN 9
#define BTN_SIZE 20
#define TEXT_DX 5

		wrc.top		= wrc.bottom + 5;
		wrc.bottom	= wrc.top + BTN_SIZE;
		wrc.left	= LEFT_CTRL_MARGIN;
		wrc.right	= wrc.left + BTN_SIZE;
		m_wdtExpertRefresh->ResizeWidget(&wrc);

		wrc.left	= wrc.right	+ 5;
		wrc.right	= wrc.left + BTN_SIZE;
		m_wdtExpertUp->ResizeWidget(&wrc);

		wrc.right	= rc.right - 5;
		wrc.left	= wrc.right + 5;

		wrc.right	= wrc.left - 5;
		wrc.left	= LEFT_CTRL_MARGIN+BTN_SIZE+5+BTN_SIZE+5;
		m_wdtExpertAddress->ResizeWidget(&wrc);

		wrc = rc;
		wrc.left += LEFT_CTRL_MARGIN;
		wrc.top		= wrc.bottom - 17;
		m_wdtExpertFileInfo->ResizeWidget(&wrc);

		wrc = rc;
		wrc.left +=  LEFT_CTRL_MARGIN;
		wrc.bottom	= rc.bottom - 20;
		wrc.top		= wrc.bottom - 1;
		m_wdtExpertUnderline->ResizeWidget(&wrc);

		wrc = rc;
		wrc.left +=  LEFT_CTRL_MARGIN;
		wrc.top		= rc.top + 58;//5 + 24 + 5 + 15;
		wrc.bottom	= rc.bottom - 20 - 2;
		m_wdtExpertFileListView->ResizeWidget(&wrc);
	}
	if ( m_wdtFileViewCustomerContainer.get() && sender == m_wdtFileViewCustomerContainer.get() )
	{
		RECT rc;
		m_wdtFileViewCustomerContainer->GetWidgetRect(&rc);
		
		RECT wrc;
		wrc = rc;
		wrc.top += 4;
		wrc.bottom	= 15;
		m_wdtCustomerLabel->ResizeWidget(&wrc);

		wrc.top		= wrc.bottom + 5;
		wrc.bottom	= wrc.top + BTN_SIZE;
		wrc.left	= LEFT_CTRL_MARGIN;
		wrc.right	= wrc.left + BTN_SIZE;
		m_wdtCustomerRefresh->ResizeWidget(&wrc);

		wrc.left	= wrc.right + 5;
		wrc.right	= wrc.left + BTN_SIZE;
		m_wdtCustomerUp->ResizeWidget(&wrc);

		wrc.right	= rc.right - 5;
		wrc.left	= wrc.right + 5;

		wrc.right	= wrc.left - 5;
		wrc.left	= LEFT_CTRL_MARGIN+BTN_SIZE+5+BTN_SIZE+5;
		m_wdtCustomerAddress->ResizeWidget(&wrc);

		wrc = rc;
		wrc.left +=  LEFT_CTRL_MARGIN;
		wrc.top		= wrc.bottom - 17;
		m_wdtCustomerFileInfo->ResizeWidget(&wrc);

		wrc = rc;
		wrc.left +=  LEFT_CTRL_MARGIN;
		wrc.bottom	= rc.bottom - 20;
		wrc.top		= wrc.bottom - 1;
		m_wdtCustomerUnderline->ResizeWidget(&wrc);

		wrc = rc;
		wrc.left +=  LEFT_CTRL_MARGIN;
		wrc.top		= rc.top + 58;//5 + 24 + 5 + 15;
		wrc.bottom	= rc.bottom - 20 - 2;
		m_wdtCustomerFileListView->ResizeWidget(&wrc);

		if ( m_wdtRequestButton.get() )
		{
			SIZE size = m_wdtRequestButton->GetTextSize();
			wrc.left	= (rc.right - rc.left - size.cx)/2;
			wrc.right	= wrc.left + size.cx;
			wrc.top		= (rc.bottom - rc.top - size.cy)/2;
			wrc.bottom	= wrc.top + size.cy;
			m_wdtRequestButton->ResizeWidget(&wrc);
		}
	}
	if ( m_wdtHistoryPanel.get() && sender == m_wdtHistoryPanel.get() )
	{
		RECT rc;
		m_wdtHistoryPanel->GetWidgetRect(&rc);
		
		RECT wrc;
		wrc = rc;
		wrc.top		= 7;
		wrc.left	= LEFT_CTRL_MARGIN;
		wrc.bottom	= wrc.top;
		wrc.right	= wrc.left;
		m_wdtExpandHistoryButton->ResizeWidget(&wrc);
		m_wdtExpandHistoryButton->AdjustSize();

		wrc = rc;
		wrc.left += LEFT_CTRL_MARGIN/2;
		wrc.top += 31;
		wrc.bottom -= 8;
		m_wdtHistoryListView->ResizeWidget(&wrc);
	}
	if ( m_wdtStatusPanel.get() && sender == m_wdtStatusPanel.get() )
	{
		RECT rc;
		m_wdtStatusPanel->GetWidgetRect(&rc);

		RECT wrc;
		wrc = rc;
		wrc.right	-= 5;
		wrc.left	= wrc.right - 50;
		wrc.top		= rc.top + 7;
		wrc.bottom	= wrc.top + 14;
		m_wdtCancelTransferButton->ResizeWidget(&wrc);

		wrc.right	= wrc.left - 7;
		wrc.left	= wrc.right - 270;
		m_wdtProgressBar->ResizeWidget(&wrc);

		if (m_wdtProgressBar->IsWindowVisible())
			wrc.right	= wrc.left - 5;

		wrc.left	= LEFT_CTRL_MARGIN;
		wrc.top		= rc.top + 7;
		wrc.bottom	= rc.bottom - 5;
	
		int cx = 0, cy = 0;
		if (NULL != m_wdtStatusDots->m_currentImage)
		{
			cx = m_wdtStatusDots->m_currentImage->GetWidth();
			cy = m_wdtStatusDots->m_currentImage->GetHeight();
		}
		m_wdtStatusDots->SetWindowPos(0, wrc.left, wrc.top + 2, cx, cy, SWP_NOZORDER);
		m_wdtStatusDots->RedrawWindow();
		wrc.left += cx + TEXT_DX;

		cx = cy = 0;
		if (NULL != m_wdtStatusImage->m_currentImage)
		{
			cx = m_wdtStatusImage->m_currentImage->GetWidth();
			cy = m_wdtStatusImage->m_currentImage->GetHeight();
		}
		m_wdtStatusImage->SetWindowPos(0, wrc.left, wrc.top, cx, cy, SWP_NOZORDER);
		wrc.left += cx + TEXT_DX;
		
		m_wdtDescriptionLabel->ResizeWidget(&wrc);
	}

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdResizeHistoryWidget(const bool expandWidget)
{
TRY_CATCH

	RECT rcHistory;
	m_wdtHistoryPanel->GetWidgetRect(&rcHistory);
	RECT rcFVContainer;
	m_wdtFileViewContainer->GetWidgetRect(&rcFVContainer);
	RECT rcMain;
	GetClientRect(m_mainWnd,&rcMain);
	
	if ( expandWidget )
	{
		m_wdtExpandHistoryButton->SetText(HIDE_HISTORY_TEXT);
		m_wdtExpandHistoryButton->SetImages(IDR_BTN_HIDE_NORMAL, IDR_BTN_HIDE_MOUSEOVER);
		m_wdtExpandHistoryButton->OnSkinChanged();

		rcHistory.bottom	= rcMain.bottom - m_locationStatusPanelHeigth;
		rcHistory.top		= rcHistory.bottom - m_locationHistoryPanelHeigthExpand;
		m_wdtHistoryPanel->ResizeWidget(&rcHistory);

		rcFVContainer.top		= m_locationToolBarHeight;
		rcFVContainer.bottom	= rcHistory.top;
		m_wdtFileViewContainer->ResizeWidget(&rcFVContainer);

		m_wdtFileViewContainer->GetWidgetRect(&rcFVContainer);
		m_wdtSplitter->ResizeWidget(&rcFVContainer);

		m_wdtHistoryListView->ShowWindow(SW_SHOW);
	}
	else
	{
		m_wdtExpandHistoryButton->SetText(SHOW_HISTORY_TEXT);
		m_wdtExpandHistoryButton->SetImages(IDR_BTN_SHOW_NORMAL, IDR_BTN_SHOW_MOUSEOVER);
		m_wdtExpandHistoryButton->OnSkinChanged();

		m_wdtHistoryListView->ShowWindow(SW_HIDE);

		rcHistory.bottom	= rcMain.bottom - m_locationStatusPanelHeigth;
		rcHistory.top		= rcHistory.bottom - m_locationHistoryPanelHeigthCollapse;
		m_wdtHistoryPanel->ResizeWidget(&rcHistory);

		rcFVContainer.top		= m_locationToolBarHeight;
		rcFVContainer.bottom	= rcHistory.top;
		m_wdtFileViewContainer->ResizeWidget(&rcFVContainer);

		m_wdtFileViewContainer->GetWidgetRect(&rcFVContainer);
		m_wdtSplitter->ResizeWidget(&rcFVContainer);
	}

	m_wdtHistoryPanel->Invalidate();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::CmdFilterChanged()
{
TRY_CATCH

	m_fileFilter.reset(new CFileFilter(DRIVE_REMOTE,m_wdtToolBar->GetDisabledAttributes()));

	m_asyncFileManager->SetScanAttributesFilter(m_fileFilter);

	CmdContentBrowse(m_currentExpertLink, false);
	if ( m_customIsConnected && m_lockCounter == 0 )
		CmdContentBrowse(m_currentCustomerLink, false);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::ChangeWidgetState(unsigned int stateAction)
{
TRY_CATCH

//	Log.Add(_MESSAGE_,_T("BGN CCommandManager::ChangeWidgetState: lockcounter: %d, action: %u"),m_lockCounter,stateAction);

	switch ( stateAction )
	{
		case CMST_TRANSFER_CANCELED:
		case CMST_DELETE_EXPERT_FILE:
		case CMST_DELETE_CUSTOM_FILE:
		case CMST_DELETE_EXPERT_COMPLETE:
		case CMST_DELETE_CUSTOM_COMPLETE:
		case CMST_TRANSFER:
		case CMST_TRANSFER_COMPLETE:
			break;
		default:
			if ( stateAction == m_currentState )
				return;
	}

	switch ( stateAction )
	{
		case CMST_RENAME_FILE:
			m_wdtCustomerUp->LockWidget();
			m_wdtCustomerRefresh->LockWidget();
			m_wdtCustomerFileListView->LockWidget();
			break;
		case CMST_RENAME_FILE_COMPLETE:
			m_wdtCustomerUp->UnlockWidget();
			m_wdtCustomerRefresh->UnlockWidget();
			m_wdtCustomerFileListView->UnlockWidget();
			m_wdtCustomerFileListView->SetWidgetFocus();
			break;
		case CMST_CREATE_DIRECTORY:
			m_wdtCustomerUp->LockWidget();
			m_wdtCustomerRefresh->LockWidget();
			m_wdtCustomerFileListView->LockWidget();
			break;
		case CMST_CREATE_DIRECTORY_COMPLETE:
			m_wdtCustomerUp->UnlockWidget();
			m_wdtCustomerRefresh->UnlockWidget();
			m_wdtCustomerFileListView->UnlockWidget();
			m_wdtCustomerFileListView->SetWidgetFocus();
			break;
		case CMST_DELETE_EXPERT_FILE:
			InterlockedIncrement(&m_lockCounter);
			break;
		case CMST_DELETE_EXPERT_COMPLETE:
			InterlockedDecrement(&m_lockCounter);
			break;
		case CMST_DELETE_CUSTOM_FILE:
		case CMST_TRANSFER:
			if ( InterlockedIncrement(&m_lockCounter) == 1 )
			{
				m_wdtCustomerUp->LockWidget();
				m_wdtCustomerRefresh->LockWidget();
				m_wdtCustomerFileListView->LockWidget();
			}
			break;
		case CMST_TRANSFER_CANCELED:
			InterlockedExchange(&m_lockCounter,0);
			m_wdtCustomerUp->UnlockWidget();
			m_wdtCustomerRefresh->UnlockWidget();
			m_wdtCustomerFileListView->UnlockWidget();
			break;
		case CMST_DELETE_CUSTOM_COMPLETE:
		case CMST_TRANSFER_COMPLETE:
			if ( InterlockedDecrement(&m_lockCounter) <= 0 )
			{
				InterlockedExchange(&m_lockCounter,0);
				m_wdtCustomerUp->UnlockWidget();
				m_wdtCustomerRefresh->UnlockWidget();
				m_wdtCustomerFileListView->UnlockWidget();
			}
			break;
		case CMST_CUSTOM_CONNECTED:
			m_customIsConnected = true;
			m_wdtCustomerFileListView->ShowWindow(SW_SHOW);
			m_wdtCustomerUnderline->ShowWindow(SW_SHOW);
			m_wdtCustomerFileInfo->ShowWindow(SW_SHOW);
			m_wdtCustomerUp->UnlockWidget();
			m_wdtCustomerRefresh->UnlockWidget();
			break;
		case CMST_CUSTOM_DISCONNECTED:
			m_customIsConnected = false;
			m_wdtCustomerFileListView->ShowWindow(SW_HIDE);
			m_wdtCustomerUnderline->ShowWindow(SW_HIDE);
			m_wdtCustomerFileInfo->ShowWindow(SW_HIDE);
			m_wdtCustomerUp->LockWidget();
			m_wdtCustomerRefresh->LockWidget();
			break;
		case CMST_EXPERT_START_BROWSE:
			m_wdtExpertUp->LockWidget();
			m_wdtExpertRefresh->LockWidget();
			m_wdtExpertFileListView->LockWidget();
			m_wdtToolBar->LockWidget();
			m_wdtToolBar->UnlockWidget(cmd_FilterChanged);
			m_expertBrowseInProcess = true;
			break;
		case CMST_CUSTOM_START_BROWSE:
			m_wdtCustomerUp->LockWidget();
			m_wdtCustomerRefresh->LockWidget();
			m_wdtCustomerFileListView->LockWidget();
			m_wdtToolBar->LockWidget();
			m_wdtToolBar->UnlockWidget(cmd_FilterChanged);
			m_customBrowseInProcess = true;
			break;
		case CMST_EXPERT_FILE_SELECTED:
			m_wdtToolBar->LockWidget();
			m_wdtToolBar->UnlockWidget(cmd_FilterChanged);
			if ( !m_currentExpertLink.IsEmpty() )
			{
				m_wdtExpertUp->UnlockWidget();
				if ( m_wdtExpertFileListView->GetSelectedItemCount() )
					m_wdtToolBar->UnlockWidget();
				else
					m_wdtToolBar->UnlockWidget(cmd_CreateDirectory);
				
				if ( m_expertBrowseInProcess )
					m_wdtToolBar->LockWidget(cmd_RenameFile);
			}
			if ( !IsAllowTransferFile() )
			{
				m_wdtToolBar->LockWidget(cmd_CopyFile);
				m_wdtToolBar->LockWidget(cmd_MoveFile);
			}

			stateAction = CMST_EXPERT_BROWSE;
			break;
		case CMST_EXPERT_BROWSE_COMPLETE:
			m_expertBrowseInProcess = false;
			m_wdtExpertFileListView->SetWidgetFocus();
		case CMST_EXPERT_BROWSE:
			m_wdtToolBar->LockWidget();
			m_wdtToolBar->UnlockWidget(cmd_FilterChanged);
			if ( !m_currentExpertLink.IsEmpty() )
			{
				m_wdtExpertUp->UnlockWidget();
				if ( m_wdtExpertFileListView->GetSelectedItemCount() )
					m_wdtToolBar->UnlockWidget();
				else
					m_wdtToolBar->UnlockWidget(cmd_CreateDirectory);
			}
			if ( !IsAllowTransferFile() )
			{
				m_wdtToolBar->LockWidget(cmd_CopyFile);
				m_wdtToolBar->LockWidget(cmd_MoveFile);
			}

			m_wdtExpertRefresh->UnlockWidget();
			m_wdtExpertFileListView->UnlockWidget();
			break;
		case CMST_CUSTOM_FILE_SELECTED:
			m_wdtToolBar->LockWidget();
			m_wdtToolBar->UnlockWidget(cmd_FilterChanged);
			if ( !m_currentCustomerLink.IsEmpty() )
			{
				m_wdtCustomerUp->UnlockWidget();
				if ( m_wdtCustomerFileListView->GetSelectedItemCount() )
					m_wdtToolBar->UnlockWidget();
				else
					m_wdtToolBar->UnlockWidget(cmd_CreateDirectory);

				if ( m_customBrowseInProcess )
					m_wdtToolBar->LockWidget(cmd_RenameFile);
			}
			if ( !IsAllowTransferFile() )
			{
				m_wdtToolBar->LockWidget(cmd_CopyFile);
				m_wdtToolBar->LockWidget(cmd_MoveFile);
			}

			stateAction = CMST_CUSTOM_BROWSE;
			break;
		case CMST_CUSTOM_BROWSE_COMPLETE:
			m_customBrowseInProcess = false;
			m_wdtCustomerFileListView->SetWidgetFocus();
		case CMST_CUSTOM_BROWSE:
			m_wdtToolBar->LockWidget();
			m_wdtToolBar->UnlockWidget(cmd_FilterChanged);
			if ( !m_currentCustomerLink.IsEmpty() )
			{
				m_wdtCustomerUp->UnlockWidget();
				if ( m_wdtCustomerFileListView->GetSelectedItemCount() )
					m_wdtToolBar->UnlockWidget();
				else
					m_wdtToolBar->UnlockWidget(cmd_CreateDirectory);
			}
			if ( !IsAllowTransferFile() )
			{
				m_wdtToolBar->LockWidget(cmd_CopyFile);
				m_wdtToolBar->LockWidget(cmd_MoveFile);
			}

			m_wdtCustomerRefresh->UnlockWidget();
			m_wdtCustomerFileListView->UnlockWidget();
			break;
	}
	m_currentState = stateAction;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CCommandManager::IsAllowTransferFile()
{
TRY_CATCH

	return !( !m_customIsConnected || m_lockCounter || m_currentCustomerLink.IsEmpty() || m_currentExpertLink.IsEmpty() );

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::SetDefaultStatusMessage(const tstring& strMessage, const UINT elapse, const EFMStateEx state)
{
TRY_CATCH

	if ( !m_widgetsCreated )
		return;

	m_defaultMsgState = state;

	if (EFMS_STATES_COUNT != state)
	{
		m_wdtStatusImage->m_currentImage = m_uiStatusIcons[state - 1];
		m_wdtStatusImage->RedrawWindow();
		
		m_wdtDescriptionLabel->FontColor1 = m_wdtStatusImage->m_currentImage->GetPixel(m_wdtStatusImage->m_currentImage->GetWidth()/2,1);
	}

	m_wdtDescriptionLabel->InitDefaultText(strMessage,elapse);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CCommandManager::CCommandManager()
	:	CAbstractCommandManager(NULL,boost::shared_ptr<CSkinsImageList>(new CSkinsImageList())),
		m_transferLog(),
		m_widgetsCreated(false),
		m_currentExpertLink(_T("")),
		m_currentCustomerLink(_T(""),false),
		m_expertPanelIsActive(true),
		m_lockCounter(0),
		m_requestHandler(NULL),
		m_historyWidgetExpanded(false),
		m_customIsConnected(false),
		m_customBrowseInProcess(false),
		m_expertBrowseInProcess(false),
		m_asyncFileManagerAvailable(false),
		m_defaultMsgState(EFMS_STATES_COUNT),
		m_currentMsgState(EFMS_STATES_COUNT)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::CCommandManager"));
	
	// Calculating difference between UTC and local time
	FILETIME utcFt, localFt;
	GetSystemTimeAsFileTime(&utcFt);
	FileTimeToLocalFileTime(&utcFt,&localFt);
	
	LARGE_INTEGER liUtc, liLocal;
	liUtc.HighPart		= utcFt.dwHighDateTime;
	liUtc.LowPart		= utcFt.dwLowDateTime;
	liLocal.HighPart	= localFt.dwHighDateTime;
	liLocal.LowPart		= localFt.dwLowDateTime;
		
	liLocal.QuadPart -= liUtc.QuadPart;
	// Trunc to minutes
	liLocal.QuadPart /= 600000000L;
	TCHAR prefix[20];
	_stprintf_s(prefix,
				sizeof(prefix)/sizeof(prefix[0]),
				_T(" GMT (%+.2d:00) "),				// Don't print a minute difference, it looks ugly
				liLocal.QuadPart/60);

	m_transferLog.SetMessagePrefix(prefix);
	
	m_dirLogging.reset(new CDirContentLogging(&m_transferLog));
	
	// Turn off windows theme for this application to prevent sound producing while CWidgetFileListView selects items
	//SetThemeAppProperties(0);
	
	// Delete registry key to prevent sound producing
	RegDeleteKey(HKEY_CURRENT_USER,_T("AppEvents\\Schemes\\Apps\\.Default\\CCSelect\\.Current"));

	Log.Add(_MESSAGE_,_T("END CCommandManager::CCommandManager"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CCommandManager::~CCommandManager()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::~CCommandManager"));

	Log.Add(_MESSAGE_,_T("END CCommandManager::~CCommandManager"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::InitManager(HWND hMainWnd, boost::shared_ptr<CCommandManager> commandManager)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::InstanceManager"));
	
	m_mainWnd = hMainWnd;
	
	const unsigned int attributesToDisable = 0;//FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM;

	m_fileFilter.reset(new CFileFilter(DRIVE_REMOTE,attributesToDisable));
	
	m_asyncFileManager.reset(new CAsyncFileManager());
	m_asyncFileManager->SetScanAttributesFilter(m_fileFilter);
	m_asyncFileManager->SetTransferHandler(boost::bind(&CCommandManager::TransferHandler,this,_1,_2,_3,_4));
	m_asyncFileManager->SetUncompressHandler(boost::bind(&CCommandManager::UncompressHandler,this,_1));
	m_asyncFileManager->SetTransferCompletionHandler(boost::bind(&CCommandManager::TransferCompletionHandler,this,_1,_2,_3,_4,_5,_6));
	m_asyncFileManager->SetAskForOverwriteHandler(boost::bind(&CCommandManager::AskForOverwriteHandler,this,_1,_2,_3,_4));
	m_asyncFileManager->SetTransferCompressingHandler(boost::bind(&CCommandManager::TransferCompressingHandler,this,_1));
	m_asyncFileManager->SetTransferMoveDeleteHandler(boost::bind(&CCommandManager::TransferMoveDeleteHandler,this,_1,_2,_3));

	m_asyncFileManager->SetCreateOperationHandler(boost::bind(&CCommandManager::CreateOperationHandler,this,_1,_2,_3));
	m_asyncFileManager->SetRenameOperationHandler(boost::bind(&CCommandManager::RenameOperationHandler,this,_1,_2,_3,_4));
	m_asyncFileManager->SetDeleteOperationHandler(boost::bind(&CCommandManager::DeleteOperationHandler,this,_1,_2,_3));
	m_asyncFileManager->SetScanHandler(boost::bind(&CCommandManager::ScanHandler,this,_1,_2));
	m_asyncFileManager->SetScanCompletionHandler(boost::bind(&CCommandManager::ScanCompletionHandler,this,_1));
	
	m_asyncFileManagerAvailable = true;
	
	//+create fonts
	HFONT font = CWindow(m_mainWnd).GetFont();
	ATLASSERT(::GetObjectType(font) == OBJ_FONT);
	LOGFONT logFont;
	GetObject(font, sizeof(LOGFONT),&logFont);
	_tcscpy_s(logFont.lfFaceName, LF_FACESIZE, _T("Verdana")); 
	logFont.lfHeight = -11;
	logFont.lfHeight = -11;
	Font = logFont;
	LOGFONT boldFont = Font;
	boldFont.lfWeight = FW_BOLD;
	LOGFONT underlinedFont = Font;
	underlinedFont.lfUnderline = TRUE;
	LOGFONT underlinedBoldFont = boldFont;
	underlinedBoldFont.lfUnderline = TRUE;
	//-create fonts

	/// Loading status icons
	/// resource identifer of statues icon
	const int uiStatusIconsId[EFMS_STATES_COUNT]={	IDR_UIS_PERMISSION_REQUEST_SEND,
													IDR_UIS_PERMISSION_RECIEVED,
													IDR_UIS_PERMISSION_DENIED,
													IDR_UIS_INSTALLATION_INPROGRESS,
													IDR_UIS_INSTALLATION_SUCCESSFUL,
													IDR_UIS_INSTALLATION_FAILED,
													IDR_UIS_SESSION_CONNECTING,
													IDR_UIS_SESSION_FAILED,
													IDR_UIS_SESSION_OFF,
													IDR_UIS_SESSION_ON};

	if (NULL != m_skinsImageList.get())
		m_skinsImageList->ImageFromRes(m_uiStatusIcons,EFMS_STATES_COUNT,uiStatusIconsId);
	else
		Log.Add(_WARNING_,_T("NULL images list while creating file manager UI"));
	
	m_wdtToolbarPanel.reset(new CWidgetPanel(m_mainWnd,commandManager,IDR_TOOLBARPANEL_BGRND));
	m_wdtToolBar.reset(new CWidgetToolBar(m_wdtToolbarPanel->m_hWnd,commandManager));
	m_wdtToolBar->SetDisabledAttributes(attributesToDisable);
	m_wdtToolBar->Font = Font;
	m_wdtToolBar->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtToolBar->OnSkinChanged();

	m_wdtFileViewContainer.reset(new CWidgetContainer(m_mainWnd,commandManager, IDR_FILEVIEW_MIDDLE, IDR_FILEVIEW_HEADER));
	m_wdtSplitter.reset(new CWidgetSplitter(m_wdtFileViewContainer->m_hWnd,commandManager));
	m_wdtFileViewExpertContainer.reset(new CWidgetPanel(m_wdtSplitter->m_hWnd,commandManager, IDR_FILEVIEW_MIDDLE, IDR_FILEVIEW_HEADER));
	m_wdtFileViewCustomerContainer.reset(new CWidgetPanel(m_wdtSplitter->m_hWnd,commandManager, IDR_FILEVIEW_MIDDLE, IDR_FILEVIEW_HEADER));
	m_wdtSplitter->SetControlledPanels(m_wdtFileViewExpertContainer,m_wdtFileViewCustomerContainer);

	m_wdtStatusPanel.reset(new CWidgetPanel(m_mainWnd,commandManager,IDR_SIMPLE_PANEL_1x1));
	m_wdtHistoryPanel.reset(new CWidgetPanel(m_mainWnd,commandManager,IDR_FILEVIEW_MIDDLE,IDR_HISTORY_HEADER,IDR_HISTORY_FOOTER));


	m_wdtExpertLabel.reset(new CWidgetLabel(m_wdtFileViewExpertContainer->m_hWnd,commandManager,m_wdtFileViewExpertContainer.get()));
	m_wdtExpertAddress.reset(new CWidgetAddress(m_wdtFileViewExpertContainer->m_hWnd,commandManager));
	m_wdtExpertAddress->Font = Font;
	m_wdtExpertAddress->EdgeColor1 = SCVUI_LBLFONTCOLOR1;

	m_wdtExpertUp.reset(new CWidgetBitmapButton(m_wdtFileViewExpertContainer->m_hWnd,
												commandManager,
												IDR_BTN_UP_NORMAL_24x24,
												IDR_BTN_UP_DISABLE_24x24,
												IDR_BTN_UP_PRESSED_24x24,
												IDR_BTN_UP_MOUSEOVER_24x24));

	m_wdtExpertRefresh.reset(new CWidgetBitmapButton(	m_wdtFileViewExpertContainer->m_hWnd,
														commandManager,
														IDR_BTN_REFRESH_NORMAL_24x24,
														IDR_BTN_REFRESH_DISABLE_24x24,
														IDR_BTN_REFRESH_PRESSED_24x24,
														IDR_BTN_REFRESH_MOUSEOVER_24x24));

	m_wdtExpertFileInfo.reset(new CWidgetLabel(m_wdtFileViewExpertContainer->m_hWnd,commandManager,m_wdtFileViewExpertContainer.get()));
	m_wdtExpertUnderline.reset(new CWidgetStatic(m_wdtFileViewExpertContainer->m_hWnd,commandManager));
	m_wdtExpertUnderline->FontColor1 = SCVUI_LBLFONTCOLOR1;
	
	m_wdtCustomerLabel.reset(new CWidgetLabel(m_wdtFileViewCustomerContainer->m_hWnd,commandManager,m_wdtFileViewCustomerContainer.get()));
	m_wdtCustomerAddress.reset(new CWidgetAddress(m_wdtFileViewCustomerContainer->m_hWnd,commandManager));
	m_wdtCustomerAddress->Font = Font;
	m_wdtCustomerAddress->EdgeColor1 = SCVUI_LBLFONTCOLOR1;

	m_wdtCustomerUp.reset(new CWidgetBitmapButton(	m_wdtFileViewCustomerContainer->m_hWnd,
													commandManager,
													IDR_BTN_UP_NORMAL_24x24,
													IDR_BTN_UP_DISABLE_24x24,
													IDR_BTN_UP_PRESSED_24x24,
													IDR_BTN_UP_MOUSEOVER_24x24));
	m_wdtCustomerRefresh.reset(new CWidgetBitmapButton(	m_wdtFileViewCustomerContainer->m_hWnd,
														commandManager,
														IDR_BTN_REFRESH_NORMAL_24x24,
														IDR_BTN_REFRESH_DISABLE_24x24,
														IDR_BTN_REFRESH_PRESSED_24x24,
														IDR_BTN_REFRESH_MOUSEOVER_24x24));

	m_wdtCustomerFileInfo.reset(new CWidgetLabel(m_wdtFileViewCustomerContainer->m_hWnd,commandManager,m_wdtFileViewCustomerContainer.get()));
	m_wdtCustomerUnderline.reset(new CWidgetStatic(m_wdtFileViewCustomerContainer->m_hWnd,commandManager));
	m_wdtCustomerUnderline->FontColor1 = SCVUI_LBLFONTCOLOR1;

	
	m_wdtExpertLabel->SetText(_T("Your PC"));
	m_wdtExpertLabel->Font = boldFont;
	m_wdtExpertLabel->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtExpertLabel->SetTextAlign(DT_CENTER);
	m_wdtExpertLabel->OnSkinChanged();

	m_wdtCustomerLabel->SetText(_T("Customer PC"));
	m_wdtCustomerLabel->Font = boldFont;
	m_wdtCustomerLabel->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtCustomerLabel->SetTextAlign(DT_CENTER);
	m_wdtCustomerLabel->OnSkinChanged();

	m_wdtExpertFileInfo->SetText(_T(""));
	m_wdtExpertFileInfo->Font = Font;
	m_wdtExpertFileInfo->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtExpertFileInfo->SetTextAlign(DT_LEFT);
	m_wdtExpertFileInfo->OnSkinChanged();

	m_wdtCustomerFileInfo->SetText(_T(""));
	m_wdtCustomerFileInfo->Font = Font;
	m_wdtCustomerFileInfo->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtCustomerFileInfo->SetTextAlign(DT_LEFT);
	m_wdtCustomerFileInfo->OnSkinChanged();

	m_wdtExpertFileListView.reset(new CWidgetFileListView(m_wdtFileViewExpertContainer->m_hWnd,commandManager));
	m_wdtExpertFileListView->FontColor1 = SCVUI_BTNFONTCOLOR1;

	m_wdtCustomerFileListView.reset(new CWidgetFileListView(m_wdtFileViewCustomerContainer->m_hWnd,commandManager));
	m_wdtCustomerFileListView->FontColor1 = SCVUI_BTNFONTCOLOR1;

	m_wdtRequestButton.reset(new CWidgetLinkLabel(	m_wdtFileViewCustomerContainer->m_hWnd,
													commandManager,
													m_wdtFileViewCustomerContainer.get() ));
	m_wdtRequestButton->SetText(_T("Click here to request customer approval"));
	m_wdtRequestButton->Font = underlinedFont;
	m_wdtRequestButton->Font2 = underlinedFont;
	m_wdtRequestButton->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtRequestButton->FontColor2 = SCVUI_BTNFONTCOLOR2;
	m_wdtRequestButton->BkColor1 = SCVUI_BTNFONTCOLOR2;
	m_wdtRequestButton->SetTextAlign(DT_CENTER);
	m_wdtRequestButton->OnSkinChanged();

	m_wdtHistoryListView.reset(new CWidgetHistoryListView(	m_wdtHistoryPanel->m_hWnd,
															commandManager,
															IDR_LV_BRUSH_SKIN_1x1,
															IDR_LV_TOP_SKIN_1x8,
															IDR_LV_BOTTOM_SKIN_1x8));
															
	m_wdtHistoryListView->FontColor1 = SCVUI_BTNFONTCOLOR1;

	m_wdtExpandHistoryButton.reset(new CWidgetLinkLabelEx(	m_wdtHistoryPanel->m_hWnd,
															commandManager,
															IDR_BTN_SHOW_NORMAL,
															IDR_BTN_SHOW_MOUSEOVER,
															m_wdtHistoryPanel.get()));
	m_wdtExpandHistoryButton->SetText(SHOW_HISTORY_TEXT);
	m_wdtExpandHistoryButton->Font = boldFont;
	m_wdtExpandHistoryButton->Font2 = boldFont;
	m_wdtExpandHistoryButton->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtExpandHistoryButton->FontColor2 = SCVUI_BTNFONTCOLOR2;
	m_wdtExpandHistoryButton->BkColor1 = SCVUI_BTNFONTCOLOR2;
	m_wdtExpandHistoryButton->OnSkinChanged();

	m_wdtDescriptionLabel.reset(new CWidgetTimerLabel(m_wdtStatusPanel->m_hWnd,commandManager,m_wdtStatusPanel.get()));
	m_wdtDescriptionLabel->Font = Font;
	m_wdtDescriptionLabel->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtDescriptionLabel->SetTextAlign(DT_LEFT);
	m_wdtDescriptionLabel->OnSkinChanged();
	m_wdtDescriptionLabel->SetText(_T("File manager session is OFF"));

	m_wdtCancelTransferButton.reset(new CWidgetLinkLabel(	m_wdtStatusPanel->m_hWnd,
															commandManager,
															m_wdtStatusPanel.get() ));
	m_wdtCancelTransferButton->SetText(_T("Cancel"));
	m_wdtCancelTransferButton->Font = Font;
	m_wdtCancelTransferButton->Font2 = underlinedFont;
	m_wdtCancelTransferButton->FontColor1 = SCVUI_LBLFONTCOLOR1;
	m_wdtCancelTransferButton->FontColor2 = SCVUI_BTNFONTCOLOR2;
	m_wdtCancelTransferButton->BkColor1 = m_wdtRequestButton->FontColor1;
	m_wdtCancelTransferButton->SetTextAlign(DT_CENTER);
	m_wdtCancelTransferButton->OnSkinChanged();

	m_wdtProgressBar.reset(new CWidgetProgress(m_wdtStatusPanel->m_hWnd,commandManager));
	m_wdtProgressBar->ShowWidget(false);

	m_wdtStatusImage.reset(new CSkinnedStatic());
	m_wdtStatusImage->Create(m_wdtStatusPanel->m_hWnd);
	
	m_wdtStatusDots.reset(new CSkinnedStatic());
	m_wdtStatusDots->Create(m_wdtStatusPanel->m_hWnd);
	
	int dotsId = IDR_UIS_STATUSDOTS_11_11;
	m_skinsImageList->ImageFromRes(&m_wdtStatusDots->m_currentImage,1,&dotsId);

	SetDefaultStatusMessage(_T("File manager session is OFF"),CM_DEFAULTTEXT_ELAPSE,EFMS_INIFIAL);

	ChangeWidgetState(CMST_CUSTOM_DISCONNECTED);

	m_widgetsCreated = true;

	RECT rc = {0,0,100,100};
	m_wdtSplitter->SetSplitterRect(&rc);
	m_wdtSplitter->SetSplitterPos();
	
	m_wdtExpertRefresh->PostCommand();

	m_wdtCustomerFileListView->ShowWindow(SW_HIDE);
	m_wdtCustomerUnderline->ShowWindow(SW_HIDE);
	m_wdtCustomerFileInfo->ShowWindow(SW_HIDE);
	m_wdtCancelTransferButton->ShowWidget(false);

	ShowStatusMessage(_T("File manager session is OFF"), EFMS_INIFIAL);

//	::ShowWindow(m_mainWnd,SW_SHOW);

	Log.Add(_MESSAGE_,_T("END CCommandManager::InstanceManager"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::InitRemoteSide(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::InitRemoteSide"));

	m_asyncFileManager->SetRemoteOperationsStream(stream);
	
	m_wdtRequestButton->ShowWindow(SW_HIDE);

	ChangeWidgetState(CMST_CUSTOM_CONNECTED);

	m_wdtCustomerRefresh->PostCommand();

	ShowStatusMessage(_T("File manager session is ON"), EFMS_ON);
	SetDefaultStatusMessage(_T("File manager session is ON"),CM_DEFAULTTEXT_ELAPSE,EFMS_ON);

	Log.Add(_MESSAGE_,_T("END CCommandManager::InitRemoteSide"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::OnRemoteSideDisconnected(const ESessionStopReason stopReason)
{
TRY_CATCH

	if ( m_wdtCancelTransferButton->IsWindowVisible() )
	{
		m_asyncFileManager->CancelAllTransferOperations();
		//WaitForSingleObjectWithMessageLoop
		ChangeWidgetState(CMST_TRANSFER_CANCELED);
	}
	else if ( m_lockCounter )
	{	
		// If any operations is occuring - show MessageBox
		CSkinnedMsgBox msgBox(this);
		msgBox.SetMessage(_T("Operation was canceled by customer\nShutting down file manager session"));
		msgBox.SetShowCancelBtnFlag(false);
		msgBox.SetShowCheckBoxFlag(false);
		msgBox.ShowDialog(false);
		m_asyncFileManager->CancelAllTransferOperations();
		while( m_lockCounter )
			WaitForSingleObjectWithMessageLoop(GetCurrentThread(), CM_WAITCANCEL_ELAPSE);
		msgBox.CloseDialog();

	}
	ChangeWidgetState(CMST_CUSTOM_DISCONNECTED);
	m_wdtRequestButton->ShowWindow(SW_SHOW);
	m_wdtRequestButton->EnableWindow();
	m_wdtRequestButton->Text = _T("Connection with the customer has been lost\nClick here to reconnect");
	ExecuteCommand(m_wdtFileViewCustomerContainer.get(), cmd_Size, 0);

	switch ( stopReason )
	{
		case REMOTE_STOP:
			SetDefaultStatusMessage(BRT_SERVICE_STOPPED_BY_CUSTOMER,CM_DEFAULTTEXT_ELAPSE, EFMS_SESSION_FAILED);
			ShowStatusMessage(BRT_SERVICE_STOPPED_BY_CUSTOMER, EFMS_SESSION_FAILED);
			break;
		default:
			SetDefaultStatusMessage(_T("File manager session is OFF"),CM_DEFAULTTEXT_ELAPSE,EFMS_INIFIAL);
			ShowStatusMessage(_T("File manager session is OFF"), EFMS_INIFIAL);
			break;
	}
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::DestroyManager()
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CCommandManager::DestroyManager"));

	m_asyncFileManagerAvailable = false;
	
	if ( m_lockCounter )		// If any operations is occuring - show MessageBox
		CSkinnedWaitMsgBox(this,&m_asyncFileManager).Show(_T("Please, wait while\nFileManager is shutting down"),true);
	else
		m_asyncFileManager.reset();

	m_wdtProgressBar.reset();
	m_wdtCancelTransferButton.reset();
	m_wdtDescriptionLabel.reset();
	m_wdtHistoryListView.reset();
	m_wdtExpandHistoryButton.reset();
	m_wdtRequestButton.reset();

	m_wdtExpertLabel.reset();
	m_wdtExpertAddress.reset();
	m_wdtExpertUp.reset();
	m_wdtExpertRefresh.reset();
	m_wdtExpertFileInfo.reset();
	m_wdtExpertFileListView.reset();
	m_wdtExpertUnderline.reset();
	
	m_wdtCustomerLabel.reset();
	m_wdtCustomerAddress.reset();
	m_wdtCustomerUp.reset();
	m_wdtCustomerRefresh.reset();
	m_wdtCustomerFileInfo.reset();
	m_wdtCustomerFileListView.reset();
	m_wdtCustomerUnderline.reset();

	m_wdtFileViewExpertContainer.reset();
	m_wdtFileViewCustomerContainer.reset();

	m_wdtSplitter.reset();

	m_wdtFileViewContainer.reset();
	
	m_wdtStatusPanel.reset();
	m_wdtHistoryPanel.reset();
	m_wdtToolBar.reset();
	m_wdtToolbarPanel.reset();

	m_widgetsCreated = false;

	Log.Add(_MESSAGE_,_T("END CCommandManager::DestroyManager"));
	

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::NotifyMainWindowPosChanged()
{
TRY_CATCH

	if ( !m_widgetsCreated )
		return;

	RECT rc;
	GetClientRect(m_mainWnd,&rc);
	
	RECT wrc;

	wrc = rc;
	wrc.bottom	= wrc.top + m_locationToolBarHeight;
	m_wdtToolbarPanel->ResizeWidget(&wrc);
	
	wrc = rc;
	wrc.top		= wrc.bottom - m_locationStatusPanelHeigth;
	m_wdtStatusPanel->ResizeWidget(&wrc);

	wrc.bottom	= wrc.top;
	wrc.top		= wrc.bottom - (( m_historyWidgetExpanded ) ? m_locationHistoryPanelHeigthExpand : m_locationHistoryPanelHeigthCollapse);
	m_wdtHistoryPanel->ResizeWidget(&wrc);

	wrc	= rc;
	wrc.top		+= m_locationToolBarHeight;
	wrc.bottom	-= m_locationStatusPanelHeigth + (( m_historyWidgetExpanded ) ? m_locationHistoryPanelHeigthExpand : m_locationHistoryPanelHeigthCollapse);
	m_wdtFileViewContainer->ResizeWidget(&wrc);

	m_wdtFileViewContainer->GetWidgetRect(&wrc);
	m_wdtSplitter->ResizeWidget(&wrc);
	
	Log.Add(_MESSAGE_,_T("END CCommandManager::NotifyMainWindowPosChanged"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::ShowStatusMessage(const tstring& strMessage, const EFMStateEx state)
{
TRY_CATCH

	if (EFMS_STATES_COUNT != state && m_currentMsgState != state )
	{
		m_wdtStatusImage->m_currentImage = m_uiStatusIcons[state - 1];
		m_wdtStatusImage->RedrawWindow();
		
		m_wdtDescriptionLabel->FontColor1 = m_wdtStatusImage->m_currentImage->GetPixel(m_wdtStatusImage->m_currentImage->GetWidth()/2,1);
		
		m_currentState = state;
	}

	if ( !m_widgetsCreated )
		throw MCException(_T("Command manager hasn't been initialized"));

	m_wdtDescriptionLabel->SetText(strMessage);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::SetRequestHandler(boost::function <void(void)> requestHandler)
{
TRY_CATCH

	m_requestHandler = requestHandler;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::EnableRequestButton(BOOL enable)
{
TRY_CATCH

	m_wdtRequestButton->EnableWindow(enable);
	m_wdtRequestButton->RedrawWindow();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CCommandManager::InitTransferLogging(const tstring& sid, const tstring& customerName, const tstring& expertName)
{
TRY_CATCH

	m_transferLog.Init(sid, customerName, expertName);

CATCH_LOG()
}

// CCommandManager [END] /////////////////////////////////////////////////////////////////////////////////
