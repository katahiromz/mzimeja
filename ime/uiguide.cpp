// uiguide.cpp --- mzimeja guideline window UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// ガイドラインウィンドウのウィンドウプロシージャ。
LRESULT CALLBACK GuideWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                     LPARAM lParam)
{
    PAINTSTRUCT ps;
    HWND hUIWnd;
    HDC hDC;
    HBITMAP hbmpGuide;
    RECT rc;

    switch (message) {
    case WM_UI_HIDE: // UIを隠したい時。
        ShowWindow(hWnd, SW_HIDE);
        break;

    case WM_UI_UPDATE: // UIを更新する時。
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT: // 描画時。
        hDC = BeginPaint(hWnd, &ps);
        GuideWnd_Paint(hWnd, hDC, NULL, 0);
        EndPaint(hWnd, &ps);
        break;

    case WM_MOUSEMOVE: // マウス移動時。
    case WM_SETCURSOR: // カーソル設定時。
    case WM_LBUTTONUP: // 左ボタン解放時。
    case WM_RBUTTONUP: // 右ボタン解放時。
        GuideWnd_Button(hWnd, message, wParam, lParam); // マウスアクション。
        if ((message == WM_SETCURSOR) && (HIWORD(lParam) != WM_LBUTTONDOWN) &&
            (HIWORD(lParam) != WM_RBUTTONDOWN))
            return DefWindowProc(hWnd, message, wParam, lParam);
        if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP)) {
            // 状態を戻す。
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, 0);
            ::SetWindowLongPtr(hWnd, FIGWL_PUSHSTATUS, 0);
        }
        break;

    case WM_MOVE: // ウィンドウ移動時。
        hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (IsWindow(hUIWnd))
            SendMessage(hUIWnd, WM_UI_GUIDEMOVE, 0, 0); // UIサーバーに送る。
        break;

    case WM_CREATE: // ウィンドウ作成時。
        hbmpGuide = TheIME.LoadBMP(TEXT("CLOSEBMP")); // ビットマップを読み込み、保存する。
        ::SetWindowLongPtr(hWnd, FIGWLP_CLOSEBMP, (LONG_PTR)hbmpGuide);
        GetClientRect(hWnd, &rc);
        break;

    case WM_DESTROY: // ウィンドウ破棄時。
        hbmpGuide = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_CLOSEBMP);
        DeleteObject(hbmpGuide); // ビットマップを破棄する。
        break;

    default: // それ以外のメッセージ。
        if (!IsImeMessage(message))
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

DWORD PASCAL CheckPushedGuide(HWND hGuideWnd, LPPOINT lppt)
{
    POINT pt;
    RECT rc;

    if (lppt) {
        pt = *lppt;
        ScreenToClient(hGuideWnd, &pt);
        GetClientRect(hGuideWnd, &rc);
        if (!PtInRect(&rc, pt)) return 0;

        rc.left = rc.right - STCLBT_DX - 2;
        rc.top = STCLBT_Y;
        rc.right = rc.left + STCLBT_DX;
        rc.bottom = rc.top + STCLBT_DY;
        if (PtInRect(&rc, pt)) return PUSHED_STATUS_CLOSE;
    }
    return 0;
}

