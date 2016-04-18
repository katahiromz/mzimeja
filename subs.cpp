// subs.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// return value: fdwConversion
void PASCAL ChangeMode(HIMC hIMC, DWORD dwToMode) {
  DWORD fdwConversion;
  TRANSMSG GnMsg;

  InputContext *lpIMC;
  if (!(lpIMC = (InputContext *)ImmLockIMC(hIMC))) return;

  fdwConversion = lpIMC->fdwConversion;

  switch (dwToMode) {
    case TO_CMODE_ALPHANUMERIC:
      fdwConversion = (fdwConversion & ~IME_CMODE_LANGUAGE);
      break;

    case TO_CMODE_KATAKANA:
      fdwConversion |= (IME_CMODE_NATIVE | IME_CMODE_KATAKANA);
      break;

    case TO_CMODE_HIRAGANA:
      fdwConversion =
          ((fdwConversion & ~IME_CMODE_LANGUAGE) | IME_CMODE_NATIVE);
      fdwConversion |= IME_CMODE_FULLSHAPE;
      break;

    case TO_CMODE_FULLSHAPE:
      if (fdwConversion & IME_CMODE_FULLSHAPE) {
        // To SBCS mode.
        fdwConversion &= ~IME_CMODE_FULLSHAPE;

        // If hiragana mode, make it katakana mode.
        if ((fdwConversion & IME_CMODE_LANGUAGE) == IME_CMODE_NATIVE)
          fdwConversion |= (IME_CMODE_NATIVE | IME_CMODE_KATAKANA);

      } else {
        // To DBCS mode.
        fdwConversion |= IME_CMODE_FULLSHAPE;
      }
      break;

    case TO_CMODE_ROMAN:
      if (fdwConversion & IME_CMODE_ROMAN)
        fdwConversion &= ~IME_CMODE_ROMAN;
      else
        fdwConversion |= IME_CMODE_ROMAN;
      break;

    case TO_CMODE_CHARCODE:
      break;
    case TO_CMODE_TOOLBAR:
      break;
    default:
      break;
  }

  if (lpIMC->fdwConversion != fdwConversion) {
    lpIMC->fdwConversion = fdwConversion;
    GnMsg.message = WM_IME_NOTIFY;
    GnMsg.wParam = IMN_SETCONVERSIONMODE;
    GnMsg.lParam = 0L;
    GenerateMessage(hIMC, lpIMC, TheApp.m_lpCurTransKey, &GnMsg);
  }

  ImmUnlockIMC(hIMC);
  return;
}

void PASCAL ChangeCompStr(HIMC hIMC, DWORD dwToMode) {
  InputContext *lpIMC;
  CompStr *lpCompStr;
  //DWORD fdwConversion;
  TRANSMSG GnMsg;
  HANDLE hDst;
  LPTSTR lpSrc;
  LPTSTR lpDst;
  LPTSTR lpSrc0;
  LPTSTR lpDst0;
  //WORD wCode;
  BOOL fChange = FALSE;
  DWORD dwSize;

  if (!(lpIMC = (InputContext *)ImmLockIMC(hIMC))) return;

  if (!(lpCompStr = lpIMC->LockCompStr()))
    goto ccs_exit40;

  //fdwConversion = lpIMC->fdwConversion;

  dwSize = (lpCompStr->dwCompStrLen + 1) * sizeof(WCHAR);
  if (!(hDst = GlobalAlloc(GHND, dwSize)))
    goto ccs_exit30;

  if (!(lpDst = (LPTSTR)GlobalLock(hDst))) goto ccs_exit20;

  switch (dwToMode) {
    case TO_CMODE_ALPHANUMERIC:
      break;

    case TO_CMODE_KATAKANA:
      lpSrc = lpCompStr->szCompStr;
      lpSrc0 = lpSrc;
      lpDst0 = lpDst;
      while (*lpSrc) {
        *lpDst++ = HiraToKata(*lpSrc);
        lpSrc++;
      }
      lstrcpy(lpSrc0, lpDst0);
      lpCompStr->dwCompStrLen = lstrlen(lpSrc0);
      fChange = TRUE;
      break;

    case TO_CMODE_HIRAGANA:
      lpSrc = lpCompStr->szCompStr;
      lpSrc0 = lpSrc;
      lpDst0 = lpDst;
      while (*lpSrc) {
        *lpDst++ = KataToHira(*lpSrc);
        lpSrc++;
      }
      lstrcpy(lpSrc0, lpDst0);
      lpCompStr->dwCompStrLen = lstrlen(lpSrc0);
      fChange = TRUE;
      break;

    case TO_CMODE_FULLSHAPE:
      break;

    case TO_CMODE_ROMAN:
      break;
  }

  if (fChange) {
    GnMsg.message = WM_IME_COMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = GCS_COMPSTR;
    GenerateMessage(hIMC, lpIMC, TheApp.m_lpCurTransKey, &GnMsg);
  }

  GlobalUnlock(hDst);
ccs_exit20:
  GlobalFree(hDst);
ccs_exit30:
  lpIMC->UnlockCompStr();
ccs_exit40:
  ImmUnlockIMC(hIMC);
  return;
}

