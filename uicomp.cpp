// uicomp.cpp --- mzimeja composition window UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define UNDERLINE_HEIGHT  2
#define CARET_WIDTH       2

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// count how may the char can be arranged in DX
static int NumCharInDX(HDC hDC, const WCHAR *psz, int dx) {
  int ret = 0;
  if (*psz) {
    SIZE siz;
    int width = 0, ich = 0;
    while (width < dx) {
      ret = ich;
      if (psz[ich] == L'\0') {
        break;
      }
      ich++;
      ::GetTextExtentPointW(hDC, psz, ich, &siz);
      width = siz.cx;
    }
  }
  return ret;
}

// count how may the char can be arranged in DY
static int NumCharInDY(HDC hDC, const WCHAR *psz, int dy) {
  int ret = 0;
  if (*psz) {
    SIZE siz;
    int height = 0, ich = 0;
    while (height < dy) {
      ret = ich;
      if (psz[ich] == L'\0') {
        break;
      }
      ich++;
      ::GetTextExtentPointW(hDC, psz, ich, &siz);
      height = siz.cy;
    }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////

void CompWnd_Create(HWND hUIWnd, LPUIEXTRA lpUIExtra, InputContext *lpIMC) {
  FOOTMARK();
  RECT rc;

  lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;
  for (int i = 0; i < MAXCOMPWND; i++) {
    HWND hwnd = lpUIExtra->uiComp[i].hWnd;
    if (!::IsWindow(hwnd)) {
      hwnd = ::CreateWindowEx(0, szCompStrClassName, NULL, WS_COMPNODEFAULT,
                              0, 0, 1, 1, hUIWnd, NULL, TheIME.m_hInst, NULL);
      lpUIExtra->uiComp[i].hWnd = hwnd;
    }
    ::SetRectEmpty(&lpUIExtra->uiComp[i].rc);
    ::SetWindowLongPtr(hwnd, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
    ::SetWindowLongPtr(hwnd, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
    ::ShowWindow(hwnd, SW_HIDE);
    lpUIExtra->uiComp[i].bShow = FALSE;
  }

  if (lpUIExtra->uiDefComp.pt.x == -1) {
    ::GetWindowRect(lpIMC->hWnd, &rc);
    lpUIExtra->uiDefComp.pt.x = rc.left;
    lpUIExtra->uiDefComp.pt.y = rc.bottom + 1;
  }

  HWND hwndDef = lpUIExtra->uiDefComp.hWnd;
  if (!::IsWindow(hwndDef)) {
    hwndDef = CreateWindowEx(
        WS_EX_WINDOWEDGE, szCompStrClassName, NULL,
        WS_COMPDEFAULT | WS_DLGFRAME, lpUIExtra->uiDefComp.pt.x,
        lpUIExtra->uiDefComp.pt.y, 1, 1, hUIWnd, NULL, TheIME.m_hInst, NULL);
    lpUIExtra->uiDefComp.hWnd = hwndDef;
  }
  ::SetWindowLong(hwndDef, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
  ::SetWindowLongPtr(hwndDef, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
  ::ShowWindow(hwndDef, SW_HIDE);
  lpUIExtra->uiDefComp.bShow = FALSE;
}

HWND ClauseToCompWnd(LPUIEXTRA lpUIExtra, InputContext *lpIMC, DWORD iClause) {
  if (lpIMC->cfCompForm.dwStyle) {
    HWND hwnd = lpUIExtra->uiComp[0].hWnd;
    for (int i = 0; i < MAXCOMPWND; i++) {
      DWORD clause = ::GetWindowLong(hwnd, FIGWL_COMPSTARTCLAUSE);
      if (iClause < clause) {
        break;
      }
      hwnd = lpUIExtra->uiComp[i].hWnd;
      if (iClause == clause) {
        break;
      }
    }
    return hwnd;
  } else {
    return lpUIExtra->uiDefComp.hWnd;
  }
}

// calc the position of composition windows and move them
void CompWnd_Move(LPUIEXTRA lpUIExtra, InputContext *lpIMC) {
  FOOTMARK();

  lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;

  HFONT hFont = NULL;
  HFONT hOldFont = NULL;
  if (lpIMC->cfCompForm.dwStyle) {  // style is not CFS_DEFAULT
    // lock the COMPOSITIONSTRING structure
    if (!lpIMC->HasCompStr()) return;
    CompStr *lpCompStr = lpIMC->LockCompStr();
    if (lpCompStr == NULL) return;

    // set the rectangle for the composition string
    RECT rcSrc;
    if (lpIMC->cfCompForm.dwStyle & CFS_RECT)
      rcSrc = lpIMC->cfCompForm.rcArea;
    else
      ::GetClientRect(lpIMC->hWnd, &rcSrc);

    POINT ptSrc = lpIMC->cfCompForm.ptCurrentPos;
    ::ClientToScreen(lpIMC->hWnd, &ptSrc);
    ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.left);
    ::ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.right);

    // check the start position
    if (!::PtInRect(&rcSrc, ptSrc)) {
      lpIMC->UnlockCompStr();
      return;
    }

    // hide the default composition window
    HWND hwndDef = lpUIExtra->uiDefComp.hWnd;
    if (::IsWindow(hwndDef)) {
      ::ShowWindow(hwndDef, SW_HIDE);
      lpUIExtra->uiDefComp.bShow = FALSE;
    }

    std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
    const WCHAR *psz = str.c_str();
    const WCHAR *pch = psz;
    DWORD ich = 0, iClause = 0;

    // clause info
    DWORD *pdw = lpCompStr->GetCompClause();
    DWORD *pdwEnd = pdw + lpCompStr->dwCompClauseLen / sizeof(DWORD);
    std::set<DWORD> clauses(pdw, pdwEnd);

    if (!lpUIExtra->bVertical) {  // not vertical font
      int dx = rcSrc.right - ptSrc.x;
      int curx = ptSrc.x, cury = ptSrc.y;

      // set the composition string to each composition window.
      // the composition windows that are given the compostion string
      // will be moved and shown.
      for (int i = 0; i < MAXCOMPWND; i++) {
        HWND hwnd = lpUIExtra->uiComp[i].hWnd;
        if (::IsWindow(hwnd)) {
          HDC hDC = ::GetDC(hwnd);
          hFont = (HFONT)::GetWindowLongPtr(hwnd, FIGWLP_FONT);
          if (hFont) hOldFont = (HFONT)::SelectObject(hDC, hFont);

          SIZE siz;
          siz.cy = 0;

          int num = NumCharInDX(hDC, pch, dx);
          if (num) {
            ::GetTextExtentPoint32W(hDC, pch, num, &siz);

            lpUIExtra->uiComp[i].rc.left = curx;
            lpUIExtra->uiComp[i].rc.top = cury;
            siz.cx += CARET_WIDTH;
            siz.cy += UNDERLINE_HEIGHT;
            lpUIExtra->uiComp[i].rc.right = siz.cx;
            lpUIExtra->uiComp[i].rc.bottom = siz.cy;
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, LONG(pch - psz));
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, num);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTCLAUSE, iClause);
            ::MoveWindow(hwnd, curx, cury, siz.cx, siz.cy, TRUE);
            ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);
            lpUIExtra->uiComp[i].bShow = TRUE;

            pch += num;
            ich += num;

            if (clauses.count(ich) > 0) {
              ++iClause;
            }
          } else {
            ::SetRectEmpty(&lpUIExtra->uiComp[i].rc);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, 0);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, 0);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTCLAUSE, MAXLONG);
            ::ShowWindow(hwnd, SW_HIDE);
            lpUIExtra->uiComp[i].bShow = FALSE;
          }
          ::InvalidateRect(hwnd, NULL, FALSE);

          dx = rcSrc.right - rcSrc.left;
          curx = rcSrc.left;
          cury += siz.cy + UNDERLINE_HEIGHT;

          if (hOldFont) ::SelectObject(hDC, hOldFont);
          ::ReleaseDC(hwnd, hDC);
        }
      }
    } else {  // vertical font
      int dy = rcSrc.bottom - ptSrc.y;
      int curx = ptSrc.x, cury = ptSrc.y;

      for (int i = 0; i < MAXCOMPWND; i++) {
        HWND hwnd = lpUIExtra->uiComp[i].hWnd;
        if (::IsWindow(hwnd)) {
          HDC hDC = ::GetDC(hwnd);
          hFont = (HFONT)::GetWindowLongPtr(hwnd, FIGWLP_FONT);
          if (hFont) hOldFont = (HFONT)::SelectObject(hDC, hFont);

          SIZE siz;
          siz.cy = 0;
          int num = NumCharInDY(hDC, pch, dy);
          if (num) {
            ::GetTextExtentPoint32W(hDC, pch, num, &siz);

            lpUIExtra->uiComp[i].rc.left = curx - siz.cy;
            lpUIExtra->uiComp[i].rc.top = cury;
            siz.cy += UNDERLINE_HEIGHT;
            siz.cx += CARET_WIDTH;
            lpUIExtra->uiComp[i].rc.right = siz.cy;
            lpUIExtra->uiComp[i].rc.bottom = siz.cx;
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, LONG(pch - psz));
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, num);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTCLAUSE, iClause);
            ::MoveWindow(hwnd, curx, cury, siz.cy, siz.cx, TRUE);
            ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);
            lpUIExtra->uiComp[i].bShow = TRUE;

            pch += num;
            ich += num;

            if (clauses.count(ich) > 0) {
              ++iClause;
            }
          } else {
            ::SetRectEmpty(&lpUIExtra->uiComp[i].rc);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, 0);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, 0);
            ::SetWindowLong(hwnd, FIGWL_COMPSTARTCLAUSE, MAXLONG);
            ::ShowWindow(hwnd, SW_HIDE);
            lpUIExtra->uiComp[i].bShow = FALSE;
          }
          ::InvalidateRect(hwnd, NULL, FALSE);

          dy = rcSrc.bottom - rcSrc.top;
          cury = rcSrc.top;
          curx -= siz.cy + UNDERLINE_HEIGHT;

          if (hOldFont) ::SelectObject(hDC, hOldFont);
          ::ReleaseDC(hwnd, hDC);
        }
      }
    }
    lpIMC->UnlockCompStr();
  } else {  // style is CFS_DEFAULT
    HWND hwndDef = lpUIExtra->uiDefComp.hWnd;
    if (::IsWindow(hwndDef)) {
      ::SetWindowLong(hwndDef, FIGWL_COMPSTARTSTR, 0);
      ::SetWindowLong(hwndDef, FIGWL_COMPSTARTNUM, 0);
      ::SetWindowLong(hwndDef, FIGWL_COMPSTARTCLAUSE, 0);

      // hide all non-default comp windows
      for (int i = 0; i < MAXCOMPWND; i++) {
        HWND hwnd = lpUIExtra->uiComp[i].hWnd;
        if (::IsWindow(hwnd)) {
          ::ShowWindow(hwnd, SW_HIDE);
          lpUIExtra->uiComp[i].bShow = FALSE;
        }
      }

      // get width and height of composition string
      int width = 0, height = 0;
      HDC hDC = ::GetDC(hwndDef);
      CompStr *lpCompStr = lpIMC->LockCompStr();
      if (lpCompStr) {
        if (lpCompStr->dwCompStrLen > 0) {
          std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
          const WCHAR *psz = str.c_str();
          SIZE siz;
          ::GetTextExtentPoint32W(hDC, psz, lstrlenW(psz), &siz);
          width = siz.cx;
          height = siz.cy;
        }
        lpIMC->UnlockCompStr();
      }
      ::ReleaseDC(hwndDef, hDC);

      // calculate new window extent
      RECT rc;
      ::GetWindowRect(hwndDef, &rc);
      lpUIExtra->uiDefComp.pt.x = rc.left;
      lpUIExtra->uiDefComp.pt.y = rc.top;
      width += 2 * ::GetSystemMetrics(SM_CXEDGE) + CARET_WIDTH;
      height += 2 * ::GetSystemMetrics(SM_CYEDGE) + UNDERLINE_HEIGHT;

      // move and show window
      ::MoveWindow(hwndDef, rc.left, rc.top, width, height, TRUE);
      ::ShowWindow(hwndDef, SW_SHOWNOACTIVATE);
      lpUIExtra->uiDefComp.bShow = TRUE;

      // redraw window
      ::InvalidateRect(hwndDef, NULL, FALSE);
    }
  }
} // CompWnd_Move

