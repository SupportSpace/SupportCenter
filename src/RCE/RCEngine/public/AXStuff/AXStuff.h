/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  AXStuff.h
///
///  stuff definition for ActiveX
///
///  @author "Archer Software" Kirill Solovyov @date 2.03.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once
//TODO #include "cexception.h"

/// dual coclass return handler
/// see for detail RCE-185
#define CATCH_LOG_COMERROR(...)\
		return S_OK;\
	CATCH_LOG()\
	return Error(L"");//CComCoClass<>
	