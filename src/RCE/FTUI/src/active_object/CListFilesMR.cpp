#include "../stdafx.h"
#include "CListFilesMR.h"

CListFilesMR::CListFilesMR(CFileAccessClient* servant, TCallback callback,const s_param& par)
:IMethodRequest( servant ),m_futureResult( callback ),m_param( par )
{
}

CListFilesMR::~CListFilesMR(void)
{
}

void CListFilesMR::callMethod()
{
	TFileInfo fi;
	m_pServant->ListFiles( m_param.remoteName,fi );
	m_futureResult( fi );
}
