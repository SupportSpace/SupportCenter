/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSkinnedElement.cpp
///
///  CSkinnedElement, base class for skinned elements
///
///  @author "Archer Software" Kirill Solovyov @date 20.06.2006
///
////////////////////////////////////////////////////////////////////////
#include "CSkinnedElement.h"
#include <atlhost.h>
#include <atlapp.h>
#include <wtl/atlctrls.h>

IStream* LoadStream(HINSTANCE hInstance, LPCTSTR lpType, LPCTSTR lpNameResource)
{
	HRESULT res=0;
	HRSRC resource = FindResource(hInstance, lpNameResource, lpType);
	if (!resource)
		return NULL;
	DWORD imageSize=SizeofResource(hInstance, resource);
	if(!imageSize)
		return NULL;
	const void* resourceData=LockResource(LoadResource(hInstance, resource));
	if(!resourceData)
		return NULL;
	HGLOBAL hBuffer=GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if(!hBuffer)
		res=GetLastError();
	else 
	{
		void* buffer=::GlobalLock(hBuffer);
		if(!buffer)
			res=GetLastError();
		else
		{
			CopyMemory(buffer, resourceData, imageSize);
			IStream* stream = NULL;
			if ((res=CreateStreamOnHGlobal(hBuffer, TRUE, &stream))==S_OK)
				return stream;
			GlobalUnlock(buffer);
		}
	}
	GlobalFree(hBuffer);
	SetLastError(res);
	return NULL;
}

///Taked from http://www.codeproject.com/dialog/BitmapHandling.asp
BOOL BitMapContourToWinRgn( HDC hDC, HBITMAP hBmp, HRGN& hRgn, COLORREF cTolerance, const ECorner corner )
{
	hRgn = NULL;	
	if (hBmp)
	{
		// Create memory DC inside which we will scan bitmap content
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC)
		{
			// Get bitmap size
			BITMAP bm;
			GetObject(hBmp, sizeof(bm), &bm);

			// Create 32 bits depth bitmap and select it into memory DC 
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {	
				sizeof(BITMAPINFOHEADER),	// biSize 
				bm.bmWidth,					// biWidth; 
				bm.bmHeight,				// biHeight; 
				1,							// biPlanes; 
				32,							// biBitCount 
				BI_RGB,						// biCompression; 
				0,							// biSizeImage; 
				0,							// biXPelsPerMeter; 
				0,							// biYPelsPerMeter; 
				0,							// biClrUsed; 
				0							// biClrImportant; 
			};
			VOID * pbits32; 
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);
				if (hDC)
				{
					// Get bytes per row for bitmap bits (rounded up to 32 bits)
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// Copy bitmap into memory DC
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);
					COLORREF cTransparentColor;
					switch(corner)
					{
						case UPPER_LEFT:
							cTransparentColor = GetPixel(hMemDC, 0, 0); 
							break;
						case UPPER_RIGHT:
							cTransparentColor = GetPixel(hMemDC, bm.bmWidth - 1, 0); 
							break;
						case LOWER_LEFT:
							cTransparentColor = GetPixel(hMemDC, 0, bm.bmHeight - 1); 
							break;
						case LOWER_RIGHT:
							cTransparentColor = GetPixel(hMemDC, bm.bmWidth - 1, bm.bmHeight - 1); 
							break;
						default:
							Log.Add(_ERROR_,_T("Unknown corned type %d"),corner);
							return FALSE;
					}
#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					BYTE lr = max(0, GetRValue(cTransparentColor) - GetRValue(cTolerance)); 
					BYTE lg = max(0, GetGValue(cTransparentColor) - GetGValue(cTolerance)); 
					BYTE lb = max(0, GetBValue(cTransparentColor) - GetBValue(cTolerance)); 
					BYTE hr = min(0xff, GetRValue(cTransparentColor) + GetRValue(cTolerance)); 
					BYTE hg = min(0xff, GetGValue(cTransparentColor) + GetGValue(cTolerance)); 
					BYTE hb = min(0xff, GetBValue(cTransparentColor) + GetBValue(cTolerance)); 

					// Scan each bitmap row from bottom to top (the bitmap is inverted vertically)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// Scan each bitmap pixel from left to right
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// Search for continuous range of "non-transparent pixels"
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = ((RGBQUAD  *)  p)->rgbRed;
								if (b >= lr && b <= hr)
								{
									b = ((RGBQUAD  *)  p)->rgbGreen;
									if (b >= lg && b <= hg)
									{
										b = ((RGBQUAD  *)  p)->rgbBlue;
										if (b >= lb && b <= hb)
											// This pixel is "transparent"
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// Add pixels (x0, y) to (x, y+1) as new rectangle in region
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// On Windows98, ExtCreateRegion() may fail if number of rectangles
								// is too large (ie: > 4000), so create region by multiple steps.
								if (pData->rdh.nCount == 2000)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}
						// Go to next row (note: bitmap is inverted vertically)
						p32 -= bm32.bmWidthBytes;
					}

					// Create or extend the region with remaining rectangles
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// CLEAN UP COMPLETELY TO PREVENT MEMORY LEAK!
					GlobalFree(hData);
				}
				DeleteObject(SelectObject(hMemDC, holdBmp));
			}
			DeleteDC(hMemDC);
		}	
	}

	if (!hRgn)
		return FALSE;

	//NOTE: win32-based apps do not need to unlock or free loaded resources (not manually) 
	return TRUE; 
}

