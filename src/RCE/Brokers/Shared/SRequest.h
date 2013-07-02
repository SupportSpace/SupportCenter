/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  SRequest.h
///
///  SRequest structure declaration
///
///  @author Kirill Solovyov @date 29.01.2008
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <AidLib/Strings/tstring.h>

/// request
struct SRequest
{
	tstring m_dstUserId;       /// destination user identifier
	unsigned long m_dstSvcId;  /// destination service identifier
	tstring m_srcUserId;       /// source user identifier
	unsigned long m_srcSvcId;  /// source service identifier
	unsigned long m_rId;       /// request identifier
	unsigned long m_rType;     /// request type
	unsigned long m_param;     /// request parameter
	tstring m_params;          /// packed request parameters

	SRequest(){}
	SRequest(const tstring dstUserId,unsigned long dstSvcId,const tstring srcUserId,unsigned long srcSvcId,unsigned long rId,unsigned long rType,unsigned long param,const tstring params)
	{
		m_dstUserId=dstUserId;
		m_dstSvcId=dstSvcId;
		m_srcUserId=srcUserId;
		m_srcSvcId=srcSvcId;
		m_rId=rId;
		m_rType=rType;
		m_param=param;
		m_params=params;
	}
	SRequest(const SRequest& instance)
	{
		m_dstUserId=instance.m_dstUserId;
		m_dstSvcId=instance.m_dstSvcId;
		m_srcUserId=instance.m_srcUserId;
		m_srcSvcId=instance.m_srcSvcId;
		m_rId=instance.m_rId;
		m_rType=instance.m_rType;
		m_param=instance.m_param;
		m_params=instance.m_params;
	}
	SRequest& operator=(const SRequest& instance)
	{
		m_dstUserId=instance.m_dstUserId;
		m_dstSvcId=instance.m_dstSvcId;
		m_srcUserId=instance.m_srcUserId;
		m_srcSvcId=instance.m_srcSvcId;
		m_rId=instance.m_rId;
		m_rType=instance.m_rType;
		m_param=instance.m_param;
		m_params=instance.m_params;
		return *this;
	}
};

