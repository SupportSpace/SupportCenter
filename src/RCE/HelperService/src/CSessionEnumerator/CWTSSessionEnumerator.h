#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWTSSessionEnumerator.h
///
///  Class, enumerating sessions through Terminal Services
///
///  @author "Archer Software" Sogin M. @date 02.10.2007
///
////////////////////////////////////////////////////////////////////////
#include "CSessionEnumerator.h"

/// Class, enumerating sessions through Terminal Services
class CWTSSessionEnumerator : public CSessionEnumerator
{
public:
	/// Returns sessions set for localhost
	/// @param sessions [out] set of sessions for localhost
	virtual void GetSessions(std::set<int>& sessions);
};

