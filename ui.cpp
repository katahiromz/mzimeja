// ui.cpp --- mzimeja UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

void PASCAL ShowUIWindows(HWND hWnd, BOOL fFlag) {
  FOOTMARK();
  LPUIEXTRA lpUIExtra = LockUIExtra(hWnd);
  if (lpUIExtra) {
    StatusWnd_Show(lpUIExtra, fFlag);
    CandWnd_Show(lpUIExtra, fFlag);
    CompWnd_Show(lpUIExtra, -2, fFlag);
    GuideWnd_Show(lpUIExtra, fFlag);
    UnlockUIExtra(hWnd);
  }
}

#ifdef _DEBUG
void PASCAL DumpUIExtra(LPUIEXTRA lpUIExtra) {
  FOOTMARK();
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

// IME UI server window procedure
LRESULT CALLBACK MZIMEWndProc(HWND hWnd, UINT message, WPARAM wParam,
                              LPARAM lParam) {
  FOOTMARK();
  InputContext *lpIMC;
  LPUIEXTRA lpUIExtra;
  HGLOBAL hUIExtra;
  LONG lRet = 0L;
  int i;
  DebugPrintA("MZIMEWndProc: ThreadID: %08X\n", ::GetCurrentThreadId());

  HIMC hIMC = (HIMC)GetWindowLongPtr(hWnd, IMMGWLP_IMC);

  // Even if there is no current UI. these messages should not be pass to
  // DefWindowProc().
  if (hIMC == NULL) {
    if (IsImeMessage(message)) {
      DebugPrint(TEXT("Why hIMC is NULL????\n"));
      DebugPrint(TEXT("\thWnd is %x\n"), hWnd);
      DebugPrint(TEXT("\tmessage is %x\n"), message);
      DebugPrint(TEXT("\twParam is %x\n"), wParam);
      DebugPrint(TEXT("\tlParam is %x\n"), lParam);
      return 0;
    }
  }

  switch (message) {
  case WM_CREATE:
    DebugPrint(TEXT("WM_CREATE\n"));
    // Allocate UI's extra memory block.
    hUIExtra = GlobalAlloc(GHND, sizeof(UIEXTRA));
    lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
    if (lpUIExtra) {
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
    }
    SetUIExtraToServerWnd(hWnd, hUIExtra);
    break;

  case WM_IME_SETCONTEXT:
    DebugPrint(TEXT("WM_IME_SETCONTEXT\n"));
    if (wParam) {
      lpUIExtra = LockUIExtra(hWnd);
      if (lpUIExtra) {
        lpUIExtra->hIMC = hIMC;

        if (hIMC) {
          lpIMC = TheIME.LockIMC(hIMC);
          if (lpIMC) {
            if (IsWindow(lpUIExtra->uiCand.hWnd)) CandWnd_Show(lpUIExtra, FALSE);
            if (lParam & ISC_SHOWUICANDIDATEWINDOW) {
              if (lpIMC->HasCandInfo()) {
                CandWnd_Create(hWnd, lpUIExtra, lpIMC);
                CandWnd_Resize(lpUIExtra, lpIMC);
                CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE);
              }
            }
            if (IsWindow(lpUIExtra->uiDefComp.hWnd)) {
              CompWnd_Show(lpUIExtra, -2, FALSE);
            }
            if (lParam & ISC_SHOWUICOMPOSITIONWINDOW) {
              if (lpIMC->HasCompStr()) {
                CompWnd_MoveShowMessage(hWnd, lpUIExtra);
              }
            }
          } else {
            CandWnd_Show(lpUIExtra, FALSE);
            CompWnd_Show(lpUIExtra, -1, FALSE);
          }
          StatusWnd_Update(lpUIExtra);
          TheIME.UnlockIMC(hIMC);
        } else {
          CandWnd_Show(lpUIExtra, FALSE);
          CompWnd_Show(lpUIExtra, -2, FALSE);
        }
        UnlockUIExtra(hWnd);
      }
    }
    // else
    //    ShowUIWindows(hWnd, FALSE);
    break;

  case WM_IME_STARTCOMPOSITION:
    DebugPrint(TEXT("WM_IME_STARTCOMPOSITION\n"));
    // Start composition! Ready to display the composition string.
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        CompWnd_Create(hWnd, lpUIExtra, lpIMC);
        TheIME.UnlockIMC(hIMC);
      }
      UnlockUIExtra(hWnd);
    }
    break;

  case WM_IME_COMPOSITION:
    DebugPrint(TEXT("WM_IME_COMPOSITION\n"));
    // Update to display the composition string.
    lpIMC = TheIME.LockIMC(hIMC);
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
      CompWnd_MoveShowMessage(hWnd, lpUIExtra);
      CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE);
      UnlockUIExtra(hWnd);
    }
    TheIME.UnlockIMC(hIMC);
    break;

  case WM_IME_ENDCOMPOSITION:
    DebugPrint(TEXT("WM_IME_ENDCOMPOSITION\n"));
    // Finish to display the composition string.
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
      CompWnd_Show(lpUIExtra, -2, FALSE);
      UnlockUIExtra(hWnd);
    }
    break;

  case WM_IME_COMPOSITIONFULL:
    DebugPrint(TEXT("WM_IME_COMPOSITIONFULL\n"));
    break;

  case WM_IME_SELECT:
    DebugPrint(TEXT("WM_IME_SELECT\n"));
    if (wParam) {
      lpUIExtra = LockUIExtra(hWnd);
      if (lpUIExtra) {
        lpUIExtra->hIMC = hIMC;
        UnlockUIExtra(hWnd);
      }
    }
    break;

  case WM_IME_CONTROL:
    DebugPrint(TEXT("WM_IME_CONTROL\n"));
    lRet = ControlCommand(hIMC, hWnd, message, wParam, lParam);
    break;

  case WM_IME_NOTIFY:
    DebugPrint(TEXT("WM_IME_NOTIFY\n"));
    lRet = NotifyCommand(hIMC, hWnd, message, wParam, lParam);
    break;

  case WM_DESTROY:
    DebugPrint(TEXT("WM_DESTROY\n"));
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
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

      UnlockUIExtra(hWnd);
      FreeUIExtra(hWnd);
    }
    break;

  case WM_UI_STATEMOVE:
    DebugPrint(TEXT("WM_UI_STATEMOVE\n"));
    // Set the position of the status window to UIExtra.
    // This message is sent by the status window.
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
      lpUIExtra->uiStatus.pt.x = (short)LOWORD(lParam);
      lpUIExtra->uiStatus.pt.y = (short)HIWORD(lParam);
      UnlockUIExtra(hWnd);
    }
    break;

  case WM_UI_DEFCOMPMOVE:
    DebugPrint(TEXT("WM_UI_DEFCOMPMOVE\n"));
    // Set the position of the composition window to UIExtra.
    // This message is sent by the composition window.
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
      if (!lpUIExtra->dwCompStyle) {
        lpUIExtra->uiDefComp.pt.x = (short)LOWORD(lParam);
        lpUIExtra->uiDefComp.pt.y = (short)HIWORD(lParam);
      }
      UnlockUIExtra(hWnd);
    }
    break;

  case WM_UI_CANDMOVE:
    DebugPrint(TEXT("WM_UI_CANDMOVE\n"));
    // Set the position of the candidate window to UIExtra.
    // This message is sent by the candidate window.
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
      lpUIExtra->uiCand.pt.x = (short)LOWORD(lParam);
      lpUIExtra->uiCand.pt.y = (short)HIWORD(lParam);
      UnlockUIExtra(hWnd);
    }
    break;

  case WM_UI_GUIDEMOVE:
    DebugPrint(TEXT("WM_UI_GUIDEMOVE\n"));
    // Set the position of the status window to UIExtra.
    // This message is sent by the status window.
    lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
      lpUIExtra->uiGuide.pt.x = (short)LOWORD(lParam);
      lpUIExtra->uiGuide.pt.y = (short)HIWORD(lParam);
      UnlockUIExtra(hWnd);
    }
    break;

  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return lRet;
}

