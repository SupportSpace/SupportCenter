// FTestRelay.cpp : Defines the entry point for the console application.
//
#include "CStressTest.h"
#include <AidLIb/Logging/cLog.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/scoped_ptr.hpp>
#include <AidLib/CException/CException.h>
#include "CHelper.h"
#include <AidLib/CSingleton/CSingleton.h>
#include "CSettings.h"
#include <NWL/Streaming/CRelayedNetworkStream.h>
#include <NWL/Streaming/CNATTraversingUDPNetworkStream.h>
#include "CRelayConnector.h"
#include <NWL/Streaming/CNetworkLayer.h>

void ParseCommandLine();

/// Initializes sockets and TLS
CTLSSystem					g_TLSSys;
CSocketSystem				g_SockSys;


int _tmain(int argc, _TCHAR* argv[])
{
TRY_CATCH
	Log.RegisterLog(new cFileLog(),true,cLog::_EXCEPTION);

	NWL_INSTANCE;

	ParseCommandLine();

	switch(CSingleton<CSettings>::instance().m_type)
	{
	case ttRelayedStreamTest:
		{
			CStressTest test(
				CSingleton<CSettings>::instance().m_poolSize, 
				CSingleton<CSettings>::instance().m_peersCount);
			printf("Creation of test is completed. Starting connections...\n");
			test.Start<CRelayedNetworkStream<> >();
		}
		break;
	case ttNATStreamTest:
		{
			CStressTest test(
				CSingleton<CSettings>::instance().m_poolSize, 
				CSingleton<CSettings>::instance().m_peersCount);
			printf("Creation of test is completed. Starting connections...\n");
			test.Start<CNATTraversingUDPNetworkStream>();
		}
		break;
	case ttConnectTest:
		{
			CRelayConnector test(CSingleton<CSettings>::instance().m_poolSize);
			printf("Creation of test is completed. Starting connections...\n");
			test.Start(CSingleton<CSettings>::instance().m_peersCount);
		}
		break;
	default:
		throw MCException(_T("Invalid test type"));
	}

CATCH_LOG()

	return 0;
}

void ParseCommandLine()
{
TRY_CATCH

	int nArgc = 0;
	// Extract arguments from command line
	PTCHAR* pArgs = CHelper::CommandLineToArgv(
			GetCommandLine(),
			&nArgc
		);

	if (!pArgs)
		return;

	// Create shared pointer for auto destroy pArgs handle
	boost::shared_ptr< boost::remove_pointer<HGLOBAL>::type > spArgs((HGLOBAL)pArgs, GlobalFree);

	tstring param(_T(""));
	while(*(++pArgs))
	{
		param.assign(*pArgs);
		tstring paramName = param.substr(0, 3);
		paramName = UpperCase(paramName);
		param.erase(0, 3);

		if(!paramName.compare(_T("-A=")))
		{
			CSingleton<CSettings>::instance().m_host = param;
		}
		else
		{
			if(!paramName.compare(_T("-P=")))
			{
				CSingleton<CSettings>::instance().m_port = atoi(param.c_str());
			}
			else
			{
				if(!paramName.compare(_T("-S=")))
				{
					CSingleton<CSettings>::instance().m_passwd = param;
				}
				else
				{
					if(!paramName.compare(_T("-T=")))
					{
						CSingleton<CSettings>::instance().m_type = static_cast<ETestType>(atoi(param.c_str()));
					}
					else
					{
						if(!paramName.compare(_T("-L=")))
						{
							CSingleton<CSettings>::instance().m_poolSize = atoi(param.c_str());
							if(CSingleton<CSettings>::instance().m_poolSize <= 0)
								CSingleton<CSettings>::instance().m_poolSize = DEF_POOL_SIZE;
						}
						else
						{
							if(!paramName.compare(_T("-E=")))
							{
								CSingleton<CSettings>::instance().m_peersCount = atoi(param.c_str());
								if(CSingleton<CSettings>::instance().m_peersCount <= 0)
									CSingleton<CSettings>::instance().m_peersCount = DEF_PEERS_COUNT;
							}
							else
							{
								if(!paramName.compare(_T("-X=")))
								{
									CSingleton<CSettings>::instance().m_extPort = atoi(param.c_str());
								}
								else
								{
									if(!paramName.compare(_T("-U=")))
									{
										CSingleton<CSettings>::instance().m_user = param;
									}
								}
							}
						}
					}
				}
			}
		}
	}

CATCH_THROW()
}
