/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SScriptEngineMsg.h
///
///  Declares SScriptEngineMsg structure, responsible for Script Engine 
///    protocol message
///
///  @author Dmitry Netrebenko @date 15.11.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "EScriptEngineMsgType.h"

#define SCRIPTENGINE_MSG_HEAD_SIZE (sizeof(SScriptEngineMsg) - 1)

#pragma pack(push)
#pragma pack(1)
///  SScriptEngineMsg structure, responsible for Script Engine 
///    protocol message
struct SScriptEngineMsg
{
	unsigned int			m_size;		/// Size of message
	EScriptEngineMsgType	m_type;		/// Type of message
	unsigned int			m_id;		/// Request Id
	char					m_data[1];	/// Data, depends of message type
};
#pragma pack(pop)

