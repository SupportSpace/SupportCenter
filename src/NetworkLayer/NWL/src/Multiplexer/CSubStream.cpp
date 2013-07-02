//////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Archer Software
///
///  CSubStream.cpp
///
///  Implements a substream class
///  Provide operations with a stream through CSubStreamDispatcher
///  
///  @author Alexander Novak @date 27.09.2007
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <NWL/Multiplexer/CSubStream.h>

// CSubStream [BEGIN] ////////////////////////////////////////////////////////////////////////////////////

unsigned int CSubStream::SendInternal(const char* data, const unsigned int& size)
{
TRY_CATCH

	m_dispatcher->SendData(m_serviceID, data, size);

	return size;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

unsigned int CSubStream::ReceiveInternal(char* data, const unsigned int& size)
{
TRY_CATCH

	m_dispatcher->ReceiveData(m_serviceID, data, size, false);

	return size;

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

CSubStream::~CSubStream()
{
TRY_CATCH

	m_dispatcher->UnregisterSubStream(m_serviceID);

CATCH_LOG();
}
//--------------------------------------------------------------------------------------------------------

void CSubStream::CancelReceiveOperation()
{
TRY_CATCH

	m_dispatcher->CancelReceiveOperation(m_serviceID);

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

bool CSubStream::HasInData()
{
TRY_CATCH

	return m_dispatcher->IsDataAvailable(m_serviceID);

CATCH_THROW();
}
//--------------------------------------------------------------------------------------------------------

unsigned int CSubStream::GetInBuffer(char* data, const unsigned int& size)
{
TRY_CATCH

	return m_dispatcher->ReceiveData(m_serviceID, data, size, true);

CATCH_THROW();
}
// CSubStream [END] //////////////////////////////////////////////////////////////////////////////////////
