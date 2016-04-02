/*++

Copyright (c) 1990-1998 Microsoft Corporation, All Rights Reserved

Module Name:

    DEBUG.C

++*/

#include "mzimeja.h"

extern "C" {

#ifdef _DEBUG

const LPTSTR g_szRegInfoPath = TEXT("software\\Katayama Hirofumi MZ\\mzimeja");

int DebugPrint(LPCTSTR lpszFormat, ...) {
  int nCount;
  TCHAR szMsg[1024];

  va_list marker;
  va_start(marker, lpszFormat);
  nCount = wvsprintf(szMsg, lpszFormat, marker);
  va_end(marker);
  OutputDebugString(szMsg);
  return nCount;
}

DWORD PASCAL GetDwordFromSetting(LPCTSTR lpszFlag) {
  HKEY hkey;
  DWORD dwRegType, dwData, dwDataSize, dwRet;

  dwData = 0;
  dwDataSize = sizeof(DWORD);
  if (ERROR_SUCCESS ==
      RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szRegInfoPath, 0, KEY_READ, &hkey)) {
    dwRet = RegQueryValueEx(hkey, lpszFlag, NULL, &dwRegType, (LPBYTE)&dwData,
                            &dwDataSize);
    RegCloseKey(hkey);
  }
  MyDebugPrint(
      (TEXT("Getting: %s=%#8.8x: dwRet=%#8.8x\n"), lpszFlag, dwData, dwRet));
  return dwData;
}

void SetDwordToSetting(LPCTSTR lpszFlag, DWORD dwFlag) {
  HKEY hkey;
  DWORD dwRet;

  if (ERROR_SUCCESS ==
      RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szRegInfoPath, 0, KEY_WRITE, &hkey)) {
    dwRet = RegSetValueEx(hkey, lpszFlag, 0, REG_DWORD, (CONST BYTE *)&dwFlag,
                          sizeof(DWORD));
    RegCloseKey(hkey);
  }
  MyDebugPrint(
      (TEXT("Setting: %s=%#8.8x: dwRet=%#8.8x\n"), lpszFlag, dwFlag, dwRet));
}

void PASCAL SetGlobalFlags() {
  dwLogFlag = GetDwordFromSetting(TEXT("LogFlag"));
  dwDebugFlag = GetDwordFromSetting(TEXT("DebugFlag"));
}

void PASCAL ImeLog(DWORD dwFlag, LPCTSTR lpStr) {
  TCHAR szBuf[80];

  if (dwFlag & dwLogFlag) {
    if (dwDebugFlag & DEBF_THREADID) {
      DWORD dwThreadId = GetCurrentThreadId();
      wsprintf(szBuf, TEXT("ThreadID = %X "), dwThreadId);
      OutputDebugString(szBuf);
    }

    OutputDebugString(lpStr);
    OutputDebugString(TEXT("\r\n"));
  }
}

#endif  // _DEBUG

}  // extern "C"
