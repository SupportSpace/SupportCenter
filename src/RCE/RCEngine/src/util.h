/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  util.h
///
///  
///
///  @author "Archer Software"  @date 11.12.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <windows.h>
#include <AidLib/Utils/Utils.h>

/// Register class. ReRegister if such exists. Throw exception on error
ATOM RegisterClassForced(WNDCLASSEX& wcx);

/// Register class. ReRegister if such exists. Throw exception on error
ATOM RegisterClassForced(WNDCLASS& wcx);
