/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  EClientType.h
///
///  Declares EClientType enumeration
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

///  EClientType enumeration (client connection type)
enum EClientType
{
	ctGloox			= 0,	/// Uses gloox library
	ctWebXml		= 1,	/// Uses WebXml activeX
	ctJajc			= 2		/// Uses JAJC library
};
