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
    SWHT_NONE,
    SWHT_CAPTION,
    SWHT_BUTTON_1,
    SWHT_BUTTON_2,
    SWHT_BUTTON_3
};

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// Create status window.
// IME状態ウィンドウを作成する。
HWND StatusWnd_Create(HWND hWnd, UIEXTRA *lpUIExtra) {
    const DWORD style = WS_DISABLED | WS_POPUP;
    const DWORD exstyle = WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME;
    HWND hwndStatus = lpUIExtra->hwndStatus;
    if (!::IsWindow(hwndStatus)) {
        INT cx, cy;
        cx = CX_MINICAPTION + CX_BUTTON * 3;
        cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
        cx += 3 * CX_BTNEDGE * 2;
        cy = CY_BUTTON;
        cy += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
        cy += 2 * CY_BTNEDGE;
        POINT pt;
        if (!TheIME.GetUserData(L"ptStatusWindow", &pt, sizeof(pt))) {
            RECT rcWorkArea;
            ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
            pt.x = rcWorkArea.right - cx;
            pt.y = rcWorkArea.bottom - cy;
        }
        hwndStatus = ::CreateWindowEx(
                exstyle, szStatusClassName, NULL, style,
                pt.x, pt.y, cx, cy,
                hWnd, NULL, TheIME.m_hInst, NULL);
        lpUIExtra->hwndStatus = hwndStatus;
    } else {
        StatusWnd_Update(lpUIExtra);
    }
    RepositionWindow(hwndStatus);
    ::ShowWindow(hwndStatus, SW_SHOWNOACTIVATE);
    ::SetWindowLongPtr(hwndStatus, FIGWLP_SERVERWND, (LONG_PTR)hWnd);
    return hwndStatus;
} // StatusWnd_Create

// Draw status window.
// IME状態ウィンドウを描画する。
void StatusWnd_Paint(HWND hWnd, HDC hDC, INT nPushed) {
    RECT rc;
    HBITMAP hbmStatus;
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
    InputContext *lpIMC = TheIME.LockIMC(hIMC);

    // draw face
    HBRUSH hbr3DFace = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    ::GetClientRect(hWnd, &rc);
    ::FillRect(hDC, &rc, hbr3DFace);
    DeleteObject(hbr3DFace);

    // draw caption
    HBRUSH hbrCaption = ::CreateSolidBrush(RGB(0, 32, 255));
    rc.right = rc.left + CX_MINICAPTION;
    ::FillRect(hDC, &rc, hbrCaption);
    ::DeleteObject(hbrCaption);

    ::GetClientRect(hWnd, &rc);
    hbmStatus = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
    HDC hMemDC = ::CreateCompatibleDC(hDC);
    if (hMemDC) {
        RECT rcButton;
        HGDIOBJ hbmOld = ::SelectObject(hMemDC, hbmStatus);

        rc.left += CX_MINICAPTION;
        rcButton.left = rc.left;
        rcButton.top = rc.top;
        rcButton.right = rc.left + CX_BUTTON + 4;
        rcButton.bottom = rc.bottom;
        if (nPushed == 1) {
            ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                               DFCS_BUTTONPUSH | DFCS_PUSHED);
        } else {
            ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH);
        }

        rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
        rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
        if (nPushed == 2) {
            ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                               DFCS_BUTTONPUSH | DFCS_PUSHED);
        } else {
            ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                               DFCS_BUTTONPUSH);
        }

        rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
        rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
        if (nPushed == 3) {
            ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                               DFCS_BUTTONPUSH | DFCS_PUSHED);
        } else {
            ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                               DFCS_BUTTONPUSH);
        }

        // draw ime on/off
        if (lpIMC) {
            if (lpIMC->IsOpen()) {
                ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                         CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 7 * CY_BUTTON, SRCCOPY);
            } else {
                ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                         CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 8 * CY_BUTTON, SRCCOPY);
            }
        } else {
            // disabled
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
        }

        // draw input mode
        rc.left += CX_BUTTON + CX_BTNEDGE * 2;
        if (lpIMC) {
            if (lpIMC->IsOpen()) {
                if (lpIMC->Conversion() & IME_CMODE_FULLSHAPE) {
                    if (lpIMC->Conversion() & IME_CMODE_JAPANESE) {
                        if (lpIMC->Conversion() & IME_CMODE_KATAKANA) {
                            // fullwidth katakana
                            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                     CX_BUTTON, CY_BUTTON,
                                     hMemDC, 0, 1 * CY_BUTTON, SRCCOPY);
                        } else {
                            // fullwidth hiragana
                            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                     CX_BUTTON, CY_BUTTON,
                                     hMemDC, 0, 0 * CY_BUTTON, SRCCOPY);
                        }
                    } else {
                        // fullwidth alphanumeric
                        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                 CX_BUTTON, CY_BUTTON,
                                 hMemDC, 0, 2 * CY_BUTTON, SRCCOPY);
                    }
                } else {
                    if (lpIMC->Conversion() & IME_CMODE_JAPANESE) {
                        // halfwidth kana
                        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                 CX_BUTTON, CY_BUTTON,
                                 hMemDC, 0, 3 * CY_BUTTON, SRCCOPY);
                    } else {
                        // halfwidth alphanumeric
                        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                 CX_BUTTON, CY_BUTTON,
                                 hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
                    }
                }
            } else {
                // halfwidth alphanumeric
                ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                         CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
            }
        } else {
            // disabled
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
        }

        // draw roman mode
        rc.left += CX_BUTTON + CX_BTNEDGE * 2;
        if (lpIMC) {
            if (lpIMC->Conversion() & IME_CMODE_ROMAN) {
                ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                         CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 5 * CY_BUTTON, SRCCOPY);
            } else {
                ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                         CX_BUTTON, CY_BUTTON,
                         hMemDC, 0, 6 * CY_BUTTON, SRCCOPY);
            }
        } else {
            // disabled
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
        }

        ::SelectObject(hMemDC, hbmOld);
        ::DeleteDC(hMemDC);
    }
    if (lpIMC) TheIME.UnlockIMC(hIMC);
} // StatusWnd_Paint

