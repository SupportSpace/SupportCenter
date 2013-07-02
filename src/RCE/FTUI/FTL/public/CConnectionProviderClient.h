//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CConnectionProviderClient.h
///
///  Declares CConnectionProviderClient class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FileTransferDef.h"
#include "CSubStreamCacheSplitter.h"
#include "CSplittedStream.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CConnectionProviderClient
	:	CSubStreamCacheSplitter,
		CThread
{
	boost::shared_ptr<CSplittedStream> m_serviceStream;
	unsigned int m_nextStreamId;

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
	CConnectionProviderClient(boost::shared_ptr<CAbstractStream> carrierStream);

public:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual ~CConnectionProviderClient();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	static boost::shared_ptr<CConnectionProviderClient> GetInstance(boost::shared_ptr<CAbstractStream> carrierStream);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void InitProvider(unsigned int initializationTimeOut);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	boost::shared_ptr<CSplittedStream> RequestOperation(EOperationCode operation, unsigned int connectTimeOut);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
