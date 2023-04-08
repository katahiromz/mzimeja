// uistate.cpp --- mzimeja status window UI
// IME状態ウィンドウ。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

// 寸法（ピクセル単位）。
#define CX_MINICAPTION 10
#define CX_BUTTON 24
#define CY_BUTTON 24
#define CX_BTNEDGE 2
#define CY_BTNEDGE 2

// 当たり判定の結果。
enum STATUS_WND_HITTEST {
    SWHT_NONE,      // なにも当たっていない。
    SWHT_CAPTION,   // キャプション。
    SWHT_BUTTON_1,  // ボタン1。
    SWHT_BUTTON_2,  // ボタン2。
    SWHT_BUTTON_3   // ボタン3。
};

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// Create status window.
// IME状態ウィンドウを作成する。
HWND StatusWnd_Create(HWND hWnd, UIEXTRA *lpUIExtra)
{
    const DWORD style = WS_DISABLED | WS_POPUP; // ウィンドウスタイル。
    const DWORD exstyle = WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME; // 拡張スタイル。
    HWND hwndStatus = lpUIExtra->hwndStatus;
    if (!::IsWindow(hwndStatus)) { // 状態ウィンドウがないか？
        // 状態ウィンドウのサイズを計算する。
        INT cx, cy;
        cx = CX_MINICAPTION + CX_BUTTON * 3;
        cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
        cx += 3 * CX_BTNEDGE * 2;
        cy = CY_BUTTON;
        cy += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
        cy += 2 * CY_BTNEDGE;
        POINT pt;
        if (!Config_GetData(L"ptStatusWindow", &pt, sizeof(pt))) { // 位置情報があるか？
            // ワークエリアを使って位置を初期化する。
            RECT rcWorkArea;
            ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
            pt.x = rcWorkArea.right - cx;
            pt.y = rcWorkArea.bottom - cy;
        }
        // 実際に状態ウィンドウを作成する。
        lpUIExtra->hwndStatus = hwndStatus = ::CreateWindowEx(
                exstyle, szStatusClassName, NULL, style,
                pt.x, pt.y, cx, cy,
                hWnd, NULL, TheIME.m_hInst, NULL);
    } else {
        StatusWnd_Update(lpUIExtra); // 状態ウィンドウを更新する。
    }
    RepositionWindow(hwndStatus); // 位置を補正する。
    ::ShowWindow(hwndStatus, SW_SHOWNOACTIVATE); // アクティブ化することなく表示する。
    ::SetWindowLongPtr(hwndStatus, FIGWLP_SERVERWND, (LONG_PTR)hWnd); // UIサーバーをセットする。
    return hwndStatus;
} // StatusWnd_Create

