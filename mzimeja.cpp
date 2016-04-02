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

#ifdef _DEBUG
  /* for DebugOptions */
  #pragma data_seg("SHAREDDATA")
  DWORD dwLogFlag = 0L;
  DWORD dwDebugFlag = 0L;
  #pragma data_seg()
#endif

//////////////////////////////////////////////////////////////////////////////

extern "C" {

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwFunction, LPVOID lpNot) {
  LPTSTR lpDicFileName;
#ifdef _DEBUG
  TCHAR szDev[80];
#endif
  MyDebugPrint((TEXT("DLLEntry:dwFunc=%d\n"), dwFunction));

  switch (dwFunction) {
    PSECURITY_ATTRIBUTES psa;

    case DLL_PROCESS_ATTACH:
      //
      // Create/open a system global named mutex.
      // The initial ownership is not needed.
      // CreateSecurityAttributes() will create
      // the proper security attribute for IME.
      //
      psa = CreateSecurityAttributes();
      if (psa != NULL) {
        hMutex = CreateMutex(psa, FALSE, TEXT("mzimeja_mutex"));
        FreeSecurityAttributes(psa);
        if (hMutex == NULL) {
          // Failed
        }
      } else {
        // Failed, not NT system
      }

      hInst = hInstDLL;
      IMERegisterClass(hInst);

      // Initialize for MZ-IME.
      lpDicFileName = (LPTSTR)&szDicFileName;
      lpDicFileName += GetWindowsDirectory(lpDicFileName, 256);
      if (*(lpDicFileName - 1) != TEXT('\\')) *lpDicFileName++ = TEXT('\\');
      LoadString(hInst, IDS_DICFILENAME, lpDicFileName, 128);

      SetGlobalFlags();

#ifdef _DEBUG
      wsprintf(szDev, TEXT("DLLEntry Process Attach hInst is %lx"), hInst);
      ImeLog(LOGF_ENTRY, szDev);
#endif
      break;

    case DLL_PROCESS_DETACH:
      UnregisterClass(szUIClassName, hInst);
      UnregisterClass(szCompStrClassName, hInst);
      UnregisterClass(szCandClassName, hInst);
      UnregisterClass(szStatusClassName, hInst);
      if (hMutex) CloseHandle(hMutex);
#ifdef _DEBUG
      wsprintf(szDev, TEXT("DLLEntry Process Detach hInst is %lx"), hInst);
      ImeLog(LOGF_ENTRY, szDev);
#endif
      break;

    case DLL_THREAD_ATTACH:
#ifdef _DEBUG
      wsprintf(szDev, TEXT("DLLEntry Thread Attach hInst is %lx"), hInst);
      ImeLog(LOGF_ENTRY, szDev);
#endif
      break;

    case DLL_THREAD_DETACH:
#ifdef _DEBUG
      wsprintf(szDev, TEXT("DLLEntry Thread Detach hInst is %lx"), hInst);
      ImeLog(LOGF_ENTRY, szDev);
#endif
      break;
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
