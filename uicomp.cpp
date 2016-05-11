
// uicomp.cpp --- mzimeja composition window UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

BOOL MyGetTextExtentPoint(HDC hDC, LPCWSTR psz, int cch, LPSIZE psiz) {
  BOOL ret = ::GetTextExtentPoint32W(hDC, psz, cch, psiz);
  return ret;
}

// Count how may the char can be arranged in DX
static int NumCharInDX(HDC hDC, LPWSTR lp, int dx) {
  SIZE sz;
  int width = 0;
  int num   = 0;
  int numT  = 0;

  if (!*lp)
    return 0;

  while ((width < dx) && *(lp + numT)) {
    num = numT;
    numT++;
    MyGetTextExtentPoint(hDC, lp, numT, &sz);
    width = sz.cx;
  }
  if (width < dx)
    num = numT;
  return num;
}

// Count how may the char can be arranged in DY
static int NumCharInDY(HDC hDC, LPWSTR lp, int dy) {
  SIZE sz;
  int width = 0;
  int num;
  int numT = 0;

  if (!*lp)
    return 0;

  while ((width < dy) && *(lp + numT)) {
    num = numT;
    numT++;
    MyGetTextExtentPoint(hDC, lp, numT, &sz);
    width = sz.cy;
  }
  return num;
}

//////////////////////////////////////////////////////////////////////////////

void CompWnd_Show(LPUIEXTRA lpUIExtra, INT nIndex, BOOL bShow) {
  FOOTMARK();
  if (nIndex == -2) {
    CompWnd_Show(lpUIExtra, -1, bShow);
    for (int i = 0; i < MAXCOMPWND; i++) {
      CompWnd_Show(lpUIExtra, i, FALSE);
    }
    return;
  }
  HWND hWnd;
  if (nIndex == -1) {
    hWnd = lpUIExtra->uiDefComp.hWnd;
    lpUIExtra->uiDefComp.bShow = bShow;
  } else {
    hWnd = lpUIExtra->uiComp[nIndex].hWnd;
    lpUIExtra->uiComp[nIndex].bShow = bShow;
  }
  if (::IsWindow(hWnd)) {
    if (bShow) {
      ::ShowWindow(hWnd, SW_SHOWNOACTIVATE);
    } else {
      ::ShowWindow(hWnd, SW_HIDE);
    }
  }
}

void CompWnd_Create(HWND hwndServer, LPUIEXTRA lpUIExtra,
                    InputContext *lpIMC) {
  RECT rc;
  FOOTMARK();
  DebugPrintA("CompWnd_Create: ThreadID: %08X\n", ::GetCurrentThreadId());

  lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;
  for (int i = 0; i < MAXCOMPWND; i++) {
    if (!IsWindow(lpUIExtra->uiComp[i].hWnd)) {
      lpUIExtra->uiComp[i].hWnd =
          ::CreateWindowEx(0, szCompStrClassName, NULL, WS_COMPNODEFAULT,
                           0, 0, 1, 1, hwndServer, NULL, TheIME.m_hInst, NULL);
    }
    lpUIExtra->uiComp[i].rc.left = 0;
    lpUIExtra->uiComp[i].rc.top = 0;
    lpUIExtra->uiComp[i].rc.right = 1;
    lpUIExtra->uiComp[i].rc.bottom = 1;
    SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd, FIGWLP_FONT,
                     (LONG_PTR)lpUIExtra->hFont);
    SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd, FIGWLP_SERVERWND,
                     (LONG_PTR)hwndServer);
    SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPINDEX, i);
    CompWnd_Show(lpUIExtra, i, lpUIExtra->uiComp[i].bShow);
  }

  if (lpUIExtra->uiDefComp.pt.x == -1) {
    GetWindowRect(lpIMC->hWnd, &rc);
    lpUIExtra->uiDefComp.pt.x = rc.left;
    lpUIExtra->uiDefComp.pt.y = rc.bottom + 1;
  }

  if (!IsWindow(lpUIExtra->uiDefComp.hWnd)) {
    lpUIExtra->uiDefComp.hWnd = CreateWindowEx(
        WS_EX_WINDOWEDGE, szCompStrClassName, NULL,
        WS_COMPDEFAULT | WS_DLGFRAME, lpUIExtra->uiDefComp.pt.x,
        lpUIExtra->uiDefComp.pt.y, 1, 1, hwndServer,
        NULL, TheIME.m_hInst, NULL);
  }

  SetWindowLong(lpUIExtra->uiDefComp.hWnd, FIGWL_COMPINDEX, -1);
  SetWindowLongPtr(lpUIExtra->uiDefComp.hWnd, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
  SetWindowLongPtr(lpUIExtra->uiDefComp.hWnd, FIGWLP_SERVERWND, (LONG_PTR)hwndServer);
  CompWnd_Show(lpUIExtra, -1, lpUIExtra->uiDefComp.bShow);
}