void FormShape( CSkinnedElement& element, const ECorner corner )
{
TRY_CATCH
	CWindow window(element.WindowHandle);
	RECT rc;
	window.GetWindowRect(&rc);
	window.ScreenToClient(&rc);
	HDC hDC(window.GetDC());
	CMemoryDC memDC(hDC, rc);
	BOOL bHandled;
	element.OnEraseBkgnd(window, 0, (WPARAM)(memDC.m_hDC), 0, bHandled);
	HRGN rgn;
	if (FALSE != BitMapContourToWinRgn(memDC, memDC.m_bmp, rgn, RGB(0,0,0), corner))
		window.SetWindowRgn(rgn);
	else
		Log.WinError(_ERROR_,_T("FormShape failed on BitMapContourToWinRgn"));
	window.ReleaseDC(hDC);
	window.RedrawWindow();
CATCH_LOG()
}

void CSkinsImageList::ImageFromRes(CImage**const images,const int imagesCount,const int *const  resources)
{
TRY_CATCH

	CCritSection cs(&m_cs);
	if(!images||!resources||imagesCount<0)
	{
		Log.Add(_MESSAGE_,_T("CSkinsImageList::ImageFromRes() invalidates parameters"));
		return;
	}
	for(int i=0; i<imagesCount; ++i)
	{
		CSkinsImageList::iterator im;
		if((im=find(resources[i]))==end())
		{
			IStream *stream=NULL;
			if(!(stream=LoadStream(_AtlBaseModule.m_hInst,RT_SKINS,MAKEINTRESOURCE(resources[i]))))
			{
				if(GetLastError())
					Log.WinError(_ERROR_,_T("Resource stream obtaining failed. index=%d and type=%s "),resources[i],RT_SKINS);
				else 
					Log.Add(_WARNING_,_T("Resource with index=%d and type=%s not found"),resources[i],RT_SKINS);
			}
			else
			{
				CImage *image=new CImage();
				if(!image)
				{
					Log.Add(_ERROR_,_T("CImage object creation failed. CSkinsImageList::ImageFromRes."));
					stream->Release();
				}
				else 
				{
					HRESULT res;
					if((res=image->Load(stream))!=ERROR_SUCCESS)
					{
						SetLastError(res);
						Log.WinError(_ERROR_,_T("Image load failed. index=%d and type=%s "),resources[i],RT_SKINS);
						delete image;
						image=NULL;
					}
					else 
						insert(value_type(resources[i],boost::shared_ptr<CImage>(image)));
				}
				images[i]=image;
			}
		}
		else
			images[i]=im->second.get();
	}
CATCH_THROW()
}

CSkinnedElement::CSkinnedElement(CSkinnedElement *parent, boost::shared_ptr<CSkinsImageList> skinsImageList)
	:	m_currentImage(NULL),
		m_color1(RGB(0,0,0)),
		m_color2(RGB(0,0,0)),
		m_hFont(NULL),
		m_parent(parent),
		m_edgeColor1(-1),
		m_skinsImageList(skinsImageList)
{
TRY_CATCH
	_tcscpy_s(m_font.lfFaceName, LF_FACESIZE, NOFONT);
	m_bkColor1=::GetSysColor(COLOR_WINDOW);
	if(!m_bkColor1)
	{
		Log.Add(_WARNING_,_T("COLOR_WINDOW system color obtaining failed"));
		m_bkColor1=0xffffff;// white color
	}
	m_bkColor2=m_bkColor1;

	/// Setting up imageist if we have such
	if (NULL != m_parent)
		m_skinsImageList = m_parent->m_skinsImageList;

CATCH_LOG()
}

CSkinnedElement::~CSkinnedElement(void)
{
TRY_CATCH
	m_currentImage = NULL;
CATCH_LOG()
}

LRESULT CSkinnedElement::OnEraseBkgnd(HWND hWnd, UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
TRY_CATCH
	if(IsWindow(hWnd))
	{
		HDC hdc=reinterpret_cast<HDC>(wParam);
		PAINTSTRUCT m_ps;
		HDC hDC(hdc?hdc:(BeginPaint(WindowHandle, &m_ps)));
		RECT windowRect;
		CWindow window(hWnd);
		window.GetWindowRect(&windowRect);
		window.ScreenToClient(&windowRect);
		LRESULT res = 0L;
		if(m_currentImage && RectIsntEmpty(windowRect))
		{
			bHandled=TRUE;
			res = m_currentImage->Draw(hDC,windowRect);
		}
		if (!hdc) 
			EndPaint(WindowHandle, &m_ps);
		return res;
	}
CATCH_LOG()
	bHandled=FALSE;
	return 0L;
}

