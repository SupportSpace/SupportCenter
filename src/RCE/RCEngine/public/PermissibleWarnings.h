/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  PermissibleWarnings.h
///
///  Warnings to permit
///
///  @author "Archer Software" Sogin M. @date 13.10.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once

#pragma warning( disable : 4800 ) //'BOOL' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning( disable : 4311 ) //'type cast' : pointer truncation from 'char *' to 'LONG'
#pragma warning( disable : 4018 ) //signed/unsigned mismatch
#pragma warning( disable : 4244 ) //conversion from 'unsigned long' to 'BYTE', possible loss of data
#pragma warning( disable : 4267 ) //'initializing' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning( disable : 4312 ) //'type cast' : conversion from 'LONG' to 'vncDesktop *' of greater size
#pragma warning( disable : 4267 ) //'argument' : conversion from 'size_t' to 'const VCard', possible loss of data


#pragma warning( disable : 4049 ) //warning LNK: locally defined symbol ... imported
#pragma warning( disable : 4251 ) // class ... needs to have dll-interface to be used by clients of class
#pragma warning( disable : 4150 ) // deletion of pointer to incomplete type 'vncServer'; no destructor called
