//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetFileListView.h
///
///  Declares CWidgetFileListView class
///  
///  
///  @author Alexander Novak @date 26.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "CFileData.h"
#include "DragDropImpl.h"
#include <boost/type_traits/remove_pointer.hpp>
#include "FTUI/FileTransfer/utils.h"
#include "..\..\RCUI\src\CSkinnedElement.h"
#include <set>
#include <AidLib/CTime/cTime.h>
#include <algorithm>
//========================================================================================================

typedef std::vector<CFileData> filedata_vec_t;
#define FLVM_INSERT_FILE_INFO				WM_USER + 1
#define FLVM_RENAME_EDITED_ITEM				WM_USER + 2
#define FLVM_RESTORE_EDITED_ITEM			WM_USER + 3
#define FLVM_UNDO_CREATE_DIRECTORY			WM_USER + 4
#define FLVM_START_CREATE_DIRECTORY			WM_USER + 5
#define FLVM_POST_SET_FOCUS					WM_USER + 6
#define FLVM_POST_REPAINT					WM_USER + 7
#define FLV_INVALID_CHARACTERS_TEXT			_T("A file name cannot contain any of\r\nthe following characters \\ / : * ? \" < > |")
#define FLV_TOOLTIP_BALOON_WIDTH			200
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSpecialCharactersEdit
	:	public CWindowImpl<CSpecialCharactersEdit, CEdit>
{
	TOOLINFO m_ti;
	tstring m_toolTipText;
	CToolTipCtrl m_toolTip;
	LRESULT OnCharEntered(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	CSpecialCharactersEdit(const TCHAR* toolTipText);
	~CSpecialCharactersEdit();
	void ShowToolTip(bool show = true);
	
	BEGIN_MSG_MAP(CSpecialCharactersEdit)

		MESSAGE_HANDLER(WM_CHAR,OnCharEntered);
		MESSAGE_HANDLER(WM_DESTROY,OnDestroy);

	END_MSG_MAP()
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetFileListView
	:	public CWindowImpl<CWidgetFileListView, CListViewCtrl>,
		public CSkinnedElement,
		public CCommandProxy,
		public CIDropTarget
{
	/// Sorting order
	enum ELVSortOrder
	{
		soASC,
		soDESC
	};
	/// Sorting settings structure
	struct SFileListSorting
	{
		int					m_column;	/// Column index
		ELVSortOrder		m_order;	/// Sorting order
		bool				m_enabled;	/// Sorting is enabled
		SFileListSorting()
			:	m_column(0)
			,	m_order(soASC)
			,	m_enabled(true)
		{};
	} m_sortSettings;
	/// Structure for sorting at creation list of files
	struct SFileListItem
	{
		boost::shared_ptr<CFileData>			m_file;		/// Pointer to file data
		boost::shared_ptr<SFileListSorting>		m_sorting;	/// Sorting settings
		/// Default constructor
		SFileListItem()
		{};
		/// Constructor
		/// @param file - pointer to file data
		/// @param sorting - sorting settings
		SFileListItem(boost::shared_ptr<CFileData> file, boost::shared_ptr<SFileListSorting> sorting)
			:	m_file(file)
			,	m_sorting(sorting)
		{};
	};
	/// Functor for sorting SFileListItem structures in set
	struct SFileListItemLess
		:	public std::less< boost::shared_ptr<SFileListItem> >
	{
		bool operator()(const boost::shared_ptr<SFileListItem>& _Left, const boost::shared_ptr<SFileListItem>& _Right)
		{
			return (-1 == SortingCompareFunc(
				reinterpret_cast<LPARAM>(_Left->m_file.get()), 
				reinterpret_cast<LPARAM>(_Right->m_file.get()), 
				reinterpret_cast<LPARAM>(_Left->m_sorting.get())));
		};
	};
	/// Functor to search specified CFileData in set
	struct SFileSearch
		:	public std::binary_function<boost::shared_ptr<SFileListItem>, const CFileData*, bool>
	{
		bool operator()(const boost::shared_ptr<SFileListItem>& item, const CFileData* file) const
		{
			return (item->m_file.get() == file);
		};
	};
	/// Define type of set with sorting items
	typedef std::set<boost::shared_ptr<SFileListItem>, SFileListItemLess> SortedFileList;
	/// Set with sorted items
	SortedFileList m_sortedFiles;
	
	int m_levelUpIconIndex;
	
	/// Class for retrieving icon index from the system image list
	CDriveIconIndex	m_getDriveIconIndex;

	/// Subclassing edit control
	CSpecialCharactersEdit m_specialCharactersEdit;
	
	bool m_createDirectoryOperation;
	CFileData* m_editedItem;
	tstring m_lastEnteredText;
	bool m_allowRenaming;
	int m_textMargin;
	
	bool m_allowDragAndDrop;

	virtual void OnSkinChanged(){};
	virtual HWND GetWindowHandle(){return 0;}
	
	LRESULT OnCustomDrawHeader(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);

	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnItemActivate(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnInsertItem(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnSelectionChanged(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnBeginLabelEdit(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnEndLabelEdit(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	
	LRESULT OnDeleteItem(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnDeleteAllItems(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);

	static int CALLBACK SortingCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	LRESULT OnColumnClick(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);

	LRESULT OnBeginDrag(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnKeyDownNotify(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);

	LRESULT ProtectInsertFileInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ProtectRenameEditedItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ProtectRestoreEditedItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ProtectUndoCreateDirectory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ProtectStartCreateDirectory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPostSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPostRepaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	int GetItemIndexByItemId(const CFileData* itemId);
	int InsertToSortSet(CFileData* file);
	void RemoveFromSortSet(CFileData* file);
	boost::shared_ptr<SFileListItem> ExtractFromSortSet(CFileData* file);

public:
	BEGIN_MSG_MAP(CWidgetFileListView)

		MESSAGE_HANDLER(FLVM_INSERT_FILE_INFO,ProtectInsertFileInfo);
		MESSAGE_HANDLER(FLVM_RENAME_EDITED_ITEM,ProtectRenameEditedItem);
		MESSAGE_HANDLER(FLVM_RESTORE_EDITED_ITEM,ProtectRestoreEditedItem);
		MESSAGE_HANDLER(FLVM_UNDO_CREATE_DIRECTORY,ProtectUndoCreateDirectory);
		MESSAGE_HANDLER(FLVM_START_CREATE_DIRECTORY,ProtectStartCreateDirectory);
		MESSAGE_HANDLER(FLVM_POST_SET_FOCUS,OnPostSetFocus);
		MESSAGE_HANDLER(FLVM_POST_REPAINT,OnPostRepaint);

		MESSAGE_HANDLER(WM_KEYUP,OnKeyUp);
		
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus);

		MESSAGE_HANDLER(WM_GETDLGCODE,OnGetDlgCode);

		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW,OnCustomDrawHeader)

		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMACTIVATE,OnItemActivate)
		
		REFLECTED_NOTIFY_CODE_HANDLER(NM_CLICK,OnSelectionChanged)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK,OnSelectionChanged)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_INSERTITEM,OnInsertItem)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINLABELEDIT,OnBeginLabelEdit)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT,OnEndLabelEdit)
		
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_DELETEITEM,OnDeleteItem)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_DELETEALLITEMS,OnDeleteAllItems)

		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK,OnColumnClick)

		REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINDRAG,OnBeginDrag)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_KEYDOWN,OnKeyDownNotify)

		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetFileListView(HWND hParentWnd, boost::shared_ptr<CAbstractCommandManager> commandManager);
	~CWidgetFileListView();
	void ResizeWidget(LPRECT rect);
	void InsertFileInfo(const CFileData& fileInfo);
	void ClearWidgetItems();
	tstring GetItemName(const void* itemId);
	tstring GetCurrentStatusString();
	bool StartRenameCurrentItem();
	boost::shared_ptr<filedata_vec_t> GetSelectedItems();
	tstring GetEnteredText();
	void RenameEditedItem();
	void RestoreEditedItem();
	void StartCreateDirectory();
	void ApplyDirectoryName();
	void CancelDirectoryName();
	void UndoCreateDirectory();
	void LockWidget();
	void UnlockWidget();
	void AllowItemRenaming(bool allow = true);
	unsigned int GetSelectedItemCount();
	void EnableSorting(const bool enabled);
	void AllowDragAndDrop(bool allowDragAndDrop);
	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject __RPC_FAR *pDataObj,
												DWORD grfKeyState,
												POINTL pt,
												DWORD __RPC_FAR *pdwEffect);
	virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD* pdwEffect);
	void SetWidgetFocus();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
