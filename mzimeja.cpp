// mzimeja.cpp --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#include "mzimeja.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

// the window classes for mzimeja UI windows
const TCHAR szUIServerClassName[] = TEXT("MZIMEUI");
const TCHAR szCompStrClassName[]  = TEXT("MZIMECompStr");
const TCHAR szCandClassName[]     = TEXT("MZIMECand");
const TCHAR szStatusClassName[]   = TEXT("MZIMEStatus");
const TCHAR szGuideClassName[]    = TEXT("MZIMEGuide");

const MZGUIDELINE glTable[] = {
  {GL_LEVEL_ERROR, GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
  {GL_LEVEL_WARNING, GL_ID_TYPINGERROR, IDS_GL_TYPINGERROR, 0},
  {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST, IDS_GL_TESTGUIDELINESTR,
   IDS_GL_TESTGUIDELINEPRIVATE}
};

// filename of the IME
const TCHAR szImeFileName[] = TEXT("mzimeja.ime");

//////////////////////////////////////////////////////////////////////////////

HFONT CheckNativeCharset(HDC hDC) {
  HFONT hOldFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);

  LOGFONT lfFont;
  GetObject(hOldFont, sizeof(LOGFONT), &lfFont);

  if (lfFont.lfCharSet != SHIFTJIS_CHARSET) {
    lfFont.lfWeight = FW_NORMAL;
    lfFont.lfCharSet = SHIFTJIS_CHARSET;
    lfFont.lfFaceName[0] = 0;
    SelectObject(hDC, CreateFontIndirect(&lfFont));
  } else {
    hOldFont = NULL;
  }
  return hOldFont;
} // CheckNativeCharset

// adjust window position
void RepositionWindow(HWND hWnd) {
  FOOTMARK();
  RECT rc, rcWorkArea;
  ::GetWindowRect(hWnd, &rc);
  ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
  SIZE siz;
  siz.cx = rc.right - rc.left;
  siz.cy = rc.bottom - rc.top;
  if (rc.right > rcWorkArea.right) {
    rc.right = rcWorkArea.right;
    rc.left = rcWorkArea.right - siz.cx;
  }
  if (rc.left < rcWorkArea.left) {
    rc.left = rcWorkArea.left;
    rc.right = rc.left + siz.cx;
  }
  if (rc.bottom > rcWorkArea.bottom) {
    rc.bottom = rcWorkArea.bottom;
    rc.top = rcWorkArea.bottom - siz.cy;
  }
  if (rc.top < rcWorkArea.top) {
    rc.top = rcWorkArea.top;
    rc.bottom = rc.top + siz.cy;
  }
  ::MoveWindow(hWnd, rc.left, rc.top, siz.cx, siz.cy, TRUE);
}

//////////////////////////////////////////////////////////////////////////////

MZIMEJA TheIME;

MZIMEJA::MZIMEJA() {
  m_hInst = NULL;
  m_hMyKL = 0;
  m_bWinLogOn = FALSE;

  m_lpCurTransKey = NULL;
  m_uNumTransKey = 0;

  m_fOverflowKey = FALSE;

  m_hMutex = NULL;
  m_hBaseData = NULL;
  m_hIMC = NULL;
  m_lpIMC = NULL;
  m_hBasicDictData = NULL;
}

BOOL MZIMEJA::Init(HINSTANCE hInstance) {
  FOOTMARK();
  m_hInst = hInstance;

  MakeMaps();

  // CreateSecurityAttributes() will create
  // the proper security attribute for IME.
  SECURITY_ATTRIBUTES *psa = CreateSecurityAttributes();
  assert(psa);

  // create a mutex
  m_hMutex = ::CreateMutexW(psa, FALSE, L"mzimeja_mutex");
  assert(m_hMutex);

  // create base data
  m_hBaseData = ::CreateFileMappingW(INVALID_HANDLE_VALUE, psa,
    PAGE_READWRITE, 0, sizeof(ImeBaseData), L"mzimeja_basedata");
  assert(m_hBaseData);
  if (m_hBaseData && ::GetLastError() != ERROR_ALREADY_EXISTS) {
    // initialize base data
    ImeBaseData *data = LockImeBaseData();
    if (data) {
      data->dwSignature = 0xDEADFACE;
      data->dwSharedDictDataSize = 0;
      UnlockImeBaseData(data);
    }
  }

  // free sa
  FreeSecurityAttributes(psa);

  // register window classes for IME
  return RegisterClasses(m_hInst);
} // MZIMEJA::Init

ImeBaseData *MZIMEJA::LockImeBaseData() {
  LPVOID pvData;
  pvData = ::MapViewOfFile(m_hBaseData, FILE_MAP_ALL_ACCESS,
                           0, 0, sizeof(ImeBaseData));
  return reinterpret_cast<ImeBaseData *>(pvData);
}

