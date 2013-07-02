#pragma once

#include <AidLib/CCrypto/CCrypto.h>
//
//
//
enum
{
	REPORT_HEX = 0,
	REPORT_DIGIT = 1
};

class CSHA1 : public CCrypto
{
public:
	CSHA1(void);
	~CSHA1(void);

	void ReportHash(unsigned __int8  digest[SHA1Size], char *szReport,DWORD dwReportBufSize, unsigned char uReportType = REPORT_HEX);	
};
