/////////////////////////////////////////////////////////////////////////
///
///  SupportSpace Ltd.
///
///  plugin.h
///
///  nsPluginInstance object implementation. The object is main plugin instance
///
///  @author Anatoly Gutnick @date 16.04.2008
///
////////////////////////////////////////////////////////////////////////
[module(name="x")];

#define DEF_FILE_LOG_NAME _T("SupportSpaceToolsProxyFFplugin.log")

#include "plugin.h"
#include "nsIServiceManager.h"
#include "nsIMemory.h"
#include "nsISupportsUtils.h"		// this is where some useful macros defined

#include "nsComponentManagerUtils.h" // this is where some useful macros defined
#include <windowsx.h>

#include <AidLib/Logging/CLogFolder.h>	

// service manager which will give the access to all public browser services
// we will use memory service as an illustration
nsIServiceManager * gServiceManager = NULL;


static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);
static WNDPROC lpOldProc = NULL;


// Unix needs this
#ifdef XP_UNIX
#define MIME_TYPES_HANDLED  "application/s2-f2"
#define PLUGIN_NAME         "Simple Plugin Example for Mozilla"
#define MIME_TYPES_DESCRIPTION  MIME_TYPES_HANDLED"::"PLUGIN_NAME
#define PLUGIN_DESCRIPTION  PLUGIN_NAME " (Plug-ins SDK sample)" 

char* NPP_GetMIMEDescription(void)
{
    return(MIME_TYPES_DESCRIPTION);
}

// get values per plugin
NPError NS_PluginGetValue(NPPVariable aVariable, void *aValue)
{
  NPError err = NPERR_NO_ERROR;
  switch (aVariable) {
    case NPPVpluginNameString:
      *((char **)aValue) = PLUGIN_NAME;
      break;
    case NPPVpluginDescriptionString:
      *((char **)aValue) = PLUGIN_DESCRIPTION;
      break;
    default:
      err = NPERR_INVALID_PARAM;
      break;
  }
  return err;
}
#endif //XP_UNIX

//////////////////////////////////////
//
// general initialization and shutdown
//
NPError NS_PluginInitialize()
{
  Log.RegisterLog(new cFileLog(Format(_T("%s\\%s"),LOGS_FOLDER_INSTANCE.GetLogsFolderName().c_str(),DEF_FILE_LOG_NAME).c_str()));
  // Reporting module initialized (before we've registered this as logger

  // this is probably a good place to get the service manager
  // note that Mozilla will add reference, so do not forget to release
  nsISupports * sm = NULL;
    
  NPN_GetValue(NULL, NPNVserviceManager, &sm);

  // Mozilla returns nsIServiceManager so we can use it directly; doing QI on
  // nsISupports here can still be more appropriate in case something is changed 
  // in the future so we don't need to do casting of any sort.
  if(sm) {
    sm->QueryInterface(NS_GET_IID(nsIServiceManager), (void**)&gServiceManager);
    NS_RELEASE(sm);
  }
  
  return NPERR_NO_ERROR;
}

void NS_PluginShutdown()
{
TRY_CATCH
  // we should release the service manager
  NS_IF_RELEASE(gServiceManager);
  gServiceManager = NULL;
CATCH_LOG(_T("NS_PluginShutdown"))
}

/////////////////////////////////////////////////////////////
//
// construction and destruction of our plugin instance object
//
nsPluginInstanceBase * NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct)
{
	nsPluginInstance * plugin = NULL;
TRY_CATCH
  if(!aCreateDataStruct)
    return NULL;

	plugin = new nsPluginInstance(aCreateDataStruct->instance);
CATCH_LOG(_T("NS_NewPluginInstance"))
  return plugin;
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
TRY_CATCH
  if(aPlugin)
    delete (nsPluginInstance *)aPlugin;
CATCH_LOG(_T("NS_DestroyPluginInstance"))
}

////////////////////////////////////////
//
// nsPluginInstance class implementation
//
nsPluginInstance::nsPluginInstance(NPP aInstance) : nsPluginInstanceBase(),
  mInstance(aInstance),
  mInitialized(FALSE),
  mScriptablePeer(NULL)
{
TRY_CATCH
CATCH_LOG(_T("nsPluginInstance::nsPluginInstance"))
}

nsPluginInstance::~nsPluginInstance()
{
	// mScriptablePeer may be also held by the browser 
	// so releasing it here does not guarantee that it is over
	// we should take precaution in case it will be called later
	// and zero its mPlugin member
	TRY_CATCH
		mScriptablePeer->SetInstance(NULL);
	NS_IF_RELEASE(mScriptablePeer);
	CATCH_LOG(_T("nsPluginInstance::~nsPluginInstance"))
}

