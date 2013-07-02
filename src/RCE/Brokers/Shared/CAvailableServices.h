/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAvailableServices.h
///
///  Available services types and enumarations declaration and its parameters
///
///  @author Kirill Solovyov @date 10.11.2007
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <AidLib/Strings/tstring.h>
#include <map>

/// Available services types
enum EBrokerServicesTypes
{
	BST_VBROKER=0,		///Visual broker service (expert's side)
	BST_BROKER,			///Broker service (customer's side)
	BST_INSTALLER,		///Installer service
	BST_RCVIEWER,		///Remote control viewer (client)
	BST_RCHOST,			///Remote control host (server)
	BST_FAVIEWER,		///File access viewer (client)
	BST_FAHOST,			///File access host (server)
	BST_SECLIENT,		///Script engine client
	BST_SEHOST,			///Script engine host

};

/// Service hardcoded parameters
struct SBrokerServiceParams
{
	EBrokerServicesTypes m_type;  /// service type
	tstring m_GUID;               /// service COM object GUID string
	tstring m_feature;            /// owner feature name (installation infrastructure)
	tstring m_displayName;        /// display name of service
	
	SBrokerServiceParams(){}
	SBrokerServiceParams(EBrokerServicesTypes type, const tstring& GUID, const tstring& feature, const tstring& displayName)
	{
		m_type=type;
		m_GUID=GUID;
		m_feature=feature;
		m_displayName=displayName;
	}

	SBrokerServiceParams(const SBrokerServiceParams& params)
	{
		m_type=params.m_type;
		m_GUID=params.m_GUID;
		m_feature=params.m_feature;
		m_displayName=params.m_displayName;
	}

	SBrokerServiceParams& operator=(const SBrokerServiceParams& params)
	{
		m_type=params.m_type;
		m_GUID=params.m_GUID;
		m_feature=params.m_feature;
		m_displayName=params.m_displayName;
		return *this;
	}
};

/// Singleton Available services class
class CAvailableServices:
	public std::map<EBrokerServicesTypes,SBrokerServiceParams>
{
public:
	CAvailableServices()
	{
		//operator[]()=SBrokerServiceParams(,_T(""),_T(""));
		operator[](BST_VBROKER)=SBrokerServiceParams(BST_VBROKER,tstring(_T("ED2B90B2-9124-4D63-8FEB-E9203FCA1BCE")),tstring(_T("VBroker")),tstring(_T("VBroker")));
		operator[](BST_BROKER)=SBrokerServiceParams(BST_BROKER,_T("2FF5923D-5B0C-4EAB-8CF7-7CC79F1A627E"),_T("Broker"),_T("Initial Installer"));
		operator[](BST_INSTALLER)=SBrokerServiceParams(BST_INSTALLER,_T("7B3BBD75-A77C-40D9-BD0E-943055093249"),_T("RCInstaller"),_T("RCInstaller"));
		operator[](BST_RCVIEWER)=SBrokerServiceParams(BST_RCVIEWER,_T("DA4679AA-3239-43EF-8D59-6E82AEF6F081"),_T("RemoteControl"),_T("RemoteControl"));
		operator[](BST_RCHOST)=SBrokerServiceParams(BST_RCHOST,_T("9DC84E6D-3B40-4AEF-BF51-F36E87B02F61"),_T("RemoteControl"),_T("RemoteControl"));
		operator[](BST_SEHOST)=SBrokerServiceParams(BST_SEHOST,_T("5DBCAD54-F260-4E79-8D1E-92405C0AB1C5"),_T("ScriptEngine"),_T("Diagnostics"));
		operator[](BST_SECLIENT)=SBrokerServiceParams(BST_SECLIENT,_T("D50A842C-B6C3-4912-8714-807F249B9519"),_T("ScriptEngineHost"),_T("ScriptEngineHost"));
		operator[](BST_FAVIEWER)=SBrokerServiceParams(BST_FAVIEWER,_T("A64B54A3-3F5B-493F-879F-41A71B09F098"),_T("FileAccess"),_T("FileAccess"));
		operator[](BST_FAHOST)=SBrokerServiceParams(BST_FAHOST,_T("BFC3A266-A3A0-4F25-901C-730CF6DC3554"),_T("FileAccess"),_T("FileAccess"));
	}
	//TODO think about const array
	//SBrokerServiceParams operator[](EBrokerServicesTypes key)const
	//{
	//	return find(key)->second;
	//}
};

// example of use 
// AVAILABLE_SERVICES[BST_VBROKER].m_GUID.c_str();
#define AVAILABLE_SERVICES CSingleton<CAvailableServices>::instance()


