/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CLogBuilder.h
///
///  class for building log object
///
///  @author "Archer Software" Sogin M. @date 02.08.2007
///
////////////////////////////////////////////////////////////////////////

class ISharedObject
{
private:
	int m_refCount;
public:

	ISharedObject()
		: m_refCount(0)
	{
	}

	int AddRef()
	{
		int oldRefCount = m_refCount;
		OnRefCountChanged(oldRefCount, ++m_refCount);
		return m_refCount;
	}

	int Release()
	{
		int oldRefCount = m_refCount;
		OnRefCountChanged(oldRefCount, --m_refCount);
		return m_refCount;
	}
protected:
	virtual void OnRefCountChanged(const int oldRefCount, const int newRefCount) = NULL;
};

/// class for building log object
/// assumed to be used as singleton with same scope as logger singleton
/// (CProcessSingleton in case of RCInstaller)
class CLogBuilder : public ISharedObject
{
protected:
	void OnRefCountChanged(const int oldRefCount, const int newRefCount);
public:
	/// Initializes logging
	CLogBuilder();
	/// Deinitializes logging
	virtual ~CLogBuilder();
};

/// Use this definition to access log builder
#define LOG_BUILDER_INSTANCE CProcessSingleton<CLogBuilder>::instance()
