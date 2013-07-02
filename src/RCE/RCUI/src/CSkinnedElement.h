/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedElement.h
///
///  CSkinnedElement, base class for skinned elements
///
///  @author "Archer Software" Kirill Solovyov @date 20.06.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0500
#endif

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlimage.h>
#include <map>
#include <AidLib/CException/CException.h>
#include <boost/shared_ptr.hpp>
#include <AidLib/Loki/Singleton.h>



//TODO sink where reposition
/// skinned control viewer UI definitions
#define SCVUI_BTNFONTCOLOR1 RGB(0x58,0x58,0x58)//RGB(104,104,104)
#define SCVUI_BTNFONTCOLOR2 RGB(160,160,160)
#define SCVUI_LBLFONTCOLOR1 RGB(0x58,0x58,0x58)
#define SCVUI_CMBBKCOLOR1 RGB(0xff,0xff,0xff)
#define SCVUI_CMBBKCOLOR2 RGB(0xf1,0xf1,0xf1)
#define SCVUI_CMBEDGECOLOR1 RGB(0x68,0x68,0x68)





#define NOFONT _T("NOFONT")

#define RT_SKINS _T("SKINS")
// The function loads the specified resource from a module's executable file and create stream via CreateStreamOnHGlobal().The application must call Release() method to delete Stream object.
// @param hInstance Handle to the instance of the module whose executable file contains the bitmap to be loaded.
// @param lpType Specifies the resource type.
// @param lpNameResource Pointer to a null-terminated string that contains the name of the resource to be loaded. Alternatively, this parameter can consist of the resource identifier in the low-order word and zero in the high-order word. The MAKEINTRESOURCE macro can be used to create this value.
IStream* LoadStream(HINSTANCE hInstance,LPCTSTR lpType,LPCTSTR lpNameResource);


