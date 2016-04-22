// uistate.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

#define CX_BUTTON 24
#define CY_BUTTON 24

enum STATUS_WND_HITTEST {
  SWHT_CAPTION,
  SWHT_BUTTON_1,
  SWHT_BUTTON_2
};

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
    cx = 10 + CX_BUTTON * 2;
    cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
    cy = CY_BUTTON;
    cy += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
    hwndStatus = ::CreateWindowEx(
      exstyle, szStatusClassName, NULL, style,
      lpUIExtra->uiStatus.pt.x, lpUIExtra->uiStatus.pt.y,
      cx, cy,
      hWnd, NULL, TheApp.m_hInst, NULL);
    lpUIExtra->uiStatus.hWnd = hwndStatus;
  }
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
      rc.right = rc.left + 10;
      ::InflateRect(&rc, -2, -2);
      ::FillRect(hDC, &rc, hbrCaption);
      ::DeleteObject(hbrCaption);

      // lpIMC->IsOpen()

      ::GetClientRect(hWnd, &rc);
      hbmStatus = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
      HDC hMemDC = ::CreateCompatibleDC(hDC);
      if (hMemDC) {
        HGDIOBJ hbmOld = ::SelectObject(hMemDC, hbmStatus);

        // draw input mode
        rc.left += 10;
        if (lpIMC->IsOpen()) {
          if (lpIMC->Conversion() & IME_CMODE_FULLSHAPE) {
            if (lpIMC->Conversion() & IME_CMODE_NATIVE) {
              if (lpIMC->Conversion() & IME_CMODE_KATAKANA) {
                // zenkaku katakana
                ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 1 * CY_BUTTON, SRCCOPY);
              } else {
                // zenkaku hiragana
                ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 0 * CY_BUTTON, SRCCOPY);
              }
            } else {
              // zenkaku alphanumeric
              ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 2 * CY_BUTTON, SRCCOPY);
            }
          } else {
            if (lpIMC->Conversion() & IME_CMODE_NATIVE) {
              // hankaku kana
              ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 3 * CY_BUTTON, SRCCOPY);
            } else {
              // hankaku alphanumeric
              ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
                       hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
            }
          }
        } else {
          // hankaku alphanumeric
          ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
                   hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
        }

        // draw roman mode
        rc.left += CX_BUTTON;
        if (lpIMC->Conversion() & IME_CMODE_ROMAN) {
          ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
                   hMemDC, 0, 5 * CY_BUTTON, SRCCOPY);
        } else {
          ::BitBlt(hDC, rc.left, rc.top, CX_BUTTON, CY_BUTTON,
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
  rc.left += 10;
  rc.right = rc.left + CX_BUTTON;
  if (::PtInRect(&rc, pt)) {
    DebugPrint(TEXT("status hit: %d"), SWHT_BUTTON_1);
    return SWHT_BUTTON_1;
  }
  ::GetClientRect(hWnd, &rc);
  rc.left += 10 + CX_BUTTON;
  rc.right = rc.left + CX_BUTTON;
  if (::PtInRect(&rc, pt)) {
    DebugPrint(TEXT("status hit: %d"), SWHT_BUTTON_2);
    return SWHT_BUTTON_2;
  }
  DebugPrint(TEXT("status hit: %d"), SWHT_CAPTION);
  return SWHT_CAPTION;
} // StatusWnd_HitTest

void StatusWnd_Update(LPUIEXTRA lpUIExtra) {
  if (IsWindow(lpUIExtra->uiStatus.hWnd))
    SendMessage(lpUIExtra->uiStatus.hWnd, WM_UI_UPDATE, 0, 0L);
} // StatusWnd_Update

void StatusWnd_OnButton(HWND hWnd, POINT pt, INT nPushed) {
  DebugPrint(TEXT("status button: %d"), nPushed);
} // StatusWnd_OnButton

void StatusWnd_OnLButton(HWND hWnd, POINT pt, BOOL bDown) {
  static POINT prev = {-1, -1};
  STATUS_WND_HITTEST hittest = StatusWnd_HitTest(hWnd, pt);
  switch (hittest) {
  case SWHT_CAPTION:
    if (prev.x != -1 && prev.y != -1) {
      RECT rc;
      ::GetWindowRect(hWnd, &rc);
      MoveWindow(hWnd,
        rc.left + (pt.x - prev.x),
        rc.top + (pt.y - prev.y),
        rc.right - rc.left,
        rc.bottom - rc.top,
        TRUE);
    } else {
      if (bDown) {
        ::SetCapture(hWnd);
      }
    }
    prev = pt;
    break;
  case SWHT_BUTTON_1:
    StatusWnd_OnButton(hWnd, pt, 1);
    break;
  case SWHT_BUTTON_2:
    StatusWnd_OnButton(hWnd, pt, 2);
    break;
  }
  HDC hDC = ::GetDC(hWnd);
  StatusWnd_Paint(hWnd, hDC, 0);
  ::ReleaseDC(hWnd, hDC);
  ::SetWindowLong(hWnd, FIGWL_MOUSE, bDown);
  if (!bDown) {
    prev.x = -1;
    prev.y = -1;
    ::ReleaseCapture();
  }
} // StatusWnd_OnLButton

LRESULT CALLBACK StatusWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                      LPARAM lParam) {
  PAINTSTRUCT ps;
  HWND hwndServer;
  HDC hDC;
  HBITMAP hbm;
  POINT pt;
  STATUS_WND_HITTEST hittest;

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

  case WM_NCHITTEST:
    DebugPrint(TEXT("status message: WM_NCHITTEST"));
    pt.x = (SHORT)LOWORD(lParam);
    pt.y = (SHORT)HIWORD(lParam);
    hittest = StatusWnd_HitTest(hWnd, pt);
    switch (hittest) {
    case SWHT_CAPTION:
      return HTCAPTION;
    default:
      return HTCLIENT;
    }

  case WM_LBUTTONDOWN:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    StatusWnd_OnLButton(hWnd, pt, TRUE);
    break;

  case WM_LBUTTONUP:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    StatusWnd_OnLButton(hWnd, pt, FALSE);
    break;

  case WM_MOUSEMOVE:
    // This message comes from the captured window.
    ::GetCursorPos(&pt);
    if (wParam & MK_LBUTTON) {
      StatusWnd_OnLButton(hWnd, pt, TRUE);
    } else {
      StatusWnd_OnLButton(hWnd, pt, FALSE);
    }
    break;

  case WM_SETCURSOR:
    // This message comes even from the disabled window.
    DebugPrint(TEXT("status message: WM_SETCURSOR"));
    ::GetCursorPos(&pt);
    switch (HIWORD(lParam)) {
    case WM_MOUSEMOVE:
      if (::GetWindowLong(hWnd, FIGWL_MOUSE)) {
        StatusWnd_OnLButton(hWnd, pt, TRUE);
      }
      break;
    case WM_LBUTTONDOWN:
      StatusWnd_OnLButton(hWnd, pt, TRUE);
      break;
    case WM_LBUTTONUP:
      StatusWnd_OnLButton(hWnd, pt, FALSE);
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
