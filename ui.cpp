// ui.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

void PASCAL ShowUIWindows(HWND hWnd, BOOL fFlag) {
  HGLOBAL hUIExtra;
  LPUIEXTRA lpUIExtra;
  int nsw = fFlag ? SW_SHOWNOACTIVATE : SW_HIDE;

  if (!(hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE))) return;

  if (!(lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra))) return;

  if (IsWindow(lpUIExtra->uiStatus.hWnd)) {
    ShowWindow(lpUIExtra->uiStatus.hWnd, nsw);
    lpUIExtra->uiStatus.bShow = fFlag;
  }

  if (IsWindow(lpUIExtra->uiCand.hWnd)) {
    ShowWindow(lpUIExtra->uiCand.hWnd, nsw);
    lpUIExtra->uiCand.bShow = fFlag;
  }

  if (IsWindow(lpUIExtra->uiDefComp.hWnd)) {
    ShowWindow(lpUIExtra->uiDefComp.hWnd, nsw);
    lpUIExtra->uiDefComp.bShow = fFlag;
  }

  if (IsWindow(lpUIExtra->uiGuide.hWnd)) {
    ShowWindow(lpUIExtra->uiGuide.hWnd, nsw);
    lpUIExtra->uiGuide.bShow = fFlag;
  }

  GlobalUnlock(hUIExtra);
}

#ifdef _DEBUG
void PASCAL DumpUIExtra(LPUIEXTRA lpUIExtra) {
  DebugPrint(TEXT("Status hWnd %lX  [%d,%d]\n"),
             lpUIExtra->uiStatus.hWnd, lpUIExtra->uiStatus.pt.x,
             lpUIExtra->uiStatus.pt.y);

  DebugPrint(TEXT("Cand hWnd %lX  [%d,%d]\n"),
             lpUIExtra->uiCand.hWnd, lpUIExtra->uiCand.pt.x,
             lpUIExtra->uiCand.pt.y);

  DebugPrint(TEXT("CompStyle hWnd %lX]\n"), lpUIExtra->dwCompStyle);

  DebugPrint(TEXT("DefComp hWnd %lX  [%d,%d]\n"),
             lpUIExtra->uiDefComp.hWnd, lpUIExtra->uiDefComp.pt.x,
             lpUIExtra->uiDefComp.pt.y);

  for (int i = 0; i < 5; i++) {
    DebugPrint(TEXT("Comp hWnd %lX  [%d,%d]-[%d,%d]\n"),
               lpUIExtra->uiComp[i].hWnd, lpUIExtra->uiComp[i].rc.left,
               lpUIExtra->uiComp[i].rc.top, lpUIExtra->uiComp[i].rc.right,
               lpUIExtra->uiComp[i].rc.bottom);
  }
}
#endif  // def _DEBUG

