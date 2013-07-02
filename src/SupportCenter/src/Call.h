// AlertDlg.h : header file
//
//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CCall
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CCall :	CCall	class contains call data required to perform action like:
//					PickUp, Reply, Forward, Snooze
//
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Anatoly Gutnick
// Date    : 10/29/2006 05:21:10 PM
// Comments: First Issue
//===========================================================================
#pragma once
#include "AidLib\Strings\tstring.h"

typedef enum _CallType
{
	CustomerDirectCall = 0,
	ConsultCall = 1,
	OfflineMsg = 2,
	SystemMsg = 3,
	InfoMsg = 4,

}eCallType;

class CCall
{
public:
	CCall(void);
	~CCall(void);

	void		 setUid(long lUid){ m_lUid = lUid; };
	long		 getUid(){ return m_lUid; };
	void		 setCallType(eCallType callType){m_CallType = callType;};

	void		 setSupporterId(tstring sSupporterId){m_sSupporterId = sSupporterId;};

	void		 setCustomerDislpayName(CString sDisplayName){ m_sDisplayName =  sDisplayName;};
	tstring		 getCustomerDislpayName(){return m_sDisplayName;} ;

	tstring		 getSupporterId(){return m_sSupporterId;};
	eCallType	 getCallType(){return m_CallType;};

	long		 getWorkflowID(){return m_WorkflowID;};
	void		 setWorkflowID(long lWorkflowID){m_WorkflowID = lWorkflowID;};

	

private:
	long		 m_lUid;				
	tstring		 m_sSupporterId;
	eCallType	 m_CallType;
	long		 m_WorkflowID;
	tstring		 m_sDisplayName;
};
