/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogFormatter.h
///
///  Unit tests output formatter for cLog
///
///  @author Sogin Max @date 08.02.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once;

#include <boost/test/unit_test_log_formatter.hpp>
#include <AidLib/Logging/cLog.h>

namespace boost {

namespace unit_test {

///  Unit tests output formatter for cLog
class CLogFormatter : public unit_test_log_formatter 
{
private:
	bool m_logValuePending;
	tstring m_logValue;
	log_entry_data m_lePending;
public:
    // Constructor
	explicit CLogFormatter( );

    // Destructor
    virtual ~CLogFormatter();

    // Formatter interface
	// Formatter interface
    virtual void        log_start( std::ostream&, counter_t test_cases_amount )
	{
	}

    virtual void        log_finish( std::ostream& )
	{
	}

    virtual void        log_build_info( std::ostream& )
	{
	}

    virtual void        test_unit_start( std::ostream& ostream, test_unit const& tu )
	{
		//ostream<<tu.p_type_name<<"("<<tu.p_name<<")"<<"\n";
	}

    virtual void test_unit_finish( std::ostream& ostream, test_unit const& tu, unsigned long elapsed );

	virtual void        test_unit_skipped( std::ostream&, test_unit const& )
	{
	}

    virtual void        log_exception( std::ostream&, log_checkpoint_data const&, const_string explanation );

    virtual void        log_entry_start( std::ostream& ostream, log_entry_data const& le, log_entry_types let );

	virtual void        log_entry_value( std::ostream&, const_string value );

	virtual void        log_entry_finish( std::ostream& );
};
}
}