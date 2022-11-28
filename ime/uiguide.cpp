// uiguide.cpp --- mzimeja guideline window UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// ガイドラインウィンドウのウィンドウプロシージャ。
LRESULT CALLBACK GuideWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                     LPARAM lParam) {
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
            SetWindowLong(hWnd, FIGWL_MOUSE, 0);
            SetWindowLong(hWnd, FIGWL_PUSHSTATUS, 0);
        }
        break;

    case WM_MOVE: // ウィンドウ移動時。
        hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (IsWindow(hUIWnd))
            SendMessage(hUIWnd, WM_UI_GUIDEMOVE, wParam, lParam); // UIサーバーに送る。
        break;

    case WM_CREATE: // ウィンドウ作成時。
        hbmpGuide = TheIME.LoadBMP(TEXT("CLOSEBMP")); // ビットマップを読み込み、保存する。
        SetWindowLongPtr(hWnd, FIGWLP_CLOSEBMP, (LONG_PTR)hbmpGuide);
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

DWORD PASCAL CheckPushedGuide(HWND hGuideWnd, LPPOINT lppt) {
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

void GuideWnd_Paint(HWND hGuideWnd, HDC hDC, LPPOINT lppt,
                    DWORD dwPushedGuide) {
    HIMC hIMC;
    HDC hMemDC;
    HBITMAP hbmpOld;
    HWND hSvrWnd;
    HANDLE hGLStr;
    WCHAR *lpGLStr;
    DWORD dwLevel;
    DWORD dwSize;

    hSvrWnd = (HWND)GetWindowLongPtr(hGuideWnd, FIGWLP_SERVERWND);

    hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC);
    if (hIMC) {
        HBITMAP hbmpGuide;
        HBRUSH hOldBrush, hBrush;
        int nCyCap = GetSystemMetrics(SM_CYSMCAPTION);
        RECT rc;

        hMemDC = CreateCompatibleDC(hDC);

        // Paint Caption.
        hBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
        hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
        GetClientRect(hGuideWnd, &rc);
        // rc.top = rc.left = 0;
        // rc.right = BTX*3;
        rc.bottom = nCyCap;
        FillRect(hDC, &rc, hBrush);
        SelectObject(hDC, hOldBrush);
        DeleteObject(hBrush);

        // Paint CloseButton.
        hbmpGuide = (HBITMAP)GetWindowLongPtr(hGuideWnd, FIGWLP_CLOSEBMP);
        hbmpOld = (HBITMAP)SelectObject(hMemDC, hbmpGuide);

        if (!(dwPushedGuide & PUSHED_STATUS_CLOSE))
            BitBlt(hDC, rc.right - STCLBT_DX - 2, STCLBT_Y, STCLBT_DX, STCLBT_DY,
                   hMemDC, 0, 0, SRCCOPY);
        else
            BitBlt(hDC, rc.right - STCLBT_DX - 2, STCLBT_Y, STCLBT_DX, STCLBT_DY,
                   hMemDC, STCLBT_DX, 0, SRCCOPY);

        dwLevel = ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0);
        if (dwLevel) {
            dwSize = ImmGetGuideLine(hIMC, GGL_STRING, NULL, 0) + 1;
            if ((dwSize > 1) && (hGLStr = GlobalAlloc(GHND, dwSize))) {
                lpGLStr = (LPTSTR)GlobalLock(hGLStr);
                if (lpGLStr) {
                    COLORREF rgb = 0;
                    HBRUSH hbrLGR = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
                    HBRUSH hbr;

                    hbr = (HBRUSH)SelectObject(hDC, hbrLGR);
                    GetClientRect(hGuideWnd, &rc);
                    PatBlt(hDC, 0, nCyCap, rc.right, rc.bottom - nCyCap, PATCOPY);
                    SelectObject(hDC, hbr);

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

                    dwSize = ImmGetGuideLine(hIMC, GGL_STRING, lpGLStr, dwSize);
                    if (dwSize) {
                        SetTextColor(hDC, rgb);
                        SetBkMode(hDC, TRANSPARENT);
                        TextOut(hDC, 0, nCyCap, lpGLStr, dwSize);
                    }
                    GlobalUnlock(hGLStr);
                }
                GlobalFree(hGLStr);
            }
        }

        SelectObject(hMemDC, hbmpOld);
        DeleteDC(hMemDC);
    }
}