void MZIMEJA::UnlockImeBaseData(ImeBaseData *data) {
  ::UnmapViewOfFile(data);
}

std::wstring MZIMEJA::GetSettingString(LPCWSTR szSettingName) const {
  HKEY hKey;
  LONG result;
  WCHAR szValue[MAX_PATH * 2];
  result = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE,
    L"SOFTWARE\\Katayama Hirofumi MZ\\mzimaja",
    0, KEY_READ, &hKey);
  if (result == ERROR_SUCCESS && hKey) {
    DWORD cbData = sizeof(szValue);
    result = ::RegQueryValueExW(hKey, szSettingName, NULL, NULL, 
      reinterpret_cast<LPBYTE>(szValue), &cbData);
    ::RegCloseKey(hKey);
    if (result == ERROR_SUCCESS) {
      return std::wstring(szValue);
    }
  }
  return std::wstring();
} // MZIMEJA::GetSettingString

BOOL MZIMEJA::RegisterClasses(HINSTANCE hInstance) {
#define CS_MZIME (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_IME)
  WNDCLASSEX wcx;
  FOOTMARK();

  // register class of UI server window.
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_MZIME;
  wcx.lpfnWndProc = MZIMEWndProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = 2 * sizeof(LONG_PTR);
  wcx.hInstance = hInstance;
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcx.hIcon = NULL;
  wcx.lpszMenuName = NULL;
  wcx.lpszClassName = szUIServerClassName;
  wcx.hbrBackground = NULL;
  wcx.hIconSm = NULL;
  if (!RegisterClassEx(&wcx)) return FALSE;

  // register class of composition window.
  wcx.cbSize = sizeof(WNDCLASSEX);
  wcx.style = CS_MZIME;
  wcx.lpfnWndProc = CompWnd_WindowProc;
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
  wcx.lpfnWndProc = CandWnd_WindowProc;
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
  wcx.lpfnWndProc = StatusWnd_WindowProc;
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
  wcx.lpfnWndProc = GuideWnd_WindowProc;
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
#undef CS_MZIME
} // MZIMEJA::RegisterClasses

HKL MZIMEJA::GetHKL(VOID) {
  FOOTMARK();
  HKL hKL = NULL;

  // get list size and allocate buffer for list
  DWORD dwSize = ::GetKeyboardLayoutList(0, NULL);
  HKL *lphkl = (HKL *)::GlobalAlloc(GPTR, dwSize * sizeof(DWORD));
  if (lphkl == NULL) return NULL;

  // get the list of keyboard layouts
  ::GetKeyboardLayoutList(dwSize, lphkl);

  // find hKL from the list
  TCHAR szFile[32];
  for (DWORD dwi = 0; dwi < dwSize; dwi++) {
    HKL hKLTemp = *(lphkl + dwi);
    ::ImmGetIMEFileName(hKLTemp, szFile, _countof(szFile));

    if (!::lstrcmp(szFile, szImeFileName)) {
      hKL = hKLTemp;
      break;
    }
  }

  // free the list
  ::GlobalFree(lphkl);
  return hKL;
}

// Update the transrate key buffer
BOOL MZIMEJA::GenerateMessage(LPTRANSMSG lpGeneMsg) {
  BOOL ret = FALSE;
  FOOTMARK();
  DebugPrint(TEXT("(%u,%d,%d)\n"),
    lpGeneMsg->message, lpGeneMsg->wParam, lpGeneMsg->lParam);

  if (m_lpCurTransKey)
    return GenerateMessageToTransKey(lpGeneMsg);

  if (m_lpIMC == NULL) {
    FOOTMARK_PRINT_CALL_STACK();
  }

  if (m_lpIMC && ::IsWindow(m_lpIMC->hWnd)) {
    DWORD dwNewSize = sizeof(TRANSMSG) * (m_lpIMC->NumMsgBuf() + 1);
    m_lpIMC->hMsgBuf = ImmReSizeIMCC(m_lpIMC->hMsgBuf, dwNewSize);
    if (m_lpIMC->hMsgBuf) {
      LPTRANSMSG lpTransMsg = m_lpIMC->LockMsgBuf();
      if (lpTransMsg) {
        lpTransMsg[m_lpIMC->NumMsgBuf()++] = *lpGeneMsg;
        m_lpIMC->UnlockMsgBuf();
        ret = ImmGenerateMessage(m_hIMC);
      }
    }
  }
  return ret;
}

BOOL MZIMEJA::GenerateMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  FOOTMARK();
  TRANSMSG genmsg;
  genmsg.message = message;
  genmsg.wParam = wParam;
  genmsg.lParam = lParam;
  return GenerateMessage(&genmsg);
}

