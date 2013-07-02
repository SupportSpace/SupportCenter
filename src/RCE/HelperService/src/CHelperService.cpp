/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CHelperService.cpp
///
///  SupportSpace tools helper service
///
///  @author "Archer Software" Sogin M. @date 01.10.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CHelperService.h"

void CHelperService::OnStart()
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Service started"));
	m_proxyStrapper.Run(true /*sync*/);
CATCH_LOG()
}

void CHelperService::OnStop() 
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Service stopped"));
	m_stopped = true;
	m_proxyStrapper.Stop();
CATCH_LOG()
}
