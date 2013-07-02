/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNATTraversingStreamTest.h
///
///  Declares CNATTraversingStreamTest class, test for CNATTraversingUDPNetworkStream connection
///
///  @author Dmitry Netrebenko @date 29.02.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CStreamTest.h"

/// CNATTraversingStreamTest class, test for CNATTraversingUDPNetworkStream connection
class CNATTraversingStreamTest
	:	public CStreamTest
{
private:
/// Prevents making copies of CNATTraversingStreamTest objects
	CNATTraversingStreamTest(const CNATTraversingStreamTest&);
	CNATTraversingStreamTest& operator=(const CNATTraversingStreamTest&);
public:
/// Constructor
/// @param settings - pointer to settings
	CNATTraversingStreamTest(boost::shared_ptr<CSettings> settings);
/// Destructor
	~CNATTraversingStreamTest();
protected:
/// Creates stream
	virtual boost::shared_ptr<CAbstractNetworkStream> CreateStream(boost::shared_ptr<SThreadInfo> threadInfo);
};

