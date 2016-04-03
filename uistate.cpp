// uistate.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK StatusWndProc(HWND hWnd, UINT message, WPARAM wParam,
                               LPARAM lParam) {
  PAINTSTRUCT ps;
  HWND hUIWnd;
  HDC hDC;
  HBITMAP hbmpStatus;

  switch (message) {
    case WM_UI_UPDATE:
      InvalidateRect(hWnd, NULL, FALSE);
      break;

    case WM_PAINT:
      hDC = BeginPaint(hWnd, &ps);
      PaintStatus(hWnd, hDC, NULL, 0);
      EndPaint(hWnd, &ps);
      break;

    case WM_MOUSEMOVE:
    case WM_SETCURSOR:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      ButtonStatus(hWnd, message, wParam, lParam);
      if ((message == WM_SETCURSOR) && (HIWORD(lParam) != WM_LBUTTONDOWN) &&
          (HIWORD(lParam) != WM_RBUTTONDOWN))
        return DefWindowProc(hWnd, message, wParam, lParam);
      if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP)) {
        SetWindowLong(hWnd, FIGWL_MOUSE, 0L);
        SetWindowLong(hWnd, FIGWL_PUSHSTATUS, 0L);
      }
      break;

    case WM_MOVE:
      hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWL_SVRWND);
      if (IsWindow(hUIWnd))
        SendMessage(hUIWnd, WM_UI_STATEMOVE, wParam, lParam);
      break;

    case WM_CREATE:
      hbmpStatus = LoadBitmap(hInst, TEXT("STATUSBMP"));
      SetWindowLongPtr(hWnd, FIGWL_STATUSBMP, (LONG_PTR)hbmpStatus);
      hbmpStatus = LoadBitmap(hInst, TEXT("CLOSEBMP"));
      SetWindowLongPtr(hWnd, FIGWL_CLOSEBMP, (LONG_PTR)hbmpStatus);
      break;

    case WM_DESTROY:
      hbmpStatus = (HBITMAP)GetWindowLongPtr(hWnd, FIGWL_STATUSBMP);
      DeleteObject(hbmpStatus);
      hbmpStatus = (HBITMAP)GetWindowLongPtr(hWnd, FIGWL_CLOSEBMP);
      DeleteObject(hbmpStatus);
      break;

    default:
      if (!MyIsIMEMessage(message))
        return DefWindowProc(hWnd, message, wParam, lParam);
      break;
  }
  return 0;
}

DWORD PASCAL CheckPushedStatus(HWND hStatusWnd, LPPOINT lppt) {
  POINT pt;
  RECT rc;

  if (lppt) {
    pt = *lppt;
    ScreenToClient(hStatusWnd, &pt);
    GetClientRect(hStatusWnd, &rc);
    if (!PtInRect(&rc, pt)) return 0;

    if (pt.y > GetSystemMetrics(SM_CYSMCAPTION)) {
      if (pt.x < BTX)
        return PUSHED_STATUS_HDR;
      else if (pt.x < (BTX * 2))
        return PUSHED_STATUS_MODE;
      else if (pt.x < (BTX * 3))
        return PUSHED_STATUS_ROMAN;
    } else {
      rc.left = STCLBT_X;
      rc.top = STCLBT_Y;
      rc.right = STCLBT_X + STCLBT_DX;
      rc.bottom = STCLBT_Y + STCLBT_DY;
      if (PtInRect(&rc, pt)) return PUSHED_STATUS_CLOSE;
    }
  }
  return 0;
}

int PASCAL BTXFromCmode(DWORD dwCmode) {
  if (dwCmode & IME_CMODE_FULLSHAPE) {
    if (!(dwCmode & IME_CMODE_LANGUAGE))
      return BTFALPH;
    else if ((dwCmode & IME_CMODE_LANGUAGE) == IME_CMODE_NATIVE)
      return BTFHIRA;
    else
      return BTFKATA;
  } else {
    if ((dwCmode & IME_CMODE_LANGUAGE) == IME_CMODE_ALPHANUMERIC)
      return BTHALPH;
    else
      return BTHKATA;
  }
}