// IME UI window procedure
LRESULT CALLBACK MZIMEWndProc(HWND hWnd, UINT message, WPARAM wParam,
                              LPARAM lParam) {
  InputContext *lpIMC;
  LPUIEXTRA lpUIExtra;
  HGLOBAL hUIExtra;
  LONG lRet = 0L;
  int i;

  HIMC hIMC = (HIMC)GetWindowLongPtr(hWnd, IMMGWLP_IMC);

  // Even if there is no current UI. these messages should not be pass to
  // DefWindowProc().
  if (!hIMC) {
    switch (message) {
      case WM_IME_STARTCOMPOSITION:
      case WM_IME_ENDCOMPOSITION:
      case WM_IME_COMPOSITION:
      case WM_IME_NOTIFY:
      case WM_IME_CONTROL:
      case WM_IME_COMPOSITIONFULL:
      case WM_IME_SELECT:
      case WM_IME_CHAR:
        DebugPrint(TEXT("Why hIMC is NULL????\n"));
        DebugPrint(TEXT("\thWnd is %x\n"), hWnd);
        DebugPrint(TEXT("\tmessage is %x\n"), message);
        DebugPrint(TEXT("\twParam is %x\n"), wParam);
        DebugPrint(TEXT("\tlParam is %x\n"), lParam);
        return 0L;
      default:
        break;
    }
  }

  switch (message) {
    case WM_CREATE:
      // Allocate UI's extra memory block.
      hUIExtra = GlobalAlloc(GHND, sizeof(UIEXTRA));
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

      // Initialize the extra memory block.
      lpUIExtra->uiStatus.pt.x = -1;
      lpUIExtra->uiStatus.pt.y = -1;
      lpUIExtra->uiDefComp.pt.x = -1;
      lpUIExtra->uiDefComp.pt.y = -1;
      lpUIExtra->uiCand.pt.x = -1;
      lpUIExtra->uiCand.pt.y = -1;
      lpUIExtra->uiGuide.pt.x = -1;
      lpUIExtra->uiGuide.pt.y = -1;
      lpUIExtra->hFont = NULL;

      GlobalUnlock(hUIExtra);
      SetWindowLongPtr(hWnd, IMMGWLP_PRIVATE, (LONG_PTR)hUIExtra);
      break;

    case WM_IME_SETCONTEXT:
      if (wParam) {
        hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
        lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
        lpUIExtra->hIMC = hIMC;

        if (hIMC) {
          //LPINPUTCONTEXT lpIMCT = NULL;
          //
          // input context was chenged.
          // if there are the child windows, the diplay have to be
          // updated.
          lpIMC = TheIME.LockIMC(hIMC);
          if (lpIMC) {
            CompStr *lpCompStr = lpIMC->LockCompStr();
            CandInfo *lpCandInfo = lpIMC->LockCandInfo();
            if (IsWindow(lpUIExtra->uiCand.hWnd)) HideCandWindow(lpUIExtra);
            if (lParam & ISC_SHOWUICANDIDATEWINDOW) {
              if (lpCandInfo->dwCount) {
                CreateCandWindow(hWnd, lpUIExtra, lpIMC);
                ResizeCandWindow(lpUIExtra, lpIMC);
                MoveCandWindow(hWnd, lpIMC, lpUIExtra, FALSE);
              }
            }

            if (IsWindow(lpUIExtra->uiDefComp.hWnd)) HideCompWindow(lpUIExtra);

            if (lParam & ISC_SHOWUICANDIDATEWINDOW) {
              if (lpCompStr->dwCompStrLen) {
                CreateCompWindow(hWnd, lpUIExtra, lpIMC);
                MoveCompWindow(lpUIExtra, lpIMC);
              }
            }
            lpIMC->UnlockCompStr();
            lpIMC->UnlockCandInfo();
          } else {
            HideCandWindow(lpUIExtra);
            HideCompWindow(lpUIExtra);
          }
          StatusWnd_Update(lpUIExtra);
          TheIME.UnlockIMC();
        } else  // it is NULL input context.
        {
          HideCandWindow(lpUIExtra);
          HideCompWindow(lpUIExtra);
        }
        GlobalUnlock(hUIExtra);
      }
      // else
      //    ShowUIWindows(hWnd, FALSE);
      break;

    case WM_IME_STARTCOMPOSITION:
      // Start composition! Ready to display the composition string.
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
      lpIMC = TheIME.LockIMC(hIMC);
      CreateCompWindow(hWnd, lpUIExtra, lpIMC);
      TheIME.UnlockIMC();
      GlobalUnlock(hUIExtra);
      break;

    case WM_IME_COMPOSITION:
      // Update to display the composition string.
      lpIMC = TheIME.LockIMC(hIMC);
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
      MoveCompWindow(lpUIExtra, lpIMC);
      MoveCandWindow(hWnd, lpIMC, lpUIExtra, TRUE);
      GlobalUnlock(hUIExtra);
      TheIME.UnlockIMC();
      break;

    case WM_IME_ENDCOMPOSITION:
      // Finish to display the composition string.
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
      HideCompWindow(lpUIExtra);
      GlobalUnlock(hUIExtra);
      break;

    case WM_IME_COMPOSITIONFULL:
      break;

    case WM_IME_SELECT:
      if (wParam) {
        hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
        lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
        lpUIExtra->hIMC = hIMC;
        GlobalUnlock(hUIExtra);
      }
      break;

    case WM_IME_CONTROL:
      lRet = ControlCommand(hIMC, hWnd, message, wParam, lParam);
      break;

    case WM_IME_NOTIFY:
      lRet = NotifyCommand(hIMC, hWnd, message, wParam, lParam);
      break;

    case WM_DESTROY:
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

      if (IsWindow(lpUIExtra->uiStatus.hWnd))
        DestroyWindow(lpUIExtra->uiStatus.hWnd);

      if (IsWindow(lpUIExtra->uiCand.hWnd))
        DestroyWindow(lpUIExtra->uiCand.hWnd);

      if (IsWindow(lpUIExtra->uiDefComp.hWnd))
        DestroyWindow(lpUIExtra->uiDefComp.hWnd);

      for (i = 0; i < MAXCOMPWND; i++) {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd))
          DestroyWindow(lpUIExtra->uiComp[i].hWnd);
      }

      if (IsWindow(lpUIExtra->uiGuide.hWnd))
        DestroyWindow(lpUIExtra->uiGuide.hWnd);

      if (lpUIExtra->hFont) DeleteObject(lpUIExtra->hFont);

      GlobalUnlock(hUIExtra);
      GlobalFree(hUIExtra);
      break;

    case WM_UI_STATEMOVE:
      // Set the position of the status window to UIExtra.
      // This message is sent by the status window.
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
      lpUIExtra->uiStatus.pt.x = (short)LOWORD(lParam);
      lpUIExtra->uiStatus.pt.y = (short)HIWORD(lParam);
      GlobalUnlock(hUIExtra);
      break;

    case WM_UI_DEFCOMPMOVE:
      // Set the position of the composition window to UIExtra.
      // This message is sent by the composition window.
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
      if (!lpUIExtra->dwCompStyle) {
        lpUIExtra->uiDefComp.pt.x = (short)LOWORD(lParam);
        lpUIExtra->uiDefComp.pt.y = (short)HIWORD(lParam);
      }
      GlobalUnlock(hUIExtra);
      break;

    case WM_UI_CANDMOVE:
      // Set the position of the candidate window to UIExtra.
      // This message is sent by the candidate window.
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
      lpUIExtra->uiCand.pt.x = (short)LOWORD(lParam);
      lpUIExtra->uiCand.pt.y = (short)HIWORD(lParam);
      GlobalUnlock(hUIExtra);
      break;

    case WM_UI_GUIDEMOVE:
      // Set the position of the status window to UIExtra.
      // This message is sent by the status window.
      hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
      lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
      lpUIExtra->uiGuide.pt.x = (long)LOWORD(lParam);
      lpUIExtra->uiGuide.pt.y = (long)HIWORD(lParam);
      GlobalUnlock(hUIExtra);
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return lRet;
}

