/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  AXmisc.h
///
///  ActiveX auxiliary miscellaneous ware
///
///  @author "Archer Software" Solovyov K. @date 12.12.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once

extern const GUID LIB_GUID; 

///	Create full name by name file plus My Documents path, excluding relative path characters, dot and question-mark characters. For example, input string ..\..\..fil?enam.e transform to “C:\\Documents and Settings\\Administrator\\My Documents\\filename.rce”. 
/// @param path Output string that mast by create before with MAX_PATH length.
/// @param fileName An input file name string.
/// @return full name in path parameter. Throw exception CStreamException other wise.
void GetRCRecFullFileName(TCHAR *path,const TCHAR *fileName);
