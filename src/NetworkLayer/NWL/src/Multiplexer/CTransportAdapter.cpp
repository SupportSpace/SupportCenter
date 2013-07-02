//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CTransportAdapter.cpp
///
///  Implements a stream adapter for incoming and outgoing datagrams
///  Transport adapter carries out operations on data without padding
///  
///  @author Alexander Novak @date 27.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <NWL/Multiplexer/CTransportAdapter.h>

// CTransportAdapter [BEGIN] /////////////////////////////////////////////////////////////////////////////

CTransportAdapter::CTransportAdapter(boost::shared_ptr<CAbstractStream> transportStream)
	:	m_transportStream(transportStream)
{
}
//--------------------------------------------------------------------------------------------------------

void CTransportAdapter::SendDatagram(const SDatagram* datagram)
{
TRY_CATCH

	if ( datagram->m_dataSize > MAX_DATAGRAM_SIZE )
		throw MCStreamException(_T("Datagram's size is invalid"));

	//Sending datagram
	m_transportStream->Send(reinterpret_cast<const char*>(datagram),
							sizeof(datagram->m_dataSize) + sizeof(datagram->m_serviceID) + datagram->m_dataSize);

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

void CTransportAdapter::ReceiveDatagram(SDatagram* datagram)
{
TRY_CATCH

	// Receiving header
	m_transportStream->Receive(	reinterpret_cast<char*>(datagram),
								sizeof(datagram->m_dataSize) + sizeof(datagram->m_serviceID));

	if ( datagram->m_dataSize > MAX_DATAGRAM_SIZE )
		throw MCStreamException(_T("Datagram's size is invalid"));

	// Receiving body
	m_transportStream->Receive(	reinterpret_cast<char*>(datagram->m_data),
								datagram->m_dataSize);

CATCH_THROW();
}
// CTransportAdapter [END] ///////////////////////////////////////////////////////////////////////////////