int PASCAL GetCompFontHeight(LPUIEXTRA lpUIExtra) {
  HDC hIC;
  HFONT hOldFont = 0;
  SIZE sz;

  hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);

  if (lpUIExtra->hFont) hOldFont = (HFONT)SelectObject(hIC, lpUIExtra->hFont);
  GetTextExtentPoint(hIC, TEXT("A"), 1, &sz);

  if (hOldFont) SelectObject(hIC, hOldFont);

  DeleteDC(hIC);

  return sz.cy;
}

// Handle WM_IME_NOTIFY messages
LONG PASCAL NotifyCommand(HIMC hIMC, HWND hWnd, UINT message,
                          WPARAM wParam, LPARAM lParam) {
  LONG lRet = 0L;
  HGLOBAL hUIExtra;
  LPUIEXTRA lpUIExtra;
  RECT rc;
  LOGFONT lf;

  InputContext *lpIMC = TheIME.LockIMC(hIMC);
  if (NULL == lpIMC) return 0L;

  hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
  lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

  switch (wParam) {
    case IMN_CLOSESTATUSWINDOW:
      if (IsWindow(lpUIExtra->uiStatus.hWnd)) {
        ::GetWindowRect(lpUIExtra->uiStatus.hWnd, &rc);
        lpUIExtra->uiStatus.pt.x = rc.left;
        lpUIExtra->uiStatus.pt.y = rc.top;
        ::ShowWindow(lpUIExtra->uiStatus.hWnd, SW_HIDE);
        lpUIExtra->uiStatus.bShow = FALSE;
      }
      break;

    case IMN_OPENSTATUSWINDOW:
      StatusWnd_Create(hWnd, lpUIExtra);
      break;

    case IMN_SETCONVERSIONMODE:
      StatusWnd_Update(lpUIExtra);
      break;

    case IMN_SETSENTENCEMODE:
      break;

    case IMN_SETCOMPOSITIONFONT:
      lf = lpIMC->lfFont.W;
      if (lpUIExtra->hFont) DeleteObject(lpUIExtra->hFont);

      if (lf.lfEscapement == 2700)
        lpUIExtra->bVertical = TRUE;
      else {
        lf.lfEscapement = 0;
        lpUIExtra->bVertical = FALSE;
      }

      // if current font can't display Japanese characters,
      // try to find Japanese font
      if (lf.lfCharSet != NATIVE_CHARSET) {
        lf.lfCharSet = NATIVE_CHARSET;
        lf.lfFaceName[0] = 0;
      }

      lpUIExtra->hFont = CreateFontIndirect(&lf);
      SetFontCompWindow(lpUIExtra);
      MoveCompWindow(lpUIExtra, lpIMC);

      break;

    case IMN_SETOPENSTATUS:
      StatusWnd_Update(lpUIExtra);
      break;

    case IMN_OPENCANDIDATE:
      CreateCandWindow(hWnd, lpUIExtra, lpIMC);
      break;

    case IMN_CHANGECANDIDATE:
      ResizeCandWindow(lpUIExtra, lpIMC);
      MoveCandWindow(hWnd, lpIMC, lpUIExtra, FALSE);
      break;

    case IMN_CLOSECANDIDATE:
      HideCandWindow(lpUIExtra);
      break;

    case IMN_GUIDELINE:
      if (ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0)) {
        if (!IsWindow(lpUIExtra->uiGuide.hWnd)) {
          HDC hdcIC;
          TEXTMETRIC tm;
          int dx, dy;

          if (lpUIExtra->uiGuide.pt.x == -1) {
            GetWindowRect(lpIMC->hWnd, &rc);
            lpUIExtra->uiGuide.pt.x = rc.left;
            lpUIExtra->uiGuide.pt.y = rc.bottom;
          }

          hdcIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
          GetTextMetrics(hdcIC, &tm);
          dx = tm.tmAveCharWidth * MAXGLCHAR;
          dy = tm.tmHeight + tm.tmExternalLeading;
          DeleteDC(hdcIC);

          lpUIExtra->uiGuide.hWnd = CreateWindowEx(
              WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME, szGuideClassName,
              NULL, WS_DISABLED | WS_POPUP | WS_BORDER, lpUIExtra->uiGuide.pt.x,
              lpUIExtra->uiGuide.pt.y, dx + 2 * GetSystemMetrics(SM_CXBORDER) +
                                           2 * GetSystemMetrics(SM_CXEDGE),
              dy + GetSystemMetrics(SM_CYSMCAPTION) +
                  2 * GetSystemMetrics(SM_CYBORDER) +
                  2 * GetSystemMetrics(SM_CYEDGE),
              hWnd, NULL, TheIME.m_hInst, NULL);
        }
        ShowWindow(lpUIExtra->uiGuide.hWnd, SW_SHOWNOACTIVATE);
        lpUIExtra->uiGuide.bShow = TRUE;
        SetWindowLongPtr(lpUIExtra->uiGuide.hWnd, FIGWLP_SERVERWND, (LONG_PTR)hWnd);
        UpdateGuideWindow(lpUIExtra);
      }
      break;

    case IMN_SETCANDIDATEPOS:
      // MZ-IME supports only one candidate list.
      if (lParam != 0x01) break;

      MoveCandWindow(hWnd, lpIMC, lpUIExtra, FALSE);
      break;

    case IMN_SETCOMPOSITIONWINDOW:
      MoveCompWindow(lpUIExtra, lpIMC);
      MoveCandWindow(hWnd, lpIMC, lpUIExtra, TRUE);
      break;

    case IMN_SETSTATUSWINDOWPOS:
    case IMN_PRIVATE:
      break;

    default:
      break;
  }

  GlobalUnlock(hUIExtra);
  TheIME.UnlockIMC();

  return lRet;
}

