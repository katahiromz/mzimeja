// uicomp.cpp --- mzimeja composition window UI
// ���m�蕶����E�B���h�E�B
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

// ���@�B
#define UNDERLINE_HEIGHT  2
#define CARET_WIDTH       2

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// Count how may the char can be arranged in DX.
// �s�N�Z���ʒudx�̍��ɉ��������邩�H
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
            ::GetTextExtentPointW(hDC, psz, ich, &siz); // ������̃s�N�Z�������擾�B
            width = siz.cx;
        }
    }
    return ret;
}

// Count how may the char can be arranged in DY.
// �s�N�Z���ʒudy�̏�ɉ��������邩�H
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
            ::GetTextExtentPointW(hDC, psz, ich, &siz); // ������̃s�N�Z���������擾�B
            height = siz.cy;
        }
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////

// ���m�蕶����E�B���h�E�̍쐬���B
void CompWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC) {
    RECT rc;
    POINT pt;

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

    if (TheIME.GetUserData(L"ptDefComp", &pt, sizeof(pt))) {
        ::GetWindowRect(lpIMC->hWnd, &rc);
        pt.x = rc.left;
        pt.y = rc.bottom + 1;
    }

    HWND hwndDef = lpUIExtra->hwndDefComp;
    if (!::IsWindow(hwndDef)) {
        hwndDef = CreateWindowEx(
                WS_EX_WINDOWEDGE, szCompStrClassName, NULL,
                WS_COMPDEFAULT | WS_DLGFRAME, pt.x, pt.y, 1, 1,
                hUIWnd, NULL, TheIME.m_hInst, NULL);
        lpUIExtra->hwndDefComp = hwndDef;
    }
    ::SetWindowLong(hwndDef, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont);
    ::SetWindowLongPtr(hwndDef, FIGWLP_SERVERWND, (LONG_PTR)hUIWnd);
    ::ShowWindow(hwndDef, SW_HIDE);
}

HWND GetCandPosHintFromComp(UIEXTRA *lpUIExtra, InputContext *lpIMC,
                            DWORD iClause, LPPOINT ppt)
{
    HWND hCompWnd;

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
    std::set<DWORD> clauses(pdw, pdwEnd);

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

        DWORD cch = ::GetWindowLong(hCompWnd, FIGWL_COMPSTARTNUM);
        DebugPrintA("ich: %d, cch: %d, dwClauseIndex: %d\n", ich, cch, dwClauseIndex);

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
void CompWnd_Move(UIEXTRA *lpUIExtra, InputContext *lpIMC) {
    lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;

    HFONT hFont = NULL;
    HFONT hOldFont = NULL;
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
        std::set<DWORD> clauses(pdw, pdwEnd);

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
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, LONG(pch - psz));
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, num);
                        ::MoveWindow(hwnd, curx, cury, siz.cx, siz.cy, TRUE);
                        ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);

                        pch += num;
                        ich += num;

                        if (clauses.count(ich) > 0) {
                            ++iClause;
                        }
                    } else {
                        ::SetRectEmpty(&lpUIExtra->rcComp[i]);
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, 0);
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, 0);
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
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, LONG(pch - psz));
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, num);
                        ::MoveWindow(hwnd, curx, cury, siz.cy, siz.cx, TRUE);
                        ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);

                        pch += num;
                        ich += num;

                        if (clauses.count(ich) > 0) {
                            ++iClause;
                        }
                    } else {
                        ::SetRectEmpty(&lpUIExtra->rcComp[i]);
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTSTR, 0);
                        ::SetWindowLong(hwnd, FIGWL_COMPSTARTNUM, 0);
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
            ::SetWindowLong(hwndDef, FIGWL_COMPSTARTSTR, 0);
            ::SetWindowLong(hwndDef, FIGWL_COMPSTARTNUM, 0);

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
            TheIME.SetUserData(L"ptDefComp", &pt, sizeof(pt));
            width += 2 * ::GetSystemMetrics(SM_CXEDGE) + CARET_WIDTH;
            height += 2 * ::GetSystemMetrics(SM_CYEDGE) + UNDERLINE_HEIGHT;

            // move and show window
            ::MoveWindow(hwndDef, rc.left, rc.top, width, height, TRUE);
            ::ShowWindow(hwndDef, SW_SHOWNOACTIVATE);

            // redraw window
            ::InvalidateRect(hwndDef, NULL, FALSE);
        }
    }
} // CompWnd_Move

