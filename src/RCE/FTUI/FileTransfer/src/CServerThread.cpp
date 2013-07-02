/////////////////////////////////////////////////////////////////////////
///
///  CServerThread.cpp
///
///  Implementation of the thread which is started by CFileAccessServer and which manages all
///  events from  CFileAccessClient
///
///  @author Dmiry S. Golub @date 12/21/2006
///
////////////////////////////////////////////////////////////////////////

#include "CServerThread.h"
#include "filetransferdata.h"
#include <process.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <AidLib/CThread/CThreadLS.h>
#include "cserverthread.h"
#include <boost/scoped_ptr.hpp>
#include "utils.h"

//#include "logger.h"
#include <AidLib/Logging/cLog.h>

#pragma warning( disable: 4996 )//<func> was declared deprecated

#define __DEBUG_REPORT__


#define CALL_HANDLER( tp , msg ) if( tp->this_->CheckPermissions( msg ) )\
	{ \
		THandlers::iterator it = tp->this_->m_handlerMap.find( msg.contentType );\
		if( it!= tp->this_->m_handlerMap.end())\
		{\
		(tp->this_->*((*it).second))( msg );\
		}\
	}



struct ThreadParKeeper
{
	CAbstractStream* stream;
	CFileAccessServer* server;
	CServerThread*   this_; 
};



CServerThread::~CServerThread()
{
TRY_CATCH
	m_pStream->CancelReceiveOperation();
	{
		CCritSection lock( &m_cs );
		m_fStopThread = true;
	}
	DWORD ret = WaitForSingleObject( m_thHandle.get() , 1000 );

	switch( ret )
	{
		case WAIT_TIMEOUT:
			TerminateThread( m_thHandle.get() , 0 );
			Log.Add(_MESSAGE_ , "\nThread doesn't finish during 1 second and was killed!!!\n");
			break;
		case WAIT_OBJECT_0:
			Log.Add(_MESSAGE_ , "\nThread finished work.\n");
			break;

	}

	DeleteCriticalSection( &m_cs );  
CATCH_LOG()
}


TransferOpearation GetOperation( const rfbFileTransferMsg& msg )
{
TRY_CATCH
	switch( msg.contentType )
	{
		case rfbDirContentRequest:
			if( rfbRDrivesList == msg.contentParam  )
				return list_drives;
			else 
				return list_files;
		case rfbFileTransferOffer:
		case rfbFilePacket:
		case rfbEndOfFile:     
				return retrieve_file;

		case rfbFileTransferRequest:
		case rfbFileHeader:
				return send_file;
		case rfbCommand:
			{
				switch( msg.contentParam  )
				{
				case rfbCDirDelete:
					return delete_directory;
				case rfbCFileDelete:
					return delete_file;
				case rfbCFileRename:
					return rename_file;
				case rfbCDirCreate:
					return create_directory;
				};
			};
	}
	return list_drives;
	CATCH_THROW("TransferOpearation GetOperation( const rfbFileTransferMsg& msg )")
}

CServerThread::CServerThread(CAbstractStream* stream, CFileAccessServer* server):m_pStream( stream ),m_pServer(server),m_fStopThread(false),m_zippedFile(false)
{
	TRY_CATCH
	InitializeCriticalSection( &m_cs );
	ThreadParKeeper* tp =  new ThreadParKeeper;
	tp->stream = m_pStream;
	tp->server = m_pServer; 
	tp->this_ = this;

	m_handlerMap.insert( std::make_pair( rfbDirContentRequest , &CServerThread::DirContentRequst  ) );
	m_handlerMap.insert( std::make_pair( rfbFileTransferOffer , &CServerThread::FileTransferOffer  ) );
	m_handlerMap.insert( std::make_pair( rfbFileTransferRequest , &CServerThread::FileTransferRequest  ) );
	m_handlerMap.insert( std::make_pair( rfbFileHeader , &CServerThread::FileHeader  ) );
	m_handlerMap.insert( std::make_pair( rfbFilePacket , &CServerThread::FilePacket ) );
	m_handlerMap.insert( std::make_pair( rfbEndOfFile , &CServerThread::EndOfFile ) );
	m_handlerMap.insert( std::make_pair( rfbCommand , &CServerThread::ClientCommandHanler ));
	m_handlerMap.insert( std::make_pair( rfbAbortFileTransfer , &CServerThread::AbortTranserOperation ) );
	m_thHandle = SPHandle((HANDLE)(_beginthreadex( 0 , 0 , &CServerThread::ThreadEntry, tp , 0 , 0)) ,
									CloseHandle );
CATCH_THROW("CServerThread::CServerThread(CAbstractStream* stream, CFileAccessServer* server)")
}


