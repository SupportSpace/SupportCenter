#include "StdAfx.h"
#include "AboutDlgInfo.h"

CAboutDlgInfo::CAboutDlgInfo(void)
{
	ReadSupportMessngerVersion();
}

CAboutDlgInfo::~CAboutDlgInfo(void)
{
}

void CAboutDlgInfo::ReadSupportMessngerVersion()
{
    TCHAR m_sFileName[_MAX_PATH];
    if (::GetModuleFileName(NULL, m_sFileName, sizeof(m_sFileName)))
    {
        DWORD hHandle;
        DWORD dwSize = ::GetFileVersionInfoSize((LPTSTR)(LPCTSTR)m_sFileName, &hHandle);
        if (dwSize)
        {
            TCHAR* abData = new TCHAR[dwSize];
            if (::GetFileVersionInfo((LPTSTR)(LPCTSTR)m_sFileName, hHandle, dwSize, abData))
            {
                LPBYTE lpBuffer = NULL;
                ::VerQueryValue(abData, _T("\\VarFileInfo\\Translation"), (void**)&lpBuffer, (UINT *)&dwSize);
                if (dwSize && lpBuffer)
                {
                    DWORD dwResult = *(unsigned long *)lpBuffer;
                    WORD  w1 = HIWORD(dwResult);
                    WORD  w2 = LOWORD(dwResult);

                    TCHAR szName[512];
                    LPBYTE lpData = NULL;
                    CString sFormat;

                    wsprintf(szName, _T("\\StringFileInfo\\%04X%04X\\InternalName"), w2, w1);
                    if (::VerQueryValue(abData, szName, (void **)&lpData, (UINT *)&dwSize))
                    {
                        //sFormat = lpData;
                        //sFormat += _T("\n");
                    }
                    wsprintf(szName, _T("\\StringFileInfo\\%04X%04X\\ProductName"), w2, w1);
                    if (::VerQueryValue(abData, szName, (void **)&lpData, (UINT *)&dwSize))
                    {
                        sFormat = lpData;
                        sFormat += _T(", (Ver %s)\n");
                    }
                    wsprintf(szName, _T("\\StringFileInfo\\%04X%04X\\LegalCopyright"), w2, w1);
                    if (::VerQueryValue(abData, szName, (void **)&lpData, (UINT *)&dwSize))
                    {
                        sFormat += (CString)lpData;
                    }
                    wsprintf(szName, _T("\\StringFileInfo\\%04X%04X\\FileVersion"), w2, w1);
                    if (::VerQueryValue(abData, szName, (void **)&lpData, (UINT *)&dwSize))
                    {
                        CString s = (CString)lpData;
						/* todo - UI would like to get version number only
						do
						{
							int Index = s.Find(',');
							if (Index == -1)	break;
							s.SetAt(Index, '.');
							s.Delete(Index+1, 1);
						}while(true);
                        m_csVersion.Format(sFormat, s);
						*/
						m_csVersion = s;
                    }
                }
            }
            delete [] abData;
        }
    }
}
