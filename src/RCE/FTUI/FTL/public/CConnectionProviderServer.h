//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CConnectionProviderServer.h
///
///  Declares CConnectionProviderServer class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FileTransferDef.h"
#include "CSubStreamCacheSplitter.h"
#include "CSplittedStream.h"
//========================================================================================================

typedef boost::function <bool (EOperationCode operationCode)> OnCheckOperationAccessHandler_t;
typedef boost::function <void (unsigned int streamId, EOperationCode operationCode)> OnRequestOperationHandler_t;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CConnectionProviderServer
	:	CSubStreamCacheSplitter,
		CThread
{
	boost::shared_ptr<CSplittedStream> m_serviceStream;
	OnRequestOperationHandler_t m_RequestOperationHandler;
	OnCheckOperationAccessHandler_t m_CheckOperationAccessHandler;

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual int OnSubStreamConnected(unsigned int streamId);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual int OnSubStreamDisconnected(unsigned int streamId);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual int OnConnectionBroke(boost::shared_ptr<CAbstractStream> carrierStream);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual void Execute(void* Params);

protected:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	CConnectionProviderServer(boost::shared_ptr<CAbstractStream> carrierStream);

public:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual ~CConnectionProviderServer();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	static void GetInstance(boost::shared_ptr<CAbstractStream> carrierStream);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void InitProvider(unsigned int initializationTimeOut);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	boost::shared_ptr<CSplittedStream> EstablishConnection(unsigned int streamId, unsigned int connectTimeOut);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void SetOnCheckOperationAccessHandler(OnCheckOperationAccessHandler_t handler);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void SetOnRequestOperationHandler(OnRequestOperationHandler_t handler);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