// IME状態ウィンドウの当たり判定。
STATUS_WND_HITTEST StatusWnd_HitTest(HWND hWnd, POINT pt) {
    ::ScreenToClient(hWnd, &pt);
    RECT rc;
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION;
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_1;
    }
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION + CX_BUTTON + 2 * CX_BTNEDGE;
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_2;
    }
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION + 2 * (CX_BUTTON + 2 * CX_BTNEDGE);
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_3;
    }
    ::GetWindowRect(hWnd, &rc);
    ::ClientToScreen(hWnd, &pt);
    if (::PtInRect(&rc, pt)) {
        return SWHT_CAPTION;
    }
    return SWHT_NONE;
} // StatusWnd_HitTest

// IME状態ウィンドウの位置を更新。
void StatusWnd_Update(UIEXTRA *lpUIExtra) {
    HWND hwndStatus = lpUIExtra->hwndStatus;
    if (::IsWindow(hwndStatus)) {
        POINT pt;
        if (TheIME.GetUserData(L"ptStatusWindow", &pt, sizeof(pt))) {
            RECT rc;
            ::GetWindowRect(hwndStatus, &rc);
            ::MoveWindow(hwndStatus, pt.x, pt.y,
                         rc.right - rc.left, rc.bottom - rc.top, TRUE);
        }
        ::SendMessage(hwndStatus, WM_UI_UPDATE, 0, 0);
    }
} // StatusWnd_Update

// IME状態ウィンドウのボタンを押したときの動作。
void StatusWnd_OnButton(HWND hWnd, STATUS_WND_HITTEST hittest) {
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
    if (hIMC == NULL) {
        return;
    }
    DWORD dwConversion, dwSentence;
    BOOL bOpen = ImmGetOpenStatus(hIMC);
    if (::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence)) {
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
}

