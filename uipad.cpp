// uipad.cpp --- mzimeja IME Pad UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// create IME Pad window
HWND ImePad_Create(HWND hWnd, UIEXTRA *lpUIExtra) {
  FOOTMARK();
  return NULL;
} // ImePad_Create

LRESULT CALLBACK
ImePad_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  FOOTMARK();

  switch (message) {
  case WM_CREATE:
    break;

  case WM_DESTROY:
    break;

  default:
    if (!IsImeMessage(message))
      return ::DefWindowProc(hWnd, message, wParam, lParam);
    break;
  }
  return 0;
} // ImePad_WindowProc

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
