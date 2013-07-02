#include "StdAfx.h"
#include "SHA1.h"
#include <AidLib/Logging/cLog.h> 
#include "AidLib/CException/CException.h"

#define MAX_TMPBUF_LEN  81

CSHA1::CSHA1(void)
{
}

CSHA1::~CSHA1(void)
{
}

void CSHA1::ReportHash(
	   unsigned __int8  digest[SHA1Size], 
	   char* szReport, 
	   DWORD dwReportBufSize, 
	   unsigned char uReportType)	
{
TRY_CATCH

	char szTemp[MAX_TMPBUF_LEN];

	if(szReport == NULL) return;

	if(uReportType == REPORT_HEX)
	{
		for(unsigned char i = 0; i < SHA1Size; i++)
		{
			sprintf_s(szTemp, MAX_TMPBUF_LEN, "%02x", digest[i]);
			strcat_s(szReport, dwReportBufSize, szTemp);
		}
	}

CATCH_LOG(_T("CSHA1 ::ReportHash"))
}