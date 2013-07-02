#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  Locale.h
///
///  Steam factory localization
///
///  @author "Archer Software" Sogin M. @date 07.04.2007
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/Localization/CLocale.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>

void InitLocale()
{
TRY_CATCH
	#include "local_strings.h"
CATCH_LOG()
}
