/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  COutFileStreamTimeStamped.h
///
///  File output stream timestamped
///
///  @author "Archer Software" Sogin M. @date 04.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <NWL/Streaming/CAbstractStream.h>
#include <AidLib/Strings/tstring.h>
#include "SBlock.h"
#include <windows.h>

/// File output stream timestamped
class COutFileStreamTimeStamped : public CAbstractStream
{
private:
	/// handle of a file, where we perform writing
	/// file is opened at construction time and closed in destructor
	HANDLE m_hFile;
	int m_TimeTick;

protected:
	/// Really outputs buffer to file
	/// this method can be redeffined to perform compressed output
	/// @param buf buffer for transfer
	/// @len length of buffer
	virtual void RealPut( const char* buf, const unsigned int &len);

	///  Abstract function to get data from the stream
	///  @param   buffer for data
	///  @param   number of bytes to get
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

	///  Abstract function to put data to stream
	///  @param   buffer with data
	///  @param   number of bytes to put
	virtual unsigned int SendInternal( const char*, const unsigned int& );

	/// Protected constructor for successors
	COutFileStreamTimeStamped() : m_hFile(NULL) {};
public:
	/// initialises object instance
	/// @fileName name of a file to perform output
	COutFileStreamTimeStamped(const tstring &fileName);
	/// dtor
	virtual ~COutFileStreamTimeStamped(void);

	///  Checks data in the stream
	///  @return true if stream has data to read
	virtual bool HasInData();
};
