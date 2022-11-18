// ui.cpp --- mzimeja UI server
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

void PASCAL ShowUIWindows(HWND hwndServer, BOOL fFlag) {
    int nsw = (fFlag ? SW_SHOWNOACTIVATE : SW_HIDE);

    UIEXTRA *lpUIExtra = LockUIExtra(hwndServer);
    if (lpUIExtra) {
        if (IsWindow(lpUIExtra->hwndStatus)) {
            ::ShowWindow(lpUIExtra->hwndStatus, nsw);
        }
        if (IsWindow(lpUIExtra->uiCand.hWnd)) {
            ::ShowWindow(lpUIExtra->uiCand.hWnd, nsw);
        }
        if (IsWindow(lpUIExtra->hwndDefComp)) {
            ::ShowWindow(lpUIExtra->hwndDefComp, nsw);
        }
        if (IsWindow(lpUIExtra->hwndGuide)) {
            ::ShowWindow(lpUIExtra->hwndGuide, nsw);
        }
        UnlockUIExtra(hwndServer);
    }
}

void OnImeSetContext(HWND hWnd, HIMC hIMC, LPARAM lParam) {
    UIEXTRA *lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
        // input context was changed.
        lpUIExtra->hIMC = hIMC;

        if (hIMC) {
            // the display have to be updated.
            InputContext *lpIMC = TheIME.LockIMC(hIMC);
            if (lpIMC) {
                CompStr *lpCompStr = lpIMC->LockCompStr();
                CandInfo *lpCandInfo = lpIMC->LockCandInfo();
                if (::IsWindow(lpUIExtra->uiCand.hWnd)) {
                    CandWnd_Hide(lpUIExtra);
                }
                if (lParam & ISC_SHOWUICANDIDATEWINDOW) {
                    if (lpCandInfo->dwCount) {
                        CandWnd_Create(hWnd, lpUIExtra, lpIMC);
                        CandWnd_Resize(lpUIExtra, lpIMC);
                        CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE);
                    }
                }

                if (::IsWindow(lpUIExtra->hwndDefComp)) {
                    CompWnd_Hide(lpUIExtra);
                }
                if (lParam & ISC_SHOWUICOMPOSITIONWINDOW) {
                    if (lpCompStr->dwCompStrLen) {
                        CompWnd_Create(hWnd, lpUIExtra, lpIMC);
                        CompWnd_Move(lpUIExtra, lpIMC);
                    }
                }

                DWORD bIsNonRoman = FALSE;
                if (TheIME.GetUserDword(L"IsNonRoman", &bIsNonRoman)) {
                    if (bIsNonRoman) {
                        lpIMC->Conversion() &= ~IME_CMODE_ROMAN;
                    } else {
                        lpIMC->Conversion() |= IME_CMODE_ROMAN;
                    }
                }

                lpIMC->UnlockCompStr();
                lpIMC->UnlockCandInfo();
                StatusWnd_Update(lpUIExtra);
                TheIME.UnlockIMC(hIMC);
            } else {
                ShowUIWindows(hWnd, FALSE);
            }
        } else { // it is NULL input context.
            ShowUIWindows(hWnd, FALSE);
        }
        UnlockUIExtra(hWnd);
    }
} // OnImeSetContext

void OnDestroy(HWND hWnd) {
    UIEXTRA *lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
        if (::IsWindow(lpUIExtra->hwndStatus))
            ::DestroyWindow(lpUIExtra->hwndStatus);

        if (::IsWindow(lpUIExtra->uiCand.hWnd))
            ::DestroyWindow(lpUIExtra->uiCand.hWnd);

        if (::IsWindow(lpUIExtra->hwndDefComp))
            ::DestroyWindow(lpUIExtra->hwndDefComp);

        for (INT i = 0; i < MAXCOMPWND; i++) {
            if (::IsWindow(lpUIExtra->uiComp[i].hWnd))
                ::DestroyWindow(lpUIExtra->uiComp[i].hWnd);
        }

        if (::IsWindow(lpUIExtra->hwndGuide))
            ::DestroyWindow(lpUIExtra->hwndGuide);

        if (lpUIExtra->hFont) {
            ::DeleteObject(lpUIExtra->hFont);
        }

        UnlockUIExtra(hWnd);
        FreeUIExtra(hWnd);
    }
}

