
#include "ini.h"

// CCommandLinePath [BEGIN] ///////////////////////////////////////////////////////////////////////////////////

CCommandLinePath::CCommandLinePath(const TCHAR* cmd_line):
	dir(0)
{
	SetCommandLine(cmd_line);
}
//-------------------------------------------------------------------------------------------------------------

CCommandLinePath::~CCommandLinePath()
{
	if (dir)
		delete[] dir;
}
//-------------------------------------------------------------------------------------------------------------

void CCommandLinePath::SetCommandLine(const TCHAR* cmd_line)
{
	CCommandLinePath::~CCommandLinePath();
	if (cmd_line){
		if (*cmd_line==_T('\"'))
			cmd_line++;
		const TCHAR* pCh=cmd_line;
		const TCHAR* last_slash=cmd_line;
		while (*pCh){
			if (*pCh==_T('\\'))
				last_slash=pCh;
			pCh++;
		}
		size_t sz_str=last_slash-cmd_line;
		if (sz_str)
			sz_str++;
		dir=new TCHAR[sz_str+1];
		memcpy(dir,cmd_line,sz_str*sizeof(TCHAR));
		dir[sz_str]=_T('\0');
	}
}
// CCommandLinePath [BEGIN] ///////////////////////////////////////////////////////////////////////////////////

// CParam [BEGIN] /////////////////////////////////////////////////////////////////////////////////////////////

CParam::CParam(const wchar_t* name):
	m_modify(false),m_long(0),m_double(0),m_name(name),m_value(L""),sz_rawdata(0),m_rawdata(NULL),
	last_take_type(REG_DWORD),m_cvtbuf(NULL){}
//-------------------------------------------------------------------------------------------------------------

CParam::CParam(const char* name):
	m_modify(false),m_long(0),m_double(0),m_value(L""),sz_rawdata(0),m_rawdata(NULL),
	last_take_type(REG_DWORD),m_cvtbuf(NULL)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,name,-1,buf,sz_buf);
		m_name=buf;
		delete[] buf;
	}
}
//-------------------------------------------------------------------------------------------------------------

CParam::CParam(const CParam& param):
	m_modify(param.m_modify),m_long(param.m_long),m_double(param.m_double),m_name(param.m_name),
	m_value(param.m_value),sz_rawdata(param.sz_rawdata),last_take_type(param.last_take_type),m_cvtbuf(NULL)
{
	m_rawdata=new(std::nothrow) BYTE[sz_rawdata];
	if (m_rawdata)
		memcpy(m_rawdata,param.m_rawdata,sz_rawdata);
}
//-------------------------------------------------------------------------------------------------------------

CParam::~CParam()
{
	if (m_rawdata)
		delete[] m_rawdata;
	if (m_cvtbuf)
		delete[] m_cvtbuf;
}
//-------------------------------------------------------------------------------------------------------------

int CParam::setRawData(const void* data, int sz_data)
{
	m_modify=true;
	sz_rawdata=0;
	if (m_rawdata)
		delete[] m_rawdata;
	m_rawdata=new(std::nothrow) BYTE[sz_data];
	if (m_rawdata){
		sz_rawdata=sz_data;
		memcpy(m_rawdata,data,sz_rawdata);
	}
	return sz_rawdata;
}
//-------------------------------------------------------------------------------------------------------------

int CParam::getRawData(void* data, int sz_data) const
{
	if (!data)
		return sz_rawdata;
	int sz_min=(sz_rawdata<sz_data)?sz_rawdata:sz_data;
	memcpy(data,m_rawdata,sz_min);
	return sz_min;
}
//-------------------------------------------------------------------------------------------------------------

const char* CParam::getNameA() const
{
	int sz_buf=WideCharToMultiByte(CP_ACP,0,m_name.c_str(),-1,NULL,0,NULL,NULL);
	if (m_cvtbuf)
		delete[] m_cvtbuf;

	m_cvtbuf=new(std::nothrow) char[sz_buf];
	if (m_cvtbuf)
		WideCharToMultiByte(CP_ACP,0,m_name.c_str(),-1,m_cvtbuf,sz_buf,NULL,NULL);
	
	return m_cvtbuf;
}
//-------------------------------------------------------------------------------------------------------------

