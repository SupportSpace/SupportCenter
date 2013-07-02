/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSocketSystem.h
///
///  Declares CSocketSystem class, responsible for initializing sockets
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

///  CSocketSystem class, responsible for initializing sockets
///  @remarks
class NWL_API CSocketSystem
{
private:
/// Prevents making copies of CSocketSystem objects.
	CSocketSystem( const CSocketSystem& );
	CSocketSystem& operator=( const CSocketSystem& );

public:
///  Constructor
	CSocketSystem();

///  Destructor
	~CSocketSystem();
	
///  Returns state of sockets
///  @return true, if sockets are initialized
///  @remarks
	bool Initialized();

private:
	bool	m_bInited;
};
