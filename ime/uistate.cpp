// uistate.cpp --- mzimeja status window UI
//////////////////////////////////////////////////////////////////////////////

#include "../mzimeja.h"
#include "resource.h"

#define CX_MINICAPTION 10
#define CX_BUTTON 24
#define CY_BUTTON 24
#define CX_BTNEDGE 2
#define CY_BTNEDGE 2

enum STATUS_WND_HITTEST {
  SWHT_NONE,
  SWHT_CAPTION,
  SWHT_BUTTON_1,
  SWHT_BUTTON_2,
  SWHT_BUTTON_3
};

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// create status window
HWND StatusWnd_Create(HWND hWnd, UIEXTRA *lpUIExtra) {
  FOOTMARK();
  const DWORD style = WS_DISABLED | WS_POPUP;
  const DWORD exstyle = WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME;
  HWND hwndStatus = lpUIExtra->hwndStatus;
  if (!::IsWindow(hwndStatus)) {
    INT cx, cy;
    cx = CX_MINICAPTION + CX_BUTTON * 3;
    cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
    cx += 3 * CX_BTNEDGE * 2;
    cy = CY_BUTTON;
    cy += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
    cy += 2 * CY_BTNEDGE;
    POINT pt;
    if (!TheIME.GetUserData(L"ptStatusWindow", &pt, sizeof(pt))) {
      RECT rcWorkArea;
      ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
      pt.x = rcWorkArea.right - cx;
      pt.y = rcWorkArea.bottom - cy;
    }
    hwndStatus = ::CreateWindowEx(
      exstyle, szStatusClassName, NULL, style,
      pt.x, pt.y, cx, cy,
      hWnd, NULL, TheIME.m_hInst, NULL);
    lpUIExtra->hwndStatus = hwndStatus;
  } else {
    StatusWnd_Update(lpUIExtra);
  }
  RepositionWindow(hwndStatus);
  ::ShowWindow(hwndStatus, SW_SHOWNOACTIVATE);
  ::SetWindowLongPtr(hwndStatus, FIGWLP_SERVERWND, (LONG_PTR)hWnd);
  return hwndStatus;
} // StatusWnd_Create

