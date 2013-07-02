/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedStatic.h
///
///  Skinned static implementation
///
///  @author "Archer Software" Sogin M., Kirill Solovyov @date 08.01.2008
///
////////////////////////////////////////////////////////////////////////<summary>

#pragma once

#include "CSkinnedElement.h"
#include <wtl/atlapp.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>

class CSkinnedStatic :
	public CWindowImpl<CSkinnedStatic, CStatic>,
	public CSkinnedElement
{
public:
	DECLARE_WND_SUPERCLASS(_T("CSkinnedStatic"), CStatic::GetWndClassName())
	BEGIN_MSG_MAP(CSkinnedStatic)
		TRY_CATCH
		CHAIN_MSG_MAP(CSkinnedElement)
		CATCH_LOG()
	END_MSG_MAP()
public:
	CSkinnedStatic(void){}
	virtual ~CSkinnedStatic(void){}

	/// Accessor to underlying window handle in CSkinnedElement. @see CSkinnedElement
	/// return control window handle
	HWND GetWindowHandle()
	{
		return static_cast<HWND>(*this);
	}

	/// Should be called each time skin was changed
	virtual void OnSkinChanged() {};
};


