/*++

Copyright (c) 1990-1998 Microsoft Corporation, All Rights Reserved

Module Name:

    FAKEIME.C
    
++*/

#include <windows.h>
#include "immdev.h"
#define _NO_EXTERN_
#include "fakeime.h"
#include "resource.h"
#include "immsec.h"


HINSTANCE   hInst;
HANDLE      hMutex = NULL;
HKL hMyKL = 0;

/* for Translat */
LPTRANSMSGLIST lpCurTransKey= NULL;
UINT    uNumTransKey;
BOOL    fOverTransKey = FALSE;

/* for UI */
#ifdef FAKEIMEM
WCHAR   wszUIClassName[]     = L"FAKEIMEMUI";
char    szUIClassName[]      = "FAKEIMEMUI";
char    szCompStrClassName[] = "FAKEIMEMCompStr";
char    szCandClassName[]    = "FAKEIMEMCand";
char    szStatusClassName[]  = "FAKEIMEMStatus";
char    szGuideClassName[]   = "FAKEIMEMGuide";
#elif defined(UNICODE)
TCHAR    szUIClassName[]      = TEXT("FAKEIMEUUI");
TCHAR    szCompStrClassName[] = TEXT("FAKEIMEUCompStr");
TCHAR    szCandClassName[]    = TEXT("FAKEIMEUCand");
TCHAR    szStatusClassName[]  = TEXT("FAKEIMEUStatus");
TCHAR    szGuideClassName[]   = TEXT("FAKEIMEUGuide");
#else
char    szUIClassName[]      = "FAKEIMEUI";
char    szCompStrClassName[] = "FAKEIMECompStr";
char    szCandClassName[]    = "FAKEIMECand";
char    szStatusClassName[]  = "FAKEIMEStatus";
char    szGuideClassName[]   = "FAKEIMEGuide";
#endif


MYGUIDELINE glTable[] = {
        {GL_LEVEL_ERROR,   GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
        {GL_LEVEL_WARNING, GL_ID_TYPINGERROR,  IDS_GL_TYPINGERROR, 0},
        {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST,IDS_GL_TESTGUIDELINESTR, IDS_GL_TESTGUIDELINEPRIVATE}
        };

/* for DIC */
TCHAR    szDicFileName[256];         /* Dictionary file name stored buffer */

#ifdef DEBUG
    /* for DebugOptions */
    #pragma data_seg("SHAREDDATA")
    DWORD dwLogFlag = 0L;
    DWORD dwDebugFlag = 0L;
    #pragma data_seg()
#endif


extern "C" {

/**********************************************************************/
/*    DLLEntry()                                                      */
/**********************************************************************/
BOOL WINAPI DllMain (
    HINSTANCE    hInstDLL,
    DWORD        dwFunction,
    LPVOID       lpNot)
{
    LPTSTR lpDicFileName;
#ifdef DEBUG
    TCHAR szDev[80];
#endif
    MyDebugPrint((TEXT("DLLEntry:dwFunc=%d\n"),dwFunction));

    switch(dwFunction)
    {
        PSECURITY_ATTRIBUTES psa;

        case DLL_PROCESS_ATTACH:
            //
            // Create/open a system global named mutex.
            // The initial ownership is not needed.
            // CreateSecurityAttributes() will create
            // the proper security attribute for IME.
            //
            psa = CreateSecurityAttributes();
            if ( psa != NULL ) {
                 hMutex = CreateMutex( psa, FALSE, TEXT("FakeIme_Mutex"));
                 FreeSecurityAttributes( psa );
                 if ( hMutex == NULL ) {
                 // Failed
                 }
            }
            else {
                  // Failed, not NT system
                  }

            hInst= hInstDLL;
            IMERegisterClass( hInst );

            // Initialize for FAKEIME.
            lpDicFileName = (LPTSTR)&szDicFileName;
            lpDicFileName += GetWindowsDirectory(lpDicFileName,256);
            if (*(lpDicFileName-1) != TEXT('\\'))
                *lpDicFileName++ = TEXT('\\');
            LoadString( hInst, IDS_DICFILENAME, lpDicFileName, 128);

            SetGlobalFlags();

#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Process Attach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_PROCESS_DETACH:
            UnregisterClass(szUIClassName,hInst);
            UnregisterClass(szCompStrClassName,hInst);
            UnregisterClass(szCandClassName,hInst);
            UnregisterClass(szStatusClassName,hInst);
            if (hMutex)
                CloseHandle( hMutex );
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Process Detach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_THREAD_ATTACH:
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Thread Attach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_THREAD_DETACH:
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Thread Detach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;
    }
    return TRUE;
}

} // extern "C"
