/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CNetworkLog.cpp
///
///  Implements CNetworkLog class, responsible for sending log messages
///    to log server 
///
///  @author Dmitry Netrebenko @date 20.03.2007
///
////////////////////////////////////////////////////////////////////////

#include <NetLog/CNetworkLog.h>
#include <NetLog/SLogMsg.h>
#include <AidLib/CCritSection/CCritSection.h>
#include <boost/bind.hpp>
#include <NetLog/CNetLogRuntimeStruct.h>
#include <AidLib/Logging/CLogVariant.h>
#include <boost/shared_ptr.hpp>
#include "CLogTransportFactory.h"
#include <NWL/Streaming/CStreamException.h>
#include <NWL/Streaming/CFirewallConfigurator.h>
#include <AidLib/CSingleton/CSingleton.h>

#define NETLOG_THREAD_WAIT_TIME	 10000

CNetworkLog::CNetLogCleaner::CNetLogCleaner()
	:	CThread()
{
	InitializeCriticalSection(&m_cs);
}
CNetworkLog::CNetLogCleaner::~CNetLogCleaner()
{
	try
	{
		DeleteCriticalSection(&m_cs);
	}
	catch(...)
	{
	}
}
void CNetworkLog::CNetLogCleaner::AddSvr(SPConnectedLogSvr svr)
{
	try
	{
		CCritSection section(&m_cs);
		m_queue.push(svr);
	}
	catch(...)
	{
	}
}
void CNetworkLog::CNetLogCleaner::Execute(void*)
{
	try
	{
		while (!Terminated())
		{
			SPConnectedLogSvr svr = GetSvr();
			if(svr.get())
			{
				WaitForSingleObject(svr->hTerminatedEvent.get(), NETLOG_THREAD_WAIT_TIME);
			}
			Sleep(1);
		}
	}
	catch(...)
	{
	}
}

SPConnectedLogSvr CNetworkLog::CNetLogCleaner::GetSvr()
{
	try
	{
		CCritSection section(&m_cs);
		if(!m_queue.empty())
		{
			SPConnectedLogSvr svr = m_queue.front();
			m_queue.pop();
			return svr;
		}
	}
	catch(...)
	{
	}
	return SPConnectedLogSvr();
}

CNetworkLog::CNetworkLog(const tstring& name)
	:	m_name(name)
	,	cLog()
	,	m_nextSvrId(0)
	,	m_transportLayer()
	,	m_showPrestoredMessages(NETLOG_DEFAULT_SHOW_PRESTORED_MESSAGES)
	,	m_prestoredMsgCount(NETLOG_PRESTORED_MESSAGES_COUNT)
{
	InitializeCriticalSection(&m_svrSection);
	InitializeCriticalSection(&m_prestoredSection);

	try
	{
		// Configure firewall for incoming connections
		FIREWALL_CONFIGURATOR_INSTANCE.AllowIncoming();
	}
	catch(...)
	{
	}

	// Obtain transport layer from factory
	tstring errorString;
	m_transportLayer = CLogTransportFactory::instance().GetTransportLayer(m_name, *this, errorString);
	if(!m_transportLayer.get())
		throw MCStreamException(Format(_T("Can not obtain transport layer: %s"),errorString.c_str()));

	TCHAR Buf[MAX_PATH];

	// Get flag which indicates using prestored messages from environment variable
	if(GetEnvironmentVariable(NETLOG_SHOW_PRESTORED_MESSAGES_ENV, Buf, MAX_PATH))
	{
		tstring value(Buf);
		if(!value.empty())
		{
			if(value.compare(i2tstring(0)))
				m_showPrestoredMessages = true;
		}
	}

	m_cleaner.Start();
}

CNetworkLog::~CNetworkLog()
{
	try
	{
		m_cleaner.Terminate();
		DeleteCriticalSection(&m_prestoredSection);
		DeleteCriticalSection(&m_svrSection);
	}
	catch(...)
	{
	}
}

