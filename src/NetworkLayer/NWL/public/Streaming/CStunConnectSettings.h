/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CStunConnectSettings.h
///
///  Declares CStunConnectSettings class, responsible for management of
///    properties of connection to STUN server
///
///  @author Dmitry Netrebenko @date 27.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

///  CStunConnectSettings class, responsible for management of
///    properties of connection to STUN server
///  @remarks
class NWL_API CStunConnectSettings
{
public:
///  Constructor
	CStunConnectSettings();

///  Destructor
	~CStunConnectSettings();

protected:
///  Bind requests delay
	unsigned int		m_nBindRetryDelay;
///  Max count of bind requests
	unsigned int		m_nBindMaxRetryCount;
///  Probe requests delay
	unsigned int		m_nProbeRetryDelay;
///  Max count of probe requests
	unsigned int		m_nProbeMaxRetryCount;
///  Port range for probe requests
	unsigned int		m_nProbePortRange;
///  Auth requests delay
	unsigned int		m_nAuthRetryDelay;
///  Max count of auth requests
	unsigned int		m_nAuthMaxRetryCount;

public:
///  Returns delay for bind requests
///  @return delay in msecs
///  @remarks
	unsigned int GetBindRetryDelay() const;

///  Returns max count of bind requests
///  @return max count of bind requests
///  @remarks
	unsigned int GetBindMaxRetryCount() const;

///  Returns delay for probe requests
///  @return delay in msecs
///  @remarks
	unsigned int GetProbeRetryDelay() const;

///  Returns max count of probe requests
///  @return max count of probe requests
///  @remarks
	unsigned int GetProbeMaxRetryCount() const;

///  Returns port range for probe requests
///  @return port range
///  @remarks
	unsigned int GetProbePortRange() const;

///  Returns delay for auth requests
///  @return delay in msecs
///  @remarks
	unsigned int GetAuthRetryDelay() const;

///  Returns max count of auth requests
///  @return max count of auth requests
///  @remarks
	unsigned int GetAuthMaxRetryCount() const;

///  Sets up bind requests retries
///  @param   requests delay
///  @param   max count of requests
///  @remarks
	void SetBindRetry( const unsigned int&, const unsigned int& );

///  Sets up probe requests retries
///  @param   requests delay
///  @param   max count of requests
///  @remarks
	void SetProbeRetry( const unsigned int&, const unsigned int& );

///  Sets up port range for probe requests
///  @param   port range
///  @remarks
	void SetProbePortRange( const unsigned int& );

///  Sets up auth requests retries
///  @param   requests delay
///  @param   max count of requests
///  @remarks
	void SetAuthRetry( const unsigned int&, const unsigned int& );

};
