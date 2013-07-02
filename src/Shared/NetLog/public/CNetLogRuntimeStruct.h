/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogRuntimeStruct.h
///
///  Declares CNetLogRuntimeStruct class, responsible for serialization 
///    NetLog messages.
///
///  @author Dmitry Netrebenko @date 23.03.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <AidLib/Strings/tstring.h>
#include <windows.h>
#include <boost/shared_ptr.hpp>

///  CNetLogRuntimeStruct class, responsible for serialization 
///    NetLog messages.
class CNetLogRuntimeStruct
{
private:
///  Prevents making copies of CNetLogRuntimeStruct objects
	CNetLogRuntimeStruct( const CNetLogRuntimeStruct& );
	CNetLogRuntimeStruct& operator=( const CNetLogRuntimeStruct& );

///  Shared pointer for char
	typedef boost::shared_ptr<char> SPChar;
///  Shared pointer for VARIANT
	typedef boost::shared_ptr<VARIANT> SPVariant;

	typedef std::map<tstring, SPVariant> LogProperties;
	typedef std::pair<tstring, SPVariant> LogProperty;

public:
///  Constructor
	CNetLogRuntimeStruct();

///  Destructor
	~CNetLogRuntimeStruct();

private:
///  NetLog properties
	LogProperties		m_props;
///  Critical section to access map
	CRITICAL_SECTION	m_section;

public:
///  Set property value
///  @param name - property name
///  @param value - property value
	void SetProperty( const tstring& name, VARIANT value );

///  Obtains property value
///  @param name - property name
///  @retuns property value
	VARIANT GetProperty( const tstring& name );

///  Clears properties map
	void Clear();

///  Encodes properties to buffer
///  @param buffer - output buffer
///  @param size - buffer size
///  @returns result number of bytes
	unsigned int EncodeToBuffer( char* buffer, const unsigned int size );

///  Decodes properties from buffer
///  @param buffer - input buffer
///  @param size - buffer size
	void DecodeFromBuffer( const char* buffer, const unsigned int size );

///  Returns size of encoded properties
	unsigned int GetEncodedSize();

private:
///  Clears variant
///  @param value - pointer to variant
	static void DestroyVariant( VARIANT* value );
};