// draw status window
void StatusWnd_Paint(HWND hWnd, HDC hDC, INT nPushed) {
  FOOTMARK();
  RECT rc;
  HBITMAP hbmStatus;
  HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
  HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
  InputContext *lpIMC = TheIME.LockIMC(hIMC);

  // draw face
  HBRUSH hbr3DFace = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
  ::GetClientRect(hWnd, &rc);
  ::FillRect(hDC, &rc, hbr3DFace);
  DeleteObject(hbr3DFace);

  // draw caption
  HBRUSH hbrCaption = ::CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
  rc.right = rc.left + CX_MINICAPTION;
  ::FillRect(hDC, &rc, hbrCaption);
  ::DeleteObject(hbrCaption);

  ::GetClientRect(hWnd, &rc);
  hbmStatus = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
  HDC hMemDC = ::CreateCompatibleDC(hDC);
  if (hMemDC) {
    RECT rcButton;
    HGDIOBJ hbmOld = ::SelectObject(hMemDC, hbmStatus);

    rc.left += CX_MINICAPTION;
    rcButton.left = rc.left;
    rcButton.top = rc.top;
    rcButton.right = rc.left + CX_BUTTON + 4;
    rcButton.bottom = rc.bottom;
    if (nPushed == 1) {
      ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
        DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
      ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH);
    }

    rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
    rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
    if (nPushed == 2) {
      ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
        DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
      ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
        DFCS_BUTTONPUSH);
    }

    rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
    rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
    if (nPushed == 3) {
      ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
        DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
      ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
        DFCS_BUTTONPUSH);
    }

    // draw ime on/off
    if (lpIMC) {
      if (lpIMC->IsOpen()) {
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 7 * CY_BUTTON, SRCCOPY);
      } else {
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 8 * CY_BUTTON, SRCCOPY);
      }
    } else {
      // disabled
      ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
               CX_BUTTON, CY_BUTTON,
               hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    // draw input mode
    rc.left += CX_BUTTON + CX_BTNEDGE * 2;
    if (lpIMC) {
      if (lpIMC->IsOpen()) {
        if (lpIMC->Conversion() & IME_CMODE_FULLSHAPE) {
          if (lpIMC->Conversion() & IME_CMODE_JAPANESE) {
            if (lpIMC->Conversion() & IME_CMODE_KATAKANA) {
              // fullwidth katakana
              ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                       CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 1 * CY_BUTTON, SRCCOPY);
            } else {
              // fullwidth hiragana
              ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                       CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 0 * CY_BUTTON, SRCCOPY);
            }
          } else {
            // fullwidth alphanumeric
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 2 * CY_BUTTON, SRCCOPY);
          }
        } else {
          if (lpIMC->Conversion() & IME_CMODE_JAPANESE) {
            // halfwidth kana
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 3 * CY_BUTTON, SRCCOPY);
          } else {
            // halfwidth alphanumeric
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
          }
        }
      } else {
        // halfwidth alphanumeric
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
      }
    } else {
      // disabled
      ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
               CX_BUTTON, CY_BUTTON,
               hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    // draw roman mode
    rc.left += CX_BUTTON + CX_BTNEDGE * 2;
    if (lpIMC) {
      if (lpIMC->Conversion() & IME_CMODE_ROMAN) {
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 5 * CY_BUTTON, SRCCOPY);
      } else {
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 6 * CY_BUTTON, SRCCOPY);
      }
    } else {
      // disabled
      ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
               CX_BUTTON, CY_BUTTON,
               hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    ::SelectObject(hMemDC, hbmOld);
    ::DeleteDC(hMemDC);
  }
  if (lpIMC) TheIME.UnlockIMC(hIMC);
} // StatusWnd_Paint

STATUS_WND_HITTEST StatusWnd_HitTest(HWND hWnd, POINT pt) {
  FOOTMARK();
  ::ScreenToClient(hWnd, &pt);
  RECT rc;
  ::GetClientRect(hWnd, &rc);
  rc.left += CX_MINICAPTION;
  rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
  if (::PtInRect(&rc, pt)) {
    return SWHT_BUTTON_1;
  }
  ::GetClientRect(hWnd, &rc);
  rc.left += CX_MINICAPTION + CX_BUTTON + 2 * CX_BTNEDGE;
  rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
  if (::PtInRect(&rc, pt)) {
    return SWHT_BUTTON_2;
  }
  ::GetClientRect(hWnd, &rc);
  rc.left += CX_MINICAPTION + 2 * (CX_BUTTON + 2 * CX_BTNEDGE);
  rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
  if (::PtInRect(&rc, pt)) {
    return SWHT_BUTTON_3;
  }
  ::GetWindowRect(hWnd, &rc);
  ::ClientToScreen(hWnd, &pt);
  if (::PtInRect(&rc, pt)) {
    return SWHT_CAPTION;
  }
  return SWHT_NONE;
} // StatusWnd_HitTest