// ガイドラインの描画時。
void GuideWnd_Paint(HWND hGuideWnd, HDC hDC, LPPOINT lppt,
                    DWORD dwPushedGuide)
{
    HBITMAP hbmpOld;
    HANDLE hGLStr;
    WCHAR *lpGLStr;
    DWORD dwLevel;
    DWORD dwSize;

    HWND hSvrWnd = (HWND)GetWindowLongPtr(hGuideWnd, FIGWLP_SERVERWND); // UIサーバー。

    HIMC hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC); // IMC。
    if (hIMC == NULL) {
        ASSERT(0);
        return;
    }

    HBITMAP hbmpGuide;
    HBRUSH hOldBrush, hBrush;
    int nCyCap = GetSystemMetrics(SM_CYSMCAPTION); // 小さいキャプションのタテ寸法。

    HDC hMemDC = CreateCompatibleDC(hDC); // メモリーDCを作成。

    // キャプションを描画する。
    hBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION)); // ブラシを作成。
    hOldBrush = (HBRUSH)SelectObject(hDC, hBrush); // ブラシ選択。
    RECT rc;
    GetClientRect(hGuideWnd, &rc); // クライアント領域を取得。
    // rc.top = rc.left = 0;
    // rc.right = BTX*3;
    rc.bottom = nCyCap; // キャプションの領域にする。
    FillRect(hDC, &rc, hBrush); // 塗りつぶす。
    SelectObject(hDC, hOldBrush); // ブラシ選択を解除。
    DeleteObject(hBrush); // ブラシを破棄。

    // 閉じるボタンのビットマップを取得。
    hbmpGuide = (HBITMAP)GetWindowLongPtr(hGuideWnd, FIGWLP_CLOSEBMP);
    hbmpOld = (HBITMAP)SelectObject(hMemDC, hbmpGuide); // ビットマップを選択。

    // 状態に応じてビットマップで描画。
    if (!(dwPushedGuide & PUSHED_STATUS_CLOSE))
        BitBlt(hDC, rc.right - STCLBT_DX - 2, STCLBT_Y, STCLBT_DX, STCLBT_DY,
               hMemDC, 0, 0, SRCCOPY);
    else
        BitBlt(hDC, rc.right - STCLBT_DX - 2, STCLBT_Y, STCLBT_DX, STCLBT_DY,
               hMemDC, STCLBT_DX, 0, SRCCOPY);

    // ガイドラインのレベルを取得。
    dwLevel = ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0);
    if (dwLevel) {
        // ガイドラインの文字列のサイズを取得。
        dwSize = ImmGetGuideLine(hIMC, GGL_STRING, NULL, 0) + 1;
        // 文字列に対するメモリー割り当て。
        if ((dwSize > 1) && (hGLStr = GlobalAlloc(GHND, dwSize))) {
            lpGLStr = (LPTSTR)GlobalLock(hGLStr); // メモリーをロック。
            if (lpGLStr) {
                COLORREF rgb = 0;
                HBRUSH hbrLGR = (HBRUSH)GetStockObject(LTGRAY_BRUSH); // ブラシを取得。
                HBRUSH hbr;

                hbr = (HBRUSH)SelectObject(hDC, hbrLGR); // ブラシ選択。
                GetClientRect(hGuideWnd, &rc); // クライアント領域を取得。
                // ブラシで塗りつぶし。
                PatBlt(hDC, 0, nCyCap, rc.right, rc.bottom - nCyCap, PATCOPY);
                SelectObject(hDC, hbr); // ブラシ選択を解除。

                // レベルに応じて色を変える。
                switch (dwLevel) {
                case GL_LEVEL_FATAL:
                case GL_LEVEL_ERROR:
                    rgb = RGB(255, 0, 0);
                    break;
                case GL_LEVEL_WARNING:
                    rgb = RGB(0, 0, 255);
                    break;
                case GL_LEVEL_INFORMATION:
                default:
                    rgb = RGB(0, 0, 0);
                    break;
                }

                // 確保したメモリーを使ってガイドライン文字列を取得。
                dwSize = ImmGetGuideLine(hIMC, GGL_STRING, lpGLStr, dwSize);
                if (dwSize) {
                    SetTextColor(hDC, rgb); // テキスト色を指定。
                    SetBkMode(hDC, TRANSPARENT); // 背景透明。
                    TextOut(hDC, 0, nCyCap, lpGLStr, dwSize); // 文字列描画。
                }
                GlobalUnlock(hGLStr); // 文字列メモリーのロックを解除。
            }
            GlobalFree(hGLStr); // 解放。
        }
    }

    SelectObject(hMemDC, hbmpOld); // ビットマップ選択を解除。
    DeleteDC(hMemDC); // メモリーDCを破棄。
}

