/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInStreamGZipped.h
///
///  Input time stamped gzipped stream
///
///  @author "Archer Software" Sogin M. @date 18.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "cinfilestreamtimestamped.h"

/// Input time stamped gzipped stream
class CInStreamGZipped : public CInFileStreamTimeStamped
{
private:
	//zLib file descriptor; null if there is no file (of gzFile type)
	void* m_file;
public:
	/// initialises object instance
	/// @fileName name of a file to perform output
	CInStreamGZipped(const tstring &fileName);
	/// dtor
	~CInStreamGZipped(void);

	/// Really gets buffer from file
	/// this method can be redeffined to perform compressed input
	/// @param buf buffer for transfer
	/// @len length of buffer
	virtual void RealGet( char* buf, const unsigned int &len);

	/// Reset current position within file
	virtual void ResetFilePos();
};
