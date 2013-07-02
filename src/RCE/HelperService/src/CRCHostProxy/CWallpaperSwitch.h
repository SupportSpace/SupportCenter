#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CWallpaperSwitch.cpp
///
///  Class for switching (temporary hiding) wallpaper
///
///  @author "Archer Software" Sogin M. @date 14.11.2007
///
////////////////////////////////////////////////////////////////////////
#include <AidLib/WatchDog/CProcessWatchDog.h>

#define STOP_TIMEOUT 10000
#define RESTORE_WALL_TIMEOUT 3000

extern const tstring g_enableWallCmd;
extern const tstring g_disableWallCmd;

/// Class for switching (temporary hiding) wallpaper
class CWallpaperSwitch : protected CProcessWatchDog
{
private:
	/// There's great issue with restoring wallpaper, when running from user, different from desktop 
	/// original user. Solution - use proxy app, with high integrity which is started from desktop original user
	/// it's assumed, that first request on wall hide will come exact from desktop original user
	/// this method run proxy app, with /restorewall paramether, with original user security token
	static void RestoreWall();
protected:
	/// @see RestoreWall
	static int m_originalPid;
public:
	/// Enables or disables active desktop
	static void EnableActiveDesktop(const bool enable = true);

	/// ctor
	CWallpaperSwitch();
	/// dtor
	virtual ~CWallpaperSwitch();

	/// Hides wallpaper, if not already hidden
	/// @param pid process, requested for wallpaper hide
	void HideWallpaperRequest(const int pid);

	/// Restores wallpaper, if no more processes left, asking for wall hide
	/// @param pid process, cancelled hide request
	void RestoreWallpaperRequest(const int pid);
};

