/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SPTypes.h
///
///  Declares shared pointer types for some windows objects
///
///  @author Dmitry Netrebenko @date 18.07.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>

///  Shared pointer to DEVMODE structure
typedef boost::shared_ptr<DEVMODE> SPDevMode;

///  Shared pointer to HANDLE type
typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type > SPHandle;

///  Shared pointer to HDC type
typedef boost::shared_ptr< boost::remove_pointer<HDC>::type > SPDC;

///  Shared pointer to HBITMAP type
typedef boost::shared_ptr< boost::remove_pointer<HBITMAP>::type > SPBitmap;

///  Shared pointer to HBRUSH type
typedef boost::shared_ptr< boost::remove_pointer<HBRUSH>::type > SPBrush;
