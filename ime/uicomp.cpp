// uicomp.cpp --- mzimeja composition window UI
// 未確定文字列ウィンドウ。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

// 寸法。
#define UNDERLINE_HEIGHT  2
#define CARET_WIDTH       2

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// Count how may the char can be arranged in DX.
// ピクセル位置dxの左に何文字あるか？
static int NumCharInDX(HDC hDC, const WCHAR *psz, int dx)
{
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
            ::GetTextExtentPointW(hDC, psz, ich, &siz); // 文字列のピクセル幅を取得。
            width = siz.cx;
        }
    }
    return ret;
}

// Count how may the char can be arranged in DY.
// ピクセル位置dyの上に何文字あるか？
static int NumCharInDY(HDC hDC, const WCHAR *psz, int dy)
{
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
            ::GetTextExtentPointW(hDC, psz, ich, &siz); // 文字列のピクセル高さを取得。
            height = siz.cy;
        }
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////

// 未確定文字列ウィンドウの作成時。
void CompWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC)
{
    RECT rc;
    POINT pt;
    FOOTMARK_FORMAT("%p, %p, %p\n", hUIWnd, lpUIExtra, lpIMC);

    lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;
    for (int i = 0; i < MAXCOMPWND; i++) {
        HWND hwnd = lpUIExtra->hwndComp[i];
        if (!::IsWindow(hwnd)) {
            hwnd = ::CreateWindowEx(0, szCompStrClassName, NULL, WS_COMPNODEFAULT,
                                    0, 0, 1, 1, hUIWnd, NULL, TheIME.m_hInst, NULL);
            lpUIExtra->hwndComp[i] = hwnd;
        }
        ::SetRectEmpty(&lpUIExtra->rcComp[i]);
        ::SetWindowLongPtr(hwnd, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
        ::SetWindowLongPtr(hwnd, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
        ::ShowWindow(hwnd, SW_HIDE);
    }

    if (!Config_GetData(L"ptDefComp", &pt, sizeof(pt))) {
        ::GetWindowRect(lpIMC->hWnd, &rc);
        pt.x = rc.left;
        pt.y = rc.bottom + 1;
    }
    DPRINTA("pt.x:%ld, pt.y:%ld\n", pt.x, pt.y);

    HWND hwndDef = lpUIExtra->hwndDefComp;
    if (!::IsWindow(hwndDef)) {
        hwndDef = CreateWindowEx(
                WS_EX_WINDOWEDGE, szCompStrClassName, NULL,
                WS_COMPDEFAULT | WS_DLGFRAME, pt.x, pt.y, 1, 1,
                hUIWnd, NULL, TheIME.m_hInst, NULL);
        lpUIExtra->hwndDefComp = hwndDef;
    }
    ::SetWindowLongPtr(hwndDef, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
    ::SetWindowLongPtr(hwndDef, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
    ::ShowWindow(hwndDef, SW_HIDE);
}

HWND GetCandPosHintFromComp(UIEXTRA *lpUIExtra, InputContext *lpIMC,
                            DWORD iClause, LPPOINT ppt, LPSIZE psizText)
{
    HWND hCompWnd;
    FOOTMARK_FORMAT("%p, %p, %d, %p\n", lpUIExtra, lpIMC, iClause, ppt);

    // is it vertical?
    BOOL fVert = (lpIMC->lfFont.A.lfEscapement == 2700);

    // get comp str
    CompStr *lpCompStr = lpIMC->LockCompStr();
    if (lpCompStr == NULL) return NULL;
    // get comp str
    std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
    const WCHAR *psz = str.c_str();
    const WCHAR *pch = psz;

    // get clause info
    DWORD *pdw = lpCompStr->GetCompClause();
    DWORD *pdwEnd = pdw + lpCompStr->dwCompClauseLen / sizeof(DWORD);
    std::unordered_set<DWORD> clauses(pdw, pdwEnd);

    DWORD ich = 0;
    DWORD dwClauseIndex = 0;
    for (int i = 0; i < MAXCOMPWND; i++) {
        if (lpIMC->cfCompForm.dwStyle) {
            hCompWnd = lpUIExtra->hwndComp[i];
        } else {
            hCompWnd = lpUIExtra->hwndDefComp;
        }

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

        DWORD cch = (DWORD)::GetWindowLongPtr(hCompWnd, FIGWL_COMPSTARTNUM);
        DPRINTA("ich: %d, cch: %d, dwClauseIndex: %d\n", ich, cch, dwClauseIndex);

        HDC hDC = ::GetDC(hCompWnd);
        HFONT hFont = (HFONT) ::GetWindowLongPtr(hCompWnd, FIGWLP_FONT);
        HFONT hOldFont = NULL;
        if (hFont) hOldFont = (HFONT) ::SelectObject(hDC, hFont);

        // is it end?
        SIZE siz;
        BOOL bIsDone = FALSE;
        const WCHAR *lpEnd = &pch[cch];
        while (pch < lpEnd) {
            // get size of text
            ::GetTextExtentPoint32W(hDC, pch, 1, &siz);

            if (dwClauseIndex == iClause) {
                if (fVert) {
                    ppt->x = x;
                    ppt->y = y + siz.cx;
                } else {
                    ppt->x = x;
                    ppt->y = y + siz.cy;
                }
                if (psizText)
                    *psizText = siz;
                ::ClientToScreen(hCompWnd, ppt);
                bIsDone = TRUE;
                break;
            }

            // go to next position
            ++pch;
            ++ich;
            if (fVert)
                y += siz.cx;
            else
                x += siz.cx;

            if (clauses.count(ich) > 0) {
                ++dwClauseIndex;
            }
        }

        ::SelectObject(hDC, hOldFont);
        ::ReleaseDC(hCompWnd, hDC);

        if (bIsDone || lpIMC->cfCompForm.dwStyle == 0) {
            break;
        }
    }

    lpIMC->UnlockCompStr();
    return hCompWnd;
} // GetCandPosHintFromComp

// calc the position of composition windows and move them
void CompWnd_Move(UIEXTRA *lpUIExtra, InputContext *lpIMC)
{
    FOOTMARK_FORMAT("%p, %p\n", lpUIExtra, lpIMC);
    lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;

    HFONT hFont = NULL;
    HFONT hOldFont = NULL;
    DPRINTA("dwStyle: 0x%08X\n", lpIMC->cfCompForm.dwStyle);
    if (lpIMC->cfCompForm.dwStyle) { // style is not CFS_DEFAULT
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
            DPRINTA("!::PtInRect\n");
            lpIMC->UnlockCompStr();
            return;
        }

        // hide the default composition window
        HWND hwndDef = lpUIExtra->hwndDefComp;
        if (::IsWindow(hwndDef)) {
            ::ShowWindow(hwndDef, SW_HIDE);
        }

        std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
        const WCHAR *psz = str.c_str();
        const WCHAR *pch = psz;
        DWORD ich = 0, iClause = 0;

        // clause info
        DWORD *pdw = lpCompStr->GetCompClause();
        DWORD *pdwEnd = pdw + lpCompStr->dwCompClauseLen / sizeof(DWORD);
        std::unordered_set<DWORD> clauses(pdw, pdwEnd);

        if (!lpUIExtra->bVertical) { // not vertical font
            int dx = rcSrc.right - ptSrc.x;
            int curx = ptSrc.x, cury = ptSrc.y;

            // set the composition string to each composition window.
            // the composition windows that are given the compostion string
            // will be moved and shown.
            for (int i = 0; i < MAXCOMPWND; i++) {
                HWND hwnd = lpUIExtra->hwndComp[i];
                if (::IsWindow(hwnd)) {
                    HDC hDC = ::GetDC(hwnd);
                    hFont = (HFONT) ::GetWindowLongPtr(hwnd, FIGWLP_FONT);
                    if (hFont) hOldFont = (HFONT) ::SelectObject(hDC, hFont);

                    SIZE siz;
                    siz.cy = 0;

                    int num = NumCharInDX(hDC, pch, dx);
                    if (num) {
                        ::GetTextExtentPoint32W(hDC, pch, num, &siz);

                        lpUIExtra->rcComp[i].left = curx;
                        lpUIExtra->rcComp[i].top = cury;
                        siz.cx += CARET_WIDTH;
                        siz.cy += UNDERLINE_HEIGHT;
                        lpUIExtra->rcComp[i].right = siz.cx;
                        lpUIExtra->rcComp[i].bottom = siz.cy;
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTSTR, LONG(pch - psz));
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTNUM, num);
                        DPRINTA("%d, %d, %d, %d\n", curx, cury, siz.cy, siz.cx);
                        ::MoveWindow(hwnd, curx, cury, siz.cx, siz.cy, TRUE);
                        ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);

                        pch += num;
                        ich += num;

                        if (clauses.count(ich) > 0) {
                            ++iClause;
                        }
                    } else {
                        ::SetRectEmpty(&lpUIExtra->rcComp[i]);
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTSTR, 0);
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTNUM, 0);
                        ::ShowWindow(hwnd, SW_HIDE);
                    }
                    ::InvalidateRect(hwnd, NULL, FALSE);

                    dx = rcSrc.right - rcSrc.left;
                    curx = rcSrc.left;
                    cury += siz.cy + UNDERLINE_HEIGHT;

                    if (hOldFont) ::SelectObject(hDC, hOldFont);
                    ::ReleaseDC(hwnd, hDC);
                }
            }
        } else { // vertical font
            int dy = rcSrc.bottom - ptSrc.y;
            int curx = ptSrc.x, cury = ptSrc.y;

            for (int i = 0; i < MAXCOMPWND; i++) {
                HWND hwnd = lpUIExtra->hwndComp[i];
                if (::IsWindow(hwnd)) {
                    HDC hDC = ::GetDC(hwnd);
                    hFont = (HFONT) ::GetWindowLongPtr(hwnd, FIGWLP_FONT);
                    if (hFont) hOldFont = (HFONT) ::SelectObject(hDC, hFont);

                    SIZE siz;
                    siz.cy = 0;
                    int num = NumCharInDY(hDC, pch, dy);
                    if (num) {
                        ::GetTextExtentPoint32W(hDC, pch, num, &siz);

                        lpUIExtra->rcComp[i].left = curx - siz.cy;
                        lpUIExtra->rcComp[i].top = cury;
                        siz.cy += UNDERLINE_HEIGHT;
                        siz.cx += CARET_WIDTH;
                        lpUIExtra->rcComp[i].right = siz.cy;
                        lpUIExtra->rcComp[i].bottom = siz.cx;
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTSTR, LONG(pch - psz));
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTNUM, num);
                        DPRINTA("%d, %d, %d, %d\n", curx, cury, siz.cy, siz.cx);
                        ::MoveWindow(hwnd, curx, cury, siz.cy, siz.cx, TRUE);
                        ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);

                        pch += num;
                        ich += num;

                        if (clauses.count(ich) > 0) {
                            ++iClause;
                        }
                    } else {
                        ::SetRectEmpty(&lpUIExtra->rcComp[i]);
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTSTR, 0);
                        ::SetWindowLongPtr(hwnd, FIGWL_COMPSTARTNUM, 0);
                        ::ShowWindow(hwnd, SW_HIDE);
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
    } else { // style is CFS_DEFAULT
        HWND hwndDef = lpUIExtra->hwndDefComp;
        if (::IsWindow(hwndDef)) {
            ::SetWindowLongPtr(hwndDef, FIGWL_COMPSTARTSTR, 0);
            ::SetWindowLongPtr(hwndDef, FIGWL_COMPSTARTNUM, 0);

            // hide all non-default comp windows
            for (int i = 0; i < MAXCOMPWND; i++) {
                HWND hwnd = lpUIExtra->hwndComp[i];
                if (::IsWindow(hwnd)) {
                    ::ShowWindow(hwnd, SW_HIDE);
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
            POINT pt;
            pt.x = rc.left;
            pt.y = rc.top;
            Config_SetData(L"ptDefComp", REG_BINARY, &pt, sizeof(pt));
            width += 2 * ::GetSystemMetrics(SM_CXEDGE) + CARET_WIDTH;
            height += 2 * ::GetSystemMetrics(SM_CYEDGE) + UNDERLINE_HEIGHT;

            // move and show window
            DPRINTA("%d, %d, %d, %d\n", rc.left, rc.top, width, height);
            ::MoveWindow(hwndDef, rc.left, rc.top, width, height, TRUE);
            ::ShowWindow(hwndDef, SW_SHOWNOACTIVATE);

            // redraw window
            ::InvalidateRect(hwndDef, NULL, FALSE);
        }
    }
} // CompWnd_Move

// 未確定文字列の一行を描画する。
void DrawTextOneLine(HWND hCompWnd, HDC hDC, const WCHAR *pch,
                     DWORD ich, DWORD cch, CompStr *lpCompStr, BOOL fVert)
{
    if (cch == 0)
        return; // 文字列の長さがゼロなら終了。

    // 属性。
    BYTE *lpattr = lpCompStr->GetCompAttr();

    // 節情報（clauses）を取得。
    DWORD *pdw = lpCompStr->GetCompClause();
    DWORD *pdwEnd = pdw + lpCompStr->dwCompClauseLen / sizeof(DWORD);
    std::unordered_set<DWORD> clauses(pdw, pdwEnd);

    // クライアント領域を取得する。
    RECT rc;
    ::GetClientRect(hCompWnd, &rc);

    // 開始位置。
    int x, y;
    if (fVert) {
        x = rc.right - UNDERLINE_HEIGHT;
        y = 0;
    } else {
        x = y = 0;
    }

    // 文字列描画において不透明モードにする。
    ::SetBkMode(hDC, OPAQUE);

    SIZE siz;
    const WCHAR *lpEnd = &pch[cch]; // 文字列終端の位置。

    while (pch < lpEnd) { // 一文字ずつ描画する。
        // 色とペンをセットする。
        HPEN hPen;
        switch (lpattr[ich]) {
        case ATTR_TARGET_CONVERTED:
            ::SetTextColor(hDC, RGB(255, 255, 255));
            ::SetBkColor(hDC, RGB(0, 51, 0));
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

        // テキストを一文字描画する。
        ::TextOutW(hDC, x, y, pch, 1);

        // テキストの寸法を取得する。
        ::GetTextExtentPoint32W(hDC, pch, 1, &siz);

        // 変換中ならば下線を描画する。
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

        // カーソルを描画する。
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

        // 次の位置へ移動する。
        ++pch;
        ++ich;
        if (fVert)
            y += siz.cx;
        else
            x += siz.cx;
    }

    // キャレットを描画する（もしあれば）。
    if (lpCompStr->dwCursorPos == ich) { // 現在の位置か？
        ::SelectObject(hDC, ::GetStockObject(BLACK_PEN)); // 黒いペンで。
        if (fVert) { // 縦書きか？
            ::MoveToEx(hDC, x, y, NULL);
            ::LineTo(hDC, x + siz.cy, y);
            ::MoveToEx(hDC, x, y + 1, NULL);
            ::LineTo(hDC, x + siz.cy, y + 1);
        } else { // 横書き。
            ::MoveToEx(hDC, x, y, NULL);
            ::LineTo(hDC, x, y + siz.cy);
            ::MoveToEx(hDC, x + 1, y, NULL);
            ::LineTo(hDC, x + 1, y + siz.cy);
        }
    }
}

// 未確定文字列を描画する。
void CompWnd_Draw(HWND hCompWnd, HDC hDC, InputContext *lpIMC, CompStr *lpCompStr)
{
    // 未確定文字列を取得。
    std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
    const WCHAR *pch = str.c_str();

    // Is it vertical? 縦書きか？
    BOOL fVert = (lpIMC->lfFont.A.lfEscapement == 2700);

    // さらにヘルパー関数を用いて描画する。
    if (lpIMC->cfCompForm.dwStyle) {
        ::SetBkMode(hDC, OPAQUE);

        DWORD ich = (DWORD)::GetWindowLongPtr(hCompWnd, FIGWL_COMPSTARTSTR);
        DWORD cch = (DWORD)::GetWindowLongPtr(hCompWnd, FIGWL_COMPSTARTNUM);
        if (cch && ich + cch <= DWORD(::lstrlenW(pch))) {
            pch += ich;
            DrawTextOneLine(hCompWnd, hDC, pch, ich, cch, lpCompStr, fVert);
        }
    } else {
        int cch = int(str.size());
        DrawTextOneLine(hCompWnd, hDC, pch, 0, cch, lpCompStr, fVert);
    }
}

// 未確定文字列ウィンドウの再描画時。
void CompWnd_Paint(HWND hCompWnd)
{
    PAINTSTRUCT ps;
    HDC hDC = ::BeginPaint(hCompWnd, &ps); // 描画を開始。

    // クライアント領域を取得する。
    RECT rc;
    ::GetClientRect(hCompWnd, &rc);

    // ちらつきを防止するため、メモリービットマップを使用。
    HDC hdcMem = ::CreateCompatibleDC(hDC);
    HBITMAP hbm = ::CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
    HGDIOBJ hbmOld = ::SelectObject(hdcMem, hbm);

    // 背景を描画する。
    ::FillRect(hdcMem, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // フォントを選択する。
    HFONT hOldFont = NULL;
    HFONT hFont = (HFONT) ::GetWindowLongPtr(hCompWnd, FIGWLP_FONT);
    if (hFont) hOldFont = (HFONT) ::SelectObject(hdcMem, hFont);

    // UIサーバーからhIMCを取得する。
    HWND hSvrWnd = (HWND) ::GetWindowLongPtr(hCompWnd, FIGWLP_SERVERWND); // UIサーバー。
    ASSERT(hSvrWnd != NULL);
    HIMC hIMC = (HIMC) ::GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC); // IMC。
    ASSERT(hIMC != NULL);

    if (hIMC) {
        InputContext *lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロック。
        ASSERT(lpIMC != NULL);
        if (lpIMC) {
            CompStr *lpCompStr = lpIMC->LockCompStr(); // 未確定文字列をロック。
            if (lpCompStr) {
                if (lpCompStr->dwCompStrLen > 0) { // 文字列があれば
                    // ヘルパー関数を用いて描画する。
                    CompWnd_Draw(hCompWnd, hdcMem, lpIMC, lpCompStr);
                }
                lpIMC->UnlockCompStr(); // 未確定文字列のロックを解除。
            }
            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。
        }
    }

    // フォントの選択を解除。
    if (hFont && hOldFont) ::SelectObject(hdcMem, hOldFont);

    // ビット群を転送（hDC←hdcMem）。
    ::BitBlt(hDC, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

    // 後始末。
    ::DeleteObject(::SelectObject(hdcMem, hbmOld));
    ::DeleteDC(hdcMem);

    ::EndPaint(hCompWnd, &ps); // 描画を終了。
} // CompWnd_Paint

// 未確定文字列ウィンドウを隠す。
void CompWnd_Hide(UIEXTRA *lpUIExtra)
{
    HWND hwndDef = lpUIExtra->hwndDefComp; // 既定のウィンドウ。
    if (::IsWindow(hwndDef)) {
        ::ShowWindow(hwndDef, SW_HIDE); // 隠す。
    }

    for (int i = 0; i < MAXCOMPWND; i++) { // 他の複数のウィンドウ。
        HWND hwnd = lpUIExtra->hwndComp[i];
        if (::IsWindow(hwnd)) {
            ::ShowWindow(hwnd, SW_HIDE); // 隠す。
        }
    }
} // CompWnd_Hide

// 未確定文字列ウィンドウのフォントを設定する。
void CompWnd_SetFont(UIEXTRA *lpUIExtra)
{
    for (int i = 0; i < MAXCOMPWND; i++) { // 既定以外のウィンドウ。
        HWND hwnd = lpUIExtra->hwndComp[i]; // ウィンドウハンドル。
        if (::IsWindow(hwnd)) {
            ::SetWindowLongPtr(hwnd, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont); // フォント指定。
        }
    }
}

// 未確定文字列ウィンドウのウィンドウプロシージャ。
LRESULT CALLBACK CompWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                    LPARAM lParam)
{
    HWND hUIWnd;

    switch (message) {
    case WM_ERASEBKGND:
        return TRUE; // ちらつきを防止するため、ここで背景を描画しない。

    case WM_PAINT: // 描画時。
        CompWnd_Paint(hWnd);
        break;

    case WM_SETCURSOR: // マウスカーソル設定時。
    case WM_MOUSEMOVE: // マウス移動時。
    case WM_LBUTTONUP: // 左ボタン解放時。
    case WM_RBUTTONUP: // 右ボタン解放時。
        DragUI(hWnd, message, wParam, lParam);
        if ((message == WM_SETCURSOR) && (HIWORD(lParam) != WM_LBUTTONDOWN) &&
            (HIWORD(lParam) != WM_RBUTTONDOWN))
            return DefWindowProc(hWnd, message, wParam, lParam);
        if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP))
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, 0); // 状態を元に戻す。
        break;

    case WM_MOVE: // ウィンドウ移動じ。
        hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (IsWindow(hUIWnd))
            SendMessage(hUIWnd, WM_UI_DEFCOMPMOVE, 0, 0); // サーバーに知らせる。
        break;

    default: // その他のメッセージ。
        if (!IsImeMessage(message))
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