void StatusWnd_Update(UIEXTRA *lpUIExtra) {
  FOOTMARK();
  HWND hwndStatus = lpUIExtra->hwndStatus;
  if (::IsWindow(hwndStatus)) {
    POINT pt;
    if (TheIME.GetUserData(L"ptStatusWindow", &pt, sizeof(pt))) {
      RECT rc;
      ::GetWindowRect(hwndStatus, &rc);
      ::MoveWindow(hwndStatus, pt.x, pt.y,
        rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }
    ::SendMessage(hwndStatus, WM_UI_UPDATE, 0, 0);
  }
} // StatusWnd_Update

void StatusWnd_OnButton(HWND hWnd, STATUS_WND_HITTEST hittest) {
  FOOTMARK();
  HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
  HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
  if (hIMC == NULL) {
    return;
  }
  DWORD dwConversion, dwSentence;
  BOOL bOpen = ImmGetOpenStatus(hIMC);
  if (::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence)) {
    INPUT_MODE imode;
    switch (hittest) {
    case SWHT_BUTTON_1:
      if (bOpen) {
        SetInputMode(hIMC, IMODE_HALF_ASCII);
      } else {
        SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
      }
      break;
    case SWHT_BUTTON_2:
      imode = InputModeFromConversionMode(bOpen, dwConversion);
      imode = NextInputMode(imode);
      SetInputMode(hIMC, imode);
      break;
    case SWHT_BUTTON_3:
      if (dwConversion & IME_CMODE_ROMAN) {
        dwConversion &= ~IME_CMODE_ROMAN;
      } else {
        dwConversion |= IME_CMODE_ROMAN;
      }
      ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
      break;
    default:
      break;
    }
  }
}

void StatusWnd_OnMouseMove(HWND hWnd, POINT pt, BOOL bDown) {
  FOOTMARK();
  static POINT prev = {-1, -1};
  if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
    if (bDown && ::GetCapture() == hWnd) {
      if (prev.x != -1 && prev.y != -1) {
        RECT rc;
        ::GetWindowRect(hWnd, &rc);
        ::MoveWindow(hWnd,
          rc.left + (pt.x - prev.x), rc.top + (pt.y - prev.y),
          rc.right - rc.left, rc.bottom - rc.top,
          TRUE);
      }
      prev = pt;
    } else {
      prev.x = -1;
      prev.y = -1;
      ::ReleaseCapture();
      ::SetWindowLong(hWnd, FIGWL_MOUSE, SWHT_NONE);
    }
  }
}

void StatusWnd_OnLButton(HWND hWnd, POINT pt, BOOL bDown) {
  FOOTMARK();

  STATUS_WND_HITTEST hittest = StatusWnd_HitTest(hWnd, pt);
  switch (hittest) {
  case SWHT_CAPTION:
    break;
  case SWHT_BUTTON_1:
    if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_1) {
      HDC hDC = ::GetDC(hWnd);
      StatusWnd_Paint(hWnd, hDC, (bDown ? 1 : 0));
      ::ReleaseDC(hWnd, hDC);
    }
    break;
  case SWHT_BUTTON_2:
    if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_2) {
      HDC hDC = ::GetDC(hWnd);
      StatusWnd_Paint(hWnd, hDC, (bDown ? 2 : 0));
      ::ReleaseDC(hWnd, hDC);
    }
    break;
  case SWHT_BUTTON_3:
    if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_3) {
      HDC hDC = ::GetDC(hWnd);
      StatusWnd_Paint(hWnd, hDC, (bDown ? 3 : 0));
      ::ReleaseDC(hWnd, hDC);
    }
    break;
  case SWHT_NONE:
    {
      HDC hDC = ::GetDC(hWnd);
      StatusWnd_Paint(hWnd, hDC, 0);
      ::ReleaseDC(hWnd, hDC);
    }
    break;
  }
  if (bDown) {
    ::SetCapture(hWnd);
    ::SetWindowLong(hWnd, FIGWL_MOUSE, hittest);
  } else {
    ::ReleaseCapture();
    if (hittest == SWHT_CAPTION) {
      RepositionWindow(hWnd);
    } else {
      StatusWnd_OnButton(hWnd, hittest);
      ::SetWindowLong(hWnd, FIGWL_MOUSE, SWHT_NONE);
    }
  }
} // StatusWnd_OnLButton

