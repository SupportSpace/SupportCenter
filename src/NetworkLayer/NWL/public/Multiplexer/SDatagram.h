//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  SDatagram.h
///
///  Declares SDatagram structure
///  Using in the multiplexed stream as a transport unit for data
///
///  @author Alexander Novak @date 25.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <windows.h>
//========================================================================================================

#define MAX_DATAGRAM_SIZE			1024*2
//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SDatagram
{
	/// Service identifier (sub stream identifier)
	unsigned int	m_serviceID;
	
	/// Real size of data in datagram
	unsigned int	m_dataSize;
	
	/// Useful data
	BYTE			m_data[MAX_DATAGRAM_SIZE];

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////