#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSessionEnumerator.h
///
///  Base class for session enumerators
///
///  @author "Archer Software" Sogin M. @date 02.10.2007
///
////////////////////////////////////////////////////////////////////////
#include <set>

/// Abstact class for session Enumerators
class CSessionEnumerator
{
public:
	/// Returns sessions set for localhost
	/// @param sessions [out] set of sessions for localhost
	virtual void GetSessions(std::set<int>& sessions) = NULL;

	/// dtor
	virtual ~CSessionEnumerator() {};
};