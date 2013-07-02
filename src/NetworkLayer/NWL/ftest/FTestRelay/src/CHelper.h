/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CHelper.h
///
///  Declares CHelper class, responsible for additional functionality
///
///  @author Dmitry Netrebenko @date 15.05.2007
///
////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

///  CHelper class, responsible for additional functionality
class CHelper
{
private:
///  Prevents making copies of CHelper objects.
	CHelper( const CHelper& );
	CHelper& operator=( const CHelper& );

public:
///  Constructor
	CHelper();
///  Destructor
	~CHelper();

///  Extracts parameters from command line
///  @param  CmdLine - command line
///  @param  _argc - number of parameters
///  @return pointer to array of parameters
	static PTCHAR* CommandLineToArgv(PTCHAR CmdLine, int* pArgc);

};
