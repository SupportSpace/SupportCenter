//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileManagerClient.cpp
///
///  Implements CFileManagerClient class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "..\public\CFileManagerClient.h"		//TODO: Fix path
#include <FTUI\FTL\CFileManagerClient.h>

// CFileManagerClient [BEGIN] ////////////////////////////////////////////////////////////////////////////

CFileManagerClient::CFileManagerClient()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CFileManagerClient::~CFileManagerClient()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::StartClient(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::CreateDirectory(const tstring& directoryName, CFileData* fileData)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::CreateUniqueDirectory(const tstring& parentDirectory, CFileData* fileData)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<std::vector<CFileData>> CFileManagerClient::ListDrives()
{
TRY_CATCH

	boost::shared_ptr<std::vector<CFileData>> ld;
	return  ld;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<std::vector<CFileData>> CFileManagerClient::ListFiles(const tstring& browseDirectory)
{
TRY_CATCH

	boost::shared_ptr<std::vector<CFileData>> lf;
	return  lf;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::RenameFile(const tstring& newRemoteFile, const tstring& oldRemoteFile)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::SendFile(const tstring& localFile, const tstring& remoteFile, void* userData, bool volatile* cancellationFlag)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::ReceiveFile(const tstring& localFile, const tstring& remoteFile, void* userData, bool volatile* cancellationFlag)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::DeleteFile(const tstring& fileName)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::CancelTransferOperations()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::OnSendFile(const tstring& localFile,
									const tstring& remoteFile,
									ETransferStatus* operationStatus,
									void* userData,
									bool volatile* cancellationFlag)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerClient::OnReceiveFile(	const tstring& localFile,
										const tstring& remoteFile,
										ETransferStatus* operationStatus,
										void* userData,
										bool volatile* cancellationFlag)
{
TRY_CATCH

CATCH_THROW()
}
// CFileManagerClient [END] //////////////////////////////////////////////////////////////////////////////