NPBool nsPluginInstance::init(NPWindow* aWindow)
{
TRY_CATCH

	if(aWindow == NULL){
		Log.Add(_ERROR_, _T("nsPluginInstance::init 1.1 failed. aWindow == NULL"));
		return FALSE;
	}

	HRESULT result;
	CoInitialize(NULL);
	Log.Add(_MESSAGE_, _T("nsPluginInstance::init 2. CoInitialize passed. (m_hWnd=0x%x)"),(HWND)aWindow->window);

	// seems we not need window at this step 
	// subclass window so we can intercept window messages do our things on it
	m_hWnd = (HWND)aWindow->window; 
	// lpOldProc = SubclassWindow(m_hWnd, (WNDPROC)PluginWinProc);
	// lpOldProc = (WNDPROC)SetWindowLongPtr( mhWnd, WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE, (LPARAM)(WNDPROC)PluginWinProc );
	// associate window with our nsPluginInstance object so we can access 
	// it in the window procedure
	// SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);

	if((result=m_broker.CoCreateInstance(L"BrokerProxy.CoBrokerProxy",NULL,CLSCTX_INPROC_SERVER))!=S_OK)
	{
		Log.Add(_ERROR_, _T("CoCreateInstance BrokerProxy.CoBrokerProxy failed"));
		return FALSE;
	}

	Log.Add(_MESSAGE_, _T("nsPluginInstance::init 3. CoCreateInstance passed"));

	// event subscribing
	if((result=m_broker.QueryInterface(&m_pBrokerUnk))!=S_OK)
		Log.Add(_ERROR_, _T("IUnknown interface obtaining of broker failed 0x%x"),result);

	Log.Add(_MESSAGE_, _T("nsPluginInstance::init 4. QueryInterface passed"));

	m_coBrokerEvents.m_owner = this;
	m_dwCustCookie = 0;

	if(S_OK!=(result=AtlAdvise(m_pBrokerUnk,&m_coBrokerEvents,__uuidof(_ICoBrokerProxyEvents),&m_dwCustCookie)))
		Log.Add(_ERROR_, _T("AtlAdvise failed result:0x%x, cookie:%d"),result,m_dwCustCookie);

	Log.Add(_MESSAGE_, _T("nsPluginInstance::init 5. AtlAdvise passed"));
	mInitialized = TRUE;

CATCH_LOG(_T("nsPluginInstance::init"))
	return TRUE;
}

void nsPluginInstance::shut()
{
TRY_CATCH
	
	Log.Add(_MESSAGE_, _T("nsPluginInstance::shut 1"));
	mInitialized = FALSE;
	m_hWnd = NULL;

	HRESULT result;
	if(S_OK!=(result=AtlUnadvise(m_pBrokerUnk,__uuidof(_ICoBrokerProxyEvents),m_dwCustCookie)))
		Log.Add(_ERROR_, _T("AtlUnadvise failed 0x%x"),result);

	Log.Add(_MESSAGE_, _T("nsPluginInstance::shut 2. AtlUnadvise passed"));
	m_pBrokerUnk.Release();

	Log.Add(_MESSAGE_, _T("nsPluginInstance::shut 3. m_pBrokerUnk.Release passed"));
	m_broker.Release();

	Log.Add(_MESSAGE_, _T("nsPluginInstance::shut 4. m_broker.Release() passed"));
	CoUninitialize();	

	Log.Add(_MESSAGE_, _T("nsPluginInstance::shut 5. CoUninitialize passed"));

CATCH_LOG(_T("nsPluginInstance::shut"))
}

NPBool nsPluginInstance::isInitialized()
{
  return mInitialized;
}

