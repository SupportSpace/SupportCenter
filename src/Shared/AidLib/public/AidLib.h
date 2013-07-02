/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  AidLib.h
///
///  Dynamic/Static, Export/Import defines
///
///  @author Dmitry Netrebenko @date 25.12.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _DYNAMIC_AID_
	#ifdef _EXPORT_AID_
		#define AIDLIB_API __declspec(dllexport)
	#else
		#define AIDLIB_API __declspec(dllimport)
	#endif
#else
	#define AIDLIB_API  
#endif

#pragma comment(lib, "Rpcrt4.lib")

#pragma warning( disable : 4251 4275 )
