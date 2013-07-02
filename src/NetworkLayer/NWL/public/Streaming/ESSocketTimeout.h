/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  ESSocketTimeout.h
///
///  Declares ESSocketTimeout enumeration
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

///  ESSocketTimeout enumeration
enum ESSocketTimeout
{
	sstNone		= 0,
	sstReceive	= 1,
	sstSend		= 2,
	sstAll		= 3
};
