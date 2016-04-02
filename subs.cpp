// subs.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

void PASCAL InitCompStr(LPCOMPOSITIONSTRING lpCompStr, DWORD dwClrFlag) {
  lpCompStr->dwSize = sizeof(MZCOMPSTR);

  if (dwClrFlag & CLR_UNDET) {
    lpCompStr->dwCompReadAttrOffset =
        (DWORD)((LONG_PTR)((LPMZCOMPSTR)lpCompStr)->bCompReadAttr -
                (LONG_PTR)lpCompStr);
    lpCompStr->dwCompReadClauseOffset =
        (DWORD)((LONG_PTR)((LPMZCOMPSTR)lpCompStr)->dwCompReadClause -
                (LONG_PTR)lpCompStr);
    lpCompStr->dwCompReadStrOffset =
        (DWORD)((LONG_PTR)((LPMZCOMPSTR)lpCompStr)->szCompReadStr -
                (LONG_PTR)lpCompStr);
    lpCompStr->dwCompAttrOffset = (DWORD)(
        (LONG_PTR)((LPMZCOMPSTR)lpCompStr)->bCompAttr - (LONG_PTR)lpCompStr);
    lpCompStr->dwCompClauseOffset = (DWORD)(
        (LONG_PTR)((LPMZCOMPSTR)lpCompStr)->dwCompClause - (LONG_PTR)lpCompStr);
    lpCompStr->dwCompStrOffset = (DWORD)(
        (LONG_PTR)((LPMZCOMPSTR)lpCompStr)->szCompStr - (LONG_PTR)lpCompStr);

    lpCompStr->dwCompStrLen = 0;
    lpCompStr->dwCompReadStrLen = 0;
    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompReadAttrLen = 0;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompReadClauseLen = 0;

    *GETLPCOMPSTR(lpCompStr) = 0;
    *GETLPCOMPREADSTR(lpCompStr) = 0;

    lpCompStr->dwCursorPos = 0;
  }

  if (dwClrFlag & CLR_RESULT) {
    lpCompStr->dwResultStrOffset = (DWORD)(
        (LONG_PTR)((LPMZCOMPSTR)lpCompStr)->szResultStr - (LONG_PTR)lpCompStr);
    lpCompStr->dwResultClauseOffset =
        (DWORD)((LONG_PTR)((LPMZCOMPSTR)lpCompStr)->dwResultClause -
                (LONG_PTR)lpCompStr);
    lpCompStr->dwResultReadStrOffset =
        (DWORD)((LONG_PTR)((LPMZCOMPSTR)lpCompStr)->szResultReadStr -
                (LONG_PTR)lpCompStr);
    lpCompStr->dwResultReadClauseOffset =
        (DWORD)((LONG_PTR)((LPMZCOMPSTR)lpCompStr)->dwResultReadClause -
                (LONG_PTR)lpCompStr);

    lpCompStr->dwResultStrLen = 0;
    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultReadClauseLen = 0;

    *GETLPRESULTSTR(lpCompStr) = 0;
    *GETLPRESULTREADSTR(lpCompStr) = 0;
  }
}

void PASCAL ClearCompStr(LPCOMPOSITIONSTRING lpCompStr, DWORD dwClrFlag) {
  lpCompStr->dwSize = sizeof(MZCOMPSTR);

  if (dwClrFlag & CLR_UNDET) {
    lpCompStr->dwCompStrOffset = 0;
    lpCompStr->dwCompClauseOffset = 0;
    lpCompStr->dwCompAttrOffset = 0;
    lpCompStr->dwCompReadStrOffset = 0;
    lpCompStr->dwCompReadClauseOffset = 0;
    lpCompStr->dwCompReadAttrOffset = 0;
    lpCompStr->dwCompStrLen = 0;
    lpCompStr->dwCompClauseLen = 0;
    lpCompStr->dwCompAttrLen = 0;
    lpCompStr->dwCompReadStrLen = 0;
    lpCompStr->dwCompReadClauseLen = 0;
    lpCompStr->dwCompReadAttrLen = 0;
    ((LPMZCOMPSTR)lpCompStr)->szCompStr[0] = 0;
    ((LPMZCOMPSTR)lpCompStr)->szCompReadStr[0] = 0;
    lpCompStr->dwCursorPos = 0;
  }

  if (dwClrFlag & CLR_RESULT) {
    lpCompStr->dwResultStrOffset = 0;
    lpCompStr->dwResultClauseOffset = 0;
    lpCompStr->dwResultReadStrOffset = 0;
    lpCompStr->dwResultReadClauseOffset = 0;
    lpCompStr->dwResultStrLen = 0;
    lpCompStr->dwResultClauseLen = 0;
    lpCompStr->dwResultReadStrLen = 0;
    lpCompStr->dwResultReadClauseLen = 0;
    ((LPMZCOMPSTR)lpCompStr)->szResultStr[0] = 0;
    ((LPMZCOMPSTR)lpCompStr)->szResultReadStr[0] = 0;
  }
}

void PASCAL ClearCandidate(LPCANDIDATEINFO lpCandInfo) {
  lpCandInfo->dwSize = 0L;
  lpCandInfo->dwCount = 0L;
  lpCandInfo->dwOffset[0] = 0L;

  ((LPMZCAND)lpCandInfo)->cl.dwSize = 0L;
  ((LPMZCAND)lpCandInfo)->cl.dwStyle = 0L;
  ((LPMZCAND)lpCandInfo)->cl.dwCount = 0L;
  ((LPMZCAND)lpCandInfo)->cl.dwSelection = 0L;
  ((LPMZCAND)lpCandInfo)->cl.dwPageStart = 0L;
  ((LPMZCAND)lpCandInfo)->cl.dwPageSize = 0L;
  ((LPMZCAND)lpCandInfo)->cl.dwOffset[0] = 0L;
}

