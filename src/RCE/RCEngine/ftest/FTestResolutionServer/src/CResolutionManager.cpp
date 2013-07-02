/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CResolutionManager.cpp
///
///  Implements CResolutionManager class, responsible for management of allowed 
///    screen resolutions
///
///  @author Dmitry Netrebenko @date 12.06.2007
///
////////////////////////////////////////////////////////////////////////

#include "CResolutionManager.h"
#include <AidLib/CException/CException.h>
#include "CHelper.h"


int operator==(const DEVMODE& _Left, const DEVMODE& _Right)
{
	if(_Left.dmPelsWidth == _Right.dmPelsWidth && 
		_Left.dmPelsHeight == _Right.dmPelsHeight && 
		_Left.dmBitsPerPel == _Right.dmBitsPerPel && 
		_Left.dmDisplayFrequency == _Right.dmDisplayFrequency)
		return 1;
	else
		return 0;
}

CResolutionManager::CResolutionManager()
{
TRY_CATCH
CATCH_THROW()
}

CResolutionManager::~CResolutionManager()
{
TRY_CATCH
CATCH_LOG()
}

SPDevMode CResolutionManager::GetNextMode()
{
TRY_CATCH

	if(!m_devModes.size())
		return SPDevMode(reinterpret_cast<LPDEVMODE>(NULL));
	if(1 == m_devModes.size())
		return SPDevMode(reinterpret_cast<LPDEVMODE>(NULL));
	
	int index;
	do
	{
		index = CHelper::GetRandom(0, 32768);
		index %= m_devModes.size();
	} while (index == m_currentModeIndex);

	m_currentModeIndex = index;
	return m_devModes[m_currentModeIndex];

CATCH_THROW()
}

void CResolutionManager::Init()
{
TRY_CATCH

	m_currentModeIndex = -1;
	m_devModes.clear();

	SPDevMode mode(new DEVMODE());
	mode->dmSize = sizeof(DEVMODE);

	if(!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, mode.get()))
		throw MCException(_T("Can not obtain current display settings"));

	for(int i = 0;; ++i)
	{
		SPDevMode nextMode(new DEVMODE());
		nextMode->dmSize = sizeof(DEVMODE);
		if(!EnumDisplaySettings(NULL, i, nextMode.get()))
			break;
		m_devModes.push_back(nextMode);
// TODO: determinate current mode index
//		if(*nextMode == *mode)
//			m_currentModeIndex = m_devModes.size();
	}
	if(!m_devModes.size())
		throw MCException(_T("Display settings list is empty"));

CATCH_THROW()
}

