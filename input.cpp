// input.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// A function which handles WM_IME_KEYDOWN
BOOL PASCAL IMEKeydownHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                              LPBYTE lpbKeyState) {
  WORD wVKey = (wParam & 0x00FF);
  switch (wVKey) {
    case VK_SHIFT:
    case VK_CONTROL:
      // goto not_proccessed;
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
