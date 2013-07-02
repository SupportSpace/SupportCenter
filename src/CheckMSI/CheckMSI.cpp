// CheckMSI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _DEBUG
	#include <windows.h>
	#include <Msi.h>
	#pragma comment (lib, "Msi.lib")
	void Foo()
	{
		MsiEnableLog(0,0,0);
	}
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}

