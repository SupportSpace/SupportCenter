/////////////////////////////////////////////////////////////////////////
///
///  CFileAccessClient.cpp
///
///  <TODO: insert file description here>
///
///  @remarks <TODO: insert remarks here>
///
///  @author Dmiry S. Golub @date 11/7/2006
///
////////////////////////////////////////////////////////////////////////

#include "CFileAccessClient.h"
#include "rfb.h"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <AidLib/CException/CException.h>
#include "cfileaccessclient.h"

#include "utils.h"

#define __DEBUG_REPORT__

#pragma warning( disable: 4996 )//<func> was declared deprecated


CFileAccessClient::~CFileAccessClient()
{
}

void CFileAccessClient::SetStream(boost::shared_ptr<CAbstractStream> stream)
{
	TRY_CATCH
	
	m_pStream = stream;
		
	CATCH_THROW()
}

CFileAccessClient::CFileAccessClient(HANDLE hAbortEvent)
	:	m_hAbortEvent( hAbortEvent )
{
	TRY_CATCH
	CATCH_THROW("CFileAccessClient::CFileAccessClient")
}


ResponseCode CFileAccessClient::DeleteDirectory(const tstring& remoteDirectory)
{
TRY_CATCH
	//ResponseCode ret = operation_completed;
	ResponseCode ret = ResponseCode(0);			//FIX Error Codes

	rfbFileTransferMsg msg={0};
	msg.contentType = rfbCommand;
	msg.contentParam = rfbCDirDelete;
	msg.length = Swap32IfLE(remoteDirectory.length());
	m_pStream->Send( (char*)&msg , sizeof( msg ) );

	ZeroMemory( &msg , sizeof(msg) );
	m_pStream->Receive( (char*)&msg , sizeof( msg ) );
	if( rfbCommandReturn == msg.contentType )
	{
		//if( Swap32IfLE( msg.size ) == accept_operation  )
		if ( Swap32IfLE( msg.size ) == operation_accepted )			//FIX Error Codes
		{
			m_pStream->Send( remoteDirectory.c_str() , static_cast<unsigned int>(remoteDirectory.length()) );
			ZeroMemory( &msg , sizeof(msg) );
			m_pStream->Receive( (char*)&msg , sizeof( msg ) );
			if(rfbCommandReturn == msg.contentType )
			{
				std::vector<char> szDirName;
				szDirName.resize( Swap32IfLE( msg.length ) + 1 );
				m_pStream->Receive( &szDirName[0] , Swap32IfLE( msg.length ) );
				szDirName[Swap32IfLE( msg.length )] = '\0';
//				Log.Add(_MESSAGE_, "Directory %s %s deleted" , &szDirName[0] , ( msg.size == operation_completed )?("wasn't "):("was succesfully ") );
				Log.Add(_MESSAGE_, "Directory %s %s deleted" , &szDirName[0] , ( msg.size != 0 )?("wasn't "):("was succesfully ") );	//FIX Error Codes
				ret =  (ResponseCode)msg.size;
			}
		}
	}
	return ret;
CATCH_THROW("CFileAccessClient::DeleteDirectory")
}