CParam& CParam::operator=(const CParam& param)
{
	if (this!=&param){

		m_modify=true;
		sz_rawdata=0;
		if (m_rawdata)
			delete[] m_rawdata;
		
		m_rawdata=new(std::nothrow) BYTE[param.sz_rawdata];
		if (m_rawdata){
			sz_rawdata=param.sz_rawdata;
			memcpy(m_rawdata,param.m_rawdata,sz_rawdata);
		}

		m_name=param.m_name;
		m_value=param.m_value;
		m_long=param.m_long;
		m_double=param.m_double;
		last_take_type=param.last_take_type;
	}
	return *this;
}
//-------------------------------------------------------------------------------------------------------------

CParam& CParam::operator=(long value)
{
	m_modify=true;
	m_long=value;
	m_double=value;
	wchar_t buf[12];
	_ltow_s(value,buf,12,10);
	m_value=buf;
	last_take_type=REG_DWORD;
	return *this;
}
//-------------------------------------------------------------------------------------------------------------

CParam& CParam::operator=(double value)
{
	m_modify=true;
	m_long=static_cast<long>(value);
	m_double=value;
	wchar_t buf[51];
	swprintf_s(buf,51,L"%g",value);
	m_value=buf;
	last_take_type=REG_QWORD;
	return *this;
}
//-------------------------------------------------------------------------------------------------------------

CParam& CParam::operator=(const wchar_t* value)
{
	m_modify=true;
	m_value=value;
	m_long=_wtol(value);
	m_double=_wtof(value);
	last_take_type=REG_SZ;
	return *this;
}
//-------------------------------------------------------------------------------------------------------------

CParam& CParam::operator=(const char* value)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,value,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,value,-1,buf,sz_buf);
		CParam::operator=(buf);
		delete[] buf;
	}
	return *this;
}
//-------------------------------------------------------------------------------------------------------------

CParam::operator const char*() const
{
	int sz_buf=WideCharToMultiByte(CP_ACP,0,m_value.c_str(),-1,NULL,0,NULL,NULL);
	if (m_cvtbuf)
		delete[] m_cvtbuf;

	m_cvtbuf=new(std::nothrow) char[sz_buf];
	if (m_cvtbuf)
		WideCharToMultiByte(CP_ACP,0,m_value.c_str(),-1,m_cvtbuf,sz_buf,NULL,NULL);
	
	return m_cvtbuf;
}
// CParam [END] ///////////////////////////////////////////////////////////////////////////////////////////////

// CSection [BEGIN] ///////////////////////////////////////////////////////////////////////////////////////////

CSection::CSection(const wchar_t* name):
	m_name(name),last_use_indx(-1),m_cvtbuf(NULL){}
//-------------------------------------------------------------------------------------------------------------

CSection::CSection(const char* name):
	last_use_indx(-1),m_cvtbuf(NULL)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,name,-1,buf,sz_buf);
		m_name=buf;
		delete[] buf;
	}
}
//-------------------------------------------------------------------------------------------------------------

CSection::CSection(const CSection& param):
	m_name(param.m_name),last_use_indx(param.last_use_indx),m_cvtbuf(NULL),vec_param(param.vec_param){}
//-------------------------------------------------------------------------------------------------------------

CSection::~CSection()
{
	if (m_cvtbuf)
		delete[] m_cvtbuf;
}
//-------------------------------------------------------------------------------------------------------------

const char* CSection::getNameA() const
{
	int sz_buf=WideCharToMultiByte(CP_ACP,0,m_name.c_str(),-1,NULL,0,NULL,NULL);
	if (m_cvtbuf)
		delete[] m_cvtbuf;

	m_cvtbuf=new(std::nothrow) char[sz_buf];
	if (m_cvtbuf)
		WideCharToMultiByte(CP_ACP,0,m_name.c_str(),-1,m_cvtbuf,sz_buf,NULL,NULL);

	return m_cvtbuf;
}
//-------------------------------------------------------------------------------------------------------------