// IME状態ウィンドウを描画する。
static void StatusWnd_Draw(HWND hWnd, HDC hDC, INT nPushed)
{
    // クライアント領域を取得。
    RECT rcClient;
    ::GetClientRect(hWnd, &rcClient);
    RECT rc = rcClient;

    // ちらつきを防止するため、メモリービットマップを使用する。
    HDC hdcMem2 = ::CreateCompatibleDC(hDC);
    HBITMAP hbm = ::CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
    HGDIOBJ hbm2Old = ::SelectObject(hdcMem2, hbm);

    // UIサーバーとIMCを取得する。
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
    InputContext *lpIMC = TheIME.LockIMC(hIMC); // 入力コンテキストをロック。

    // クライアント領域を塗りつぶす。
    ::FillRect(hdcMem2, &rc, (HBRUSH)(COLOR_3DFACE + 1));

    // 青っぽい色でミニタイトルバーを塗りつぶす。
    HBRUSH hbrCaption = ::CreateSolidBrush(RGB(0, 32, 255));
    rc.right = rc.left + CX_MINICAPTION;
    ::FillRect(hdcMem2, &rc, hbrCaption);
    ::DeleteObject(hbrCaption);

    // クライアント領域を再取得する。ミニタイトルバー領域を除外する。
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION;

    // 状態ビットマップ選択用のメモリーDCを作成する。
    HDC hdcMem1 = ::CreateCompatibleDC(hdcMem2);
    ASSERT(hdcMem1 != NULL);

    // 状態ビットマップを選択する。
    HBITMAP hbmStatus = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
    HGDIOBJ hbm1Old = ::SelectObject(hdcMem1, hbmStatus);
    ASSERT(hbmStatus != NULL);

    // 最初のボタン1の背景。
    RECT rcButton;
    rcButton.left = rc.left;
    rcButton.top = rc.top;
    rcButton.right = rc.left + CX_BUTTON + 4;
    rcButton.bottom = rc.bottom;
    if (nPushed == 1) {
        ::DrawFrameControl(hdcMem2, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
        ::DrawFrameControl(hdcMem2, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH);
    }

    // ボタン2の背景。
    rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
    rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
    if (nPushed == 2) {
        ::DrawFrameControl(hdcMem2, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
        ::DrawFrameControl(hdcMem2, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH);
    }

    // ボタン3の背景。
    rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
    rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
    if (nPushed == 3) {
        ::DrawFrameControl(hdcMem2, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
        ::DrawFrameControl(hdcMem2, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH);
    }

    // IMEのOn/Offをビットマップで描画する。
    if (lpIMC) {
        if (lpIMC->IsOpen()) {
            ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hdcMem1, 0, 7 * CY_BUTTON, SRCCOPY);
        } else {
            ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hdcMem1, 0, 8 * CY_BUTTON, SRCCOPY);
        }
    } else {
        ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hdcMem1, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    // 入力モードをビットマップで描画する。
    rc.left += CX_BUTTON + CX_BTNEDGE * 2;
    if (lpIMC) { // 入力コンテキストが有効か？
        if (lpIMC->IsOpen()) { // 開かれているか？
            if (lpIMC->Conversion() & IME_CMODE_FULLSHAPE) { // 全角か？
                if (lpIMC->Conversion() & IME_CMODE_JAPANESE) { // 日本語入力か？
                    if (lpIMC->Conversion() & IME_CMODE_KATAKANA) { // カタカナか？
                        ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                 CX_BUTTON, CY_BUTTON,
                                 hdcMem1, 0, 1 * CY_BUTTON, SRCCOPY);
                    } else { // ひらがなか？
                        ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                 CX_BUTTON, CY_BUTTON,
                                 hdcMem1, 0, 0 * CY_BUTTON, SRCCOPY);
                    }
                } else { // 全角英数入力か？
                    ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                             CX_BUTTON, CY_BUTTON,
                             hdcMem1, 0, 2 * CY_BUTTON, SRCCOPY);
                }
            } else { // 半角入力か？
                if (lpIMC->Conversion() & IME_CMODE_JAPANESE) { // 半角カナか？
                    ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                             CX_BUTTON, CY_BUTTON,
                             hdcMem1, 0, 3 * CY_BUTTON, SRCCOPY);
                } else { // 半角英数か？
                    ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                             CX_BUTTON, CY_BUTTON,
                             hdcMem1, 0, 4 * CY_BUTTON, SRCCOPY);
                }
            }
        } else { // 閉じられているか？
            ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE, CX_BUTTON, CY_BUTTON,
                     hdcMem1, 0, 4 * CY_BUTTON, SRCCOPY);
        }
    } else { // 入力コンテキストがない。
        ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE, CX_BUTTON, CY_BUTTON,
                 hdcMem1, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    // ローマ字入力をビットマップで描画する。
    rc.left += CX_BUTTON + CX_BTNEDGE * 2;
    if (lpIMC) { // 入力コンテキストが有効か？
        if (lpIMC->Conversion() & IME_CMODE_ROMAN) { // ローマ字入力か？
            ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE, CX_BUTTON, CY_BUTTON,
                     hdcMem1, 0, 5 * CY_BUTTON, SRCCOPY);
        } else {
            ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hdcMem1, 0, 6 * CY_BUTTON, SRCCOPY);
        }
    } else { // 入力コンテキストが無効か？
        ::BitBlt(hdcMem2, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE, CX_BUTTON, CY_BUTTON,
                 hdcMem1, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    // ビットを転送する（hDC←hdcMem2）。
    ::BitBlt(hDC, 0, 0, rcClient.right, rcClient.bottom, hdcMem2, 0, 0, SRCCOPY);

    // 後始末。
    ::DeleteObject(::SelectObject(hdcMem1, hbm1Old));
    ::DeleteDC(hdcMem1);
    ::DeleteObject(::SelectObject(hdcMem2, hbm2Old));
    ::DeleteDC(hdcMem2);

    if (lpIMC) TheIME.UnlockIMC(hIMC);
} // StatusWnd_Draw

// IME状態ウィンドウの当たり判定。
static STATUS_WND_HITTEST StatusWnd_HitTest(HWND hWnd, POINT pt)
{
    ::ScreenToClient(hWnd, &pt);
    RECT rc;
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION;
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_1; // ボタン1。
    }
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION + CX_BUTTON + 2 * CX_BTNEDGE;
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_2; // ボタン2。
    }
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION + 2 * (CX_BUTTON + 2 * CX_BTNEDGE);
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_3; // ボタン3。
    }
    ::GetWindowRect(hWnd, &rc);
    ::ClientToScreen(hWnd, &pt);
    if (::PtInRect(&rc, pt)) {
        return SWHT_CAPTION; // キャプション。
    }
    return SWHT_NONE;
} // StatusWnd_HitTest

