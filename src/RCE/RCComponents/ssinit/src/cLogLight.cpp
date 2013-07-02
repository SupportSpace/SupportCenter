/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  cLogLight.cpp
///
///  cLogLight, light cLog class
///
///  @author "Archer Software" Kirill Solovyov. @date 21.05.2007
///
////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#define MSG_BUF_SIZE 4096

tstring i2tstring(const int i, const int radix)
{
	TCHAR buf[30];
	_itot_s(i,buf,radix);
	return buf;
}

void cLogLight::Add(const DWORD error,const TCHAR* Format, ...)
{
	PTCHAR Mess(NULL);
try
{
	va_list args;
	va_start( args, Format );	// Init the user arguments list.

	unsigned int i=1;
	unsigned mult=1;
	int result;
	for(Mess=new TCHAR[MSG_BUF_SIZE];
		(result=_vsntprintf_s( Mess, MSG_BUF_SIZE*mult,_TRUNCATE, Format, args ))<0 && i<MSG_BUF_SIZE_DOUBLING;
		i++, mult <<= 1)
	{
		delete [] Mess;
		Mess = NULL;
		Mess=new TCHAR[MSG_BUF_SIZE * (mult << 1)];
	}
	tstring winError;
	if (result<0)
	{
		TCHAR truncated[]=_T("MESSAGE TRUNCATED");
		_tcscpy_s(Mess+MSG_BUF_SIZE*mult-_countof(truncated)-1,_countof(truncated),truncated);
	} else
	{
		if(error!=ERROR_SUCCESS)
		{
			LPVOID lpBuffer;
			if(FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
												FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
												NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
												(LPTSTR) &lpBuffer, 16, NULL) != 0)
			{
				winError+=_T(". (0x")+i2tstring(error,16)+_T(") ")+reinterpret_cast<PTCHAR>(lpBuffer);
				LocalFree(lpBuffer);
			}
			else
			{
				winError+=_T(". Unknown WinError code ")+i2tstring(error,16);
			}
		}
	}
	AddList(Mess+winError);
	delete [] Mess;
	Mess = NULL;
}
catch(...)
{
	try
	{
		if (Mess) delete [] Mess; //Preventing memory leaks
		AddList(_T("cLog::Add: Exception happened"));
	}
	catch(...)
	{
		//One more exception happened
		//going further
	}
}
}
