// uicand.cpp --- mzimeja candidate window UI
// 候補ウィンドウ。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

// 寸法。
#define CX_HEADER 32
#define CX_BORDER ::GetSystemMetrics(SM_CXBORDER)
#define CY_BORDER ::GetSystemMetrics(SM_CYBORDER)

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// 候補ウィンドウの当たり判定。
DWORD CandWnd_HitTest(HWND hWnd, POINT pt, InputContext *lpIMC)
{
    DWORD ret = 0;
    int height = 0;
    HDC hDC = ::CreateCompatibleDC(NULL);
    ASSERT(hDC != NULL);
    HFONT hOldFont = CheckNativeCharset(hDC);
    CandInfo *lpCandInfo = lpIMC->LockCandInfo(); // 候補情報をロックする。
    ASSERT(lpCandInfo != NULL);
    if (lpCandInfo) {
        if (lpCandInfo->dwCount > 0) { // 候補があれば
            CANDINFOEXTRA *pExtra = lpCandInfo->GetExtra(); // 余剰情報を取得。
            DWORD iList = 0;
            if (pExtra) iList = pExtra->iClause;
            CandList *lpCandList = lpCandInfo->GetList(iList);
            DWORD i, end = lpCandList->GetPageEnd();
            for (i = lpCandList->dwPageStart; i < end; ++i) {
                WCHAR *psz = lpCandList->GetCandString(i);
                SIZE siz;
                ::GetTextExtentPoint32W(hDC, psz, ::lstrlenW(psz), &siz);
                INT cy = siz.cy + CY_BORDER * 2;
                if (height <= pt.y && pt.y < height + cy) {
                    ret = i;
                    break;
                }
                height += cy;
            }
        }
        lpIMC->UnlockCandInfo();
    }
    if (hOldFont) {
        ::DeleteObject(::SelectObject(hDC, hOldFont));
    }
    ::DeleteDC(hDC);
    return ret;
}

void CandWnd_OnClick(HWND hWnd)
{
    POINT pt;
    ::GetCursorPos(&pt);
    ::ScreenToClient(hWnd, &pt);

    DWORD dwSelection = 0xFFFFFFFF;
    HWND hSvrWnd = (HWND) ::GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    HIMC hIMC = (HIMC) ::GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC);
    if (hIMC) {
        InputContext *lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            dwSelection = CandWnd_HitTest(hWnd, pt, lpIMC);
            lpIMC->SelectCand(dwSelection);
            TheIME.UnlockIMC(hIMC);
        }
    }

    if (dwSelection != 0xFFFFFFFF) {
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // CandWnd_OnClick

LRESULT CALLBACK CandWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                    LPARAM lParam)
{
    HWND hUIWnd;

    switch (message) {
    case WM_ERASEBKGND:
        return TRUE; // ちらつきを防止するため、ここで背景を描画しない。

    case WM_PAINT:
        CandWnd_Paint(hWnd);
        break;

    case WM_SETCURSOR:
        if (HIWORD(lParam) == WM_LBUTTONUP) {
            CandWnd_OnClick(hWnd);
        }
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
        break;

    case WM_MOVE:
        hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (::IsWindow(hUIWnd)) ::SendMessage(hUIWnd, WM_UI_CANDMOVE, 0, 0);
        break;

    default:
        if (!IsImeMessage(message))
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
} // CandWnd_WindowProc

BOOL GetCandPosFromCompWnd(InputContext *lpIMC, UIEXTRA *lpUIExtra, LPPOINT lppt)
{
    BOOL ret = FALSE;
    FOOTMARK_FORMAT("%p, %p, %p\n", lpIMC, lpUIExtra, lppt);

    DWORD iClause = 0;
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
        if (lpCandInfo->dwCount > 0) ret = TRUE;
        CANDINFOEXTRA *pExtra = lpCandInfo->GetExtra();
        if (pExtra) {
            iClause = pExtra->iClause;
        }
        lpIMC->UnlockCandInfo();

        if (ret) {
            HWND hCompWnd = GetCandPosHintFromComp(lpUIExtra, lpIMC, iClause, lppt);
            ret = ::IsWindowVisible(hCompWnd);
        }
    }

    return ret;
}

BOOL GetCandPosFromCompForm(InputContext *lpIMC, UIEXTRA *lpUIExtra,
                            LPPOINT lppt)
{
    FOOTMARK_FORMAT("%p, %p, %p\n", lpIMC, lpUIExtra, lppt);
    if (GetCandPosFromCompWnd(lpIMC, lpUIExtra, lppt)) {
        ::ScreenToClient(lpIMC->hWnd, lppt);
        return TRUE;
    }
    return FALSE;
} // GetCandPosFromCompForm

void CandWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC)
{
    POINT pt;
    FOOTMARK_FORMAT("%p, %p, %p\n", hUIWnd, lpUIExtra, lpIMC);

    if (GetCandPosFromCompWnd(lpIMC, lpUIExtra, &pt)) {
        lpUIExtra->ptCand.x = pt.x;
        lpUIExtra->ptCand.y = pt.y;
    }

    if (!::IsWindow(lpUIExtra->hwndCand)) {
        lpUIExtra->hwndCand =
                ::CreateWindowEx(WS_EX_WINDOWEDGE, szCandClassName, NULL,
                                 WS_COMPDEFAULT | WS_DLGFRAME,
                                 lpUIExtra->ptCand.x, lpUIExtra->ptCand.y,
                                 1, 1, hUIWnd, NULL, TheIME.m_hInst, NULL);
    }

    ::SetWindowLongPtr(lpUIExtra->hwndCand, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
    ::ShowWindow(lpUIExtra->hwndCand, SW_HIDE);
} // CandWnd_Create

// WM_PAINT
void CandWnd_Paint(HWND hCandWnd)
{
    RECT rc;
    ::GetClientRect(hCandWnd, &rc);

    PAINTSTRUCT ps;
    HDC hDC = ::BeginPaint(hCandWnd, &ps);

    // ちらつきを防止するため、メモリービットマップを使用する。
    HDC hdcMem = ::CreateCompatibleDC(hDC);
    HBITMAP hbm = ::CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
    HGDIOBJ hbmOld = ::SelectObject(hdcMem, hbm);

    // 背景を塗りつぶす。
    ::FillRect(hdcMem, &rc, (HBRUSH)(COLOR_WINDOW + 1));

    ::SetBkMode(hdcMem, TRANSPARENT);
    HWND hSvrWnd = (HWND) ::GetWindowLongPtr(hCandWnd, FIGWLP_SERVERWND);

    HIMC hIMC = (HIMC) ::GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC);
    if (hIMC) {
        InputContext *lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            HFONT hOldFont = CheckNativeCharset(hdcMem);
            CandInfo *lpCandInfo = lpIMC->LockCandInfo();
            if (lpCandInfo) {
                INT x1 = ::GetSystemMetrics(SM_CXEDGE);
                INT x2 = ::GetSystemMetrics(SM_CXEDGE) + CX_HEADER + CX_BORDER * 2;
                INT y = ::GetSystemMetrics(SM_CYEDGE);
                CANDINFOEXTRA *pExtra = lpCandInfo->GetExtra();
                DWORD iList = 0;
                if (pExtra) iList = pExtra->iClause;
                CandList *lpCandList = lpCandInfo->GetList(iList);
                WCHAR sz[4];
                DWORD i, k = 1, end = lpCandList->GetPageEnd();
                for (i = lpCandList->dwPageStart; i < end; ++i, ++k) {
                    // get size of cand string
                    WCHAR *psz = lpCandList->GetCandString(i);
                    SIZE siz;
                    ::GetTextExtentPoint32W(hdcMem, psz, lstrlenW(psz), &siz);

                    // draw header
                    RECT rcHeader;
                    ::SetRect(&rcHeader, x1, y + CY_BORDER,
                              x1 + CX_HEADER, y + siz.cy + CY_BORDER * 2);
                    ::DrawFrameControl(hdcMem, &rcHeader, DFC_BUTTON,
                                       DFCS_BUTTONPUSH | DFCS_ADJUSTRECT);
                    StringCchPrintf(sz, _countof(sz), TEXT("%u"), k);
                    ::SetTextColor(hdcMem, ::GetSysColor(COLOR_BTNTEXT));
                    ::InflateRect(&rcHeader, -::GetSystemMetrics(SM_CXBORDER), 0);
                    ::DrawTextW(hdcMem, sz, -1, &rcHeader,
                                DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX);

                    // draw text
                    RECT rcText;
                    ::SetRect(&rcText, x2, y + CY_BORDER,
                              rc.right, y + siz.cy + CY_BORDER * 2);
                    ::InflateRect(&rcHeader, -CX_BORDER, -CY_BORDER);
                    if (lpCandList->dwSelection == i) {
                        ::SetTextColor(hdcMem, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
                        ::FillRect(hdcMem, &rcText, (HBRUSH)(COLOR_HIGHLIGHT + 1));
                    } else {
                        ::SetTextColor(hdcMem, ::GetSysColor(COLOR_WINDOWTEXT));
                        ::FillRect(hdcMem, &rcText, (HBRUSH)(COLOR_WINDOW + 1));
                    }
                    ::DrawTextW(hdcMem, psz, -1, &rcText,
                                DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX);

                    // go to next line
                    y += siz.cy + CY_BORDER * 2;
                }
                lpIMC->UnlockCandInfo();
            }
            if (hOldFont) {
                ::DeleteObject(SelectObject(hdcMem, hOldFont));
            }
            TheIME.UnlockIMC(hIMC);
        }
    }

    // ビット群を転送する（hDC ← hdcMem）。
    ::BitBlt(hDC, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

    // 後始末。
    ::DeleteObject(::SelectObject(hdcMem, hbmOld));
    ::DeleteDC(hdcMem);

    // 描画を終了する。
    ::EndPaint(hCandWnd, &ps);
} // CandWnd_Paint

SIZE CandWnd_CalcSize(UIEXTRA *lpUIExtra, InputContext *lpIMC)
{
    int width1 = 0, height = 0;
    HDC hDC = ::CreateCompatibleDC(NULL);
    HFONT hOldFont = CheckNativeCharset(hDC);
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
        if (lpCandInfo->dwCount > 0) {
            CANDINFOEXTRA *pExtra = lpCandInfo->GetExtra();
            DWORD iList = 0;
            if (pExtra) iList = pExtra->iClause;
            CandList *lpCandList = lpCandInfo->GetList(iList);
            DWORD i, end = lpCandList->GetPageEnd();
            for (i = lpCandList->dwPageStart; i < end; ++i) {
                WCHAR *psz = lpCandList->GetCandString(i);
                SIZE siz;
                ::GetTextExtentPoint32W(hDC, psz, ::lstrlenW(psz), &siz);
                if (width1 < siz.cx) width1 = siz.cx;
                height += siz.cy + CY_BORDER * 2;
            }
        } else {
            FOOTMARK_POINT();
            lpCandInfo->Dump();
            ASSERT(0);
        }
        lpIMC->UnlockCandInfo();
    }
    if (hOldFont) {
        ::DeleteObject(::SelectObject(hDC, hOldFont));
    }
    ::DeleteDC(hDC);
    SIZE ret;
    ret.cx = width1 + CX_HEADER + CX_BORDER * 4;
    ret.cy = height + CY_BORDER * 2;
    return ret;
} // CandWnd_CalcSize

