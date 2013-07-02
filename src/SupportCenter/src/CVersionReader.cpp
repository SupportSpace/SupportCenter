/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CVersionReader.cpp
///
///  class for retriving msi components versions
///
///  @author "Archer Software" Sogin M. @date 16.05.2007
///
////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <AidLib/Strings/tstring.h>
#include "CVersionReader.h"

CVersionReader::CVersionReader(	const tstring& productKey, const ESearchKeyType keyType)
{

	/// Filling install states
	m_msiStatesMap[INSTALLSTATE_NOTUSED]	= _T("The component being requested is disabled on the computer");
	m_msiStatesMap[INSTALLSTATE_ABSENT]		= _T("The component is not installed");
	m_msiStatesMap[INSTALLSTATE_INVALIDARG] = _T("One of the function parameters is invalid");
	m_msiStatesMap[INSTALLSTATE_LOCAL]		= _T("The component is installed locally");
	m_msiStatesMap[INSTALLSTATE_SOURCE]		= _T("The component is installed to run from source");
	m_msiStatesMap[INSTALLSTATE_SOURCEABSENT] = _T("The component source is inaccessible");
	m_msiStatesMap[INSTALLSTATE_UNKNOWN]	= _T("The product code or component ID is unknown");
	m_msiStatesMap[INSTALLSTATE_MOREDATA]	= _T("Return buffer overflow");

	/// Setting up product code
	switch(keyType)
	{
		case SK_COMPONENT_GUID:
			m_productCode = GetProductCodeByComponentGUID(productKey);
			break;
		case SK_UPGRAGE_CODE:
			m_productCode = GetProductCodeByUpgradeCode(productKey);
			break;
		case SK_PRODUCT_CODE:
			m_productCode = productKey;
			break;
		default:
			throw tstring(_T("Unknown key type"));
	}
}

CVersionReader::~CVersionReader()
{
}

tstring CVersionReader::InstallState2String(INSTALLSTATE state) const
{
	std::map<INSTALLSTATE,tstring>::const_iterator stateString = m_msiStatesMap.find(state);
	if (stateString != m_msiStatesMap.end())
		return stateString->second;
	return _T("Unknown install state");
}

tstring CVersionReader::GetProductCodeByUpgradeCode(const tstring& upgradeCode)
{
	//Searching for all products to find one with our UpgradeCode
	TCHAR code[MSI_GUID_LEN+1];
	tstring productCode;
	UINT res = MsiEnumRelatedProducts(upgradeCode.c_str(), 0 /*reserved*/, 0 /*first product*/, code);
	if (res != ERROR_SUCCESS) 
	{
		SetLastError(res);
		throw Format(_T("Failed to MsiEnumRelatedProducts. ErroCode: %d"), res ); //TODO: add winerror description
	}
	productCode = code;
	res = MsiEnumRelatedProducts(upgradeCode.c_str(), 0 /*reserved*/, 1 /*next product*/, code);
	if (res == ERROR_SUCCESS) 
	{
		SetLastError(res);
		throw Format(_T("There's more than one product installed. ErroCode: %d"), res ); //TODO: add winerror description
	}
	return productCode;
}

tstring CVersionReader::GetProductCodeByComponentGUID(const tstring& componentGUID)
{
	TCHAR productCode[MSI_GUID_LEN+1];
	UINT res = MsiGetProductCode(componentGUID.c_str(),productCode);
	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		throw tstring(_T("Failed to MsiGetProductCode")); //TODO: add winerror description
	}
	return productCode;
}

tstring CVersionReader::GetComponentVersion(const tstring& componentCode) const
{
	// Retriving component path
	DWORD size = 0;
	INSTALLSTATE state = MsiGetComponentPath(m_productCode.c_str(), componentCode.c_str(), NULL, &size);
	if (state != INSTALLSTATE_MOREDATA && state != INSTALLSTATE_LOCAL)
		throw InstallState2String(state);
	std::auto_ptr<TCHAR> buf;
	buf.reset(new TCHAR[++size]);
	state = MsiGetComponentPath(m_productCode.c_str(), componentCode.c_str(), buf.get(), &size);
	if (state != INSTALLSTATE_LOCAL)
		throw InstallState2String(state);
	// Retriving component version
	TCHAR version[MAX_PATH];
	size = MAX_PATH;
	UINT res = MsiGetFileVersion(buf.get(), version, &size, NULL, NULL);
	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		throw tstring(_T("Failed to MsiGetFileVersion")); //TODO: add winerror description
	}
	return version;
}

tstring CVersionReader::GetProductVersion() const
{
	DWORD  cchProductVersion = MAX_PATH;
	TCHAR  lpProductVersion[MAX_PATH];
	UINT   uiStatus = ERROR_SUCCESS;

	// obtain Product version derived from ProductVersion Property.
	UINT res = MsiGetProductInfo(
		m_productCode.c_str(),
		INSTALLPROPERTY_VERSIONSTRING,
		lpProductVersion,
		&cchProductVersion);

	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		throw Format(_T("Failed to MsiGetFileVersion. ErroCode: %d"), res ); //TODO: add winerror description
	}
	return lpProductVersion;
}