// IME状態ウィンドウの位置を更新。
void StatusWnd_Update(UIEXTRA *lpUIExtra)
{
    HWND hwndStatus = lpUIExtra->hwndStatus;
    if (::IsWindow(hwndStatus)) {
        // 設定データ "ptStatusWindow" を使って、ウィンドウの位置を復元する。
        POINT pt;
        if (Config_GetData(L"ptStatusWindow", &pt, sizeof(pt))) {
            RECT rc;
            ::GetWindowRect(hwndStatus, &rc);
            if (pt.x != rc.left || pt.y != rc.top) {
                ::MoveWindow(hwndStatus, pt.x, pt.y,
                             rc.right - rc.left, rc.bottom - rc.top, TRUE);
            }
        }
        ::SendMessage(hwndStatus, WM_UI_UPDATE, 0, 0);
    }
} // StatusWnd_Update

// IME状態ウィンドウのボタンを押したときの動作。
static void StatusWnd_OnButton(HWND hWnd, STATUS_WND_HITTEST hittest)
{
    // UIサーバーウィンドウとIMCを取得する。
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND); // サーバーウィンドウ。
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC); // IMC。
    if (hIMC == NULL) {
        ASSERT(0);
        return;
    }

    // IMEの状態を取得する。
    DWORD dwConversion, dwSentence;
    BOOL bOpen = ImmGetOpenStatus(hIMC);
    if (!::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence)) {
        ASSERT(0);
        return;
    }

    INPUT_MODE imode;
    switch (hittest) {
    case SWHT_BUTTON_1:
        // 変換モードを切り替える。
        if (bOpen) {
            SetInputMode(hIMC, IMODE_HALF_ASCII);
        } else {
            SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
        }
        break;
    case SWHT_BUTTON_2:
        // 入力モードを切り替える。
        imode = InputModeFromConversionMode(bOpen, dwConversion);
        imode = NextInputMode(imode);
        SetInputMode(hIMC, imode);
        break;
    case SWHT_BUTTON_3:
        // ローマ字入力モードを切り替える。
        if (dwConversion & IME_CMODE_ROMAN) {
            dwConversion &= ~IME_CMODE_ROMAN;
        } else {
            dwConversion |= IME_CMODE_ROMAN;
        }
        ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
        break;
    default:
        break;
    }
}