// ���m�蕶����̈�s��`�悷��B
void DrawTextOneLine(HWND hCompWnd, HDC hDC, const WCHAR *pch,
                     DWORD ich, DWORD cch, CompStr *lpCompStr, BOOL fVert) {
    if (cch == 0) return; // ������̒������[���Ȃ�I���B

    // Attribute. �����B
    BYTE *lpattr = lpCompStr->GetCompAttr();

    // Get clause info. �ߏ����擾�B
    DWORD *pdw = lpCompStr->GetCompClause();
    DWORD *pdwEnd = pdw + lpCompStr->dwCompClauseLen / sizeof(DWORD);
    std::set<DWORD> clauses(pdw, pdwEnd);

    // Get client rect and fill white. �N���C�A���g�̈���擾���A���œh��Ԃ��B
    RECT rc;
    ::GetClientRect(hCompWnd, &rc);
    ::FillRect(hDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

    // Starting position. �J�n�ʒu�B
    int x, y;
    if (fVert) {
        x = rc.right - UNDERLINE_HEIGHT;
        y = 0;
    } else {
        x = y = 0;
    }

    // Set opaque mode. ������`��ɂ����ĕs�������[�h�ɂ���B
    ::SetBkMode(hDC, OPAQUE);

    // Is it end? �I��肩�H
    SIZE siz;
    const WCHAR *lpEnd = &pch[cch];
    while (pch < lpEnd) {

        // set color and pen
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

    // Draw caret at last if any. �L�����b�g��`�悷��i��������΁j�B
    if (lpCompStr->dwCursorPos == ich) { // ���݂̈ʒu���H
        ::SelectObject(hDC, ::GetStockObject(BLACK_PEN)); // �����y���ŁB
        if (fVert) { // �c�������H
            ::MoveToEx(hDC, x, y, NULL);
            ::LineTo(hDC, x + siz.cy, y);
            ::MoveToEx(hDC, x, y + 1, NULL);
            ::LineTo(hDC, x + siz.cy, y + 1);
        } else { // �������B
            ::MoveToEx(hDC, x, y, NULL);
            ::LineTo(hDC, x, y + siz.cy);
            ::MoveToEx(hDC, x + 1, y, NULL);
            ::LineTo(hDC, x + 1, y + siz.cy);
        }
    }
}

// ���m�蕶�����`�悷��B
void CompWnd_Draw(HWND hCompWnd, HDC hDC, InputContext *lpIMC, CompStr *lpCompStr) {
    // ���m�蕶������擾�B
    std::wstring str(lpCompStr->GetCompStr(), lpCompStr->dwCompStrLen);
    const WCHAR *pch = str.c_str();

    // Is it vertical? �c�������H
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

// ���m�蕶����E�B���h�E�̍ĕ`�掞�B
void CompWnd_Paint(HWND hCompWnd) {
    PAINTSTRUCT ps;
    HDC hDC = ::BeginPaint(hCompWnd, &ps); // �`����J�n�B

    HFONT hOldFont = NULL;
    HFONT hFont = (HFONT) ::GetWindowLongPtr(hCompWnd, FIGWLP_FONT); // �t�H���g�B
    if (hFont) hOldFont = (HFONT) ::SelectObject(hDC, hFont); // �t�H���g��I���B

    HWND hSvrWnd = (HWND) ::GetWindowLongPtr(hCompWnd, FIGWLP_SERVERWND); // UI�T�[�o�[�B
    ASSERT(hSvrWnd != NULL);

    HIMC hIMC = (HIMC) ::GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC); // IMC�B
    ASSERT(hIMC != NULL);
    if (hIMC) {
        InputContext *lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N�B
        ASSERT(lpIMC != NULL);
        if (lpIMC) {
            CompStr *lpCompStr = lpIMC->LockCompStr(); // ���m�蕶��������b�N�B
            if (lpCompStr) {
                if (lpCompStr->dwCompStrLen > 0) { // �����񂪂����
                    CompWnd_Draw(hCompWnd, hDC, lpIMC, lpCompStr); // �`�悷��B
                }
                lpIMC->UnlockCompStr(); // ���m�蕶����̃��b�N�������B
            }
            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B
        }
    }
    if (hFont && hOldFont) ::SelectObject(hDC, hOldFont); // �t�H���g�̑I���������B
    ::EndPaint(hCompWnd, &ps); // �`����I���B
} // CompWnd_Paint

// ���m�蕶����E�B���h�E���B���B
void CompWnd_Hide(UIEXTRA *lpUIExtra) {
    HWND hwndDef = lpUIExtra->hwndDefComp; // ����̃E�B���h�E�B
    if (::IsWindow(hwndDef)) {
        ::ShowWindow(hwndDef, SW_HIDE); // �B���B
    }

    for (int i = 0; i < MAXCOMPWND; i++) { // ���̕����̃E�B���h�E�B
        HWND hwnd = lpUIExtra->hwndComp[i];
        if (::IsWindow(hwnd)) {
            ::ShowWindow(hwnd, SW_HIDE); // �B���B
        }
    }
} // CompWnd_Hide

// ���m�蕶����E�B���h�E�̃t�H���g��ݒ肷��B
void CompWnd_SetFont(UIEXTRA *lpUIExtra) {
    for (int i = 0; i < MAXCOMPWND; i++) { // ����ȊO�̃E�B���h�E�B
        HWND hwnd = lpUIExtra->hwndComp[i]; // �E�B���h�E�n���h���B
        if (::IsWindow(hwnd)) {
            ::SetWindowLongPtr(hwnd, FIGWLP_FONT, (LONG_PTR)lpUIExtra->hFont); // �t�H���g�w��B
        }
    }
}

// ���m�蕶����E�B���h�E�̃E�B���h�E�v���V�[�W���B
LRESULT CALLBACK CompWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                    LPARAM lParam) {
    HWND hUIWnd;

    switch (message) {
    case WM_PAINT: // �`�掞�B
        CompWnd_Paint(hWnd);
        break;

    case WM_SETCURSOR: // �}�E�X�J�[�\���ݒ莞�B
    case WM_MOUSEMOVE: // �}�E�X�ړ����B
    case WM_LBUTTONUP: // ���{�^��������B
    case WM_RBUTTONUP: // �E�{�^��������B
        DragUI(hWnd, message, wParam, lParam);
        if ((message == WM_SETCURSOR) && (HIWORD(lParam) != WM_LBUTTONDOWN) &&
            (HIWORD(lParam) != WM_RBUTTONDOWN))
            return DefWindowProc(hWnd, message, wParam, lParam);
        if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP))
            SetWindowLong(hWnd, FIGWL_MOUSE, 0); // ��Ԃ����ɖ߂��B
        break;

    case WM_MOVE: // �E�B���h�E�ړ����B
        hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (IsWindow(hUIWnd))
            SendMessage(hUIWnd, WM_UI_DEFCOMPMOVE, 0, 0); // �T�[�o�[�ɒm�点��B
        break;

    default: // ���̑��̃��b�Z�[�W�B
        if (!IsImeMessage(message))
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
