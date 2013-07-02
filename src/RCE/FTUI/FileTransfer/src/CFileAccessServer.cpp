/////////////////////////////////////////////////////////////////////////
///
///  CFileAccessServer.cpp
///
///  CFileAccessServer class implementation
///
///  @author Dmiry S. Golub @date 12/21/2006
///
////////////////////////////////////////////////////////////////////////


#include "CFileAccessServer.h"
#include "CServerThread.h"
//#include "logger.h"
#include <AidLib/Logging/cLog.h>


///TODO delete this

char* TransferOpearationStr[]=
{
	"list_drives",		/** list drives opeartion */
	"list_files",			/** list files opeartion */
	"send_file",			/** send file opeartion */
	"retrieve_file",		/** retrieve file opeartion */
	"delete_file",		/** delete file opeartion */
	"delete_directory",	/** delete directory opeartion */
	"rename_file",		/** rename file opeartion */
	"create_directory"	/** create directory opeartion */
};


CFileAccessServer::~CFileAccessServer()
{

}


CFileAccessServer::CFileAccessServer(CAbstractStream* stream)
	:	m_pStream( stream ),
		m_transferLog(),
		m_activityChangedHandler(NULL)
{
	// Calculating difference between UTC and local time
	FILETIME utcFt, localFt;
	GetSystemTimeAsFileTime(&utcFt);
	FileTimeToLocalFileTime(&utcFt,&localFt);
	
	LARGE_INTEGER liUtc, liLocal;
	liUtc.HighPart		= utcFt.dwHighDateTime;
	liUtc.LowPart		= utcFt.dwLowDateTime;
	liLocal.HighPart	= localFt.dwHighDateTime;
	liLocal.LowPart		= localFt.dwLowDateTime;
		
	liLocal.QuadPart -= liUtc.QuadPart;
	// Trunc to minutes
	liLocal.QuadPart /= 600000000L;
	TCHAR prefix[20];
	_stprintf_s(prefix,
				sizeof(prefix)/sizeof(prefix[0]),
				_T(" GMT (%+.2d:00) "),				// Don't print a minute difference, it looks ugly
				liLocal.QuadPart/60);

	m_transferLog.SetMessagePrefix(prefix);
}

void CFileAccessServer::SetActivityChangedHandler(ActivityChangedHandler activityChangedHandler)
{
TRY_CATCH
	m_activityChangedHandler = activityChangedHandler;
CATCH_THROW()
}

ResponseCode CFileAccessServer::NotifyOperation(BYTE status, TransferOpearation operation)
{
	TRY_CATCH
	//Fixed RCE-107
	bool res = m_userPermisiions[/*CServerThread::powOf2( operation )*/ m_userPermisiions.size() - operation-1];
	//return   ((res)?accept_operation:reject_operation);  //powOf2( send_file );
	return   ( res ? operation_accepted : operation_rejected );		//FIX Error Codes
	CATCH_THROW("CFileAccessServer::NotifyOperation")
}


void CFileAccessServer::NotifyTransferStatus(Status status)
{
}


void CFileAccessServer::SetAuthorization(const TransferOpearation& operation, bool auth)
{
	TRY_CATCH
		//Fixed RCE-107
		m_userPermisiions.set( /*CServerThread::powOf2( operation )*/ (m_userPermisiions.size() - operation -1)  , auth );
	Log.Add( _MESSAGE_, "Operation %s permission %i\n" , TransferOpearationStr[operation] , auth );
	CATCH_THROW("CFileAccessServer::SetAutorization")
}


void CFileAccessServer::Start()
{
	TRY_CATCH
	m_spWorkerThread = boost::shared_ptr<CServerThread>( new CServerThread( m_pStream , this ));
	CATCH_THROW("CFileAccessServer::Start")
}


void CFileAccessServer::Stop()
{
	TRY_CATCH
    //m_pStream->CancelReceiveOperation();
	m_spWorkerThread.reset();
	CATCH_THROW("CFileAccessServer::Stop")
}

void CFileAccessServer::InitTransferLogging(const tstring& sid, const tstring& customerName, const tstring& expertName)
{
TRY_CATCH
	m_transferLog.Init(sid, customerName, expertName);
CATCH_LOG()
}

void CFileAccessServer::NotifyActivity(const tstring description)
{
TRY_CATCH
	if (NULL != m_activityChangedHandler)
		m_activityChangedHandler(description);
CATCH_LOG()
}