#define MESSAGE_HANDLER_HWND(msg, func) \
	if(uMsg == msg) \
	{ \
		bHandled = TRUE; \
		lResult = func(hWnd, uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

/// Resource skins repository
class CSkinsImageList : public std::map<int,boost::shared_ptr<CImage>>, public CInstanceTracker
{
	/// To protect map from accessing from different threads
	CRITICAL_SECTION m_cs;
public:
	CSkinsImageList() : CInstanceTracker("CSkinsImageList")
	{
		InitializeCriticalSection(&m_cs);
	}
	~CSkinsImageList()
	{
		DeleteCriticalSection(&m_cs);
	}
	/// The method obtain images by resource identifier of RT_SKINS resource type
	/// @param images destination array of skins pointers
	/// @param imagesCount element count of destination array of skins pointers (images)
	/// @param resources source array of resource identifier
	/// @return no return value. If resource with passed identifier not exist corresponding element of images array set to NULL
	void ImageFromRes(CImage**const images,const int imagesCount,const int *const  resources);
};

/// base class of skining elements
class CSkinnedElement: virtual public CMessageMap
{
private:
	/// Parent pointer (nullable)
	CSkinnedElement* m_parent;
	/// Element font
	LOGFONT m_font;
	/// Element font #2
	LOGFONT m_font2;
	/// Element color #1
	COLORREF m_color1;
	/// Element color #2
	COLORREF m_color2;
		/// bk color #1
	COLORREF m_bkColor1;
	/// bk color #2
	COLORREF m_bkColor2;
	///	edge color
	COLORREF m_edgeColor1;
	/// Element's text
	tstring m_sText;
	/// Element's hint
	tstring m_hint;
protected:
	/// currently used font
	HFONT m_hFont;

	/// SkinsImage list
	boost::shared_ptr<CSkinsImageList> m_skinsImageList;

BEGIN_MSG_MAP(CSkinnedElement)
	TRY_CATCH
	NOTIFY_CODE_HANDLER(TTN_GETDISPINFO,OnGetDispInfo)
	MESSAGE_HANDLER_HWND(WM_ERASEBKGND,OnEraseBkgnd)
	MESSAGE_HANDLER_HWND(WM_PAINT,OnPaint)
	MESSAGE_HANDLER_HWND(WM_NCPAINT,OnNCPaint)
	CATCH_LOG()
END_MSG_MAP()

	virtual LRESULT OnEraseBkgnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnNCPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnGetDispInfo(int id, LPNMHDR lpnmhdr, BOOL &bHandled);

public:
	/// current skin image
	CImage *m_currentImage;
	CSkinnedElement(CSkinnedElement *parent = NULL, 
					boost::shared_ptr<CSkinsImageList> skinsImageList = boost::shared_ptr<CSkinsImageList>(reinterpret_cast<CSkinsImageList*>(NULL)));
	virtual ~CSkinnedElement(void);
	/// Accessor to underlying window handle.
	virtual HWND GetWindowHandle() = NULL;

	/// Accessor to underlying window handle.
	__declspec( property( get=GetWindowHandle )) HWND WindowHandle;
	// Accessor to element's parent
	__declspec(property (get=GetParentElement, put=SetParentElement)) CSkinnedElement* Parent;
	/// Accessor to element's font.
	__declspec( property( get=GetLogFont, put=SetLogFont )) LOGFONT Font;
	/// Accessor to element's font #2.
	__declspec( property( get=GetLogFont2, put=SetLogFont2 )) LOGFONT Font2;
	/// Accessor to element's font color1.
	__declspec( property( get=GetFontColor1, put=SetFontColor1 )) COLORREF FontColor1;
	/// Accessor to element's font color2.
	__declspec( property( get=GetFontColor2, put=SetFontColor2 )) COLORREF FontColor2;
	/// Accessor to element's text.
	__declspec( property( get=GetText, put=SetText )) tstring Text;
	/// Accessor to element's hint.
	__declspec( property( get=GetHint, put=SetHint )) tstring Hint;
		/// Accessor to element's edge color1.
	__declspec( property( get=GetEdgeColor1, put=SetEdgeColor1 )) COLORREF EdgeColor1;
	/// Accessor to element's bk color1.
	__declspec( property( get=GetBkColor1, put=SetBkColor1 )) COLORREF BkColor1;
	/// Accessor to element's bk color2.
	__declspec( property( get=GetBkColor2, put=SetBkColor2 )) COLORREF BkColor2;

	virtual LOGFONT GetLogFont() const;
	virtual LOGFONT GetLogFont2() const;
	virtual COLORREF GetFontColor1() const;
	virtual COLORREF GetFontColor2() const;
	virtual tstring GetText() const;
	virtual tstring GetHint() const;
	virtual COLORREF GetBkColor1()const;
	virtual COLORREF GetBkColor2()const;
	virtual COLORREF GetEdgeColor1()const;

	virtual void SetLogFont(LOGFONT newVal);
	virtual void SetLogFont2(LOGFONT newVal);
	virtual void SetFontColor1(COLORREF newVal);
	virtual void SetFontColor2(COLORREF newVal);
	virtual void SetText(tstring newVal);
	virtual void SetHint(tstring newVal);
	virtual void SetBkColor1(COLORREF newVal);
	virtual void SetBkColor2(COLORREF newVal);
	virtual void SetEdgeColor1(COLORREF newVal);

	/// Parent element getter
	/// @return parent element (NULL if has no parent)
	CSkinnedElement* GetParentElement() const { return m_parent; };

	/// Parent element setter
	/// @parent parent element (NULL if has no parent)
	virtual void SetParentElement(CSkinnedElement* parent) { m_parent = parent; };


	/// Draws current image rect
	/// @param hDC device context for drawing
	/// @param x x coordinate for drawing
	/// @param y y coordinate for drawing
	/// @param rect rectangle withing image for drawing on hDC
	virtual void DrawCurrentImage(HDC hDC, const int x, const int y, const RECT &rect);
	
	/// Converts onw coordinates to parent coordinates
	/// @param rect rectangle - to convert coordinates
	virtual void OwnToParent(RECT& rect);

	/// Should be called each time skin was changed
	virtual void OnSkinChanged() = NULL;

	/// The method setup skinned element
	/// @param font font of element
	/// @param fontColor1 font color #1 (text color enabled control)
	/// @param fontColor2 font color #2 (text color disabled control)
	/// @param bkColor1 background color #1 
	/// @param bkColor2 background color #2 (selected item in combobox)
	/// @param edgeColor1 edge color (combobox list edge color for combobox)
	/// @param text Element's text
	/// @param hintElement's hint
	void Setup(LOGFONT font,COLORREF fontColor1,COLORREF fontColor2,COLORREF bkColor1,COLORREF bkColor2,COLORREF edgeColor1,
							const tstring& text,const tstring& hint);
};



template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CWindowImplEx;
template <class T, class TBase /* = CWindow */, class TWinTraits  /*= CControlWinTraits */ >
/// CWindowImpl analogue with OnFinalMessage defined method
/// wich set m_hWnd to NULL
class CWindowImplEx : public CWindowImpl< T, TBase >
{
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		m_hWnd = NULL;
	}
};

/// reuturns true if rectangle isn't valid for ex empty
inline bool RectIsntEmpty(const RECT& rc)
{
	return	rc.bottom > rc.top &&
			rc.right > rc.left;
}

/// Corner of a rectangle
typedef enum _ECorner
{
	UPPER_LEFT,
	UPPER_RIGHT,
	LOWER_LEFT,
	LOWER_RIGHT
} ECorner;

/// Utility function to help in forming element shape (window region by it's shape)
/// @element element to form
/// @corner Corner, color from which should be considered as transparent
void FormShape( CSkinnedElement& element, const ECorner corner );