/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWindowsMinimizer.h
///
///  CWindowsMinimizer windows minimizer class declaration
///
///  @author Kirill Solovyov @date 12.05.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include "Windows.h"
#include <vector>
/// the class minimize window of process. thread unsafe class.
class CWindowsMinimizer
{
protected:
	/// process id of process whose window minimized
	DWORD m_pId;
	/// hwnd list of window to minimize
	std::vector<HWND> m_hwnds;
	/// inner flag of modal window - if modal window open in process then don't minimize any window
	bool m_modal;
	/// The method is callback function used with the EnumWindows 
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam);
public:
	CWindowsMinimizer(void);
	virtual ~CWindowsMinimizer(void);
	/// The method minimize all top windows of process with processId
	/// @param processId process id whose top window must be minimized
	void MinimizeProcessWindows(DWORD processId);
};
