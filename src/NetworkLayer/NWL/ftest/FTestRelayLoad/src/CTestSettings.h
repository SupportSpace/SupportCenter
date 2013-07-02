/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestSettings.h
///
///  Declares CTestSettings class, responsible for storing test settings
///
///  @author Dmitry Netrebenko @date 21.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <boost/shared_ptr.hpp>
#include "CSettings.h"
#include "ETestType.h"

/// CTestSettings class, responsible for storing test settings
class CTestSettings
{
private:
/// Prevents making copies of CTestSettings objects
	CTestSettings(const CTestSettings&);
	CTestSettings& operator=(const CTestSettings&);
public:
/// Constructor
	CTestSettings(const ETestType id, const tstring& name);
/// Destructor
	~CTestSettings();
private:
/// Id of test
	ETestType		m_id;
/// Test name
	tstring			m_name;
/// User settings
	boost::shared_ptr<CSettings>	m_settings;
public:
/// Returns Id of test
	ETestType GetId() const;
/// Returns test name
	tstring GetName() const;
/// Returns settins
	boost::shared_ptr<CSettings> GetSettings() const;
};