ResponseCode CFileAccessClient::DeleteFile(const tstring& remoteFile)
{
TRY_CATCH
	//BOOL ret = FALSE;
	ResponseCode ret = operation_rejected;		//FIX Error Codes
	
	rfbFileTransferMsg msg={0};
	msg.contentType = rfbCommand;
	msg.contentParam = rfbCFileDelete;
	msg.length = Swap32IfLE(remoteFile.length());
	m_pStream->Send( (char*)&msg , sizeof( msg ) );
	m_pStream->Send( remoteFile.c_str() , static_cast<unsigned int>(remoteFile.length()) );
	ZeroMemory( &msg , sizeof(msg) );
	m_pStream->Receive( (char*)&msg , sizeof( msg ) );
	
	if( rfbCommandReturn == msg.contentType )
	{
		std::vector<char> szDirName;
		szDirName.resize( Swap32IfLE( msg.length ) + 1 );
		m_pStream->Receive( &szDirName[0] , Swap32IfLE( msg.length ) );
		szDirName[Swap32IfLE( msg.length )] = '\0';
		//Log.Add(_MESSAGE_, "File %s %s renamed" , &szDirName[0] , ( Swap32IfLE( msg.size ) == operation_completed )?("wasn't "):("was succesfully ") );
		Log.Add(_MESSAGE_, "File %s %s renamed" , &szDirName[0] , ( Swap32IfLE( msg.size ) != 0 )?("wasn't "):("was succesfully ") );		//FIX Error Codes

		//ret = ( Swap32IfLE( msg.size ) == operation_completed  )?TRUE:FALSE;
		ret = ResponseCode(Swap32IfLE( msg.size ));		//FIX Error Codes

	}

	return ret;
	CATCH_THROW("CFileAccessClient::DeleteFile")
}


ResponseCode CFileAccessClient::ListDrives(TDriveInfo& di)
{
TRY_CATCH

//	Log.Add(_MESSAGE_,_T("+ CFileAccessClient::ListDrives"));

	rfbFileTransferMsg msg;
	msg.contentType = rfbDirContentRequest;
	msg.contentParam =  rfbRDrivesList;

//	Log.Add(_MESSAGE_,_T("  CFileAccessClient::ListDrives: Do send"));
	m_pStream->Send( (char*)&msg , sizeof( msg ) );

	ZeroMemory( &msg , sizeof(msg) );

//	Log.Add(_MESSAGE_,_T("  CFileAccessClient::ListDrives: Do the first receive"));
	m_pStream->Receive( (char*)&msg , sizeof( msg ) );

	if( rfbCommandReturn == msg.contentType )
	{
//		Log.Add(_MESSAGE_,_T("  CFileAccessClient::ListDrives: rfbCommandReturn == msg.contentType"));
	
		//if( Swap32IfLE( msg.size ) == accept_operation  )
		if ( Swap32IfLE( msg.size ) == operation_accepted )			//FIX Error Codes
		{

//			Log.Add(_MESSAGE_,_T("  CFileAccessClient::ListDrives: Swap32IfLE( msg.size ) == operation_accepted"));
//			Log.Add(_MESSAGE_,_T("  CFileAccessClient::ListDrives: Do the second receive"));
			m_pStream->Receive( (char*)&msg , sizeof( rfbFileTransferMsg ) );

			if( rfbDirPacket == msg.contentType )
			{
//				Log.Add(_MESSAGE_,_T("  CFileAccessClient::ListDrives: rfbDirPacket == msg.contentType"));
	
				boost::scoped_array<char> devices( new char[Swap32IfLE(msg.length)+1] );

				Log.Add(_MESSAGE_,_T("  CFileAccessClient::ListDrives: Do the loop receive"));
				m_pStream->Receive( devices.get() , Swap32IfLE(msg.length) );
				TokenizeDrives(di, Swap32IfLE(msg.length), devices.get()   );

				return ResponseCode(0);
			}
		}
		//return operation_completed;
		//return ResponseCode(0);					//FIX Error Codes
	}
	//return unknown_message;
	return operation_rejected;					//FIX Error Codes
CATCH_THROW("CFileAccessClient::ListDrives")
}


