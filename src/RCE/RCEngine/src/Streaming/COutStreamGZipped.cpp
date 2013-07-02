/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COutStreamGZipped.cpp
///
///  Output time stamped gzipped stream
///
///  @author "Archer Software" Sogin M. @date 18.10.2006
///
////////////////////////////////////////////////////////////////////////
#include "coutstreamgzipped.h"
#include <AidLib/Logging/cLog.h>
#include <zlib/zlib.h>

COutStreamGZipped::COutStreamGZipped(const tstring &fileName)
	: COutFileStreamTimeStamped(), m_file(NULL)
{
TRY_CATCH

	m_file = gzopen(fileName.c_str(),"wb9");
	if (!m_file)
	{
		throw MCException_Win("Failed to gzopen");
	}

CATCH_THROW("COutStreamGZipped::COutStreamGZipped")
}

COutStreamGZipped::~COutStreamGZipped(void)
{
TRY_CATCH
	if (m_file)
		gzclose(m_file);
CATCH_LOG("COutStreamGZipped::~COutStreamGZipped")
}

void COutStreamGZipped::RealPut( const char* buf, const unsigned int &len)
{
TRY_CATCH
	if (gzwrite(m_file, buf, len)!=len)
		throw MCStreamException_Win("failed to gzwrite",GetLastError());
CATCH_THROW("COutStreamGZipped::RealPut")
}
