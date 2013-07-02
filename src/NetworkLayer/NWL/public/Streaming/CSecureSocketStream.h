/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSecureSocketStream.h
///
///  Declares CSecureSocketStream class, socket stream with abstract
///    security connection
///
///  @author Dmitry Netrebenko @date 17.09.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include "CSocketStream.h"
#include "CAbstractSecureStream.h"
#include <NWL/NetworkLayer.h>

///  CSecureSocketStream class, socket stream with abstract
///    security connection
class NWL_API CSecureSocketStream
	:	public CSocketStream
	,	public CAbstractSecureStream
{
private:
/// Prevents making copies of CSecureSocketStream objects.
	CSecureSocketStream( const CSecureSocketStream& );
	CSecureSocketStream& operator=( const CSecureSocketStream& );

public:
///  Constructor
	CSecureSocketStream();

///  Destructor
	virtual ~CSecureSocketStream();
};
