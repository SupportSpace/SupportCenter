/////////////////////////////////////////////////////////////////////////
///
///  SupportSpace Ltd.
///
///  plugin.h
///
///  nsPluginInstance object declaration. The object is main plugin instance
///
///  @author Anatoly Gutnick @date 16.04.2008
///
////////////////////////////////////////////////////////////////////////

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#define _WIN32_DCOM

#include "../../shared/pluginbase.h"
#include "nsScriptablePeer.h"

#include "stdafx.h"
#include "C_ICoBrokerProxyEvents.h"

class nsPluginInstance : public nsPluginInstanceBase, public IEventListener
{
public:
  nsPluginInstance(NPP aInstance);
  ~nsPluginInstance();

  NPBool init(NPWindow* aWindow);
  void shut();
  NPBool isInitialized();

  // we need to provide implementation of this method as it will be
  // used by Mozilla to retrive the scriptable peer
  // and couple of other things on Unix
  NPError	GetValue(NPPVariable variable, void *value);

  nsScriptablePeer* getScriptablePeer();

  // locals
  void getVersion(char* *aVersion);

  // we need to implement this methods as broker proxy adaptor methods implementation 
  long Init(const char* msiPath, const char* version, const char* productCode);
  long InitSession(const char* relaySrv, const char* sId, const char* userId, const char* passwd, const char* remoteUserId);
  long HandleRequest(const char* dstUserId, ULONG dstSvcId, const char* srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, const char* params);

  // we need to implement this methods as broker proxy adaptor callbacks implementation 
  long DoNotifyLogMessage(const char* message, ULONG severity);
  long DoRequestSent(const char* dstUserId, ULONG dstSvcId, const char* srcUserId, ULONG srcSvcId, ULONG rId, ULONG rType, ULONG param, const char* params);

private:
// pure ns plugin related data members
  NPP mInstance;
  NPBool mInitialized;
  nsScriptablePeer * mScriptablePeer;

// brokerproxy related data members
  CComPtr<IDispatch> m_broker;
  C_ICoBrokerProxyEvents m_coBrokerEvents;
  CComPtr<IUnknown> m_pBrokerUnk;
  DWORD m_dwCustCookie;

  HWND m_hWnd;//hwnd passed by ff
};

#endif // __PLUGIN_H__
