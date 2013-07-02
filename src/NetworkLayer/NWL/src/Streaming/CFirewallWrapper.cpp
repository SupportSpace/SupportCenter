/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFirewallWrapper.cpp
///
///  Implements CFirewallWrapper class, wrapper for firewall
///
///  @author Dmitry Netrebenko @date 27.12.2007
///
////////////////////////////////////////////////////////////////////////

#include <NWL/Streaming/CFirewallWrapper.h>
#include <NWL/Streaming/CStreamException.h>

CFirewallWrapper::CFirewallWrapper()
{
TRY_CATCH
	/// Create manager
	CComPtr<INetFwMgr> manager;
	HRESULT result = CoCreateInstance(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwMgr), (void**)&manager);
	if(FAILED(result) || !manager)
		throw MCStreamException_Win(_T("Create instance of INetFwMgr failed"), result);
	/// Save manager
	m_manager.SetInterface(manager);
	/// Retrieve the local firewall policy.
	CComPtr<INetFwPolicy> policy;
	result = manager->get_LocalPolicy(&policy);
	if (FAILED(result) || !policy)
		throw MCStreamException_Win(_T("INetFwMgr->get_LocalPolicy failed"), result);
	/// Retrieve the firewall profile currently in effect.
	CComPtr<INetFwProfile> profile;
	result = policy->get_CurrentProfile(&profile);
	if (FAILED(result) || !profile)
		throw MCStreamException_Win(_T("INetFwPolicy->get_CurrentProfile failed"), result);
	/// Save profile
	m_profile.SetInterface(profile);
CATCH_THROW()
}

CFirewallWrapper::~CFirewallWrapper()
{
TRY_CATCH
CATCH_LOG()
}

tstring CFirewallWrapper::GetProcessName(const tstring& fileName)
{
TRY_CATCH
	tstring strResult(fileName);
	tstring strDirSeparator(_T("\\"));
	tstring strExtSeparator(_T("."));
	tstring::size_type index;
	index = strResult.rfind(strDirSeparator);
	if(tstring::npos != index)
		strResult = strResult.substr(index + 1);
	index = strResult.rfind(strExtSeparator);
	if(tstring::npos != index)
		strResult = strResult.substr(0, index);
	return strResult;
CATCH_THROW()
}

SPAuthAppSettings CFirewallWrapper::AuthorizeApplication(const tstring& fileName)
{
TRY_CATCH
	CComBSTR imageName(fileName.c_str());
	CComBSTR name(GetProcessName(fileName).c_str());
	SPAuthAppSettings newSettings(new SAuthAppSettings(name, imageName, VARIANT_TRUE, NET_FW_IP_VERSION_ANY, NET_FW_SCOPE_ALL));
	return SetApplicationSettings(fileName, newSettings);
CATCH_THROW()
}

void CFirewallWrapper::RemoveApplication(const tstring& fileName)
{
TRY_CATCH
	SetApplicationSettings(fileName, SPAuthAppSettings());
CATCH_THROW()
}

bool CFirewallWrapper::EnableExceptions(const bool enable)
{
TRY_CATCH
	CComPtr<INetFwProfile> profile = m_profile.GetInterface();
	if(!profile)
		throw MCStreamException(_T("Firewall profile is not obtained."));
	HRESULT result = S_OK;
	/// Retrieve the authorized application collection.
	VARIANT_BOOL notAllowed;
	result = profile->get_ExceptionsNotAllowed(&notAllowed);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwProfile->get_ExceptionsNotAllowed failed"), result);
	result = profile->put_ExceptionsNotAllowed(enable?VARIANT_FALSE:VARIANT_TRUE);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwProfile->put_ExceptionsNotAllowed failed"), result);
	return (VARIANT_FALSE == notAllowed);
CATCH_THROW()
}

