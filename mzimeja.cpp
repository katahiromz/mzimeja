// mzimeja.cpp --- MZ-IME Japanese Input
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "immsec.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

HINSTANCE hInst;
HANDLE hMutex = NULL;
HKL hMyKL = 0;

/* for Translat */
LPTRANSMSGLIST lpCurTransKey = NULL;
UINT uNumTransKey;
BOOL fOverTransKey = FALSE;

TCHAR szUIClassName[] = TEXT("MZIMEUUI");
TCHAR szCompStrClassName[] = TEXT("MZIMEUCompStr");
TCHAR szCandClassName[] = TEXT("MZIMEUCand");
TCHAR szStatusClassName[] = TEXT("MZIMEUStatus");
TCHAR szGuideClassName[] = TEXT("MZIMEUGuide");

MYGUIDELINE glTable[] = {
  {GL_LEVEL_ERROR, GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
  {GL_LEVEL_WARNING, GL_ID_TYPINGERROR, IDS_GL_TYPINGERROR, 0},
  {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST, IDS_GL_TESTGUIDELINESTR,
   IDS_GL_TESTGUIDELINEPRIVATE}};

/* for DIC */
TCHAR szDicFileName[256]; /* Dictionary file name stored buffer */

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

BOOL InitMZIME(HINSTANCE hInstance) {
  hInst = hInstance;

  // Create/open a system global named mutex.
  // The initial ownership is not needed.
  // CreateSecurityAttributes() will create
  // the proper security attribute for IME.
  PSECURITY_ATTRIBUTES psa = CreateSecurityAttributes();
  if (psa != NULL) {
    hMutex = CreateMutex(psa, FALSE, TEXT("mzimeja_mutex"));
    FreeSecurityAttributes(psa);
    assert(hMutex);
  } else {
    // Failed, not NT system
    assert(0);
    return FALSE;
  }

  IMERegisterClass(hInst);

  LPTSTR lpDicFileName = szDicFileName;
  lpDicFileName += GetWindowsDirectory(lpDicFileName, 256);
  if (*(lpDicFileName - 1) != TEXT('\\')) *lpDicFileName++ = TEXT('\\');
  LoadString(hInstance, IDS_DICFILENAME, lpDicFileName, 128);
  return TRUE;
}

VOID DestroyMZIME(VOID) {
  UnregisterClass(szUIClassName, hInst);
  UnregisterClass(szCompStrClassName, hInst);
  UnregisterClass(szCandClassName, hInst);
  UnregisterClass(szStatusClassName, hInst);
  if (hMutex) CloseHandle(hMutex);
}

//////////////////////////////////////////////////////////////////////////////
// DLL entry point

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwFunction, LPVOID lpNot) {
  DebugPrint(TEXT("DLLEntry:dwFunc=%d\n"), dwFunction);

  switch (dwFunction) {
    case DLL_PROCESS_ATTACH:
      InitMZIME(hInstDLL);
      DebugPrint(TEXT("DLLEntry Process Attach hInst is %lx"), hInst);
      break;

    case DLL_PROCESS_DETACH:
      DestroyMZIME();
      DebugPrint(TEXT("DLLEntry Process Detach hInst is %lx"), hInst);
      break;

    case DLL_THREAD_ATTACH:
      DebugPrint(TEXT("DLLEntry Thread Attach hInst is %lx"), hInst);
      break;

    case DLL_THREAD_DETACH:
      DebugPrint(TEXT("DLLEntry Thread Detach hInst is %lx"), hInst);
      break;
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
