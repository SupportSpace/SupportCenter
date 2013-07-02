/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  Rects.h
///
///  Declares additional types to manage rectangles
///
///  @author Dmitry Netrebenko @date 14.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once 

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <algorithm>

///  Shared pointer to RECT structure
typedef boost::shared_ptr<RECT> SPRect;

///  Vector of rectangles
typedef std::vector<SPRect> Rects;

///  Unary function for detection is rectangle is empty
struct RectEmpty : public std::unary_function<SPRect, bool>
{
	bool operator()( const SPRect rect ) const
	{
		if(!rect.get())
			return true;
		return (!(rect->right - rect->left) && !(rect->bottom - rect->top));
	};
};

///  Binary function for composition of the two rectangles
struct RectComposition : public std::binary_function<SPRect, SPRect, SPRect>
{
	SPRect operator()( const SPRect _Left, const SPRect _Right ) const
	{
		if(RectEmpty()(_Left))
			return _Right;
		if(RectEmpty()(_Right))
			return _Left;
		SPRect rect(new RECT());
		rect->left = min(_Left->left, _Right->left);
		rect->top = min(_Left->top, _Right->top);
		rect->right = max(_Left->right, _Right->right);
		rect->bottom = max(_Left->bottom, _Right->bottom);
		return rect;
	};
};
