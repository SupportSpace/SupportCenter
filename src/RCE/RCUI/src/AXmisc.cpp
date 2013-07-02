/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  AXmisc.cpp
///
///  ActiveX auxiliary miscellaneous ware
///
///  @author "Archer Software" Solovyov K. @date 12.12.2006
///
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"// TODO ask Max how take away this.

#include "AXmisc.h"

#include <shlobj.h>
#include <NWL/Streaming/CStreamException.h>

const GUID LIB_GUID = 
	{0x15C5A5E9, 0x7C69, 0x436E, 0x9E, 0x89, 0xA2, 0xA5, 0x66, 0xFB, 0xB7, 0x7C};


void GetRCRecFullFileName(TCHAR *path,const TCHAR *fileName)
{		if(!::SHGetSpecialFolderPath(NULL,path,CSIDL_PERSONAL,false))throw MCStreamException("My Documents path not found");
		size_t k=_tcslen(path);if(k+2>=MAX_PATH)throw MCStreamException("My Documents path is too long");
		path[k]=_T('\\');path[k+1]=_T('\0');++k;//add '\\' to path My Documents
		const TCHAR* tch=_tcsrchr(fileName,_T('\\'));if(!tch)tch=fileName;else ++tch;//find last '\\' character
		for(;*tch!='\0'&&k<MAX_PATH;++tch)//copy valid characters
			if(*tch!=_T('.')&&*tch!=_T('?'))path[k++]=*tch;//TODO add invalid characters
		if(k+5<MAX_PATH)//add ".rce"
		{	path[k++]=_T('.');path[k++]=_T('r');path[k++]=_T('c');path[k++]=_T('e');path[k]=_T('\0');}
		else MCStreamException("File name is too long");
}