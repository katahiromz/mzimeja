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
    HFONT hFont = (HFONT)::GetWindowLongPtr(hWnd, FIGWLP_FONT);
    HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);
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
    ::SelectObject(hDC, hOldFont);
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

static BOOL
GetCandPosFromCompWnd(InputContext *lpIMC, UIEXTRA *lpUIExtra, LPPOINT lppt, LPSIZE psizText)
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
            HWND hCompWnd = GetCandPosHintFromComp(lpUIExtra, lpIMC, iClause, lppt, psizText);
            ret = ::IsWindowVisible(hCompWnd);
        }
    }

    return ret;
}

void CandWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC)
{
    POINT pt;
    SIZE sizText;
    FOOTMARK_FORMAT("%p, %p, %p\n", hUIWnd, lpUIExtra, lpIMC);

    if (GetCandPosFromCompWnd(lpIMC, lpUIExtra, &pt, &sizText)) {
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
    ::SetWindowLongPtr(lpUIExtra->hwndCand, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
    ::ShowWindow(lpUIExtra->hwndCand, SW_HIDE);
} // CandWnd_Create

// 候補ウィンドウのフォントを設定する。
void CandWnd_SetFont(UIEXTRA *lpUIExtra)
{
    ::SetWindowLongPtr(lpUIExtra->hwndCand, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
}

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
            HFONT hFont = (HFONT)::GetWindowLongPtr(hCandWnd, FIGWLP_FONT);
            HFONT hOldFont = (HFONT)::SelectObject(hdcMem, hFont);
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
            ::SelectObject(hdcMem, hOldFont);
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
    INT width1 = 0, height = 0;

    // 準備。
    HDC hDC = ::CreateCompatibleDC(NULL);
    HFONT hFont = (HFONT)::GetWindowLongPtr(lpUIExtra->hwndCand, FIGWLP_FONT);
    HFONT hOldFont = (HFONT)::SelectObject(hDC, hFont);

    // 候補情報をロックする。
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
        // 候補があれば
        if (lpCandInfo->dwCount > 0) {
            // 現在の候補リストのインデックスを取得する。
            CANDINFOEXTRA *pExtra = lpCandInfo->GetExtra();
            DWORD iList = 0;
            if (pExtra) iList = pExtra->iClause;

            // 候補リストとページ終端を取得する。
            CandList *lpCandList = lpCandInfo->GetList(iList);
            DWORD i, end = lpCandList->GetPageEnd();
            // 候補リスト中のページ内の候補ごとにサイズを取得し、幅を拡張し、高さを加算する。
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

    // 後始末。
    ::SelectObject(hDC, hOldFont);
    ::DeleteDC(hDC);

    // 求められた幅と高さに応じて戻り値を設定する。
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
        DPRINTA("%d, %d, %d, %d\n", rc.left, rc.top, siz.cx, siz.cy);
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
        DPRINTA("%d, %d, %d, %d\n", -1, -1, 0, 0);
        ::MoveWindow(lpUIExtra->hwndCand, -1, -1, 0, 0, TRUE);
        ::ShowWindow(lpUIExtra->hwndCand, SW_HIDE);
    }
} // CandWnd_Hide

// 候補ウィンドウの移動時。
void CandWnd_Move(UIEXTRA *lpUIExtra, InputContext *lpIMC)
{
    FOOTMARK_FORMAT("%p, %p\n", lpUIExtra, lpIMC);

    HWND hwndCand = lpUIExtra->hwndCand;
    HWND hSvrWnd = (HWND)::GetWindowLongPtr(hwndCand, FIGWLP_SERVERWND);

    // ワークエリアを取得。
    RECT rcWork;
    HMONITOR hMonitor = ::MonitorFromWindow(hwndCand, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    if (::GetMonitorInfo(hMonitor, &mi))
        rcWork = mi.rcWork;
    else
        ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, FALSE);

    // 現在のウィンドウの長方形を取得。
    RECT rcCand;
    ::GetWindowRect(hwndCand, &rcCand);
    INT cx = rcCand.right - rcCand.left, cy = rcCand.bottom - rcCand.top;

    // 位置を決める。
    POINT pt;
    SIZE sizText;
    GetCandPosFromCompWnd(lpIMC, lpUIExtra, &pt, &sizText);
    if (lpUIExtra->bVertical) { // 縦書き。
        if (pt.x - cx < rcWork.left) {
            pt.x += sizText.cx;
        } else {
            pt.x -= cx;
        }
    } else { // 横書き。
        if (pt.y + cy > rcWork.bottom) {
            pt.y -= cy + sizText.cy;
        }
    }

    // 初期化されてないか？
    if (lpIMC->cfCandForm[0].dwIndex == (DWORD)-1) {
        lpIMC->DumpCandInfo();
        lpUIExtra->ptCand.x = pt.x;
        lpUIExtra->ptCand.y = pt.y;

        DPRINTA("%d, %d, %d, %d\n", pt.x, pt.y, cx, cy);
        ::MoveWindow(hwndCand, pt.x, pt.y, cx, cy, TRUE);
        ::ShowWindow(hwndCand, SW_SHOWNOACTIVATE);
        ::InvalidateRect(hwndCand, NULL, FALSE);

        HWND hSvrWnd = (HWND) ::GetWindowLongPtr(hwndCand, FIGWLP_SERVERWND);
        ::SendMessage(hSvrWnd, WM_UI_CANDMOVE, 0, 0);
        return;
    }

    // 候補があるか？
    if (!lpIMC->HasCandInfo()) {
        FOOTMARK_POINT();
        lpIMC->DumpCandInfo();
        return;
    }

    DWORD dwStyle = lpIMC->cfCandForm[0].dwStyle;
    if (dwStyle == CFS_EXCLUDE) {
        DPRINTA("CFS_EXCLUDE\n");
    } else if (dwStyle == CFS_CANDIDATEPOS) {
        DPRINTA("CFS_CANDIDATEPOS\n");
        // 位置情報を取得する。
        pt.x = lpIMC->cfCandForm[0].ptCurrentPos.x;
        pt.y = lpIMC->cfCandForm[0].ptCurrentPos.y;
        ::ClientToScreen(lpIMC->hWnd, &pt);
    } else {
        DPRINTA("dwStyle: 0x%08lX\n", dwStyle);
        return;
    }

    // 候補ウィンドウを移動・表示する。
    if (::IsWindow(hwndCand)) {
        DPRINTA("%d, %d, %d, %d\n", pt.x, pt.y, cx, cy);
        ::MoveWindow(hwndCand, pt.x, pt.y, cx, cy, TRUE);
        ::ShowWindow(hwndCand, SW_SHOWNOACTIVATE);
        ::InvalidateRect(hwndCand, NULL, FALSE);
    }

    // 動いたことをUIサーバーに通知。
    ::SendMessage(hSvrWnd, WM_UI_CANDMOVE, 0, 0);
} // CandWnd_Move

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
