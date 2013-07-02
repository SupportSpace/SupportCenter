
#include "CPortMappingHelper.h"

// CPortMappingHelper [BEGIN] ////////////////////////////////////////////////////////////

CPortMappingHelper::CPortMappingHelper(const TCHAR* ansiIniFile):
	m_currentInfoItemIndex(0),
	m_count_active_thread(0),
	m_count_fail_thread(0),
	m_count_success_thread(0),
	m_increment(0)	
{
TRY_CATCH
	InitializeCriticalSection(&cs);
	
	m_ini_file.loadAnsiFile(ansiIniFile);
CATCH_THROW()
}
//----------------------------------------------------------------------------------------

CPortMappingHelper::~CPortMappingHelper()
{
TRY_CATCH
	DeleteCriticalSection(&cs);
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

SPortMappingInfo CPortMappingHelper::GetDataForPortMapping()
{
	SPortMappingInfo pmi;
TRY_CATCH

	EnterCriticalSection(&cs);

	pmi.m_server_address	= m_ini_file[_T("SERVER")][_T("Address")];
	pmi.m_server_port		= (unsigned short)(int)m_ini_file[_T("SERVER")][_T("Port")];
	pmi.m_search_delay		= m_ini_file[_T("CONFIG")][_T("SearchDelay")];
	pmi.m_count_to_try		= m_ini_file[_T("CONFIG")][_T("CountToTry")];
	pmi.m_delete_mapping	= m_ini_file[_T("CONFIG")][_T("DeleteMapping")];

	if (int cntUsersParameters=m_ini_file[_T("USERS")].countParams())
	{
		m_currentInfoItemIndex		= (m_currentInfoItemIndex + 1) % cntUsersParameters;
		
		const TCHAR* sectionName	= m_ini_file[_T("USERS")][m_currentInfoItemIndex];
		pmi.m_user_id				= m_ini_file[sectionName][_T("Name")];
		pmi.m_user_password			= m_ini_file[sectionName][_T("Password")];
		pmi.m_user_external_port	= (unsigned short)(int)m_ini_file[sectionName][_T("ExternalPort")];
		pmi.m_user_internal_port	= (unsigned short)(int)m_ini_file[sectionName][_T("InternalPort")];
		pmi.m_protocol				= m_ini_file[sectionName][_T("Protocol")];
	}
	else
	{
		TCHAR strIncrement[10];
		_itot_s(m_increment,strIncrement,10,10);
		
		pmi.m_user_id				= m_ini_file[_T("AUTOGENUSER")][_T("Name")];
		pmi.m_user_password			= m_ini_file[_T("AUTOGENUSER")][_T("Password")];
		pmi.m_user_external_port	= (unsigned short)(int)m_ini_file[_T("AUTOGENUSER")][_T("ExternalPort")];
		pmi.m_user_internal_port	= (unsigned short)(int)m_ini_file[_T("AUTOGENUSER")][_T("InternalPort")];
		pmi.m_protocol				= m_ini_file[_T("AUTOGENUSER")][_T("Protocol")];

		pmi.m_user_id				+= strIncrement;
		pmi.m_user_external_port	+= m_increment;
		pmi.m_user_internal_port	+= m_increment;

		m_increment++;
	}
CATCH_LOG()
	LeaveCriticalSection(&cs);

	return pmi;
}
//----------------------------------------------------------------------------------------

void CPortMappingHelper::ThreadFailedNotify()
{
	InterlockedDecrement(&m_count_active_thread);
	InterlockedIncrement(&m_count_fail_thread);
}
//----------------------------------------------------------------------------------------

void CPortMappingHelper::ThreadSuccessedNotify()
{
	InterlockedDecrement(&m_count_active_thread);
	InterlockedIncrement(&m_count_success_thread);
}
// CPortMappingHelper [END] //////////////////////////////////////////////////////////////