void CFileAccessClient::ListFiles(const tstring& path,TFileInfo& fi)
{
	//rfbRDirContent
TRY_CATCH
	rfbFileTransferMsg msg;
	msg.contentType = rfbDirContentRequest;
	msg.contentParam =  rfbRDirContent;
	msg.length = Swap32IfLE( path.length() );
	m_pStream->Send( (char*)&msg , sizeof( msg ) );

	ZeroMemory( &msg , sizeof(msg) );
	m_pStream->Receive( (char*)&msg , sizeof( msg ) );
	if( rfbCommandReturn == msg.contentType )
	{
		//if( Swap32IfLE( msg.size ) == accept_operation  )
		if ( Swap32IfLE( msg.size ) == operation_accepted )			//FIX Error Codes
		{

			m_pStream->Send( path.c_str() , static_cast<unsigned int>(path.length())  );

			m_pStream->Receive( (char*)&msg , sizeof( msg ) );

			char buf[255];
			while( msg.contentType && Swap32IfLE(msg.length)  )
			{
				m_pStream->Receive( buf , Swap32IfLE(msg.length) );
				WIN32_FIND_DATA* ptr = reinterpret_cast<WIN32_FIND_DATA*>( buf );
				fi.push_back( *ptr );
				m_pStream->Receive( (char*)&msg , sizeof( msg ) );
			}
		}

	}
CATCH_THROW("CFileAccessClient::ListFiles")
}

void CFileAccessClient::NotifyTransferStatus(Status& status)
{
}

void CFileAccessClient::NotifyUncompressStatus()
{
}

ResponseCode CFileAccessClient::RenameFile(const tstring& newName, const tstring& oldName)
{
TRY_CATCH
	//ResponseCode ret = operation_completed;
	ResponseCode ret = operation_rejected;							//FIX Error Codes
	
	rfbFileTransferMsg msg={0};
	msg.contentType = rfbCommand;
	msg.contentParam = rfbCFileRename;

	//m_pStream->Send( (char*)&msg , sizeof( msg ) );
	tstring name = oldName;
	name+="*";
	name+=newName;

	msg.length = Swap32IfLE(name.length());
	m_pStream->Send( (char*)&msg , sizeof( msg ) );

	ZeroMemory( &msg , sizeof(msg) );
	m_pStream->Receive( (char*)&msg , sizeof( msg ) );
	if( rfbCommandReturn == msg.contentType )
	{
		//if( Swap32IfLE( msg.size ) == accept_operation  )
		if ( Swap32IfLE( msg.size ) == operation_accepted )			//FIX Error Codes
		{
			m_pStream->Send( name.c_str() , static_cast<unsigned int>(name.length()) );
			ZeroMemory( &msg , sizeof(msg) );
			m_pStream->Receive( (char*)&msg , sizeof( msg ) );
			if (rfbCommandReturn == msg.contentType )
			{
				std::vector<char> szDirName;
				szDirName.resize( Swap32IfLE( msg.length ) + 1 );
				m_pStream->Receive( &szDirName[0] , Swap32IfLE( msg.length ) );
				szDirName[Swap32IfLE( msg.length )] = '\0';
				//Log.Add(_MESSAGE_, "File %s %s renamed" , &szDirName[0] , ( Swap32IfLE( msg.size ) == operation_completed )?("wasn't "):("was succesfully ") );
				Log.Add(_MESSAGE_, "File %s %s renamed" , &szDirName[0] , ( Swap32IfLE( msg.size ) != 0 )?("wasn't "):("was succesfully ") );	    //FIX Error Codes
				ret = ResponseCode(msg.size);
			}
		}
		else
			//ret = reject_operation;
			ret = operation_rejected;								//FIX Error Codes

	}
	return ret;	
CATCH_THROW("CFileAccessClient::RemaneFile")
}

int Swap32IfLE_f(int l)
{
	int ret = ((CARD32) ((((l) & 0xff000000) >> 24) | (((l) & 0x00ff0000) >> 8)  | (((l) & 0x0000ff00) << 8)  | (((l) & 0x000000ff) << 24)));
	return ret;
}

