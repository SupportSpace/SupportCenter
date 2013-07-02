/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTLSAuthSettings.h
///
///  Declares CTLSAuthSettings class, responsible for GNUTLS authentication
///    settings
///
///  @author Dmitry Netrebenko @date 17.09.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/TLS/TLSStructs.h>
#include <NWL/NetworkLayer.h>

///  CTLSAuthSettings class, responsible for GNUTLS authentication
///    settings
class NWL_API CTLSAuthSettings
{
private:
/// Prevents making copies of CTLSAuthSettings objects.
	CTLSAuthSettings( const CTLSAuthSettings& );
	CTLSAuthSettings& operator=( const CTLSAuthSettings& );

public:
///  Constructor
	CTLSAuthSettings();

///  Destructor
	virtual ~CTLSAuthSettings();

protected:
/// TLS cipher suite
	STLSSuite				m_Suite;
	
/// TLS credentials	
	STLSCredentials			m_Credentials;

public:

///  Returns settings of TLS ciphersuite
///  @return STLSSuite structure
///  @remarks
	STLSSuite& GetSuite();

///  Sets settings of TLS ciphersuite
///  @param   new ciphersuite
///  @remarks
	void SetSuite( const STLSSuite& );

///  Returns credentials
///  @return STLSServerCredentials structure
///  @remarks
	STLSCredentials& GetCredentials();

///  Sets credentials
///  @param   new credentials
///  @remarks
	void SetCredentials( const STLSCredentials& );

};