void CSection::deleteParam(const wchar_t* param_name)
{
	last_use_indx=-1;
	std::vector<CParam>::iterator i=vec_param.begin();
	while ((i!=vec_param.end()) && wcscoll(i->getNameW(),param_name))
		i++;
	if (i!=vec_param.end())
		vec_param.erase(i);
}
//-------------------------------------------------------------------------------------------------------------

void CSection::deleteParam(const char* param_name)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,param_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,param_name,-1,buf,sz_buf);
		CSection::deleteParam(buf);
		delete[] buf;
	}
}
//-------------------------------------------------------------------------------------------------------------

CSection& CSection::operator=(const CSection& param)
{
	if (this!=&param){
		m_name=param.m_name;
		last_use_indx=param.last_use_indx;
		vec_param.assign(param.vec_param.begin(),param.vec_param.end());
	}
	return *this;
}
//-------------------------------------------------------------------------------------------------------------

CParam& CSection::operator[](const wchar_t* param_name)
{
	if ((last_use_indx!=-1) && !wcscoll(vec_param[last_use_indx].getNameW(),param_name))
		return vec_param[last_use_indx];
	std::vector<CParam>::iterator i=vec_param.begin();
	last_use_indx=-1;
	while ((i!=vec_param.end()) && wcscoll(i->getNameW(),param_name)){
		i++;
		last_use_indx++;
	}
	if (i!=vec_param.end())
		return *i;
	vec_param.push_back(CParam(param_name));
	return *vec_param.rbegin();
}
//-------------------------------------------------------------------------------------------------------------

CParam& CSection::operator[](const char* param_name)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,param_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,param_name,-1,buf,sz_buf);
		CParam& retParam=CSection::operator[](buf);
		delete[] buf;
		return retParam;
	}
	return CSection::operator[](L"");
}
// CSectionReg [END] //////////////////////////////////////////////////////////////////////////////////////////

// CIniReg [BEGIN] ////////////////////////////////////////////////////////////////////////////////////////////

void CIniReg::enumRegValues(HKEY hKey, const wchar_t* section)
{
	DWORD max_name_len, max_data_len;
	
	if (RegQueryInfoKeyW(hKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&max_name_len,&max_data_len,NULL,NULL)==ERROR_SUCCESS){
		
		DWORD val_type;
		wchar_t* val_name=new(std::nothrow) wchar_t[max_name_len+1];
		BYTE* val_data=new(std::nothrow) BYTE[max_data_len];
				
		if (val_name && val_data){

			for (DWORD index=0;; index++){
				DWORD name_len=max_name_len+1;
				DWORD data_len=max_data_len;
				if (RegEnumValueW(hKey,index,val_name,&name_len,NULL,&val_type,val_data,&data_len)!=ERROR_SUCCESS)
					break;

				switch (val_type){
					case REG_DWORD:
						(*this)[section][val_name]=*(reinterpret_cast<long*>(val_data));
					break;
					case REG_QWORD:
						(*this)[section][val_name]=*(reinterpret_cast<double*>(val_data));
					break;
					case REG_SZ:
						(*this)[section][val_name]=reinterpret_cast<wchar_t*>(val_data);
					break;
					default:
						(*this)[section][val_name].setRawData(val_data,data_len);
				}
				(*this)[section][val_name].m_modify=false;
			}
		}
		if (val_name)
			delete[] val_name;
		if (val_data)
			delete[] val_data;
	}
}
//-------------------------------------------------------------------------------------------------------------

