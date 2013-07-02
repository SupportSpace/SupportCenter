///////////////////////////////////////////////////////////
//  CRFmethodRequest.cpp
//  Implementation of the Class CRFmethodRequest
//  Created on:      11-Dec-2006 3:43:00 PM
///////////////////////////////////////////////////////////
#include "../stdafx.h"
#include "CRFmethodRequest.h"


namespace transfer_utils
{
	tstring get_file_name( const tstring&);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CListDrivesMR::CListDrivesMR(CFileAccessClient* servant, TCallback callback)
:IMethodRequest( servant ),m_futureResult( callback )
{
}

CListDrivesMR::~CListDrivesMR(void)
{
}

void CListDrivesMR::callMethod()
{
	TDriveInfo di;
	m_par.ret_code = m_pServant->ListDrives( di );
	m_futureResult( di , m_par );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRetrieveFileMR::CRetrieveFileMR( CFileAccessClient* s, TCallback callback, const s_param&  par)
:IMethodRequest(s),m_futureResult( callback ),m_par(par)
{
}



CRetrieveFileMR::~CRetrieveFileMR()
{

}





void CRetrieveFileMR::callMethod()
{
	//m_futureResult(m_pServant->RetriveFile( m_par.remoteName ,  m_par.localName , m_par.permission ));
	m_par.ret_code = m_pServant->RetriveFile( m_par.remoteName ,  m_par.localName , m_par.permission );
	m_futureResult( m_par );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSendFileMR::CSendFileMR( CFileAccessClient* s, TCallback callback, const s_param&  par)
:IMethodRequest(s),m_futureResult( callback ),m_par(par)
{
}



CSendFileMR::~CSendFileMR()
{

}





void CSendFileMR::callMethod()
{
	//m_futureResult(m_pServant->SendFile( m_par.remoteName ,  m_par.localName , m_par.permission ));
	m_par.ret_code = m_pServant->SendFile( m_par.remoteName ,  m_par.localName , m_par.permission );
	/// RCE-141
	m_par.remoteName = transfer_utils::get_file_name( m_par.localName );
	m_futureResult( m_par );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCreateDirectoryMR::CCreateDirectoryMR(CFileAccessClient* servant, TCallback callback,const s_param& par):IMethodRequest( servant )
,m_futureResult( callback ),m_par( par )
{
}



CCreateDirectoryMR::~CCreateDirectoryMR()
{
}


void CCreateDirectoryMR::callMethod()
{
	m_par.ret_code =  m_pServant->CreateDirectory(m_par.remoteName);
	m_futureResult( m_par );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRemoveDirectoryMR::CRemoveDirectoryMR(CFileAccessClient* servant, TCallback callback,const s_param& par):IMethodRequest( servant )
,m_futureResult( callback ),m_par( par )
{
}



CRemoveDirectoryMR::~CRemoveDirectoryMR()
{
}


void CRemoveDirectoryMR::callMethod()
{
	//m_futureResult( m_pServant->DeleteDirectory(m_par.remoteName) );
	m_par.ret_code = m_pServant->DeleteDirectory(m_par.remoteName);
	m_futureResult( m_par );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CRenameMR::CRenameMR(CFileAccessClient* servant, TCallback callback,const s_param& par):IMethodRequest( servant )
,m_futureResult( callback ),m_par( par )
{
}



CRenameMR::~CRenameMR()
{
}


void CRenameMR::callMethod()
{
	//m_futureResult( m_pServant->RemaneFile(m_par.localName , m_par.remoteName) );
	m_par.ret_code = m_pServant->RemaneFile(m_par.localName , m_par.remoteName);
	m_futureResult( m_par );
}