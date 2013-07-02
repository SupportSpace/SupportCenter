/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWindowsMinimizer.cpp
///
///  CWindowsMinimizer windows minimizer class implementation
///
///  @author Kirill Solovyov @date 12.05.2008
///
////////////////////////////////////////////////////////////////////////

#include "CWindowsMinimizer.h"
#include <AidLib/Strings/tstring.h>
#include <AidLib/CException/CException.h>

CWindowsMinimizer::CWindowsMinimizer(void)
{
TRY_CATCH
CATCH_LOG()
}

CWindowsMinimizer::~CWindowsMinimizer(void)
{
TRY_CATCH
CATCH_LOG()
}

void CWindowsMinimizer::MinimizeProcessWindows(DWORD processId)
{
TRY_CATCH
	m_modal=false;
	m_pId=processId;
	if(!EnumWindows(EnumWindowsProc,reinterpret_cast<LPARAM>(this)))
		throw MCException_Win("EnumWindows() failed");
	if(!m_modal)// if no one modal window
		for(std::vector<HWND>::size_type i=0;i<m_hwnds.size();++i)
		{
			ShowWindow(m_hwnds[i],SW_MINIMIZE);
			Log.Add(_MESSAGE_,_T("MINIMIZE WINDOW 0x%x"),m_hwnds[i]);
		}
	m_hwnds.clear();
CATCH_THROW()
}
BOOL CALLBACK CWindowsMinimizer::EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
TRY_CATCH
	CWindowsMinimizer *_this=reinterpret_cast<CWindowsMinimizer*>(lParam);
	DWORD pId=0;
	GetWindowThreadProcessId(hwnd,&pId);
	if(pId==_this->m_pId)
	{
		WINDOWINFO wi={sizeof(wi)};
		if(!GetWindowInfo(hwnd,&wi))
			throw MCException_Win("GetWindowInfo() failed");
		if(wi.dwStyle&WS_VISIBLE)
		{
			//if model window openned in requested process stoping minimizing
			if(wi.dwExStyle&WS_EX_DLGMODALFRAME)
			{
				_this->m_modal=true;
			}
			_this->m_hwnds.push_back(hwnd);
		}
	}
	return TRUE;
CATCH_LOG()
	return FALSE;
}