void CIniReg::Init(HKEY hKey, const wchar_t* subKey)
{
	last_use_indx=-1;
	hPriKey=NULL;
	DWORD disposition;
	LONG resCreate=RegCreateKeyExW(hKey,subKey,NULL,NULL,REG_OPTION_NON_VOLATILE,
		KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,NULL,&hPriKey,&disposition);

	if (resCreate==ERROR_SUCCESS && disposition==REG_OPENED_EXISTING_KEY){
		wchar_t section[256];

		enumRegValues(hPriKey,L"");

		for (DWORD index=0;; index++){
			DWORD section_len=256;
			if (RegEnumKeyExW(hPriKey,index,section,&section_len,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
				break;

			HKEY hSubKey;
			if (RegOpenKeyExW(hPriKey,section,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hSubKey)==ERROR_SUCCESS){
				enumRegValues(hSubKey,section);
				RegCloseKey(hSubKey);
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------------------

CIniReg::CIniReg(HKEY hKey, const char* subKey)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,subKey,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,subKey,-1,buf,sz_buf);
		Init(hKey,buf);
		delete[] buf;
	}
}
//-------------------------------------------------------------------------------------------------------------

CIniReg::~CIniReg()
{
	RegCloseKey(hPriKey);
}
//-------------------------------------------------------------------------------------------------------------

CSection& CIniReg::operator[](const wchar_t* section_name)
{
	if ((last_use_indx!=-1) && !wcscoll(vec_section[last_use_indx].getNameW(),section_name))
		return vec_section[last_use_indx];
	std::vector<CSection>::iterator i=vec_section.begin();
	last_use_indx=-1;
	while ((i!=vec_section.end()) && wcscoll(i->getNameW(),section_name)){
		i++;
		last_use_indx++;
	}
	if (i!=vec_section.end())
		return *i;
	vec_section.push_back(CSection(section_name));
	return *vec_section.rbegin();
}
//-------------------------------------------------------------------------------------------------------------

CSection& CIniReg::operator[](const char* section_name)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,section_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,section_name,-1,buf,sz_buf);
		CSection& retParam=CIniReg::operator[](buf);
		delete[] buf;
		return retParam;
	}
	return CIniReg::operator[](L"");
}
//-------------------------------------------------------------------------------------------------------------

bool CIniReg::save()
{
	if (hPriKey==NULL)
		return false;

	for (int i=0; i<countSections(); i++){
		HKEY hKey;
		
		if (RegCreateKeyExW(hPriKey,vec_section[i].getNameW(),NULL,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hKey,NULL)==ERROR_SUCCESS){
			for (int k=0; k<vec_section[i].countParams(); k++){

				if (vec_section[i][k].isModify()){
					
					const wchar_t* value_name=vec_section[i][k].getNameW();
					int szRawData=vec_section[i][k].getRawData(NULL,0);
					if (szRawData){
						BYTE* data=new(std::nothrow) BYTE[szRawData];
						if (data){
							vec_section[i][k].getRawData(data,szRawData);
							RegSetValueExW(hKey,value_name,NULL,REG_BINARY,data,szRawData);
							delete[] data;
						}
					}
					else{
						switch (vec_section[i][k].lastType()){
							case REG_DWORD:{
								long data=vec_section[i][k];
								RegSetValueExW(hKey,value_name,NULL,REG_DWORD,(BYTE*)&data,sizeof(long));
							}
							break;
							case REG_QWORD:{
								double data=vec_section[i][k];
								RegSetValueExW(hKey,value_name,NULL,REG_QWORD,(BYTE*)&data,sizeof(double));
							}
							break;
							case REG_SZ:{
								const wchar_t* data=vec_section[i][k];
								RegSetValueExW(hKey,value_name,NULL,REG_SZ,(const BYTE*)data,(int)(wcslen(data)+1)*sizeof(wchar_t));
							}
							break;
						}
					}
				}
			}
		}
	}
	return true;
}
//-------------------------------------------------------------------------------------------------------------

void CIniReg::deleteSection(const wchar_t* section_name)
{
	last_use_indx=-1;
	std::vector<CSection>::iterator i=vec_section.begin();
	while ((i!=vec_section.end()) && wcscoll(i->getNameW(),section_name))
		i++;
	if (i!=vec_section.end())
		vec_section.erase(i);
}
//-------------------------------------------------------------------------------------------------------------