// IME UI server window procedure
LRESULT CALLBACK MZIMEWndProc(HWND hWnd, UINT message, WPARAM wParam,
                              LPARAM lParam) {
    InputContext *lpIMC;
    UIEXTRA *lpUIExtra;
    HGLOBAL hUIExtra;
    LONG lRet = 0;

    HIMC hIMC = (HIMC)GetWindowLongPtr(hWnd, IMMGWLP_IMC);

    // Even if there is no current UI. these messages should not be pass to
    // DefWindowProc().
    if (hIMC == NULL) {
        if (IsImeMessage2(message)) {
            DebugPrintA("Why hIMC is NULL?\n");
            DebugPrintA("hWnd: %x, message: %x, wParam: %x, lParam: %x\n",
                        (LONG)hWnd, message, wParam, lParam);
            return 0;
        }
    }

    switch (message) {
    case WM_CREATE:
        DebugPrintA("WM_CREATE\n");
        // Allocate UI's extra memory block.
        hUIExtra = GlobalAlloc(GHND, sizeof(UIEXTRA));
        lpUIExtra = (UIEXTRA *)GlobalLock(hUIExtra);
        if (lpUIExtra) {
            lpUIExtra->uiCand.pt.x = -1;
            lpUIExtra->uiCand.pt.y = -1;
            lpUIExtra->hFont = NULL;
            GlobalUnlock(hUIExtra);
        }
        SetUIExtraToServerWnd(hWnd, hUIExtra);
        break;

    case WM_IME_SETCONTEXT:
        DebugPrintA("WM_IME_SETCONTEXT\n");
        if (wParam) {
            OnImeSetContext(hWnd, hIMC, lParam);
        }
        // else
        //    ShowUIWindows(hWnd, FALSE);
        break;

    case WM_IME_STARTCOMPOSITION:
        DebugPrintA("WM_IME_STARTCOMPOSITION\n");
        // Start composition! Ready to display the composition string.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            lpIMC = TheIME.LockIMC(hIMC);
            CompWnd_Create(hWnd, lpUIExtra, lpIMC);
            TheIME.UnlockIMC(hIMC);
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_IME_COMPOSITION:
        DebugPrintA("WM_IME_COMPOSITION\n");
        // Update to display the composition string.
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            lpUIExtra = LockUIExtra(hWnd);
            if (lpUIExtra) {
                CompWnd_Move(lpUIExtra, lpIMC);
                CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE);
                UnlockUIExtra(hWnd);
            }
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case WM_IME_ENDCOMPOSITION:
        DebugPrintA("WM_IME_ENDCOMPOSITION\n");
        // Finish to display the composition string.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            CompWnd_Hide(lpUIExtra);
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_IME_COMPOSITIONFULL:
        DebugPrintA("WM_IME_COMPOSITIONFULL\n");
        break;

    case WM_IME_SELECT:
        DebugPrintA("WM_IME_SELECT\n");
        if (wParam) {
            lpUIExtra = LockUIExtra(hWnd);
            if (lpUIExtra) {
                lpUIExtra->hIMC = hIMC;
                UnlockUIExtra(hWnd);
            }
        }
        break;

    case WM_IME_CONTROL:
        DebugPrintA("WM_IME_CONTROL\n");
        lRet = ControlCommand(hIMC, hWnd, wParam, lParam);
        break;

    case WM_IME_NOTIFY:
        DebugPrintA("WM_IME_NOTIFY\n");
        lRet = NotifyCommand(hIMC, hWnd, wParam, lParam);
        break;

    case WM_DESTROY:
        DebugPrintA("WM_DESTROY\n");
        OnDestroy(hWnd);
        break;

    case WM_UI_STATEMOVE:
        DebugPrintA("WM_UI_STATEMOVE\n");
        // Set the position of the status window to UIExtra.
        // This message is sent by the status window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            POINT pt;
            pt.x = (short)LOWORD(lParam);
            pt.y = (short)HIWORD(lParam);
            TheIME.SetUserData(L"ptStatusWindow", &pt, sizeof(pt));
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_UI_DEFCOMPMOVE:
        DebugPrintA("WM_UI_DEFCOMPMOVE\n");
        // Set the position of the composition window to UIExtra.
        // This message is sent by the composition window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            if (!lpUIExtra->dwCompStyle) {
                POINT pt;
                pt.x = (short)LOWORD(lParam);
                pt.y = (short)HIWORD(lParam);
                TheIME.SetUserData(L"ptDefComp", &pt, sizeof(pt));
            }
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_UI_CANDMOVE:
        DebugPrintA("WM_UI_CANDMOVE\n");
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
        DebugPrintA("WM_UI_GUIDEMOVE\n");
        // Set the position of the status window to UIExtra.
        // This message is sent by the status window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            POINT pt;
            pt.x = (short)LOWORD(lParam);
            pt.y = (short)HIWORD(lParam);
            TheIME.SetUserData(L"ptGuide", &pt, sizeof(pt));
            UnlockUIExtra(hWnd);
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return lRet;
}

