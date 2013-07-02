///////////////////////////////////////////////////////////
//  CFAProxy.cpp
//  Implementation of the Class CFAProxy
//  Created on:      11-Dec-2006 3:40:58 PM
///////////////////////////////////////////////////////////
#include "../stdafx.h"
#include "CFAProxy.h"
#include <FTUI/FileTransfer/CFileAccessClient.h>
//#include "CCDMethodRequest.h"
#include "../FAManager.h"
//#include "LDMethodRequest.h"
//#include "CListFilesMR.h"
#include "CRFmethodRequest.h"
#include "../FileAccessClientImpl.h"

CFAProxy::CFAProxy(/*CAbstractStream* c*/CFileAccessClient* client , SPHandle h ):
m_hEvent(h),m_scheduler(m_hEvent),m_pFAClient( client )
{
	 /*= boost::shared_ptr<CFileAccessClient>( new CFileAccessClientImpl( c ) ) ;*/
}



CFAProxy::~CFAProxy(){

}



//CCDMethodRequest(CFileAccessClient*,TCallback,const tstring& par);
/*

list_drives,
list_files,
send_file,
retrieve_file,
delete_file,
delete_directory,
,


*/
void CFAProxy::CreateDirectory(const tstring& fn)
{
	CCreateDirectoryMR::TCallback f = boost::bind( &CFAManager::cmCreateDirectory , m_pManager , _1 );
	s_param par;
	par.remoteName = fn;
	par.operation = create_directory;
	CScheduler::TMethod mr( CScheduler::TMethod( new CCreateDirectoryMR( m_pFAClient ,f ,par ) ) );
	m_scheduler.AddMethod( mr );
}


void CFAProxy::DeleteDirectory(const tstring& fn)
{
	CRemoveDirectoryMR::TCallback f = boost::bind( &CFAManager::cmRetreiveFile , m_pManager , _1 );
	s_param par;
	par.remoteName = fn;
	par.operation = delete_directory;
	CScheduler::TMethod mr( CScheduler::TMethod( new CRemoveDirectoryMR( m_pFAClient ,f ,par ) ) );
	m_scheduler.AddMethod( mr );
}


void CFAProxy::DeleteFile(const tstring& fn)
{
}


void CFAProxy::ListDrives()
{
	CListDrivesMR::TCallback f = boost::bind( &CFAManager::cmListDrives , m_pManager , _1 ,_2 );
	CScheduler::TMethod mr( CScheduler::TMethod( new CListDrivesMR( m_pFAClient ,f ) ) );
	m_scheduler.AddMethod( mr );
}


void CFAProxy::ListFiles(const tstring& path)
{
	s_param par;
	par.remoteName = path;
	par.operation = list_files;
	CListFilesMR::TCallback f = boost::bind( &CFAManager::cmListFiles , m_pManager , _1 );
	CScheduler::TMethod mr( CScheduler::TMethod( new CListFilesMR( m_pFAClient ,f , par ) ) );
	m_scheduler.AddMethod( mr );	
}


void CFAProxy::RenameFile(const tstring& oldName, const tstring& newName)
{
	s_param par;
	par.localName = oldName;
	par.remoteName =newName;
	par.operation = rename_file;
	CRenameMR::TCallback f = boost::bind( &CFAManager::cmRetreiveFile , m_pManager , _1 );
	CScheduler::TMethod mr( CScheduler::TMethod( new CRenameMR( m_pFAClient ,f , par ) ) );
	m_scheduler.AddMethod( mr );	
}


void CFAProxy::RetrieveFile(const tstring& remoteFile, const tstring& localFile, bool permission)
{
	s_param par;
	par.localName = localFile;
	par.remoteName = remoteFile;
	par.permission = permission;
	par.operation = retrieve_file;
	CRetrieveFileMR::TCallback f = boost::bind( &CFAManager::cmRetreiveFile , m_pManager , _1 );
	CScheduler::TMethod mr( CScheduler::TMethod( new CRetrieveFileMR( m_pFAClient ,f , par ) ) );
	m_scheduler.AddMethod( mr );	
}


void CFAProxy::SendFile(const tstring& remoteFile, const tstring& localFile, bool permission)
{
	//CSendFileMR
	s_param par;
	par.localName = localFile;
	par.remoteName = remoteFile;
	par.permission = permission;
	par.operation = send_file;
	CSendFileMR::TCallback f = boost::bind( &CFAManager::cmRetreiveFile , m_pManager , _1 );
	CScheduler::TMethod mr( CScheduler::TMethod( new CSendFileMR( m_pFAClient ,f , par ) ) );
	m_scheduler.AddMethod( mr );	
}


