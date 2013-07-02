/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  CSingleton.h
///
///  singleton pattern
///
///  @author "Archer Software" Sogin M. @date 24.11.2006
///
////////////////////////////////////////////////////////////////////////

#pragma once
#include <AidLib/Logging/cLog.h>

template <typename T>
class CSingleton
{
	CSingleton();
	CSingleton(const CSingleton&);
	CSingleton& operator=(const CSingleton&);
public:
	static T& instance()
	{
		static T obj;
		return obj;
	}
};


///create singleton object within process
//TODO Who first request object, delete its, but other which gotten pointer of it, don’t know about this.
//TODO cLogSingletonProcess are not supported multi type singleton
template <typename T>
class CProcessSingleton
{
	/// This object created only in pair with FileMapping, from module, initiated logging
	/// therefore it's deleted in the same module, and it's assumed, 
	/// that module initiated logging is deleted latest
	class CMapViewDeleter
	{	
		HANDLE m_h;
	public:
		CMapViewDeleter(HANDLE h)
			:	m_h(h)
		{}
		~CMapViewDeleter()
		{
			// Doesn't deleting singleton object, since we do not know how long log will exists

			// As we not planning to use win9x - this not a memory leak
			void *pp=::MapViewOfFile(m_h,FILE_MAP_WRITE,0,0,0);
			::UnmapViewOfFile(pp);
			::CloseHandle(m_h);
		}
	};
	CProcessSingleton();
	CProcessSingleton(const CProcessSingleton&);
	CProcessSingleton& operator=(const CProcessSingleton&);
public:
	static T& instance()
	{
		static T* pObj=NULL;
		try
		{
			if (pObj==NULL) 
			{
				//pointer of object are not initiated in this module yet
				HANDLE hFileMap;
				//TODO ask Dima N What the reason of Global name space?
				//tstring sysObjStr = Format(_T("Global\\CProcessSingleton_%s-%08X"),__FUNCTION__,GetCurrentProcessId());
				tstring sysObjStr = Format(_T("CProcessSingleton_%s-%08X"),__FUNCTION__,GetCurrentProcessId());
				if((hFileMap=::CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT,0,sizeof(T*),sysObjStr.c_str()))!=NULL)
				{
					//object creation is success
					DWORD res=::GetLastError();
					T** ppObj=reinterpret_cast<T**>(::MapViewOfFile(hFileMap,FILE_MAP_WRITE,0,0,0));
					//TODO check the pointer
					if(res!=ERROR_ALREADY_EXISTS)
					{
						pObj=new T();
						*ppObj=pObj;
						static CMapViewDeleter ppp(hFileMap);
					}else
					{
						pObj=*ppObj;
						::CloseHandle(hFileMap);
					}
					::UnmapViewOfFile(ppObj);
				} else
				{
					throw false;
				}
			}
		}
		catch(...)
		{
			static int reportedOnce = false;
			if (!reportedOnce)
			{
				reportedOnce = true;
				tstring message = Format(_T("Failed to create CProcessSingleton instance for ID(%s), creating as not process singleton (unique for dll only)..."),__FUNCTION__);
				cDbgOutLog().WinError(_ERROR_,message.c_str());
				cFileLog().WinError(_ERROR_,message.c_str());
			}
			if (pObj==NULL)
			{
				pObj=new T();
			}
		}
		return *pObj;
	}
};