void CIniReg::deleteSection(const char* section_name)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,section_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,section_name,-1,buf,sz_buf);
		CIniReg::deleteSection(buf);
		delete[] buf;
	}
}
// CIniReg [END] //////////////////////////////////////////////////////////////////////////////////////////////

// CIniFile [BEGIN] ///////////////////////////////////////////////////////////////////////////////////////////

bool CIniFile::getline(c_string& line)
{
	bool result=pFStartA!=pFEndA;
	line.clear();
	while ((pFStartA!=pFEndA)&&((*pFStartA=='\r')||(*pFStartA=='\n')))
		pFStartA++;
	while ((pFStartA!=pFEndA)&&(*pFStartA!='\r')){
		line.push_back(*pFStartA);
		pFStartA++;
	}
	line.push_back(L'\0');
	return result;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::getline(w_string& line)
{
	bool result=pFStartW!=pFEndW;
	line.clear();
	while ((pFStartW!=pFEndW)&&((*pFStartW==L'\r')||(*pFStartW==L'\n')))
		pFStartW++;
	while ((pFStartW!=pFEndW)&&(*pFStartW!=L'\r')){
		line.push_back(*pFStartW);
		pFStartW++;
	}
	line.push_back(L'\0');
	return result;
}
//-------------------------------------------------------------------------------------------------------------

CSection& CIniFile::operator[](const wchar_t* section_name)
{
	if ((last_use_indx!=-1) && !wcscoll(vec_section[last_use_indx].getNameW(),section_name))
		return vec_section[last_use_indx];
	std::vector<CSection>::iterator i=vec_section.begin();
	last_use_indx=-1;
	while ((i!=vec_section.end()) && wcscoll(i->getNameW(),section_name)){
		i++;
		last_use_indx++;
	}
	if (i!=vec_section.end())
		return *i;
	vec_section.push_back(CSection(section_name));
	return *vec_section.rbegin();
}
//-------------------------------------------------------------------------------------------------------------

CSection& CIniFile::operator[](const char* section_name)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,section_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,section_name,-1,buf,sz_buf);
		CSection& retParam=CIniFile::operator[](buf);
		delete[] buf;
		return retParam;
	}
	return CIniFile::operator[](L"");
}
//-------------------------------------------------------------------------------------------------------------

void CIniFile::deleteSection(const wchar_t* section_name)
{
	last_use_indx=-1;
	std::vector<CSection>::iterator i=vec_section.begin();
	while ((i!=vec_section.end()) && wcscoll(i->getNameW(),section_name))
		i++;
	if (i!=vec_section.end())
		vec_section.erase(i);
}
//-------------------------------------------------------------------------------------------------------------