// ガイドラインウィンドウのマウスアクション。
void GuideWnd_Button(HWND hGuideWnd, UINT message, WPARAM wParam,
                     LPARAM lParam) {
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

    hDC = GetDC(hGuideWnd);
    switch (message) {
    case WM_SETCURSOR:
        if (HIWORD(lParam) == WM_LBUTTONDOWN ||
            HIWORD(lParam) == WM_RBUTTONDOWN) {
            GetCursorPos(&pt);
            SetCapture(hGuideWnd);
            GetWindowRect(hGuideWnd, &drc);
            ptdif.x = pt.x - drc.left;
            ptdif.y = pt.y - drc.top;
            rc = drc;
            rc.right -= rc.left;
            rc.bottom -= rc.top;
            SetWindowLong(hGuideWnd, FIGWL_MOUSE, FIM_CAPUTURED);
            SetWindowLong(hGuideWnd, FIGWL_PUSHSTATUS,
                          dwPushedGuide = CheckPushedGuide(hGuideWnd, &pt));
            GuideWnd_Paint(hGuideWnd, hDC, &pt, dwPushedGuide);
            dwCurrentPushedGuide = dwPushedGuide;
        }
        break;

    case WM_MOUSEMOVE:
        dwMouse = GetWindowLong(hGuideWnd, FIGWL_MOUSE);
        if (!(dwPushedGuide = GetWindowLong(hGuideWnd, FIGWL_PUSHSTATUS))) {
            if (dwMouse & FIM_MOVED) {
                DrawUIBorder(&drc);
                GetCursorPos(&pt);
                drc.left = pt.x - ptdif.x;
                drc.top = pt.y - ptdif.y;
                drc.right = drc.left + rc.right;
                drc.bottom = drc.top + rc.bottom;
                DrawUIBorder(&drc);
            } else if (dwMouse & FIM_CAPUTURED) {
                DrawUIBorder(&drc);
                SetWindowLong(hGuideWnd, FIGWL_MOUSE, dwMouse | FIM_MOVED);
            }
        } else {
            GetCursorPos(&pt);
            dwTemp = CheckPushedGuide(hGuideWnd, &pt);
            if ((dwTemp ^ dwCurrentPushedGuide) & dwPushedGuide)
                GuideWnd_Paint(hGuideWnd, hDC, &pt, dwPushedGuide & dwTemp);
            dwCurrentPushedGuide = dwTemp;
        }
        break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        dwMouse = GetWindowLong(hGuideWnd, FIGWL_MOUSE);
        if (dwMouse & FIM_CAPUTURED) {
            ReleaseCapture();
            if (dwMouse & FIM_MOVED) {
                DrawUIBorder(&drc);
                GetCursorPos(&pt);
                MoveWindow(hGuideWnd, pt.x - ptdif.x, pt.y - ptdif.y, rc.right,
                           rc.bottom, TRUE);
            }
        }
        hSvrWnd = (HWND)GetWindowLongPtr(hGuideWnd, FIGWLP_SERVERWND);

        hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC); // IMC。
        if (hIMC) {
            GetCursorPos(&pt);
            dwPushedGuide = GetWindowLong(hGuideWnd, FIGWL_PUSHSTATUS);
            dwPushedGuide &= CheckPushedGuide(hGuideWnd, &pt);
            if (!dwPushedGuide) {
            } else if (dwPushedGuide == PUSHED_STATUS_CLOSE) {
                PostMessage(hGuideWnd, WM_UI_HIDE, 0, 0);
            }
        }
        GuideWnd_Paint(hGuideWnd, hDC, NULL, 0);
        break;
    }
    ReleaseDC(hGuideWnd, hDC);
}

// ガイドラインの更新。
void GuideWnd_Update(UIEXTRA *lpUIExtra) {
    // ガイドラインウィンドウに更新メッセージを送る。
    if (::IsWindow(lpUIExtra->hwndGuide)) {
        ::SendMessage(lpUIExtra->hwndGuide, WM_UI_UPDATE, 0, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
