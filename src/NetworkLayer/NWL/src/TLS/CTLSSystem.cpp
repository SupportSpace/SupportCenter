/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTLSSystem.cpp
///
///  Implements CTLSSystem class, responsible for initializing GnuTLS library
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#include <NWL/TLS/CTLSSystem.h>
#include <NWL/TLS/TLSWrapper.h>
#include <malloc.h>

//static struct gcry_thread_cbs gcry_threads_other;// = { asd };

CTLSSystem::CTLSSystem()
: m_bInited(false)
{
	//gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_other);
	m_bInited = ( 0 == gnutls_global_init() );
	if ( !m_bInited )
		return;
	m_bInited = m_bInited && ( 0 == gnutls_global_init_extra() );
}

CTLSSystem::~CTLSSystem()
{
	if ( m_bInited )
	{
		gnutls_global_deinit();
	}
}

bool CTLSSystem::Initialized()
{
	return m_bInited;
}