void CompWnd_MoveShowDefault(HWND hwnd, LPUIEXTRA lpUIExtra, InputContext *lpIMC) {
  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (lpCompStr && lpCompStr->dwCompStrLen > 0) {
    if (::IsWindow(hwnd)) {
      HDC hDC = ::GetDC(hwnd);
      if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING)) &&
          (lpCompStr->dwCompStrLen > 0))
      {
        LPWSTR lpstr = lpCompStr->GetCompStr();
        SIZE siz;
        siz.cx = siz.cy = 0;
        MyGetTextExtentPoint(hDC, lpstr, lstrlenW(lpstr), &siz);
        assert(siz.cx);
        assert(siz.cy);
        int width = siz.cx;
        int height = siz.cy + 1;

        lpIMC->UnlockCompStr();
        lpCompStr = NULL;

        RECT rc;
        ::GetWindowRect(hwnd, &rc);
        lpUIExtra->uiDefComp.pt.x = rc.left;
        lpUIExtra->uiDefComp.pt.y = rc.top;
        ::MoveWindow(hwnd, rc.left, rc.top,
                     width + 2 * GetSystemMetrics(SM_CXEDGE),
                     height + 2 * GetSystemMetrics(SM_CYEDGE), TRUE);
        CompWnd_Show(lpUIExtra, -1, TRUE);
        ::InvalidateRect(hwnd, NULL, FALSE);
      }
      ::ReleaseDC(hwnd, hDC);
    }
  }
  if (lpCompStr) lpIMC->UnlockCompStr();
}

void CompWnd_MoveShowHorizontal(HWND hwnd, LPUIEXTRA lpUIExtra, InputContext *lpIMC) {
  if (lpIMC->cfCompForm.dwStyle == 0) return;
  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (lpCompStr) {
    if (lpCompStr->dwCompStrLen > 0) {
      RECT rcSrc;
      if (lpIMC->cfCompForm.dwStyle & CFS_RECT)
        rcSrc = lpIMC->cfCompForm.rcArea;
      else
        ::GetClientRect(lpIMC->hWnd, &rcSrc);

      POINT ptSrc = lpIMC->cfCompForm.ptCurrentPos;
      ::ClientToScreen(lpIMC->hWnd, &ptSrc);
      ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.left);
      ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.right);

      int nCompIndex =
        (int)GetWindowLong(lpUIExtra->uiDefComp.hWnd, FIGWL_COMPINDEX);
      if (nCompIndex == -1) {
        CompWnd_Show(lpUIExtra, -1, FALSE);
      }

      if (::PtInRect(&rcSrc, ptSrc)) {
        LPWSTR lpstr = lpCompStr->GetCompStr();;
        LPWSTR pch = lpstr;
        HDC hDC = ::CreateCompatibleDC(NULL);

        HFONT hFont;
        hFont = (HFONT)GetWindowLongPtr(
          lpUIExtra->uiComp[nCompIndex].hWnd, FIGWLP_FONT);
        HGDIOBJ hOldFont = (HFONT)::SelectObject(hDC, hFont);

        int dx = rcSrc.right - ptSrc.x;
        int curx = ptSrc.x, cury = ptSrc.y;
        for (int i = 0; i < MAXCOMPWND; ++i) {
          if (!::IsWindow(lpUIExtra->uiComp[i].hWnd)) continue;
          SIZE siz;
          siz.cx = siz.cy = 0;

          int num = NumCharInDX(hDC, pch, dx);
          if (num) {
            MyGetTextExtentPoint(hDC, pch, num, &siz);
            assert(siz.cx);
            assert(siz.cy);

            if (i == nCompIndex) {
              lpUIExtra->uiComp[i].rc.left = curx;
              lpUIExtra->uiComp[i].rc.top = cury;
              lpUIExtra->uiComp[i].rc.right = siz.cx;
              lpUIExtra->uiComp[i].rc.bottom = siz.cy + 1;
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR,
                            (DWORD)(pch - lpstr));
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, num);
              MoveWindow(lpUIExtra->uiComp[i].hWnd, curx, cury, siz.cx, siz.cy,
                         TRUE);
              CompWnd_Show(lpUIExtra, i, TRUE);
            }
            pch += num;
          } else {
            lpUIExtra->uiComp[i].rc.left = 0;
            lpUIExtra->uiComp[i].rc.top = 0;
            lpUIExtra->uiComp[i].rc.right = 0;
            lpUIExtra->uiComp[i].rc.bottom = 0;
            if (i == nCompIndex) {
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR, 0L);
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, 0L);
              CompWnd_Show(lpUIExtra, i, FALSE);
            }
          }

          if (i == nCompIndex) {
            ::InvalidateRect(lpUIExtra->uiComp[i].hWnd, NULL, FALSE);
          }

          dx = rcSrc.right - rcSrc.left;
          curx = rcSrc.left;
          cury += siz.cy + 1;
        }
        ::SelectObject(hDC, hOldFont);
        ::DeleteDC(hDC);
      }
    }
    lpIMC->UnlockCompStr();
  }
}

