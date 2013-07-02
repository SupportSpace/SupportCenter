/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CRIdGen.h
///
///  CRIdGen class declaration - generate new request id by call RId() method
///
///  @author Kirill Solovyov @date 12.05.2008
///
////////////////////////////////////////////////////////////////////////
#pragma once
#include "Windows.h"
class CRIdGen
{
private:
	/// inner request id
	unsigned int m_rId;
public:
	CRIdGen(unsigned long initialRId=0):m_rId(initialRId){}
	/// The method genarate new request identifier (rId) value. It's used by requests sending. The method is thread safe.
	/// @return new value of rId
	unsigned long RId(void)
	{
			return InterlockedIncrement(reinterpret_cast<long*>(&m_rId));
	}
};