/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CInFileStreamTimeStamped.h
///
///  File output stream timestamped
///
///  @author "Archer Software" Sogin M. @date 04.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <NWL/Streaming/CAbstractStream.h>
#include "SBlock.h"
#include <windows.h>
#include <AidLib/CThread/CThread.h>
#include <vector>
#include <queue>

/// File output stream timestamped
class CInFileStreamTimeStamped : public CAbstractStream, protected CThread
{
public:
	typedef enum _EStreamState
	{
		STREAM_STOPPED,
		STREAM_RUNNING,
		STREAM_PAUSED
	} EStreamState;
private:
	/// handle of a file, where we perform writing
	/// file is opened at construction time and closed in destructor
	HANDLE m_hFile;
	/// Locking get operations with this critical section
	CRITICAL_SECTION m_cs;
	/// Own critical section for get operation
	CRITICAL_SECTION m_getCs;
	/// EOF indication
	bool m_EOF;
	/// Internal data read event
	HANDLE m_dataReadEvent;

	/// internal queue -  read data
	std::vector<char> m_queue;

	/// can be 1.0 to keep the same delay factor, <1.0 to make it run faster (0 = fastest) and > 1.0 to make it run slower.
	float m_delayFactor;

	/// true if receive operation is cancelled
    bool m_readCancelled;

	/// Throws exception if thread is cancelled
	void inline CheckCancelled()
	{
		if (m_readCancelled) 
		{
			m_readCancelled = false;
			throw MCStreamException(_T("Cancelled"));
		}
	}

protected:

	/// Class invariant
	EStreamState m_state;

	/// Really gets buffer from file
	/// this method can be redeffined to perform compressed input
	/// @param buf buffer for transfer
	/// @len length of buffer
	virtual void RealGet( char* buf, const unsigned int &len);

	/// Reset current position within file
	virtual void ResetFilePos();

	///  Abstract function to get data from the stream
	///  @param   buffer for data
	///  @param   number of bytes to get
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

	///  Abstract function to put data to stream
	///  @param   buffer with data
	///  @param   number of bytes to put
	virtual unsigned int SendInternal( const char*, const unsigned int& );

	/// Thread entry point
	void Execute(void*);
	
	/// Protected constructor for successors
	CInFileStreamTimeStamped();
public:
	/// initialises object instance
	/// @fileName name of a file to perform output
	CInFileStreamTimeStamped(const tstring &fileName);
	/// dtor
	virtual ~CInFileStreamTimeStamped(void);

	/// Checks data in the stream
	/// @return returns amount of available data
	virtual bool HasInData();

	///  Cancel reading from the stream
	///  @remarks
	virtual void CancelReceiveOperation();

	///  Extracts data from input buffer
	///  @param   Pointer to buffer
	///  @param   Buffer size
	///  @return  Number of bytes
	///  @remarks redefining in this class to improve performance
	virtual unsigned int GetInBuffer( char*, const unsigned int& );

	/// Starts stream if it was stopped or paused
	virtual void Start();

	/// Stops stream if it was running or paused
	virtual void Stop();

	/// Pauses stream if it was running
	virtual void Pause();

	/// Set delay factor
	/// @param delayFactor can be 1.0 to keep the same delay factor, <1.0 to make it run faster (0 = fastest) 
	/// and > 1.0 to make it run slower.
	virtual void SetDelayFactor(float delayFactor);
};
