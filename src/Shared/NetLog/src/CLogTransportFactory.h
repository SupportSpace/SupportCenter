/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogTransportFactory.h
///
///  Declares CLogTransportFactory class, transport layer factory for NetLog
///
///  @author Dmitry Netrebenko @date 30.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NetLog/CLogTransportLayer.h>

///  CLogTransportFactory class, transport layer factory for NetLog
///  Singleton
class CLogTransportFactory
{
private:
///  Prevents making copies of CLogTransportFactory objects.
	CLogTransportFactory( const CLogTransportFactory& );
	CLogTransportFactory& operator=( const CLogTransportFactory& );
///  Constructor
	CLogTransportFactory();

public:
///  Accessor
	static CLogTransportFactory& instance()
	{
		static CLogTransportFactory obj;
		return obj;
	}

///  Destructor
	~CLogTransportFactory();

///  Returns transport layer for network log
///  @param name - log name
///  @param netLog - reference to CNetworkLog
	SPLogTransportLayer GetTransportLayer( const tstring& name, CNetworkLog& netLog, tstring& errorString );
};