void PASCAL PaintStatus(HWND hStatusWnd, HDC hDC, LPPOINT lppt,
                        DWORD dwPushedStatus) {
  HIMC hIMC;
  HDC hMemDC;
  HBITMAP hbmpOld;
  int x;
  HWND hSvrWnd;

  hSvrWnd = (HWND)GetWindowLongPtr(hStatusWnd, FIGWL_SVRWND);

  hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC);
  if (hIMC) {
    HBITMAP hbmpStatus;
    HBRUSH hOldBrush, hBrush;
    int nCyCap = GetSystemMetrics(SM_CYSMCAPTION);
    RECT rc;

    InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
    hMemDC = CreateCompatibleDC(hDC);

    // Paint Caption.
    hBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
    hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
    rc.top = rc.left = 0;
    rc.right = BTX * 3;
    rc.bottom = nCyCap;
    FillRect(hDC, &rc, hBrush);
    SelectObject(hDC, hOldBrush);
    DeleteObject(hBrush);

    // Paint CloseButton.
    hbmpStatus = (HBITMAP)GetWindowLongPtr(hStatusWnd, FIGWL_CLOSEBMP);
    hbmpOld = (HBITMAP)SelectObject(hMemDC, hbmpStatus);

    if (!(dwPushedStatus & PUSHED_STATUS_CLOSE))
      BitBlt(hDC, STCLBT_X, STCLBT_Y, STCLBT_DX, STCLBT_DY, hMemDC, 0, 0,
             SRCCOPY);
    else
      BitBlt(hDC, STCLBT_X, STCLBT_Y, STCLBT_DX, STCLBT_DY, hMemDC, STCLBT_DX,
             0, SRCCOPY);

    hbmpStatus = (HBITMAP)GetWindowLongPtr(hStatusWnd, FIGWL_STATUSBMP);
    SelectObject(hMemDC, hbmpStatus);

    // Paint HDR.
    x = BTEMPT;
    if (lpIMC->fOpen) x = 0;

    if (!(dwPushedStatus & PUSHED_STATUS_HDR))
      BitBlt(hDC, 0, nCyCap, BTX, BTY, hMemDC, x, 0, SRCCOPY);
    else
      BitBlt(hDC, 0, nCyCap, BTX, BTY, hMemDC, x, BTY, SRCCOPY);

    // Paint MODE.
    x = BTXFromCmode(lpIMC->fdwConversion);

    if (!(dwPushedStatus & PUSHED_STATUS_MODE))
      BitBlt(hDC, BTX, nCyCap, BTX, BTY, hMemDC, x, 0, SRCCOPY);
    else
      BitBlt(hDC, BTX, nCyCap, BTX, BTY, hMemDC, x, BTY, SRCCOPY);

    // Paint Roman MODE.
    x = BTEMPT;
    if (lpIMC->fdwConversion & IME_CMODE_ROMAN) x = BTROMA;

    if (!(dwPushedStatus & PUSHED_STATUS_ROMAN))
      BitBlt(hDC, BTX * 2, nCyCap, BTX, BTY, hMemDC, x, 0, SRCCOPY);
    else
      BitBlt(hDC, BTX * 2, nCyCap, BTX, BTY, hMemDC, x, BTY, SRCCOPY);

    SelectObject(hMemDC, hbmpOld);
    DeleteDC(hMemDC);
    ImmUnlockIMC(hIMC);
  }
}

DWORD PASCAL GetUINextMode(DWORD fdwConversion, DWORD dwPushed) {
  DWORD dwTemp;
  BOOL fFullShape = ((fdwConversion & IME_CMODE_FULLSHAPE) != 0);

  //
  // When the mode button is pushed, the convmode will be chage as follow
  // rotation.
  //
  //     FULLSHAPE,HIRAGANA     ->
  //     FULLSHAPE,KATAKANA     ->
  //     FULLSHAPE,ALPHANUMERIC ->
  //     HALFSHAPE,KATAKANA     ->
  //     HALFSHAPE,ALPHANUMERIC ->
  //     FULLSHAPE,HIRAGANA
  //
  if (dwPushed == PUSHED_STATUS_MODE) {
    dwTemp = fdwConversion & IME_CMODE_LANGUAGE;

    if ((fFullShape) && (dwTemp == IME_CMODE_NATIVE))
      return (fdwConversion & ~IME_CMODE_LANGUAGE) | IME_CMODE_KATAKANA |
             IME_CMODE_NATIVE;

    if ((fFullShape) && (dwTemp == (IME_CMODE_KATAKANA | IME_CMODE_NATIVE)))
      return (fdwConversion & ~IME_CMODE_LANGUAGE);

    if ((fFullShape) && (dwTemp == 0)) {
      fdwConversion &= ~IME_CMODE_FULLSHAPE;
      return (fdwConversion & ~IME_CMODE_LANGUAGE) | IME_CMODE_KATAKANA |
             IME_CMODE_NATIVE;
    }

    if ((!fFullShape) && (dwTemp == (IME_CMODE_KATAKANA | IME_CMODE_NATIVE)))
      return (fdwConversion & ~IME_CMODE_LANGUAGE);

    if ((!fFullShape) && (!dwTemp)) {
      fdwConversion |= IME_CMODE_FULLSHAPE;
      return (fdwConversion & ~IME_CMODE_LANGUAGE) | IME_CMODE_NATIVE;
    }
  }
  if (dwPushed == PUSHED_STATUS_ROMAN) {
    if (fdwConversion & IME_CMODE_ROMAN)
      return fdwConversion & ~IME_CMODE_ROMAN;
    else
      return fdwConversion | IME_CMODE_ROMAN;
  }
  return fdwConversion;
}

