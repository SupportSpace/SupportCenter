/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLocale.h
///
///  current locale object
///
///  @author "Archer Software" Sogin M. @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/Localization/clocale.h>
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>

CLocale::CLocale(void)
{
TRY_CATCH
CATCH_THROW()
}

CLocale::~CLocale(void)
{
TRY_CATCH
CATCH_LOG()
}

tstring CLocale::GetString(const tstring& str)
{
TRY_CATCH
	std::map<tstring,tstring,tstring_less>::iterator i = m_strings.find(str);
	if (i == m_strings.end())
	{
		m_strings[str] = str;
		return str;
	} else
	{
		return i->second;
	}
CATCH_THROW()
}

void CLocale::SetString(const tstring& key, const tstring& value)
{
TRY_CATCH
	m_strings[key] = value;
CATCH_THROW()
}