// IME状態ウィンドウ上でマウスが移動している。
void StatusWnd_OnMouseMove(HWND hWnd, POINT pt, BOOL bDown) {
    static POINT prev = {-1, -1};
    if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) { // キャプションをドラッグしている。
        if (bDown && ::GetCapture() == hWnd) {
            if (prev.x != -1 && prev.y != -1) {
                RECT rc;
                ::GetWindowRect(hWnd, &rc);
                ::MoveWindow(hWnd,
                             rc.left + (pt.x - prev.x), rc.top + (pt.y - prev.y),
                             rc.right - rc.left, rc.bottom - rc.top,
                             TRUE);
            }
            prev = pt;
        } else { // それ以外。
            prev.x = -1;
            prev.y = -1;
            ::ReleaseCapture();
            ::SetWindowLong(hWnd, FIGWL_MOUSE, SWHT_NONE);
        }
    }
}

// IME状態ウィンドウで左ボタンを押された／離された。
void StatusWnd_OnLButton(HWND hWnd, POINT pt, BOOL bDown) {
    STATUS_WND_HITTEST hittest = StatusWnd_HitTest(hWnd, pt); // 当たり判定を行う。
    switch (hittest) {
    case SWHT_CAPTION: // キャプション上。
        break;
    case SWHT_BUTTON_1: // ボタン1。
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_1) {
            // 再描画。
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Paint(hWnd, hDC, (bDown ? 1 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_BUTTON_2: // ボタン2。
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_2) {
            // 再描画。
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Paint(hWnd, hDC, (bDown ? 2 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_BUTTON_3: // ボタン3。
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_3) {
            // 再描画。
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Paint(hWnd, hDC, (bDown ? 3 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_NONE: // それ以外。
    {
        // 再描画。
        HDC hDC = ::GetDC(hWnd);
        StatusWnd_Paint(hWnd, hDC, 0);
        ::ReleaseDC(hWnd, hDC);
    }
    break;
    }
    if (bDown) { // 押された。
        ::SetCapture(hWnd); // ドラッグを開始するため、キャプチャーをセットする。
        ::SetWindowLong(hWnd, FIGWL_MOUSE, hittest); // 押された位置を覚えておく。
    } else { // 離された。
        ::ReleaseCapture(); // キャプチャーを解放し、ドラッグを終了する。
        if (hittest == SWHT_CAPTION) { // キャプション上であれば
            RepositionWindow(hWnd); // 位置を補正する。
        } else { // さもなければ
            StatusWnd_OnButton(hWnd, hittest); // ボタンのアクションを発動する。
            ::SetWindowLong(hWnd, FIGWL_MOUSE, SWHT_NONE); // 押された位置をクリアする。
        }
    }
} // StatusWnd_OnLButton

// IME状態ウィンドウを右クリックされた。
static BOOL StatusWnd_OnRClick(HWND hWnd, POINT pt) {
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

    TPMPARAMS params = { sizeof(params); };
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
StatusWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HWND hwndServer;
    HDC hDC;
    HBITMAP hbm;
    POINT pt;

    switch (message) {
    case WM_CREATE: // ウィンドウ作成時。
        // ビットマップを読み込み、セットする。
        hbm = TheIME.LoadBMP(TEXT("MODESBMP"));
        SetWindowLongPtr(hWnd, FIGWLP_STATUSBMP, (LONG_PTR)hbm);
        break;

    case WM_PAINT: // 描画時。
        hDC = ::BeginPaint(hWnd, &ps);
        StatusWnd_Paint(hWnd, hDC, 0);
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
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
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
        case WM_MOUSEMOVE:
            if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
                StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
            }
            break;
        case WM_LBUTTONDOWN:
            StatusWnd_OnLButton(hWnd, pt, TRUE);
            break;
        case WM_LBUTTONUP:
            StatusWnd_OnLButton(hWnd, pt, FALSE);
            break;
        case WM_RBUTTONDOWN:
            break;
        case WM_RBUTTONUP:
            StatusWnd_OnRClick(hWnd, pt);
            break;
        }
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
        break;

    case WM_MOVE: // ウィンドウ移動時。
        hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (::IsWindow(hwndServer))
            SendMessage(hwndServer, WM_UI_STATEMOVE, wParam, lParam); // UIサーバーに送る。
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
