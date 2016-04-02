// input.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// A function which handles WM_IMEKEYDOWN
BOOL PASCAL IMEKeydownHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                              LPBYTE lpbKeyState) {
  WORD wVKey;

  switch (wVKey = (LOWORD(wParam) & 0x00FF)) {
    case VK_SHIFT:
    case VK_CONTROL:
      // goto not_proccessed;
      break;

    default:
      if (!DicKeydownHandler(hIMC, wVKey, lParam, lpbKeyState)) {
// This WM_IMEKEYDOWN has actual character code in itself.
        AddChar(hIMC, HIWORD(wParam));
        // CharHandler( hIMC,  (WORD)((BYTE)HIBYTE(wParam)), lParam );
      }
      break;
  }
  return TRUE;
}

// A function which handles WM_IMEKEYUP
BOOL PASCAL IMEKeyupHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                            LPBYTE lpbKeyState) {
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
