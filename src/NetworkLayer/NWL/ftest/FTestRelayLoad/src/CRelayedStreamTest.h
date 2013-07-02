/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRelayedStreamTest.h
///
///  Declares CRelayedStreamTest class, test for CRelayedNetworkStream connection
///
///  @author Dmitry Netrebenko @date 27.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CStreamTest.h"

/// CRelayedStreamTest class, test for CRelayedNetworkStream connection
class CRelayedStreamTest
	:	public CStreamTest
{
private:
/// Prevents making copies of CRelayedStreamTest objects
	CRelayedStreamTest(const CRelayedStreamTest&);
	CRelayedStreamTest& operator=(const CRelayedStreamTest&);
public:
/// Constructor
/// @param settings - pointer to settings
	CRelayedStreamTest(boost::shared_ptr<CSettings> settings);
/// Destructor
	~CRelayedStreamTest();
protected:
/// Creates stream
	virtual boost::shared_ptr<CAbstractNetworkStream> CreateStream(boost::shared_ptr<SThreadInfo> threadInfo);
};
