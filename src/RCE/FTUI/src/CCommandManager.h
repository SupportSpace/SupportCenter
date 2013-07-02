//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CCommandManager.h
///
///  Declares CCommandManager class
///  Mediator class for interaction between UI and FileManager object
///  
///  @author Alexander Novak @date 22.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CAsyncFileManager.h"
#include "CommandDefinitions.h"
#include "CWidgetPanel.h"
#include "CWidgetButton.h"
#include "CWidgetBitmapButton.h"
#include "CWidgetLabel.h"
#include "CWidgetSplitter.h"
#include "CWidgetAddress.h"
#include "CWidgetFileListView.h"
#include "CWidgetHistoryListView.h"
#include "CWidgetToolBar.h"
#include "CWidgetProgress.h"
#include "CWidgetStatic.h"
#include "CFileFilter.h"
#include "CWidgetTimerLabel.h"
#include <FTUI/FileTransfer/CFileTransferLog.h>
#include "..\..\RCUI\src\CSkinnedStatic.h"
//========================================================================================================

#define HIDE_HISTORY_TEXT _T("Hide session file transfer history")
#define SHOW_HISTORY_TEXT _T("View session file transfer history")

#define CMST_EXPERT_BROWSE					0x00000001
#define CMST_CUSTOM_BROWSE					0x00000002
#define CMST_CUSTOM_CONNECTED				0x00000003
#define CMST_CUSTOM_DISCONNECTED			0x00000004
#define CMST_EXPERT_FILE_SELECTED			0x00000005
#define CMST_CUSTOM_FILE_SELECTED			0x00000006
#define CMST_EXPERT_START_BROWSE			0x00000007
#define CMST_CUSTOM_START_BROWSE			0x00000008
#define CMST_TRANSFER						0x00000009
#define CMST_TRANSFER_COMPLETE				0x0000000A
#define CMST_DELETE_EXPERT_FILE				0x0000000B
#define CMST_DELETE_CUSTOM_FILE				0x0000000C
#define CMST_DELETE_EXPERT_COMPLETE			0x0000000D
#define CMST_DELETE_CUSTOM_COMPLETE			0x0000000E
#define CMST_TRANSFER_CANCELED				0x0000000F
#define CMST_RENAME_FILE					0x00000010
#define CMST_RENAME_FILE_COMPLETE			0x00000011
#define CMST_CREATE_DIRECTORY				0x00000012
#define CMST_CREATE_DIRECTORY_COMPLETE		0x00000013
#define CMST_EXPERT_BROWSE_COMPLETE			0x00000014
#define CMST_CUSTOM_BROWSE_COMPLETE			0x00000015

#define CM_DLGID_DELETE_EXPERT				0x00000001
#define CM_DLGID_DELETE_CUSTOM				0x00000002
#define CM_DLGID_MOVE_EXPERT				0x00000003
#define CM_DLGID_MOVE_CUSTOM				0x00000004
#define CM_DLGID_CONFIRM_OVERWRITE			0x00000005

#define CM_DEFAULTTEXT_ELAPSE				5000
#define CM_WAITCANCEL_ELAPSE				1000
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Extended file transfer state enum
enum EFMStateEx
{
	EFMS_PERMISSION_REQUEST_SENT = 1,
	EFMS_PERMISSION_RECEIVED,
	EFMS_PERMISSION_DENIED,
	EFMS_INSTALLATION_PROGRESS,
	EFMS_INSTALLATION_SUCCESSFUL,
	EFMS_INSTALLATION_FAILED,
	EFMS_CONNECTING,
	EFMS_SESSION_FAILED,
	EFMS_INIFIAL,
	EFMS_ON,
	EFMS_STATES_COUNT
};

