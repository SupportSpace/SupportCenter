#pragma once

#include <wincrypt.h>
#pragma comment(lib, "Crypt32.lib")

class CProtectedData
{
public:
	CProtectedData(void);
	~CProtectedData(void);

	void	CryptData(IN TCHAR*  str_in, OUT DATA_BLOB* DataOut);
	void	DeCryptData(IN DATA_BLOB* DataIn, OUT TCHAR* str_out);
};
