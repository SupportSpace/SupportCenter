/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogVariant.h
///
///  Declares CLogVariant class, additional class for serialization 
///    NetLog messages.
///
///  @author Dmitry Netrebenko @date 23.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <AidLib/Strings/tstring.h>
#include <AidLib/CTime/CTime.h>
#include <boost/shared_ptr.hpp>
#include <AidLib/AidLib.h>

///  CLogVariant class, additional class for serialization 
///    NetLog messages.
class AIDLIB_API CLogVariant
{
private:
///  Prevents making copies of CLogVariant objects
	CLogVariant( const CLogVariant& );
	CLogVariant& operator=( const CLogVariant& );

///  Shared pointer for char
	typedef boost::shared_ptr<char> SPChar;

public:
///  Constructors
	CLogVariant( VARIANT var );
	CLogVariant( VARIANT* var );
	CLogVariant( const void* buffer, const unsigned int size, const VARTYPE type );
	CLogVariant( const int value );
	CLogVariant( const double value );
	CLogVariant( const tstring& value );
	CLogVariant( const bool value );
	CLogVariant( const cDate& value );

///  Destructor
	~CLogVariant();

private:
///  Internal variant
	VARIANT			m_var;
///  Internal buffer
	SPChar			m_buffer;
///  Buffer size
	unsigned int	m_size;

public:
///  Returns pointer to buffer
	char* GetBuffer() const;

///  Returns buffer size
	unsigned int GetSize() const;

///  Returns type of variant
	VARTYPE GetType() const;

///  Returns reference to internal variant
	const VARIANT* Variant() const;

///  Conversion operators
	operator int() const;
	operator double() const;
	operator tstring() const;
	operator bool() const;
	operator cDate() const;

private:
///  Initializes internal structures form pointer to VARIANT
	void InitFromVariantPtr( VARIANT* var );
};
