/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetLogRuntimeStruct.cpp
///
///  Implements CNetLogRuntimeStruct class, responsible for serialization 
///    NetLog messages.
///
///  @author Dmitry Netrebenko @date 23.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <NetLog/CNetLogRuntimeStruct.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <AidLib/Logging/CLogVariant.h>
#include <NWL/Streaming/CStreamException.h>

CNetLogRuntimeStruct::CNetLogRuntimeStruct()
{
	// Initialize critical section
	InitializeCriticalSection(&m_section);
}

CNetLogRuntimeStruct::~CNetLogRuntimeStruct()
{
	// Remove critical section
	DeleteCriticalSection(&m_section);
}

void CNetLogRuntimeStruct::SetProperty(const tstring& name, VARIANT value)
{
	// Enter critical section
	CCritSection section(&m_section);

	// Create new VARIANT and copy
	SPVariant spVar(new VARIANT(), DestroyVariant);
	VariantInit(spVar.get());
	VariantCopy(spVar.get(), &value);

	// Add property
	m_props[name] = spVar;
}

VARIANT CNetLogRuntimeStruct::GetProperty(const tstring& name)
{
	// Enter critical section
	CCritSection section(&m_section);
	LogProperties::iterator prop = m_props.find(name);
	VARIANT vt;
	VariantInit(&vt);
	if (prop == m_props.end())
	{
		vt.bstrVal = NULL;
	}
	else
	{
		// Copy variant
		VariantCopy(&vt, prop->second.get());
	}
	return vt;
}

void CNetLogRuntimeStruct::Clear()
{
	// Enter critical section
	CCritSection section(&m_section);
	// Remove all properties
	m_props.clear();
}

unsigned int CNetLogRuntimeStruct::EncodeToBuffer(char* buffer, const unsigned int size)
{
	// Enter critical section
	CCritSection section(&m_section);

	// Get output data size
	unsigned int sz = GetEncodedSize();

	if(sz <= size)
	{
		// Encode properties
		LogProperties::iterator index;
		char* current = buffer;
		for(index = m_props.begin(); index != m_props.end(); ++index)
		{
			memcpy(current, index->first.c_str(), index->first.length());
			current += index->first.length();
			*current = '\0';
			current++;
			CLogVariant logVariant(index->second.get());
			VARTYPE vt = logVariant.GetType();
			memcpy(current, &vt, sizeof(VARTYPE));
			current += sizeof(VARTYPE);
			unsigned int v_size = logVariant.GetSize();
			memcpy(current, &v_size, sizeof(unsigned int));
			current += sizeof(unsigned int);
			char* buf = logVariant.GetBuffer();
			memcpy(current, buf, v_size);
			current += v_size;
		}
		return sz;
	}
	else
		throw MCStreamException(_T("Buffer too small for encoding."));
}

void CNetLogRuntimeStruct::DecodeFromBuffer(const char* buffer, const unsigned int size)
{
	// Enter critical section
	CCritSection section(&m_section);

	// Remove all properties
	m_props.clear();

	// Decode properties
	char* current = const_cast<char*>(buffer);
	while(current < buffer + size)
	{
		tstring name(current);
		current += static_cast<unsigned int>(name.length()) + 1;
		VARTYPE vt;
		memcpy(&vt, current, sizeof(VARTYPE));
		current += sizeof(VARTYPE);
		unsigned int v_size;
		memcpy(&v_size, current, sizeof(unsigned int));
		current += sizeof(unsigned int);
		SPChar buf(new char[v_size]);
		memcpy(buf.get(), current, v_size);
		current += v_size;
		CLogVariant logVariant(buf.get(), v_size, vt);
		SPVariant spVariant(new VARIANT(), DestroyVariant);
		VariantInit(spVariant.get());
		VariantCopy(spVariant.get(), const_cast<VARIANT*>(logVariant.Variant()));
		m_props[name] = spVariant;
	}
}

unsigned int CNetLogRuntimeStruct::GetEncodedSize()
{
	// Enter critical section
	CCritSection section(&m_section);

	// Calculate encoded properties size
	int sz = 0;
	LogProperties::iterator index;
	for(index = m_props.begin(); index != m_props.end(); ++index)
	{
		sz += static_cast<unsigned int>(index->first.length());
		sz++;
		sz += sizeof(VARTYPE);
		sz += sizeof(unsigned int);
		CLogVariant logVariant(index->second.get());
		sz += logVariant.GetSize();
	}
	return sz;
}

void CNetLogRuntimeStruct::DestroyVariant(VARIANT* value)
{
	if(value)
	{
		VariantClear(value);
		delete value;
	}
}
