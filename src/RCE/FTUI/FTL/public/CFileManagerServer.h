//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileManagerServer.h
///
///  Declares CFileManagerServer class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/threadpool.hpp>
#include "CConnectionProviderServer.h"
//========================================================================================================

enum EAccessToken
{
	atNoAccess	= 0x00,
	atList		= 0x01,
	atRead		= 0x02,
	atWrite		= 0x04
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileManagerServer
{
	boost::threadpool::pool m_threadPool;
	boost::shared_ptr<CConnectionProviderServer> m_connectionProvider;

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	bool CheckOperationAccessHandler(EOperationCode operationCode);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void RequestOperationHandler(unsigned int streamId, EOperationCode operationCode);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void CreateDirectory();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void CreateUniqueDirectory();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void ListDrives();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void ListFiles();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void RenameFile();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void SendFile();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void ReceiveFile();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void DeleteFile();

public:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	CFileManagerServer(EAccessToken accessMode = atNoAccess);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual ~CFileManagerServer();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void SetAccessMode(EAccessToken accessMode);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void StartServer(boost::shared_ptr<CAbstractStream> stream);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
