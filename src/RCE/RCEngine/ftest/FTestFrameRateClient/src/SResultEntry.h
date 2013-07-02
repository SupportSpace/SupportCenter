/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SResultEntry.h
///
///  Declares SResultEntry structure, responsible for one result entry
///
///  @author Dmitry Netrebenko @date 16.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <vector>

///  SResultEntry structure, responsible for one result entry
struct SResultEntry
{
	DWORD	m_time;
	double	m_fps;
	SResultEntry()
		:	m_time(0)
		,	m_fps(0.0)
	{};
};

///  Shared pointer to result entry
typedef boost::shared_ptr<SResultEntry> SPResultEntry;

///  Vector of result entries
typedef std::vector<SPResultEntry> ResultEntries;
