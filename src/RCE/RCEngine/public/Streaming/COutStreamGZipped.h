/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COutStreamGZipped.h
///
///  Output time stamped gzipped stream
///
///  @author "Archer Software" Sogin M. @date 18.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "coutfilestreamtimestamped.h"

/// Output time stamped gzipped stream
class COutStreamGZipped : public COutFileStreamTimeStamped
{
private:
	//zLib file descriptor; null if there is no file (of gzFile type)
	void* m_file;
public:
	/// initialises object instance
	/// @fileName name of a file to perform output
	COutStreamGZipped(const tstring &fileName);

	/// dtor
	virtual ~COutStreamGZipped(void);

	/// Really outputs buffer to file
	/// this method can be redeffined to perform compressed output
	/// @param buf buffer for transfer
	/// @len length of buffer
	virtual void RealPut( const char* buf, const unsigned int &len);
};