void CandWnd_Resize(UIEXTRA *lpUIExtra, InputContext *lpIMC)
{
    if (::IsWindow(lpUIExtra->hwndCand)) {
        SIZE siz = CandWnd_CalcSize(lpUIExtra, lpIMC);
        siz.cx += 4 * GetSystemMetrics(SM_CXEDGE);
        siz.cy += 4 * GetSystemMetrics(SM_CYEDGE);

        RECT rc;
        ::GetWindowRect(lpUIExtra->hwndCand, &rc);
        DPRINT("%d, %d, %d, %d\n", rc.left, rc.top, siz.cx, siz.cy);
        ::MoveWindow(lpUIExtra->hwndCand, rc.left, rc.top, siz.cx, siz.cy, TRUE);
    }
} // CandWnd_Resize

void CandWnd_Hide(UIEXTRA *lpUIExtra)
{
    RECT rc;
    FOOTMARK_FORMAT("%p\n", lpUIExtra);

    if (::IsWindow(lpUIExtra->hwndCand)) {
        ::GetWindowRect(lpUIExtra->hwndCand, (LPRECT)&rc);
        lpUIExtra->ptCand.x = rc.left;
        lpUIExtra->ptCand.y = rc.top;
        DPRINT("%d, %d, %d, %d\n", -1, -1, 0, 0);
        ::MoveWindow(lpUIExtra->hwndCand, -1, -1, 0, 0, TRUE);
        ::ShowWindow(lpUIExtra->hwndCand, SW_HIDE);
    }
} // CandWnd_Hide

