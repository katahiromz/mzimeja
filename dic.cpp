// dic.cpp
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "mzimeja.h"
#include "immsec.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

int CopyCandidateStringsFromDictionary(LPTSTR lpDic, LPTSTR lpRead,
                                       LPTSTR lpBuf, DWORD dwBufLen) {
  FOOTMARK();
  DWORD dwWritten = 0;
  LPTSTR lpSection, lpTemp;
  const LPTSTR szSep = TEXT(" \r\n\t");

  LPTSTR lpToken = wcstok(lpDic, szSep);
  while (NULL != lpToken) {
    if (TEXT('[') == *lpToken) {
      lpSection = lpToken + 1;
      if (NULL != (lpTemp = wcschr(lpSection, TEXT(']'))))
        *lpTemp = 0;
      if (0 == lstrcmp(lpSection, lpRead)) {
        lpToken = wcstok(NULL, szSep);
        break;  // found it.
      }
    }
    lpToken = wcstok(NULL, szSep);
  }
  if (NULL != lpToken) {
    LPTSTR lpWrite = lpBuf;
    DWORD dwW;
    while ((NULL != lpToken) && ((dwBufLen - dwWritten) > 1) &&
           (TEXT('[') != *lpToken)) {
      if (NULL != (lpTemp = wcschr(lpToken, TEXT('='))))
        *lpTemp = 0;
      lstrcpyn(lpWrite, lpToken, dwBufLen - dwWritten - 1);
      dwW = lstrlen(lpToken) + 1;
      lpWrite += dwW;
      dwWritten += dwW;
      lpToken = wcstok(NULL, szSep);
    }
    *lpWrite = 0;
    dwWritten++;
    return dwWritten;
  }
  return 0;
}

int GetCandidateStringsFromDictionary(LPTSTR lpRead, LPTSTR lpBuf,
                                      DWORD dwBufLen, LPTSTR lpFilename) {
  HANDLE hTblFile;
  PSECURITY_ATTRIBUTES psa;
  int nSize = 0;
  DWORD dwFileSize, dwRead;
  LPTSTR lpDic;

  FOOTMARK();
  psa = CreateSecurityAttributes();

  hTblFile = CreateFile(lpFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hTblFile == INVALID_HANDLE_VALUE) {
    goto Err0;
  }

  if (dwBufLen > 2) {
    if ((dwFileSize = GetFileSize(hTblFile, NULL)) != 0xffffffff) {
      if ((lpDic = (LPTSTR)GlobalAlloc(GPTR, dwFileSize + 2))) {
        if (ReadFile(hTblFile, lpDic, dwFileSize, &dwRead, NULL)) {
          if (*lpDic == 0xfeff) {
            *(LPWSTR)(((LPBYTE)lpDic) + dwFileSize) = 0;
            nSize = CopyCandidateStringsFromDictionary(lpDic + 1, lpRead, lpBuf,
                                                       dwBufLen);
          }
        }
        GlobalFree(lpDic);
      }
    }
  }

  CloseHandle(hTblFile);

Err0:
  FreeSecurityAttributes(psa);
  return nSize;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
