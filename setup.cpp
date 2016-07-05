// setup.cpp --- MZ-IME setup program
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <cstring>

//////////////////////////////////////////////////////////////////////////////

LPCTSTR DoLoadString(HINSTANCE hInstance, INT nID) {
  static WCHAR s_szBuf[1024];
  s_szBuf[0] = 0;
  LoadStringW(hInstance, nID, s_szBuf, 1024);
  return s_szBuf[0] ? s_szBuf : L"Internal Error";
}

INT DoCopyFiles(VOID) {
  WCHAR szPathSrc[MAX_PATH], szPathDest[MAX_PATH], *pch;

  // source
  GetModuleFileName(NULL, szPathSrc, MAX_PATH);
  pch = wcsrchr(szPathSrc, L'\\');
  lstrcpy(pch, L"\\mzimeja.dic");
  // dest
  GetWindowsDirectory(szPathDest, MAX_PATH);
  wcscat(szPathDest, L"\\mzimeja.dic");
  // copy
  BOOL b0 = CopyFile(szPathSrc, szPathDest, FALSE);
  if (!b0) {
    return 1;
  }

  // source
  GetModuleFileName(NULL, szPathSrc, MAX_PATH);
  pch = wcsrchr(szPathSrc, L'\\');
  lstrcpy(pch, L"\\mzimeja.ime");
  // dest
  GetSystemDirectory(szPathDest, MAX_PATH);
  wcscat(szPathDest, L"\\mzimeja.ime");
  // copy
  BOOL b1 = CopyFile(szPathSrc, szPathDest, FALSE);
  if (!b1) {
    return 2;
  }

  return 0;
}

BOOL DoSetRegSz(HKEY hKey, const WCHAR *pszName, const WCHAR *pszValue) {
  DWORD cbData = (lstrlenW(pszValue) + 1) * sizeof(WCHAR);
  LONG result;
  result = RegSetValueExW(hKey, pszName, 0, REG_SZ, (BYTE *)pszValue, cbData);
  return result == ERROR_SUCCESS;
}

INT DoSetRegistry(VOID) {
  BOOL ret = FALSE;
  HKEY hKey;
  LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
    L"system\\currentcontrolset\\control\\keyboard layouts",
    0, KEY_WRITE, &hKey);
  if (result == ERROR_SUCCESS && hKey != NULL) {
    HKEY hkLayouts;
    DWORD dwDisposition;
    result = RegCreateKeyExW(hKey, L"E0120411", 0, NULL, 0,
                             KEY_WRITE, NULL, &hkLayouts, &dwDisposition);
    if (result == ERROR_SUCCESS && hkLayouts) {
      if (DoSetRegSz(hkLayouts, L"layout file", L"kbdjp.kbd") &&
        DoSetRegSz(hkLayouts, L"layout text", L"“ú–{Œê (MZ-IME)") &&
        DoSetRegSz(hkLayouts, L"layout file", L"kbdjp.kbd") &&
        DoSetRegSz(hkLayouts, L"IME file", L"mzimeja.ime"))
      {
        
        ret = TRUE;
      }
      RegCloseKey(hkLayouts);
    }
    RegCloseKey(hKey);
  }
  return (ret ? 0 : -1);
}

//////////////////////////////////////////////////////////////////////////////

extern "C"
INT WINAPI
wWinMain(
  HINSTANCE hInstance,
  HINSTANCE HPrevInstance,
  LPWSTR    lpCmdLine,
  INT       nCmdShow)
{
  if (0 != DoSetRegistry()) {
    ::MessageBoxW(NULL, DoLoadString(hInstance, 2), NULL, MB_ICONERROR);
    return 2;
  }

  if (0 != DoCopyFiles()) {
    ::MessageBoxW(NULL, DoLoadString(hInstance, 3), NULL, MB_ICONERROR);
    return 1;
  }

  WCHAR szPath[MAX_PATH];
  ::GetSystemDirectoryW(szPath, MAX_PATH);
  wcscat(szPath, L"\\mzimeja.ime");
  if (!ImmInstallIME(szPath, DoLoadString(hInstance, 4))) {
    WCHAR szMsg[128];
    DWORD dwError = ::GetLastError();
    ::wsprintfW(szMsg, DoLoadString(hInstance, 5), dwError);
    ::MessageBoxW(NULL, szMsg, NULL, MB_ICONERROR);
    return 3;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
