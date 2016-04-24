// subs.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// return value: fdwConversion
void PASCAL ChangeMode(HIMC hIMC, DWORD dwToMode) {
  DWORD fdwConversion;

  InputContext *lpIMC = TheIME.LockIMC(hIMC);
  if (!lpIMC) return;

  fdwConversion = lpIMC->Conversion();

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

  if (lpIMC->Conversion() != fdwConversion) {
    lpIMC->Conversion() = fdwConversion;
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_SETCONVERSIONMODE);
  }

  TheIME.UnlockIMC(hIMC);
  return;
}

void PASCAL ChangeCompStr(HIMC hIMC, DWORD dwToMode) {
  InputContext *lpIMC;
  CompStr *lpCompStr;
  //DWORD fdwConversion;
  HANDLE hDst;
  LPTSTR lpSrc;
  LPTSTR lpDst;
  LPTSTR lpSrc0;
  LPTSTR lpDst0;
  //WORD wCode;
  BOOL fChange = FALSE;
  DWORD dwSize;

  lpIMC = TheIME.LockIMC(hIMC);
  if (!lpIMC) return;

  if (!(lpCompStr = lpIMC->LockCompStr()))
    goto ccs_exit40;

  //fdwConversion = lpIMC->Conversion();

  dwSize = (lpCompStr->dwCompStrLen + 1) * sizeof(WCHAR);
  if (!(hDst = GlobalAlloc(GHND, dwSize)))
    goto ccs_exit30;

  if (!(lpDst = (LPTSTR)GlobalLock(hDst))) goto ccs_exit20;

  switch (dwToMode) {
    case TO_CMODE_ALPHANUMERIC:
      break;

    case TO_CMODE_KATAKANA:
      lpSrc = lpCompStr->GetCompStr();
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
      lpSrc = lpCompStr->GetCompStr();
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
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPSTR);
  }

  GlobalUnlock(hDst);
ccs_exit20:
  GlobalFree(hDst);
ccs_exit30:
  lpIMC->UnlockCompStr();
ccs_exit40:
  TheIME.UnlockIMC(hIMC);
  return;
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