ResponseCode CFileAccessClient::ReceiveFile(const tstring& remotefile, const tstring& localFile_ , bool always_rewrite)
{
TRY_CATCH

	rfbFileTransferMsg ft = {0};
	ft.type = rfbFileTransfer;
	ft.contentType = rfbFileTransferRequest;
	//ft.contentParam = 0;
	//ft.size = Swap32IfLE(n2SrcSize.LowPart); // File Size in bytes
	ft.length = Swap32IfLE(remotefile.length());
	m_pStream->Send( (char*)&ft , sizeof( ft ) );

	ZeroMemory( &ft , sizeof(ft) );
	m_pStream->Receive( (char*)&ft , sizeof( ft ) );
	if( rfbCommandReturn == ft.contentType )
	{
		//if( Swap32IfLE( ft.size ) == accept_operation  )
		if ( Swap32IfLE( ft.size ) == operation_accepted )			//FIX Error Codes
		{

			m_pStream->Send( remotefile.c_str() , static_cast<unsigned int>(remotefile.length()) );

			ZeroMemory( &ft , sizeof( rfbFileTransferMsg ) );
			m_pStream->Receive( (char*)(&ft) , sizeof(ft) );

			if( rfbFileHeader == ft.contentType )
			{
				long size = Swap32IfLE(ft.size); 
				
				bool bZipped = (( ft.contentParam & FT_ZIPPED_FOLDER ) != 0);

				std::vector<char> szfileName;
				szfileName.resize( Swap32IfLE(ft.length) + 1 );
				m_pStream->Receive( &szfileName[0] , Swap32IfLE(ft.length) );
				szfileName[Swap32IfLE(ft.length)]='\0';

				char drive[_MAX_DRIVE];
				char dir[_MAX_DIR];
				char fname[_MAX_FNAME];
				char ext[_MAX_EXT];
				_splitpath( &szfileName[0], drive, dir, fname, ext ); 

				//m_curFileStatus.fileName = fname;
				//m_curFileStatus.fileName+= ext;

				tstring localFile(localFile_);
				if ( bZipped )
				{
					TCHAR tmpPath[MAX_PATH];
					GetTempPath(MAX_PATH,tmpPath);
					
					//localFile=fname;
					//localFile+=ext;
					
					localFile = tmpPath;
					localFile += fname;
					localFile += ext;
				}

				/** NOTE contentParam of rfbFileTransferMsg consists information about behavior of the server side
				0 - try to create new file. If file exists server returns file_exist in size member of rfbFileTransferMsg
				1 - always create new file 
				*/

				/// RCE-120
				DWORD open_flag = (always_rewrite || bZipped )?( CREATE_ALWAYS ):(CREATE_NEW  );

				// Create the local Destination file
				///typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type> spFileHandle;

				if ( bZipped && !::CreateDirectory(localFile_.c_str(),NULL) ) //FIX Directory overwrite
				{
					DWORD err = GetLastError();

					if ( err == ERROR_ALREADY_EXISTS )
					{
						if ( !always_rewrite )
							return ResponseCode(ERROR_FILE_EXISTS);
					}
					else
						return ResponseCode(err);
				}

				SPHandle hDestFile = SPHandle( 
					CreateFile(localFile.c_str(), 
								GENERIC_WRITE | GENERIC_READ,
								FILE_SHARE_READ | FILE_SHARE_WRITE, 
								NULL,
								open_flag,
								FILE_FLAG_SEQUENTIAL_SCAN,
								NULL),
								CloseHandle);


				if( hDestFile.get() == INVALID_HANDLE_VALUE )
				{
					//if( GetLastError() == ERROR_FILE_EXISTS )
					//{
					//	ZeroMemory( &ft , sizeof( rfbFileTransferMsg ) );

					//	ft.contentType = rfbEndOfFile; 
					//	m_pStream->Send((char *)&ft,sizeof(rfbFileTransferMsg));
					//	m_curFileStatus.status = completed;
					//	NotifyTransferStatus( m_curFileStatus );
					//	return file_exist;
					//}
					//return access_denided;
					
					return ResponseCode(GetLastError());			//FIX Error Codes
				}

				m_curFileStatus.fileName=localFile_; //FIX localFile_;
				m_curFileStatus.location = remotefile;
				m_curFileStatus.size = size;
				m_curFileStatus.transferred = 0;
				m_curFileStatus.status = starting;

				NotifyTransferStatus( m_curFileStatus );


				ZeroMemory( &ft , sizeof( rfbFileTransferMsg ) );

				ft.contentType = rfbFileHeader; 
				m_pStream->Send( (char*)(&ft) , sizeof(ft) );

				ZeroMemory( &ft , sizeof( ft ) );
				const unsigned bufsize = g_uTransferChunk*1;
				char buf[bufsize];

				for (;;)
				{
					DWORD dwWritten = 0;
					m_pStream->Receive( (char*)&ft , sizeof( ft ) );
					if( !ft.size )
						break;

					int test_size = Swap32IfLE_f( ft.size );
					m_pStream->Receive( buf , Swap32IfLE( ft.size ) );

					//Log.Add(_MESSAGE_,"Received %d\n", Swap32IfLE( ft.size ));

					WriteFile( hDestFile.get() , buf , Swap32IfLE( ft.size ) , &dwWritten , 0 );
					m_curFileStatus.transferred+= Swap32IfLE( ft.size );
					m_curFileStatus.status = inprocess;
					NotifyTransferStatus( m_curFileStatus );
					Sleep(1);

					DWORD dwAbort = WaitForSingleObject( m_hAbortEvent , 2 );
					switch( dwAbort )
					{
						case WAIT_OBJECT_0:
						{
							hDestFile.reset();
							::DeleteFile( localFile.c_str() );
							ZeroMemory( &ft , sizeof( rfbFileTransferMsg ) );
							ft.contentType = rfbAbortFileTransfer; 
							m_pStream->Send( (char*)(&ft) , sizeof(ft) );
							ResetEvent( m_hAbortEvent );
							m_curFileStatus.status = completed;
							NotifyTransferStatus( m_curFileStatus );
							//return abort_operation;
							return operation_was_aborted;		//FIX Error codes
						}

					};
					/// make loop for the server
					ZeroMemory( &ft , sizeof( rfbFileTransferMsg ) );
					ft.contentType = rfbFileHeader; 
					m_pStream->Send( (char*)(&ft) , sizeof(ft) );
				}

				if( bZipped )
				{
					hDestFile.reset();
					//transfer_utils::UnzipAndDeleteFile( localFile );

					tstring path( localFile_ );
					size_t pos = path.find_last_of("\\");
					if( path[pos-1]!=':' )
						path[ pos ] = '\0';
					else
						path[pos+1] = '\0';

					NotifyUncompressStatus();
					transfer_utils::UnzipAndDeleteFile( path, localFile);
				}
				m_curFileStatus.status = completed;
				NotifyTransferStatus( m_curFileStatus );
			}
			else
				return (ResponseCode)Swap32IfLE( ft.size );
		}
		else
			return (ResponseCode)Swap32IfLE( ft.size );
	}
	//return operation_completed;
	return ResponseCode(0);					//FIX Error Codes

CATCH_THROW(" CFileAccessClient::RetriveFile")
}


