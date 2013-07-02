/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SControlPoint.h
///
///  Declares SControlPoint structure, responsible for control point
///
///  @author Dmitry Netrebenko @date 16.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <vector>

///  SControlPoint structure, responsible for control point
struct SControlPoint
{
	POINT		m_point;
	COLORREF	m_color;
	SControlPoint()
		:	m_color(RGB(0,0,0))
	{ m_point.x = 0; m_point.y = 0; };
};

///  Shared pointer to SControlPoint structure
typedef boost::shared_ptr<SControlPoint> SPControlPoint;

///  Vector of control points
typedef std::vector<SPControlPoint> ControlPoints;