int GetCompFontHeight(UIEXTRA *lpUIExtra) {
    HDC hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
    HFONT hOldFont = NULL;
    if (lpUIExtra->hFont) hOldFont = (HFONT)SelectObject(hIC, lpUIExtra->hFont);
    SIZE siz;
    GetTextExtentPoint(hIC, TEXT("A"), 1, &siz);
    if (hOldFont) SelectObject(hIC, hOldFont);
    DeleteDC(hIC);
    return siz.cy;
}

// Handle WM_IME_NOTIFY messages
LONG NotifyCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam) {
    LONG ret = 0;
    RECT rc;
    LOGFONT lf;
    InputContext *lpIMC;

    UIEXTRA *lpUIExtra = LockUIExtra(hWnd);

    switch (wParam) {
    case IMN_CLOSESTATUSWINDOW:
        DebugPrintA("IMN_CLOSESTATUSWINDOW\n");
        if (::IsWindow(lpUIExtra->hwndStatus)) {
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            POINT pt;
            pt.x = rc.left;
            pt.y = rc.top;
            TheIME.SetUserData(L"ptStatusWindow", &pt, sizeof(pt));
            ::ShowWindow(lpUIExtra->hwndStatus, SW_HIDE);
        }
        break;

    case IMN_OPENSTATUSWINDOW:
        DebugPrintA("IMN_OPENSTATUSWINDOW\n");
        StatusWnd_Create(hWnd, lpUIExtra);
        break;

    case IMN_SETCONVERSIONMODE:
        DebugPrintA("IMN_SETCONVERSIONMODE\n");
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            if (lpIMC->Conversion() & IME_CMODE_ROMAN) {
                TheIME.SetUserDword(L"IsNonRoman", FALSE);
            } else {
                TheIME.SetUserDword(L"IsNonRoman", TRUE);
            }
            TheIME.UnlockIMC(hIMC);
        }
        StatusWnd_Update(lpUIExtra);
        break;

    case IMN_SETSENTENCEMODE:
        DebugPrintA("IMN_SETSENTENCEMODE\n");
        break;

    case IMN_SETCOMPOSITIONFONT:
        DebugPrintA("IMN_SETCOMPOSITIONFONT\n");
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
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
            CompWnd_Move(lpUIExtra, lpIMC);

            TheIME.UnlockIMC(hIMC);
        }
        break;

    case IMN_SETOPENSTATUS:
        DebugPrintA("IMN_SETOPENSTATUS\n");
        StatusWnd_Update(lpUIExtra);
        break;

    case IMN_OPENCANDIDATE:
        DebugPrintA("IMN_OPENCANDIDATE\n");
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            CandWnd_Create(hWnd, lpUIExtra, lpIMC);
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case IMN_CHANGECANDIDATE:
        DebugPrintA("IMN_CHANGECANDIDATE\n");
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            CandWnd_Resize(lpUIExtra, lpIMC);
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE);
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case IMN_CLOSECANDIDATE:
        DebugPrintA("IMN_CLOSECANDIDATE\n");
        CandWnd_Hide(lpUIExtra);
        break;

    case IMN_GUIDELINE:
        DebugPrintA("IMN_GUIDELINE\n");
        if (::ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0)) {
            if (!::IsWindow(lpUIExtra->hwndGuide)) {
                HDC hdcIC;
                TEXTMETRIC tm;
                int dx, dy;
                POINT pt;

                lpIMC = TheIME.LockIMC(hIMC);
                if (lpIMC) {
                    if (!TheIME.GetUserData(L"ptGuide", &pt, sizeof(pt))) {
                        ::GetWindowRect(lpIMC->hWnd, &rc);
                        pt.x = rc.left;
                        pt.y = rc.bottom;
                    }
                    TheIME.UnlockIMC(hIMC);
                }

                hdcIC = ::CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
                ::GetTextMetrics(hdcIC, &tm);
                dx = tm.tmAveCharWidth * MAXGLCHAR;
                dy = tm.tmHeight + tm.tmExternalLeading;
                ::DeleteDC(hdcIC);

                lpUIExtra->hwndGuide = ::CreateWindowEx(
                        WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME, szGuideClassName,
                        NULL, WS_DISABLED | WS_POPUP | WS_BORDER, pt.x, pt.y,
                        dx + 2 * GetSystemMetrics(SM_CXBORDER) + 2 * GetSystemMetrics(SM_CXEDGE),
                        dy + GetSystemMetrics(SM_CYSMCAPTION) + 2 * GetSystemMetrics(SM_CYBORDER) +
                        2 * GetSystemMetrics(SM_CYEDGE),
                        hWnd, NULL, TheIME.m_hInst, NULL);
            }
            ::ShowWindow(lpUIExtra->hwndGuide, SW_SHOWNOACTIVATE);
            ::SetWindowLongPtr(lpUIExtra->hwndGuide, FIGWLP_SERVERWND, (LONG_PTR)hWnd);
            GuideWnd_Update(lpUIExtra);
        }
        break;

    case IMN_SETCANDIDATEPOS:
        DebugPrintA("IMN_SETCANDIDATEPOS\n");
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE);
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case IMN_SETCOMPOSITIONWINDOW:
        DebugPrintA("IMN_SETCOMPOSITIONWINDOW\n");
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            CompWnd_Move(lpUIExtra, lpIMC);
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE);
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case IMN_SETSTATUSWINDOWPOS:
        DebugPrintA("IMN_SETSTATUSWINDOWPOS\n");
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            POINT pt = lpIMC->ptStatusWndPos;
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            ::MoveWindow(lpUIExtra->hwndStatus, pt.x, pt.y,
                         rc.right - rc.left, rc.bottom - rc.top, TRUE);
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case IMN_PRIVATE:
        DebugPrintA("IMN_PRIVATE\n");
        if (HIWORD(lParam) == 0xFACE) {
            std::wstring imepad_file;
            if (TheIME.GetComputerString(L"ImePadFile", imepad_file)) {
                ::ShellExecuteW(NULL, NULL, imepad_file.c_str(),
                                NULL, NULL, SW_SHOWNOACTIVATE);
            }
            break;
        }
        if (HIWORD(lParam) == 0xDEAD) {
            std::wstring verinfo_file;
            if (TheIME.GetComputerString(L"VerInfoFile", verinfo_file)) {
                ::ShellExecuteW(NULL, NULL, verinfo_file.c_str(),
                                NULL, NULL, SW_SHOWNOACTIVATE);
            }
            break;
        }
        break;

    default:
        break;
    }

    UnlockUIExtra(hWnd);

    return ret;
}

