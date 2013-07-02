#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CHelperService.h
///
///  SupportSpace tools helper service
///
///  @author "Archer Software" Sogin M. @date 01.10.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/CService/CService.h>
#include "CProxyStrapper.h"

#define HELPER_SERVICE_NAME				_T("SupportSpaceHelperService")
#define HELPER_SERVICE_DISPLAY_NAME		_T("SupportSpace platform helper service")

/// SupportSpace tools helper service
SERVICE_CLASS(CHelperService)

	private:
		bool m_stopped;

		/// Proxy strapper subservice
		CProxyStrapper m_proxyStrapper;

		/// private ctor
		CHelperService() : m_stopped(false) {};

		/// Service entry point
		virtual void OnStart();

		/// Called when service manager try to stop service
		virtual void OnStop();

END_OF_SERVICE_CLASS
