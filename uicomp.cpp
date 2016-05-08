
// uicomp.cpp --- mzimeja composition window UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

struct CriticalSection {
  CRITICAL_SECTION m_cs;
  CriticalSection() {
    ::InitializeCriticalSection(&m_cs);
  }
  void Enter() {
    ::EnterCriticalSection(&m_cs);
  }
  void Leave() {
    ::LeaveCriticalSection(&m_cs);
  }
  ~CriticalSection() {
    ::DeleteCriticalSection(&m_cs);
  }
};

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

BOOL MyGetTextExtentPoint(HDC hDC, LPCWSTR psz, int cch, LPSIZE psiz) {
  static CriticalSection lock;
  lock.Enter();
  BOOL ret = ::GetTextExtentPoint32W(hDC, psz, cch, psiz);
  lock.Leave();
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
  if (!bShow) {
    FOOTMARK_PRINT_CALL_STACK();
  }
  if (nIndex == -1) {
    if (bShow) {
      ::ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_SHOWNOACTIVATE);
      lpUIExtra->uiDefComp.bShow = TRUE;
    } else {
      ::ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_HIDE);
      lpUIExtra->uiDefComp.bShow = FALSE;
    }
  } else {
    if (bShow) {
      ::ShowWindow(lpUIExtra->uiComp[nIndex].hWnd, SW_SHOWNOACTIVATE);
      lpUIExtra->uiComp[nIndex].bShow = TRUE;
    } else {
      ::ShowWindow(lpUIExtra->uiComp[nIndex].hWnd, SW_HIDE);
      lpUIExtra->uiComp[nIndex].bShow = FALSE;
    }
  }
}

void CompWnd_Create(HWND hUIWnd, LPUIEXTRA lpUIExtra,
                    InputContext *lpIMC) {
  RECT rc;
  FOOTMARK();

  lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;
  for (int i = 0; i < MAXCOMPWND; i++) {
    if (!IsWindow(lpUIExtra->uiComp[i].hWnd)) {
      lpUIExtra->uiComp[i].hWnd =
          CreateWindowEx(0, szCompStrClassName, NULL, WS_COMPNODEFAULT,
                         0, 0, 1, 1, hUIWnd, NULL, TheIME.m_hInst, NULL);
    }
    lpUIExtra->uiComp[i].rc.left = 0;
    lpUIExtra->uiComp[i].rc.top = 0;
    lpUIExtra->uiComp[i].rc.right = 1;
    lpUIExtra->uiComp[i].rc.bottom = 1;
    SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd, FIGWLP_FONT,
                     (LONG_PTR)lpUIExtra->hFont);
    SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
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
        lpUIExtra->uiDefComp.pt.y, 1, 1, hUIWnd, NULL, TheIME.m_hInst, NULL);
  }

  // SetWindowLong(lpUIExtra->uiDefComp.hWnd,FIGWLP_FONT,(DWORD)lpUIExtra->hFont);
  SetWindowLongPtr(lpUIExtra->uiDefComp.hWnd, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
  CompWnd_Show(lpUIExtra, -1, lpUIExtra->uiDefComp.bShow);

  return;
}