// 候補ウィンドウの移動時。
void CandWnd_Move(HWND hUIWnd, InputContext *lpIMC, UIEXTRA *lpUIExtra,
                  BOOL fForceComp)
{
    RECT rc;
    POINT pt;
    FOOTMARK_FORMAT("%p, %p, %p, %d\n", hUIWnd, lpIMC, lpUIExtra, fForceComp);

    // Not initialized yet? 初期化されてないか？
    if (lpIMC->cfCandForm[0].dwIndex == (DWORD)-1) {
        lpIMC->DumpCandInfo();
        if (GetCandPosFromCompWnd(lpIMC, lpUIExtra, &pt)) {
            lpUIExtra->ptCand.x = pt.x;
            lpUIExtra->ptCand.y = pt.y;
            HWND hwndCand = lpUIExtra->hwndCand;
            ::GetWindowRect(hwndCand, &rc);
            int cx, cy;
            cx = rc.right - rc.left;
            cy = rc.bottom - rc.top;
            DPRINT("%d, %d, %d, %d\n", pt.x, pt.y, cx, cy);
            ::MoveWindow(hwndCand, pt.x, pt.y, cx, cy, TRUE);
            ::ShowWindow(hwndCand, SW_SHOWNOACTIVATE);
            ::InvalidateRect(hwndCand, NULL, FALSE);
            ::SendMessage(hUIWnd, WM_UI_CANDMOVE, 0, 0);
        }
        return;
    }

    // Has any candidates? 候補があるか？
    if (!lpIMC->HasCandInfo()) {
        FOOTMARK_POINT();
        lpIMC->DumpCandInfo();
        return;
    }

    DWORD dwStyle = lpIMC->cfCandForm[0].dwStyle;
    if (dwStyle == CFS_EXCLUDE) {
        DPRINT("CFS_EXCLUDE\n");
        // get work area and app window rect
        RECT rcWork, rcAppWnd;
        ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, FALSE);
        ::GetWindowRect(lpIMC->hWnd, &rcAppWnd);

        // get the specified position in screen coordinates
        ::GetClientRect(lpUIExtra->hwndCand, &rc);
        if (!lpUIExtra->bVertical) {
            pt.x = lpIMC->cfCandForm[0].ptCurrentPos.x;
            pt.y = lpIMC->cfCandForm[0].rcArea.bottom;
            ::ClientToScreen(lpIMC->hWnd, &pt);
            if (pt.y + rc.bottom > rcWork.bottom) {
                pt.y = rcAppWnd.top + lpIMC->cfCandForm[0].rcArea.top - rc.bottom;
            }
        } else {
            pt.x = lpIMC->cfCandForm[0].rcArea.left - rc.right;
            pt.y = lpIMC->cfCandForm[0].ptCurrentPos.y;
            ::ClientToScreen(lpIMC->hWnd, &pt);
            if (pt.x < 0) {
                pt.x = rcAppWnd.left + lpIMC->cfCandForm[0].rcArea.right;
            }
        }

        // move and show candidate window
        HWND hwndCand = lpUIExtra->hwndCand;
        if (::IsWindow(hwndCand)) {
            ::GetWindowRect(hwndCand, &rc);
            int cx, cy;
            cx = rc.right - rc.left;
            cy = rc.bottom - rc.top;
            DPRINT("%d, %d, %d, %d\n", pt.x, pt.y, cx, cy);
            ::MoveWindow(hwndCand, pt.x, pt.y, cx, cy, TRUE);
            ::ShowWindow(hwndCand, SW_SHOWNOACTIVATE);
            ::InvalidateRect(hwndCand, NULL, FALSE);
        }
        ::SendMessage(hUIWnd, WM_UI_CANDMOVE, 0, 0);
    } else if (dwStyle == CFS_CANDIDATEPOS) {
        DPRINT("CFS_CANDIDATEPOS\n");
        // get the specified position in screen coordinates
        pt.x = lpIMC->cfCandForm[0].ptCurrentPos.x;
        pt.y = lpIMC->cfCandForm[0].ptCurrentPos.y;
        ::ClientToScreen(lpIMC->hWnd, &pt);

        // move and show candidate window
        HWND hwndCand = lpUIExtra->hwndCand;
        if (::IsWindow(hwndCand)) {
            ::GetWindowRect(hwndCand, &rc);
            int cx, cy;
            cx = rc.right - rc.left;
            cy = rc.bottom - rc.top;
            DPRINT("%d, %d, %d, %d\n", pt.x, pt.y, cx, cy);
            ::MoveWindow(hwndCand, pt.x, pt.y, cx, cy, TRUE);
            ::ShowWindow(hwndCand, SW_SHOWNOACTIVATE);
            ::InvalidateRect(hwndCand, NULL, FALSE);
        }
        ::SendMessage(hUIWnd, WM_UI_CANDMOVE, 0, 0);
    } else {
        DPRINT("dwStyle: 0x%08lX\n", dwStyle);
    }
} // CandWnd_Move

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
