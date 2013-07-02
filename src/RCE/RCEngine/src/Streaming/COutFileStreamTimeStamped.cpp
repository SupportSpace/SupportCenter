/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COutFileStreamTimeStamped.cpp
///
///  File output stream timestamped
///
///  @author "Archer Software" Sogin M. @date 04.10.2006
///
////////////////////////////////////////////////////////////////////////

#include "COutFileStreamTimeStamped.h"
#include <AidLib/Logging/CLog.h>
#include "SBlock.h"
#include <memory>

COutFileStreamTimeStamped::COutFileStreamTimeStamped(const tstring &fileName)
	: m_TimeTick(-1)
{
TRY_CATCH

	m_hFile = CreateFile(	fileName.c_str(),
							GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							CREATE_ALWAYS,
							0,
							NULL );
	if (m_hFile == INVALID_HANDLE_VALUE)
		throw MCStreamException_Win("failed to CreateFile",GetLastError());

CATCH_THROW("COutFileStreamTimeStamped::COutFileStreamTimeStamped")
}

COutFileStreamTimeStamped::~COutFileStreamTimeStamped(void)
{
TRY_CATCH
	if (m_hFile)
		CloseHandle(m_hFile);
CATCH_LOG("COutFileStreamTimeStamped::~COutFileStreamTimeStamped")
}

unsigned int COutFileStreamTimeStamped::SendInternal( const char*buf, const unsigned int& len )
{
TRY_CATCH

	DWORD now = timeGetTime();
	if (m_TimeTick > 0)
	{
		//Putting timestamp
		DWORD timeDelta =  now - m_TimeTick;
		std::auto_ptr<SBlock> dataBlock = std::auto_ptr<SBlock>(reinterpret_cast<SBlock*>(new char[BLOCK_HEAD_SIZE + sizeof(DWORD)]));
		dataBlock->type = TIMESTAMP;
		dataBlock->size = BLOCK_HEAD_SIZE + sizeof(DWORD);
		memcpy(dataBlock->buf,&timeDelta,sizeof(DWORD));
		RealPut(reinterpret_cast<char*>(dataBlock.get()), dataBlock->size);
	} 
	m_TimeTick = now;
	
	//Putting data
	std::auto_ptr<SBlock> dataBlock = std::auto_ptr<SBlock>(reinterpret_cast<SBlock*>(new char[BLOCK_HEAD_SIZE + len]));
	dataBlock->type = DATA;
	dataBlock->size = BLOCK_HEAD_SIZE + len;
	memcpy(dataBlock->buf,buf,len);
	RealPut(reinterpret_cast<char*>(dataBlock.get()),dataBlock->size);
	return len;

CATCH_THROW("COutFileStreamTimeStamped::SendInternal")
}

void COutFileStreamTimeStamped::RealPut( const char* buf, const unsigned int &len)
{
TRY_CATCH

	DWORD Written;
	if (!WriteFile(m_hFile, buf, len, &Written, NULL))
		throw MCStreamException_Win("failed to WriteFile",GetLastError());

CATCH_THROW("COutFileStreamTimeStamped::RealPut")
}

unsigned int COutFileStreamTimeStamped::ReceiveInternal( char*, const unsigned int& )
{
TRY_CATCH
	throw MCStreamException("COutFileStreamTimeStamped not support get operation");
CATCH_THROW("COutFileStreamTimeStamped::ReceiveInternal")
}

bool COutFileStreamTimeStamped::HasInData()
{
TRY_CATCH
	throw MCStreamException("COutFileStreamTimeStamped not HasInData");
CATCH_THROW("COutFileStreamTimeStamped::HasInData")
}