//#define lpcfCandForm ((LPCANDIDATEFORM)lParam)

// Handle WM_IME_CONTROL messages
LONG PASCAL ControlCommand(HIMC hIMC, HWND hWnd, UINT message,
                           WPARAM wParam, LPARAM lParam) {
  LONG lRet = 1L;
  InputContext *lpIMC;
  HGLOBAL hUIExtra;
  LPUIEXTRA lpUIExtra;

  lpIMC = TheIME.LockIMC(hIMC);
  if (NULL == lpIMC) return 1L;

  hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd, IMMGWLP_PRIVATE);
  lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

  switch (wParam) {
  case IMC_GETCANDIDATEPOS:
    if (IsWindow(lpUIExtra->uiCand.hWnd)) {
      // MZ-IME has only one candidate list.
      *(LPCANDIDATEFORM)lParam = lpIMC->cfCandForm[0];
      lRet = 0;
    }
    break;

  case IMC_GETCOMPOSITIONWINDOW:
    *(LPCOMPOSITIONFORM)lParam = lpIMC->cfCompForm;
    lRet = 0;
    break;

  case IMC_GETSTATUSWINDOWPOS:
    lRet = MAKELONG(lpUIExtra->uiStatus.pt.x, lpUIExtra->uiStatus.pt.y);
    break;

  default:
    break;
  }

  GlobalUnlock(hUIExtra);
  TheIME.UnlockIMC();

  return lRet;
}