// ガイドラインウィンドウのマウスアクション。
void GuideWnd_Button(HWND hGuideWnd, UINT message, WPARAM wParam,
                     LPARAM lParam)
{
    POINT pt;
    HDC hDC;
    DWORD dwMouse;
    DWORD dwPushedGuide;
    DWORD dwTemp;
    HIMC hIMC;
    HWND hSvrWnd;
    static POINT ptdif;
    static RECT drc;
    static RECT rc;
    static DWORD dwCurrentPushedGuide;

    hDC = GetDC(hGuideWnd); // DCを取得。
    switch (message) {
    case WM_SETCURSOR: // カーソル形状の設定時。
        if (HIWORD(lParam) == WM_LBUTTONDOWN ||
            HIWORD(lParam) == WM_RBUTTONDOWN) { // 左ボタンか右ボタンが押された時。
            GetCursorPos(&pt); // カーソル位置を取得。
            SetCapture(hGuideWnd); // キャプチャーをセットしてドラッグを開始。
            GetWindowRect(hGuideWnd, &drc); // ウィンドウの位置とサイズを取得。
            // ウィンドウ位置とクリック位置の差を取得。
            ptdif.x = pt.x - drc.left;
            ptdif.y = pt.y - drc.top;
            rc = drc;
            rc.right -= rc.left;
            rc.bottom -= rc.top;
            ::SetWindowLongPtr(hGuideWnd, FIGWL_MOUSE, FIM_CAPUTURED); // キャプチャ状態を保存。
            dwPushedGuide = CheckPushedGuide(hGuideWnd, &pt);
            ::SetWindowLongPtr(hGuideWnd, FIGWL_PUSHSTATUS, dwPushedGuide); // 押された状態を保存。
            GuideWnd_Paint(hGuideWnd, hDC, &pt, dwPushedGuide); // 再描画。
            dwCurrentPushedGuide = dwPushedGuide; // 押された状態を覚える。
        }
        break;

    case WM_MOUSEMOVE:
        dwMouse = (DWORD)::GetWindowLongPtr(hGuideWnd, FIGWL_MOUSE); // 状態を取得。
        if (!(dwPushedGuide = (DWORD)::GetWindowLongPtr(hGuideWnd, FIGWL_PUSHSTATUS))) { // 押された状態がなければ
            if (dwMouse & FIM_MOVED) { // 移動した？
                DrawUIBorder(&drc);
                GetCursorPos(&pt);
                drc.left = pt.x - ptdif.x;
                drc.top = pt.y - ptdif.y;
                drc.right = drc.left + rc.right;
                drc.bottom = drc.top + rc.bottom;
                DrawUIBorder(&drc);
            } else if (dwMouse & FIM_CAPUTURED) {
                DrawUIBorder(&drc);
                ::SetWindowLongPtr(hGuideWnd, FIGWL_MOUSE, dwMouse | FIM_MOVED);
            }
        } else {
            GetCursorPos(&pt); // マウスカーソル位置を取得。
            dwTemp = CheckPushedGuide(hGuideWnd, &pt);
            if ((dwTemp ^ dwCurrentPushedGuide) & dwPushedGuide)
                GuideWnd_Paint(hGuideWnd, hDC, &pt, dwPushedGuide & dwTemp);
            dwCurrentPushedGuide = dwTemp;
        }
        break;

    case WM_LBUTTONUP: // 左ボタン解放時。
    case WM_RBUTTONUP: // 右ボタン解放時。
        dwMouse = (DWORD)::GetWindowLongPtr(hGuideWnd, FIGWL_MOUSE); // マウス状態を取得。
        if (dwMouse & FIM_CAPUTURED) { // キャプチャしている？
            ReleaseCapture(); // キャプチャを解放。
            if (dwMouse & FIM_MOVED) { // 移動した？
                DrawUIBorder(&drc); // ボーダーを再描画。
                GetCursorPos(&pt); // マウスカーソル位置を取得。
                // マウス位置に移動。
                ::MoveWindow(hGuideWnd, pt.x - ptdif.x, pt.y - ptdif.y, rc.right,
                             rc.bottom, TRUE);
            }
        }

        // UIサーバーを取得。
        hSvrWnd = (HWND)GetWindowLongPtr(hGuideWnd, FIGWLP_SERVERWND);

        hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC); // IMC。
        if (hIMC) {
            GetCursorPos(&pt); // マウス位置を取得。
            dwPushedGuide = (DWORD)::GetWindowLongPtr(hGuideWnd, FIGWL_PUSHSTATUS); // 押された状態を取得。
            dwPushedGuide &= CheckPushedGuide(hGuideWnd, &pt);
            if (!dwPushedGuide) {
            } else if (dwPushedGuide == PUSHED_STATUS_CLOSE) {
                PostMessage(hGuideWnd, WM_UI_HIDE, 0, 0);
            }
        }
        GuideWnd_Paint(hGuideWnd, hDC, NULL, 0); // 再描画。
        break;
    }
    ReleaseDC(hGuideWnd, hDC); // DCを解放。
}

// ガイドラインの更新。
void GuideWnd_Update(UIEXTRA *lpUIExtra)
{
    // ガイドラインウィンドウに更新メッセージを送る。
    if (::IsWindow(lpUIExtra->hwndGuide)) {
        ::SendMessage(lpUIExtra->hwndGuide, WM_UI_UPDATE, 0, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