void CompWnd_MoveShowVertical(HWND hwnd, LPUIEXTRA lpUIExtra, InputContext *lpIMC) {
  if (lpIMC->cfCompForm.dwStyle == 0) return;
  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (lpCompStr) {
    if (lpCompStr->dwCompStrLen > 0) {
      RECT rcSrc;
      if (lpIMC->cfCompForm.dwStyle & CFS_RECT)
        rcSrc = lpIMC->cfCompForm.rcArea;
      else
        ::GetClientRect(lpIMC->hWnd, &rcSrc);

      POINT ptSrc = lpIMC->cfCompForm.ptCurrentPos;
      ::ClientToScreen(lpIMC->hWnd, &ptSrc);
      ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.left);
      ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.right);

      int nCompIndex =
        (int)GetWindowLong(lpUIExtra->uiDefComp.hWnd, FIGWL_COMPINDEX);
      if (nCompIndex == -1) {
        CompWnd_Show(lpUIExtra, -1, FALSE);
      }

      if (::PtInRect(&rcSrc, ptSrc)) {
        LPWSTR lpstr = lpCompStr->GetCompStr();;
        LPWSTR pch = lpstr;
        HDC hDC = ::CreateCompatibleDC(NULL);

        HFONT hFont;
        hFont = (HFONT)GetWindowLongPtr(
          lpUIExtra->uiComp[nCompIndex].hWnd, FIGWLP_FONT);
        HGDIOBJ hOldFont = (HFONT)SelectObject(hDC, hFont);

        int nCompIndex =
          (int)GetWindowLong(lpUIExtra->uiDefComp.hWnd, FIGWL_COMPINDEX);

        int dy = rcSrc.bottom - ptSrc.y;
        int curx = ptSrc.x, cury = ptSrc.y;
        for (int i = 0; i < MAXCOMPWND; i++) {
          if (!::IsWindow(lpUIExtra->uiComp[i].hWnd)) continue;

          SIZE siz;
          siz.cx = siz.cy = 0;
          int num = NumCharInDY(hDC, pch, dy);
          if (num) {
            MyGetTextExtentPoint(hDC, pch, num, &siz);
            assert(siz.cx);
            assert(siz.cy);

            if (i == nCompIndex) {
              lpUIExtra->uiComp[i].rc.left = curx - siz.cy;
              lpUIExtra->uiComp[i].rc.top = cury;
              lpUIExtra->uiComp[i].rc.right = siz.cy + 1;
              lpUIExtra->uiComp[i].rc.bottom = siz.cx;
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR,
                            (DWORD)(pch - lpstr));
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, num);
              MoveWindow(lpUIExtra->uiComp[i].hWnd, curx, cury, siz.cy, siz.cx,
                         TRUE);
              CompWnd_Show(lpUIExtra, i, TRUE);
            }
            pch += num;
          } else {
            if (i == nCompIndex) {
              lpUIExtra->uiComp[i].rc.left = 0;
              lpUIExtra->uiComp[i].rc.top = 0;
              lpUIExtra->uiComp[i].rc.right = 0;
              lpUIExtra->uiComp[i].rc.bottom = 0;
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR, 0L);
              SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, 0L);
              CompWnd_Show(lpUIExtra, i, FALSE);
            }
          }

          if (i == nCompIndex) {
            ::InvalidateRect(lpUIExtra->uiComp[i].hWnd, NULL, FALSE);
          }

          dy = rcSrc.bottom - rcSrc.top;
          cury = rcSrc.top;
          curx -= siz.cy + 1;
        }
        ::SelectObject(hDC, hOldFont);
        ::DeleteDC(hDC);
      }
    }
    lpIMC->UnlockCompStr();
  }
}

