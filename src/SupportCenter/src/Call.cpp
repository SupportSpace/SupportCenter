#include "StdAfx.h"
#include "Call.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

CCall::CCall(void)
{
	TRY_CATCH

	CATCH_THROW(_T("CCall ::CCall"))
}

CCall::~CCall(void)
{
	TRY_CATCH

	CATCH_LOG(_T("CCall ::~CCall"))
}
