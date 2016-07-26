// verinfo.cpp --- MZ-IME version info program
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS   // use fopen
#include "targetver.h"      // target Windows version

#define NOMINMAX
#include <windows.h>
#include <dlgs.h>
#include <cstdlib>    // for __argc, __wargv
#include <cstring>    // for wcsrchr
#include <algorithm>  // for std::max

HINSTANCE g_hInstance;

//////////////////////////////////////////////////////////////////////////////

LPCTSTR DoLoadString(INT nID) {
  static WCHAR s_szBuf[1024];
  s_szBuf[0] = 0;
  LoadStringW(g_hInstance, nID, s_szBuf, 1024);
  return s_szBuf[0] ? s_szBuf : L"Internal Error";
}

//////////////////////////////////////////////////////////////////////////////

void OnPsh1(HWND hWnd) {
  ::ShellExecute(hWnd, NULL, DoLoadString(2), NULL, NULL, SW_SHOWNORMAL);
}

//////////////////////////////////////////////////////////////////////////////

extern "C"
INT_PTR CALLBACK
DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_INITDIALOG:
    {
      HICON hIcon = ::LoadIcon(g_hInstance, MAKEINTRESOURCE(1));
      ::SendDlgItemMessage(hWnd, ico1, STM_SETICON, (WPARAM)hIcon, 0);
    }
    return TRUE;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
      ::EndDialog(hWnd, IDOK);
      break;
    case IDCANCEL:
      ::EndDialog(hWnd, IDCANCEL);
      break;
    case psh1:
      OnPsh1(hWnd);
      break;
    }
  }
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////////

extern "C"
INT WINAPI
wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR    lpCmdLine,
  INT       nCmdShow)
{
  g_hInstance = hInstance;

  LPCTSTR pszTitle = DoLoadString(1);

  HWND hWnd = ::FindWindow(WC_DIALOG, pszTitle);
  if (hWnd) {
    ::SetForegroundWindow(hWnd);
    ::ShowWindow(hWnd, SW_SHOWNORMAL);
  } else {
    ::DialogBoxW(hInstance, MAKEINTRESOURCEW(1), NULL, DialogProc);
  }

  return 0;
} // wWinMain

//////////////////////////////////////////////////////////////////////////////