void CompWnd_MoveShow(HWND hwnd) {
  HWND hwndServer = (HWND)GetWindowLongPtr(hwnd, FIGWLP_SERVERWND);
  LPUIEXTRA lpUIExtra = LockUIExtra(hwndServer);
  assert(lpUIExtra);
  if (lpUIExtra) {
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;
      int nCompIndex = (int)GetWindowLong(lpUIExtra->uiDefComp.hWnd, FIGWL_COMPINDEX);
      if (nCompIndex == -1) {
        CompWnd_MoveShowDefault(hwnd, lpUIExtra, lpIMC);
      } else {
        if (!lpUIExtra->bVertical) {
          CompWnd_MoveShowHorizontal(hwnd, lpUIExtra, lpIMC);
        } else {
          CompWnd_MoveShowVertical(hwnd, lpUIExtra, lpIMC);
        }
      }
      TheIME.UnlockIMC(hIMC);
    }
    UnlockUIExtra(hwndServer);
  }
}

void DrawTextOneLine(HWND hCompWnd, HDC hDC, LPTSTR lpstr,
                     LPBYTE lpattr, int num, BOOL fVert, DWORD dwCursor) {
  FOOTMARK();
  //LPTSTR lpStart = lpstr;
  LPTSTR lpEnd = lpstr + num - 1;
  int x, y;
  RECT rc;

  if (num == 0) return;

  GetClientRect(hCompWnd, &rc);

  if (!fVert) {
    x = 0;
    y = 0;
  } else {
    x = rc.right;
    y = 0;
  }

  DWORD ich = 0;
  while (lpstr <= lpEnd) {
    BYTE bAttr = *lpattr;
    SIZE siz;

    BOOL bConverted = FALSE;
    switch (bAttr) {
    case ATTR_TARGET_CONVERTED:
    case ATTR_CONVERTED:
      bConverted = TRUE;
      break;

    default:
      break;
    }

    HPEN hPen;
    if (bConverted) {
      SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
      SetBkMode(hDC, OPAQUE);
      SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
      hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
    } else {
      SetTextColor(hDC, RGB(0, 127, 0));
      SetBkMode(hDC, OPAQUE);
      SetBkColor(hDC, RGB(255, 255, 255));
      hPen = CreatePen(PS_DOT, 1, RGB(0, 127, 0));
    }

    TextOut(hDC, x, y, lpstr, 1);
    MyGetTextExtentPoint(hDC, lpstr, 1, &siz);

    HGDIOBJ hPenOld = SelectObject(hDC, hPen);
    if (fVert) {
      MoveToEx(hDC, x, y, NULL);
      LineTo(hDC, x, y + siz.cx - 1);
    } else {
      MoveToEx(hDC, x, y + siz.cy - 1, NULL);
      LineTo(hDC, x + siz.cx, y + siz.cy - 1);
    }
    SelectObject(hDC, hPenOld);
    DeleteObject(hPen);

    SelectObject(hDC, GetStockObject(BLACK_PEN));
    if (dwCursor == ich) {
      if (fVert) {
        MoveToEx(hDC, x, y, NULL);
        LineTo(hDC, x + siz.cy, y);
      } else {
        MoveToEx(hDC, x, y, NULL);
        LineTo(hDC, x, y + siz.cy);
      }
    }

    ++lpstr;
    ++ich;
    ++lpattr;

    if (!fVert)
      x += siz.cx;
    else
      y += siz.cx;
  }
}

