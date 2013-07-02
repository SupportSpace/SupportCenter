#include <stdafx.h>
#include <atlbase.h>
#include "MessengerLocalData.h"

/**
	* SaveToRegistry local settings
	* @param 
*/
BOOL CLastLogiedInEntry::Save()
{
	HRESULT  hResult = S_OK;
	CRegKey  m_regKey;
	
	TCHAR	 szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s(szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"), szBaseRegistryPath, szLastEntryKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	hResult = m_regKey.SetValue(
		sLastLogedInSupporterId, REG_SZ, m_supporterId.c_str(), (ULONG)_tcslen(m_supporterId.c_str())*sizeof(TCHAR));
	
	hResult = m_regKey.SetBinaryValue(sLastAppState, m_AppState, m_AppStateSize);
	
	m_regKey.Close();
	return TRUE;	
}
/**
	* LoadFromRegistry local settings
	* @param 
*/
BOOL CLastLogiedInEntry::Load()
{
	CRegKey  m_regKey;
	ULONG	 nBytes = 0;	
	HRESULT	 lResult = S_OK;
	TCHAR	 szTemp[MAX_BIGBUF_LEN] = { 0 };

	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"), szBaseRegistryPath, szLastEntryKey );//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	nBytes = sizeof(szTemp);
	lResult = m_regKey.QueryStringValue(sLastLogedInSupporterId, szTemp, &nBytes);
	if(nBytes != 0 && lResult == S_OK)
	{
		m_supporterId = szTemp;
	}

	m_AppStateSize = MAX_BUFFER_;
	lResult = m_regKey.QueryBinaryValue(sLastAppState, m_AppState, &m_AppStateSize);
	if(lResult!=ERROR_SUCCESS)
		m_AppStateSize = 0;

	m_regKey.Close();
	return TRUE;
}

BOOL	CMessengerLocalData::LoadSupportersList()
{
	CRegKey  m_regKey;
	HRESULT	 lResult = S_OK;
	TCHAR	 szTemp[MAX_BIGBUF_LEN] = { 0 };

	TCHAR	szCurrentUserPath[MAX_REGBUF_LEN] = { 0 };
	_stprintf_s( szCurrentUserPath, MAX_REGBUF_LEN, _T("%s\\%s"), szBaseRegistryPath, szSupportersKey);//todo

	if( m_regKey.Create(HKEY_DEFINED_KEY, szCurrentUserPath)!=ERROR_SUCCESS )
		return FALSE;

	DWORD iIndex = 0;
	ULONG nBytes = MAX_BIGBUF_LEN;

	do{
		lResult = m_regKey.EnumKey(iIndex, szTemp, &nBytes);

		if(nBytes != 0 && lResult == S_OK)
		{
			m_supportersList.push_back(szTemp);
			nBytes = MAX_BIGBUF_LEN;
			iIndex++;
		}
		else
		{
			break;
		}

	}while(TRUE);

	m_regKey.Close();
	return TRUE;
}