// Update the transrate key buffer
BOOL MZIMEJA::GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg) {
  FOOTMARK();

  // increment the number
  ++m_uNumTransKey;

  // check overflow
  if (m_uNumTransKey >= m_lpCurTransKey->uMsgCount) {
    m_fOverflowKey = TRUE;
    return FALSE;
  }

  // put one message to TRANSMSG buffer
  LPTRANSMSG lpgmT0 = m_lpCurTransKey->TransMsg + (m_uNumTransKey - 1);
  *lpgmT0 = *lpGeneMsg;

  return TRUE;
}

BOOL MZIMEJA::DoCommand(HIMC hIMC, DWORD dwCommand) {
  FOOTMARK();
  switch (dwCommand) {
  case IDM_RECONVERT:
    break;
  case IDM_ABOUT:
    break;
  case IDM_HIRAGANA:
    SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
    break;
  case IDM_FULL_KATAKANA:
    SetInputMode(hIMC, IMODE_FULL_KATAKANA);
    break;
  case IDM_FULL_ASCII:
    SetInputMode(hIMC, IMODE_FULL_ASCII);
    break;
  case IDM_HALF_KATAKANA:
    SetInputMode(hIMC, IMODE_HALF_KANA);
    break;
  case IDM_HALF_ASCII:
    SetInputMode(hIMC, IMODE_HALF_ASCII);
    break;
  case IDM_CANCEL:
    break;
  case IDM_ROMAN_INPUT:
    SetRomanMode(hIMC, TRUE);
    break;
  case IDM_KANA_INPUT:
    SetRomanMode(hIMC, FALSE);
    break;
  case IDM_HIDE:
    break;
  case IDM_PROPERTY:
    break;
  case IDM_ADD_WORD:
    break;
  default:
    return FALSE;
  }
  return TRUE;
} // MZIMEJA::DoCommand

void MZIMEJA::UpdateIndicIcon(HIMC hIMC) {
  FOOTMARK();
  if (!m_hMyKL) {
    m_hMyKL = GetHKL();
    if (!m_hMyKL) return;
  }

  HWND hwndIndicate = ::FindWindow(INDICATOR_CLASS, NULL);
  if (::IsWindow(hwndIndicate)) {
    BOOL fOpen = FALSE;
    if (hIMC) {
      fOpen = ImmGetOpenStatus(hIMC);
    }

    ATOM atomTip = ::GlobalAddAtom(TEXT("MZ-IME Open"));
    LPARAM lParam = (LPARAM)m_hMyKL;
    ::PostMessage(hwndIndicate, INDICM_SETIMEICON, fOpen ? 1 : (-1), lParam);
    ::PostMessage(hwndIndicate, INDICM_SETIMETOOLTIPS, (fOpen ? atomTip : (-1)),
                  lParam);
    ::PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS,
                  (fOpen ? (RDMI_LEFT) : 0), lParam);
  }
}

VOID MZIMEJA::Uninit(VOID) {
  FOOTMARK();
  ::UnregisterClass(szUIServerClassName, m_hInst);
  ::UnregisterClass(szCompStrClassName, m_hInst);
  ::UnregisterClass(szCandClassName, m_hInst);
  ::UnregisterClass(szStatusClassName, m_hInst);
  if (m_hMutex) {
    ::CloseHandle(m_hMutex);
    m_hMutex = NULL;
  }
  if (m_hBaseData) {
    ::CloseHandle(m_hBaseData);
    m_hBaseData = NULL;
  }
}

HBITMAP MZIMEJA::LoadBMP(LPCTSTR pszName) {
  FOOTMARK();
  return ::LoadBitmap(m_hInst, pszName);
}

WCHAR *MZIMEJA::LoadSTR(INT nID) {
  FOOTMARK();
  static WCHAR sz[512];
  sz[0] = 0;
  ::LoadStringW(m_hInst, nID, sz, 512);
  return sz;
}

InputContext *MZIMEJA::LockIMC(HIMC hIMC) {
  FOOTMARK();
  DebugPrint(TEXT("MZIMEJA::LockIMC: locking: %p\n"), hIMC);
  InputContext *context = (InputContext *)::ImmLockIMC(hIMC);
  if (context) {
    m_hIMC = hIMC;
    m_lpIMC = context;
    DebugPrint(TEXT("MZIMEJA::LockIMC: locked: %p\n"), hIMC);
  } else {
    DebugPrint(TEXT("MZIMEJA::LockIMC: cannot lock: %p\n"), hIMC);
  }
  return context;
}