void DrawTextOneLine(HWND hCompWnd, HDC hDC, const WCHAR *pch,
                     DWORD ich, DWORD cch, CompStr *lpCompStr, BOOL fVert) {
  FOOTMARK();

  if (cch == 0) return;

  // attribute
  BYTE *lpattr = lpCompStr->GetCompAttr();

  // get clause info
  DWORD *pdw = lpCompStr->GetCompClause();
  DWORD *pdwEnd = pdw + lpCompStr->dwCompClauseLen / sizeof(DWORD);
  std::set<DWORD> clauses(pdw, pdwEnd);

  // get client rect
  RECT rc;
  ::GetClientRect(hCompWnd, &rc);

  // starting position
  int x, y;
  if (fVert) {
    x = rc.right - UNDERLINE_HEIGHT;
    y = 0;
  } else {
    x = y = 0;
  }

  // is it end?
  SIZE siz;
  const WCHAR *lpEnd = &pch[cch];
  while (pch < lpEnd) {
    ::SetBkMode(hDC, OPAQUE);

    // set color and pen
    HPEN hPen;
    switch (lpattr[ich]) {
    case ATTR_TARGET_CONVERTED:
      ::SetTextColor(hDC, RGB(0, 51, 0));
      ::SetBkColor(hDC, RGB(255, 255, 255));
      hPen = ::CreatePen(PS_SOLID, 1, RGB(0, 51, 0));
      break;
    case ATTR_CONVERTED:
      ::SetTextColor(hDC, RGB(0, 51, 0));
      ::SetBkColor(hDC, RGB(255, 255, 255));
      hPen = ::CreatePen(PS_SOLID, 1, RGB(0, 51, 0));
      break;
    default:
      ::SetTextColor(hDC, RGB(0, 51, 0));
      ::SetBkColor(hDC, RGB(255, 255, 255));
      hPen = ::CreatePen(PS_DOT, 1, RGB(0, 121, 0));
      break;
    }
    // draw text
    ::TextOutW(hDC, x, y, pch, 1);

    // get size of text
    ::GetTextExtentPoint32W(hDC, pch, 1, &siz);

    // draw underline if target converted
    INT nClauseSep = 2 * (clauses.count(ich + 1) > 0);
    HGDIOBJ hPenOld = ::SelectObject(hDC, hPen);
    if (lpattr[ich] == ATTR_TARGET_CONVERTED) {
      if (fVert) {
        ::MoveToEx(hDC, x + 1, y, NULL);
        ::LineTo(hDC, x + 1, y + siz.cx - nClauseSep);
        ::MoveToEx(hDC, x + 2, y, NULL);
        ::LineTo(hDC, x + 2, y + siz.cx - nClauseSep);
      } else {
        ::MoveToEx(hDC, x, y + siz.cy - 1, NULL);
        ::LineTo(hDC, x + siz.cx - nClauseSep, y + siz.cy - 1);
        ::MoveToEx(hDC, x, y + siz.cy, NULL);
        ::LineTo(hDC, x + siz.cx - nClauseSep, y + siz.cy);
      }
    } else {
      if (fVert) {
        ::MoveToEx(hDC, x + 1, y, NULL);
        ::LineTo(hDC, x + 1, y + siz.cx - nClauseSep);
      } else {
        ::MoveToEx(hDC, x, y + siz.cy - 1, NULL);
        ::LineTo(hDC, x + siz.cx - nClauseSep, y + siz.cy - 1);
      }
    }
    ::DeleteObject(::SelectObject(hDC, hPenOld));

    // draw underline if not target converted
    if (lpattr[ich] != ATTR_TARGET_CONVERTED) {
      ::SelectObject(hDC, ::GetStockObject(WHITE_PEN));
      if (fVert) {
        ::MoveToEx(hDC, x + 2, y, NULL);
        ::LineTo(hDC, x + 2, y + siz.cx - nClauseSep);
      } else {
        ::MoveToEx(hDC, x, y + siz.cy, NULL);
        ::LineTo(hDC, x + siz.cx - nClauseSep, y + siz.cy);
      }
    }

    // draw cursor (caret)
    if (lpCompStr->dwCursorPos == ich) {
      ::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
      if (fVert) {
        ::MoveToEx(hDC, x, y, NULL);
        ::LineTo(hDC, x + siz.cy, y);
        ::MoveToEx(hDC, x, y + 1, NULL);
        ::LineTo(hDC, x + siz.cy, y + 1);
      } else {
        ::MoveToEx(hDC, x, y, NULL);
        ::LineTo(hDC, x, y + siz.cy);
        ::MoveToEx(hDC, x + 1, y, NULL);
        ::LineTo(hDC, x + 1, y + siz.cy);
      }
    }

    // go to next position
    ++pch;
    ++ich;
    if (fVert)
      y += siz.cx;
    else
      x += siz.cx;
  }

  // draw caret at last if any
  if (lpCompStr->dwCursorPos == ich) {
    ::SelectObject(hDC, ::GetStockObject(BLACK_PEN));
  } else {
    ::SelectObject(hDC, ::GetStockObject(WHITE_PEN));
  }
  if (fVert) {
    ::MoveToEx(hDC, x, y, NULL);
    ::LineTo(hDC, x + siz.cy, y);
    ::MoveToEx(hDC, x, y + 1, NULL);
    ::LineTo(hDC, x + siz.cy, y + 1);
  } else {
    ::MoveToEx(hDC, x, y, NULL);
    ::LineTo(hDC, x, y + siz.cy);
    ::MoveToEx(hDC, x + 1, y, NULL);
    ::LineTo(hDC, x + 1, y + siz.cy);
  }
}

