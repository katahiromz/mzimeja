// uistate.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
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

enum InputMode {
  IMODE_ZEN_HIRAGANA,
  IMODE_ZEN_KATAKANA,
  IMODE_ZEN_EISUU,
  IMODE_HAN_KANA,
  IMODE_HAN_EISUU
};

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

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// create status window
HWND StatusWnd_Create(HWND hWnd, LPUIEXTRA lpUIExtra) {
  const DWORD style = WS_DISABLED | WS_POPUP;
  const DWORD exstyle = WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME;
  HWND hwndStatus = lpUIExtra->uiStatus.hWnd;
  if (!::IsWindow(hwndStatus)) {
    INT cx, cy;
    cx = CX_MINICAPTION + CX_BUTTON * 3;
    cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
    cx += 3 * CX_BTNEDGE * 2;
    cy = CY_BUTTON;
    cy += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
    cy += 2 * CY_BTNEDGE;
    hwndStatus = ::CreateWindowEx(
      exstyle, szStatusClassName, NULL, style,
      lpUIExtra->uiStatus.pt.x, lpUIExtra->uiStatus.pt.y,
      cx, cy,
      hWnd, NULL, TheApp.m_hInst, NULL);
    lpUIExtra->uiStatus.hWnd = hwndStatus;
  }
  RepositionWindow(hwndStatus);
  ::ShowWindow(hwndStatus, SW_SHOWNOACTIVATE);
  lpUIExtra->uiStatus.bShow = TRUE;
  SetWindowLongPtr(hwndStatus, FIGWLP_SERVERWND, (LONG_PTR)hWnd);
  return hwndStatus;
} // StatusWnd_Create

// draw status window
void StatusWnd_Paint(HWND hWnd, HDC hDC, INT nPushed) {
  RECT rc;
  HBITMAP hbmStatus;
  HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
  HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
  if (hIMC) {
    InputContext *lpIMC = TheApp.LockIMC(hIMC);
    if (lpIMC) {
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
        if (lpIMC->IsOpen()) {
          ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                   CX_BUTTON, CY_BUTTON,
                   hMemDC, 0, 7 * CY_BUTTON, SRCCOPY);
        } else {
          ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                   CX_BUTTON, CY_BUTTON,
                   hMemDC, 0, 8 * CY_BUTTON, SRCCOPY);
        }

        // draw input mode
        rc.left += CX_BUTTON + CX_BTNEDGE * 2;
        if (lpIMC->IsOpen()) {
          if (lpIMC->Conversion() & IME_CMODE_FULLSHAPE) {
            if (lpIMC->Conversion() & IME_CMODE_NATIVE) {
              if (lpIMC->Conversion() & IME_CMODE_KATAKANA) {
                // zenkaku katakana
                ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                         CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 1 * CY_BUTTON, SRCCOPY);
              } else {
                // zenkaku hiragana
                ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                         CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 0 * CY_BUTTON, SRCCOPY);
              }
            } else {
              // zenkaku alphanumeric
              ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                       CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 2 * CY_BUTTON, SRCCOPY);
            }
          } else {
            if (lpIMC->Conversion() & IME_CMODE_NATIVE) {
              // hankaku kana
              ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                       CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 3 * CY_BUTTON, SRCCOPY);
            } else {
              // hankaku alphanumeric
              ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                       CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
            }
          }
        } else {
          // hankaku alphanumeric
          ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                   CX_BUTTON, CY_BUTTON,
                   hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
        }

        // draw roman mode
        rc.left += CX_BUTTON + CX_BTNEDGE * 2;
        if (lpIMC->Conversion() & IME_CMODE_ROMAN) {
          ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                   CX_BUTTON, CY_BUTTON,
                   hMemDC, 0, 5 * CY_BUTTON, SRCCOPY);
        } else {
          ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                   CX_BUTTON, CY_BUTTON,
                   hMemDC, 0, 6 * CY_BUTTON, SRCCOPY);
        }

        ::SelectObject(hMemDC, hbmOld);
        ::DeleteDC(hMemDC);
      }
      TheApp.UnlockIMC();
    }
  }
} // StatusWnd_Paint

STATUS_WND_HITTEST StatusWnd_HitTest(HWND hWnd, POINT pt) {
  ::ScreenToClient(hWnd, &pt);
  RECT rc;
  ::GetClientRect(hWnd, &rc);
  rc.left += CX_MINICAPTION;
  rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
  if (::PtInRect(&rc, pt)) {
    DebugPrint(TEXT("status hit: %d"), SWHT_BUTTON_1);
    return SWHT_BUTTON_1;
  }
  ::GetClientRect(hWnd, &rc);
  rc.left += CX_MINICAPTION + CX_BUTTON + 2 * CX_BTNEDGE;
  rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
  if (::PtInRect(&rc, pt)) {
    DebugPrint(TEXT("status hit: %d"), SWHT_BUTTON_2);
    return SWHT_BUTTON_2;
  }
  ::GetClientRect(hWnd, &rc);
  rc.left += CX_MINICAPTION + 2 * (CX_BUTTON + 2 * CX_BTNEDGE);
  rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
  if (::PtInRect(&rc, pt)) {
    DebugPrint(TEXT("status hit: %d"), SWHT_BUTTON_3);
    return SWHT_BUTTON_3;
  }
  ::GetWindowRect(hWnd, &rc);
  ::ClientToScreen(hWnd, &pt);
  if (::PtInRect(&rc, pt)) {
    return SWHT_CAPTION;
  }
  return SWHT_NONE;
} // StatusWnd_HitTest

