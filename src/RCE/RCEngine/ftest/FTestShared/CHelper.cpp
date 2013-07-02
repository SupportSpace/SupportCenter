/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CHelper.cpp
///
///  Implements CHelper class, responsible for additional functionality
///
///  @author Dmitry Netrebenko @date 15.05.2007
///
////////////////////////////////////////////////////////////////////////

#include "CHelper.h"
#include <AidLib/CException/CException.h>
#include <stdlib.h>

CHelper::CHelper()
{
TRY_CATCH
CATCH_THROW()
}

CHelper::~CHelper()
{
TRY_CATCH
CATCH_LOG()
}

PTCHAR* CHelper::CommandLineToArgv(PTCHAR CmdLine, int* pArgc)
{
TRY_CATCH

	PTCHAR*		pArgv;
	PTCHAR		Argv;
	ULONG		Len;
	ULONG		Argc = 0;
	TCHAR		cSym;
	ULONG		Idx1, Idx2 = 0;
	BOOLEAN		bInQuotas = FALSE;
	BOOLEAN		bInText = FALSE;
	BOOLEAN		bInSpace = TRUE;

	Len = (ULONG)strlen(CmdLine);
	Idx1 = ((Len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);

	pArgv = (PTCHAR*)GlobalAlloc( 
		GMEM_FIXED,
		Idx1 + (Len + 2) * sizeof(TCHAR) 
	);

	Argv = (PTCHAR)(((PUCHAR)pArgv) + Idx1);

	pArgv[Argc] = Argv;
	Idx1 = 0;

	while( cSym = CmdLine[Idx1] ) 
	{
		if( bInQuotas ) 
		{
			if( '\"' == cSym ) 
			{
				bInQuotas = FALSE;
			} 
			else 
			{
				Argv[Idx2] = cSym;
				Idx2++;
			}
		} 
		else 
		{
			switch( cSym ) 
			{
				case '\"':
					bInQuotas = TRUE;
					bInText = TRUE;
					if( bInSpace ) 
					{
						pArgv[Argc] = Argv + Idx2;
						Argc++;
					}
					bInSpace = FALSE;
					break;

				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if( bInText ) 
					{
						Argv[Idx2] = '\0';
						Idx2++;
					}
					bInText = FALSE;
					bInSpace = TRUE;
					break;

				default:
					bInText = TRUE;
					if( bInSpace ) 
					{
						pArgv[Argc] = Argv + Idx2;
						Argc++;
					}
					Argv[Idx2] = cSym;
					Idx2++;
					bInSpace = FALSE;
					break;
			}
		}
		Idx1++;
	}

	Argv[Idx2] = '\0';
	pArgv[Argc] = NULL;

	(*pArgc) = Argc;

	return pArgv;

CATCH_THROW()
}


int CHelper::GetRandom(int from, int to)
{
TRY_CATCH
	
	int ret = 0;
	srand(GetTickCount());
	do
	{
		ret = rand();
	} while ((ret < from) || (ret > to));

	return ret;

CATCH_THROW()
}