LRESULT CSkinnedElement::OnPaint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
TRY_CATCH
	return OnEraseBkgnd(hWnd,uMsg,wParam,lParam,bHandled);
CATCH_LOG()
	return 0L;
}

LRESULT CSkinnedElement::OnNCPaint(HWND hWnd, UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
TRY_CATCH
	if(IsWindow(hWnd))
	{
		HDC hdc;
		if(wParam==1)
			hdc=GetDCEx(hWnd,0,DCX_WINDOW|DCX_PARENTCLIP);
		else
			hdc=GetDCEx(hWnd,reinterpret_cast<HRGN>(wParam),DCX_WINDOW|DCX_INTERSECTRGN);
		DWORD r=GetLastError();
		RECT windowRect;
		CWindow window(hWnd);
		window.GetWindowRect(&windowRect);
		window.ScreenToClient(&windowRect);
		windowRect.right-=windowRect.left;
		windowRect.bottom-=windowRect.top;
		windowRect.left=0;
		windowRect.top=0;
		BOOL res=0;
		if(m_currentImage && hdc && RectIsntEmpty(windowRect))
		{
			res=m_currentImage->Draw(hdc,windowRect);
		}
		ReleaseDC(hWnd,hdc);
		if(res)
			return 0;
	}
CATCH_LOG()
	bHandled=FALSE;
	return 1L;//unprocessed
}

LOGFONT CSkinnedElement::GetLogFont() const
{
	return m_font;
}

LOGFONT CSkinnedElement::GetLogFont2() const
{
	return m_font2;
}


COLORREF CSkinnedElement::GetFontColor1() const
{
	return m_color1;
}

COLORREF CSkinnedElement::GetFontColor2() const
{
	return m_color2;
}

tstring CSkinnedElement::GetText() const
{
	return m_sText;
}
tstring CSkinnedElement::GetHint() const
{
	return m_hint;
}
void CSkinnedElement::SetLogFont(LOGFONT newVal)
{
	m_font = newVal;
}

void CSkinnedElement::SetLogFont2(LOGFONT newVal)
{
	m_font2 = newVal;
}

void CSkinnedElement::SetFontColor1(COLORREF newVal)
{
	m_color1 = newVal;
}

void CSkinnedElement::SetFontColor2(COLORREF newVal)
{
	m_color2 = newVal;
}

void CSkinnedElement::SetText(tstring newVal)
{
	m_sText = newVal;
}
void CSkinnedElement::SetHint(tstring newVal)
{
	m_hint = newVal;
}

void CSkinnedElement::DrawCurrentImage(HDC hDC, const int x, const int y, const RECT &rect)
{
TRY_CATCH
	if (m_currentImage == NULL)
		return;
	RECT rectDest;
	GetClientRect(WindowHandle, &rectDest);
	rectDest.left -= rect.left;
	rectDest.top -= rect.top;
	rectDest.right -= rect.left;
	rectDest.bottom -= rect.top;
	if (RectIsntEmpty(rectDest))
		m_currentImage->Draw( hDC, rectDest );
CATCH_THROW()
}

void CSkinnedElement::OwnToParent(RECT& rect)
{
TRY_CATCH
	if (Parent == NULL)
		return;
	POINT p;
	p.x = rect.left;
	p.y = rect.top;
	ClientToScreen(WindowHandle,&p);
	ScreenToClient(Parent->WindowHandle,&p);
	int dx = p.x - rect.left;
	int dy = p.y - rect.top;
	rect.left += dx;
	rect.right += dx;
	rect.bottom += dy;
	rect.top += dy;
CATCH_THROW()
}
LRESULT CSkinnedElement::OnGetDispInfo(int id, LPNMHDR lpnmhdr, BOOL &bHandled)
{
TRY_CATCH
	NMTTDISPINFO *dispInfo=reinterpret_cast<NMTTDISPINFO*>(lpnmhdr);
	dispInfo->lpszText=const_cast<LPTSTR>(m_hint.c_str());
CATCH_LOG()
	return 0;
}
void CSkinnedElement::SetEdgeColor1(COLORREF newVal)
{
	m_edgeColor1 = newVal;
}
COLORREF CSkinnedElement::GetEdgeColor1() const
{
	return m_edgeColor1;
}void CSkinnedElement::SetBkColor1(COLORREF newVal)
{
	m_bkColor1 = newVal;
}
void CSkinnedElement::SetBkColor2(COLORREF newVal)
{
	m_bkColor2 = newVal;
}
COLORREF CSkinnedElement::GetBkColor1() const
{
	return m_bkColor1;
}
COLORREF CSkinnedElement::GetBkColor2() const
{
	return m_bkColor2;
}

void CSkinnedElement::Setup(LOGFONT font,COLORREF fontColor1,COLORREF fontColor2,COLORREF bkColor1,COLORREF bkColor2,COLORREF edgeColor1,
							const tstring& text,const tstring& hint)
{
	Font=font;
	FontColor1=fontColor1;
	FontColor2=fontColor2;
	BkColor1=bkColor1;
	BkColor2=bkColor2;
	EdgeColor1=edgeColor1;
	Text=text;
	Hint=hint;
}