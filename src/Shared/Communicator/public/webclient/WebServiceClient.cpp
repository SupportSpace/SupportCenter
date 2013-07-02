// WebServiceClient.cpp: implementation of the CWebServiceClient class.
//
//////////////////////////////////////////////////////////////////////
#include <windows.h>
#include "WebServiceClient.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWebServiceClient::CWebServiceClient()
{
	m_hInternet = NULL;
	m_hRequest = NULL;
	m_certStoreType = certStoreMY;
	m_hStore = NULL;
	m_hSession = NULL;
	m_pContext = NULL;
	m_wPort = 443;
	m_strAgentName = _T("");	

	m_secureFlags = INTERNET_FLAG_KEEP_CONNECTION |
					INTERNET_FLAG_EXISTING_CONNECT |
					INTERNET_FLAG_DONT_CACHE |
					INTERNET_FLAG_RELOAD;
	m_ReqID = 0;
}

CWebServiceClient::CWebServiceClient(const tstring& sServerName,
									 INTERNET_PORT port, 
									 const tstring& sObjectName,
									 const tstring& sUserName, 
									 const tstring& sPassword):
							m_strServerName(sServerName),
							m_strObjectName(sObjectName),
							m_wPort(port),
							m_strUserName(sUserName),
							m_strPassword(sPassword)	
{
	m_hRequest = NULL;
	m_hStore = NULL;
	m_hSession = NULL;
	m_pContext = NULL;
	m_strAgentName = _T("");	

	m_secureFlags = INTERNET_FLAG_KEEP_CONNECTION |
					INTERNET_FLAG_EXISTING_CONNECT |
					INTERNET_FLAG_DONT_CACHE |
					INTERNET_FLAG_RELOAD;
	m_ReqID = 0;
}

CWebServiceClient::~CWebServiceClient()
{
	ClearHandles();
}


bool CWebServiceClient::ConnectToHttpsServer(const tstring& strVerb)
{
	try {	
		m_hInternet = InternetOpen(m_strAgentName.c_str(), INTERNET_OPEN_TYPE_PRECONFIG , 
				NULL, NULL, INTERNET_FLAG_DONT_CACHE);
		if (!m_hInternet) {
			m_strLastError = _T("Cannot open internet");
			m_lastErrorCode = GetLastError();
			return false;
		}
			
		m_hSession = InternetConnect(
			m_hInternet, 
			m_strServerName.c_str(), 
			m_wPort,
			m_strUserName.c_str(), 
			m_strPassword.c_str(),
			INTERNET_SERVICE_HTTP,
			0,
			0);
		if (!m_hSession) {			
			m_strLastError = _T("Cannot connect to internet");
			m_lastErrorCode = GetLastError();
			ClearHandles();
			return false;
		}
		m_hRequest = HttpOpenRequest(
				m_hSession, 
				strVerb.c_str(),
				m_strObjectName.c_str(),
				NULL,
				_T(""),
				NULL,
				m_secureFlags,
				m_ReqID);
		if (!m_hRequest) {
			m_strLastError = _T("Cannot perform http request");
			m_lastErrorCode = GetLastError();
			ClearHandles();		
			return false;
		}
		
		m_ReqID++;
	}
	catch(...) {
		m_strLastError = _T("Memory Exception occured");
		m_lastErrorCode = GetLastError();
		return false;
	}
	return true;
}

bool CWebServiceClient::SendHttpsRequest(const tstring& sSoapMessageXML, DWORD dwSoapMsgLength)
{
	TCHAR	 strHeaders[1024] = {0};
	_stprintf_s( strHeaders,1024, _T("Content-Type: text/xml; charset=utf-8\nContent-Length:%d"), dwSoapMsgLength);
	DWORD	 dwHeadersLength = _tcslen(strHeaders);

	try {
		//todo number of retries
		for (int tries = 0; tries < 3; ++tries) {		
			int result =  HttpSendRequest(m_hRequest, strHeaders, dwHeadersLength, (LPVOID)sSoapMessageXML.c_str(), dwSoapMsgLength);
			if (result) 				
				return true;							
			int lastErr = GetLastError();
			if (lastErr == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED) {
				if (!SetClientCert()) {
					m_strLastError = _T("Cannot perform http request, client authentication needed but couldnt detect required client certificate");
					m_lastErrorCode = GetLastError();
					return false;
				}					
			}
			else if (lastErr == ERROR_INTERNET_INVALID_CA) {
				m_strLastError = _T("Cannot perform http request, client authentication needed, invalid client certificate is used");
				m_lastErrorCode = GetLastError();
				return false;
			}
			else {
				m_strLastError = _T("Cannot perform http request");
				m_lastErrorCode = GetLastError();
				return false;
			}
		} 
	}
	catch(...) {
		m_strLastError = _T("Memory Exception occured");
		m_lastErrorCode = GetLastError();
		return false;
	}
	return false;
}