int GetCompFontHeight(LPUIEXTRA lpUIExtra) {
  FOOTMARK();
  HDC hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
  HFONT hOldFont = NULL;
  if (lpUIExtra->hFont) hOldFont = (HFONT)SelectObject(hIC, lpUIExtra->hFont);
  SIZE siz;
  MyGetTextExtentPoint(hIC, TEXT("A"), 1, &siz);
  if (hOldFont) SelectObject(hIC, hOldFont);
  DeleteDC(hIC);
  return siz.cy;
}

// Handle WM_IME_NOTIFY messages
LONG NotifyCommand(HIMC hIMC, HWND hWnd, UINT message, WPARAM wParam,
                   LPARAM lParam) {
  FOOTMARK();
  LONG lRet = 0L;
  LPUIEXTRA lpUIExtra;
  RECT rc;
  LOGFONT lf;

  InputContext *lpIMC = TheIME.LockIMC(hIMC);
  lpUIExtra = LockUIExtra(hWnd);

  switch (wParam) {
  case IMN_CLOSESTATUSWINDOW:
    DebugPrintA("IMN_CLOSESTATUSWINDOW\n");
    if (IsWindow(lpUIExtra->uiStatus.hWnd)) {
      ::GetWindowRect(lpUIExtra->uiStatus.hWnd, &rc);
      lpUIExtra->uiStatus.pt.x = rc.left;
      lpUIExtra->uiStatus.pt.y = rc.top;
      StatusWnd_Show(lpUIExtra, FALSE);
    }
    break;

  case IMN_OPENSTATUSWINDOW:
    DebugPrintA("IMN_OPENSTATUSWINDOW\n");
    if (lpUIExtra) {
      StatusWnd_Create(hWnd, lpUIExtra);
    }
    break;

  case IMN_SETCONVERSIONMODE:
    DebugPrintA("IMN_SETCONVERSIONMODE\n");
    if (lpUIExtra) {
      StatusWnd_Update(lpUIExtra);
    }
    break;

  case IMN_SETSENTENCEMODE:
    DebugPrintA("IMN_SETSENTENCEMODE\n");
    break;

  case IMN_SETCOMPOSITIONFONT:
    DebugPrintA("IMN_SETCOMPOSITIONFONT\n");
    if (lpUIExtra && lpIMC) {
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
      if (lf.lfCharSet != SHIFTJIS_CHARSET) {
        lf.lfCharSet = SHIFTJIS_CHARSET;
        lf.lfFaceName[0] = 0;
      }

      lpUIExtra->hFont = CreateFontIndirect(&lf);
      CompWnd_SetFont(lpUIExtra);
      CompWnd_MoveShowMessage(hWnd, lpUIExtra);
    }
    break;

  case IMN_SETOPENSTATUS:
    DebugPrintA("IMN_SETOPENSTATUS\n");
    if (lpUIExtra) {
      StatusWnd_Update(lpUIExtra);
    }
    break;

  case IMN_OPENCANDIDATE:
    DebugPrintA("IMN_OPENCANDIDATE\n");
    if (lpUIExtra && lpIMC) {
      CandWnd_Create(hWnd, lpUIExtra, lpIMC);
    }
    break;

  case IMN_CHANGECANDIDATE:
    DebugPrintA("IMN_CHANGECANDIDATE\n");
    if (lpUIExtra && lpIMC) {
      CandWnd_Resize(lpUIExtra, lpIMC);
      CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE);
    }
    break;

  case IMN_CLOSECANDIDATE:
    DebugPrintA("IMN_CLOSECANDIDATE\n");
    if (lpUIExtra) {
      CandWnd_Show(lpUIExtra, FALSE);
    }
    break;

  case IMN_GUIDELINE:
    DebugPrintA("IMN_GUIDELINE\n");
    if (lpIMC && ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0)) {
      if (!IsWindow(lpUIExtra->uiGuide.hWnd)) {
        HDC hdcIC;
        TEXTMETRIC tm;
        int dx, dy;

        if (lpUIExtra->uiGuide.pt.x == -1) {
          ::GetWindowRect(lpIMC->hWnd, &rc);
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
      GuideWnd_Show(lpUIExtra, TRUE);
      SetWindowLongPtr(lpUIExtra->uiGuide.hWnd, FIGWLP_SERVERWND, (LONG_PTR)hWnd);
      GuideWnd_Update(lpUIExtra);
    }
    break;

  case IMN_SETCANDIDATEPOS:
    DebugPrintA("IMN_SETCANDIDATEPOS\n");
    if (lpIMC && lpUIExtra) {
      // MZ-IME supports only one candidate list.
      if (lParam != 0x01) break;

      CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE);
    }
    break;

  case IMN_SETCOMPOSITIONWINDOW:
    DebugPrintA("IMN_SETCOMPOSITIONWINDOW\n");
    if (lpIMC && lpUIExtra) {
      CompWnd_MoveShowMessage(hWnd, lpUIExtra);
      CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE);
    }
    break;

  case IMN_SETSTATUSWINDOWPOS:
    DebugPrintA("IMN_SETSTATUSWINDOWPOS\n");
    break;

  case IMN_PRIVATE:
    DebugPrintA("IMN_PRIVATE\n");
    break;

  default:
    DebugPrintA("IMN_(unknown)\n");
    break;
  }

  UnlockUIExtra(hWnd);
  TheIME.UnlockIMC(hIMC);

  return lRet;
}

