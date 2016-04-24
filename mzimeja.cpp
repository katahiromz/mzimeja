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

BOOL IsInputModeOpen(InputMode imode) {
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
  case IMODE_ZEN_KATAKANA:
  case IMODE_ZEN_EISUU:
  case IMODE_HAN_KANA:
    return TRUE;
  case IMODE_HAN_EISUU:
    return FALSE;
  }
  return FALSE;
}

InputMode InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion) {
  if (bOpen) {
    if (dwConversion & IME_CMODE_FULLSHAPE) {
      if (dwConversion & IME_CMODE_JAPANESE) {
        if (dwConversion & IME_CMODE_KATAKANA) {
          return IMODE_ZEN_KATAKANA;
        } else {
          return IMODE_ZEN_HIRAGANA;
        }
      } else {
        return IMODE_ZEN_EISUU;
      }
    } else {
      if (dwConversion & (IME_CMODE_JAPANESE | IME_CMODE_KATAKANA)) {
        return IMODE_HAN_KANA;
      } else {
        return IMODE_HAN_EISUU;
      }
    }
  } else {
    return IMODE_HAN_EISUU;
  }
}

UINT CommandFromInputMode(InputMode imode) {
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    return IDM_HIRAGANA;
  case IMODE_ZEN_KATAKANA:
    return IDM_ZEN_KATAKANA;
  case IMODE_ZEN_EISUU:
    return IDM_ZEN_ALNUM;
  case IMODE_HAN_KANA:
    return IDM_HAN_KATAKANA;
  case IMODE_HAN_EISUU:
    return IDM_ALNUM;
  default:
    return IDM_ALNUM;
  }
}

InputMode GetInputMode(HIMC hIMC) {
  DWORD dwConversion, dwSentence;
  ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
  BOOL bOpen = ::ImmGetOpenStatus(hIMC);
  return InputModeFromConversionMode(bOpen, dwConversion);
}

InputMode NextInputMode(InputMode imode) {
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    return IMODE_ZEN_KATAKANA;
  case IMODE_ZEN_KATAKANA:
    return IMODE_ZEN_EISUU;
  case IMODE_ZEN_EISUU:
    return IMODE_HAN_KANA;
  case IMODE_HAN_KANA:
    return IMODE_HAN_EISUU;
  case IMODE_HAN_EISUU:
  default:
    return IMODE_ZEN_HIRAGANA;
  }
}

void SetInputMode(HIMC hIMC, InputMode imode) {
  DWORD dwConversion, dwSentence;
  ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion &= ~IME_CMODE_KATAKANA;
    dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
    break;
  case IMODE_ZEN_KATAKANA:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
    break;
  case IMODE_ZEN_EISUU:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion &= ~(IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
    dwConversion |= IME_CMODE_FULLSHAPE;
    break;
  case IMODE_HAN_KANA:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion &= ~IME_CMODE_FULLSHAPE;
    dwConversion |= IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
    break;
  case IMODE_HAN_EISUU:
    ImmSetOpenStatus(hIMC, FALSE);
    dwConversion &= ~(IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
    break;
  }
  ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

BOOL IsRomajiMode(HIMC hIMC) {
  DWORD dwConversion, dwSentence;
  ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
  return (dwConversion & IME_CMODE_ROMAN);
}

void SetRomajiMode(HIMC hIMC, BOOL bRomaji) {
  DWORD dwConversion, dwSentence;
  ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
  if (bRomaji) {
    dwConversion |= IME_CMODE_ROMAN;
  } else {
    dwConversion &= ~IME_CMODE_ROMAN;
  }
  ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

void RepositionWindow(HWND hWnd) {
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
BOOL MZIMEJA::GenerateMessage(LPTRANSMSG lpGeneMsg) {
  BOOL ret = FALSE;
  DebugPrint(TEXT("GenerateMessage(%u,%d,%d)\n"),
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
  TRANSMSG genmsg;
  genmsg.message = message;
  genmsg.wParam = wParam;
  genmsg.lParam = lParam;
  return GenerateMessage(&genmsg);
}

// Update the transrate key buffer
BOOL MZIMEJA::GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg) {
  ++m_uNumTransKey;
  if (m_uNumTransKey >= m_lpCurTransKey->uMsgCount) {
    m_fOverTransKey = TRUE;
    return FALSE;
  }

  LPTRANSMSG lpgmT0 = m_lpCurTransKey->TransMsg + (m_uNumTransKey - 1);
  *lpgmT0 = *lpGeneMsg;

  return TRUE;
}

BOOL MZIMEJA::DoCommand(HIMC hIMC, DWORD dwCommand) {
  switch (dwCommand) {
  case IDM_RECONVERT:
    break;
  case IDM_ABOUT:
    break;
  case IDM_HIRAGANA:
    SetInputMode(hIMC, IMODE_ZEN_HIRAGANA);
    break;
  case IDM_ZEN_KATAKANA:
    SetInputMode(hIMC, IMODE_ZEN_KATAKANA);
    break;
  case IDM_ZEN_ALNUM:
    SetInputMode(hIMC, IMODE_ZEN_EISUU);
    break;
  case IDM_HAN_KATAKANA:
    SetInputMode(hIMC, IMODE_HAN_KANA);
    break;
  case IDM_ALNUM:
    SetInputMode(hIMC, IMODE_HAN_EISUU);
    break;
  case IDM_CANCEL:
    break;
  case IDM_ROMAJI_INPUT:
    SetRomajiMode(hIMC, TRUE);
    break;
  case IDM_KANA_INPUT:
    SetRomajiMode(hIMC, FALSE);
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
  if (!m_hMyKL) {
    m_hMyKL = GetHKL();
    if (!m_hMyKL) return;
  }

  HWND hwndIndicate = FindWindow(INDICATOR_CLASS, NULL);

  BOOL fOpen = FALSE;
  if (hIMC) {
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      fOpen = lpIMC->IsOpen();
      TheIME.UnlockIMC(hIMC);
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
  DebugPrint(TEXT("MZIMEJA::UnlockIMC: unlocking: %p\n"), hIMC);
  ::ImmUnlockIMC(hIMC);
  DebugPrint(TEXT("MZIMEJA::UnlockIMC: unlocked: %p\n"), hIMC);
  m_hIMC = NULL;
  m_lpIMC = NULL;
}

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////
// for debugging

#ifndef NDEBUG
  int DebugPrintA(LPCSTR lpszFormat, ...) {
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
  int DebugPrintW(LPCWSTR lpszFormat, ...) {
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

//////////////////////////////////////////////////////////////////////////////
// DLL entry point

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwFunction, LPVOID lpNot) {
  FOOTMARK();
  switch (dwFunction) {
    case DLL_PROCESS_ATTACH:
      DebugPrint(TEXT("DLL_PROCESS_ATTACH: hInst is %lx\n"), TheIME.m_hInst);
      TheIME.Init(hInstDLL);
      break;

    case DLL_PROCESS_DETACH:
      DebugPrint(TEXT("DLL_PROCESS_DETACH: hInst is %lx\n"), TheIME.m_hInst);
      TheIME.Destroy();
      break;

    case DLL_THREAD_ATTACH:
      DebugPrint(TEXT("DLL_THREAD_ATTACH: hInst is %lx\n"), TheIME.m_hInst);
      break;

    case DLL_THREAD_DETACH:
      DebugPrint(TEXT("DLL_THREAD_DETACH: hInst is %lx\n"), TheIME.m_hInst);
      break;
  }
  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
