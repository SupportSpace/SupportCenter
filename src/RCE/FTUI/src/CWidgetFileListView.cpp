//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetFileListView.cpp
///
///  Implements CWidgetFileListView class
///  
///  
///  @author Alexander Novak @date 26.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWidgetFileListView.h"
#include "CCommandManager.h"
#include "resource.h"

// CWidgetFileListView [BEGIN] ///////////////////////////////////////////////////////////////////////////

LRESULT CWidgetFileListView::OnItemActivate(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMITEMACTIVATE itemActivate = (LPNMITEMACTIVATE)notifyHeader;

	CFileData* itemData = reinterpret_cast<CFileData*>(GetItemData(itemActivate->iItem));

	if ( itemData->IsRootDirectory() )
		DispatchCommand(cmd_BrowseForFilesUp);
	else if ( itemData->IsDirectory() || itemData->IsDrive() )
		DispatchCommand(cmd_BrowseForFiles,itemData);
		
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnCustomDrawHeader(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMCUSTOMDRAW customDraw = (LPNMCUSTOMDRAW)notifyHeader;
	
	switch ( customDraw->dwDrawStage )
	{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYPOSTPAINT;

		case CDDS_POSTPAINT:
		{
			if ( customDraw->rc.right <= customDraw->rc.left || customDraw->rc.bottom <= customDraw->rc.top ) 
				return CDRF_SKIPDEFAULT;
				
			customDraw->rc.right++;
			customDraw->rc.bottom++;
			FillRect(customDraw->hdc,&customDraw->rc,static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
			customDraw->rc.bottom-=2;

			int oldBkMode = SetBkMode(customDraw->hdc,TRANSPARENT);
			::SetTextColor(customDraw->hdc,FontColor1);
			HGDIOBJ oldPen = SelectObject(customDraw->hdc,CreatePen(PS_SOLID,1,FontColor1));

			int itemCount = Header_GetItemCount(customDraw->hdr.hwndFrom);
			for ( int i = 0; i < itemCount; i++ )
			{
				RECT rcItem;
				Header_GetItemRect(customDraw->hdr.hwndFrom,i,&rcItem);
				
				TCHAR textBuffer[101];
				HDITEM itemInfo;
				itemInfo.mask		= HDI_FORMAT|HDI_TEXT;
				itemInfo.pszText	= textBuffer;
				itemInfo.cchTextMax	= sizeof(textBuffer)/sizeof(textBuffer[0]);
				Header_GetItem(customDraw->hdr.hwndFrom,i,&itemInfo);
				
				itemInfo.fmt &= HDF_JUSTIFYMASK;
				UINT textFormat = DT_VCENTER|DT_SINGLELINE;
				switch ( itemInfo.fmt )
				{
					case HDF_CENTER:
						textFormat |= DT_CENTER;
						break;
					case HDF_RIGHT:
						textFormat |= DT_RIGHT;
						break;
					default:
						textFormat |= DT_LEFT;
				}
				rcItem.left		+= m_textMargin;
				rcItem.right	-= m_textMargin;

				DrawText(customDraw->hdc,itemInfo.pszText,static_cast<int>(_tcslen(itemInfo.pszText)),&rcItem,textFormat);
				
				rcItem.right	+= m_textMargin;
				rcItem.top		+= m_textMargin/2;
				rcItem.bottom	-= m_textMargin;

				MoveToEx(customDraw->hdc,rcItem.right,rcItem.top,NULL);
				LineTo(customDraw->hdc,rcItem.right,rcItem.bottom);
			}
			RECT rcHeader;
			::GetClientRect(customDraw->hdr.hwndFrom,&rcHeader);
			rcHeader.bottom--;
			MoveToEx(customDraw->hdc,customDraw->rc.left,rcHeader.bottom,NULL);
			LineTo(customDraw->hdc,customDraw->rc.right,rcHeader.bottom);

			DeleteObject(SelectObject(customDraw->hdc,oldPen));
			SetBkMode(customDraw->hdc,oldBkMode);

			return CDRF_SKIPDEFAULT;
		}
	}
	return CDRF_DODEFAULT;
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = FALSE;

	DispatchCommand(cmd_UpdateFileStatus);
	DispatchCommand(cmd_FileSelected);

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	bHandled = FALSE;

	DispatchCommand(cmd_ChangePanel);

	PostMessage(FLVM_POST_REPAINT);

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnInsertItem(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	bHandled = FALSE;
	DispatchCommand(cmd_UpdateFileStatus);

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnSelectionChanged(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	bHandled = FALSE;
	DispatchCommand(cmd_UpdateFileStatus);
	DispatchCommand(cmd_FileSelected);

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnBeginLabelEdit(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	NMLVDISPINFO* lvdi = (NMLVDISPINFO*)notifyHeader;

	// Don't rename a level up item
	if ( reinterpret_cast<const CFileData*>(GetItemData(lvdi->item.iItem))->IsRootDirectory() )
		return TRUE;

	// Subclass an edit control
	m_specialCharactersEdit.SubclassWindow(GetEditControl());

	m_editedItem = NULL;

	if ( m_createDirectoryOperation )
		DispatchCommand(cmd_StartDirCreate);
	else
	{
		if ( !m_sortSettings.m_enabled )
			return TRUE;

		DispatchCommand(cmd_StartRenaming);

		if ( !m_allowRenaming )
			return TRUE;
	}
	return FALSE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnEndLabelEdit(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetFileListView::OnEndLabelEdit"));

	NMLVDISPINFO* pdi = (NMLVDISPINFO*)notifyHeader;
	
	if ( !pdi->item.pszText && m_createDirectoryOperation )
	{
		DeleteItem(pdi->item.iItem);
		m_createDirectoryOperation = false;
	}
	if ( pdi->item.pszText && _tcslen(pdi->item.pszText) )
	{
		m_lastEnteredText	= pdi->item.pszText;
		m_editedItem		= reinterpret_cast<CFileData*>(pdi->item.lParam);
		DispatchCommand( ( m_createDirectoryOperation ) ? cmd_EndDirCreate : cmd_EndRenaming, m_editedItem);
		m_lastEnteredText.clear();
		return TRUE;
	}
	return FALSE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnDeleteItem(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetFileListView::OnDeleteItem"));

	LVITEM lvi;
	lvi.iItem		= ((LPNMLISTVIEW)notifyHeader)->iItem;
	lvi.iSubItem	= 0;
	lvi.mask		= LVIF_PARAM;

	if ( GetItem(&lvi) )
		RemoveFromSortSet(reinterpret_cast<CFileData*>(lvi.lParam));
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnDeleteAllItems(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	m_sortedFiles.clear();
	return TRUE;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetFileListView::CWidgetFileListView(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager)
	:	CSkinnedElement(commandManager.get()),
		CCommandProxy(commandManager),
		CIDropTarget(hParentWnd),
		m_createDirectoryOperation(false),
		m_allowRenaming(false),
		m_allowDragAndDrop(false),
		m_specialCharactersEdit(FLV_INVALID_CHARACTERS_TEXT)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetFileListView::CWidgetFileTreeView"));
	
	m_textMargin = GetSystemMetrics(SM_CXEDGE) * 3; // Hardcoded magic number inside Header control

	Create(	hParentWnd,
			0,
			NULL,
			WS_CHILD|WS_VISIBLE|LVS_EDITLABELS|LVS_REPORT|LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS);
	
	SHFILEINFO shfi;
	HIMAGELIST imageList = reinterpret_cast<HIMAGELIST>(SHGetFileInfo(	_T(""),
																		FILE_ATTRIBUTE_NORMAL,
																		&shfi,
																		sizeof(shfi),
																		SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_USEFILEATTRIBUTES));
	
	DWORD errLast = GetLastError();
	Log.Add(_MESSAGE_,_T("        CWidgetFileListView::CWidgetFileTreeView: ImageList Handle: %u, WinErrCode: %u"),imageList,errLast);
	CImageList oldImgList = SetImageList(imageList,LVSIL_SMALL);
	errLast = GetLastError();
	Log.Add(_MESSAGE_,_T("        CWidgetFileListView::CWidgetFileTreeView: oldImgList: %u, WinErrCode: %u"),oldImgList.IsNull(),errLast);

	// Add icon for level up button
	m_levelUpIconIndex = ImageList_AddIcon(imageList,LoadIcon((HINSTANCE)GetWindowLongPtr(GWL_HINSTANCE),MAKEINTRESOURCE(IDI_BTN_LEVEL_UP)));
	
	InsertColumn(0,_T("Name"),0,160);
	InsertColumn(1,_T("Size"),0/*LVCFMT_RIGHT*/,75);
	InsertColumn(2,_T("Date"),0/*LVCFMT_RIGHT*/,100);

	CIDropTarget::AddRef();

	if ( FAILED( RegisterDragDrop(m_hWnd,this) ) )		// Calls AddRef inside
		throw MCException(_T("Can't create register drop target"));

	FORMATETC ftetc={0};
	ftetc.cfFormat	= CF_PRIVATEFIRST;
	ftetc.dwAspect	= DVASPECT_CONTENT;
	ftetc.lindex	= -1;
	ftetc.tymed		= TYMED_HGLOBAL;
	AddSuportedFormat(ftetc);
	
	Log.Add(_MESSAGE_,_T("END CWidgetFileListView::CWidgetFileTreeView"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CWidgetFileListView::~CWidgetFileListView()
{
TRY_CATCH

	RevokeDragDrop(m_hWnd);								// Calls Release inside

	DestroyWindow();

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::ResizeWidget(LPRECT rect)
{
TRY_CATCH

	MoveWindow(rect,TRUE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::InsertFileInfo(const CFileData& fileInfo)
{
TRY_CATCH

	SendMessage(FLVM_INSERT_FILE_INFO,(WPARAM)&fileInfo);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void  CWidgetFileListView::ClearWidgetItems()
{
TRY_CATCH

	m_sortedFiles.clear();
	DeleteAllItems();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

tstring CWidgetFileListView::GetItemName(const void* itemId)
{
TRY_CATCH

	if ( itemId != NULL )
	{
		const CFileData* itemData = reinterpret_cast<const CFileData*>(itemId);
		return itemData->GetFileName();
	}
	else
		return _T("");

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

tstring CWidgetFileListView::GetCurrentStatusString()
{
TRY_CATCH

	TCHAR frmtStr[MAX_PATH + 50];
	frmtStr[0] = _T('\0');

	int itemIndex = GetSelectionMark();
	int selCount = GetSelectedCount();

	if ( itemIndex == -1 )
		return frmtStr;

	CFileData* itemData = reinterpret_cast<CFileData*>(GetItemData(itemIndex));
	
	if ( !itemData )
		return frmtStr;

	if ( selCount == 1 && !itemData->IsDrive() )
	{
		FILETIME localTime;
		FileTimeToLocalFileTime(&itemData->GetLastWriteTime(),&localTime);
		SYSTEMTIME sysTime;
		FileTimeToSystemTime(&localTime,&sysTime);

		if ( itemData->IsDirectory() )
		{
			if ( itemData->IsRootDirectory() )
			{
				_stprintf_s(frmtStr,sizeof(frmtStr)/sizeof(frmtStr[0]),_T("Object count: %d"),GetItemCount()-1);
			}
			else
				_stprintf_s(frmtStr,
							sizeof(frmtStr)/sizeof(frmtStr[0]),
							_T("%s, %d/%.2d/%d %d:%.2d"),
							itemData->GetFileName(),
							sysTime.wDay,
							sysTime.wMonth,
							sysTime.wYear,
							sysTime.wHour,
							sysTime.wMinute);
		}
		else
		{
			ULARGE_INTEGER fileSizeKB = itemData->GetFileSize();
			fileSizeKB.QuadPart /= 1024;

			_stprintf_s(frmtStr,
						sizeof(frmtStr)/sizeof(frmtStr[0]),
						_T("%s, %I64u KB, %d/%.2d/%d %d:%.2d"),
						itemData->GetFileName(),
						fileSizeKB.QuadPart,
						sysTime.wDay,
						sysTime.wMonth,
						sysTime.wYear,
						sysTime.wHour,
						sysTime.wMinute);
		}
	}
	else
	{
		const CFileData* itemData = reinterpret_cast<const CFileData*>(GetItemData(0));

		if ( GetItemState(0,LVIS_SELECTED) == LVIS_SELECTED && itemData->IsRootDirectory() )
			selCount--;

		if ( !selCount )
		{
			selCount = GetItemCount();

			if ( itemData->IsRootDirectory() )
				selCount--;
		}
			
		_stprintf_s(frmtStr,sizeof(frmtStr)/sizeof(frmtStr[0]),_T("Object count: %d"),selCount);
	}
	return frmtStr;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

bool CWidgetFileListView::StartRenameCurrentItem()
{
TRY_CATCH

	m_createDirectoryOperation = false;

	SetFocus();

	int itemIndex = GetSelectionMark();
	if ( itemIndex != -1 )
		EditLabel(itemIndex);

	return itemIndex != -1;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<filedata_vec_t> CWidgetFileListView::GetSelectedItems()
{
TRY_CATCH

	boost::shared_ptr<filedata_vec_t> selectedItems(new filedata_vec_t);
	
	int selCount = GetSelectedCount();
	int itemCount = GetItemCount();
	int itemIndex = 0;
	
	while ( selCount && itemIndex < itemCount )
	{
		const CFileData* fileData = reinterpret_cast<const CFileData*>(GetItemData(itemIndex));
		
		if ( GetItemState(itemIndex,LVIS_SELECTED) == LVIS_SELECTED && !fileData->IsRootDirectory() )
		{
			selectedItems->push_back(*fileData);
			selCount--;
		}
		itemIndex++;
	}

	return selectedItems;
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

tstring CWidgetFileListView::GetEnteredText()
{
TRY_CATCH

	return m_lastEnteredText;
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::RenameEditedItem()
{
TRY_CATCH

	SendMessage(FLVM_RENAME_EDITED_ITEM);
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::RestoreEditedItem()
{
TRY_CATCH

	SendMessage(FLVM_RESTORE_EDITED_ITEM);
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::StartCreateDirectory()
{
TRY_CATCH

	SendMessage(FLVM_START_CREATE_DIRECTORY);
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::ApplyDirectoryName()
{
TRY_CATCH

	RenameEditedItem();
	m_createDirectoryOperation = false;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::CancelDirectoryName()
{
TRY_CATCH

	RestoreEditedItem();
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::UndoCreateDirectory()
{
TRY_CATCH

	SendMessage(FLVM_UNDO_CREATE_DIRECTORY);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::LockWidget()
{
TRY_CATCH

	EnableWindow(FALSE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::UnlockWidget()
{
TRY_CATCH

	EnableWindow();

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::AllowItemRenaming(bool allow)
{
TRY_CATCH

	m_allowRenaming = allow;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

unsigned int CWidgetFileListView::GetSelectedItemCount()
{
TRY_CATCH

	unsigned int selCount = GetSelectedCount();

	if ( selCount && GetItemState(0,LVIS_SELECTED) == LVIS_SELECTED )
	{
		const CFileData* itemData = reinterpret_cast<const CFileData*>(GetItemData(0));

		if ( itemData && itemData->IsRootDirectory() )
			selCount--;
	}
	return selCount;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CALLBACK CWidgetFileListView::SortingCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
TRY_CATCH
	/// Get two sorted items and sorting settings
	SFileListSorting* sort = reinterpret_cast<SFileListSorting*>(lParamSort);
	bool ascOrder = (soASC == sort->m_order);
	CFileData* firstFile = reinterpret_cast<CFileData*>(lParam1);
	CFileData* secondFile = reinterpret_cast<CFileData*>(lParam2);
	/// Check up for directory
	bool firstDir = firstFile->IsDirectory();
	bool secondDir = secondFile->IsDirectory();
	if(firstDir^secondDir)
	{
		return firstDir ? -1 : 1;
	}
	
	if ( firstFile->IsRootDirectory() )			// Hack for a level up directory. It must be always on top.
		return -1;
	if ( secondFile->IsRootDirectory() )
		return 1;
	
	switch(sort->m_column)
	{
	case 0:		/// Compare by name
		{
			tstring firstName(UpperCase(firstFile->GetFileName()));
			tstring secondName(UpperCase(secondFile->GetFileName()));
			if(firstName < secondName)
				return ascOrder ? -1 : 1;
			else
			{
				if(firstName == secondName)
					return 0;
				else
					return ascOrder ? 1 : -1;
			}
		}
		break;
	case 1:		/// Compare by size
		{
			ULARGE_INTEGER firstSize = firstFile->GetFileSize();
			ULARGE_INTEGER secondSize = secondFile->GetFileSize();
			if(firstSize.QuadPart < secondSize.QuadPart)
				return ascOrder ? -1 : 1;
			else
			{
				if(firstSize.QuadPart == secondSize.QuadPart)
				{
					/// Compare by name for files with equal size
					SFileListSorting sorting;
					return SortingCompareFunc(lParam1, lParam2, reinterpret_cast<LPARAM>(&sorting));
				}
				else
					return ascOrder ? 1 : -1;
			}
		}
		break;
	case 2:		/// Compare by date
		{
			cDate firstTime(firstFile->GetLastWriteTime());
			cDate secondTime(secondFile->GetLastWriteTime());
			if(firstTime < secondTime)
				return ascOrder ? -1 : 1;
			else
			{
				if(firstTime == secondTime)
				{
					/// Compare by name for files with equal date
					SFileListSorting sorting;
					return SortingCompareFunc(lParam1, lParam2, reinterpret_cast<LPARAM>(&sorting));
				}
				else
					return ascOrder ? 1 : -1;
			}
		}
		break;
	}
CATCH_LOG()
	return 0;
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnColumnClick(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH
	/// Exit if sorting is disabled
	if(!m_sortSettings.m_enabled)
		return TRUE;
	/// Get index of column
	int column = ((LPNMLISTVIEW)notifyHeader)->iSubItem;
	/// Column is the same as previous sorting column
	if(m_sortSettings.m_column == column)
	{
		/// Change order
		m_sortSettings.m_order = (m_sortSettings.m_order == soASC) ? soDESC : soASC;
	}
	else
	{
		/// Set sorting settings
		m_sortSettings.m_column = column;
		m_sortSettings.m_order = soASC;
	}
	/// Sort items
	SortItems(&SortingCompareFunc, reinterpret_cast<LPARAM>(&m_sortSettings));
	return TRUE;
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::EnableSorting(const bool enabled)
{
TRY_CATCH

	m_sortSettings.m_enabled = enabled;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnBeginDrag(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	if ( GetItemState(0,LVIS_SELECTED) == LVIS_SELECTED )
	{
		const CFileData* itemData = reinterpret_cast<const CFileData*>(GetItemData(0));
		
		if ( itemData && itemData->IsRootDirectory() )
		{
			UINT selCount = GetSelectedCount();
			SetItemState(0,0,LVIS_SELECTED);

			if ( selCount == 1 )
				return 0;
		}
	}
	DispatchCommand(cmd_BeginDragDrop);
	
	NMLISTVIEW* pnmlv = (NMLISTVIEW*) notifyHeader;

	CIDropSource* pdsrc = new CIDropSource;
	if(pdsrc == NULL)
		return 0;
	pdsrc->AddRef();

	CIDataObject* pdobj = new CIDataObject(pdsrc);
	if(pdobj == NULL)
		return 0;
	pdobj->AddRef();
	
	AllowDragAndDrop(false);

	FORMATETC fmtetc = {0};
	fmtetc.cfFormat	= CF_PRIVATEFIRST;
	fmtetc.dwAspect	= DVASPECT_CONTENT;
	fmtetc.lindex	= -1;
	fmtetc.tymed	= TYMED_HGLOBAL;

	STGMEDIUM medium = {0};
	medium.tymed = TYMED_HGLOBAL;
	pdobj->SetData(&fmtetc,&medium,TRUE);

	CDragSourceHelper dragSrcHelper;
	dragSrcHelper.InitializeFromWindow(m_hWnd, pnmlv->ptAction, pdobj);
	DWORD dwEffect;
	HRESULT hr = ::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY, &dwEffect);
	pdsrc->Release();
	pdobj->Release();

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnKeyDownNotify(int wParam, LPNMHDR notifyHeader, BOOL& bHandled)
{
TRY_CATCH

	LPNMLVKEYDOWN lvkd = (LPNMLVKEYDOWN)notifyHeader;
	
	switch ( lvkd->wVKey )
	{
		case VK_BACK:
			DispatchCommand(cmd_BrowseForFilesUp);
			break;
		case VK_F2:
			DispatchCommand(cmd_RenameFile);
			break;
	}
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CWidgetFileListView::InsertToSortSet(CFileData* file)
{
TRY_CATCH

	int index = 0;
	/// Create new soting item
	boost::shared_ptr<SFileListItem> sortItem(new SFileListItem());
	/// Add file data
	sortItem->m_file.reset(file);
	/// Add sorting settings
	if(m_sortedFiles.size())
	{
		SortedFileList::iterator idx = m_sortedFiles.begin();
		boost::shared_ptr<SFileListItem> item = *idx;
		sortItem->m_sorting = item->m_sorting;
	}
	else
	{
		sortItem->m_sorting.reset(new SFileListSorting());
		*(sortItem->m_sorting) = m_sortSettings;
	}
	/// Add new item to set
	std::pair<SortedFileList::iterator, bool> insIter = m_sortedFiles.insert(sortItem);
	if(!insIter.second)
		index = MAXLONG;
	else
	{
		/// Get number of elements before inserted element
		index = std::distance(m_sortedFiles.begin(), insIter.first);
	}
	return index;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------	

void CWidgetFileListView::RemoveFromSortSet(CFileData* file)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetFileListView::RemoveFromSortSet"));

	SortedFileList::iterator index = std::find_if(m_sortedFiles.begin(), m_sortedFiles.end(), std::bind2nd(SFileSearch(), file));
	if(index != m_sortedFiles.end())
		m_sortedFiles.erase(index);

	Log.Add(_MESSAGE_,_T("END CWidgetFileListView::RemoveFromSortSet"));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------	

boost::shared_ptr<CWidgetFileListView::SFileListItem> CWidgetFileListView::ExtractFromSortSet(CFileData* file)
{
TRY_CATCH

	boost::shared_ptr<SFileListItem> item;
	SortedFileList::iterator index = std::find_if(m_sortedFiles.begin(), m_sortedFiles.end(), std::bind2nd(SFileSearch(), file));
	if(index != m_sortedFiles.end())
	{
		item = *index;
		m_sortedFiles.erase(index);
	}
	return item;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------	

LRESULT CWidgetFileListView::ProtectInsertFileInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	const CFileData* fileInfo = reinterpret_cast<const CFileData*>(wParam);
	CFileData* itemData = new CFileData();
	*itemData = *fileInfo;

	UINT itemMask = LVIF_PARAM|LVIF_TEXT;
	SHFILEINFO shfi;
	
	if ( fileInfo->IsDrive() )
	{
		if ( shfi.iIcon = m_getDriveIconIndex(EDriveTypes(fileInfo->GetAttributes())) )
			itemMask |= LVIF_IMAGE;
	}
	else if ( fileInfo->IsRootDirectory() && m_levelUpIconIndex != -1 )
	{
		shfi.iIcon = m_levelUpIconIndex;
		itemMask |= LVIF_IMAGE;
	}
	else
	{
		if ( SHGetFileInfo(	fileInfo->GetFileName(),	
							fileInfo->GetAttributes(),
							&shfi,
							sizeof(SHFILEINFO),
							SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_USEFILEATTRIBUTES))
			itemMask |= LVIF_IMAGE;
	}

	int indxItem = InsertItem(itemMask,InsertToSortSet(itemData),fileInfo->GetFileName(),0,0,shfi.iIcon,(LPARAM)itemData);
	
	if ( indxItem != -1 )
	{
		TCHAR frmtStr[25];
		frmtStr[0] = _T('\0');
		
		if ( !fileInfo->IsDirectory() )
		{
			ULARGE_INTEGER fileSizeKB = fileInfo->GetFileSize();
			if ( fileInfo->IsDrive() )
			{
				fileSizeKB.QuadPart /= 1024*1024*1024;
				if ( fileSizeKB.QuadPart )
					_stprintf_s(frmtStr,sizeof(frmtStr)/sizeof(frmtStr[0]),_T("%I64u GB"),fileSizeKB.QuadPart);
			}
			else
			{
				fileSizeKB.QuadPart /= 1024;
				_stprintf_s(frmtStr,sizeof(frmtStr)/sizeof(frmtStr[0]),_T("%I64u KB"),fileSizeKB.QuadPart);
			}
		}
		
		SetItem(indxItem,1,LVIF_TEXT,frmtStr,0,0,0,NULL);

		frmtStr[0] = _T('\0');
		
		if ( !fileInfo->IsDrive() && !fileInfo->IsRootDirectory())
		{
			FILETIME localTime;
			FileTimeToLocalFileTime(&fileInfo->GetLastWriteTime(),&localTime);
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&localTime,&sysTime);
			_stprintf_s(	frmtStr,
							sizeof(frmtStr)/sizeof(frmtStr[0]),
							_T("%d/%.2d/%d %d:%.2d"),
							sysTime.wDay,
							sysTime.wMonth,
							sysTime.wYear,
							sysTime.wHour,
							sysTime.wMinute);
		}
		SetItem(indxItem,2,LVIF_TEXT,frmtStr,0,0,0,NULL);
		if(lParam)
		{
			BOOL bRet = SetItemState(indxItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			if(bRet)
				bRet = EnsureVisible(indxItem, FALSE);
		}
	}

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::ProtectRenameEditedItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	if ( m_editedItem != NULL )
	{
		CFileData newFileData(*m_editedItem);
		int m_editedItemIndex = GetItemIndexByItemId(m_editedItem);
		TCHAR strBuffer[MAX_PATH];
		GetItemText(m_editedItemIndex,0,strBuffer,sizeof(strBuffer)/sizeof(strBuffer[0]));
		_tcscpy_s(newFileData.cFileName,sizeof(newFileData.cFileName),strBuffer);
		m_editedItem = NULL;
		DeleteItem(m_editedItemIndex);
		BOOL res;
		ProtectInsertFileInfo(0, reinterpret_cast<WPARAM>(&newFileData), 1, res);
	}

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::ProtectRestoreEditedItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	if ( m_editedItem != NULL )
	{
		int m_editedItemIndex = GetItemIndexByItemId(m_editedItem);
		SetItemText(m_editedItemIndex,0,m_editedItem->GetFileName());

		SetFocus();
		EditLabel(m_editedItemIndex);
	}
	m_editedItem = NULL;

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::ProtectUndoCreateDirectory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	if ( m_editedItem != NULL )
	{
		int m_editedItemIndex = GetItemIndexByItemId(m_editedItem);

		DeleteItem(m_editedItemIndex);
	}

	m_editedItem = NULL;

	m_createDirectoryOperation = false;

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnPostSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	SetFocus();
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnPostRepaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	InvalidateRect(NULL,FALSE);
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::ProtectStartCreateDirectory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	Log.Add(_MESSAGE_,_T("BGN CWidgetFileListView::ProtectStartCreateDirectory"));

	if ( m_createDirectoryOperation )
		return 0;

	CFileData* itemData = new CFileData();
	ZeroMemory(itemData,sizeof(CFileData));
	itemData->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	GetSystemTimeAsFileTime(&itemData->ftLastAccessTime);
	itemData->ftLastWriteTime = itemData->ftCreationTime = itemData->ftLastAccessTime;

	UINT itemMask;
	SHFILEINFO shfi;
	if ( SHGetFileInfo(	itemData->GetFileName(),	
						itemData->GetAttributes(),
						&shfi,
						sizeof(SHFILEINFO),
						SHGFI_SYSICONINDEX|SHGFI_USEFILEATTRIBUTES) )
		itemMask = LVIF_PARAM|LVIF_TEXT|LVIF_IMAGE;
	else
		itemMask = LVIF_PARAM|LVIF_TEXT;

	int indxItem = InsertItem(itemMask,InsertToSortSet(itemData),itemData->GetFileName(),0,0,shfi.iIcon,(LPARAM)itemData);
	if ( indxItem != -1 )
	{
		TCHAR frmtStr[50];
		frmtStr[0] = _T('\0');
		
		FILETIME localTime;
		FileTimeToLocalFileTime(&itemData->GetLastWriteTime(),&localTime);
		SYSTEMTIME sysTime;
		FileTimeToSystemTime(&localTime,&sysTime);
		_stprintf_s(	frmtStr,
						sizeof(frmtStr)/sizeof(frmtStr[0]),
						_T("%d/%.2d/%d %d:%.2d"),
						sysTime.wDay,
						sysTime.wMonth,
						sysTime.wYear,
						sysTime.wHour,
						sysTime.wMinute);

		SetItem(indxItem,2,LVIF_TEXT,frmtStr,0,0,0,NULL);
		
		m_createDirectoryOperation = true;
		SetFocus();
		if ( !EditLabel(indxItem) )	
			DeleteItem(indxItem);
	}
	Log.Add(_MESSAGE_,_T("END CWidgetFileListView::ProtectStartCreateDirectory"));

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CWidgetFileListView::OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	return DLGC_WANTMESSAGE;
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CWidgetFileListView::GetItemIndexByItemId(const CFileData* itemId)
{
TRY_CATCH

	return std::distance(m_sortedFiles.begin(), std::find_if(m_sortedFiles.begin(), m_sortedFiles.end(), std::bind2nd(SFileSearch(), itemId)));

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CWidgetFileListView::AllowDragAndDrop(bool allowDragAndDrop)
{
TRY_CATCH

	m_allowDragAndDrop = allowDragAndDrop;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------	

void CWidgetFileListView::SetWidgetFocus()
{
TRY_CATCH

	PostMessage(FLVM_POST_SET_FOCUS);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------	

HRESULT STDMETHODCALLTYPE CWidgetFileListView::DragEnter(	IDataObject __RPC_FAR *pDataObj,
															DWORD grfKeyState,
															POINTL pt,
															DWORD __RPC_FAR *pdwEffect)
{
	HRESULT hResult = CIDropTarget::DragEnter(pDataObj,grfKeyState,pt,pdwEffect);
	if ( hResult == S_OK && m_bAllowDrop )
	{
		m_bAllowDrop = m_allowDragAndDrop;
		QueryDrop(grfKeyState, pdwEffect);
	}
	
	return hResult;
}
//--------------------------------------------------------------------------------------------------------

bool CWidgetFileListView::OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
{
TRY_CATCH

	DispatchCommand(cmd_DragDropCopy,0);
	return true;

CATCH_THROW()
}
// CWidgetFileListView [END] /////////////////////////////////////////////////////////////////////////////

// CSpecialCharactersEdit [BEGIN] ////////////////////////////////////////////////////////////////////////

CSpecialCharactersEdit::CSpecialCharactersEdit(const TCHAR* toolTipText)
	:	m_toolTipText(toolTipText)
{
TRY_CATCH

	m_ti.cbSize		= sizeof(TOOLINFO);
	m_ti.uFlags		= TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	m_ti.hwnd		= NULL;
	m_ti.uId		= NULL;
	m_ti.hinst		= NULL;
	m_ti.lpszText	= const_cast<TCHAR*>(m_toolTipText.c_str());
	m_ti.rect.left = m_ti.rect.right = m_ti.rect.top = m_ti.rect.bottom = 0;
	
	m_toolTip.Create(0,0,0,WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP|TTS_BALLOON);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CSpecialCharactersEdit::~CSpecialCharactersEdit()
{
TRY_CATCH

	m_toolTip.DestroyWindow();
	
CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CSpecialCharactersEdit::ShowToolTip(bool show)
{
TRY_CATCH

	if ( show )
	{
		if ( m_ti.hwnd != m_hWnd )
		{
			m_toolTip.TrackActivate(&m_ti,FALSE);
			m_toolTip.DelTool(&m_ti);

			m_ti.hwnd	= m_hWnd;
			m_ti.uId	= (UINT)m_hWnd;
			m_toolTip.AddTool(&m_ti);
			m_toolTip.SetMaxTipWidth(FLV_TOOLTIP_BALOON_WIDTH);
		}
		RECT rc;
		GetClientRect(&rc);
		ClientToScreen(&rc);
		m_toolTip.TrackPosition(rc.left + (rc.right - rc.left) / 2, rc.bottom);
		m_toolTip.TrackActivate(&m_ti,TRUE);
	}
	else
	{
		if ( m_ti.hwnd )
		{
			m_toolTip.TrackActivate(&m_ti,FALSE);
			m_toolTip.DelTool(&m_ti);
			m_ti.hwnd = NULL;
		}
	}

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CSpecialCharactersEdit::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	ShowToolTip(false);
	::SetFocus(GetParent());
	
	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CSpecialCharactersEdit::OnCharEntered(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	switch ( wParam )
	{
		case _T('\\'):
		case _T('/'):
		case _T(':'):
		case _T('*'):
		case _T('?'):
		case _T('\"'):
		case _T('<'):
		case _T('>'):
		case _T('|'):
		{
			ShowToolTip();
			break;
		}
		default:
			bHandled = FALSE;
			ShowToolTip(false);
	}
	return 0;

CATCH_THROW()
}
// CSpecialCharactersEdit [END] //////////////////////////////////////////////////////////////////////////
