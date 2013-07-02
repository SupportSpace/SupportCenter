//===========================================================================
// SupportSpace ltd. @{SRCH}
//								CExecutionQueueNode
//
//---------------------------------------------------------------------------
// DESCRIPTION: @{HDES}
// -----------
// CExecutionQueueNode :  class
//---------------------------------------------------------------------------
// CHANGES LOG: @{HREV}
// -----------
// Revision: 01.00
// By      : Alex Gantman
// Date    : 10/02/2007 05:21:10 PM
// Comments: First Issue
// Modified by: Anatoly Gutnick	
//===========================================================================
#pragma once

#include "AidLib\Strings\tstring.h"
#include "JabberWrapper.h"
#include "..\Communicator.h"

class CExecutionQueueNode
{
public:
	CExecutionQueueNode(){};
	virtual ~CExecutionQueueNode(void){};

	virtual void PerformJob(CJabberWrapper<class CCommunicator>& wrapper) = 0;

protected:
	void connect(CJabberWrapper<class CCommunicator>& wrapper) {wrapper.connect(); }
	void disconnect(CJabberWrapper<class CCommunicator>& wrapper) {wrapper.disconnect(); }
	void update_status(CJabberWrapper<class CCommunicator>& wrapper,Presence  status, const tstring& msg) {wrapper.update_status(status, msg); }
	void send_msg(CJabberWrapper<class CCommunicator>& wrapper,tstring m_sTo,tstring m_sMessageBody,tstring m_sMessageSubject, tstring m_sToResource) {wrapper.send_msg(m_sTo, m_sMessageBody, m_sMessageSubject, m_sToResource);}
	void send_keepalive(CJabberWrapper<class CCommunicator>& wrapper) {wrapper.send_keepalive(); }

};

class COMMUNICATOR_API CNodeSendMessage : public CExecutionQueueNode
{
public:
	CNodeSendMessage(const tstring& To, const tstring& Subject,const tstring& Body, const tstring& ToResource);
	~CNodeSendMessage(void);

	tstring GetTo() const { return m_sTo;}
	tstring GetSubject() const  { return m_sMessageSubject;}
	tstring GetBody() const  { return m_sMessageBody;}
	tstring GetToResource() const  { return m_sToResource;}

	void SetTo(const tstring& To){ m_sTo =  To;}
	void SetSubject(const tstring& Subject){ m_sMessageSubject =  Subject;} 
	void SetBody(const tstring& Body){ m_sTo =  Body;}
	void SetToResource(const tstring& ToResource){ m_sToResource =  ToResource;}

	virtual	void PerformJob(CJabberWrapper<CCommunicator>& wrapper);

private:
	tstring m_sTo;
	tstring m_sToResource;
	tstring m_sMessageSubject;
	tstring m_sMessageBody;
};

class COMMUNICATOR_API CNodeSendDisconnect : public CExecutionQueueNode
{
public:
	CNodeSendDisconnect(){};
	~CNodeSendDisconnect(void){};

	virtual void PerformJob(CJabberWrapper<CCommunicator>& wrapper)
	{
		Log.Add(_MESSAGE_, _T("CNodeSendDisconnect::PerformJob()"));
		disconnect(wrapper); 
	};
};

class COMMUNICATOR_API CNodeSendConnect : public CExecutionQueueNode
{
public:
	CNodeSendConnect(){};
	virtual ~CNodeSendConnect(void){};

	virtual void PerformJob(CJabberWrapper<CCommunicator>& wrapper)
	{
		Log.Add(_MESSAGE_, _T("CNodeSendConnect::PerformJob()"));
		connect(wrapper);		
	}
};

class COMMUNICATOR_API CNodeUpdateStatus : public CExecutionQueueNode
{
public:
	CNodeUpdateStatus(Presence  status, const tstring& msg = _T("")):m_status(status),m_msg(msg)
	{};
	virtual ~CNodeUpdateStatus(void){};

	virtual void PerformJob(CJabberWrapper<CCommunicator>& wrapper)
	{
		Log.Add(_MESSAGE_, _T("CNodeUpdateStatus::PerformJob()"));
		update_status(wrapper, m_status, m_msg);		
	}
private:
	Presence  m_status;
	tstring	  m_msg;
};

class COMMUNICATOR_API CNodeSendKeepAlive : public CExecutionQueueNode
{
public:
	CNodeSendKeepAlive()
	{};
	virtual ~CNodeSendKeepAlive(void){};

	virtual void PerformJob(CJabberWrapper<CCommunicator>& wrapper)
	{
		Log.Add(_MESSAGE_, _T("CNodeSendKeepAlive::PerformJob()"));
		send_keepalive( wrapper);		
	}
private:
};