void StatusWnd_Update(LPUIEXTRA lpUIExtra) {
  if (IsWindow(lpUIExtra->uiStatus.hWnd))
    SendMessage(lpUIExtra->uiStatus.hWnd, WM_UI_UPDATE, 0, 0L);
} // StatusWnd_Update

void StatusWnd_OnButton(HWND hWnd, STATUS_WND_HITTEST hittest) {
  HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
  HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
  DWORD dwConversion, dwSentence;
  BOOL bOpen = ImmGetOpenStatus(hIMC);
  if (::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence)) {
    InputMode imode;
    switch (hittest) {
    case SWHT_BUTTON_1:
      if (bOpen) {
        SetInputMode(hIMC, IMODE_HAN_EISUU);
      } else {
        SetInputMode(hIMC, IMODE_ZEN_HIRAGANA);
      }
      break;
    case SWHT_BUTTON_2:
      DebugPrint(TEXT("dwConversion: %08X"), dwConversion);
      imode = InputModeFromConversionMode(bOpen, dwConversion);
      DebugPrint(TEXT("imode1: %d"), imode);
      imode = NextInputMode(imode);
      DebugPrint(TEXT("imode2: %d"), imode);
      SetInputMode(hIMC, imode);
      break;
    case SWHT_BUTTON_3:
      if (dwConversion & IME_CMODE_ROMAN) {
        dwConversion &= ~IME_CMODE_ROMAN;
      } else {
        dwConversion |= IME_CMODE_ROMAN;
      }
      ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
      break;
    default:
      break;
    }
  }
}

void StatusWnd_OnMouseMove(HWND hWnd, POINT pt, BOOL bDown) {
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

LRESULT CALLBACK StatusWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                      LPARAM lParam) {
  PAINTSTRUCT ps;
  HWND hwndServer;
  HDC hDC;
  HBITMAP hbm;
  POINT pt;

  switch (message) {
  case WM_CREATE:
    DebugPrint(TEXT("status message: WM_CREATE"));
    hbm = TheApp.LoadBMP(TEXT("MODESBMP"));
    SetWindowLongPtr(hWnd, FIGWLP_STATUSBMP, (LONG_PTR)hbm);
    break;

  case WM_PAINT:
    hDC = ::BeginPaint(hWnd, &ps);
    StatusWnd_Paint(hWnd, hDC, 0);
    ::EndPaint(hWnd, &ps);
    break;

  case WM_DESTROY:
    DebugPrint(TEXT("status message: WM_DESTROY"));
    hbm = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
    ::DeleteObject(hbm);
    break;

  case WM_UI_UPDATE:
    DebugPrint(TEXT("status message: WM_UI_UPDATE"));
    ::InvalidateRect(hWnd, NULL, FALSE);
    break;

  case WM_LBUTTONUP:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    DebugPrint(TEXT("status: WM_LBUTTONUP"));
    StatusWnd_OnLButton(hWnd, pt, FALSE);
    break;

  case WM_LBUTTONDOWN:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    DebugPrint(TEXT("status: WM_LBUTTONDOWN"));
    StatusWnd_OnLButton(hWnd, pt, TRUE);
    break;

  case WM_MOUSEMOVE:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    DebugPrint(TEXT("status: WM_MOUSEMOVE"));
    if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
      StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
    }
    break;

  case WM_RBUTTONUP:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    DebugPrint(TEXT("status: WM_RBUTTONUP"));
    break;

  case WM_RBUTTONDOWN:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    DebugPrint(TEXT("status: WM_RBUTTONDOWN"));
    break;

  case WM_SETCURSOR:
    // This message comes even from the disabled window.
    ::GetCursorPos(&pt);
    switch (HIWORD(lParam)) {
    case WM_MOUSEMOVE:
      DebugPrint(TEXT("status: WM_SETCURSOR WM_MOUSEMOVE"));
      if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
        StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
      }
      break;
    case WM_LBUTTONDOWN:
      DebugPrint(TEXT("status: WM_SETCURSOR WM_LBUTTONDOWN"));
      StatusWnd_OnLButton(hWnd, pt, TRUE);
      break;
    case WM_LBUTTONUP:
      DebugPrint(TEXT("status: WM_SETCURSOR WM_LBUTTONUP"));
      StatusWnd_OnLButton(hWnd, pt, FALSE);
      break;
    case WM_RBUTTONDOWN:
      DebugPrint(TEXT("status: WM_SETCURSOR WM_RBUTTONDOWN"));
      break;
    case WM_RBUTTONUP:
      DebugPrint(TEXT("status: WM_SETCURSOR WM_RBUTTONUP"));
      break;
    }
    ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    break;

  case WM_MOVE:
    DebugPrint(TEXT("status message: WM_MOVE"));
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
