/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInstaller.h
///
///  CInstaller object declaration 
///
///  @author Kirill Solovyov @date 28.01.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <AidLib/Strings/tstring.h>
#include <windows.h>
#include <msi.h>

#define GUID_LENGTH			39		/// Including trailing NULL
//#define UNICODE_MAX_PATH	32768	/// maximum length for a unicode path 
#define ACTION_COUNT_GUESS	23		/// assumed actions count


class CInstaller
{
public:
	CInstaller(void);
	virtual ~CInstaller(void);

	/// old UI message handler
	INSTALLUI_HANDLER	m_oldUI;
	/// internal (for this class, external for WI) function UI message handler
	static INT CALLBACK InstallUIHandler(LPVOID pvContext,UINT iMessageType,LPCTSTR szMessage);
	///	internal current Total tick value for current state. used in InstallUIHandler function
	int m_progressTotal;
	///	internal current tick value for current state. used in InstallUIHandler function
	int m_progressCurrent;
	///	internal InstallUIHandler return value, used for canceling WI operations
	DWORD m_cancelInstalling;
	/// internal current progress message
	tstring m_progressStatus;
	/// internal result of installation
	DWORD m_result;
	///
	DWORD m_logMode;
	///
	tstring m_logFile;
	///
	DWORD m_logAttributes;




	/// CRCInstaller component id in msi package. Generate by GUIDGEN in RCComponents (WiX) project. 
	const TCHAR*const m_GUIDCRCInstaller;

	/// The DirectConfigureProductEx method installs or uninstalls a product. A product command line may be specified.
	/// @param commandLine Specifies the command line property settings. This should be a list of the format Property=Setting Property=Setting. The command line passed in as szCommandLine can contain any of the Feature Installation Options Properties (MSDN > Windows Installer > Properties).
	/// @return 
	UINT DirectConfigureProductEx(const tstring& commandLine);

	/// The event notifies that installation has completed.
	/// @param result Equal 0 if feature installation is success, nonzero value otherwise.
	virtual void OnInstalled(LONG result){};

	/// The event notifies progress of installation
	/// @param percentCompleted percent completed of installing process
	/// @param status Status is a text message that describes the current step.
	virtual void OnInstalling(LONG percentCompleted, const tstring& status){};


	/// Return description from action string
	/// The format of this string is Action [1]: [2]. [3] where:
	/// [1] is the TIME the action started
	/// [2] is the name of the action (from the particular Sequence table)
	/// [3] is the description of the action (as described in the Description column of the ActionText table or established via a MsiProcessMessage call)
	/// For example:
	/// Action 16:31:56: RemoveRegistryValues. Removing system registry 
	/// @see http://msdn2.microsoft.com/En-US/library/aa370573.aspx
	/// @if there's no action description, then action name is returned
	static tstring GetActionDescription(const tstring& actionString);

	/// The SetInternalUI method enables the installer's internal user interface. Then this user interface is used for all subsequent calls to user-interface-generating installer functions in this process.
	/// @param dwUILevel Specifies the level of complexity of the user interface. This parameter can be one of the following values: \n INSTALLUILEVEL_NOCHANGE = 0,		// UI level is unchanged	\n INSTALLUILEVEL_DEFAULT  = 1,    // default UI is used	\n INSTALLUILEVEL_NONE     = 2,    // completely silent installation	\n INSTALLUILEVEL_BASIC    = 3,    // simple progress and error handling	\n INSTALLUILEVEL_REDUCED  = 4,    // authored UI, wizard dialogs suppressed	\n INSTALLUILEVEL_FULL     = 5,    // authored UI with wizards, progress, errors	\n INSTALLUILEVEL_ENDDIALOG    = 0x80, // display success/failure dialog at end of install	\n INSTALLUILEVEL_PROGRESSONLY = 0x40, // display only progress dialog	\n INSTALLUILEVEL_HIDECANCEL   = 0x20, // do not display the cancel button in basic UI	\n INSTALLUILEVEL_SOURCERESONLY = 0x100, // force display of source resolution even if quiet
	/// @param dwOldUILevel	[out,retval] The previous user interface level is returned. If an invalid dwUILevel is passed, then INSTALLUILEVEL_NOCHANGE = 0 is returned.
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	void SetInternalUI(ULONG dwUILevel, ULONG* dwOldUILevel);
	/// The EnableLog method sets the log mode for all subsequent installations that are initiated in the calling process.
	/// Create file name due to change extension Component Path full file name on a ".log".
	/// @param logMode Specifies the log mode. This parameter can be one or more of the following values:\n0x00000001 INSTALLLOGMODE_FATALEXIT Logs out of memory or fatal exit information.\n0x00000002 INSTALLLOGMODE_ERROR Logs the error messages.\n0x00002000 INSTALLLOGMODE_EXTRADEBUG Sends extra debugging information, such as handle creation information, to the log file.	Windows XP/2000 and Windows 98/95:  This feature is not supported.\n0x00000004 INSTALLLOGMODE_WARNING Logs the warning messages.\n0x00000008 INSTALLLOGMODE_USER Logs the user requests.\n0x00000010 INSTALLLOGMODE_INFO Logs the status messages that are not displayed.\n0x00000040 INSTALLLOGMODE_RESOLVESOURCE Request to determine a valid source location.\n0x00000080 INSTALLLOGMODE_OUTOFDISKSPACE Indicates insufficient disk space.\n0x00000100 INSTALLLOGMODE_ACTIONSTART Logs the start of new installation actions.\n0x00000200 INSTALLLOGMODE_ACTIONDATA Logs the data record with the installation action.\n0x00000800 INSTALLLOGMODE_COMMONDATA Logs the parameters for user-interface initialization.\n0x00000400 INSTALLLOGMODE_PROPERTYDUMP Logs the property values at termination.\n0x00001000 INSTALLLOGMODE_VERBOSE Sends large amounts of information to a log file not generally useful to users. May be used for technical support.\n0x00004000 INSTALLLOGMODE_SHOWDIALOG \n Set to zero disable logging.
	/// @param logAttributes Specifies how frequently the log buffer is to be flushed. \n0x00000001 INSTALLLOGATTRIBUTES_APPEND If this value is set, the installer appends the existing log specified by szLogFile. If not set, any existing log specified by szLogFile is overwritten.\n0x00000002 INSTALLLOGATTRIBUTES_FLUSHEACHLINE Forces the log buffer to be flushed after each line. If this value is not set, the installer flushes the log buffer after 20 lines by calling FlushFileBuffers. 
	/// @return If method is successes return S_OK, otherwise E_FAIL.
	void EnableLog(ULONG logMode,const tstring& logFile,ULONG logAttributes);
};
