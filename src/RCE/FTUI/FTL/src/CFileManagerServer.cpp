//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileManagerServer.cpp
///
///  Implements CFileManagerServer class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "..\public\CFileManagerServer.h"		//TODO: Fix path
#include <FTUI\FTL\CFileManagerServer.h>

// CFileManagerServer [BEGIN] ////////////////////////////////////////////////////////////////////////////

bool CFileManagerServer::CheckOperationAccessHandler(EOperationCode operationCode)
{
TRY_CATCH

	return false;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::RequestOperationHandler(unsigned int streamId, EOperationCode operationCode)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::CreateDirectory()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::CreateUniqueDirectory()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::ListDrives()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::ListFiles()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::RenameFile()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::SendFile()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::ReceiveFile()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::DeleteFile()
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CFileManagerServer::CFileManagerServer(EAccessToken accessMode)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CFileManagerServer::~CFileManagerServer()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::SetAccessMode(EAccessToken accessMode)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CFileManagerServer::StartServer(boost::shared_ptr<CAbstractStream> stream)
{
TRY_CATCH

CATCH_THROW()
}
// CFileManagerServer [END] //////////////////////////////////////////////////////////////////////////////