void PASCAL ButtonStatus(HWND hStatusWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  POINT pt;
  HDC hDC;
  DWORD dwMouse;
  DWORD dwPushedStatus;
  DWORD dwTemp;
  DWORD fdwConversion;
  HIMC hIMC;
  HWND hSvrWnd;
  BOOL fOpen;
  HMENU hMenu;
  static POINT ptdif;
  static RECT drc;
  static RECT rc;
  static DWORD dwCurrentPushedStatus;

  hDC = GetDC(hStatusWnd);
  switch (message) {
    case WM_SETCURSOR:
      if (HIWORD(lParam) == WM_LBUTTONDOWN ||
          HIWORD(lParam) == WM_RBUTTONDOWN) {
        GetCursorPos(&pt);
        SetCapture(hStatusWnd);
        GetWindowRect(hStatusWnd, &drc);
        ptdif.x = pt.x - drc.left;
        ptdif.y = pt.y - drc.top;
        rc = drc;
        rc.right -= rc.left;
        rc.bottom -= rc.top;
        SetWindowLong(hStatusWnd, FIGWL_MOUSE, FIM_CAPUTURED);
        SetWindowLong(hStatusWnd, FIGWL_PUSHSTATUS,
                      dwPushedStatus = CheckPushedStatus(hStatusWnd, &pt));
        PaintStatus(hStatusWnd, hDC, &pt, dwPushedStatus);
        dwCurrentPushedStatus = dwPushedStatus;
      }
      break;

    case WM_MOUSEMOVE:
      dwMouse = GetWindowLong(hStatusWnd, FIGWL_MOUSE);
      if (!(dwPushedStatus = GetWindowLong(hStatusWnd, FIGWL_PUSHSTATUS))) {
        if (dwMouse & FIM_MOVED) {
          DrawUIBorder(&drc);
          GetCursorPos(&pt);
          drc.left = pt.x - ptdif.x;
          drc.top = pt.y - ptdif.y;
          drc.right = drc.left + rc.right;
          drc.bottom = drc.top + rc.bottom;
          DrawUIBorder(&drc);
        } else if (dwMouse & FIM_CAPUTURED) {
          DrawUIBorder(&drc);
          SetWindowLong(hStatusWnd, FIGWL_MOUSE, dwMouse | FIM_MOVED);
        }
      } else {
        GetCursorPos(&pt);
        dwTemp = CheckPushedStatus(hStatusWnd, &pt);
        if ((dwTemp ^ dwCurrentPushedStatus) & dwPushedStatus)
          PaintStatus(hStatusWnd, hDC, &pt, dwPushedStatus & dwTemp);
        dwCurrentPushedStatus = dwTemp;
      }
      break;

    case WM_RBUTTONUP:
      dwMouse = GetWindowLong(hStatusWnd, FIGWL_MOUSE);
      if (dwMouse & FIM_CAPUTURED) {
        ReleaseCapture();
        if (dwMouse & FIM_MOVED) {
          DrawUIBorder(&drc);
          GetCursorPos(&pt);
          MoveWindow(hStatusWnd, pt.x - ptdif.x, pt.y - ptdif.y, rc.right,
                     rc.bottom, TRUE);
        }
      }
      PaintStatus(hStatusWnd, hDC, NULL, 0);

      hSvrWnd = (HWND)GetWindowLongPtr(hStatusWnd, FIGWL_SVRWND);

      hMenu = LoadMenu(hInst, TEXT("RIGHTCLKMENU"));
      if (hMenu && (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC))) {
        int cmd;
        HMENU hSubMenu = GetSubMenu(hMenu, 0);

        pt.x = (int)LOWORD(lParam), pt.y = (int)HIWORD(lParam),

        ClientToScreen(hStatusWnd, &pt);

        cmd = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hStatusWnd,
                             NULL);
        switch (cmd) {
          case IDM_RECONVERT: {
            DWORD dwSize =
                (DWORD)ImmRequestMessage(hIMC, IMR_RECONVERTSTRING, 0);
            if (dwSize) {
              LPRECONVERTSTRING lpRS;

              lpRS = (LPRECONVERTSTRING)GlobalAlloc(GPTR, dwSize);
              lpRS->dwSize = dwSize;

              dwSize = (DWORD)ImmRequestMessage(hIMC, IMR_RECONVERTSTRING,
                                                (LPARAM)lpRS);
              if (dwSize) {
                LPTSTR lpDump = (LPTSTR)((LPBYTE)lpRS + lpRS->dwStrOffset);
                *(LPTSTR)(lpDump + lpRS->dwStrLen) = 0;

                DebugPrint(TEXT("IMR_RECONVERTSTRING\r\n"));
                DebugPrint(TEXT("dwSize            %x"), lpRS->dwSize);
                DebugPrint(TEXT("dwVersion         %x"), lpRS->dwVersion);
                DebugPrint(TEXT("dwStrLen          %x"), lpRS->dwStrLen);
                DebugPrint(TEXT("dwStrOffset       %x"), lpRS->dwStrOffset);
                DebugPrint(TEXT("dwCompStrLen      %x"), lpRS->dwCompStrLen);
                DebugPrint(TEXT("dwCompStrOffset   %x"), lpRS->dwCompStrOffset);
                DebugPrint(TEXT("dwTargetStrLen    %x"), lpRS->dwTargetStrLen);
                DebugPrint(TEXT("dwTargetStrOffset %x"), lpRS->dwTargetStrOffset);
                DebugPrint(TEXT("%s"), lpDump);

                ImmRequestMessage(hIMC, IMR_CONFIRMRECONVERTSTRING,
                                  (LPARAM)lpRS);
              }
              else
                DebugPrint(TEXT("ImmRequestMessage returned 0\r\n"));
              GlobalFree(lpRS);
            }
            break;
          }

          case IDM_ABOUT:
            ImmConfigureIME(GetKeyboardLayout(0), NULL, IME_CONFIG_GENERAL, 0);
            break;

          default:
            break;
        }
      }
      if (hMenu) DestroyMenu(hMenu);

      break;

    case WM_LBUTTONUP:
      dwMouse = GetWindowLong(hStatusWnd, FIGWL_MOUSE);
      if (dwMouse & FIM_CAPUTURED) {
        ReleaseCapture();
        if (dwMouse & FIM_MOVED) {
          DrawUIBorder(&drc);
          GetCursorPos(&pt);
          MoveWindow(hStatusWnd, pt.x - ptdif.x, pt.y - ptdif.y, rc.right,
                     rc.bottom, TRUE);
        }
      }
      hSvrWnd = (HWND)GetWindowLongPtr(hStatusWnd, FIGWL_SVRWND);

      hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC);
      if (hIMC) {
        GetCursorPos(&pt);
        dwPushedStatus = GetWindowLong(hStatusWnd, FIGWL_PUSHSTATUS);
        dwPushedStatus &= CheckPushedStatus(hStatusWnd, &pt);
        if (!dwPushedStatus) {
        } else if (dwPushedStatus == PUSHED_STATUS_CLOSE) {
        } else if (dwPushedStatus == PUSHED_STATUS_HDR) {
          fOpen = ImmGetOpenStatus(hIMC);
          fOpen = !fOpen;
          ImmSetOpenStatus(hIMC, fOpen);
        } else {
          ImmGetConversionStatus(hIMC, &fdwConversion, &dwTemp);
          fdwConversion = GetUINextMode(fdwConversion, dwPushedStatus);
          ImmSetConversionStatus(hIMC, fdwConversion, dwTemp);
        }
      }
      PaintStatus(hStatusWnd, hDC, NULL, 0);
      break;
  }
  ReleaseDC(hStatusWnd, hDC);
}

void PASCAL UpdateStatusWindow(LPUIEXTRA lpUIExtra) {
  if (IsWindow(lpUIExtra->uiStatus.hWnd))
    SendMessage(lpUIExtra->uiStatus.hWnd, WM_UI_UPDATE, 0, 0L);
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
