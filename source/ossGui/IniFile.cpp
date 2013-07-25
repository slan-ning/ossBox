// IniFile.cpp  
#include "stdafx.h"
#include "IniFile.h"   
  
void CIniFile::Init()  
{  
    m_unMaxSection = 512;  
    m_unSectionNameMaxSize = 33; // 32位UID串   
}  
  
CIniFile::CIniFile()  
{  
    Init();  
}  
  
CIniFile::CIniFile(LPCTSTR szFileName)  
{  
    // (1) 绝对路径，需检验路径是否存在   
    // (2) 以"./"开头，则需检验后续路径是否存在   
    // (3) 以"../"开头，则涉及相对路径的解析   
      
    Init();  
  
    // 相对路径   
    m_szFileName.Format("%s", szFileName);  
}  
  
CIniFile::~CIniFile()    
{  
      
}  
  
void CIniFile::SetFileName(LPCTSTR szFileName)  
{  
    m_szFileName.Format(".//%s", szFileName);  
}  
  
DWORD CIniFile::GetProfileSectionNames(CStringArray &strArray)  
{  
    int nAllSectionNamesMaxSize = m_unMaxSection*m_unSectionNameMaxSize+1;  
    char *pszSectionNames = new char[nAllSectionNamesMaxSize];  
    DWORD dwCopied = 0;  
    dwCopied = ::GetPrivateProfileSectionNames(pszSectionNames, nAllSectionNamesMaxSize, m_szFileName);  
      
    strArray.RemoveAll();  
  
    char *pSection = pszSectionNames;  
    do   
    {  
        CString szSection(pSection);  
        if (szSection.GetLength() < 1)  
        {  
            delete[] pszSectionNames;  
            return dwCopied;  
        }  
        strArray.Add(szSection);  
  
        pSection = pSection + szSection.GetLength() + 1; // next section name   
    } while (pSection && pSection<pszSectionNames+nAllSectionNamesMaxSize);  
  
    delete[] pszSectionNames;  
    return dwCopied;  
}  
  
DWORD CIniFile::GetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, CString& szKeyValue)  
{  
    DWORD dwCopied = 0;  
    dwCopied = ::GetPrivateProfileString(lpszSectionName, lpszKeyName, "",   
        szKeyValue.GetBuffer(MAX_PATH), MAX_PATH, m_szFileName);  
    szKeyValue.ReleaseBuffer();  
  
    return dwCopied;  
}  
  
int CIniFile::GetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName)  
{  
    int nKeyValue = ::GetPrivateProfileInt(lpszSectionName, lpszKeyName, 0, m_szFileName);  
      
    return nKeyValue;  
}  
  
BOOL CIniFile::SetProfileString(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, LPCTSTR lpszKeyValue)  
{  
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, lpszKeyValue, m_szFileName);  
}  
  
BOOL CIniFile::SetProfileInt(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName, int nKeyValue)  
{  
    CString szKeyValue;  
    szKeyValue.Format("%d", nKeyValue);  
  
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, szKeyValue, m_szFileName);  
}  
  
BOOL CIniFile::DeleteSection(LPCTSTR lpszSectionName)  
{  
    return ::WritePrivateProfileSection(lpszSectionName, NULL, m_szFileName);  
}  
  
BOOL CIniFile::DeleteKey(LPCTSTR lpszSectionName, LPCTSTR lpszKeyName)  
{  
    return ::WritePrivateProfileString(lpszSectionName, lpszKeyName, NULL, m_szFileName);  
}  