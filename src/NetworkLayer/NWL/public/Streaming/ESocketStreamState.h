/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ESocketStreamState.h
///
///  Declares ESocketStreamState enumeration
///
///  @author Dmitry Netrebenko @date 11.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

/// ESocketStreamState enumeration
enum ESocketStreamState
{
	ssDisconnecting			=	0,
	ssDisconnected			=	1,
	ssConnecting			=	2,
	ssConnected				=	3
};
