//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CConnectionProviderClient.cpp
///
///  Implements CConnectionProviderClient class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "..\public\CConnectionProviderClient.h"		//TODO: Fix path
#include <FTUI\FTL\CConnectionProviderClient.h>

// CConnectionProviderClient [BEGIN] /////////////////////////////////////////////////////////////////////

int CConnectionProviderClient::OnSubStreamConnected(unsigned int streamId)
{
TRY_CATCH

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CConnectionProviderClient::OnSubStreamDisconnected(unsigned int streamId)
{
TRY_CATCH

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

int CConnectionProviderClient::OnConnectionBroke(boost::shared_ptr<CAbstractStream> carrierStream)
{
TRY_CATCH

	return 0;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CConnectionProviderClient::Execute(void* Params)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CConnectionProviderClient::CConnectionProviderClient(boost::shared_ptr<CAbstractStream> carrierStream)
	:	CSubStreamCacheSplitter(carrierStream)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

CConnectionProviderClient::~CConnectionProviderClient()
{
TRY_CATCH

CATCH_LOG()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<CConnectionProviderClient> CConnectionProviderClient::GetInstance(boost::shared_ptr<CAbstractStream> carrierStream)
{
TRY_CATCH

	boost::shared_ptr<CConnectionProviderClient> cpc;
	return  cpc;

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

void CConnectionProviderClient::InitProvider(unsigned int initializationTimeOut)
{
TRY_CATCH

CATCH_THROW()
}
//--------------------------------------------------------------------------------------------------------

boost::shared_ptr<CSplittedStream> CConnectionProviderClient::RequestOperation(EOperationCode operation, unsigned int connectTimeOut)
{
TRY_CATCH

	boost::shared_ptr<CSplittedStream> ss;
	return  ss;

CATCH_THROW()
}
// CConnectionProviderClient [END] ///////////////////////////////////////////////////////////////////////
