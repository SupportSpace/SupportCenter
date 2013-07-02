/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  NetworkLayer.h
///
///  Dynamic/Static, Export/Import defines
///
///  @author Dmitry Netrebenko @date 09.10.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _DYNAMIC_NWL_
	#ifdef _EXPORT_NWL_
		#define NWL_API __declspec(dllexport)
	#else
		#define NWL_API __declspec(dllimport)
	#endif
#else
	#define NWL_API  
#endif

#pragma comment(lib, "Rpcrt4.lib")

#pragma warning( disable : 4251 4275 )

/// Session stop reason enumeration
typedef enum _ESessionStopReason
{
	LOCAL_STOP		= 0,	/// Stop() called; 
	REMOTE_STOP		= 1,	/// Remote Stop() called; 
	STREAM_ERROR	= 2,	/// Stream signaled 
	PROTOCOL_ERROR	= 3,	/// Some protocol error
	CHANGE_DISPLAY_MODE,
	CONNECTING_ERROR,		///when connection has not.
	OPENFILE_ERROR			///open file failed
} ESessionStopReason;