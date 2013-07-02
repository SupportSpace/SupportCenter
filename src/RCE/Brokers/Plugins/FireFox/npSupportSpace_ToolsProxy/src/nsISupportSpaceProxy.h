/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM nsISupportSpaceProxy.idl
 */

#ifndef __gen_nsISupportSpaceProxy_h__
#define __gen_nsISupportSpaceProxy_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsISupportSpaceProxy */
#define NS_ISUPPORTSPACEPROXY_IID_STR "482e1890-1fe5-11d5-9cf8-0060b0fbd8ac"

#define NS_ISUPPORTSPACEPROXY_IID \
  {0x482e1890, 0x1fe5, 0x11d5, \
    { 0x9c, 0xf8, 0x00, 0x60, 0xb0, 0xfb, 0xd8, 0xac }}

class NS_NO_VTABLE nsISupportSpaceProxy : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISUPPORTSPACEPROXY_IID)

  /* long Init (in string msiPath, in string version, in string productCode); */
  NS_IMETHOD Init(const char *msiPath, const char *version, const char *productCode, PRInt32 *_retval) = 0;

  /* long InitSession (in string relaySrv, in string sId, in string userId, in string passwd, in string remoteUserId); */
  NS_IMETHOD InitSession(const char *relaySrv, const char *sId, const char *userId, const char *passwd, const char *remoteUserId, PRInt32 *_retval) = 0;

  /* long HandleRequest (in string dstUserId, in unsigned long dstSvcId, in string srcUserId, in unsigned long srcSvcId, in unsigned long rId, in unsigned long rType, in unsigned long param, in string params); */
  NS_IMETHOD HandleRequest(const char *dstUserId, PRUint32 dstSvcId, const char *srcUserId, PRUint32 srcSvcId, PRUint32 rId, PRUint32 rType, PRUint32 param, const char *params, PRInt32 *_retval) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISUPPORTSPACEPROXY \
  NS_IMETHOD Init(const char *msiPath, const char *version, const char *productCode, PRInt32 *_retval); \
  NS_IMETHOD InitSession(const char *relaySrv, const char *sId, const char *userId, const char *passwd, const char *remoteUserId, PRInt32 *_retval); \
  NS_IMETHOD HandleRequest(const char *dstUserId, PRUint32 dstSvcId, const char *srcUserId, PRUint32 srcSvcId, PRUint32 rId, PRUint32 rType, PRUint32 param, const char *params, PRInt32 *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISUPPORTSPACEPROXY(_to) \
  NS_IMETHOD Init(const char *msiPath, const char *version, const char *productCode, PRInt32 *_retval) { return _to Init(msiPath, version, productCode, _retval); } \
  NS_IMETHOD InitSession(const char *relaySrv, const char *sId, const char *userId, const char *passwd, const char *remoteUserId, PRInt32 *_retval) { return _to InitSession(relaySrv, sId, userId, passwd, remoteUserId, _retval); } \
  NS_IMETHOD HandleRequest(const char *dstUserId, PRUint32 dstSvcId, const char *srcUserId, PRUint32 srcSvcId, PRUint32 rId, PRUint32 rType, PRUint32 param, const char *params, PRInt32 *_retval) { return _to HandleRequest(dstUserId, dstSvcId, srcUserId, srcSvcId, rId, rType, param, params, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISUPPORTSPACEPROXY(_to) \
  NS_IMETHOD Init(const char *msiPath, const char *version, const char *productCode, PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(msiPath, version, productCode, _retval); } \
  NS_IMETHOD InitSession(const char *relaySrv, const char *sId, const char *userId, const char *passwd, const char *remoteUserId, PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitSession(relaySrv, sId, userId, passwd, remoteUserId, _retval); } \
  NS_IMETHOD HandleRequest(const char *dstUserId, PRUint32 dstSvcId, const char *srcUserId, PRUint32 srcSvcId, PRUint32 rId, PRUint32 rType, PRUint32 param, const char *params, PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->HandleRequest(dstUserId, dstSvcId, srcUserId, srcSvcId, rId, rType, param, params, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSupportSpaceProxy : public nsISupportSpaceProxy
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISUPPORTSPACEPROXY

  nsSupportSpaceProxy();

private:
  ~nsSupportSpaceProxy();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsSupportSpaceProxy, nsISupportSpaceProxy)

nsSupportSpaceProxy::nsSupportSpaceProxy()
{
  /* member initializers and constructor code */
}

nsSupportSpaceProxy::~nsSupportSpaceProxy()
{
  /* destructor code */
}

/* long Init (in string msiPath, in string version, in string productCode); */
NS_IMETHODIMP nsSupportSpaceProxy::Init(const char *msiPath, const char *version, const char *productCode, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long InitSession (in string relaySrv, in string sId, in string userId, in string passwd, in string remoteUserId); */
NS_IMETHODIMP nsSupportSpaceProxy::InitSession(const char *relaySrv, const char *sId, const char *userId, const char *passwd, const char *remoteUserId, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long HandleRequest (in string dstUserId, in unsigned long dstSvcId, in string srcUserId, in unsigned long srcSvcId, in unsigned long rId, in unsigned long rType, in unsigned long param, in string params); */
NS_IMETHODIMP nsSupportSpaceProxy::HandleRequest(const char *dstUserId, PRUint32 dstSvcId, const char *srcUserId, PRUint32 srcSvcId, PRUint32 rId, PRUint32 rType, PRUint32 param, const char *params, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsISupportSpaceProxy_h__ */
