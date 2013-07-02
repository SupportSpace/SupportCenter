// UTestNWL.cpp : Defines the entry point for the console application.
//

/// Setting up execution limit to 600 secs
#define LIMIT_EXECUTION_TIME 600000

#define WAIT_FOR(Expr,Time) {\
	int i = 0;\
	while(!Expr && (i < Time)) { Sleep(1); i++; } \
	}

#include "UTestNWLSettings.h"
#include <winsock2.h>
#include <boostTestLib/boostTestLib.h>

#include "CAbstractStreamTestSuite.h"
#include "CAbstractNetworkStreamTestSuite.h"
#include "CTLSAuthSettingsTestSuite.h"
#include "CDirectNetworkStreamTestSuite.h"
#include "CJabberConnector.h"
#include "CNATTraversingUDPNetworkStreamTestSuite.h"
#include "CRelayedNetworkStreamTestSuite.h"
#include "CExternalIPRequestTestSuite.h"
#include "CStreamFactoryTestSuite.h"
#include "COpenPortRequestTestSuite.h"
#include "CMultiplexStreamTestSuite.h"
#include <NWL/Streaming/CSocketSystem.h>
#include <NWL/TLS/CTLSSystem.h>
#include <AidLib/CCrypto/CCrypto.h>

CSocketSystem			g_socketSystem;
CTLSSystem				g_tlsSystem;

/// Main test project entry point
test_suite* init_unit_test_suite( int, char* [] ) 
{
	// initing singletons (threading issues)
	CRYPTO_INSTANCE;

	/// Connect Jabber users
	static CJabberConnector jabberUser1(JABBER_SERVER, JABBER_RESOURCE, JABBER_USER_NAME_1, JABBER_PASSWD_1);
	static CJabberConnector jabberUser2(JABBER_SERVER, JABBER_RESOURCE, JABBER_USER_NAME_2, JABBER_PASSWD_2);

	test_suite* root = BOOST_TEST_SUITE( "root" );
	root->add( getCAbstractStreamTestSuite() );
	root->add( getCAbstractNetworkStreamTestSuite() );
	root->add( getCTLSAuthSettingsTestSuite() );
	root->add( getCDirectNetworkStreamTestSuite() );
	root->add( getCNATTraversingUDPNetworkStreamTestSuite() );
	root->add( getCRelayedNetworkStreamTestSuite() );
	root->add( getCExternalIPRequestTestSuite() );
	root->add( getCStreamFactoryTestSuite() );
	root->add( getCOpenPortRequestTestSuite() );
	root->add( getCMultiplexStreamTestSuite() );

    return root;
}
