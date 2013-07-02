/////////////////////////////////////////////////////////////////////////
///
///  CServerThread.h
///
///  Thread which is started by CFileAccessServer and which manages all
///  events from  CFileAccessClient
///
///  @author Dmiry S. Golub @date 12/21/2006
///
////////////////////////////////////////////////////////////////////////

#if !defined(EA_77480349_DE44_4c04_80F6_3F19E774D414__INCLUDED_)
#define EA_77480349_DE44_4c04_80F6_3F19E774D414__INCLUDED_

#include "CFileAccessServer.h"
#include <NWL/Streaming/CAbstractStream.h>
#include <map>
#include "filetransferdata.h"
#include "rfb.h"
//#include <boost/shared_ptr.hpp>
//#include <boost/type_traits/remove_pointer.hpp>



///  Thread which is started by CFileAccessServer and which manages all
///  events from  CFileAccessClient
class CServerThread
{
public:
	/// type definition 
	typedef std::map<unsigned,void(CServerThread::*)(const rfbFileTransferMsg&)> THandlers;
	///typedef boost::shared_ptr< boost::remove_pointer<HANDLE>::type> spFileHandle;
public:
	/// .ctor
	CServerThread(CAbstractStream* stream, CFileAccessServer* server);
	/// returns  power of the 2
	static int powOf2( int );
	/// .dtor
	virtual ~CServerThread();
protected:
	/// Handles content operation request 	
	void DirContentRequst(const rfbFileTransferMsg&);
	/// Handles end of the file operation 
	void EndOfFile(const rfbFileTransferMsg&);
	/// deprecated
	void FileChecksums(const rfbFileTransferMsg&);
	/// Handles file header request
	void FileHeader(const rfbFileTransferMsg&);
	/// Handles file packet event
	void FilePacket(const rfbFileTransferMsg&);
	/// Handles file offer event
	void FileTransferOffer(const rfbFileTransferMsg&);
	/// Handles content operation request 	
	void FileTransferRequest(const rfbFileTransferMsg&);
	/// Handles check permission request
	bool CheckPermissions(const rfbFileTransferMsg& );
	/// 
	void ClientCommandHanler(const rfbFileTransferMsg& msg);
	/// abort transfer operation
	void AbortTranserOperation( const rfbFileTransferMsg& msg );
	/// thread entry function
	static  unsigned __stdcall ThreadEntry(void*);
private:
	/// map which consists all events handlers
	THandlers m_handlerMap;
	/// pointer to network stream
	CAbstractStream* m_pStream;
	/// pointer to owner class
	CFileAccessServer* m_pServer;
	/// handle for the read/write file
	SPHandle m_hFile;
	/// thread handle
	///boost::shared_ptr<boost::remove_pointer<HANDLE>::type> m_thHandle;
	SPHandle m_thHandle;
	/// stop thread class
	bool m_fStopThread;
	/// critical section
	CRITICAL_SECTION m_cs;
	/// flag which shows zipped or not file
	bool m_zippedFile;
	/// open file name
	tstring m_openedFileName;
	tstring m_zippedFileName;
};
#endif // !defined(EA_77480349_DE44_4c04_80F6_3F19E774D414__INCLUDED_)
