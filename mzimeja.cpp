// mzimeja.cpp --- MZ-IME Japanese Input
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "mzimeja.h"
#include "immsec.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

const TCHAR szUIClassName[]       = TEXT("MZIMEUUI");
const TCHAR szCompStrClassName[]  = TEXT("MZIMEUCompStr");
const TCHAR szCandClassName[]     = TEXT("MZIMEUCand");
const TCHAR szStatusClassName[]   = TEXT("MZIMEUStatus");
const TCHAR szGuideClassName[]    = TEXT("MZIMEUGuide");

const MZGUIDELINE glTable[] = {
  {GL_LEVEL_ERROR, GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
  {GL_LEVEL_WARNING, GL_ID_TYPINGERROR, IDS_GL_TYPINGERROR, 0},
  {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST, IDS_GL_TESTGUIDELINESTR,
   IDS_GL_TESTGUIDELINEPRIVATE}
};

//////////////////////////////////////////////////////////////////////////////

MZIMEJA TheApp;

BOOL MZIMEJA::Init(HINSTANCE hInstance) {
  m_hInst = hInstance;

  // Create/open a system global named mutex.
  // The initial ownership is not needed.
  // CreateSecurityAttributes() will create
  // the proper security attribute for IME.
  PSECURITY_ATTRIBUTES psa = CreateSecurityAttributes();
  if (psa != NULL) {
    m_hMutex = CreateMutex(psa, FALSE, TEXT("mzimeja_mutex"));
    FreeSecurityAttributes(psa);
    assert(TheApp.m_hMutex);
  } else {
    // Failed, not NT system
    assert(0);
    return FALSE;
  }

  IMERegisterClasses(TheApp.m_hInst);

  LPTSTR lpDicFileName = TheApp.m_szDicFileName;
  lpDicFileName += GetWindowsDirectory(lpDicFileName, 256);
  if (*(lpDicFileName - 1) != TEXT('\\')) *lpDicFileName++ = TEXT('\\');
  LoadString(hInstance, IDS_DICFILENAME, lpDicFileName, 128);
  return TRUE;
}

VOID MZIMEJA::Destroy(VOID) {
  ::UnregisterClass(szUIClassName, m_hInst);
  ::UnregisterClass(szCompStrClassName, m_hInst);
  ::UnregisterClass(szCandClassName, m_hInst);
  ::UnregisterClass(szStatusClassName, m_hInst);
  if (m_hMutex) ::CloseHandle(m_hMutex);
}

HBITMAP MZIMEJA::LoadBMP(LPCTSTR pszName) {
  return ::LoadBitmap(m_hInst, pszName);
}

LPTSTR MZIMEJA::LoadSTR(INT nID) {
  static TCHAR sz[512];
  sz[0] = 0;
  ::LoadString(m_hInst, nID, sz, 512);
  return sz;
}

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////
// for debugging

#ifdef _DEBUG
int DebugPrint(LPCTSTR lpszFormat, ...) {
  int nCount;
  TCHAR szMsg[1024];

  va_list marker;
  va_start(marker, lpszFormat);
  nCount = wvsprintf(szMsg, lpszFormat, marker);
  va_end(marker);

  //OutputDebugString(szMsg);
  FILE *fp = fopen("C:\\mzimeja.log", "ab");
  if (fp) {
#ifdef UNICODE
    CHAR szAnsi[1024];
    szAnsi[0] = 0;
    WideCharToMultiByte(CP_ACP, 0, szMsg, -1, szAnsi, 1024, NULL, NULL);
    fprintf(fp, "%s\r\n", szAnsi);
#else
    fprintf(fp, "%s\r\n", szMsg);
#endif
    fclose(fp);
  }
  return nCount;
}
#endif  // def _DEBUG

#ifdef _DEBUG
VOID WarnOut(LPCTSTR pStr) {
  DebugPrint(TEXT("%s"), pStr);
}
#endif  // def _DEBUG

#ifdef _DEBUG
VOID ErrorOut(LPCTSTR pStr) {
  DWORD dwError;
  DWORD dwResult;
  TCHAR buf1[512];

  dwError = GetLastError();
  dwResult =
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
                    MAKELANGID(LANG_ENGLISH, LANG_NEUTRAL), buf1, 512, NULL);

  if (dwResult > 0) {
    DebugPrint(TEXT("%s:%s(0x%x)"), pStr, buf1, dwError);
  } else {
    DebugPrint(TEXT("%s:(0x%x)"), pStr, dwError);
  }
}
#endif  // def _DEBUG

//////////////////////////////////////////////////////////////////////////////
// DLL entry point

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwFunction, LPVOID lpNot) {
  DebugPrint(TEXT("DLLEntry:dwFunc=%d\n"), dwFunction);

  switch (dwFunction) {
    case DLL_PROCESS_ATTACH:
      TheApp.Init(hInstDLL);
      DebugPrint(TEXT("DLLEntry Process Attach hInst is %lx"), TheApp.m_hInst);
      break;

    case DLL_PROCESS_DETACH:
      TheApp.Destroy();
      DebugPrint(TEXT("DLLEntry Process Detach hInst is %lx"), TheApp.m_hInst);
      break;

    case DLL_THREAD_ATTACH:
      DebugPrint(TEXT("DLLEntry Thread Attach hInst is %lx"), TheApp.m_hInst);
      break;

    case DLL_THREAD_DETACH:
      DebugPrint(TEXT("DLLEntry Thread Detach hInst is %lx"), TheApp.m_hInst);
      break;
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
