// ui.cpp --- mzimeja UI server
// UIサーバーウィンドウ。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// 指定されているUIウィンドウを表示する／非表示にする。
void PASCAL ShowUIWindows(HWND hwndServer, BOOL fFlag)
{
    FOOTMARK_FORMAT("%p, %d\n", hwndServer, fFlag);
    INT nsw = (fFlag ? SW_SHOWNOACTIVATE : SW_HIDE);

    UIEXTRA *lpUIExtra = LockUIExtra(hwndServer);
    if (lpUIExtra) {
        if (IsWindow(lpUIExtra->hwndStatus)) {
            ::ShowWindow(lpUIExtra->hwndStatus, nsw);
        }
        if (IsWindow(lpUIExtra->hwndCand)) {
            ::ShowWindow(lpUIExtra->hwndCand, nsw);
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

void OnImeSetContext(HWND hWnd, HIMC hIMC, LPARAM lParam)
{
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
                if (::IsWindow(lpUIExtra->hwndCand)) {
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

                if (Config_GetDWORD(L"IsNonRoman", FALSE)) {
                    lpIMC->Conversion() &= ~IME_CMODE_ROMAN;
                } else {
                    lpIMC->Conversion() |= IME_CMODE_ROMAN;
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

void OnDestroy(HWND hWnd)
{
    UIEXTRA *lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
        if (::IsWindow(lpUIExtra->hwndStatus))
            ::DestroyWindow(lpUIExtra->hwndStatus);

        if (::IsWindow(lpUIExtra->hwndCand))
            ::DestroyWindow(lpUIExtra->hwndCand);

        if (::IsWindow(lpUIExtra->hwndDefComp))
            ::DestroyWindow(lpUIExtra->hwndDefComp);

        for (INT i = 0; i < MAXCOMPWND; i++) {
            if (::IsWindow(lpUIExtra->hwndComp[i]))
                ::DestroyWindow(lpUIExtra->hwndComp[i]);
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
// IME UIサーバーウィンドウのウィンドウプロシージャ。
LRESULT CALLBACK MZIMEWndProc(HWND hWnd, UINT message, WPARAM wParam,
                              LPARAM lParam)
{
    InputContext *lpIMC;
    UIEXTRA *lpUIExtra;
    HGLOBAL hUIExtra;
    LONG lRet = 0;

    HIMC hIMC = (HIMC)GetWindowLongPtr(hWnd, IMMGWLP_IMC);

    // Even if there is no current UI. these messages should not be pass to
    // DefWindowProc().
    if (hIMC == NULL) {
        if (IsImeMessage2(message)) {
            DPRINT("Why hIMC is NULL?\n");
            DPRINT("hWnd: %p, message: 0x%x, wParam: %x, lParam: %x\n",
                   hWnd, message, wParam, lParam);
            return 0;
        }
    }

    switch (message) {
    case WM_CREATE: // ウィンドウ作成時。
        DPRINT("WM_CREATE\n");
        // Allocate UI's extra memory block.
        hUIExtra = GlobalAlloc(GHND, sizeof(UIEXTRA));
        lpUIExtra = (UIEXTRA *)GlobalLock(hUIExtra);
        if (lpUIExtra) {
            lpUIExtra->ptCand.x = -1;
            lpUIExtra->ptCand.y = -1;
            lpUIExtra->hFont = NULL;
            GlobalUnlock(hUIExtra);
        }
        SetUIExtraToServerWnd(hWnd, hUIExtra);
        break;

    case WM_IME_SETCONTEXT: // IMEコンテキスト設定時。
        DPRINT("WM_IME_SETCONTEXT\n");
        if (wParam) {
            OnImeSetContext(hWnd, hIMC, lParam);
        }
        // else
        //    ShowUIWindows(hWnd, FALSE);
        break;

    case WM_IME_STARTCOMPOSITION: // IME変換開始時。
        DPRINT("WM_IME_STARTCOMPOSITION\n");
        // Start composition! Ready to display the composition string.
        lpUIExtra = LockUIExtra(hWnd); // 余剰情報をロック。
        if (lpUIExtra) {
            lpIMC = TheIME.LockIMC(hIMC);
            CompWnd_Create(hWnd, lpUIExtra, lpIMC); // 未確定文字列ウィンドウを作成。
            TheIME.UnlockIMC(hIMC);
            UnlockUIExtra(hWnd); // 余剰情報のロックを解除。
        }
        break;

    case WM_IME_COMPOSITION: // IME変換時。
        DPRINT("WM_IME_COMPOSITION\n");
        // Update to display the composition string.
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            lpUIExtra = LockUIExtra(hWnd); // 余剰情報をロック。
            if (lpUIExtra) {
                CompWnd_Move(lpUIExtra, lpIMC); // 未確定文字列を移動。
                CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE); // 候補ウィンドウを移動。
                UnlockUIExtra(hWnd); // 余剰情報のロックを解除。
            }
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case WM_IME_ENDCOMPOSITION: // IME変換終了時。
        DPRINT("WM_IME_ENDCOMPOSITION\n");
        // Finish to display the composition string.
        lpUIExtra = LockUIExtra(hWnd); // 余剰情報をロック。
        if (lpUIExtra) {
            CompWnd_Hide(lpUIExtra); // 未確定文字列を隠す。
            UnlockUIExtra(hWnd); // 余剰情報のロックを解除。
        }
        break;

    case WM_IME_COMPOSITIONFULL: // 変換文字列がいっぱい。
        DPRINT("WM_IME_COMPOSITIONFULL\n");
        break;

    case WM_IME_SELECT: // IME選択時。
        DPRINT("WM_IME_SELECT\n");
        if (wParam) {
            lpUIExtra = LockUIExtra(hWnd); // 余剰情報をロック。
            if (lpUIExtra) {
                lpUIExtra->hIMC = hIMC; // hIMCをセット。
                UnlockUIExtra(hWnd); // 余剰情報のロックを解除。
            }
        }
        break;

    case WM_IME_CONTROL: // IME制御時。
        DPRINT("WM_IME_CONTROL\n");
        lRet = ControlCommand(hIMC, hWnd, wParam, lParam);
        break;

    case WM_IME_NOTIFY: // IME通知時。
        DPRINT("WM_IME_NOTIFY\n");
        lRet = NotifyCommand(hIMC, hWnd, wParam, lParam);
        break;

    case WM_DESTROY: // ウィンドウ破棄時。
        DPRINT("WM_DESTROY\n");
        OnDestroy(hWnd);
        break;

    case WM_UI_STATEMOVE: // IME状態ウィンドウが移動。
        // Set the position of the status window to UIExtra.
        // This message is sent by the status window.
        lpUIExtra = LockUIExtra(hWnd); // 余剰情報をロック。
        if (lpUIExtra) {
            // 位置を取得し、
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            // 覚えておく。
            POINT pt = { rc.left, rc.top };
            DPRINT("%d, %d\n", pt.x, pt.y);
            Config_SetData(L"ptStatusWindow", REG_BINARY, &pt, sizeof(pt));
            UnlockUIExtra(hWnd); // 余剰情報のロックを解除。
        }
        break;

    case WM_UI_DEFCOMPMOVE:
        // Set the position of the composition window to UIExtra.
        // This message is sent by the composition window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            if (!lpUIExtra->dwCompStyle) {
                RECT rc;
                ::GetWindowRect(lpUIExtra->hwndDefComp, &rc);
                POINT pt = { rc.left, rc.top };
                DPRINT("%d, %d\n", pt.x, pt.y);
                Config_SetData(L"ptDefComp", REG_BINARY, &pt, sizeof(pt));
            }
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_UI_CANDMOVE:
        // Set the position of the candidate window to UIExtra.
        // This message is sent by the candidate window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            // 位置を取得。
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndCand, &rc);
            // 位置を覚えておく。
            POINT pt = { rc.left, rc.top };
            DPRINT("%d, %d\n", pt.x, pt.y);
            lpUIExtra->ptCand = pt;
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_UI_GUIDEMOVE: // ガイドラインウィンドウが移動した。
        // Set the position of the status window to UIExtra.
        // This message is sent by the status window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            // 位置を取得。
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndGuide, &rc);
            // 位置を覚えておく。
            POINT pt = { rc.left, rc.top };
            Config_SetData(L"ptGuide", REG_BINARY, &pt, sizeof(pt));
            UnlockUIExtra(hWnd);
        }
        break;

    default: // その他のメッセージ。
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return lRet;
}

// フォントの高さを取得する。
int GetCompFontHeight(UIEXTRA *lpUIExtra)
{
    HDC hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL); // DCを作成する（情報のみ）。
    HFONT hOldFont = NULL;

    // フォントがあればフォントを選択。
    if (lpUIExtra->hFont) hOldFont = (HFONT)SelectObject(hIC, lpUIExtra->hFont);

    // サイズを取得。
    SIZE siz;
    GetTextExtentPoint(hIC, TEXT("A"), 1, &siz);

    if (hOldFont) SelectObject(hIC, hOldFont); // フォントの選択を解除する。

    DeleteDC(hIC); // DCを破棄する。
    return siz.cy; // これが高さ。
}

// WM_IME_NOTIFY メッセージを処理する。
LONG NotifyCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LONG ret = 0;
    RECT rc;
    LOGFONT lf;
    InputContext *lpIMC;

    UIEXTRA *lpUIExtra = LockUIExtra(hWnd); // 余剰情報。

    switch (wParam) {
    case IMN_CLOSESTATUSWINDOW: // 状態ウィンドウを閉じる。
        DPRINT("IMN_CLOSESTATUSWINDOW\n");
        if (::IsWindow(lpUIExtra->hwndStatus)) { // 状態ウィンドウが生きている？
            // 位置を保存。
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            POINT pt;
            pt.x = rc.left;
            pt.y = rc.top;
            Config_SetData(L"ptStatusWindow", REG_BINARY, &pt, sizeof(pt));

            ::ShowWindow(lpUIExtra->hwndStatus, SW_HIDE); // 実際には破棄されるのではなく隠される。
        }
        break;

    case IMN_OPENSTATUSWINDOW: // 状態ウィンドウが開かれる。
        DPRINT("IMN_OPENSTATUSWINDOW\n");
        StatusWnd_Create(hWnd, lpUIExtra); // 状態ウィンドウを作成する。
        break;

    case IMN_SETCONVERSIONMODE: // 変換モードがセットされる。
        DPRINT("IMN_SETCONVERSIONMODE\n");
        lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
        if (lpIMC) {
            // ローマ字モードを保存。
            if (lpIMC->Conversion() & IME_CMODE_ROMAN) {
                Config_SetDWORD(L"IsNonRoman", FALSE);
            } else {
                Config_SetDWORD(L"IsNonRoman", TRUE);
            }
            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。
        }
        StatusWnd_Update(lpUIExtra); // 余剰情報を更新。
        break;

    case IMN_SETSENTENCEMODE:
        DPRINT("IMN_SETSENTENCEMODE\n");
        break;

    case IMN_SETCOMPOSITIONFONT: // 未確定文字列のフォントがセットされる。
        DPRINT("IMN_SETCOMPOSITIONFONT\n");
        lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
        if (lpIMC) {
            lf = lpIMC->lfFont.W; // 論理フォント。
            if (lpUIExtra->hFont) DeleteObject(lpUIExtra->hFont); // すでにあれば破棄。

            // フォントの向き。
            if (lf.lfEscapement == 2700) // 270度。
                lpUIExtra->bVertical = TRUE; // 縦書き。
            else {
                lf.lfEscapement = 0;
                lpUIExtra->bVertical = FALSE; // 横書き。
            }

            // 現在のフォントが日本語でなければ別の日本語フォントを探す。
            if (lf.lfCharSet != SHIFTJIS_CHARSET) {
                lf.lfCharSet = SHIFTJIS_CHARSET;
                lf.lfFaceName[0] = 0;
            }

            lpUIExtra->hFont = CreateFontIndirect(&lf); // 論理フォントからフォントを作成。
            CompWnd_SetFont(lpUIExtra); // フォントを余剰情報にセット。
            CompWnd_Move(lpUIExtra, lpIMC); // 未確定文字列ウィンドウを移動。

            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除
        }
        break;

    case IMN_SETOPENSTATUS: // IMEのON/OFFを切り替え。
        DPRINT("IMN_SETOPENSTATUS\n");
        StatusWnd_Update(lpUIExtra); // 状態ウィンドウを更新する。
        break;

    case IMN_OPENCANDIDATE: // 候補が開かれる。
        DPRINT("IMN_OPENCANDIDATE\n");
        lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
        if (lpIMC) {
            CandWnd_Create(hWnd, lpUIExtra, lpIMC); // 候補ウィンドウを作成。
            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。
        }
        break;

    case IMN_CHANGECANDIDATE: // 候補が変更される。
        DPRINT("IMN_CHANGECANDIDATE\n");
        lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
        if (lpIMC) {
            CandWnd_Resize(lpUIExtra, lpIMC); // 候補ウィンドウのサイズを変更。
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE); // 候補ウィンドウを移動。
            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。
        }
        break;

    case IMN_CLOSECANDIDATE: // 候補が閉じられる。
        DPRINT("IMN_CLOSECANDIDATE\n");
        CandWnd_Hide(lpUIExtra); // 候補ウィンドウを隠す。
        break;

    case IMN_GUIDELINE:
        DPRINT("IMN_GUIDELINE\n");
        if (::ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0)) {
            if (!::IsWindow(lpUIExtra->hwndGuide)) {
                HDC hdcIC;
                TEXTMETRIC tm;
                int dx, dy;
                POINT pt;

                lpIMC = TheIME.LockIMC(hIMC);
                if (lpIMC) {
                    if (!Config_GetData(L"ptGuide", &pt, sizeof(pt))) {
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

    case IMN_SETCANDIDATEPOS: // 候補の位置がセットされる。
        DPRINT("IMN_SETCANDIDATEPOS\n");
        lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
        if (lpIMC) {
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE); // 候補ウィンドウを移動。
            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。
        }
        break;

    case IMN_SETCOMPOSITIONWINDOW: // 未確定文字列ウィンドウがセットされる。
        DPRINT("IMN_SETCOMPOSITIONWINDOW\n");
        lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
        if (lpIMC) {
            CompWnd_Move(lpUIExtra, lpIMC); // 未確定文字列ウィンドウを移動。
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE); // 候補ウィンドウも移動。
            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。
        }
        break;

    case IMN_SETSTATUSWINDOWPOS: // 状態ウィンドウの位置がセットされる。
        DPRINT("IMN_SETSTATUSWINDOWPOS\n");
        lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
        if (lpIMC) {
            // 位置をセットする。幅と高さはそのまま。
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            POINT pt = lpIMC->ptStatusWndPos;
            ::MoveWindow(lpUIExtra->hwndStatus, pt.x, pt.y,
                         rc.right - rc.left, rc.bottom - rc.top, TRUE);

            TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。
        }
        break;

    case IMN_PRIVATE: // プライベートな通知。
        DPRINT("IMN_PRIVATE\n");
        if (HIWORD(lParam) == 0xFACE) {
            std::wstring imepad_file;
            if (Config_GetSz(L"ImePadFile", imepad_file)) {
                ::ShellExecuteW(NULL, NULL, imepad_file.c_str(),
                                NULL, NULL, SW_SHOWNOACTIVATE);
            }
            break;
        }
        if (HIWORD(lParam) == 0xDEAD) {
            std::wstring verinfo_file;
            if (Config_GetSz(L"VerInfoFile", verinfo_file)) {
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

// WM_IME_CONTROL メッセージを処理する。
LONG ControlCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LONG ret = 1;

    InputContext *lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロックする。
    if (NULL == lpIMC) return ret;

    UIEXTRA *lpUIExtra = LockUIExtra(hWnd); // 余剰情報をロックする。
    if (lpUIExtra) {
        switch (wParam) {
        case IMC_GETCANDIDATEPOS: // 候補の位置が取得される。
            DPRINT("IMC_GETCANDIDATEPOS\n");
            if (IsWindow(lpUIExtra->hwndCand)) { // 候補ウィンドウが生きていれば
                *(LPCANDIDATEFORM)lParam = lpIMC->cfCandForm[0]; // 入力コンテキストから取得。
                ret = 0;
            }
            break;

        case IMC_GETCOMPOSITIONWINDOW: // 未確定文字列の構造体を取得。
            DPRINT("IMC_GETCOMPOSITIONWINDOW\n");
            *(LPCOMPOSITIONFORM)lParam = lpIMC->cfCompForm; // 入力コンテキストから取得。
            ret = 0;
            break;

        case IMC_GETSTATUSWINDOWPOS: // 状態ウィンドウの位置が取得される。
            DPRINT("IMC_GETSTATUSWINDOWPOS\n");
            {
                RECT rc;
                ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
                ret = MAKELONG(rc.left, rc.top); // 位置を返す。
            }
            break;

        default:
            break;
        }

        UnlockUIExtra(hWnd); // 余剰情報のロックを解除。
    }

    TheIME.UnlockIMC(hIMC); // 入力コンテキストのロックを解除。

    return ret;
}

// 子ウィンドウがドラッグされていれば、この関数はボーダーを描画する。
void DrawUIBorder(LPRECT lprc)
{
    HDC hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL); // DCを作成。
    SelectObject(hDC, GetStockObject(GRAY_BRUSH)); // ブラシを選択。これでPatBltで塗りつぶす。

    // ボーダー幅と高さ。
    INT sbx = GetSystemMetrics(SM_CXBORDER);
    INT sby = GetSystemMetrics(SM_CYBORDER);

    // 塗りつぶす。
    PatBlt(hDC, lprc->left, lprc->top, lprc->right - lprc->left - sbx, sby, PATINVERT);
    PatBlt(hDC, lprc->right - sbx, lprc->top, sbx, lprc->bottom - lprc->top - sby, PATINVERT);
    PatBlt(hDC, lprc->right, lprc->bottom - sby, -(lprc->right - lprc->left - sbx), sby, PATINVERT);
    PatBlt(hDC, lprc->left, lprc->bottom, sbx, -(lprc->bottom - lprc->top - sby), PATINVERT);

    DeleteDC(hDC); // DCを破棄。
}

// Handling mouse messages for the child windows
void DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, FIM_CAPUTURED);
        }
        break;

    case WM_MOUSEMOVE:
        dwT = (DWORD)::GetWindowLongPtr(hWnd, FIGWL_MOUSE);
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
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, dwT | FIM_MOVED);
        }
        break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        dwT = (DWORD)::GetWindowLongPtr(hWnd, FIGWL_MOUSE);

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

// IMEメッセージか（その１）？
// Any UI window should not pass the IME messages to DefWindowProc.
BOOL IsImeMessage(UINT message)
{
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

// IMEメッセージか（その２）？
BOOL IsImeMessage2(UINT message)
{
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