// IME状態ウィンドウ上でマウスが移動している。
static void StatusWnd_OnMouseMove(HWND hWnd, POINT pt, BOOL bDown)
{
    static POINT prev = {-1, -1}; // 一つ前の位置。
    if (::GetWindowLongPtr(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) { // キャプションをドラッグしている。
        if (bDown && ::GetCapture() == hWnd) { // ドラッグ中か？
            if (prev.x != -1 && prev.y != -1) { // 一つ前の位置があるか？
                // ウィンドウの位置をずらす。
                RECT rc;
                ::GetWindowRect(hWnd, &rc);
                ::MoveWindow(hWnd,
                             rc.left + (pt.x - prev.x), rc.top + (pt.y - prev.y),
                             rc.right - rc.left, rc.bottom - rc.top,
                             TRUE);
            }
            prev = pt;
        } else { // それ以外。ドラッグをキャンセルする。
            prev.x = -1;
            prev.y = -1;
            ::ReleaseCapture();
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, SWHT_NONE);
        }
    }
}

// IME状態ウィンドウで左ボタンを押された／離された。
static void StatusWnd_OnLButton(HWND hWnd, POINT pt, BOOL bDown)
{
    STATUS_WND_HITTEST hittest = StatusWnd_HitTest(hWnd, pt); // 当たり判定を行う。
    switch (hittest) {
    case SWHT_CAPTION: // キャプション上。
        break;
    case SWHT_BUTTON_1: // ボタン1。
        if (::GetWindowLongPtr(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_1) {
            // 再描画。
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Draw(hWnd, hDC, (bDown ? 1 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_BUTTON_2: // ボタン2。
        if (::GetWindowLongPtr(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_2) {
            // 再描画。
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Draw(hWnd, hDC, (bDown ? 2 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_BUTTON_3: // ボタン3。
        if (::GetWindowLongPtr(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_3) {
            // 再描画。
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Draw(hWnd, hDC, (bDown ? 3 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_NONE: // それ以外。
    {
        // 再描画。
        HDC hDC = ::GetDC(hWnd);
        StatusWnd_Draw(hWnd, hDC, 0);
        ::ReleaseDC(hWnd, hDC);
    }
    break;
    }
    if (bDown) { // 押された。
        ::SetCapture(hWnd); // ドラッグを開始するため、キャプチャーをセットする。
        ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, hittest); // 押された位置を覚えておく。
    } else { // 離された。
        ::ReleaseCapture(); // キャプチャーを解放し、ドラッグを終了する。
        if (hittest == SWHT_CAPTION) { // キャプション上であれば
            RepositionWindow(hWnd); // 位置を補正する。
        } else { // さもなければ
            StatusWnd_OnButton(hWnd, hittest); // ボタンのアクションを発動する。
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, SWHT_NONE); // 押された位置をクリアする。
        }
    }
} // StatusWnd_OnLButton

// IME状態ウィンドウを右クリックされた。
static BOOL StatusWnd_OnRClick(HWND hWnd, POINT pt)
{
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND); // IME UIサーバーウィンドウ。
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC); // IMC。
    if (hIMC == NULL) {
        ASSERT(0);
        return FALSE;
    }

    HMENU hMenu = ::LoadMenu(TheIME.m_hInst, TEXT("STATUSRMENU")); // メニューをリソースから読み込む。
    if (hMenu == NULL) {
        ASSERT(0);
        return FALSE;
    }

    HMENU hSubMenu = ::GetSubMenu(hMenu, 0); // インデックス0の子メニューを取得。

    TPMPARAMS params = { sizeof(params) };
    ::GetWindowRect(hWnd, &params.rcExclude);

    HWND hwndFore = ::GetForegroundWindow(); // 最前面ウィンドウを覚えておく。
    ::SetForegroundWindow(hWnd); // TrackPopupMenuExのバグを回避。

    // メニューの入力モードにラジオマークを付ける。
    UINT uCheck = CommandFromInputMode(GetInputMode(hIMC));
    ::CheckMenuRadioItem(hSubMenu, IDM_HIRAGANA, IDM_HALF_ASCII, uCheck, MF_BYCOMMAND);
    if (IsRomanMode(hIMC)) {
        ::CheckMenuRadioItem(hSubMenu, IDM_ROMAN_INPUT, IDM_KANA_INPUT,
                             IDM_ROMAN_INPUT, MF_BYCOMMAND);
    } else {
        ::CheckMenuRadioItem(hSubMenu, IDM_ROMAN_INPUT, IDM_KANA_INPUT,
                             IDM_KANA_INPUT, MF_BYCOMMAND);
    }

    // メニューを表示して選択されるのを待つ。選択されたらコマンドIDを返す。
    UINT nCommand = ::TrackPopupMenuEx(hSubMenu, TPM_RETURNCMD | TPM_NONOTIFY,
                                       pt.x, pt.y, hWnd, &params);
    TheIME.DoCommand(hIMC, nCommand); // コマンド発動。
    ::PostMessage(hWnd, WM_NULL, 0, 0); // TrackPopupMenuExのバグ回避。
    ::DestroyMenu(hMenu); // メニューを破棄する。
    ::SetForegroundWindow(hwndFore); // 最前面ウィンドウを戻す。
    return TRUE;
} // StatusWnd_OnRClick

// IME状態ウィンドウのウィンドウプロシージャ。
LRESULT CALLBACK
StatusWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HWND hwndServer;
    HDC hDC;
    HBITMAP hbm;
    POINT pt;

    switch (message) {
    case WM_CREATE: // ウィンドウ作成時。
        // ビットマップを読み込み、セットする。
        hbm = TheIME.LoadBMP(TEXT("MODESBMP"));
        ::SetWindowLongPtr(hWnd, FIGWLP_STATUSBMP, (LONG_PTR)hbm);
        break;

    case WM_ERASEBKGND:
        return TRUE; // ちらつきを防止するため、ここで背景を描画しない。

    case WM_PAINT: // 描画時。
        hDC = ::BeginPaint(hWnd, &ps);
        StatusWnd_Draw(hWnd, hDC, 0);
        ::EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY: // ウィンドウ破棄時。
        hbm = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
        ::DeleteObject(hbm);
        break;

    case WM_UI_UPDATE: // UI更新時。
        ::InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_LBUTTONUP: // 左ボタン解放時。
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        StatusWnd_OnLButton(hWnd, pt, FALSE);
        break;

    case WM_LBUTTONDOWN: // 左ボタンが押された時。
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        StatusWnd_OnLButton(hWnd, pt, TRUE);
        break;

    case WM_MOUSEMOVE: // マウス移動時。特にドラッグ時。
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        if (::GetWindowLongPtr(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
            StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
        }
        break;

    case WM_RBUTTONUP: // 右ボタン解放時。
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        break;

    case WM_RBUTTONDOWN: // 右ボタンが押された時。
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        break;

    case WM_SETCURSOR: // マウスカーソル設定時。
        // This message comes even from the disabled window.
        // このメッセージは無効なウィンドウでも来る。
        ::GetCursorPos(&pt);
        switch (HIWORD(lParam)) {
        case WM_MOUSEMOVE: // マウス移動時。
            if (::GetWindowLongPtr(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
                StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
            }
            break;
        case WM_LBUTTONDOWN: // 左ボタンが押された時。
            StatusWnd_OnLButton(hWnd, pt, TRUE);
            break;
        case WM_LBUTTONUP: // 左ボタン解放時。
            StatusWnd_OnLButton(hWnd, pt, FALSE);
            break;
        case WM_RBUTTONDOWN: // 右ボタンが押された時。
            break;
        case WM_RBUTTONUP: // 右ボタン解放時。
            StatusWnd_OnRClick(hWnd, pt);
            break;
        }
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW)); // 矢印カーソルを指定。
        break;

    case WM_MOVE: // ウィンドウ移動時。
        hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (::IsWindow(hwndServer))
            SendMessage(hwndServer, WM_UI_STATEMOVE, 0, 0); // UIサーバーに送る。
        break;

    default: // その他のメッセージ。
        if (!IsImeMessage(message))
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
} // StatusWnd_WindowProc

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