class CCommandManager
	:	public CAbstractCommandManager
{
	friend void CCommandProxy::DispatchCommand(EWidgetCommand,const void*);

	CFileTransferLog m_transferLog;
	boost::shared_ptr<CDirContentLogging> m_dirLogging;

	boost::shared_ptr<CFileFilter> m_fileFilter;
	
	boost::shared_ptr<CAsyncFileManager> m_asyncFileManager;
	boost::function <void(void)> m_requestHandler;

	CLink m_currentExpertLink;
	CLink m_currentCustomerLink;

	HWND m_mainWnd;
	
	const static int m_locationToolBarHeight				= 35;
	const static int m_locationHistoryPanelHeigthCollapse	= 31;
	const static int m_locationHistoryPanelHeigthExpand		= 150;
	const static int m_locationStatusPanelHeigth			= 30;
	
	bool m_historyWidgetExpanded;
	
	boost::shared_ptr<CWidgetContainer>			m_wdtFileViewContainer;
	boost::shared_ptr<CWidgetPanel>				m_wdtFileViewExpertContainer;
	boost::shared_ptr<CWidgetPanel>				m_wdtFileViewCustomerContainer;
	boost::shared_ptr<CWidgetSplitter>			m_wdtSplitter;
	boost::shared_ptr<CWidgetPanel>				m_wdtToolbarPanel;
	boost::shared_ptr<CWidgetPanel>				m_wdtStatusPanel;
	boost::shared_ptr<CWidgetPanel>				m_wdtHistoryPanel;
	boost::shared_ptr<CWidgetLinkLabel>			m_wdtRequestButton;
	
	boost::shared_ptr<CWidgetLabel>				m_wdtExpertLabel;
	boost::shared_ptr<CWidgetAddress>			m_wdtExpertAddress;
	boost::shared_ptr<CWidgetBitmapButton>		m_wdtExpertRefresh;
	boost::shared_ptr<CWidgetBitmapButton>		m_wdtExpertUp;
	boost::shared_ptr<CWidgetLabel>				m_wdtExpertFileInfo;
	boost::shared_ptr<CWidgetStatic>			m_wdtExpertUnderline;

	boost::shared_ptr<CWidgetLabel>				m_wdtCustomerLabel;
	boost::shared_ptr<CWidgetAddress>			m_wdtCustomerAddress;
	boost::shared_ptr<CWidgetBitmapButton>		m_wdtCustomerRefresh;
	boost::shared_ptr<CWidgetBitmapButton>		m_wdtCustomerUp;
	boost::shared_ptr<CWidgetLabel>				m_wdtCustomerFileInfo;
	boost::shared_ptr<CWidgetStatic>			m_wdtCustomerUnderline;
	
	boost::shared_ptr<CWidgetToolBar>			m_wdtToolBar;

	bool m_expertPanelIsActive;

	boost::shared_ptr<CWidgetFileListView>		m_wdtExpertFileListView;
	boost::shared_ptr<CWidgetFileListView>		m_wdtCustomerFileListView;
	
	boost::shared_ptr<CWidgetLinkLabelEx>		m_wdtExpandHistoryButton;

	boost::shared_ptr<CWidgetHistoryListView>	m_wdtHistoryListView;

	boost::shared_ptr<CWidgetTimerLabel>		m_wdtDescriptionLabel;
	boost::shared_ptr<CWidgetLinkLabel>			m_wdtCancelTransferButton;

	boost::shared_ptr<CWidgetProgress>			m_wdtProgressBar;

	/// status image control
	boost::shared_ptr<CSkinnedStatic>			m_wdtStatusImage;

	/// status image control
	boost::shared_ptr<CSkinnedStatic>			m_wdtStatusDots;
	
	/// poineters on images of status icon
	CImage *m_uiStatusIcons[EFMS_STATES_COUNT];

	bool m_widgetsCreated;
	volatile bool m_asyncFileManagerAvailable;

	virtual void OnSkinChanged(){};
	virtual HWND GetWindowHandle(){return m_mainWnd;}

	void TransferHandler(	const CLink& destinationItem,
							const CLink& sourceItem,
							const ULARGE_INTEGER totalSize,
							const ULARGE_INTEGER transferedSize);

	void UncompressHandler(const CLink& uncompressLink);

	void TransferCompletionHandler(	const CLink& destinationItem,
									const CLink& sourceItem,
									const CFileData& fileData,
									const bool copyOperation,
									const DWORD winErr,
									const DWORD internalErr);
	void TransferMoveDeleteHandler(	const CLink& destinationItem,
									const CLink& sourceItem,
									const CFileData& fileData);
	bool AskForOverwriteHandler(const CLink& destinationItem,
								const CLink& sourceItem,
								const CFileData& fileData,
								const bool copyOperation);
	void TransferCompressingHandler(const CFileData& fileData);

	void CreateOperationHandler(const CLink& directoryName, const DWORD winErr, const DWORD internalErr);
	void RenameOperationHandler(const CLink& newItemName, const CLink& oldItemName, const DWORD winErr, const DWORD internalErr);
	void DeleteOperationHandler(const CLink& targetItem, const DWORD winErr, const DWORD internalErr);
	void ScanHandler(const CLink& path, const CFileData& fileData);
	void ScanCompletionHandler(const CLink& path);

	void ExecuteCommand(const CCommandProxy* sender, EWidgetCommand command, const void* commandData);

	void CmdContentBrowse(const CLink& link, bool changeStatus = true);
	void CmdRenameFile();
	void CmdRenameFileStart();
	void CmdRenameFileComplete(const bool expertSize, const void* itemId);
	void CmdCreateDirectory();
	void CmdCreateDirectoryComplete(const bool expertSize);
	void CmdDeleteFile();
	void CmdTransferOperations(const bool moveOperations);
	void CmdActivatePanel(const CCommandProxy* sender);
	void CmdRearrangeWidgets(const CCommandProxy* sender);
	void CmdResizeHistoryWidget(const bool expandWidget);
	void CmdFilterChanged();
	
	volatile LONG m_lockCounter;
	bool m_customIsConnected;
	bool m_customBrowseInProcess;
	bool m_expertBrowseInProcess;
	unsigned int m_currentState;
	void ChangeWidgetState(unsigned int stateAction);

	bool IsAllowTransferFile();
	
	EFMStateEx m_defaultMsgState;
	EFMStateEx m_currentMsgState;
	void SetDefaultStatusMessage(const tstring& strMessage, const UINT elapse, const EFMStateEx state = EFMS_STATES_COUNT);
public:
	CCommandManager();
	~CCommandManager();
	void InitManager(HWND hMainWnd,	boost::shared_ptr<CCommandManager> commandManager);
	void InitRemoteSide(boost::shared_ptr<CAbstractStream> stream);
	/// Remote side disconnected event handler
	void OnRemoteSideDisconnected(const ESessionStopReason stopReason);
	void DestroyManager();
	void NotifyMainWindowPosChanged();
	void ShowStatusMessage(const tstring& strMessage, const EFMStateEx state = EFMS_STATES_COUNT);
	void SetRequestHandler(boost::function <void(void)> requestHandler);
	/// Enables or disables request control
	void EnableRequestButton(BOOL enable = TRUE);

	/// Initializes transfer logging
	/// @param sid session id
	/// @param customerName - customer user name
	/// @param expertName - expert user name
	void InitTransferLogging(const tstring& sid, const tstring& customerName, const tstring& expertName);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