void CIniFile::deleteSection(const char* section_name)
{
	int sz_buf=MultiByteToWideChar(CP_ACP,0,section_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,section_name,-1,buf,sz_buf);
		CIniFile::deleteSection(buf);
		delete[] buf;
	}
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::loadAnsiFile(const wchar_t* file_name)
{
	bool result=true;
	HANDLE hFile=CreateFileW(file_name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;
	HANDLE hMap=CreateFileMappingW(hFile,NULL,PAGE_READONLY,0,0,NULL);
	void* pData=MapViewOfFile(hMap,FILE_MAP_READ,0,0,0);

	int sz_cvt=256;
	wchar_t* cvt_buf=new(std::nothrow) wchar_t[sz_cvt];
	if (pData && cvt_buf){

		pFStartA=static_cast<char*>(pData);
		pFEndA=pFStartA+GetFileSize(hFile,NULL);
		c_string buf;
		w_string workbuf;
		while (getline(buf)){

			int sz_need_cvt=MultiByteToWideChar(CP_ACP,0,buf.c_str(),-1,NULL,0);
			if (sz_cvt<=sz_need_cvt){
				sz_cvt=sz_need_cvt+100;
				delete[] cvt_buf;
				cvt_buf=new(std::nothrow) wchar_t[sz_cvt];
				if (!cvt_buf){
					result=false;
					break;
				}
			}
			MultiByteToWideChar(CP_ACP,0,buf.c_str(),-1,cvt_buf,sz_cvt);
			workbuf=cvt_buf;

			w_string::size_type pB=workbuf.rfind(L'[');
			w_string::size_type pE=workbuf.find(L']');

			if (pB!=w_string::npos && pE!=w_string::npos && pB<pE)
				(*this)[workbuf.substr(pB+1,pE-pB-1).c_str()];
			else{
				pB=workbuf.find(L'=');
				if (pB!=w_string::npos){
					if (countSections())
						(*this)[countSections()-1][workbuf.substr(0,pB).c_str()]=workbuf.substr(pB+1).c_str();
					else
						(*this)[L""][workbuf.substr(0,pB).c_str()]=workbuf.substr(pB+1).c_str();
				}
			}
		}
		if (cvt_buf)
			delete[] cvt_buf;
	}
	UnmapViewOfFile(pData);
	CloseHandle(hMap);
	CloseHandle(hFile);
	return result;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::loadAnsiFile(const char* file_name)
{
	bool result=false;
	int sz_buf=MultiByteToWideChar(CP_ACP,0,file_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,file_name,-1,buf,sz_buf);
		result=CIniFile::loadAnsiFile(buf);
		delete[] buf;
	}
	return result;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::loadUnicodeFile(const wchar_t* file_name)
{
	bool result=true;
	HANDLE hFile=CreateFileW(file_name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;
	
	DWORD fSize=GetFileSize(hFile,NULL);
	if (fSize & 1)
		fSize++;
	
	HANDLE hMap=CreateFileMappingW(hFile,NULL,PAGE_READONLY,0,fSize,NULL);
	void* pData=MapViewOfFile(hMap,FILE_MAP_READ,0,0,0);
	if (pData){
		pFStartW=static_cast<wchar_t*>(pData);
		pFEndW=pFStartW+fSize/sizeof(wchar_t);
		if (*pFStartW==L'\xFEFF')
			pFStartW++;

		w_string workbuf;
		while (getline(workbuf)){

			w_string::size_type pB=workbuf.rfind(L'[');
			w_string::size_type pE=workbuf.find(L']');

			if (pB!=w_string::npos && pE!=w_string::npos && pB<pE)
				(*this)[workbuf.substr(pB+1,pE-pB-1).c_str()];
			else{
				pB=workbuf.find(L'=');
				if (pB!=w_string::npos){
					if (countSections())
						(*this)[countSections()-1][workbuf.substr(0,pB).c_str()]=workbuf.substr(pB+1).c_str();
					else
						(*this)[L""][workbuf.substr(0,pB).c_str()]=workbuf.substr(pB+1).c_str();
				}
			}
		}
	}
	UnmapViewOfFile(pData);
	CloseHandle(hMap);
	CloseHandle(hFile);
	return result;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::loadUnicodeFile(const char* file_name)
{
	bool result=false;
	int sz_buf=MultiByteToWideChar(CP_ACP,0,file_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,file_name,-1,buf,sz_buf);
		result=CIniFile::loadUnicodeFile(buf);
		delete[] buf;
	}
	return result;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::saveAnsiFile(const wchar_t* file_name)
{
	int sz_cvt=256;
	char* cvt_buf=new(std::nothrow) char[sz_cvt];
	if (!cvt_buf)
		return false;

	HANDLE hFile=CreateFileW(file_name,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;
	DWORD cntWritten, calcWritten=0, totalWritten=0;
	w_string buffer;

	for (int isctn=0; isctn<countSections(); isctn++){
		buffer=L"\r\n[";
		buffer+=vec_section[isctn].getNameW();
		buffer+=L"]\r\n";

		int sz_need_cvt=WideCharToMultiByte(CP_ACP,0,buffer.c_str(),-1,NULL,0,NULL,NULL);
		if (sz_cvt<=sz_need_cvt){
			sz_cvt=sz_need_cvt+100;
			delete[] cvt_buf;
			cvt_buf=new(std::nothrow) char[sz_cvt];
			if (!cvt_buf){
				calcWritten=-1;
				break;
			}
		}
		sz_need_cvt=WideCharToMultiByte(CP_ACP,0,buffer.c_str(),-1,cvt_buf,sz_cvt,NULL,NULL)-1;
		WriteFile(hFile,cvt_buf,sz_need_cvt,&cntWritten,NULL);
		calcWritten+=sz_need_cvt;
		totalWritten+=cntWritten;

		int cnt_params=vec_section[isctn].countParams();
		for (int iprms=0; iprms<cnt_params; iprms++){
			buffer=vec_section[isctn][iprms].getNameW();
			buffer+=L"=";
			buffer+=vec_section[isctn][iprms];
			buffer+=L"\r\n";

			int sz_need_cvt=WideCharToMultiByte(CP_ACP,0,buffer.c_str(),-1,NULL,0,NULL,NULL);
			if (sz_cvt<=sz_need_cvt){
				sz_cvt=sz_need_cvt+100;
				delete[] cvt_buf;
				cvt_buf=new(std::nothrow) char[sz_cvt];
			}
			sz_need_cvt=WideCharToMultiByte(CP_ACP,0,buffer.c_str(),-1,cvt_buf,sz_cvt,NULL,NULL)-1;
			WriteFile(hFile,cvt_buf,sz_need_cvt,&cntWritten,NULL);
			calcWritten+=sz_need_cvt;
			totalWritten+=cntWritten;
		}
	}
	if (cvt_buf)
		delete[] cvt_buf;
	CloseHandle(hFile);
	return calcWritten==totalWritten;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::saveAnsiFile(const char* file_name)
{
	bool result=false;
	int sz_buf=MultiByteToWideChar(CP_ACP,0,file_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,file_name,-1,buf,sz_buf);
		result=CIniFile::saveAnsiFile(buf);
		delete[] buf;
	}
	return result;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::saveUnicodeFile(const wchar_t* file_name)
{
	HANDLE hFile=CreateFileW(file_name,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;
	DWORD cntWritten, calcWritten=0, totalWritten=0;
	w_string buffer;

	WriteFile(hFile,"\xFF\xFE",2,&cntWritten,NULL);
	calcWritten+=2;
	totalWritten+=cntWritten;

	for (int isctn=0; isctn<countSections(); isctn++){
		buffer=L"\r\n[";
		buffer+=vec_section[isctn].getNameW();
		buffer+=L"]\r\n";

		WriteFile(hFile,buffer.c_str(),static_cast<DWORD>(buffer.size()*sizeof(wchar_t)),&cntWritten,NULL);
		calcWritten+=static_cast<DWORD>(buffer.size()*sizeof(wchar_t));
		totalWritten+=cntWritten;

		int cnt_params=vec_section[isctn].countParams();
		for (int iprms=0; iprms<cnt_params; iprms++){
			buffer=vec_section[isctn][iprms].getNameW();
			buffer+=L"=";
			buffer+=vec_section[isctn][iprms];
			buffer+=L"\r\n";

			WriteFile(hFile,buffer.c_str(),static_cast<DWORD>(buffer.size()*sizeof(wchar_t)),&cntWritten,NULL);
			calcWritten+=static_cast<DWORD>(buffer.size()*sizeof(wchar_t));
			totalWritten+=cntWritten;
		}
	}
	CloseHandle(hFile);
	return calcWritten==totalWritten;
}
//-------------------------------------------------------------------------------------------------------------

bool CIniFile::saveUnicodeFile(const char* file_name)
{
	bool result=false;
	int sz_buf=MultiByteToWideChar(CP_ACP,0,file_name,-1,NULL,0);
	wchar_t* buf=new(std::nothrow) wchar_t[sz_buf];
	if (buf){
		MultiByteToWideChar(CP_ACP,0,file_name,-1,buf,sz_buf);
		result=CIniFile::saveUnicodeFile(buf);
		delete[] buf;
	}
	return result;
}
// CIniFile [END] /////////////////////////////////////////////////////////////////////////////////////////////