// Calc the position of composition windows and move them
void CompWnd_Move(LPUIEXTRA lpUIExtra, InputContext *lpIMC) {
  FOOTMARK();

  HDC hDC;
  HFONT hFont = NULL;
  HFONT hOldFont = NULL;
  CompStr *lpCompStr;
  LPTSTR lpstr;
  RECT rc;
  RECT oldrc;
  SIZE siz;
  int width = 0;
  int height = 0;

  // Save the composition form style into lpUIExtra.
  lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;

  if (lpIMC->cfCompForm.dwStyle) {  // Style is not CFS_DEFAULT.
    LPTSTR pch;
    int num;

    // Lock the COMPOSITIONSTRING structure.
    lpCompStr = lpIMC->LockCompStr();
    if (lpCompStr == NULL) {
      return;
    }
    if (lpCompStr->dwCompStrLen == 0) {
      lpIMC->UnlockCompStr();
      return;
    }

    // Set the rectangle for the composition string.
    RECT rcSrc;
    if (lpIMC->cfCompForm.dwStyle & CFS_RECT)
      rcSrc = lpIMC->cfCompForm.rcArea;
    else
      ::GetClientRect(lpIMC->hWnd, &rcSrc);

    POINT ptSrc = lpIMC->cfCompForm.ptCurrentPos;
    ::ClientToScreen(lpIMC->hWnd, &ptSrc);
    ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.left);
    ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.right);

    // Check the start position.
    if (!::PtInRect(&rcSrc, ptSrc)) {
      lpIMC->UnlockCompStr();
      return;
    }

    // Hide the default composition window.
    if (::IsWindow(lpUIExtra->uiDefComp.hWnd)) {
      CompWnd_Show(lpUIExtra, -1, FALSE);
    }

    pch = lpstr = lpCompStr->GetCompStr();
    num = 1;

    hDC = ::CreateCompatibleDC(NULL);
    if (!lpUIExtra->bVertical) {
      int dx = rcSrc.right - ptSrc.x;
      int curx = ptSrc.x, cury = ptSrc.y;

      // Set the composition string to each composition window.
      // The composition windows that are given the compostion string
      // will be moved and shown.
      for (int i = 0; i < MAXCOMPWND; i++) {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
          hFont = (HFONT)GetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,
                                          FIGWLP_FONT);
          if (hFont)
            hOldFont = (HFONT)SelectObject(hDC, hFont);

          siz.cx = siz.cy = 0;
          oldrc = lpUIExtra->uiComp[i].rc;

          num = NumCharInDX(hDC, pch, dx);
          if (num) {
            DebugPrintA("ThreadID: %08X\n", ::GetCurrentThreadId());
            DebugPrintA("!lpUIExtra->bVertical: '%lc', %d; %d, %d, %d, %d\n", *pch, num, curx, cury, siz.cx, siz.cy);
            MyGetTextExtentPoint(hDC, pch, num, &siz);
            assert(siz.cx);
            assert(siz.cy);

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

            pch += num;
          } else {
            lpUIExtra->uiComp[i].rc.left = 0;
            lpUIExtra->uiComp[i].rc.top = 0;
            lpUIExtra->uiComp[i].rc.right = 0;
            lpUIExtra->uiComp[i].rc.bottom = 0;
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR, 0L);
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, 0L);
            CompWnd_Show(lpUIExtra, i, FALSE);
          }

          InvalidateRect(lpUIExtra->uiComp[i].hWnd, NULL, FALSE);

          dx = rcSrc.right - rcSrc.left;
          curx = rcSrc.left;
          cury += siz.cy + 1;

          if (hOldFont) SelectObject(hDC, hOldFont);
        }
      }
    } else {
      // when it is vertical fonts.
      int dy = rcSrc.bottom - ptSrc.y;
      int curx = ptSrc.x, cury = ptSrc.y;

      for (int i = 0; i < MAXCOMPWND; i++) {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
          hFont = (HFONT)GetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,
                                          FIGWLP_FONT);
          if (hFont)
            hOldFont = (HFONT)SelectObject(hDC, hFont);

          siz.cx = siz.cy = 0;
          num = NumCharInDY(hDC, pch, dy);
          if (num) {
            DebugPrintA("ThreadID: %08X\n", ::GetCurrentThreadId());
            DebugPrintA("lpUIExtra->bVertical: %d, %d, %d, %d\n", curx, cury, siz.cx, siz.cy);
            MyGetTextExtentPoint(hDC, pch, num, &siz);
            assert(siz.cx);
            assert(siz.cy);

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
            pch += num;
          } else {
            lpUIExtra->uiComp[i].rc.left = 0;
            lpUIExtra->uiComp[i].rc.top = 0;
            lpUIExtra->uiComp[i].rc.right = 0;
            lpUIExtra->uiComp[i].rc.bottom = 0;
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR, 0L);
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, 0L);
            CompWnd_Show(lpUIExtra, i, FALSE);
          }

          InvalidateRect(lpUIExtra->uiComp[i].hWnd, NULL, FALSE);

          dy = rcSrc.bottom - rcSrc.top;
          cury = rcSrc.top;
          curx -= siz.cy + 1;

          if (hOldFont) SelectObject(hDC, hOldFont);
        }
      }
    }
    ::DeleteDC(hDC);

    lpIMC->UnlockCompStr();
  } else {
    // When the style is DEFAULT, show the default composition window.
    hDC = ::CreateCompatibleDC(NULL);
    if (IsWindow(lpUIExtra->uiDefComp.hWnd)) {
      for (int i = 0; i < MAXCOMPWND; i++) {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
          CompWnd_Show(lpUIExtra, i, FALSE);
        }
      }

      lpCompStr = lpIMC->LockCompStr();
      if (lpCompStr) {
        if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING)) &&
            (lpCompStr->dwCompStrLen > 0)) {
          lpstr = lpCompStr->GetCompStr();
          MyGetTextExtentPoint(hDC, lpstr, lstrlenW(lpstr), &siz);
          assert(siz.cx);
          assert(siz.cy);
          width = siz.cx;
          height = siz.cy + 1;
        }
        lpIMC->UnlockCompStr();
      }

      GetWindowRect(lpUIExtra->uiDefComp.hWnd, &rc);
      lpUIExtra->uiDefComp.pt.x = rc.left;
      lpUIExtra->uiDefComp.pt.y = rc.top;
      MoveWindow(lpUIExtra->uiDefComp.hWnd, rc.left, rc.top,
                 width + 2 * GetSystemMetrics(SM_CXEDGE),
                 height + 2 * GetSystemMetrics(SM_CYEDGE), TRUE);
      DebugPrintA("ThreadID: %08X\n", ::GetCurrentThreadId());
      DebugPrintA("default: %d, %d, %d, %d\n", rc.left, rc.top, width, height);

      CompWnd_Show(lpUIExtra, -1, TRUE);
      InvalidateRect(lpUIExtra->uiDefComp.hWnd, NULL, FALSE);
    }
    ::DeleteDC(hDC);
  }
} // CompWnd_Move

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

  HWND hSvrWnd = (HWND)GetWindowLongPtr(hCompWnd, FIGWLP_SERVERWND);

  hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC);
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

