/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CBlockBitmap.h
///
///  Declares CBlockBitmap class, responsible for template bitmap
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Loki/Singleton.h>

///  CBlockBitmap class, responsible for template bitmap
///  Access through CSingleton
class CBlockBitmap
{
private:
///  Prevents making copies of CBlockBitmap objects.
	CBlockBitmap( const CBlockBitmap& );
	CBlockBitmap& operator=( const CBlockBitmap& );

public:
///  Constructor
	CBlockBitmap();
///  Destructor
	~CBlockBitmap();

private:
///  Template bitmap
	HBITMAP			m_bitmap;
///  Templates's DC
	HDC				m_bitDC;

public:
///  Returnt template's DC
	HDC GetDC();
};

/// Should be used to CBlockBitmap as single instance
#define BLOCKBITMAP_INSTANCE Loki::SingletonHolder<CBlockBitmap, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