void CNetworkLog::AddList(const cEventDesc &EventDesc, const TCHAR *Item, ...)
{
	try
	{
		// Exit if verbosity level is _NO_TRACE_ of high than defined level
		if( (_NO_TRACE_ == EventDesc.Verbosity) || (EventDesc.Verbosity > m_verbosity) )
			return;

		CNetLogRuntimeStruct runtimeStruct;

		// Get severity structure
		sSeverity severity = m_severities[EventDesc.Severity];
		// Get set of information fields for this severity
		unsigned int fields = severity.m_informFields;

		// For each information field
		for(unsigned int i = 0; i<MAX_INFORM_FIELD_INDEX; i++)
		{
			sInformField field = m_informFields[i];
			// Process next field if field not in set
			if(!(field.m_type&fields))
				continue;
			// Get information string for field
			tstring fieldName = field.m_informString;
			tstring fieldValue = _T("");

			VARIANT varValue;
			VariantInit(&varValue);

			// Add params for MESSAGE
			if(_FIELD_MESSAGE == field.m_type)
			{
				va_list vl;
				for(va_start(vl, Item); Item; Item=va_arg(vl, PTCHAR))
					fieldValue += Item;
				va_end(vl);
				if(!fieldValue.empty())
				{
					CLogVariant logVariant(fieldValue);
					VariantCopy(&varValue, const_cast<VARIANT*>(logVariant.Variant()));
				}
			}
			else
			{
				// Get field value
				if(!field.m_getter)
					continue;
				field.m_getter(EventDesc, &varValue);
			}

			if(VT_EMPTY != varValue.vt)
				runtimeStruct.SetProperty(fieldName, varValue);

			VariantClear(&varValue);
		}

		// Calculate size of encoded data
		unsigned int sz = runtimeStruct.GetEncodedSize();

		// Allocate memory for message
		SPLogMsg msg(reinterpret_cast<SLogMsg*>(new char[NETLOG_MESSAGE_HEADER_SIZE + sz]));

		// Encode log message
		sz = runtimeStruct.EncodeToBuffer(reinterpret_cast<char*>(&msg->m_data), sz);

		msg->m_severity = EventDesc.Severity;
		msg->m_verbosity = EventDesc.Verbosity;
		msg->m_size = NETLOG_MESSAGE_HEADER_SIZE + sz;

		// Add messages to prestored list
		if(m_showPrestoredMessages)
			AddMsgToPrestoredList(msg);

		// Enter critical section
		CCritSection section(&m_svrSection);
		LogServers::iterator index;

		// Send log message to all servers
		for(index = m_connectedLogSvrs.begin(); index != m_connectedLogSvrs.end(); ++index)
		{
			if(!index->second->Terminated())
				index->second->AddLogMsg(msg);
		}
	}
	catch(...)
	{
	}
}

void CNetworkLog::OnSvrEvent(int svrId, EConnectedLogSvrEvent svrEvent, void* params)
{
	try
	{
		switch(svrEvent)
		{
		case clseDisconnect:
			DisconnectServer(svrId);
			break;

		case clseChangeVerbosity:
			{
				eVerbosity* verbosity = reinterpret_cast<eVerbosity*>(params);
				Log.SetVerbosity(*verbosity);
			}
			break;

		default:
			;
		}
	}
	catch(...)
	{
	}
}

void CNetworkLog::DisconnectServer(int svrId)
{
	try
	{
		// Enter critical section
		CCritSection section(&m_svrSection);

		// Find end remove log server
		LogServers::iterator index = m_connectedLogSvrs.find(svrId);
		if(index != m_connectedLogSvrs.end())
		{
			index->second->Terminate();
			SPConnectedLogSvr svr = index->second;
			m_connectedLogSvrs.erase(index);
			m_cleaner.AddSvr(svr);
		}
	}
	catch(...)
	{
	}
}

void CNetworkLog::AddConnectedSvr(SPConnectedLogSvr svr)
{
	try
	{
		{
			// Enter critical section
			CCritSection section(&m_svrSection);

			// Add server to map
			m_connectedLogSvrs[svr->GetSvrId()] = svr;
			// Set up handler for server event
			svr->SetLogSvrEvent(boost::bind(&CNetworkLog::OnSvrEvent, this, _1, _2, _3));

			// Send prestored messages to connected server
			if(m_showPrestoredMessages)
				SendPrestoredMsgsToSvr(svr);

			// Start server thread
			svr->Start();
		}

		Log.Add(_MESSAGE_, _T("NetLog server connected."));
	}
	catch(...)
	{
	}
}

int CNetworkLog::GetNextSvrId()
{
	try
	{
		// Enter critical section
		CCritSection section(&m_svrSection);

		int ret = m_nextSvrId;
		// Get next server id
		m_nextSvrId++;
		return ret;
	}
	catch(...)
	{
		return 0;
	}
}

void CNetworkLog::AddMsgToPrestoredList(SPLogMsg msg)
{
	try
	{
		// Enter critical section
		CCritSection section(&m_prestoredSection);

		// Add message to list
		m_prestoredMessages.push_back(msg);

		// Check queue size
		if(m_prestoredMessages.size() > m_prestoredMsgCount)
		{
			// Remove first message
			m_prestoredMessages.pop_front();
		}
	}
	catch(...)
	{
	}
}

void CNetworkLog::SendPrestoredMsgsToSvr(SPConnectedLogSvr svr)
{
	try
	{
		// Enter critical section
		CCritSection section(&m_prestoredSection);

		// Sends all prestored messages to net log server
		MsgList::iterator index;
		for(index = m_prestoredMessages.begin(); index != m_prestoredMessages.end(); ++index)
			svr->AddLogMsg(*index);
	}
	catch(...)
	{
	}
}

