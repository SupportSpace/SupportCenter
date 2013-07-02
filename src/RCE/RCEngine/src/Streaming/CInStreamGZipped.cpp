/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInStreamGZipped.cpp
///
///  Input time stamped gzipped stream
///
///  @author "Archer Software" Sogin M. @date 18.10.2006
///
////////////////////////////////////////////////////////////////////////
#include "cinstreamgzipped.h"
#include <zlib/zlib.h>

CInStreamGZipped::CInStreamGZipped(const tstring &fileName)
	: CInFileStreamTimeStamped()
{
TRY_CATCH
	m_file = gzopen(fileName.c_str(),"rb9");
	if (!m_file)
	{
		throw MCStreamException_Win("Failed to gzopen",GetLastError());
	}
CATCH_THROW("CInStreamGZipped::CInStreamGZipped")
}

CInStreamGZipped::~CInStreamGZipped(void)
{
TRY_CATCH
	if (m_file)
		gzclose(m_file);
CATCH_LOG("CInStreamGZipped::~CInStreamGZipped")
}

void CInStreamGZipped::RealGet( char* buf, const unsigned int &len)
{
TRY_CATCH
	if (gzread(m_file, buf, len)!=len)
		throw MCStreamException_Win("failed to gzread",GetLastError());
CATCH_THROW("CInStreamGZipped::RealGet")
}

void CInStreamGZipped::ResetFilePos()
{
TRY_CATCH
	if (gzrewind(m_file) == -1)
		throw MCStreamException_Win("failed to gzseek",GetLastError());
CATCH_THROW("CInStreamGZipped::ResetFilePos")
}