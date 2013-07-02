//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSkinnedWaitMsgBox.cpp
///
///  Implements CSkinnedWaitMsgBox class
///  Uses for showing message box while a boost::shared_ptr is destroying
///  
///  @author Alexander Novak @date 25.01.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CSkinnedWaitMsgBox.h"

// CSkinnedWaitMsgBox [BEGIN] ////////////////////////////////////////////////////////////////////////////

void CSkinnedWaitMsgBox::Execute(void* Params)
{
TRY_CATCH

	m_objForDestroy->reset();

	PostMessage(WM_CLOSE);

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

LRESULT CSkinnedWaitMsgBox::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH

	LRESULT result = CSkinnedMsgBox::OnInitDialog(uMsg,wParam,lParam,bHandled);
	
	m_okBtn.ShowWindow(SW_HIDE);
	
	Start();

	return result;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CSkinnedWaitMsgBox::CSkinnedWaitMsgBox(CSkinnedElement* parent, boost::shared_ptr<CAsyncFileManager>* objForDestroy)
	:	CSkinnedMsgBox(parent,false,false),
		m_objForDestroy(objForDestroy)
{
TRY_CATCH
	
CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CSkinnedWaitMsgBox::Show(const tstring& text, bool autoBreakText)
{
TRY_CATCH

	CSkinnedMsgBox::Show(0,text,autoBreakText);

CATCH_THROW()
}
// CSkinnedWaitMsgBox [END] //////////////////////////////////////////////////////////////////////////////