void CServerThread::DirContentRequst(const rfbFileTransferMsg& msg)
{
TRY_CATCH
	switch (msg.contentParam)
	{
		// Client requests the List of Local Drives
	case rfbRDrivesList:
		{
			Log.Add(_MESSAGE_,_T("CServerThread::DirContentRequst rfbRDrivesList"));
			m_pServer->NotifyActivity(_T("Expert is currently retrieving drive names from your PC"));
			Log.Add(_MESSAGE_,_T("CServerThread::DirContentRequst notification sent"));
			transfer_utils::spPair drives = transfer_utils::get_drives();
			rfbFileTransferMsg ft;
			ft.type = rfbFileTransfer;
			ft.contentType = rfbDirPacket;
			ft.contentParam = rfbADrivesList;
			ft.length = Swap32IfLE((int)drives.first);
			m_pStream->Send( (char*)&ft , sizeof( rfbFileTransferMsg ) );
			m_pStream->Send( (char *)drives.second.get(), (int)drives.first );
			m_pServer->NotifyActivity(_T(""));
			Log.Add(_MESSAGE_,_T("CServerThread::DirContentRequst rfbRDrivesList completed"));
		}
		break;

		// Client requests the content of a directory
	case rfbRDirContent:
		{
			//	//boost::recursive_mutex::scoped_lock l(m_client->GetUpdateLock());
			const UINT length = Swap32IfLE(msg.length);
			char szDir[MAX_PATH];
			if (length > sizeof(szDir)) break;

			// Read in the Name of Dir to explore
			m_pStream->Receive(szDir, length);
			szDir[length] = 0;

			m_pServer->NotifyActivity(Format(_T("Expert is currently retrieving content of %s directory from your PC"),szDir));
			// sf@2004 - Shortcuts Case
			// Todo: Cultures translation ?
			int nFolder = -1;
			//char szP[MAX_PATH + 2];
			bool fShortError = false;
			//if (!strnicmp(szDir, "My Documents", 11))
			//	nFolder = CSIDL_PERSONAL;
			//if (!strnicmp(szDir, "Desktop", 7))
			//	nFolder = CSIDL_DESKTOP;
			//if (!strnicmp(szDir, "Network Favorites", 17))
			//	nFolder = CSIDL_NETHOOD;

			//	if (nFolder != -1)
			//		if (SHGetSpecialFolderPath(NULL, szP, nFolder, FALSE))
			//		if (m_client->GetSpecialFolderPath(nFolder, szP))
			//		{
			//			if (szP[strlen(szP)-1] != '\\') strcat(szP,"\\");
			//			strcpy(szDir, szP);
			//		}
			//		else
			//			fShortError = true;

			strcat(szDir, "*.*");

			WIN32_FIND_DATA fd;
			HANDLE ff;
			BOOL fRet = TRUE;

			rfbFileTransferMsg ft;
			ft.type = rfbFileTransfer;
			ft.contentType = rfbDirPacket;
			ft.contentParam = rfbADirectory; // or rfbAFile...

			SetErrorMode(SEM_FAILCRITICALERRORS); // No popup please !
			ff = FindFirstFile(szDir, &fd);
			SetErrorMode( 0 );

			// Case of media not accessible
			if (ff == INVALID_HANDLE_VALUE || fShortError)
			{
				ft.length = Swap32IfLE(0);										
				ft.contentParam = 0;
				m_pStream->Send( (char *)&ft , sizeof( ft ) );
				return;
			}

			ft.length = Swap32IfLE(strlen(szDir)-1);										
			//	m_socket->SendExact((char *)&ft, sz_rfbFileTransferMsg, rfbFileTransfer);
			// sf@2004 - Also send back the full directory path to the viewer (necessary for Shorcuts)
			//	m_socket->SendExact((char *)szDir, strlen(szDir)-1);


			while ( fRet )
			{
				// sf@2003 - Convert file time to local time
				// We've made the choice off displaying all the files 
				// off client AND server sides converted in clients local
				// time only. So we don't convert server's files times.
				/* 
				FILETIME LocalFileTime;
				FileTimeToLocalFileTime(&fd.ftLastWriteTime, &LocalFileTime);
				fd.ftLastWriteTime.dwLowDateTime = LocalFileTime.dwLowDateTime;
				fd.ftLastWriteTime.dwHighDateTime = LocalFileTime.dwHighDateTime;
				*/

				if (((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY && strcmp(fd.cFileName, "."))
					||
					(!strcmp(fd.cFileName, "..")))
				{
					// Serialize the interesting part of WIN32_FIND_DATA
					char szFileSpec[sizeof(WIN32_FIND_DATA)];
					int nOptLen = sizeof(WIN32_FIND_DATA) - MAX_PATH - 14 + lstrlen(fd.cFileName);
					memcpy(szFileSpec, &fd, nOptLen);

					ft.length = Swap32IfLE(nOptLen);
					m_pStream->Send((char *)&ft,  sizeof( ft ) /*sz_rfbFileTransferMsg, rfbFileTransfer*/);
					m_pStream->Send( szFileSpec , nOptLen );
				}
				else if (strcmp(fd.cFileName, "."))
				{
					// Serialize the interesting part of WIN32_FIND_DATA
					// Get rid of the trailing blanck chars. It makes a BIG
					// difference when there's a lot of files in the dir.
					char szFileSpec[sizeof(WIN32_FIND_DATA)];
					int nOptLen = sizeof(WIN32_FIND_DATA) - MAX_PATH - 14 + lstrlen(fd.cFileName);
					memcpy(szFileSpec, &fd, nOptLen);

					ft.length = Swap32IfLE(nOptLen);
					m_pStream->Send((char *)&ft,  sizeof( ft ));
					m_pStream->Send( szFileSpec , nOptLen );
				}
				fRet = FindNextFile(ff, &fd);
			}
			FindClose(ff);

			// End of the transfer
			ft.contentParam = 0;
			ft.length = Swap32IfLE(0);
			m_pStream->Send( (char *)&ft , sizeof( ft ) );

			m_pServer->NotifyActivity(_T(""));
		}
	}
CATCH_THROW("CServerThread::DirContentRequst")
}


void CServerThread::EndOfFile(const rfbFileTransferMsg&)
{
TRY_CATCH
	
//	tstring logFileName(m_openedFileName);

	m_hFile.reset();

	if ( m_zippedFile )
	{
		//transfer_utils::UnzipAndDeleteFile( m_openedFileName );
		tstring path( m_openedFileName );
		size_t pos = path.find_last_of("\\");
		if( path[pos-1]!=':' )
			path[ pos ] = '\0';
		else
			path[pos+1] = '\0';

		transfer_utils::UnzipAndDeleteFile( path, m_zippedFileName );
		
//		tstring::size_type dotPos = logFileName.rfind(_T('.'));
//		if ( dotPos != tstring::npos )
//			logFileName.erase(dotPos);
	}
	m_zippedFile = false;
	
	// Doesn't matter what we are sending, it's just notify client what operation was completed
	rfbFileTransferMsg ft = {0};
	m_pStream->Send((char*)&ft,sizeof(ft));
	
	m_pServer->m_transferLog.AddMessage(Format(_T("Received from expert %s"),m_openedFileName.c_str()).c_str());

	// Show folder content after decompressing where is correct folder name path m_openedFileName
	CDirContentLogging(&m_pServer->m_transferLog).ContentToLog(m_openedFileName,_T("Received from expert "));

	m_pServer->NotifyActivity(_T(""));
		
CATCH_THROW("CServerThread::EndOfFile")
}


void CServerThread::FileChecksums(const rfbFileTransferMsg&)
{

}


void CServerThread::FileHeader(const rfbFileTransferMsg& msg)
{
TRY_CATCH
	const unsigned bufsize = g_uTransferChunk*1;
	BYTE sendBuf[bufsize];
	DWORD dwReaden = 0;
	DWORD dwEOF = 0;
	///while( !dwEOF )
	//{
		BOOL ret = ReadFile( m_hFile.get() , sendBuf , bufsize , &dwReaden , 0 );
		if( !dwReaden  )
		{
			if( !GetLastError() )
			{
				rfbFileTransferMsg ft = {0};
				ft.contentType = rfbFilePacket;
				ft.size = Swap32IfLE( 0 ) ;
				ft.length = 0;
				m_pStream->Send( (char*)&ft , sizeof(ft) );

				m_hFile.reset();
				if( m_zippedFile )
				{
					m_zippedFile = false;
					::DeleteFile( m_openedFileName.c_str() );

					m_pServer->m_transferLog.AddMessage(Format(_T("Sent to expert %s"),m_zippedFileName.c_str()).c_str());

					/// Show folder content after decompressing where is correct folder name path m_zippedFileName
					CDirContentLogging(&m_pServer->m_transferLog).ContentToLog(m_zippedFileName,_T("Sent to expert "));
				}
				else
					m_pServer->m_transferLog.AddMessage(Format(_T("Sent to expert %s"),m_openedFileName.c_str()).c_str());

				m_pServer->NotifyActivity(_T(""));

				return;
			}
			/// Fixed bug RCE-59
		}

		rfbFileTransferMsg ft = {0};
		ft.contentType = rfbFilePacket;
		ft.size = Swap32IfLE( dwReaden ) ;
		ft.length = 0;
		m_pStream->Send( (char*)&ft , sizeof(ft) );
		m_pStream->Send( (char*)sendBuf , dwReaden );
	//	Sleep(1);
	//}
	CATCH_THROW("CServerThread::FileHeader")
	// read file
	// pack it if it needs
	// send message rfbFilePacker ( size of the buffer )
	// send buffer
}


void CServerThread::FilePacket(const rfbFileTransferMsg& msg)
{
	TRY_CATCH
	const unsigned bufsize = g_uTransferChunk;
	char receivBuf[bufsize];
	m_pStream->Receive( receivBuf , Swap32IfLE( msg.size ) );
	DWORD dwWritten = 0;
	WriteFile( m_hFile.get(), receivBuf , Swap32IfLE( msg.size ) , &dwWritten , 0 );
	
	rfbFileTransferMsg ft = {0};
	ft.contentType = rfbFilePacket;
	m_pStream->Send((char*)&ft ,sizeof(ft));
	
	CATCH_THROW("CServerThread::FilePacket")
}


void CServerThread::FileTransferOffer(const rfbFileTransferMsg& msg)
{
	TRY_CATCH
	// in usual I must check if this file exist
	// if file has exist already I ought to calculatr checksum and 
	// send rfbChecksums message to client

	const UINT length = Swap32IfLE(msg.length);
	std::vector<char> szFullDestName;
	szFullDestName.resize( length + 1 );
	// Read in the Name of the file to create
	m_pStream->Receive(&szFullDestName[0], length); 
	szFullDestName[length] = '\0';

	m_pServer->NotifyActivity(Format(_T("Expert is currently sending file (%s) to your PC"), &szFullDestName[0]));


	CARD32 sizeL = Swap32IfLE(msg.size);
	CARD32 sizeH = 0;
	CARD32 sizeHtmp = 0;

	sizeH = Swap32IfLE(sizeHtmp);

	rfbFileTransferMsg ft = {0};
	ft.contentType = rfbFileAcceptHeader;
	ft.size = Swap32IfLE(0); // File Size in bytes, 0xFFFFFFFF (-1) means error
	ft.length = Swap32IfLE(strlen(&szFullDestName[0]));

    /** NOTE Here you ought to check contentParam member of rfbFileTransferMsg

		- if it's equal 0   you try to create new file if file exists you must return 
		  file_exist in contentParam member of rfbFileTransferMsg and break function execution

		  - if it's equal 1  you must alway create new file
	*/

	DWORD open_flag = (msg.contentParam & FT_CREATE_ALWAYS )?( CREATE_ALWAYS ):(CREATE_NEW  );
	m_zippedFile = ( msg.contentParam & FT_ZIPPED_FOLDER )?true:false;
	
	if ( m_zippedFile )
	{
		tstring dirName(&szFullDestName[0]);
		dirName.erase(dirName.rfind(_T('.')));

		if ( !CreateDirectory(dirName.c_str(),NULL) )
		{
			DWORD err = GetLastError();
			if ( err == ERROR_ALREADY_EXISTS )					//FIX Directory overwrite
			{
				if ( open_flag == CREATE_NEW )
				{
					ft.contentParam = ERROR_FILE_EXISTS;			//FIX Error Codes
					m_pStream->Send((char *)&ft,sizeof(rfbFileTransferMsg));
					return;
				}
			}
			else
			{
				ft.contentParam = err;
				m_pStream->Send((char *)&ft,sizeof(rfbFileTransferMsg));
				return;
			}
		}
	}
	if ( m_zippedFile )
	{
		TCHAR tmpPath[MAX_PATH];
		GetTempPath(MAX_PATH,tmpPath);

		tstring archiveName = &szFullDestName[0];
		archiveName.erase(0,archiveName.find_last_of(_T('\\'))+1);
		archiveName.insert(0,tmpPath);
		
		m_zippedFileName = archiveName;
		m_openedFileName = &szFullDestName[0];
		m_openedFileName.erase(m_openedFileName.rfind(_T('.')));

		m_hFile = SPHandle( CreateFile(	m_zippedFileName.c_str(),GENERIC_WRITE | GENERIC_READ,
										FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL) , CloseHandle	 );
	}
	else
	{
		m_hFile = SPHandle( CreateFile(	&szFullDestName[0],GENERIC_WRITE | GENERIC_READ,
										FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,open_flag,	FILE_FLAG_SEQUENTIAL_SCAN,NULL) , CloseHandle	 );

		m_openedFileName = &szFullDestName[0];
	}

	if( m_hFile.get() == INVALID_HANDLE_VALUE )
	{
		/*
		if( GetLastError() == ERROR_FILE_EXISTS )
		{
			ft.contentParam = file_exist;
			m_pStream->Send((char *)&ft,sizeof(rfbFileTransferMsg));
			return;
		}
		if( GetLastError() == ERROR_ACCESS_DENIED )
		{
			ft.contentParam = access_denided;
			m_pStream->Send((char *)&ft,sizeof(rfbFileTransferMsg));
			return;
		}
		*/

		//RCE-133
		//switch( GetLastError()  )
		//{
		//	case ERROR_FILE_EXISTS:
		//		ft.contentParam = file_exist;
		//		break;
		//	case ERROR_ACCESS_DENIED:
		//		ft.contentParam = access_denided;
		//		break;
		//	default:
		//		ft.contentParam = unknown_erorr;
		//}
		ft.contentParam = GetLastError();		//FIX Error Codes		

		m_hFile.reset();

		m_pStream->Send((char *)&ft,sizeof(rfbFileTransferMsg));
		return;
	}

	m_pStream->Send((char *)&ft,sizeof(rfbFileTransferMsg));
	m_pStream->Send((char *)&szFullDestName[0], static_cast<unsigned int>(strlen(&szFullDestName[0])));
	Log.Add(_MESSAGE_ ,  "File name %s\n" , &szFullDestName[0] );
	CATCH_THROW("CServerThread::FileTransferOffer")
}


void CServerThread::FileTransferRequest(const rfbFileTransferMsg& msg)
{
TRY_CATCH
	const UINT length = Swap32IfLE(msg.length);
	std::vector<char> szFullDestName;
	szFullDestName.resize( length + 1 );
	// Read in the Name of the file to create
	m_pStream->Receive(&szFullDestName[0], length); 
	szFullDestName[length] = '\0';

	tstring sfile;
	//(transfer_utils::IsFolder( &szFullDestName[0]))?( transfer_utils::ZipFolder( &szFullDestName[0] , sfile ) ):(sfile = &szFullDestName[0]);

	
	//IS IT FIX ?
	m_hFile.reset();
	
	BOOL ret = transfer_utils::IsFolder( &szFullDestName[0]);
	if( ret )
	{
		m_pServer->NotifyActivity(Format(_T("Expert is currently retrieving folder (%s) from your PC"), &szFullDestName[0]));

		transfer_utils::ZipFolder( &szFullDestName[0] , sfile );
		//m_openedFileName = sfile;
		m_zippedFile = true;
		m_zippedFileName = &szFullDestName[0];
	}
	else
	{
		sfile = &szFullDestName[0];
		m_pServer->NotifyActivity(Format(_T("Expert is currently retrieving file (%s) from your PC"), sfile.c_str()));
	}

	m_openedFileName = sfile;

	m_hFile = SPHandle( CreateFile(sfile.c_str(),GENERIC_READ,FILE_SHARE_READ,
		NULL,OPEN_EXISTING,	FILE_FLAG_SEQUENTIAL_SCAN,NULL) , CloseHandle	 );

	rfbFileTransferMsg ft={0};
	
	// Fixed bug RCE-18
	if( m_hFile.get() == INVALID_HANDLE_VALUE )
	{
		// it needs to observe client part of the code
		ft.type = rfbFileTransfer;
		ft.contentType = rfbFilePacket;
		ft.contentParam = 0;
//		ft.size = Swap32IfLE( reject_operation );
		ft.size = Swap32IfLE( GetLastError() );			//FIX Error Codes
		m_pStream->Send( (char*)&ft , sizeof( ft ) );

		m_hFile.reset();

		return;
	}


	ft.type = rfbFileTransfer;
	ft.contentType = rfbFileHeader;
	ft.contentParam = 0;


	LARGE_INTEGER n2SrcSize;
	GetFileSizeEx( m_hFile.get() , &n2SrcSize );


	//ft.contentType = rfbFileTransferOffer;

	if( ret )
		ft.contentParam |= FT_ZIPPED_FOLDER;

	ft.size = Swap32IfLE(n2SrcSize.LowPart); // File Size in bytes
	ft.length = Swap32IfLE(sfile.length());
	m_pStream->Send( (char*)&ft , sizeof( ft ) );
	m_pStream->Send( sfile.c_str() , static_cast<unsigned int>(sfile.length()));

CATCH_THROW("CServerThread::FileTransferRequest")
}




//static __stdcall void ThreadEntry(void*);
unsigned __stdcall CServerThread::ThreadEntry(void* p )
{
SET_THREAD_LS;

TRY_CATCH
	boost::scoped_ptr<ThreadParKeeper> tp(reinterpret_cast<ThreadParKeeper*>( p ));
	rfbFileTransferMsg msg;
	while(!tp->this_->m_fStopThread) 
	{
//		Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry Receive exchange datagram");
		tp->stream->Receive( (char*)(&msg) , sizeof( msg )  );
		//Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry Datagram body is");

		//Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry msg.contentParam = %d", msg.contentParam);
		//Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry msg.contentType	 = %d", msg.contentType);
		//Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry msg.length       = %d", msg.length);
		//Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry msg.size         = %d", msg.size);
		//Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry msg.type         = %d", msg.type);

		if( msg.contentType == rfbAbortFileTransfer )
		{
			Log.Add(_MESSAGE_ , "rfbAbortFileTransfer");
		}
		//CALL_HANDLER( tp.get() , msg );
//		Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry Check permissions");
		if( tp->this_->CheckPermissions( msg ) )
		{ 
			THandlers::iterator it = tp->this_->m_handlerMap.find( msg.contentType );
			if( it!= tp->this_->m_handlerMap.end())
			{
//				Log.Add(_MESSAGE_ , "CServerThread::ThreadEntry Call the handler");
				(tp->this_->*((*it).second))( msg );
			}
		}
	}

CATCH_LOG("CServerThread::ThreadEntry")
	return 0;
}

void CServerThread::ClientCommandHanler(const rfbFileTransferMsg& msg)
{
TRY_CATCH
	switch (msg.contentParam)
	{
	case rfbCDirDelete:
		{
			const UINT length = Swap32IfLE(msg.length);
			char szDir[MAX_PATH]={'\0'};
			if (length > sizeof(szDir)) break;

			// Read in the Name of Dir to explore
			m_pStream->Receive(szDir, length);

			//szDir[length] = 0;
			int res;
			SHFILEOPSTRUCT fo;

			ZeroMemory(&fo, sizeof(fo));
			fo.pFrom  = szDir;
			fo.pTo    = NULL;				//FIX
			fo.wFunc  = FO_DELETE;
			fo.fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_NOERRORUI;

			CDirContentLogging dcl(&m_pServer->m_transferLog);
			dcl.StoreContent(szDir);

			m_pServer->NotifyActivity(Format(_T("Expert is currently deleting %s directory on your computer"),szDir));
			
			res = SHFileOperation(&fo);
			
			if ( !res )
			{
				dcl.StoredContentToLog(_T("Deleted by expert "));
				m_pServer->m_transferLog.AddMessage(Format(_T("Deleted by expert %s"),szDir).c_str());
			}

			m_pServer->NotifyActivity(_T(""));

			rfbFileTransferMsg ft;
			ft.type = rfbFileTransfer;
			ft.contentType = rfbCommandReturn;
			ft.contentParam = rfbCDirDelete;
			//ft.size = res ? -1 : operation_completed;
			ft.size = res;			//FIX Error Codes

			ft.length = msg.length;

			m_pStream->Send((char *)&ft, sizeof(ft));
			m_pStream->Send((char *)szDir, (int)length);
		}
		break;
		// Client requests the creation of a directory
	case rfbCDirCreate:
		{
			const UINT length = Swap32IfLE(msg.length);
			std::vector<char> szDir;
			szDir.resize( length + 1 );
			// Read in the Name of Dir to explore
			m_pStream->Receive(&szDir[0], length);

			szDir[length] = '\0';

			m_pServer->NotifyActivity(Format(_T("Expert is currently creating %s directory on your computer"),szDir));

			// Create the Dir
			BOOL fRet = CreateDirectory(&szDir[0], NULL);

			m_pServer->NotifyActivity(_T(""));

			if ( fRet )
				m_pServer->m_transferLog.AddMessage(Format(_T("Created by expert %s"),&szDir[0]).c_str());

			rfbFileTransferMsg ft;
			ft.type = rfbFileTransfer;
			ft.contentType = rfbCommandReturn;
			ft.contentParam = rfbADirCreate;
			//ft.size = fRet ? operation_completed : -1;
			ft.size = fRet ? 0 : GetLastError();			//FIX Error Codes

			ft.length = msg.length;

			m_pStream->Send((char *)&ft, sizeof(ft));
			m_pStream->Send((char *)&szDir[0], (int)length);
		}
		break;

		// Client requests the deletion of a file
	case rfbCFileDelete:
		{
			const UINT length = Swap32IfLE(msg.length);
			char szFile[MAX_PATH];
			if (length > sizeof(szFile)) break;

			// Read in the Name of the File 
			m_pStream->Receive(szFile, length);
			szFile[length] = 0;

			m_pServer->NotifyActivity(Format(_T("Expert is currently deleting %s file on your computer"),szFile));

			// Delete the file
			BOOL fRet = DeleteFile(szFile);

			if ( fRet )
				m_pServer->m_transferLog.AddMessage(Format(_T("Deleted by expert %s"),szFile).c_str());
			m_pServer->NotifyActivity(_T(""));

			rfbFileTransferMsg ft;
			ft.type = rfbFileTransfer;
			ft.contentType = rfbCommandReturn;
			ft.contentParam = rfbAFileDelete;
			//ft.size = fRet ? operation_completed : -1;
			ft.size = fRet ? 0 : GetLastError();				//FIX Error Codes
			
			ft.length = msg.length;

			m_pStream->Send((char *)&ft, sizeof(rfbFileTransferMsg));
			m_pStream->Send((char *)szFile, (int)length);
		}
		break;

		// Client requests the Renaming of a file/directory
	case rfbCFileRename:
		{
			const UINT length = Swap32IfLE(msg.length);
			char szNames[(2 * MAX_PATH) + 1];
			if (length > sizeof(szNames)) break;

			// Read in the Names
			m_pStream->Receive(szNames, length);
			szNames[length] = 0;

			char *p = strrchr(szNames, '*');
			if (p == NULL) break;
			char szCurrentName[MAX_PATH];
			char szNewName[MAX_PATH];

			strcpy(szNewName, p + 1); 
			*p = '\0';
			strcpy(szCurrentName, szNames);
			*p = '*';

			m_pServer->NotifyActivity(Format(_T("Expert is currently renaming %s file on your computer"),szCurrentName));

			// Rename
			BOOL fRet = MoveFile(szCurrentName, szNewName);
			
			if ( fRet )
				m_pServer->	m_transferLog.AddMessage(Format(_T("Renamed by expert %s to %s"),
															szCurrentName,
															szNewName).c_str());

				
			m_pServer->NotifyActivity(_T(""));

			rfbFileTransferMsg ft;
			ft.type = rfbFileTransfer;
			ft.contentType = rfbCommandReturn;
			ft.contentParam = rfbAFileRename;
			//ft.size = fRet ? operation_completed :-1;
			ft.size = fRet ? 0 : GetLastError();		//FIX Error Codes
			
			ft.length = msg.length;

			m_pStream->Send((char *)&ft, sizeof(rfbFileTransferMsg));
			m_pStream->Send((char *)szNames, (int)length);
		}
		break;
	}  // End of swith
CATCH_THROW("CServerThread::ClintCommandHanler")
}

bool CServerThread::CheckPermissions(const rfbFileTransferMsg& msg)
{
TRY_CATCH
		if( 
			(msg.contentType == rfbFilePacket)
			|| (msg.contentType == rfbEndOfFile)
			|| (msg.contentType == rfbFileHeader )
			|| ( msg.contentType == rfbAbortFileTransfer ) 
		   )
		return true;

//	Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions doesn't return true, go further...");

	TransferOpearation op = GetOperation( msg );
	//Fixed RCE-107
	ResponseCode ret = m_pServer->NotifyOperation( m_pServer->m_userPermisiions[ m_pServer->m_userPermisiions.size() - op - 1 /*CServerThread::powOf2( op ) */] , op );//((accept_operation == m_pServer->NotifyOperation( m_pServer->m_userPermisiions[ CServerThread::powOf2( op ) ] , op )))?true:false;

	rfbFileTransferMsg ft;
	ft.type = rfbFileTransfer;
	ft.contentType = rfbCommandReturn;
	ft.contentParam = op;
	ft.size = Swap32IfLE(ret);
	ft.length = 0;

	//Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions rfbFileTransferMsg body is");
	//
	//Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions ft.type         = %d",ft.type);
	//Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions ft.contentType  = %d",ft.contentType);
	//Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions ft.contentParam = %d",ft.contentParam);
	//Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions ft.size         = %d",ft.size);
	//Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions ft.length       = %d",ft.length);

	m_pStream->Send((char *)&ft, sizeof(rfbFileTransferMsg));

	//if ( ret == operation_accepted )
	//	Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions operation accepted");
	//else
	//	Log.Add(_MESSAGE_ , "CServerThread::CheckPermissions operation declined");
	
	//return (ret==accept_operation)?true:false;
	return ( ret == operation_accepted );		//FIX Error Codes

CATCH_THROW("CServerThread::CheckPermissions")
}

int CServerThread::powOf2( int v )
{
TRY_CATCH
	const unsigned int b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00,0xFFFF0000};
	register unsigned int r = (v & b[0]) != 0;
	for (int i = 4; i > 0; i--) // unroll for speed...
	{
		r |= ((v & b[i]) != 0) << i;
	}
	return r;
CATCH_THROW("CServerThread::powOf2")
}

void CServerThread::AbortTranserOperation( const rfbFileTransferMsg& msg )
{
	m_hFile.reset();
	if( msg.size == 0 && msg.length == send_file )
		// delete sending file	
		::DeleteFile( m_openedFileName.c_str() );
	
	
	tstring logFileName(m_openedFileName);
	if ( m_zippedFile )
	{
		tstring::size_type dotPos = logFileName.rfind(_T('.'));
		if ( dotPos != tstring::npos )
			logFileName.erase(dotPos);
	}
	if ( msg.length == send_file )
		m_pServer->m_transferLog.AddMessage(Format(_T("Receiving from expert %s was canceled"),logFileName.c_str()).c_str());
	else
		m_pServer->m_transferLog.AddMessage(Format(_T("Sending to expert %s was canceled"),logFileName.c_str()).c_str());

	m_pServer->NotifyActivity(_T(""));

	m_openedFileName.clear();
}
