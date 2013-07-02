/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFrame.h
///
///  Declares CFrame class, responsible for one frame
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include "Rects.h"
#include <vector>
#include "SControlPoint.h"

///  Declares CFrame class, responsible for one frame
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
	HBITMAP			m_bitmap;
///  Frame's DC
	HDC				m_bitDC;
///  Frame control points
	ControlPoints	m_controlPoints;
///  Bound rectangle
	SPRect			m_boundRect;

public:
///  Draws bitmap on window
	void Draw();

///  Prepares frame bitmap
///  @param rects - vector of rectangles
///  @param points - vector of control points
///  @param prevFrameRect - previous frame bound rectangle
	void Init( const Rects& rects, const ControlPoints& points, SPRect prevFrameRect );

///  Returns vector with control points
	ControlPoints& GetControlPoints();

///  Returns bound rect
	SPRect GetBoundRect() const;

};

///  Shared pointer to CFrame class
typedef boost::shared_ptr<CFrame> SPFrame;

///  Vector of frames
typedef std::vector<SPFrame> Frames;