static BOOL StatusWnd_OnRClick(HWND hWnd, POINT pt) {
  FOOTMARK();
  HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
  HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
  if (hIMC == NULL) return FALSE;

  HMENU hMenu = ::LoadMenu(TheIME.m_hInst, TEXT("STATUSRMENU"));
  if (hMenu) {
    HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
    TPMPARAMS params;
    params.cbSize = sizeof(params);
    ::GetWindowRect(hWnd, &params.rcExclude);
    HWND hwndFore = ::GetForegroundWindow();
    ::SetForegroundWindow(hWnd);

    UINT uCheck = CommandFromInputMode(GetInputMode(hIMC));
    ::CheckMenuRadioItem(hSubMenu, IDM_HIRAGANA, IDM_HALF_ASCII, uCheck, MF_BYCOMMAND);

    if (IsRomanMode(hIMC)) {
      ::CheckMenuRadioItem(hSubMenu, IDM_ROMAN_INPUT, IDM_KANA_INPUT,
                           IDM_ROMAN_INPUT, MF_BYCOMMAND);
    } else {
      ::CheckMenuRadioItem(hSubMenu, IDM_ROMAN_INPUT, IDM_KANA_INPUT,
                           IDM_KANA_INPUT, MF_BYCOMMAND);
    }

    UINT nCommand = ::TrackPopupMenuEx(hSubMenu, TPM_RETURNCMD | TPM_NONOTIFY,
      pt.x, pt.y, hWnd, &params);
    TheIME.DoCommand(hIMC, nCommand);
    ::PostMessage(hWnd, WM_NULL, 0, 0);
    ::DestroyMenu(hMenu);
    ::SetForegroundWindow(hwndFore);
  }
  return TRUE;
} // StatusWnd_OnRClick

LRESULT CALLBACK
StatusWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  FOOTMARK();
  PAINTSTRUCT ps;
  HWND hwndServer;
  HDC hDC;
  HBITMAP hbm;
  POINT pt;

  switch (message) {
  case WM_CREATE:
    hbm = TheIME.LoadBMP(TEXT("MODESBMP"));
    SetWindowLongPtr(hWnd, FIGWLP_STATUSBMP, (LONG_PTR)hbm);
    break;

  case WM_PAINT:
    hDC = ::BeginPaint(hWnd, &ps);
    StatusWnd_Paint(hWnd, hDC, 0);
    ::EndPaint(hWnd, &ps);
    break;

  case WM_DESTROY:
    hbm = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
    ::DeleteObject(hbm);
    break;

  case WM_UI_UPDATE:
    ::InvalidateRect(hWnd, NULL, FALSE);
    break;

  case WM_LBUTTONUP:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    StatusWnd_OnLButton(hWnd, pt, FALSE);
    break;

  case WM_LBUTTONDOWN:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    StatusWnd_OnLButton(hWnd, pt, TRUE);
    break;

  case WM_MOUSEMOVE:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
      StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
    }
    break;

  case WM_RBUTTONUP:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    break;

  case WM_RBUTTONDOWN:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    break;

  case WM_SETCURSOR:
    // This message comes even from the disabled window.
    ::GetCursorPos(&pt);
    switch (HIWORD(lParam)) {
    case WM_MOUSEMOVE:
      if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
        StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
      }
      break;
    case WM_LBUTTONDOWN:
      StatusWnd_OnLButton(hWnd, pt, TRUE);
      break;
    case WM_LBUTTONUP:
      StatusWnd_OnLButton(hWnd, pt, FALSE);
      break;
    case WM_RBUTTONDOWN:
      break;
    case WM_RBUTTONUP:
      StatusWnd_OnRClick(hWnd, pt);
      break;
    }
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    break;

  case WM_MOVE:
    hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    if (::IsWindow(hwndServer))
      SendMessage(hwndServer, WM_UI_STATEMOVE, wParam, lParam);
    break;

  default:
    if (!IsImeMessage(message))
      return ::DefWindowProc(hWnd, message, wParam, lParam);
    break;
  }
  return 0;
} // StatusWnd_WindowProc

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
