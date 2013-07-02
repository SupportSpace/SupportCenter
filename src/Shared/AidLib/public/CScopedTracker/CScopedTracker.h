#pragma once
/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CScopedTracker.h
///
///  Sometimes we need something like boost::shared_ptr, or scoped_ptr 
///  but not for pointers
///  Here is the solution
///
///  @author "Archer Software" Sogin M., Dmitry Netrebenko @date 04.10.2007
///
////////////////////////////////////////////////////////////////////////
#include <boost/function.hpp>

///  Sometimes we need something like boost::shared_ptr, or scoped_ptr 
///  but not for pointers
///  CScopedTracker is the solution for such cases
template<class T> class CScopedTracker
{
private:
	T m_instance;
	boost::function<void(T)> m_deleter;
	CScopedTracker<T>& operator = (const CScopedTracker<T>&);
	CScopedTracker(const CScopedTracker<T>&);

public:
	CScopedTracker(const T &instance, boost::function<void(T)> deleter)
		:	m_instance(instance),
			m_deleter(deleter)
	{
	}

	CScopedTracker()
		:	m_deleter(NULL)
	{
	}

	~CScopedTracker()
	{
		if (NULL != m_deleter)
			m_deleter(m_instance);
	}

	void reset(const T &instance, boost::function<void(T)> deleter)
	{
		if (NULL != m_deleter)
			m_deleter(m_instance);
		m_instance = instance;
		m_deleter = deleter;
	}

	T& get()
	{
		return m_instance;
	}

	operator T()
	{
		return m_instance;
	}
};