/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogVariant.cpp
///
///  Implements CLogVariant class, additional class for serialization 
///    NetLog messages.
///
///  @author Dmitry Netrebenko @date 23.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <AidLib/Logging/CLogVariant.h>
#include <AidLib/CException/CException.h>
#include <algorithm>
#include <atlbase.h>

void CLogVariant::InitFromVariantPtr(VARIANT* var)
{
	if(!var)
		throw MCException(_T("Variant pointer not initialized."));

	USES_CONVERSION;

	// Init variant
	VariantInit(&m_var);
	// Copy variant
	VariantCopy(&m_var, var);

	// Get variant's value
	switch(m_var.vt)
	{
	case VT_INT:
		m_size = sizeof(int);
		m_buffer.reset(new char[m_size]);
		memcpy(m_buffer.get(), &m_var.intVal, m_size);
		return;
	case VT_R8:
		m_size = sizeof(double);
		m_buffer.reset(new char[m_size]);
		memcpy(m_buffer.get(), &m_var.dblVal, m_size);
		return;
	case VT_BOOL:
		m_size = sizeof(bool);
		m_buffer.reset(new char[m_size]);
		memcpy(m_buffer.get(), &m_var.boolVal, m_size);
		return;
	case VT_DATE:
		m_size = sizeof(SYSTEMTIME);
		m_buffer.reset(new char[m_size]);
		SYSTEMTIME stime;
		if(FALSE == VariantTimeToSystemTime(m_var.date, &stime))
			throw MCException(_T("Can not convert VariantTime to SYSTEMTIME."));
		memcpy(m_buffer.get(), &stime, m_size);
		return;
	case VT_BSTR:
		tstring tmp(_T(""));
		if(NULL != m_var.bstrVal)
			tmp = tstring(OLE2T(m_var.bstrVal));
		m_size = static_cast<unsigned int>(tmp.length());
		m_buffer.reset(new char[m_size]);
		memcpy(m_buffer.get(), tmp.c_str(), m_size);
		return;
	}
	throw MCException(_T("Invalid variant type."));
}

CLogVariant::CLogVariant(VARIANT var)
{
	InitFromVariantPtr(&var);
}

CLogVariant::CLogVariant(VARIANT* var)
{
	InitFromVariantPtr(var);
}

CLogVariant::CLogVariant(const void* buffer, const unsigned int size, const VARTYPE type)
{
	USES_CONVERSION;

	// Init variant
	VariantInit(&m_var);

	// Get variant's value
	m_size = size;
	m_buffer.reset(new char[m_size]);
	memcpy(m_buffer.get(), buffer, m_size);
	m_var.vt = type;
	switch(type)
	{
	case VT_INT:
		memcpy(&m_var.intVal, m_buffer.get(), m_size);
		return;
	case VT_R8:
		memcpy(&m_var.dblVal, m_buffer.get(), m_size);
		return;
	case VT_BOOL:
		bool val;
		memcpy(&val, m_buffer.get(), m_size);
		m_var.boolVal = val;
		return;
	case VT_DATE:
		double vtime;
		SYSTEMTIME stime;
		memcpy(&stime, m_buffer.get(), m_size);
		if(FALSE == SystemTimeToVariantTime(&stime, &vtime))
			throw MCException(_T("Can not convert SYSTEMTIME to VariantTime."));
		m_var.date = vtime;
		return;
	case VT_BSTR:
		SPChar buf(new char[m_size + 1]);
		memset(buf.get(), 0, m_size + 1);
		memcpy(buf.get(), m_buffer.get(), m_size);
		tstring tmp(buf.get());
		m_var.bstrVal = ::SysAllocString( T2OLE( tmp.c_str() ) );
		return;
	}
	throw MCException(_T("Invalid variant type."));
}

CLogVariant::CLogVariant(const int value)
{
	// Init variant
	VariantInit(&m_var);

	// Get variant's value
	m_size = sizeof(int);
	m_buffer.reset(new char[m_size]);
	memcpy(m_buffer.get(), &value, m_size);
	m_var.vt = VT_INT;
	m_var.intVal = value;
}

CLogVariant::CLogVariant(const double value)
{
	// Init variant
	VariantInit(&m_var);
	
	// Get variant's value
	m_size = sizeof(double);
	m_buffer.reset(new char[m_size]);
	memcpy(m_buffer.get(), &value, m_size);
	m_var.vt = VT_R8;
	m_var.dblVal = value;
}

CLogVariant::CLogVariant(const tstring& value)
{
	// Init variant
	VariantInit(&m_var);

	// Get variant's value
	m_size = static_cast<unsigned int>(value.length());
	m_buffer.reset(new char[m_size]);
	memcpy(m_buffer.get(), value.c_str(), m_size);
	m_var.vt = VT_BSTR;

	USES_CONVERSION;
	m_var.bstrVal = ::SysAllocString( T2OLE( value.c_str() ) );
}

CLogVariant::CLogVariant( const bool value )
{
	// Init variant
	VariantInit(&m_var);
	
	// Get variant's value
	m_size = sizeof(bool);
	m_buffer.reset(new char[m_size]);
	memcpy(m_buffer.get(), &value, m_size);
	m_var.vt = VT_BOOL;
	m_var.boolVal = value;
}

CLogVariant::CLogVariant( const cDate& value )
{
	// Init variant
	VariantInit(&m_var);
	
	// Get variant's value
	m_size = sizeof(SYSTEMTIME);
	m_buffer.reset(new char[m_size]);
	
	SYSTEMTIME stime = (SYSTEMTIME)value;
	memcpy(m_buffer.get(), &stime, m_size);

	double vtime;
	if(FALSE == SystemTimeToVariantTime(&stime, &vtime))
		throw MCException(_T("Can not convert SYSTEMTIME to ViriantTime."));

	m_var.vt = VT_DATE;
	m_var.date = vtime;
}

CLogVariant::~CLogVariant()
{
	// Clear variant
	VariantClear(&m_var);
}

char* CLogVariant::GetBuffer() const
{
	return m_buffer.get();
}

unsigned int CLogVariant::GetSize() const
{
	return m_size;
}

VARTYPE CLogVariant::GetType() const
{
	return m_var.vt;
}

const VARIANT* CLogVariant::Variant() const
{
	return &m_var;
}

CLogVariant::operator int() const
{
	int ret;
	memset(&ret, 0, sizeof(int));
	unsigned int sz = std::min<unsigned int>(sizeof(int), m_size);
	memcpy(&ret, m_buffer.get(), sz);
	return ret;
}

CLogVariant::operator double() const
{
	double ret;
	memset(&ret, 0, sizeof(double));
	unsigned int sz = std::min<unsigned int>(sizeof(double), m_size);
	memcpy(&ret, m_buffer.get(), sz);
	return ret;
}

CLogVariant::operator tstring() const
{
	SPChar buf(new char[m_size + 1]);
	memset(buf.get(), 0, m_size + 1);
	memcpy(buf.get(), m_buffer.get(), m_size);
	tstring ret(buf.get());
	return ret;
}

CLogVariant::operator bool() const
{
	bool ret;
	memset(&ret, 0, sizeof(bool));
	unsigned int sz = std::min<unsigned int>(sizeof(bool), m_size);
	memcpy(&ret, m_buffer.get(), sz);
	return ret;
}

CLogVariant::operator cDate() const
{
	SYSTEMTIME ret;
	memset(&ret, 0, sizeof(SYSTEMTIME));
	unsigned int sz = std::min<unsigned int>(sizeof(SYSTEMTIME), m_size);
	memcpy(&ret, m_buffer.get(), sz);
	return cDate(ret);
}