// When draging the child window, this function draws the border
void PASCAL DrawUIBorder(LPRECT lprc) {
  HDC hDC;
  int sbx, sby;

  hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
  SelectObject(hDC, GetStockObject(GRAY_BRUSH));
  sbx = GetSystemMetrics(SM_CXBORDER);
  sby = GetSystemMetrics(SM_CYBORDER);
  PatBlt(hDC, lprc->left, lprc->top, lprc->right - lprc->left - sbx, sby,
         PATINVERT);
  PatBlt(hDC, lprc->right - sbx, lprc->top, sbx, lprc->bottom - lprc->top - sby,
         PATINVERT);
  PatBlt(hDC, lprc->right, lprc->bottom - sby,
         -(lprc->right - lprc->left - sbx), sby, PATINVERT);
  PatBlt(hDC, lprc->left, lprc->bottom, sbx, -(lprc->bottom - lprc->top - sby),
         PATINVERT);
  DeleteDC(hDC);
}

// Handling mouse messages for the child windows
void PASCAL DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  POINT pt;
  static POINT ptdif;
  static RECT drc;
  static RECT rc;
  DWORD dwT;

  switch (message) {
    case WM_SETCURSOR:
      if (HIWORD(lParam) == WM_LBUTTONDOWN ||
          HIWORD(lParam) == WM_RBUTTONDOWN) {
        GetCursorPos(&pt);
        SetCapture(hWnd);
        GetWindowRect(hWnd, &drc);
        ptdif.x = pt.x - drc.left;
        ptdif.y = pt.y - drc.top;
        rc = drc;
        rc.right -= rc.left;
        rc.bottom -= rc.top;
        SetWindowLong(hWnd, FIGWL_MOUSE, FIM_CAPUTURED);
      }
      break;

    case WM_MOUSEMOVE:
      dwT = GetWindowLong(hWnd, FIGWL_MOUSE);
      if (dwT & FIM_MOVED) {
        DrawUIBorder(&drc);
        GetCursorPos(&pt);
        drc.left = pt.x - ptdif.x;
        drc.top = pt.y - ptdif.y;
        drc.right = drc.left + rc.right;
        drc.bottom = drc.top + rc.bottom;
        DrawUIBorder(&drc);
      } else if (dwT & FIM_CAPUTURED) {
        DrawUIBorder(&drc);
        SetWindowLong(hWnd, FIGWL_MOUSE, dwT | FIM_MOVED);
      }
      break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      dwT = GetWindowLong(hWnd, FIGWL_MOUSE);

      if (dwT & FIM_CAPUTURED) {
        ReleaseCapture();
        if (dwT & FIM_MOVED) {
          DrawUIBorder(&drc);
          GetCursorPos(&pt);
          MoveWindow(hWnd, pt.x - ptdif.x, pt.y - ptdif.y, rc.right, rc.bottom,
                     TRUE);
        }
      }
      break;
  }
}

// Any UI window should not pass the IME messages to DefWindowProc
BOOL PASCAL IsImeMessage(UINT message) {
  switch (message) {
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_COMPOSITION:
    case WM_IME_NOTIFY:
    case WM_IME_SETCONTEXT:
    case WM_IME_CONTROL:
    case WM_IME_COMPOSITIONFULL:
    case WM_IME_SELECT:
    case WM_IME_CHAR:
      return TRUE;
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
