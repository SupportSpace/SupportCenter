//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CWidgetFileTreeView.h
///
///  Declares CWidgetHistoryListView class
///  
///  
///  @author Alexander Novak @date 28.11.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include "CCommandProxy.h"
#include "CFileData.h"
#include <boost/type_traits/remove_pointer.hpp>
#include "..\..\RCUI\src\CSkinnedElement.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWidgetHistoryListView
	:	public CWindowImpl<CWidgetHistoryListView, CListViewCtrl>,
		public CSkinnedElement,
		public CCommandProxy
{
	struct SHistoryInfo
	{
		tstring m_fileName;
		tstring m_fileSize;
		tstring m_fileDate;
		bool m_receiveDirection;
		boost::shared_ptr<boost::remove_pointer<HICON>::type> m_typeIcon;
	};
	std::vector<SHistoryInfo> m_updateInfo;

	CImage* m_fillImage;
	CImage* m_topImage;
	CImage* m_bottomImage;
	CImage* m_receiveImage;
	CImage* m_sendImage;
	int m_textMargin;
	int m_iconMargin;
	
	virtual void OnSkinChanged(){};
	virtual HWND GetWindowHandle(){return 0;}
	
	LRESULT OnCustomDrawHeader(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnCustomDrawList(int wParam, LPNMHDR notifyHeader, BOOL& bHandled);
	LRESULT OnUpdateControl(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	BEGIN_MSG_MAP(CWidgetHistoryListView)

		REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW,OnCustomDrawList)
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW,OnCustomDrawHeader)
		MESSAGE_HANDLER(OCM_MEASUREITEM,OnMeasureItem)
		MESSAGE_HANDLER(WM_NCCALCSIZE,OnUpdateControl)
		MESSAGE_HANDLER(WM_VSCROLL,OnUpdateControl)
		MESSAGE_HANDLER(WM_HSCROLL,OnUpdateControl)
		MESSAGE_HANDLER(WM_MOUSEWHEEL,OnUpdateControl)
		DEFAULT_REFLECTION_HANDLER()

	END_MSG_MAP()

	CWidgetHistoryListView(	HWND hParentWnd,
							boost::shared_ptr<CAbstractCommandManager> commandManager,
							const int fillImageId = -1,
							const int topImageId = -1,
							const int bottomImageId = -1);
	~CWidgetHistoryListView();
	void ResizeWidget(LPRECT rect);
	void AddTransferInfo(const CFileData& fileData, bool receiveDirection);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