ResponseCode CFileAccessClient::SendFile(const tstring& remotefile_, const tstring& localFile_ , bool always_create)
{
TRY_CATCH

    tstring sfile;
	BOOL isFolder=transfer_utils::IsFolder( localFile_);

	if ( isFolder )
	{
		transfer_utils::ZipFolder( localFile_ , sfile );
		/// Checking if transfer was aborted------------------------------------
		DWORD dwAbort = WaitForSingleObject( m_hAbortEvent, 2 );
		if ( dwAbort == WAIT_OBJECT_0 )
		{
			rfbFileTransferMsg ft = {0};
			ft.contentType	= rfbAbortFileTransfer; 
			ft.length		= send_file;
			ft.size			= 0;
			
			m_pStream->Send( (char*)(&ft) , sizeof(ft) );
			
			ResetEvent( m_hAbortEvent );
			if( isFolder )
				::DeleteFile( sfile.c_str() );

			return operation_was_aborted;
		}
		/// Checking if transfer was aborted------------------------------------
	}
	else
	{
		sfile = localFile_;
	}

	///typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type> SPHandle;
	SPHandle hFile = SPHandle( CreateFile(sfile.c_str(),
		GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,NULL) , CloseHandle	 );

	// Fixed bug RCE-18
	// if file is busy or something else ...
	if( hFile.get() == INVALID_HANDLE_VALUE )
		//return reject_operation;
		return ResponseCode(GetLastError());			//FIX Error Codes

	LARGE_INTEGER n2SrcSize;
	GetFileSizeEx( hFile.get() , &n2SrcSize );

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath( remotefile_.c_str(), drive, dir, fname, ext ); 

	tstring remotefile(drive);
	remotefile+= dir;

	_splitpath( sfile.c_str(), drive, dir, fname, ext ); 

	remotefile+=fname;
	remotefile+=ext;

	m_curFileStatus.fileName=localFile_ ;
	m_curFileStatus.location = remotefile;
	m_curFileStatus.size = n2SrcSize.LowPart;
	m_curFileStatus.transferred = 0;
	m_curFileStatus.status = starting;
	NotifyTransferStatus( m_curFileStatus );

    /** NOTE contentParam of rfbFileTransferMsg consists information about behavior of the server side
		0 - try to create new file. If file exists server returns file_exist in size member of rfbFileTransferMsg
		FT_CREATE_ALWAYS - always create new file 
	*/
	rfbFileTransferMsg ft={0};
	ft.type = rfbFileTransfer;
	ft.contentType = rfbFileTransferOffer;

	if( always_create )
		ft.contentParam |= FT_CREATE_ALWAYS;

	if( isFolder )
		ft.contentParam |= FT_ZIPPED_FOLDER;

	
	ft.size = Swap32IfLE(n2SrcSize.LowPart); // File Size in bytes
	ft.length = Swap32IfLE(remotefile.length());
	m_pStream->Send( (char*)&ft , sizeof( ft ) );	// send info about file
	ZeroMemory( &ft , sizeof(ft) );
	m_pStream->Receive( (char*)&ft , sizeof( ft ) );// Reception of the sanction to record of a file
	if( rfbCommandReturn == ft.contentType )
	{
		//if( Swap32IfLE( ft.size ) == accept_operation  )// If a place under a file not empty
		if ( Swap32IfLE( ft.size ) == operation_accepted )		//FIX Error Codes
		{
			// Whether we ask it is possible to send a concrete file remotefile
			m_pStream->Send( remotefile.c_str() , static_cast<unsigned int>(remotefile.length()) );
			ZeroMemory( &ft , sizeof( ft ) );
			m_pStream->Receive( (char*)&ft , sizeof( ft ) );

			//if( ft.contentParam == file_exist || ft.contentParam == access_denided || ft.contentParam == unknown_erorr)
			//{
			//	m_curFileStatus.status = completed;
			//	NotifyTransferStatus( m_curFileStatus );
			//	return (ResponseCode)ft.contentParam;
			//}
			if ( ft.contentParam )							//FIX Error Codes
			{												//FIX Error Codes
				m_curFileStatus.status = completed;			//FIX Error Codes
				NotifyTransferStatus( m_curFileStatus );	//FIX Error Codes
				return ResponseCode(ft.contentParam);		//FIX Error Codes
			}												//FIX Error Codes

			if(  rfbFileAcceptHeader == ft.contentType )
			{
				//char szFile[255]={'\0'}; 
				std::vector<char> szFile;
				szFile.resize( Swap32IfLE(ft.length) + 1 );
				m_pStream->Receive( &szFile[0] , Swap32IfLE(ft.length) );


				const unsigned bufsize = g_uTransferChunk*1;
				BYTE sendBuf[bufsize];
				DWORD dwReaden = 0;
				DWORD err;
				
				for (;;)
				{
					BOOL rs = ReadFile( hFile.get() , sendBuf , bufsize , &dwReaden , 0 );
					
					if( 0 == dwReaden  )// If nothing is read through
					{
						if( !(err = GetLastError()) ) // If the error is not present
						{
							// No error happened
							rfbFileTransferMsg ft = {0};
							ft.contentType = rfbEndOfFile;
							ft.size = Swap32IfLE( 0 ) ;
							ft.length = 0;
							m_pStream->Send( (char*)&ft , sizeof(ft) );// send 0
							m_curFileStatus.status = completed;
							NotifyTransferStatus( m_curFileStatus );
							if( isFolder )
							{
								hFile.reset();
								::DeleteFile( sfile.c_str() );
								NotifyUncompressStatus();
							}
							m_pStream->Receive((char*)&ft,sizeof(ft));	// Wait until server processes EnfOfFile message
							//return operation_completed;
							return ResponseCode(0);				//FIX Error Codes
						} else
						{
							// Some error occured
							// There was a error of reading from a file
							SetLastError(err);
							Log.WinError(_ERROR_, _T("Failed to ReadFile"));
							rfbFileTransferMsg ft = {0};
							ft.contentType = rfbEndOfFile;
							ft.size = Swap32IfLE( 0 ) ;
							ft.length = 0;
							m_pStream->Send( (char*)&ft , sizeof(ft) );
							m_curFileStatus.status = completed;
							NotifyTransferStatus( m_curFileStatus );
							if( isFolder )
							{
								hFile.reset();
								::DeleteFile( sfile.c_str() );
								NotifyUncompressStatus();
							}
							m_pStream->Receive((char*)&ft,sizeof(ft));	// Wait until server processes EnfOfFile message
							//return operation_completed; 
							return ResponseCode(err);			//FIX Error Codes
						}
					}	// if( 0 == dwReaden  )

					rfbFileTransferMsg ft = {0};
					ft.contentType = rfbFilePacket;
					ft.size = Swap32IfLE( dwReaden ) ;
					ft.length = 0;
					m_pStream->Send( (char*)&ft , sizeof(ft) );
					m_pStream->Send( (char*)sendBuf , dwReaden );

					m_curFileStatus.transferred	+= dwReaden;
					m_curFileStatus.status		= inprocess;
					NotifyTransferStatus( m_curFileStatus );
					/// RCE-112

					//Fix bug with network overflow
					m_pStream->Receive((char*)(&ft),sizeof(ft));
					bool networkOverflow = ft.contentType != rfbFilePacket;

					DWORD dwAbort = WaitForSingleObject( m_hAbortEvent, 2 );
					if ( dwAbort == WAIT_OBJECT_0 || networkOverflow )
					{
						hFile.reset();
						rfbFileTransferMsg ft = {0};
						ft.contentType	= rfbAbortFileTransfer; 
						ft.length		= send_file;
						ft.size			= 0;
						
						m_pStream->Send( (char*)(&ft) , sizeof(ft) );
						
						ResetEvent( m_hAbortEvent );
						if( isFolder )
							::DeleteFile( sfile.c_str() );
						
						//return abort_operation;
						return operation_was_aborted;				//FIX Error Codes
					}
				}
			}
		}
		m_curFileStatus.status = completed;
		NotifyTransferStatus( m_curFileStatus );
		return (ResponseCode)Swap32IfLE( ft.size );
	}

	if( isFolder )
	{
		hFile.reset();
		::DeleteFile( sfile.c_str() );
	}
	//return operation_completed;
	return ResponseCode(0);							//FIX Error Codes

CATCH_THROW("CFileAccessClient::SendFile")
}