void CWebServiceClient::ClearHandles()
{
	if (m_hInternet) {
		InternetCloseHandle(m_hInternet);
		m_hInternet = NULL;
	}
		
	if (m_hSession) {
		InternetCloseHandle(m_hSession);
		m_hSession = NULL;
	}
		
	if (m_pContext) {
		CertFreeCertificateContext(m_pContext);
		m_pContext = NULL;
	}
	if (m_hStore) {
		CertCloseStore(m_hStore, CERT_CLOSE_STORE_FORCE_FLAG);
		m_hStore = NULL;
	}

	if (m_hRequest) {
	  InternetCloseHandle(m_hRequest);
	  m_hInternet = NULL;
	}
}

bool CWebServiceClient::SetClientCert()
{
	TCHAR *lpszStoreName;
	switch (m_certStoreType) {
	case certStoreMY:
		lpszStoreName = _T("MY");
		break;
	case certStoreCA:
		lpszStoreName = _T("CA");
		break;
	case certStoreROOT:
		lpszStoreName = _T("ROOT");
		break;
	case certStoreSPC:
		lpszStoreName = _T("SPC");
		break;
	}

 	m_hStore = CertOpenSystemStore( NULL, lpszStoreName);
	if (!m_hStore) {
		m_strLastError = _T("Cannot open system store");
		m_strLastError += lpszStoreName;
		m_lastErrorCode = GetLastError();
		ClearHandles();
		return false;
	}
	
	m_pContext = FindCertWithOUNITName();

	if (!m_pContext) {
		m_strLastError = _T("Cannot find the required certificate");
		m_lastErrorCode = GetLastError();
		ClearHandles();
		return false;
	}
	
	// INTERNET_OPTION_CLIENT_CERT_CONTEXT is 84
	int res = InternetSetOption(m_hRequest, 
							INTERNET_OPTION_CLIENT_CERT_CONTEXT, 
							(void *) m_pContext, sizeof(CERT_CONTEXT));
	if (!res) {
		m_strLastError = _T("Cannot set certificate context");
		m_lastErrorCode = GetLastError();
		ClearHandles();
		return false;
	}
	
	return true;
}

PCCERT_CONTEXT CWebServiceClient::FindCertWithOUNITName()
{
	//This function performs a certificate contex search
	//by the organizational unit name of the issuer
	//Take this function as a sample for your possible different search functions
	PCCERT_CONTEXT pCertContext = NULL;		
	CERT_RDN certRDN;

	certRDN.cRDNAttr = 1;
	certRDN.rgRDNAttr = new CERT_RDN_ATTR;
	certRDN.rgRDNAttr->pszObjId = szOID_ORGANIZATIONAL_UNIT_NAME;
	certRDN.rgRDNAttr->dwValueType = CERT_RDN_ANY_TYPE;
	certRDN.rgRDNAttr->Value.pbData = (BYTE *) m_strOName.c_str();
	certRDN.rgRDNAttr->Value.cbData = _tcslen(m_strOName.c_str());

	pCertContext = CertFindCertificateInStore(m_hStore, 
		X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
		0, CERT_FIND_ISSUER_ATTR, &certRDN, NULL);
	

	delete certRDN.rgRDNAttr;
	return pCertContext;
}

tstring CWebServiceClient::GetRequestResult()
{
	DWORD dwNumberOfBytesRead;
#define CHUNK_SIZE	1024
	TCHAR szBuf[CHUNK_SIZE] = {0};
	tstring strResult;
	int result; 
	do {
		result = InternetReadFile(m_hRequest, szBuf, CHUNK_SIZE-1, &dwNumberOfBytesRead);												
		if(result && dwNumberOfBytesRead != 0)
		{
			strResult += szBuf;
			memset(szBuf, 0, CHUNK_SIZE);
		}
		else
			break;
	} while(true);

	// 
	// TrimRight \r\n\r\n
	strResult.erase( strResult.find_last_not_of(_T("\r\n\r\n") + 1) );
	return strResult;
}