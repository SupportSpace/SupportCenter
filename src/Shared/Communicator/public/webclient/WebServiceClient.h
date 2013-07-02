// SslCon: interface for the CSslConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINETSEC_H__91AD1B9B_5B03_457E_A6B6_D66BB03147B7__INCLUDED_)
#define AFX_WINETSEC_H__91AD1B9B_5B03_457E_A6B6_D66BB03147B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Wininet.h>
#include <wincrypt.h>

#pragma warning(disable:4786)

#include <Aidlib\Strings\tstring.h>
enum CertStoreType {certStoreMY, certStoreCA, certStoreROOT, certStoreSPC};

class CWebServiceClient  
{
public:
	CWebServiceClient();
	CWebServiceClient(const tstring& sServerName,
					  INTERNET_PORT  port, 
					  const tstring& sObjectName, 
					  const tstring& sUserName, 
					  const tstring& sPassword);
	virtual ~CWebServiceClient();	
public:	
	bool ConnectToHttpsServer(const tstring& strVerb = _T("POST") );
	bool SendHttpsRequest(const tstring& sSoapMessageXML,  DWORD  dwSoapMsgLength);
	tstring GetRequestResult();
public: //accessors
	void SetAgentName(const tstring& strAgentName) { m_strAgentName = strAgentName; }
	void SetCertStoreType(CertStoreType storeID) { m_certStoreType = storeID; }
	void SetServerName(const tstring& strServerName) { m_strServerName = strServerName; }
	void SetObjectName(const tstring& strObjectName) { m_strObjectName = strObjectName; }
	void SetPort(INTERNET_PORT wPort = INTERNET_DEFAULT_HTTPS_PORT) { m_wPort = wPort; }
	void SetRequestID(int reqID) { m_ReqID = reqID; }
	void SetSecurityFlags(int flags) { m_secureFlags = flags; }
	//Search indicators	
	void SetOrganizationName(const tstring& strOName) { m_strOName = strOName;} 
	tstring GetLastErrorString() { return m_strLastError; }
	int GetLastErrorCode() { return m_lastErrorCode; }
	
private:
	// examine the following function in order to perform different certificate 
	// property searchs in stores. It detects the desired certificate with the organization name
	PCCERT_CONTEXT FindCertWithOUNITName();	
	/////////////////////////////////////
	bool SetClientCert();
	void ClearHandles();
private:
	HINTERNET m_hInternet;
	HINTERNET m_hRequest;
	HINTERNET m_hSession;

	tstring m_strServerName;
	tstring m_strObjectName;
	INTERNET_PORT m_wPort;
	int m_secureFlags;

	HCERTSTORE m_hStore;
	PCCERT_CONTEXT m_pContext;
	CertStoreType m_certStoreType;	
	tstring m_strUserName;
	tstring m_strPassword;		
	tstring m_strAgentName;
	tstring m_strOName;
	tstring m_strLastError;
	int m_lastErrorCode;
	int m_ReqID;
};

#endif // !defined(AFX_WINETSEC_H__91AD1B9B_5B03_457E_A6B6_D66BB03147B7__INCLUDED_)

