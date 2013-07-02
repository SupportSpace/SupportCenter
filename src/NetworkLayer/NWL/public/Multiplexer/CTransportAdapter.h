//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CTransportAdapter.h
///
///  Declares a stream adapter for incoming and outgoing datagrams
///  Transport adapter carries out operations on data without padding
///  
///  @author Alexander Novak @date 27.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/Multiplexer/SDatagram.h>
#include <NWL/Streaming/CAbstractStream.h>
#include <boost/shared_ptr.hpp>
//////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTransportAdapter
{
	boost::shared_ptr<CAbstractStream> m_transportStream;

	CTransportAdapter(const CTransportAdapter&);
	CTransportAdapter& operator=(const CTransportAdapter&);
public:

	/// Create TransportAdamper with transport stream
	/// @param transportStream		Stream for transport datagrams
	CTransportAdapter(boost::shared_ptr<CAbstractStream> transportStream);

	/// Send datagram into the transport stream
	/// @param datagram				Pointer to datagram for sending
	void SendDatagram(const SDatagram* datagram);

	/// Receive datagram from the transport stream
	/// @param datagram				Pointer to datagram for receive
	void ReceiveDatagram(SDatagram* datagram);

	/// Replace the transport stream to another one
	/// @param transportStream		New transport stream
	void SetTransportStream(boost::shared_ptr<CAbstractStream> transportStream);

	/// Gets currently use transport stream
	/// @return				Transport stream
	boost::shared_ptr<CAbstractStream> GetTransportStream();

	/// Cancel receive operation on transport stream
	void DropTransportReceive();
};
//--------------------------------------------------------------------------------------------------------

inline void CTransportAdapter::SetTransportStream(boost::shared_ptr<CAbstractStream> transportStream)
{
	m_transportStream = transportStream;
}
//--------------------------------------------------------------------------------------------------------

inline boost::shared_ptr<CAbstractStream> CTransportAdapter::GetTransportStream()
{
	return m_transportStream;
}
//--------------------------------------------------------------------------------------------------------

inline void CTransportAdapter::DropTransportReceive()
{
TRY_CATCH

	m_transportStream->CancelReceiveOperation();

CATCH_THROW();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
