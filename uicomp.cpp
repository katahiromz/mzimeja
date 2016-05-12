// uicomp.cpp --- mzimeja composition window UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// Count how may the char can be arranged in DX
static int NumCharInDX(HDC hDC, LPCWSTR psz, int dx) {
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

// Count how may the char can be arranged in DY
static int NumCharInDY(HDC hDC, LPCWSTR psz, int dy) {
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
    ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
    lpUIExtra->uiComp[i].bShow = FALSE;
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
  ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_HIDE);
  lpUIExtra->uiDefComp.bShow = FALSE;

  return;
}

// Calc the position of composition windows and move them
void CompWnd_Move(LPUIEXTRA lpUIExtra, InputContext *lpIMC) {
  FOOTMARK();

  HDC hDC;
  HFONT hFont = NULL;
  HFONT hOldFont = NULL;
  CompStr *lpCompStr;
  LPCWSTR lpstr;
  RECT rc;
  RECT oldrc;
  SIZE siz;
  int width = 0;
  int height = 0;

  // Save the composition form style into lpUIExtra.
  lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;

  if (lpIMC->cfCompForm.dwStyle) {  // Style is not CFS_DEFAULT.
    LPCWSTR pch;
    int num;

    if (!lpIMC->HasCompStr()) {
      return;
    }

    // Lock the COMPOSITIONSTRING structure.
    lpCompStr = lpIMC->LockCompStr();
    if (lpCompStr == NULL) {
      return;
    }

    // Set the rectangle for the composition string.
    RECT rcSrc;
    if (lpIMC->cfCompForm.dwStyle & CFS_RECT)
      rcSrc = lpIMC->cfCompForm.rcArea;
    else
      GetClientRect(lpIMC->hWnd, &rcSrc);

    POINT ptSrc = lpIMC->cfCompForm.ptCurrentPos;
    ClientToScreen(lpIMC->hWnd, &ptSrc);
    ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.left);
    ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.right);

    // Check the start position.
    if (!PtInRect(&rcSrc, ptSrc)) {
      lpIMC->UnlockCompStr();
      return;
    }

    // Hide the default composition window.
    if (IsWindow(lpUIExtra->uiDefComp.hWnd)) {
      ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_HIDE);
      lpUIExtra->uiDefComp.bShow = FALSE;
    }

    std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
    pch = lpstr = str.c_str();
    num = 1;

    if (!lpUIExtra->bVertical) {
      int dx = rcSrc.right - ptSrc.x;
      int curx = ptSrc.x, cury = ptSrc.y;

      // Set the composition string to each composition window.
      // The composition windows that are given the compostion string
      // will be moved and shown.
      for (int i = 0; i < MAXCOMPWND; i++) {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
          hDC = GetDC(lpUIExtra->uiComp[i].hWnd);

          hFont = (HFONT)GetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,
                                          FIGWLP_FONT);
          if (hFont)
            hOldFont = (HFONT)SelectObject(hDC, hFont);

          siz.cy = 0;
          oldrc = lpUIExtra->uiComp[i].rc;

          num = NumCharInDX(hDC, pch, dx);
          if (num) {
            ::GetTextExtentPoint32W(hDC, pch, num, &siz);

            lpUIExtra->uiComp[i].rc.left = curx;
            lpUIExtra->uiComp[i].rc.top = cury;
            lpUIExtra->uiComp[i].rc.right = siz.cx;
            lpUIExtra->uiComp[i].rc.bottom = siz.cy + 1;
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR,
                          (DWORD)(pch - lpstr));
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, num);
            MoveWindow(lpUIExtra->uiComp[i].hWnd, curx, cury, siz.cx, siz.cy,
                       TRUE);
            ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_SHOWNOACTIVATE);
            lpUIExtra->uiComp[i].bShow = TRUE;

            pch += num;
          } else {
            lpUIExtra->uiComp[i].rc.left = 0;
            lpUIExtra->uiComp[i].rc.top = 0;
            lpUIExtra->uiComp[i].rc.right = 0;
            lpUIExtra->uiComp[i].rc.bottom = 0;
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR, 0L);
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, 0L);
            ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
            lpUIExtra->uiComp[i].bShow = FALSE;
          }

          InvalidateRect(lpUIExtra->uiComp[i].hWnd, NULL, FALSE);

          dx = rcSrc.right - rcSrc.left;
          curx = rcSrc.left;
          cury += siz.cy + 1;

          if (hOldFont) SelectObject(hDC, hOldFont);
          ReleaseDC(lpUIExtra->uiComp[i].hWnd, hDC);
        }
      }
    } else {
      // when it is vertical fonts.
      int dy = rcSrc.bottom - ptSrc.y;
      int curx = ptSrc.x, cury = ptSrc.y;

      for (int i = 0; i < MAXCOMPWND; i++) {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
          hDC = GetDC(lpUIExtra->uiComp[i].hWnd);

          hFont = (HFONT)GetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,
                                          FIGWLP_FONT);
          if (hFont)
            hOldFont = (HFONT)SelectObject(hDC, hFont);

          siz.cy = 0;
          num = NumCharInDY(hDC, pch, dy);
          if (num) {
            ::GetTextExtentPoint32W(hDC, pch, num, &siz);

            lpUIExtra->uiComp[i].rc.left = curx - siz.cy;
            lpUIExtra->uiComp[i].rc.top = cury;
            lpUIExtra->uiComp[i].rc.right = siz.cy + 1;
            lpUIExtra->uiComp[i].rc.bottom = siz.cx;
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR,
                          (DWORD)(pch - lpstr));
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, num);
            MoveWindow(lpUIExtra->uiComp[i].hWnd, curx, cury, siz.cy, siz.cx,
                       TRUE);
            ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_SHOWNOACTIVATE);
            lpUIExtra->uiComp[i].bShow = TRUE;

            pch += num;
          } else {
            lpUIExtra->uiComp[i].rc.left = 0;
            lpUIExtra->uiComp[i].rc.top = 0;
            lpUIExtra->uiComp[i].rc.right = 0;
            lpUIExtra->uiComp[i].rc.bottom = 0;
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTSTR, 0L);
            SetWindowLong(lpUIExtra->uiComp[i].hWnd, FIGWL_COMPSTARTNUM, 0L);
            ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
            lpUIExtra->uiComp[i].bShow = FALSE;
          }

          InvalidateRect(lpUIExtra->uiComp[i].hWnd, NULL, FALSE);

          dy = rcSrc.bottom - rcSrc.top;
          cury = rcSrc.top;
          curx -= siz.cy + 1;

          if (hOldFont) SelectObject(hDC, hOldFont);
          ReleaseDC(lpUIExtra->uiComp[i].hWnd, hDC);
        }
      }
    }

    lpIMC->UnlockCompStr();
  } else {
    // When the style is DEFAULT, show the default composition window.
    if (IsWindow(lpUIExtra->uiDefComp.hWnd)) {
      for (int i = 0; i < MAXCOMPWND; i++) {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
          ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
          lpUIExtra->uiComp[i].bShow = FALSE;
        }
      }

      hDC = GetDC(lpUIExtra->uiDefComp.hWnd);

      lpCompStr = lpIMC->LockCompStr();
      if (lpCompStr) {
        if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING)) &&
            (lpCompStr->dwCompStrLen > 0)) {

          std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
          lpstr = str.c_str();
          ::GetTextExtentPoint32W(hDC, lpstr, lstrlenW(lpstr), &siz);
          width = siz.cx;
          height = siz.cy + 1;
        }
        lpIMC->UnlockCompStr();
      }

      ReleaseDC(lpUIExtra->uiDefComp.hWnd, hDC);

      GetWindowRect(lpUIExtra->uiDefComp.hWnd, &rc);
      lpUIExtra->uiDefComp.pt.x = rc.left;
      lpUIExtra->uiDefComp.pt.y = rc.top;
      MoveWindow(lpUIExtra->uiDefComp.hWnd, rc.left, rc.top,
                 width + 2 * GetSystemMetrics(SM_CXEDGE),
                 height + 2 * GetSystemMetrics(SM_CYEDGE), TRUE);

      ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_SHOWNOACTIVATE);
      lpUIExtra->uiDefComp.bShow = TRUE;
      InvalidateRect(lpUIExtra->uiDefComp.hWnd, NULL, FALSE);
    }
  }
}

void DrawTextOneLine(HWND hCompWnd, HDC hDC, LPCWSTR lpstr,
                     LPBYTE lpattr, int num, BOOL fVert, DWORD dwCursor) {
  FOOTMARK();
  //LPTSTR lpStart = lpstr;
  LPCWSTR lpEnd = lpstr + num - 1;
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
    GetTextExtentPoint(hDC, lpstr, 1, &siz);

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
        LPCWSTR lpstr;
        LPBYTE lpattr;
        INT lstart;
        INT num;
        BOOL fVert = FALSE;
        DWORD dwCursor = lpCompStr->dwCursorPos;

        if (hFont) fVert = (lpIMC->lfFont.A.lfEscapement == 2700);

        std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
        lpstr = str.c_str();
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
          num = (INT)str.size();
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

    ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_HIDE);
    lpUIExtra->uiDefComp.bShow = FALSE;
  }

  for (int i = 0; i < MAXCOMPWND; i++) {
    if (IsWindow(lpUIExtra->uiComp[i].hWnd)) {
      ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
      lpUIExtra->uiComp[i].bShow = FALSE;
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

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
