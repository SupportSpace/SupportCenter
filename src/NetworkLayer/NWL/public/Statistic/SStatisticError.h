/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SStatisticError.h
///
///  Declares SStatisticError - structure of statistic error
///
///  @author Dmitry Netrebenko @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>
#include <AidLib/Strings/tstring.h>

#define STAT_ERROR_SIZE 128

#pragma pack(push)
#pragma pack(1)
struct NWL_API SStatisticError
{
	int		m_code;						/// Error code
	char	m_msg[STAT_ERROR_SIZE];		/// Error message
	/// Initializes structure 
	/// @param code - error code
	/// @param msg - error message
	/// @remark we must use this function instead of constructor, 
	///   because with constructor we have problems at SStatisticMessage compilation
	void Init(const int code, const tstring& msg)
	{
		memset(this, 0, sizeof(SStatisticError));
		m_code = code;
		memcpy(m_msg, msg.c_str(), min(STAT_ERROR_SIZE, msg.length()));
	};
};
#pragma pack(pop)