ResponseCode CFileAccessClient::CreateDirectory(const tstring& remotefile)
{
TRY_CATCH
	//ResponseCode ret = reject_operation;
	ResponseCode ret = operation_rejected;			//FIX Error Codes

	rfbFileTransferMsg msg={0};
	msg.contentType = rfbCommand;
	msg.contentParam = rfbCDirCreate;

	//m_pStream->Send( (char*)&msg , sizeof( msg ) );

	msg.length = Swap32IfLE(remotefile.length());
	m_pStream->Send( (char*)&msg , sizeof( msg ) );

	ZeroMemory( &msg , sizeof(msg) );
	m_pStream->Receive( (char*)&msg , sizeof( msg ) );
	if( rfbCommandReturn == msg.contentType )
	{
		//if( Swap32IfLE( msg.size ) == accept_operation  )
		if ( Swap32IfLE( msg.size ) == operation_accepted )			//FIX Error Codes
		{
			m_pStream->Send( remotefile.c_str() , static_cast<unsigned int>(remotefile.length()) );
			ZeroMemory( &msg , sizeof(msg) );
			m_pStream->Receive( (char*)&msg , sizeof( msg ) );
			if(rfbCommandReturn == msg.contentType )
			{
				std::vector<char> szDirName;
				szDirName.resize( Swap32IfLE( msg.length ) + 1 );
				m_pStream->Receive( &szDirName[0] , Swap32IfLE( msg.length ) );
				szDirName[Swap32IfLE( msg.length )] = '\0';
				//Log.Add(_MESSAGE_, "File %s %s renamed" , &szDirName[0] , ( ( msg.size ) == operation_completed )?("wasn't "):("was succesfully ") );
				Log.Add(_MESSAGE_, "File %s %s renamed" , &szDirName[0] , ( ( msg.size ) != 0 )?("wasn't "):("was succesfully ") );		//FIX Error Codes

				ret = (ResponseCode)(  msg.size );
			}
		}

	}
	return ret;	
CATCH_THROW("CFileAccessClient::CreateDirectory")
}
/**
 * You recived from server information looks like "C:l:total_size:free_size<NULL>D:
 * c:total_size:free_size<NULL>....Z:n\<NULL><NULL>" and this function parses this
 * information and saves in sotore.
 */
