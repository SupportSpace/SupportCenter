//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSkinnedWaitMsgBox.h
///
///  Declares CSkinnedWaitMsgBox class
///  Uses for showing message box while a boost::shared_ptr is destroying
///  
///  @author Alexander Novak @date 25.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSkinnedMsgBox.h"
#include "CAsyncFileManager.h"
#include <AidLib/CThread/CThread.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSkinnedWaitMsgBox
	:	public CSkinnedMsgBox,
		public CThread
{
	boost::shared_ptr<CAsyncFileManager>* m_objForDestroy;
	
	/// Thread's method for object destroying
	virtual void Execute(void *Params);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:

	BEGIN_MSG_MAP(CSkinnedMsgBox)

		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CSkinnedMsgBox)

	END_MSG_MAP()

	/// Creates CSkinnedWaitMsgBox object
	/// @param parent			Parent window for dialog centering
	/// @param objForDestroy	Object what will be destroyed
	CSkinnedWaitMsgBox(CSkinnedElement* parent, boost::shared_ptr<CAsyncFileManager>* objForDestroy);

	/// Shows dialog while a boost::shared_ptr is destroying
	/// @param text				Text which will be shown to user
	/// @param autoBreakText	If it's true then uses DT_WORDBREAK flag for text printing
	void Show(const tstring& text, bool autoBreakText = false);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
