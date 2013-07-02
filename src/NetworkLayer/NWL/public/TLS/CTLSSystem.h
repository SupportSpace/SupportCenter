/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTLSSystem.h
///
///  Declares CTLSSystem class, responsible for initializing GnuTLS library
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

///  CTLSSystem class, responsible for initializing GnuTLS library
///  @remarks
class NWL_API CTLSSystem
{
private:
/// Prevents making copies of CTLSSystem objects.
	CTLSSystem( const CTLSSystem& );
	CTLSSystem& operator=( const CTLSSystem& );

public:
///  Constructor
	CTLSSystem();

///  Destructor
	~CTLSSystem();

///  Returns state of the library
///  @return true, if library is initialized
///  @remarks
	bool Initialized();

private:
	bool	m_bInited;
};
