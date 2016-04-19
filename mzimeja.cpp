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
    assert(m_hMutex);
  } else {
    // Failed, not NT system
    assert(0);
    return FALSE;
  }

  RegisterClasses(m_hInst);

  LPTSTR lpDicFileName = m_szDicFileName;
  lpDicFileName += ::GetWindowsDirectory(lpDicFileName, _countof(m_szDicFileName));
  if (*(lpDicFileName - 1) != TEXT('\\')) *lpDicFileName++ = TEXT('\\');
  ::LoadString(hInstance, IDS_DICFILENAME, lpDicFileName, 128);
  return TRUE;
}

#define CS_MZIME (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_IME)

BOOL MZIMEJA::RegisterClasses(HINSTANCE hInstance) {
  WNDCLASSEX wcx;

  // register class of UI window.
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_MZIME;
  wcx.lpfnWndProc = MZIMEWndProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = 2 * sizeof(LONG_PTR);
  wcx.hInstance = hInstance;
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcx.hIcon = NULL;
  wcx.lpszMenuName = NULL;
  wcx.lpszClassName = szUIClassName;
  wcx.hbrBackground = NULL;
  wcx.hIconSm = NULL;
  if (!RegisterClassEx(&wcx)) return FALSE;

  // register class of composition window.
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_MZIME;
  wcx.lpfnWndProc = CompStrWndProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = UIEXTRASIZE;
  wcx.hInstance = hInstance;
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcx.hIcon = NULL;
  wcx.lpszMenuName = NULL;
  wcx.lpszClassName = szCompStrClassName;
  wcx.hbrBackground = NULL;
  wcx.hIconSm = NULL;
  if (!RegisterClassEx(&wcx)) return FALSE;

  // register class of candidate window.
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_MZIME;
  wcx.lpfnWndProc = CandWndProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = UIEXTRASIZE;
  wcx.hInstance = hInstance;
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcx.hIcon = NULL;
  wcx.lpszMenuName = NULL;
  wcx.lpszClassName = szCandClassName;
  wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wcx.hIconSm = NULL;
  if (!RegisterClassEx(&wcx)) return FALSE;

  // register class of status window.
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_MZIME;
  wcx.lpfnWndProc = StatusWndProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = UIEXTRASIZE;
  wcx.hInstance = hInstance;
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcx.hIcon = NULL;
  wcx.lpszMenuName = NULL;
  wcx.lpszClassName = szStatusClassName;
  wcx.hbrBackground = NULL;
  wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wcx.hIconSm = NULL;
  if (!RegisterClassEx(&wcx)) return FALSE;

  // register class of guideline window.
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_MZIME;
  wcx.lpfnWndProc = GuideWndProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = UIEXTRASIZE;
  wcx.hInstance = hInstance;
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcx.hIcon = NULL;
  wcx.lpszMenuName = NULL;
  wcx.lpszClassName = szGuideClassName;
  wcx.hbrBackground = NULL;
  wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wcx.hIconSm = NULL;
  if (!RegisterClassEx(&wcx)) return FALSE;

  return TRUE;
}

HKL MZIMEJA::GetHKL(VOID) {
  HKL hKL = 0, *lphkl;

  DWORD dwSize = GetKeyboardLayoutList(0, NULL);
  lphkl = (HKL *)GlobalAlloc(GPTR, dwSize * sizeof(DWORD));
  if (!lphkl) return NULL;

  GetKeyboardLayoutList(dwSize, lphkl);

  TCHAR szFile[32];
  for (DWORD dwi = 0; dwi < dwSize; dwi++) {
    HKL hKLTemp = *(lphkl + dwi);
    ImmGetIMEFileName(hKLTemp, szFile, _countof(szFile));

    if (!lstrcmp(szFile, MZIME_FILENAME)) {
      hKL = hKLTemp;
      break;
    }
  }

  GlobalFree(lphkl);
  return hKL;
}

// Update the transrate key buffer
BOOL MZIMEJA::GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg) {
  LPTRANSMSG lpgmT0;

  ++m_uNumTransKey;
  if (m_uNumTransKey >= m_lpCurTransKey->uMsgCount) {
    m_fOverTransKey = TRUE;
    return FALSE;
  }

  lpgmT0 = m_lpCurTransKey->TransMsg + (m_uNumTransKey - 1);
  *lpgmT0 = *lpGeneMsg;

  return TRUE;
}

BOOL MZIMEJA::GenerateMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  TRANSMSG genmsg;
  genmsg.message = message;
  genmsg.wParam = wParam;
  genmsg.lParam = lParam;
  return GenerateMessage(genmsg);
}

// Update the transrate key buffer
BOOL MZIMEJA::GenerateMessage(
  HIMC hIMC, InputContext *lpIMC, LPTRANSMSG lpGeneMsg)
{
  if (m_lpCurTransKey)
    return GenerateMessageToTransKey(lpGeneMsg);

  if (::IsWindow(lpIMC->hWnd)) {
    DWORD dwNewSize = sizeof(TRANSMSG) * (lpIMC->NumMsgBuf() + 1);
    if (!(lpIMC->hMsgBuf = ImmReSizeIMCC(lpIMC->hMsgBuf, dwNewSize)))
      return FALSE;

    LPTRANSMSG lpTransMsg = lpIMC->LockMsgBuf();
    if (NULL == lpTransMsg) return FALSE;
    lpTransMsg[lpIMC->NumMsgBuf()] = *lpGeneMsg;
    lpIMC->NumMsgBuf()++;
    lpIMC->UnlockMsgBuf();

    ImmGenerateMessage(hIMC);
  }
  return TRUE;
}

void MZIMEJA::UpdateIndicIcon(HIMC hIMC) {
  if (!m_hMyKL) {
    m_hMyKL = GetHKL();
    if (!m_hMyKL) return;
  }

  HWND hwndIndicate = FindWindow(INDICATOR_CLASS, NULL);

  BOOL fOpen = FALSE;
  if (hIMC) {
    InputContext *lpIMC = TheApp.LockIMC(hIMC);
    if (lpIMC) {
      fOpen = lpIMC->fOpen;
      TheApp.UnlockIMC();
    }
  }

  if (IsWindow(hwndIndicate)) {
    ATOM atomTip;

    atomTip = GlobalAddAtom(TEXT("MZ-IME Open"));
    PostMessage(hwndIndicate, INDICM_SETIMEICON, fOpen ? 1 : (-1),
                (LPARAM)m_hMyKL);
    PostMessage(hwndIndicate, INDICM_SETIMETOOLTIPS, fOpen ? atomTip : (-1),
                (LPARAM)m_hMyKL);
    PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS,
                // fOpen ? (RDMI_LEFT | RDMI_RIGHT) : 0, (LPARAM)m_hMyKL);
                fOpen ? (RDMI_LEFT) : 0, (LPARAM)m_hMyKL);
  }
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

InputContext *MZIMEJA::LockIMC(HIMC hIMC) {
  InputContext *context;
  context = (InputContext *)::ImmLockIMC(hIMC);
  if (context) {
    m_hIMC = hIMC;
    m_lpIMC = context;
  }
  return context;
}

VOID MZIMEJA::UnlockIMC() {
  if (m_hIMC) {
    ::ImmUnlockIMC(m_hIMC);
    m_hIMC = NULL;
    m_lpIMC = NULL;
  }
}

BOOL MZIMEJA::GenerateMessage(TRANSMSG& msg) {
  if (m_lpIMC) {
    return GenerateMessage(m_hIMC, m_lpIMC, &msg);
  }
  return FALSE;
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