void CompWnd_Draw(HWND hCompWnd, HDC hDC, InputContext *lpIMC, CompStr *lpCompStr) {
  // get comp string
  std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
  const WCHAR *pch = str.c_str();

  // is it vertical?
  BOOL fVert = (lpIMC->lfFont.A.lfEscapement == 2700);

  if (lpIMC->cfCompForm.dwStyle) {
    ::SetBkMode(hDC, OPAQUE);

    DWORD ich = ::GetWindowLong(hCompWnd, FIGWL_COMPSTARTSTR);
    DWORD cch = ::GetWindowLong(hCompWnd, FIGWL_COMPSTARTNUM);
    if (cch && ich + cch <= DWORD(::lstrlenW(pch))) {
      pch += ich;
      DrawTextOneLine(hCompWnd, hDC, pch, ich, cch, lpCompStr, fVert);
    }
  } else {
    int cch = int(str.size());
    DrawTextOneLine(hCompWnd, hDC, pch, 0, cch, lpCompStr, fVert);
  }
}

void CompWnd_Paint(HWND hCompWnd) {
  FOOTMARK();

  PAINTSTRUCT ps;
  HDC hDC = ::BeginPaint(hCompWnd, &ps);

  HFONT hOldFont = NULL;
  HFONT hFont = (HFONT)::GetWindowLongPtr(hCompWnd, FIGWLP_FONT);
  if (hFont) hOldFont = (HFONT)::SelectObject(hDC, hFont);

  HWND hSvrWnd = (HWND)::GetWindowLongPtr(hCompWnd, FIGWLP_SERVERWND);

  HIMC hIMC = (HIMC)::GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC);
  if (hIMC) {
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      CompStr *lpCompStr = lpIMC->LockCompStr();
      if (lpCompStr) {
        if (lpCompStr->dwCompStrLen > 0) {
          CompWnd_Draw(hCompWnd, hDC, lpIMC, lpCompStr);
        }
        lpIMC->UnlockCompStr();
      }
      TheIME.UnlockIMC(hIMC);
    }
  }
  if (hFont && hOldFont) ::SelectObject(hDC, hOldFont);
  ::EndPaint(hCompWnd, &ps);
} // CompWnd_Paint

void CompWnd_Hide(LPUIEXTRA lpUIExtra) {
  FOOTMARK();

  RECT rc;
  HWND hwndDef = lpUIExtra->uiDefComp.hWnd;
  if (IsWindow(hwndDef)) {
    if (!lpUIExtra->dwCompStyle) {
      ::GetWindowRect(hwndDef, &rc);
    }
    ::ShowWindow(hwndDef, SW_HIDE);
    lpUIExtra->uiDefComp.bShow = FALSE;
  }

  for (int i = 0; i < MAXCOMPWND; i++) {
    HWND hwnd = lpUIExtra->uiComp[i].hWnd;
    if (::IsWindow(hwnd)) {
      ::ShowWindow(hwnd, SW_HIDE);
      lpUIExtra->uiComp[i].bShow = FALSE;
    }
  }
} // CompWnd_Hide

void CompWnd_SetFont(LPUIEXTRA lpUIExtra) {
  FOOTMARK();
  for (int i = 0; i < MAXCOMPWND; i++) {
    HWND hwnd = lpUIExtra->uiComp[i].hWnd;
    if (::IsWindow(hwnd)) {
      ::SetWindowLongPtr(hwnd, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
    }
  }
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

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
