/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ENATTraversalStreamState.h
///
///  Declares ENATTraversalStreamState enumeration
///
///  @author Dmitry Netrebenko @date 28.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

enum ENATTraversalStreamState
{
	ntssNone				= 0,	/// Initial state
	ntssAuthRequested		= 1,	/// Authentication request message is sent
	ntssAuthResponsed		= 2,	/// Authentication response message is received
	ntssBindRequested		= 4,	/// Bind request message is sent
	ntssBindNullResponsed	= 8,	/// Bind response message without addresses is received
	ntssBindResponsed		= 16,	/// Bind response messaage with peer's addresses is received
	ntssProbeRequested		= 32,	/// Probe request messages are sent
	ntssProbeResponsed		= 64	/// Probe response message is received
};
