/*
	Объявление классов для работы с файлами инициализации и данными реестра
	ГКБ "Южное"
	Александр Новак
	14.10.2005
*/

#ifndef _CB97EFC4_D6CB_4ba7_8E85_A8EDDBB91CE2_
#define _CB97EFC4_D6CB_4ba7_8E85_A8EDDBB91CE2_

#include <vector>
#include <string>
#include <tchar.h>
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
	#define getName		getNameW
#else
	#define getName		getNameA
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCommandLinePath{
	TCHAR* dir;
public:
	CCommandLinePath(const TCHAR* cmd_line=0);
	~CCommandLinePath();
	void SetCommandLine(const TCHAR* cmd_line);
	const TCHAR* GetDir() const;
};
//-------------------------------------------------------------------------------------------------------------

inline const TCHAR* CCommandLinePath::GetDir() const
{
	return dir;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSection;

class CIniReg{
	friend class CParam;
	typedef std::basic_string<wchar_t> w_string;
	int last_use_indx;
	HKEY hPriKey;
	std::vector<CSection> vec_section;
	void enumRegValues(HKEY hKey, const wchar_t* section);
	void Init(HKEY hKey, const wchar_t* subKey);
	CIniReg(const CIniReg& param);
	CIniReg& operator=(const CIniReg& param);
public:
	CIniReg(HKEY hKey, const wchar_t* subKey);
	CIniReg(HKEY hKey, const char* subKey);
	virtual ~CIniReg();
	CSection& operator[](const wchar_t* section_name);
	CSection& operator[](const char* section_name);
	CSection& operator[](int section_index);
	bool save();
	void deleteSection(const wchar_t* section_name);
	void deleteSection(const char* section_name);
	int countSections() const;
};
//-------------------------------------------------------------------------------------------------------------

inline CIniReg::CIniReg(HKEY hKey, const wchar_t* subKey)
{
	Init(hKey,subKey);
}
//-------------------------------------------------------------------------------------------------------------

inline int CIniReg::countSections() const
{
	return static_cast<int>(vec_section.size());
}
//-------------------------------------------------------------------------------------------------------------

inline CSection& CIniReg::operator[](int section_index)
{
	return vec_section[section_index];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CParam{
	typedef std::basic_string<wchar_t> w_string;
	bool m_modify;
	long m_long;
	double m_double;
	w_string m_name;
	w_string m_value;
	int sz_rawdata;
	BYTE* m_rawdata;
	DWORD last_take_type;
	mutable char* m_cvtbuf;
	friend void CIniReg::enumRegValues(HKEY, const wchar_t*);
public:
	CParam(const wchar_t* name);
	CParam(const char* name);
	CParam(const CParam& param);
	virtual ~CParam();
	CParam& operator=(const CParam& param);
	CParam& operator=(long value);
	CParam& operator=(int value);
	CParam& operator=(double value);
	CParam& operator=(const wchar_t* value);
	CParam& operator=(const char* value);
	operator long() const;
	operator int() const;
	operator double() const;
	operator bool() const;
	operator const wchar_t*() const;
	operator const char*() const;
	int setRawData(const void* data, int sz_data);
	int getRawData(void* data, int sz_data) const;
	bool isModify() const;
	DWORD lastType() const;
	const wchar_t* getNameW() const;
	const char* getNameA() const;
};
//-------------------------------------------------------------------------------------------------------------

inline bool CParam::isModify() const
{
	return m_modify;
}
//-------------------------------------------------------------------------------------------------------------

inline DWORD CParam::lastType() const
{
	return last_take_type;
}
//-------------------------------------------------------------------------------------------------------------

inline const wchar_t* CParam::getNameW() const
{
	return m_name.c_str();
}
//-------------------------------------------------------------------------------------------------------------

inline CParam& CParam::operator=(int value)
{
	return *this=static_cast<long>(value);
}
//-------------------------------------------------------------------------------------------------------------

inline CParam::operator long() const
{
	return m_long;
}
//-------------------------------------------------------------------------------------------------------------

inline CParam::operator int() const
{
	return static_cast<int>(m_long);
}
//-------------------------------------------------------------------------------------------------------------

inline CParam::operator double() const
{
	return m_double;
}
//-------------------------------------------------------------------------------------------------------------

inline CParam::operator bool() const
{
	return m_long!=0;
}
//-------------------------------------------------------------------------------------------------------------

inline CParam::operator const wchar_t*() const
{
	return m_value.c_str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSection{
	typedef std::basic_string<wchar_t> w_string;
	int last_use_indx;
	w_string m_name;
	std::vector<CParam> vec_param;
	mutable char* m_cvtbuf;
public:
	CSection(const wchar_t* name);
	CSection(const char* name);
	CSection(const CSection& param);
	virtual ~CSection();
	CSection& operator=(const CSection& param);
	CParam& operator[](const wchar_t* param_name);
	CParam& operator[](const char* param_name);
	CParam& operator[](int param_index);
	const wchar_t* getNameW() const;
	const char* getNameA() const;
	void deleteParam(const wchar_t* param_name);
	void deleteParam(const char* param_name);
	int countParams() const;
};
//-------------------------------------------------------------------------------------------------------------

inline const wchar_t* CSection::getNameW() const
{
	return m_name.c_str();
}
//-------------------------------------------------------------------------------------------------------------

inline int CSection::countParams() const
{
	return static_cast<int>(vec_param.size());
}
//-------------------------------------------------------------------------------------------------------------

inline CParam& CSection::operator[](int param_index)
{
	return vec_param[param_index];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CIniFile{
	typedef std::basic_string<char> c_string;
	typedef std::basic_string<wchar_t> w_string;
	int last_use_indx;
	std::vector<CSection> vec_section;
	char* pFStartA;
	char* pFEndA;
	wchar_t* pFStartW;
	wchar_t* pFEndW;
	bool getline(c_string& line);
	bool getline(w_string& line);
	CIniFile(const CIniFile& param);
	CIniFile& operator=(const CIniFile& param);
public:
	CIniFile():last_use_indx(-1){}
	virtual ~CIniFile(){}
	CSection& operator[](const wchar_t* section_name);
	CSection& operator[](const char* section_name);
	CSection& operator[](int section_index);
	bool loadAnsiFile(const wchar_t* file_name);
	bool loadAnsiFile(const char* file_name);
	bool loadUnicodeFile(const wchar_t* file_name);
	bool loadUnicodeFile(const char* file_name);
	bool saveAnsiFile(const wchar_t* file_name);
	bool saveAnsiFile(const char* file_name);
	bool saveUnicodeFile(const wchar_t* file_name);
	bool saveUnicodeFile(const char* file_name);
	void deleteSection(const wchar_t* section_name);
	void deleteSection(const char* section_name);
	int countSections() const;
};
//-------------------------------------------------------------------------------------------------------------

inline int CIniFile::countSections() const
{
	return static_cast<int>(vec_section.size());
}
//-------------------------------------------------------------------------------------------------------------

inline CSection& CIniFile::operator[](int section_index)
{
	return vec_section[section_index];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_CB97EFC4_D6CB_4ba7_8E85_A8EDDBB91CE2_