void CompWnd_Hide(LPUIEXTRA lpUIExtra) {
  FOOTMARK();

  RECT rc;
  if (IsWindow(lpUIExtra->uiDefComp.hWnd)) {
    if (!lpUIExtra->dwCompStyle)
      GetWindowRect(lpUIExtra->uiDefComp.hWnd, &rc);

    CompWnd_Show(lpUIExtra, -1, FALSE);
  }

  for (int i = 0; i < MAXCOMPWND; i++) {
    if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
      CompWnd_Show(lpUIExtra, i, FALSE);
    }
  }
}

void CompWnd_SetFont(LPUIEXTRA lpUIExtra) {
  FOOTMARK();
  for (int i = 0; i < MAXCOMPWND; i++)
    if (IsWindow(lpUIExtra->uiComp[i].hWnd))
      SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd, FIGWLP_FONT,
                       (LONG_PTR)lpUIExtra->hFont);
}

LRESULT CALLBACK CompWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                    LPARAM lParam) {
  FOOTMARK();
  HWND hUIWnd;

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
    hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    if (IsWindow(hUIWnd))
      SendMessage(hUIWnd, WM_UI_DEFCOMPMOVE, wParam, lParam);
    break;

  default:
    if (!IsImeMessage(message))
      return DefWindowProc(hWnd, message, wParam, lParam);
    break;
  }
  return 0;
}

void CompWnd_MoveMessage(HWND hSvrWnd, LPUIEXTRA lpUIExtra) {
  if (::IsWindow(hSvrWnd)) {
    ::SendMessage(hSvrWnd, WM_UI_COMPMOVE, 0, 0);
  }
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
