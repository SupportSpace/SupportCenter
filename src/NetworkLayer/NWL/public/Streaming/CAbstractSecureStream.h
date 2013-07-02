/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CAbstractSecureStream.h
///
///  Declares CAbstractSecureStream class, interface for secure connection
///
///  @author Dmitry Netrebenko @date 17.09.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>

///  CAbstractSecureStream class, interface for secure connection
class NWL_API CAbstractSecureStream
{
private:
/// Prevents making copies of CAbstractSecureStream objects.
	CAbstractSecureStream( const CAbstractSecureStream& );
	CAbstractSecureStream& operator=( const CAbstractSecureStream& );

public:
///  Constructor
	CAbstractSecureStream();

///  Destructor
	virtual ~CAbstractSecureStream();

///  Returns true if secure connection is established
	virtual bool HasSecureConnection() const = 0;

protected:
///  Initializes secure connection
///  @param masterRole - stream has master role
	virtual void InitSecureConnection( bool masterRole ) = 0;

};
