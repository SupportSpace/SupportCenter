
#include <NWL/Streaming/CMatchPortMapping.h>

// CMatchPortMapping [BEGIN] /////////////////////////////////////////////////////////////

volatile LONG CMatchPortMapping::m_search_in_process=FALSE;
//----------------------------------------------------------------------------------------

CMatchPortMapping::	CMatchPortMapping(	const tstring& serverAddress,
										unsigned short serverPort,
										int searchDelay,
										int countTry)
	:	m_search_delay(searchDelay),
		m_count_try(countTry),
		m_open_port_request(serverAddress,serverPort)
{
TRY_CATCH
CATCH_THROW()
}
//----------------------------------------------------------------------------------------

CMatchPortMapping::~CMatchPortMapping()
{
TRY_CATCH
CATCH_LOG()
}
//----------------------------------------------------------------------------------------

bool CMatchPortMapping::CreateMappedPorts(	const tstring& userID,
											const tstring& password,
											unsigned short* preferenceExPort,
											unsigned long inAddress,
											unsigned short* preferenceInPort,
											const char* protocol)
{
TRY_CATCH
	if (!preferenceExPort || !preferenceInPort || !protocol)
		throw MCException(_T("Invalid Arguments"));

	Log.Add(_MESSAGE_,_T("Create the port mapping for %s with ExPort: %u and InPort: %u"),
			userID.c_str(),*preferenceExPort,*preferenceInPort);

	//Try to use dicovering exclusively 
	if ( InterlockedCompareExchange(&m_search_in_process,TRUE,FALSE) )
	{
		Log.Add(_MESSAGE_,_T("Discovering in use"));
		return false;
	}
	
	EErrPortMapping searchResult=m_port_mapping.Search(m_search_delay);
	
	//Discovering finished
	InterlockedExchange(&m_search_in_process,FALSE);
	
	switch (searchResult)
	{
		case E_PM_SUCCESS:
			Log.Add(_MESSAGE_,_T("UPnP Router was found"));
			break;

		case E_PM_NOUPNPDEVICES:
			Log.Add(_MESSAGE_,_T("UPnP devices not found in network"));
			throw MCException(_T("UPnP devices not found in network"));

		case E_PM_NOVALIDDEVICES:
			Log.Add(_MESSAGE_,_T("Not valid UPnP device found in network"));
			throw MCException(_T("Not valid UPnP device found in network"));
	}
	//Make local copy
	unsigned short exPort=*preferenceExPort;
	unsigned short inPort=*preferenceInPort;

	//Try for port mapping. Select a next random values for ports if the binding was fail.
	Log.Add(_MESSAGE_,_T("Try to do port mapping on exPort: %u, inPort: %u"),exPort,inPort);

	int iTry=m_count_try;
	while (	m_port_mapping.AddMapping(exPort,inAddress,inPort,protocol)!=E_PM_SUCCESS && 
			iTry > 0 )
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		inPort = (unsigned short)(li.LowPart % UNSIGNED_SHORT_MAX + 1); //Getting the random port number
		
		QueryPerformanceCounter(&li);
		exPort = (unsigned short)(li.LowPart % UNSIGNED_SHORT_MAX + 1);
		
		iTry--;
		Log.Add(_MESSAGE_,_T("Try to do port mapping on exPort: %u, inPort: %u"),exPort,inPort);
	}
	if (!iTry)
	{
		Log.Add(_MESSAGE_,_T("Can't create port mapping after %u attempts"),m_count_try);
		throw MCException(_T("Can't create port mapping"));
	}
	Log.Add(_MESSAGE_,_T("The port mapping was created on exPort: %u, inPort: %u"),exPort,inPort);
	Log.Add(_MESSAGE_,_T("Checking for port availability"));

	m_open_port_request.CheckPortAvailability(userID,password,inPort,exPort);

	Log.Add(_MESSAGE_,_T("The ports are available"));
	//Return actual port numbers
	*preferenceExPort=exPort;
	*preferenceInPort=inPort;
CATCH_THROW()
	return true;
}
//----------------------------------------------------------------------------------------

void CMatchPortMapping::DeleteMappedPorts(	const tstring& userID,
											const tstring& password,
											unsigned short exPort,
											unsigned short inPort,
											const char* protocol)
{
TRY_CATCH
	Log.Add(_MESSAGE_,_T("Delete mapping for %s with ExPort: %u and InPort: %u"),
			userID.c_str(),exPort,inPort);

	m_port_mapping.DeleteMapping(exPort,protocol);

	Log.Add(_MESSAGE_,_T("Checking for port availability"));
	try
	{
		m_open_port_request.CheckPortAvailability(userID,password,inPort,exPort);
	}
	catch(CStreamException&)
	{
		Log.Add(_MESSAGE_,_T("The port mapping for %s with ExPort: %u and InPort: %u was deleted"),
				userID.c_str(),exPort,inPort);
		return;
	}
	throw MCException(_T("Can't delete the port mapping"));
CATCH_THROW()
}
// CMatchPortMapping [END] ///////////////////////////////////////////////////////////////
