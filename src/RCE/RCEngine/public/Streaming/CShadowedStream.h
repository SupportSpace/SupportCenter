/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CShadowedStream.h
///
///  Shadowed stream
///
///  @author "Archer Software" Sogin M. @date 17.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include <NWL/Streaming/CAbstractStream.h>
#include <AidLib/CCritSection/CCritSection.h>
#include "boost/shared_ptr.hpp"


/// Shadowed stream
/// main stream + shadow stream. 
/// All Send operations are duplicated to shadow stream	
class CShadowedStream : public CAbstractStream
{
public:
	/// Type of shadowing
	typedef enum _EShadowType
	{
		INPUT,		/// Shadow stream input
		OUTPUT,		/// Shadow stream output
		BOTH		/// Shadow stream both input and output
	} EShadowType;
private:
	/// main stream
	boost::shared_ptr<CAbstractStream> m_mainStream;
	/// shadow stream
	boost::shared_ptr<CAbstractStream> m_shadowStream;
	/// critical section to perform simple synchronization
	CRITICAL_SECTION m_cs;
	/// type of shadowing (input, output or both)
	EShadowType m_type;
public:
	/// initializes object instance
	/// @param mainStream main stream
	/// @param type type of shadowing (input, output or both)
	CShadowedStream(boost::shared_ptr<CAbstractStream> mainStream, const EShadowType type);
	/// dtor
	virtual ~CShadowedStream(void);

	/// Set new shadow stream
	/// @param shadowStream new shadowStream object
	/// @return old shadow stream object
	/// @remark due to sync. issues method can block until SendInternal method 
	/// of previous ShadowStream object is complete
	boost::shared_ptr<CAbstractStream> SetShadowStream(boost::shared_ptr<CAbstractStream> shadowStream);

	/// Returns main stream
	/// @return main stream pointer
	boost::shared_ptr<CAbstractStream> GetMainStream();

	///  Cancel reading from the stream
	///  @remarks
	virtual void CancelReceiveOperation();

protected:
	///  get data from the stream
	///  @param   buffer for data
	///  @param   number of bytes to get
	virtual unsigned int ReceiveInternal( char*, const unsigned int& );

	///  put data to stream
	///  @param   buffer with data
	///  @param   number of bytes to put
	virtual unsigned int SendInternal( const char*, const unsigned int& );

public:
	///  Extracts data from input buffer
	///  @param   Pointer to buffer
	///  @param   Buffer size
	///  @return  Number of bytes
	virtual unsigned int GetInBuffer( char*, const unsigned int& );

	///  Checks data in the stream
	///  @return true if stream has data to read
	virtual bool HasInData();

	/// Setup critical section ¹1 for some internal issues
	/// @param cs1 pointer to critical section ¹1 structure
	virtual void SetCS1(CRITICAL_SECTION *cs1);
};
