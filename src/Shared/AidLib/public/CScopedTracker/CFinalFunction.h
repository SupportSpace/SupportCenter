/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CFinalFunction.h
///
///  Declares CFinalFunction class, responsible for execution some function
///    at leaving scope
///
///  @author Dmitry Netrebenko @date 30.01.2008
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <NWL/NetworkLayer.h>
#include <boost/function.hpp>

/// CFinalFunction class, responsible for execution some function
///   at leaving scope
class NWL_API CFinalFunction
{
private:
/// Function to execute
	boost::function<void(void)>	m_func;
public:
/// Constructor
/// @param func - function to execute when object will be destroyed
	CFinalFunction(boost::function<void(void)> func)
		:	m_func(func)
	{
	};
/// Destructor
	~CFinalFunction()
	{
		if(NULL != m_func)
			m_func();
	};
};
