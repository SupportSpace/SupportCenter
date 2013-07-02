/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SAuthAppSettings.h
///
///  Declares SAuthAppSettings structure, settings of application which
///    authorized on firewall
///
///  @author Dmitry Netrebenko @date 27.12.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <NWL/NetworkLayer.h>
#include <atlbase.h>
#include <boost/shared_ptr.hpp>
#include <netfw.h>

/// SAuthAppSettings structure, settings of application which
///   authorized on firewall
struct NWL_API SAuthAppSettings
{
	CComBSTR			m_name;
	CComBSTR			m_image;
	VARIANT_BOOL		m_enabled;
	NET_FW_IP_VERSION	m_ipVersion;
	NET_FW_SCOPE		m_scope;
/// Default constructor
	SAuthAppSettings()
		:	m_name(_T(""))
		,	m_image(_T(""))
		,	m_enabled(VARIANT_FALSE)
		,	m_ipVersion(NET_FW_IP_VERSION_ANY)
		,	m_scope(NET_FW_SCOPE_ALL)
	{};
	SAuthAppSettings(CComBSTR name, CComBSTR image, VARIANT_BOOL enabled, NET_FW_IP_VERSION ipVersion, NET_FW_SCOPE scope)
		:	m_name(name)
		,	m_image(image)
		,	m_enabled(enabled)
		,	m_ipVersion(ipVersion)
		,	m_scope(scope)
	{};
};

/// Shared pointer to SAuthAppSettings structure
typedef boost::shared_ptr<SAuthAppSettings> SPAuthAppSettings;