SPAuthAppSettings CFirewallWrapper::GetApplicationSettings(CComPtr<INetFwAuthorizedApplication> app)
{
TRY_CATCH
	HRESULT result = S_OK;
	if(!app)
		throw MCStreamException(_T("Pointer to interface is NULL."));
	SPAuthAppSettings settings(new SAuthAppSettings());
	/// Get the process image file name.
	result = app->get_ProcessImageFileName(&settings->m_image);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->get_ProcessImageFileName failed"), result);
	/// Get the application friendly name.
	result = app->get_Name(&settings->m_name);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->get_Name failed"), result);
	/// Get ip version
	result = app->get_IpVersion(&settings->m_ipVersion);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->get_IpVersion failed"), result);
	/// Get enabled
	result = app->get_Enabled(&settings->m_enabled);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->get_Enabled failed"), result);
	/// Get scope 
	result = app->get_Scope(&settings->m_scope);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->get_Scope failed"), result);
	return settings;
CATCH_THROW()
}

void CFirewallWrapper::SetApplicationSettings(CComPtr<INetFwAuthorizedApplication> app, SPAuthAppSettings settings)
{
TRY_CATCH
	HRESULT result = S_OK;
	if(!app)
		throw MCStreamException(_T("Pointer to interface is NULL."));
	if(!settings.get())
		throw MCStreamException(_T("Pointer to settings is NULL."));
	/// Set the process image file name.
	result = app->put_ProcessImageFileName(settings->m_image);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->put_ProcessImageFileName failed"), result);
	/// Set the application friendly name.
	result = app->put_Name(settings->m_name);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->put_Name failed"), result);
	/// Set ip version
	result = app->put_IpVersion(settings->m_ipVersion);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->put_IpVersion failed"), result);
	/// Set enabled
	result = app->put_Enabled(settings->m_enabled);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->put_Enabled failed"), result);
	/// Set scope 
	result = app->put_Scope(settings->m_scope);
	if(FAILED(result))
		throw MCStreamException_Win(_T("INetFwAuthorizedApplication->put_Scope failed"), result);
CATCH_THROW()
}

SPAuthAppSettings CFirewallWrapper::SetApplicationSettings(const tstring& fileName, SPAuthAppSettings settings)
{
TRY_CATCH
	CComPtr<INetFwProfile> profile = m_profile.GetInterface();
	if(!profile)
		throw MCStreamException(_T("Firewall profile is not obtained."));
	HRESULT result = S_OK;
	
	CComPtr<INetFwAuthorizedApplication> authApp;
	CComPtr<INetFwAuthorizedApplications> authApps;
	CComBSTR imageName(fileName.c_str());

	/// Retrieve the authorized application collection.
	result = profile->get_AuthorizedApplications(&authApps);
	if(FAILED(result) || !authApps)
		throw MCStreamException_Win(_T("INetFwProfile->get_AuthorizedApplications failed"), result);

	/// Attempt to retrieve the authorized application.
	result = authApps->Item(imageName, &authApp);
	SPAuthAppSettings prevSettings;
	if(SUCCEEDED(result) && authApp)
	{
		/// Application found
		/// Get previous settings
		prevSettings = GetApplicationSettings(authApp);
		if(settings.get())
			SetApplicationSettings(authApp, settings);
		else
		{
			result = authApps->Remove(imageName);
			if(FAILED(result))
				throw MCStreamException_Win(_T("INetFwAuthorizedApplications->Remove failed"), result);
		}
	}
	else
	{
		/// Application not found
		// Create an instance of an authorized application.
		if(settings.get())
		{
			result = CoCreateInstance(__uuidof(NetFwAuthorizedApplication), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwAuthorizedApplication), (void**)&authApp);
			if(FAILED(result) || !authApp)
				throw MCStreamException_Win(_T("Create instance of INetFwAuthorizedApplication failed"), result);
			SetApplicationSettings(authApp, settings);
			/// Add the application to the collection.
			result = authApps->Add(authApp);
			if(FAILED(result))
				throw MCStreamException_Win(_T("INetFwAuthorizedApplications->Add failed"), result);
		}
	}
	return prevSettings;
CATCH_THROW()
}

