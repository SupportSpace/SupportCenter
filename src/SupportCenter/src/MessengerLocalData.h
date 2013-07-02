// CMessengerLocalData.h : header file for class that contains local data per Messenger
//
//===========================================================================
// SupportMessengerApp ltd. @{SRCH}
//								CMessengerLocalData
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// SupportMessengerApp :	
//
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================

#include "LocalDataDef.h"

#include <string>
#include <vector>

typedef std::vector<tstring> CStrList;
typedef std::vector<tstring>::const_iterator StringListIter;
//
//
//
#define MAX_BUFFER_	4096

class CLastLogiedInEntry
{
public:
	CLastLogiedInEntry(){m_AppStateSize = 0;};
	~CLastLogiedInEntry(){};

	BOOL	Load();
	BOOL	Save();

	void	setSupporterId(const tstring& supporterId){m_supporterId = supporterId;};
	tstring	getSupporterId(){return m_supporterId;};

	void	setAppState(PBYTE pValue, ULONG nBytes)
	{
		memcpy(m_AppState, pValue, nBytes); 
		m_AppStateSize = nBytes;
	};
	void	getAppState(void* pValue, ULONG* pnBytes)
	{
		if(m_AppStateSize!=0)
			memcpy(pValue, m_AppState, m_AppStateSize); 

		*pnBytes = m_AppStateSize;
	};

private:
	tstring  m_supporterId;

	byte	 m_AppState[MAX_BUFFER_];   
	DWORD	 m_AppStateSize;
	//tstring  langId; //todo Sprint5 in the future when language support will be added
};
//
//
//
class CMessengerLocalData
{
public:
	CMessengerLocalData()
	{
		pcLastLogiedInEntry = new CLastLogiedInEntry();
	};
	~CMessengerLocalData()
	{
		if(pcLastLogiedInEntry)
			delete pcLastLogiedInEntry;
	};

	BOOL				LoadSupportersList();
	CStrList			getSupporterList(){ return m_supportersList;};

	CLastLogiedInEntry* getLastLogiedInEntry(){return pcLastLogiedInEntry;}

private:
	//CCommonPref		pcCommonPref;			 // common prefrencies like ServerIP
	CLastLogiedInEntry*	pcLastLogiedInEntry;	 // information about last user that loged in
	CStrList			m_supportersList;		 // list of supporters
};