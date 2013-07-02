//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  SCacheStreamItem.h
///
///  Declares SCacheStreamItem structure
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>
//========================================================================================================

typedef boost::shared_ptr<boost::remove_pointer<HANDLE>::type> autohandle_t;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SCacheStreamItem
{
	unsigned int m_streamId;
	DWORD m_lastUsageTime;
	bool m_inUse;
	DWORD m_internalError;
	autohandle_t m_fireEvent;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////