BOOL PASCAL IsCompStr(HIMC hIMC) {
  BOOL fRet = FALSE;

  InputContext *lpIMC;
  if (!(lpIMC = (InputContext *)ImmLockIMC(hIMC))) return FALSE;

  if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof(COMPOSITIONSTRING)) {
    ImmUnlockIMC(hIMC);
    return FALSE;
  }

  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (lpCompStr) {
    fRet = (lpCompStr->dwCompStrLen > 0);
    lpIMC->UnlockCompStr();
  }

  ImmUnlockIMC(hIMC);

  return fRet;
}

BOOL PASCAL IsConvertedCompStr(HIMC hIMC) {
  BOOL fRet = FALSE;

  InputContext *lpIMC;
  if (!(lpIMC = (InputContext *)ImmLockIMC(hIMC))) return FALSE;

  if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof(MZCOMPSTR)) {
    ImmUnlockIMC(hIMC);
    return FALSE;
  }

  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (lpCompStr->dwCompStrLen > 0)
    fRet = (lpCompStr->bCompAttr[0] > 0);

  lpIMC->UnlockCompStr();
  ImmUnlockIMC(hIMC);

  return fRet;
}

HKL PASCAL GetMyHKL() {
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

void PASCAL UpdateIndicIcon(HIMC hIMC) {
  if (!TheApp.m_hMyKL) {
    TheApp.m_hMyKL = GetMyHKL();
    if (!TheApp.m_hMyKL) return;
  }

  HWND hwndIndicate = FindWindow(INDICATOR_CLASS, NULL);

  BOOL fOpen = FALSE;
  if (hIMC) {
    InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
    if (lpIMC) {
      fOpen = lpIMC->fOpen;
      ImmUnlockIMC(hIMC);
    }
  }

  if (IsWindow(hwndIndicate)) {
    ATOM atomTip;

    atomTip = GlobalAddAtom(TEXT("MZ-IME Open"));
    PostMessage(hwndIndicate, INDICM_SETIMEICON, fOpen ? 1 : (-1),
                (LPARAM)TheApp.m_hMyKL);
    PostMessage(hwndIndicate, INDICM_SETIMETOOLTIPS, fOpen ? atomTip : (-1),
                (LPARAM)TheApp.m_hMyKL);
    PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS,
                // fOpen ? (RDMI_LEFT | RDMI_RIGHT) : 0, (LPARAM)TheApp.m_hMyKL);
                fOpen ? (RDMI_LEFT) : 0, (LPARAM)TheApp.m_hMyKL);
  }
}

HFONT CheckNativeCharset(HDC hDC) {
  HFONT hOldFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);

  LOGFONT lfFont;
  GetObject(hOldFont, sizeof(LOGFONT), &lfFont);

  if (lfFont.lfCharSet != NATIVE_CHARSET) {
    lfFont.lfWeight = FW_NORMAL;
    lfFont.lfCharSet = NATIVE_CHARSET;
    lfFont.lfFaceName[0] = 0;
    SelectObject(hDC, CreateFontIndirect(&lfFont));
  } else {
    hOldFont = NULL;
  }
  return hOldFont;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