VOID MZIMEJA::UnlockIMC(HIMC hIMC) {
  FOOTMARK();
  DebugPrint(TEXT("MZIMEJA::UnlockIMC: unlocking: %p\n"), hIMC);
  ::ImmUnlockIMC(hIMC);
  DebugPrint(TEXT("MZIMEJA::UnlockIMC: unlocked: %p\n"), hIMC);
  if (::ImmGetIMCLockCount(hIMC) == 0) {
    m_hIMC = NULL;
    m_lpIMC = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////
// UI extra related

HGLOBAL GetUIExtraFromServerWnd(HWND hwndServer) {
  FOOTMARK();
  return (HGLOBAL)GetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE);
}

void SetUIExtraToServerWnd(HWND hwndServer, HGLOBAL hUIExtra) {
  FOOTMARK();
  SetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE, (LONG_PTR)hUIExtra);
}

UIEXTRA *LockUIExtra(HWND hwndServer) {
  FOOTMARK();
  HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
  UIEXTRA *lpUIExtra = (UIEXTRA *)::GlobalLock(hUIExtra);
  assert(lpUIExtra);
  return lpUIExtra;
}

void UnlockUIExtra(HWND hwndServer) {
  FOOTMARK();
  HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
  ::GlobalUnlock(hUIExtra);
}

void FreeUIExtra(HWND hwndServer) {
  FOOTMARK();
  HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
  ::GlobalFree(hUIExtra);
  SetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE, (LONG_PTR)NULL);
}

//////////////////////////////////////////////////////////////////////////////
// for debugging

#ifndef NDEBUG
  int DebugPrintA(const char *lpszFormat, ...) {
    int nCount;
    char szMsgA[1024];

    va_list marker;
    va_start(marker, lpszFormat);
    nCount = wvsprintfA(szMsgA, lpszFormat, marker);
    va_end(marker);

    WCHAR szMsgW[1024];
    szMsgW[0] = 0;
    ::MultiByteToWideChar(CP_ACP, 0, szMsgA, -1, szMsgW, 1024);

    //OutputDebugString(szMsg);
    FILE *fp = fopen("C:\\mzimeja.log", "ab");
    if (fp) {
      int len = lstrlenW(szMsgW);
      if (len > 0 && szMsgW[len - 1] == L'\n') {
        szMsgW[len - 1] = L'\r';
        szMsgW[len] = L'\n';
        ++len;
        szMsgW[len] = L'\0';
      }
      fwrite(szMsgW, len * sizeof(WCHAR), 1, fp);
      fclose(fp);
    }
    return nCount;
  }
  int DebugPrintW(const WCHAR *lpszFormat, ...) {
    int nCount;
    WCHAR szMsg[1024];

    va_list marker;
    va_start(marker, lpszFormat);
    nCount = wvsprintfW(szMsg, lpszFormat, marker);
    va_end(marker);

    //OutputDebugString(szMsg);
    FILE *fp = fopen("C:\\mzimeja.log", "ab");
    if (fp) {
      int len = lstrlenW(szMsg);
      if (len > 0 && szMsg[len - 1] == L'\n') {
        szMsg[len - 1] = L'\r';
        szMsg[len] = L'\n';
        ++len;
        szMsg[len] = L'\0';
      }
      fwrite(szMsg, len * sizeof(WCHAR), 1, fp);
      fclose(fp);
    }
    return nCount;
  }
#endif  // ndef NDEBUG

#ifndef NDEBUG
  LONG WINAPI MyUnhandledExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo) {
    DebugPrint(TEXT("### Abnormal Status ###\n"));
    FOOTMARK_PRINT_CALL_STACK();
    return EXCEPTION_EXECUTE_HANDLER;
  }
#endif

//////////////////////////////////////////////////////////////////////////////
// DLL entry point

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwFunction, LPVOID lpNot) {
  FOOTMARK();
  static LPTOP_LEVEL_EXCEPTION_FILTER s_old_handler;
  switch (dwFunction) {
  case DLL_PROCESS_ATTACH:
    ::DisableThreadLibraryCalls(hInstDLL);
    #ifndef NDEBUG
      s_old_handler = ::SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
    #endif
    DebugPrint(TEXT("DLL_PROCESS_ATTACH: hInst is %p\n"), TheIME.m_hInst);
    TheIME.Init(hInstDLL);
    break;

  case DLL_PROCESS_DETACH:
    DebugPrint(TEXT("DLL_PROCESS_DETACH: hInst is %p\n"), TheIME.m_hInst);
    TheIME.Uninit();
    #ifndef NDEBUG
      ::SetUnhandledExceptionFilter(s_old_handler);
    #endif
    break;

  case DLL_THREAD_ATTACH:
    DebugPrint(TEXT("DLL_THREAD_ATTACH: hInst is %p\n"), TheIME.m_hInst);
    break;

  case DLL_THREAD_DETACH:
    DebugPrint(TEXT("DLL_THREAD_DETACH: hInst is %p\n"), TheIME.m_hInst);
    break;
  }
  return TRUE;
} // DllMain

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
