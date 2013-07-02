//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CConnectionProviderServer.cpp
///
///  Implements CConnectionProviderServer class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "..\public\CConnectionProviderServer.h"		//TODO: Fix path
#include <FTUI\FTL\CConnectionProviderServer.h>

// CConnectionProviderServer [BEGIN] /////////////////////////////////////////////////////////////////////

int CConnectionProviderServer::OnSubStreamConnected(unsigned int streamId)
{
TRY_CATCH

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CConnectionProviderServer::OnSubStreamDisconnected(unsigned int streamId)
{
TRY_CATCH

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CConnectionProviderServer::OnConnectionBroke(boost::shared_ptr<CAbstractStream> carrierStream)
{
TRY_CATCH

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CConnectionProviderServer::Execute(void* Params)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CConnectionProviderServer::CConnectionProviderServer(boost::shared_ptr<CAbstractStream> carrierStream)
	:	CSubStreamCacheSplitter(carrierStream)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CConnectionProviderServer::~CConnectionProviderServer()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

void CConnectionProviderServer::GetInstance(boost::shared_ptr<CAbstractStream> carrierStream)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CConnectionProviderServer::InitProvider(unsigned int initializationTimeOut)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<CSplittedStream> CConnectionProviderServer::EstablishConnection(unsigned int streamId, unsigned int connectTimeOut)
{
TRY_CATCH

	boost::shared_ptr<CSplittedStream> css;
	return  css;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CConnectionProviderServer::SetOnCheckOperationAccessHandler(OnCheckOperationAccessHandler_t handler)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CConnectionProviderServer::SetOnRequestOperationHandler(OnRequestOperationHandler_t handler)
{
TRY_CATCH

CATCH_THROW()
}
// CConnectionProviderServer [END] ///////////////////////////////////////////////////////////////////////
