//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CStreamMultiplexerBase.h
///
///  Declares a CStreamMultiplexerBase class
///  Creates substreams and provides events for substream's service.
///  As a transport stream uses the CAbstractStream
///  
///  @author Alexander Novak @date 27.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/CSubStreamDispatcher.h>
#include <boost/weak_ptr.hpp>
//========================================================================================================

#define SUBSTREAM_BUFFERSIZE		1024*1024*4		//Default size for receive and send buffer = 4 MB
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class NWL_API CStreamMultiplexerBase
	:	protected CSubStreamDispatcher
{
	friend class boost::weak_ptr<CSubStreamDispatcher>;
	
	/// Pointer to itself for destroying object safely
	boost::weak_ptr<CSubStreamDispatcher> m_dispatcher;

	CStreamMultiplexerBase(const CStreamMultiplexerBase&);
	CStreamMultiplexerBase& operator=(const CStreamMultiplexerBase&);
protected:

	/// Creates CStreamMultiplexerBase with transport stream 
	/// @param transportStream		The transport stream for substreams
	/// @remarks			You mustn't call constructor directly
	///						Get instance through GetInstance method
	CStreamMultiplexerBase(boost::shared_ptr<CAbstractStream> transportStream);
public:

	virtual ~CStreamMultiplexerBase();

	/// Creates instance of the CStreamMultiplexerBase class
	/// @param transportStream		The transport stream for substreams
	template <typename InstanceType>
	static boost::shared_ptr<InstanceType> GetInstance(boost::shared_ptr<CAbstractStream> transportStream);

	/// Create new substream
	/// @param serviceID			Service identifier
	/// @param priorityLevel		Priority level for the substream
	/// @param sizeBuffer			Size of buffer for the substream
	/// @return				New substream
	/// @remarks			Throw an exception if substream hasn't been created
	boost::shared_ptr<CAbstractStream> GetSubStream(unsigned int serviceID,
													unsigned int priorityLevel,
													unsigned int sizeBuffer = SUBSTREAM_BUFFERSIZE);

	/// Called if substream was created on the other side
	/// @param serviceID			Service identifier
	virtual int OnSubStreamConnected(unsigned int serviceID);

	/// Called if substream was deleted on the other side
	/// @param serviceID			Service identifier
	virtual int OnSubStreamDisconnected(unsigned int serviceID);

	/// Called if transport stream was broken
	/// @param transportStream		Thr broken transport stream
	virtual int OnConnectionBroke(boost::shared_ptr<CAbstractStream> transportStream);
};
//--------------------------------------------------------------------------------------------------------

template <typename InstanceType>
boost::shared_ptr<InstanceType> CStreamMultiplexerBase::GetInstance(boost::shared_ptr<CAbstractStream> transportStream)
{
	boost::shared_ptr<InstanceType> pointerToItSelf(new InstanceType(transportStream));
	
	pointerToItSelf->m_dispatcher = pointerToItSelf;
	
	return pointerToItSelf;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
