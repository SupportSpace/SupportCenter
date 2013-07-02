/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  TestEvents.h
///
///  Declares events for tests
///
///  @author Dmitry Netrebenko @date 09.08.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/function.hpp>
#include <AidLib/Strings/tstring.h>

typedef boost::function<void (bool)> OnCompleteEvent;

typedef boost::function<void (unsigned int)> OnProgressEvent;

typedef boost::function<void (const tstring&)> OnMessageEvent;