void nsPluginInstance::getVersion(char* *aVersion)
{
  const char *ua = NPN_UserAgent(mInstance);
  char*& version = *aVersion;

// http://www.mozilla.org/projects/plugins/bi-directional-plugin-scripting.html
  NPN_GetURL(mInstance, "javascript:callbackTest('hello world of callbacks','param2')", NULL);
  
  // although we can use NPAPI NPN_MemAlloc call to allocate memory:
  //    version = (char*)NPN_MemAlloc(strlen(ua) + 1);
  // for illustration purposed we use the service manager to access 
  // the memory service provided by Mozilla
  nsIMemory * nsMemoryService = NULL;
  
  if (gServiceManager) {
    // get service using its contract id and use it to allocate the memory
    gServiceManager->GetServiceByContractID("@mozilla.org/xpcom/memory-service;1", NS_GET_IID(nsIMemory), (void **)&nsMemoryService);
    if(nsMemoryService)
      version = (char *)nsMemoryService->Alloc(strlen(ua) + 1);
  }

  if (version)
    strcpy_s(version, 1024 ,ua);

  // release service
  NS_IF_RELEASE(nsMemoryService);
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// here the plugin is asked by Mozilla to tell if it is scriptable
// we should return a valid interface id and a pointer to 
// nsScriptablePeer interface which we should have implemented
// and which should be defined in the corressponding *.xpt file
// in the bin/components folder
NPError	nsPluginInstance::GetValue(NPPVariable aVariable, void *aValue)
{
  NPError rv = NPERR_NO_ERROR;

  switch (aVariable) {
    case NPPVpluginScriptableInstance: {
      // addref happens in getter, so we don't addref here
      nsISupportSpaceProxy * scriptablePeer = getScriptablePeer();
      if (scriptablePeer) {
        *(nsISupports **)aValue = scriptablePeer;
      } else
        rv = NPERR_OUT_OF_MEMORY_ERROR;
    }
    break;

    case NPPVpluginScriptableIID: {
      static nsIID scriptableIID = NS_ISUPPORTSPACEPROXY_IID;
      nsIID* ptr = (nsIID *)NPN_MemAlloc(sizeof(nsIID));
      if (ptr) {
          *ptr = scriptableIID;
          *(nsIID **)aValue = ptr;
      } else
        rv = NPERR_OUT_OF_MEMORY_ERROR;
    }
    break;

    default:
      break;
  }

  return rv;
}


long nsPluginInstance::Init(const char* msiPath, const char* version, const char* productCode)
{
TRY_CATCH

	HRESULT result;
	CComVariant init_arg[3];

	init_arg[2]=msiPath;
	init_arg[1]=version;
	init_arg[0]=productCode;
		
	if((result=m_broker.InvokeN(L"Init",init_arg,_countof(init_arg)))!=S_OK)
		Log.Add(_ERROR_, _T("nsPluginInstance::Init InvokeN Init failed"));
			
CATCH_LOG(_T("nsPluginInstance::Init"))
	return 0;
}

long nsPluginInstance::InitSession(const char* relaySrv, const char* sId, const char* userId, const char* passwd, const char* remoteUserId)
{
TRY_CATCH

	HRESULT result;
	CComVariant init_session_arg[5];

	init_session_arg[4]=relaySrv;
	init_session_arg[3]=sId;
	init_session_arg[2]=userId;
	init_session_arg[1]=passwd;
	init_session_arg[0]=remoteUserId;

	if((result=m_broker.InvokeN(L"InitSession",init_session_arg,_countof(init_session_arg)))!=S_OK)
		Log.Add(_ERROR_, _T("nsPluginInstance::Init InvokeN InitSession failed"));

CATCH_LOG(_T("nsPluginInstance::InitSession"))
	return 0;
}

long nsPluginInstance::HandleRequest(const char* dstUserId, ULONG dstSvcId, const char* srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, const char* params)
{
TRY_CATCH

    HRESULT result;
	CComVariant arg_handle_requets[8];

	arg_handle_requets[7]=dstUserId;
	arg_handle_requets[6]=dstSvcId;
	arg_handle_requets[5]=srcUserId;
	arg_handle_requets[4]=srcSvcId;
	arg_handle_requets[3]=rId;
	arg_handle_requets[2]=rType;
	arg_handle_requets[1]=param;
	arg_handle_requets[0]=params;

	if((result=m_broker.InvokeN(L"HandleRequest",arg_handle_requets,_countof(arg_handle_requets)))!=S_OK)
		Log.Add(_ERROR_, _T("nsPluginInstance::Init InvokeN HandleRequest failed"));

CATCH_LOG(_T("nsPluginInstance::HandleRequest"))
	return 0;
}

long nsPluginInstance::DoNotifyLogMessage(const char* message, ULONG severity)
{
TRY_CATCH
	//char url[2048] = {'\0'};
	//sprintf_s(url, 2048, "javascript:NotifyLogMessage('%s','%d')", message, severity);
	//Log.Add(_MESSAGE_, _T("nsPluginInstance::DoNotifyLogMessage %s"),url);
	//Log.Add(_MESSAGE_, _T("nsPluginInstance::DoNotifyLogMessage '%s','%d'"),message, severity);
	AtlTrace("nsPluginInstance::DoNotifyLogMessage '%s','%d'",message, severity);
	//NPN_GetURL(mInstance, url, NULL);
CATCH_LOG(_T("nsPluginInstance::DoNotifyLogMessage"))
	return 0;
}

long nsPluginInstance::DoRequestSent(const char* dstUserId, ULONG dstSvcId, const char* srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, const char* params)
{
	char url[2048] = {'\0'};
	sprintf_s(url, 2048, "javascript:OnRequestSent('%s','%d','%s','%d','%d','%d','%d','%s')", 
		dstUserId, dstSvcId, srcUserId, srcSvcId, rId, rType, param, params);

	Log.Add(_MESSAGE_, _T("nsPluginInstance::DoRequestSent %s"),url);
	NPN_GetURL(mInstance, url, NULL);
	return 0;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// this method will return the scriptable object (and create it if necessary)
nsScriptablePeer* nsPluginInstance::getScriptablePeer()
{
	if (!mScriptablePeer) {
		mScriptablePeer = new nsScriptablePeer(this);
		if(!mScriptablePeer)
			return NULL;

		NS_ADDREF(mScriptablePeer);
	}

	// add reference for the caller requesting the object
	NS_ADDREF(mScriptablePeer);
	return mScriptablePeer;
}


static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			{
			}
			break;
		case WM_TIMER:
			//PostMessage( hWnd, WM_PAINT, NULL, NULL );
			break;
		case WM_CLOSE:
			//PostQuitMessage( 0 );
			break;
		default:
			break;				
	}
	return DefWindowProc( hWnd, message, wParam, lParam );
}