void CFileAccessClient::TokenizeDrives(TDriveInfo& dl, unsigned int len, char* drives)
{
TRY_CATCH
	char p[64];
	DrivesInfo di;
	for( unsigned i = 0, k = 0; i < len; ++i )
	{
		if( drives[i]!='\0' )
			p[k++] = drives[i];
		else
		{
			p[k] = '\0';
			TokenizeInfo( di ,p  );
			dl.push_back( di );
			k=0;
		}
	}
CATCH_THROW("CFileAccessClient::TokenizeDrives")
 }


/**
 * This is a helpful function for TokenizeDrives function
 */
void CFileAccessClient::TokenizeInfo(TDriveInfo::value_type& di, char* info)
{
TRY_CATCH
	char d;
	int ret = sscanf( info , "%c:\\:%i:%I64u:%I64u" , &d , &di.type , &di.space , &di.used_space );
	
	di.drive = d;
	di.drive+=":";
CATCH_THROW("CFileAccessClient::TokenizeInfo")
}

//BOOL CFileAccessClient::CheckFolder(const tstring& fn , tstring& nfn)
//{
//	if( GetFileAttributes( fn.c_str() ) & FILE_ATTRIBUTE_DIRECTORY   )
//	{
//		std::vector<char> szZipPath;
//		szZipPath.resize( 512 );
//
//		GetTempPath( szZipPath.capacity() , &szZipPath[0] );
//		lstrcat(&szZipPath[0], "folder.arc");
//
//		//char szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
//		//_splitpath(fn.c_str(), szDrive, szFolder, szName, szExt);
//
//		int pos = fn.find_last_of("\\");
//		tstring path( fn.begin() , fn.begin() + pos );
//		tstring folder( fn.begin() + ++pos , fn.end() );
//
//		CZipUnZip32 zip_arch;
//		TCHAR chPath[512]={'\0'}; 
//		TCHAR chFolder[512]={'\0'}; 
//		strcpy( chPath , path.c_str() );
//		strcpy( chFolder , folder.c_str() );
//		if( zip_arch.ZipDirectory( chPath , chFolder , &szZipPath[0] , true )) 
//		{
//			nfn =  &szZipPath[0];
//			return TRUE;
//		};
//
//
//
//	
//	}
//	return FALSE;
//}
