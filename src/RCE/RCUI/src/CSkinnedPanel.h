/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedPanel.h
///
///  Skinned Panel implementation
///
///  @author "Archer Software" Sogin M. @date 18.12.2007
///
////////////////////////////////////////////////////////////////////////<summary>

#pragma once

// CSkinnedPanel
#include "CSkinnedElement.h"
#include <wtl/atlapp.h>
///#include <wtl/atlmisc.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>


/// Skinned Panel control implementation<summary>
/// @see CSkinnedPanelMultiline
class CSkinnedPanel :
	public CWindowImplEx<CSkinnedPanel, CStatic>, 
	public CSkinnedElement
{
protected:
	bool m_UnderMouse;

	/// Text alignment
	UINT m_textAlign;

	/// Memory DC with stretched image
	std::auto_ptr<CMemoryDC> m_memDC;

	/// Top of the panel image
	CImage *m_headImage;
	/// Tail of the panel image
	CImage *m_tailImage;
	/// Middle panel image
	CImage *m_middleImage;

public:
	/// ctor
	/// @param parent pointer to parent control, that is needed since Panel is redrawn in pair with it's parent
	CSkinnedPanel(	const int imageId, 
					const int headImageId = -1,
					const int tailImageId = -1,
					CSkinnedElement *parent = NULL );
	/// class destructor
	virtual ~CSkinnedPanel();

	DECLARE_WND_SUPERCLASS(_T("CSkinnedPanel"), CStatic::GetWndClassName())

	// Accessor to element's text alignment
	__declspec(property (get=GetTextAlign, put=SetTextAlign)) UINT TextAlign;

	UINT GetTextAlign() const;
	void SetTextAlign(UINT newVal);

	/// Proper control initialization
	/// @param  hWnd Controls window handle
	/// @param  Parent Parent control
	/// @return TRUE if successful
	virtual BOOL Attach(HWND hWnd);
	
	/// Accessor to underlying window handle in CSkinnedElement. @see CSkinnedElement
	/// return control window handle
	HWND GetWindowHandle()
	{
		return static_cast<HWND>(*this);
	}
	
	/// Text property setter method @see CSkinnedElement::SetText
	/// @param newVal new Text value
	void SetText(tstring newVal)
	{
		SetWindowText(newVal.c_str());
	}

	/// DOXYS_OFF 
	BEGIN_MSG_MAP(CSkinnedPanel)
		TRY_CATCH
		MSG_WM_SIZE(OnSize)
		MSG_WM_PRINT(OnPrint)
		CHAIN_MSG_MAP(CSkinnedElement)
		CATCH_LOG("BEGIN_MSG_MAP(CSkinnedPanel)")
	END_MSG_MAP()
	/// DOXYS_ON

	/// Notification from CSkinEngine after it applies skin attributes.
	void OnSkinChanged();

	/// Size changed event handler
	void OnSize(UINT, CSize& size);

	void OnPrint(HDC hDC, int) {BOOL handled;CSkinnedElement::OnPaint(m_hWnd,WM_PAINT,(WPARAM)hDC,0,handled);}//{OnPaint(hDC);}

	void OnUpdateUIState(DWORD, DWORD) {};
};


