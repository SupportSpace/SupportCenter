/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTestResults.h
///
///  Declares CTestResults class, responsible for managementof test results
///
///  @author Dmitry Netrebenko @date 16.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "SResultEntry.h"
#include <boost/shared_ptr.hpp>

///  CTestResults class, responsible for managementof test results
class CTestResults
{
private:
///  Prevents making copies of CTestResults objects.
	CTestResults( const CTestResults& );
	CTestResults& operator=( const CTestResults& );

public:
///  Constructor
	CTestResults();
///  Destructor
	~CTestResults();

private:
///  Result entries
	ResultEntries		m_entries;
///  Current entry
	int					m_current;
///  Number of entries
	int					m_entriesCount;

public:
///  Saves test results to file
	void SaveResults();

///  Adds new result entry
///  @param time - frame time
///  @param fps - fps
	void AddEntry( DWORD time, double fps );
};

///  Shared pointer to CTestResults
typedef boost::shared_ptr<CTestResults> SPTestResults;
