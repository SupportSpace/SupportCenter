/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestSettings.cpp
///
///  Implements CTestSettings class, responsible for storing test settings
///
///  @author Dmitry Netrebenko @date 21.02.2008
///
////////////////////////////////////////////////////////////////////////

#include "CTestSettings.h"
#include <AidLib/CException/CException.h>
#include "ETestType.h"

CTestSettings::CTestSettings(const ETestType id, const tstring& name)
	:	m_id(id)
	,	m_name(name)
	,	m_settings(new CSettings())
{
TRY_CATCH
CATCH_THROW()
}

CTestSettings::~CTestSettings()
{
TRY_CATCH
CATCH_LOG()
}

ETestType CTestSettings::GetId() const
{
TRY_CATCH
	return m_id;
CATCH_THROW()
}

tstring CTestSettings::GetName() const
{
TRY_CATCH
	return m_name;
CATCH_THROW()
}

boost::shared_ptr<CSettings> CTestSettings::GetSettings() const
{
TRY_CATCH
	return m_settings;
CATCH_THROW()
}

