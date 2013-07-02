/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogFormatter.cpp
///
///  Unit tests output formatter for cLog
///
///  @author Sogin Max @date 08.02.2007
///
////////////////////////////////////////////////////////////////////////

#include <boostTestLib/cLogFormatter.h>
#include <boost/test/framework.hpp>
#include <boost/test/results_collector.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/results_reporter.hpp>

// Boost
#include <boost/cstdlib.hpp>

// STL
#include <stdexcept>
#include <iostream>
#include <boost/test/detail/suppress_warnings.hpp>
#include <boost/test/unit_test_log.hpp>

// AidLib
#include <AidLib/CException/CException.h>
#pragma comment(lib, "AidLib.lib")


using namespace boost;
using namespace boost::unit_test;

HANDLE hunitTestComplete = 0;
int hangupTimeOut = 0;

CLogFormatter::CLogFormatter( )
	:	unit_test_log_formatter( ),
		m_logValuePending(false)
{
}

CLogFormatter::~CLogFormatter()
{
}

void CLogFormatter::test_unit_finish( std::ostream& ostream, test_unit const& tu, unsigned long elapsed )
{

	char* result = results_collector.results(tu.p_id).passed()?"Passed":"Failed";
	switch(tu.p_type)
	{
		case tut_case:
			Log.Add(_UTEST_CASE_(tu.p_name->c_str()),result);
			break;
		case tut_suite:
			Log.Add(_UTEST_SUITE_(tu.p_name->c_str()),result);
			break;
		default:
			Log.Add(_UTEST_SUITE_(tu.p_name->c_str()),result);
			break;
	}
}

void CLogFormatter::log_exception( std::ostream&, log_checkpoint_data const& checkPointData, const_string expl )
{
	//TODO: output checkPointData
	Log.Add(_ERROR_,expl.begin());
}

void CLogFormatter::log_entry_start( std::ostream& ostream, log_entry_data const& le, log_entry_types let )
{
	switch(let)
	{
		case BOOST_UTL_ET_INFO:
		case BOOST_UTL_ET_MESSAGE:
			break;
		case BOOST_UTL_ET_WARNING:
		case BOOST_UTL_ET_ERROR:
		case BOOST_UTL_ET_FATAL_ERROR:
			m_lePending = le;
			m_logValuePending = true;
			break;
	}
}

void CLogFormatter::log_entry_value( std::ostream&, const_string value )
{
	if (m_logValuePending)
	{
		if (!m_logValue.empty())
			m_logValue.append(_T(" "));
		m_logValue.append(value.begin());
	}
}

void CLogFormatter::log_entry_finish( std::ostream& )
{
	if (m_logValuePending)
	{
		m_logValuePending = false;
		CExceptionBase ex(static_cast<unsigned int>(m_lePending.m_line), const_cast<const PTCHAR>(m_lePending.m_file.c_str()), _T("-"), Format(_T("BOOST.TEST.ERROR: %s"),m_logValue.c_str()));
		m_logValue.clear();
		Log.Add(ex);
	}
}

