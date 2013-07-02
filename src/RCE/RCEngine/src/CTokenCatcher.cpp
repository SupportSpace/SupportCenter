/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CTokenCatcher.h
///
///  Catches tokens within streams
///
///  @author "Archer Software" Sogin M. @date 16.10.2006
///
////////////////////////////////////////////////////////////////////////
#include "ctokencatcher.h"
#include <AidLib/Logging/cLog.h>
#include <AidLib/CException/CException.h>

CTokenCatcher::CTokenCatcher(const char* token, const unsigned int tokenLen)
	:	m_token(new char[tokenLen]), 
		m_tokenLen(tokenLen), 
		m_bufferLen(tokenLen * 2),
		m_buffer(new char[tokenLen * 2]), 
		m_pos(0)
{
TRY_CATCH
	if (!tokenLen) throw MCException("tokenLen == 0");
	if (!token) throw MCException("token == NULL");
	memcpy(m_token.get(),token,m_tokenLen);
CATCH_THROW("CTokenCatcher::CTokenCatcher")
}

CTokenCatcher::~CTokenCatcher(void)
{
TRY_CATCH
CATCH_LOG("CTokenCatcher::~CTokenCatcher")
}

bool CTokenCatcher::MyMemcmp(char* buf1, char* buf2, unsigned int len)
{
TRY_CATCH
	for(unsigned int i=0; i<len; ++i)
		if (buf1[i] != buf2[i]) return false;
	return true;
CATCH_THROW("_mymemcmp")
}

/// Send buffer from stream input
/// @param buf buffer to put
/// @param length length of buffer
/// @return true if token found in overall input sequence
bool CTokenCatcher::Send(const char* buf, const unsigned int length)
{
TRY_CATCH
	if (length > m_tokenLen*2)
		throw MCException("length > m_tokenLen*2 for this moment not supported");
	
	//Moving left internal buffer if we need
	if (m_pos + length >= m_bufferLen)
	{
		unsigned int delta = m_pos + length - m_bufferLen;
		memmove(m_buffer.get(), m_buffer.get() + delta, m_pos - delta);
		memcpy(m_buffer.get() + (m_pos - delta), buf, length);
		m_pos = m_bufferLen;
	} else
	{
		//Inserting new data
		memcpy(m_buffer.get() + m_pos, buf, length);
		m_pos += length;
	}
	
	///Searching token
	for(int i=0; i<=static_cast<int>(m_pos-m_tokenLen); ++i)
	{
		if (MyMemcmp(m_buffer.get() + i, m_token.get(), m_tokenLen))
			return true; //match found
	}
	return false; //match not found
CATCH_THROW("CTokenCatcher::Send")
}

void CTokenCatcher::Reset()
{
TRY_CATCH
	m_pos = 0;
CATCH_THROW("CTokenCatcher::Reset")
}