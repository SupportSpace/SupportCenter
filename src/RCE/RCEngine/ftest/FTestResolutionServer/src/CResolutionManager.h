/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResolutionManager.h
///
///  Declares CResolutionManager class, responsible for management of allowed 
///    screen resolutions
///
///  @author Dmitry Netrebenko @date 12.06.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <AidLib/Loki/Singleton.h>

///  Shared pointer to DEVMODE structure
typedef boost::shared_ptr<DEVMODE> SPDevMode;
///  Vector of pointers to DEVMODE structures
typedef std::vector<SPDevMode> DevModes;

///  CResolutionManager class, responsible for management of allowed 
///    screen resolutions
///  Access through singleton
class CResolutionManager
{
private:
///  Prevents making copies of CResolutionManager objects.
	CResolutionManager( const CResolutionManager& );
	CResolutionManager& operator=( const CResolutionManager& );

public:
///  Constructor
	CResolutionManager();
///  Destructor
	~CResolutionManager();

private:
///  Allowed modes
	DevModes		m_devModes;
///  Index of the current mode
	int				m_currentModeIndex;

public:
///  Returns next mode
	SPDevMode GetNextMode();

///  Initializes internal structures
	void Init();
};

/// Should be used to CResolutionManager as single instance
#define RESOLUTIONMANAGER_INSTANCE Loki::SingletonHolder<CResolutionManager, Loki::CreateUsingNew, Loki::DefaultLifetime, Loki::ClassLevelLockable>::Instance()
