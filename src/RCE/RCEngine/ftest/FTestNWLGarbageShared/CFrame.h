/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrame.h
///
///  Declares CFrame class, responsible for frame 
///
///  @author Dmitry Netrebenko @date 07.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include "SControlPoint.h"

///  Shared pointer to HDC type
typedef boost::shared_ptr< boost::remove_pointer<HDC>::type > SPDC;
///  Shared pointer to HBITMAP type
typedef boost::shared_ptr< boost::remove_pointer<HBITMAP>::type > SPBitmap;

///  CFrame class, responsible for frame 
class CFrame
{
private:
///  Prevents making copies of CFrame objects.
	CFrame( const CFrame& );
	CFrame& operator=( const CFrame& );

public:
///  Constructor
	CFrame();
///  Destructor
	~CFrame();

private:
///  Frame's bitmap
	SPBitmap			m_bitmap;
///  Frame's DC
	SPDC				m_bitDC;
///  Control points
	ControlPoints		m_points;

public:
///  Draws bitmap on window
	void Draw();

///  Initializes frame
///  @param firstFilled - is first block is filled
	void Init( bool firstFilled );

///  Checks up is DC has current frame
///  @param hdc - tested DC
///  @return true if DC has current frame
	bool Check( HDC hdc );
};

///  Shared pointer to frame
typedef boost::shared_ptr<CFrame> SPFrame;
