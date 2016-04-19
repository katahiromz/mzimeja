// input.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "vksub.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

BOOL PASCAL DicKeydownHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                              LPBYTE lpbKeyState) {
  InputContext *lpIMC;

  switch (wParam) {
    case VK_ESCAPE:
      FlushText(hIMC);
      break;

    case VK_DELETE:
    case VK_BACK:
      DeleteChar(hIMC, wParam);
      break;

    case VK_SPACE:
      ConvKanji(hIMC);
      break;

    case VK_F3:
      if (IsCTLPushed(lpbKeyState)) ChangeMode(hIMC, TO_CMODE_ROMAN);
      break;

    case VK_F6:
      if (IsCTLPushed(lpbKeyState))
        ChangeMode(hIMC, TO_CMODE_HIRAGANA);
      else
        ChangeCompStr(hIMC, TO_CMODE_HIRAGANA);
      break;

    case VK_F7:
      if (IsCTLPushed(lpbKeyState))
        ChangeMode(hIMC, TO_CMODE_KATAKANA);
      else
        ChangeCompStr(hIMC, TO_CMODE_KATAKANA);
      break;

    case VK_F8:
      if (IsCTLPushed(lpbKeyState))
        ChangeMode(hIMC, TO_CMODE_FULLSHAPE);
      else
        ChangeCompStr(hIMC, TO_CMODE_FULLSHAPE);
      break;

    case VK_F9:
      if (IsCTLPushed(lpbKeyState))
        ChangeMode(hIMC, TO_CMODE_ALPHANUMERIC);
      else
        ChangeCompStr(hIMC, TO_CMODE_ALPHANUMERIC);
      break;

    case VK_RETURN:
      lpIMC = TheApp.LockIMC(hIMC);
      if (lpIMC) {
        if (!(lpIMC->fdwConversion & IME_CMODE_CHARCODE))
          MakeResultString(hIMC, TRUE);
        else
          FlushText(hIMC);

        TheApp.UnlockIMC();
      }
      break;

    case VK_G:
      //if (IsCTLPushed(lpbKeyState)) {
      //  MakeGuideLine(hIMC, MYGL_TESTGUIDELINE);
      //  return (TRUE);
      //}
      break;

    default:
      break;
  }

  if ((VK_0 <= wParam && VK_9 >= wParam) ||
      (VK_A <= wParam && VK_Z >= wParam) ||
      (VK_NUMPAD0 <= wParam && VK_NUMPAD9 >= wParam) ||
      (VK_OEM_1 <= wParam && VK_OEM_9 >= wParam) ||
      (VK_MULTIPLY <= wParam && VK_DIVIDE >= wParam)) {
    return FALSE;
  } else {
    return TRUE;
  }
}

// A function which handles WM_IME_KEYDOWN
BOOL PASCAL IMEKeydownHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                              LPBYTE lpbKeyState) {
  WORD wVKey = (wParam & 0x00FF);
  switch (wVKey) {
    case VK_SHIFT:
    case VK_CONTROL:
      break;

    default:
      if (!DicKeydownHandler(hIMC, wVKey, lParam, lpbKeyState)) {
        // This WM_IME_KEYDOWN has actual character code in itself.
        AddChar(hIMC, HIWORD(wParam));
        // CharHandler( hIMC,  (WORD)((BYTE)HIBYTE(wParam)), lParam );
      }
      break;
  }
  return TRUE;
}

// A function which handles WM_IME_KEYUP
BOOL PASCAL IMEKeyupHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                            LPBYTE lpbKeyState) {
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
