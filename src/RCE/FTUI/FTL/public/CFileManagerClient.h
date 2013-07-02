//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CFileManagerClient.h
///
///  Declares CFileManagerClient class
///  XXXXXX
///  
///  @author Alexander Novak @date XX.02.2008
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//#include "d:\MyProg\SupportSpace\RCE\FTUI\src\CFileData.h"		//TODO: Fix path
struct CFileData{};
#include "CConnectionProviderClient.h"
//========================================================================================================

enum ETransferStatus
{
	tsStarted,
	tsAskForOverwrite,
	tsOverwriteApplied,
	tsTransfering,
	tsCanceled,
	tsFinished
};

struct SErrorCode
{
	DWORD m_internalError;
	DWORD m_windowsError;
	SErrorCode(DWORD internalError = 0, DWORD windowsError = 0)
		:	m_internalError(internalError),
			m_windowsError(windowsError){}
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFileManagerClient
{
	boost::shared_ptr<CConnectionProviderClient> m_connectionProvider;
public:
	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	CFileManagerClient();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual ~CFileManagerClient();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void StartClient(boost::shared_ptr<CAbstractStream> stream);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void CreateDirectory(const tstring& directoryName, CFileData* fileData);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void CreateUniqueDirectory(const tstring& parentDirectory, CFileData* fileData);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	boost::shared_ptr<std::vector<CFileData>> ListDrives();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	boost::shared_ptr<std::vector<CFileData>> ListFiles(const tstring& browseDirectory);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void RenameFile(const tstring& newRemoteFile, const tstring& oldRemoteFile);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void SendFile(const tstring& localFile, const tstring& remoteFile, void* userData, bool volatile* cancellationFlag);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void ReceiveFile(const tstring& localFile, const tstring& remoteFile, void* userData, bool volatile* cancellationFlag);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void DeleteFile(const tstring& fileName);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	void CancelTransferOperations();

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual void OnSendFile(const tstring& localFile,
							const tstring& remoteFile,
							ETransferStatus* operationStatus,
							void* userData,
							bool volatile* cancellationFlag);

	/// XXXXXXXXXXXXX
	/// @param xxxxxx				XXXXXX
	/// @return				XXXXXX
	/// @remarks			XXXXXX
	virtual void OnReceiveFile(	const tstring& localFile,
								const tstring& remoteFile,
								ETransferStatus* operationStatus,
								void* userData,
								bool volatile* cancellationFlag);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