// return value: fdwConversion
void PASCAL ChangeMode(HIMC hIMC, DWORD dwToMode) {
  LPINPUTCONTEXT lpIMC;
  DWORD fdwConversion;
  TRANSMSG GnMsg;

  if (!(lpIMC = ImmLockIMC(hIMC))) return;

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
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
  }

  ImmUnlockIMC(hIMC);
  return;
}

void PASCAL ChangeCompStr(HIMC hIMC, DWORD dwToMode) {
  LPINPUTCONTEXT lpIMC;
  LPCOMPOSITIONSTRING lpCompStr;
  //DWORD fdwConversion;
  TRANSMSG GnMsg;
  HANDLE hDst;
  LPTSTR lpSrc;
  LPTSTR lpDst;
  LPTSTR lpSrc0;
  LPTSTR lpDst0;
  //WORD wCode;
  BOOL fChange = FALSE;

  if (!(lpIMC = ImmLockIMC(hIMC))) return;

  if (!(lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)))
    goto ccs_exit40;

  //fdwConversion = lpIMC->fdwConversion;

  if (!(hDst =
            GlobalAlloc(GHND, (lpCompStr->dwCompStrLen + 1) * sizeof(WCHAR))))
    goto ccs_exit30;

  if (!(lpDst = (LPTSTR)GlobalLock(hDst))) goto ccs_exit20;

  switch (dwToMode) {
    case TO_CMODE_ALPHANUMERIC:
      break;

    case TO_CMODE_KATAKANA:
      lpSrc = ((LPMZCOMPSTR)lpCompStr)->szCompStr;
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
      lpSrc = ((LPMZCOMPSTR)lpCompStr)->szCompStr;
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
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
  }

  GlobalUnlock(hDst);
ccs_exit20:
  GlobalFree(hDst);
ccs_exit30:
  ImmUnlockIMCC(lpIMC->hCompStr);
ccs_exit40:
  ImmUnlockIMC(hIMC);
  return;
}

BOOL PASCAL IsCompStr(HIMC hIMC) {
  LPINPUTCONTEXT lpIMC;
  LPCOMPOSITIONSTRING lpCompStr;
  BOOL fRet = FALSE;

  if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

  if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof(COMPOSITIONSTRING)) {
    ImmUnlockIMC(hIMC);
    return FALSE;
  }

  lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

  fRet = (lpCompStr->dwCompStrLen > 0);

  ImmUnlockIMCC(lpIMC->hCompStr);
  ImmUnlockIMC(hIMC);

  return fRet;
}

BOOL PASCAL IsConvertedCompStr(HIMC hIMC) {
  LPINPUTCONTEXT lpIMC;
  LPCOMPOSITIONSTRING lpCompStr;
  BOOL fRet = FALSE;

  if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

  if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof(MZCOMPSTR)) {
    ImmUnlockIMC(hIMC);
    return FALSE;
  }

  lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

  if (lpCompStr->dwCompStrLen > 0)
    fRet = (((LPMZCOMPSTR)lpCompStr)->bCompAttr[0] > 0);

  ImmUnlockIMCC(lpIMC->hCompStr);
  ImmUnlockIMC(hIMC);

  return fRet;
}

BOOL PASCAL IsCandidate(LPINPUTCONTEXT lpIMC) {
  LPCANDIDATEINFO lpCandInfo;
  BOOL fRet = FALSE;

  if (ImmGetIMCCSize(lpIMC->hCandInfo) < sizeof(CANDIDATEINFO)) return FALSE;

  lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
  if (lpCandInfo) {
    fRet = (lpCandInfo->dwCount > 0);
    ImmUnlockIMCC(lpIMC->hCandInfo);
  }
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
    ImmGetIMEFileName(hKLTemp, szFile, sizeof(szFile) / sizeof(szFile[0]));

    if (!lstrcmp(szFile, MZIME_FILENAME)) {
      hKL = hKLTemp;
      break;
    }
  }

  GlobalFree((HANDLE)lphkl);
  return hKL;
}

void PASCAL UpdateIndicIcon(HIMC hIMC) {
  if (!hMyKL) {
    hMyKL = GetMyHKL();
    if (!hMyKL) return;
  }

  HWND hwndIndicate = FindWindow(INDICATOR_CLASS, NULL);

  LPINPUTCONTEXT lpIMC;
  BOOL fOpen = FALSE;
  if (hIMC) {
    lpIMC = ImmLockIMC(hIMC);
    if (lpIMC) {
      fOpen = lpIMC->fOpen;
      ImmUnlockIMC(hIMC);
    }
  }

  if (IsWindow(hwndIndicate)) {
    ATOM atomTip;

    atomTip = GlobalAddAtom(TEXT("MZ-IME Open"));
    PostMessage(hwndIndicate, INDICM_SETIMEICON, fOpen ? 1 : (-1),
                (LPARAM)hMyKL);
    PostMessage(hwndIndicate, INDICM_SETIMETOOLTIPS, fOpen ? atomTip : (-1),
                (LPARAM)hMyKL);
    PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS,
                // fOpen ? (RDMI_LEFT | RDMI_RIGHT) : 0, (LPARAM)hMyKL);
                fOpen ? (RDMI_LEFT) : 0, (LPARAM)hMyKL);
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
