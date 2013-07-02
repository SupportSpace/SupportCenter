/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogClientList.cpp
///
///  NetLogClientList COM object
///
///  @author Sogin Max @date 27.03.2007
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNetLogClientList.h"

// CNetLogClientList


STDMETHODIMP NetLogClientList::CNetLogClientList::AddClient(INetLogClient* client)
{
TRY_CATCH
	m_coll.push_back(client);
	return S_OK;
CATCH_LOG_COMERROR()
}