//#define lpcfCandForm ((LPCANDIDATEFORM)lParam)

// Handle WM_IME_CONTROL messages
LONG ControlCommand(HIMC hIMC, HWND hWnd, UINT message, WPARAM wParam,
                    LPARAM lParam) {
  FOOTMARK();
  LONG lRet = 1L;
  InputContext *lpIMC;
  LPUIEXTRA lpUIExtra;

  lpIMC = TheIME.LockIMC(hIMC);
  if (NULL == lpIMC) return 1L;

  lpUIExtra = LockUIExtra(hWnd);

  switch (wParam) {
  case IMC_GETCANDIDATEPOS:
    DebugPrintA("IMC_GETCANDIDATEPOS\n");
    if (IsWindow(lpUIExtra->uiCand.hWnd)) {
      // MZ-IME has only one candidate list.
      *(LPCANDIDATEFORM)lParam = lpIMC->cfCandForm[0];
      lRet = 0;
    }
    break;

  case IMC_GETCOMPOSITIONWINDOW:
    DebugPrintA("IMC_GETCOMPOSITIONWINDOW\n");
    *(LPCOMPOSITIONFORM)lParam = lpIMC->cfCompForm;
    lRet = 0;
    break;

  case IMC_GETSTATUSWINDOWPOS:
    DebugPrintA("IMC_GETSTATUSWINDOWPOS\n");
    lRet = MAKELONG(lpUIExtra->uiStatus.pt.x, lpUIExtra->uiStatus.pt.y);
    break;

  default:
    DebugPrintA("IMC_(unknown)\n");
    break;
  }

  UnlockUIExtra(hWnd);
  TheIME.UnlockIMC(hIMC);

  return lRet;
}

// When draging the child window, this function draws the border
void DrawUIBorder(LPRECT lprc) {
  FOOTMARK();
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
void DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  FOOTMARK();
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
BOOL IsImeMessage(UINT message) {
  FOOTMARK();
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
