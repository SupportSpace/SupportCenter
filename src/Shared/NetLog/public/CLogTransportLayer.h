/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogTransportLayer.h
///
///  Declares CLogTransportLayer class, base class for logging transport layer
///
///  @author Dmitry Netrebenko @date 22.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <boost/shared_ptr.hpp>

///  Forward declaration
class CNetworkLog;

///  CLogTransportLayer class, base class for logging transport layer
class CLogTransportLayer
{
private:
///  Prevents making copies of CLogTransportLayer objects.
	CLogTransportLayer( const CLogTransportLayer& );
	CLogTransportLayer& operator=( const CLogTransportLayer& );

public:
///  Constructor
///  @param name - log name
///  @param netLog - reference to CNetworkLog
	CLogTransportLayer( const tstring& name, CNetworkLog& netLog );

///  Destructor
	~CLogTransportLayer();

protected:
///  Log name
	tstring			m_name;
///  Reference to NetworkLog
	CNetworkLog&	m_netLog;
};

///  Shared pointer to CLogTransportLayer
typedef boost::shared_ptr<CLogTransportLayer> SPLogTransportLayer;