void CompWnd_Paint(HWND hCompWnd) {
  FOOTMARK();

  HIMC hIMC;
  HDC hDC;
  RECT rc;
  HFONT hOldFont = NULL;

  PAINTSTRUCT ps;
  hDC = BeginPaint(hCompWnd, &ps);

  HFONT hFont = (HFONT)GetWindowLongPtr(hCompWnd, FIGWLP_FONT);
  if (hFont)
    hOldFont = (HFONT)SelectObject(hDC, hFont);

  HWND hwndServer = (HWND)GetWindowLongPtr(hCompWnd, FIGWLP_SERVERWND);

  hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
  if (hIMC) {
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    CompStr *lpCompStr = lpIMC->LockCompStr();
    if (lpCompStr) {
      if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING)) &&
          (lpCompStr->dwCompStrLen > 0)) {
        LPTSTR lpstr;
        LPBYTE lpattr;
        LONG lstart;
        LONG num;
        BOOL fVert = FALSE;
        DWORD dwCursor = lpCompStr->dwCursorPos;

        if (hFont) fVert = (lpIMC->lfFont.A.lfEscapement == 2700);

        lpstr = lpCompStr->GetCompStr();
        lpattr = lpCompStr->GetCompAttr();
        if (lpIMC->cfCompForm.dwStyle) {
          GetClientRect(hCompWnd, &rc);
          SetBkMode(hDC, OPAQUE);

          lstart = GetWindowLong(hCompWnd, FIGWL_COMPSTARTSTR);
          num = GetWindowLong(hCompWnd, FIGWL_COMPSTARTNUM);

          if (!num || ((lstart + num) > lstrlen(lpstr))) goto end_pcw;

          lpstr += lstart;
          lpattr += lstart;
          dwCursor -= lstart;
          DrawTextOneLine(hCompWnd, hDC, lpstr, lpattr, num, fVert, dwCursor);
        } else {
          num = lstrlen(lpstr);
          DrawTextOneLine(hCompWnd, hDC, lpstr, lpattr, num, fVert, dwCursor);
        }
      }
    end_pcw:
      lpIMC->UnlockCompStr();
    }
    TheIME.UnlockIMC(hIMC);
  }
  if (hFont && hOldFont) SelectObject(hDC, hOldFont);
  EndPaint(hCompWnd, &ps);
}

void CompWnd_SetFont(LPUIEXTRA lpUIExtra) {
  FOOTMARK();
  for (int i = 0; i < MAXCOMPWND; i++) {
    if (IsWindow(lpUIExtra->uiComp[i].hWnd))
      SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd, FIGWLP_FONT,
                       (LONG_PTR)lpUIExtra->hFont);
  }
  SetWindowLongPtr(lpUIExtra->uiDefComp.hWnd, FIGWLP_FONT,
                   (LONG_PTR)lpUIExtra->hFont);
}

LRESULT CALLBACK CompWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                    LPARAM lParam) {
  FOOTMARK();
  HWND hwndServer;

  switch (message) {
  case WM_PAINT:
    CompWnd_Paint(hWnd);
    break;

  case WM_SETCURSOR:
  case WM_MOUSEMOVE:
  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
    DragUI(hWnd, message, wParam, lParam);
    if ((message == WM_SETCURSOR) && (HIWORD(lParam) != WM_LBUTTONDOWN) &&
        (HIWORD(lParam) != WM_RBUTTONDOWN))
      return DefWindowProc(hWnd, message, wParam, lParam);
    if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP))
      SetWindowLong(hWnd, FIGWL_MOUSE, 0L);
    break;

  case WM_MOVE:
    hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    if (IsWindow(hwndServer))
      SendMessage(hwndServer, WM_UI_DEFCOMPMOVE, wParam, lParam);
    break;

  case WM_UI_COMPMOVESHOW:
    CompWnd_MoveShow(hWnd);
    break;

  default:
    if (!IsImeMessage(message))
      return DefWindowProc(hWnd, message, wParam, lParam);
    break;
  }
  return 0;
}

void CompWnd_MoveShowMessage(HWND hwndServer, LPUIEXTRA lpUIExtra) {
  ::PostMessage(lpUIExtra->uiDefComp.hWnd, WM_UI_COMPMOVESHOW, 0, 0);
  for (int i = 0; i < MAXCOMPWND; i++) {
    ::PostMessage(lpUIExtra->uiComp[i].hWnd, WM_UI_COMPMOVESHOW, 0, 0);
  }
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
