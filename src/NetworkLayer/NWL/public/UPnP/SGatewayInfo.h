/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SGatewayInfo.h
///
///  Declares SGatewayInfo structure for gateway information
///
///  @author Dmitry Netrebenko @date 11.10.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

#define GATEWAY_INFO_FIELD_SIZE 128

///  Gateway device information structure
#pragma pack(push)
#pragma pack(1)
struct NWL_API SGatewayInfo
{
	char m_name			[GATEWAY_INFO_FIELD_SIZE];		/// Device name
	char m_manufacturer	[GATEWAY_INFO_FIELD_SIZE];		/// Manufacturer
	char m_manURL		[GATEWAY_INFO_FIELD_SIZE];		/// Manufacturer's URL
	char m_model		[GATEWAY_INFO_FIELD_SIZE];		/// Model name
	char m_modelDesc	[GATEWAY_INFO_FIELD_SIZE];		/// Model's description
	char m_modelURL		[GATEWAY_INFO_FIELD_SIZE];		/// Model's URL
};
#pragma pack(pop)