//#define lpcfCandForm ((LPCANDIDATEFORM)lParam)

// Handle WM_IME_CONTROL messages
LONG ControlCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam) {
    LONG ret = 1;

    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (NULL == lpIMC) return ret;

    UIEXTRA *lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
        switch (wParam) {
        case IMC_GETCANDIDATEPOS:
            DebugPrintA("IMC_GETCANDIDATEPOS\n");
            if (IsWindow(lpUIExtra->uiCand.hWnd)) {
                *(LPCANDIDATEFORM)lParam = lpIMC->cfCandForm[0];
                ret = 0;
            }
            break;

        case IMC_GETCOMPOSITIONWINDOW:
            DebugPrintA("IMC_GETCOMPOSITIONWINDOW\n");
            *(LPCOMPOSITIONFORM)lParam = lpIMC->cfCompForm;
            ret = 0;
            break;

        case IMC_GETSTATUSWINDOWPOS:
            DebugPrintA("IMC_GETSTATUSWINDOWPOS\n");
            {
                RECT rc;
                ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
                ret = MAKELONG(rc.left, rc.top);
            }
            break;

        default:
            break;
        }
        UnlockUIExtra(hWnd);
    }
    TheIME.UnlockIMC(hIMC);

    return ret;
}

// When draging the child window, this function draws the border
void DrawUIBorder(LPRECT lprc) {
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

BOOL IsImeMessage2(UINT message) {
    switch (message) {
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_COMPOSITION:
    //case WM_IME_NOTIFY:
    //case WM_IME_SETCONTEXT:
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
