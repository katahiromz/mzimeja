// vkeycheck.c --- Windows virtual key checker
// Copyright (C) 2014 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
// All Rights Reserved.

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

HINSTANCE   g_hInstance;
HWND        g_hMainWnd;
UINT        g_vk = 0;

static const TCHAR s_szName[] = TEXT("katahiromz's virtual key checker");

LPTSTR GetVirutalKeyName(UINT vk)
{
    LANGID langid;
    static TCHAR s_szBuf[256];
    LPTSTR pch = s_szBuf;

    *pch = 0;
    #ifndef NO_MODIFIERS
        /* [Shift] key */
        if (GetAsyncKeyState(VK_LSHIFT) < 0)
        {
            lstrcat(pch, TEXT("VK_LSHIFT + "));
        }
        else if (GetAsyncKeyState(VK_RSHIFT) < 0)
        {
            lstrcat(pch, TEXT("VK_RSHIFT + "));
        }
        /* [Ctrl] key */
        if (GetAsyncKeyState(VK_LCONTROL) < 0)
        {
            lstrcat(pch, TEXT("VK_LCONTROL + "));
        }
        else if (GetAsyncKeyState(VK_RCONTROL) < 0)
        {
            lstrcat(pch, TEXT("VK_RCONTROL + "));
        }
        /* [Alt] key */
        if (GetAsyncKeyState(VK_LMENU) < 0)
        {
            lstrcat(pch, TEXT("VK_LMENU + "));
        }
        else if (GetAsyncKeyState(VK_RMENU) < 0)
        {
            lstrcat(pch, TEXT("VK_RMENU + "));
        }
    #endif  /* ndef NO_MODIFIERS */

    switch (vk)
    {
    case 0x00: lstrcat(pch, TEXT("")); break;
    case 0x01: lstrcat(pch, TEXT("VK_LBUTTON")); break;
    case 0x02: lstrcat(pch, TEXT("VK_RBUTTON")); break;
    case 0x03: lstrcat(pch, TEXT("VK_CANCEL")); break;
    case 0x04: lstrcat(pch, TEXT("VK_MBUTTON")); break;
    case 0x05: lstrcat(pch, TEXT("VK_XBUTTON1")); break;
    case 0x06: lstrcat(pch, TEXT("VK_XBUTTON2")); break;
    case 0x08: lstrcat(pch, TEXT("VK_BACK")); break;
    case 0x09: lstrcat(pch, TEXT("VK_TAB")); break;
    case 0x0C: lstrcat(pch, TEXT("VK_CLEAR")); break;
    case 0x0D: lstrcat(pch, TEXT("VK_RETURN")); break;
    case 0x10: lstrcat(pch, TEXT("VK_SHIFT")); break;
    case 0x11: lstrcat(pch, TEXT("VK_CONTROL")); break;
    case 0x12: lstrcat(pch, TEXT("VK_MENU")); break;
    case 0x13: lstrcat(pch, TEXT("VK_PAUSE")); break;
    case 0x14: lstrcat(pch, TEXT("VK_CAPITAL")); break;
    case 0x17: lstrcat(pch, TEXT("VK_JUNJA")); break;
    case 0x18: lstrcat(pch, TEXT("VK_FINAL")); break;
    case 0x1B: lstrcat(pch, TEXT("VK_ESCAPE")); break;
    case 0x1C: lstrcat(pch, TEXT("VK_CONVERT")); break;
    case 0x1D: lstrcat(pch, TEXT("VK_NONCONVERT")); break;
    case 0x1E: lstrcat(pch, TEXT("VK_ACCEPT")); break;
    case 0x1F: lstrcat(pch, TEXT("VK_MODECHANGE")); break;
    case 0x20: lstrcat(pch, TEXT("VK_SPACE")); break;
    case 0x21: lstrcat(pch, TEXT("VK_PRIOR")); break;
    case 0x22: lstrcat(pch, TEXT("VK_NEXT")); break;
    case 0x23: lstrcat(pch, TEXT("VK_END")); break;
    case 0x24: lstrcat(pch, TEXT("VK_HOME")); break;
    case 0x25: lstrcat(pch, TEXT("VK_LEFT")); break;
    case 0x26: lstrcat(pch, TEXT("VK_UP")); break;
    case 0x27: lstrcat(pch, TEXT("VK_RIGHT")); break;
    case 0x28: lstrcat(pch, TEXT("VK_DOWN")); break;
    case 0x29: lstrcat(pch, TEXT("VK_SELECT")); break;
    case 0x2A: lstrcat(pch, TEXT("VK_PRINT")); break;
    case 0x2B: lstrcat(pch, TEXT("VK_EXECUTE")); break;
    case 0x2C: lstrcat(pch, TEXT("VK_SNAPSHOT")); break;
    case 0x2D: lstrcat(pch, TEXT("VK_INSERT")); break;
    case 0x2E: lstrcat(pch, TEXT("VK_DELETE")); break;
    case 0x2F: lstrcat(pch, TEXT("VK_HELP")); break;
    case 0x5B: lstrcat(pch, TEXT("VK_LWIN")); break;
    case 0x5C: lstrcat(pch, TEXT("VK_RWIN")); break;
    case 0x5D: lstrcat(pch, TEXT("VK_APPS")); break;
    case 0x5F: lstrcat(pch, TEXT("VK_SLEEP")); break;
    case 0x60: lstrcat(pch, TEXT("VK_NUMPAD0")); break;
    case 0x61: lstrcat(pch, TEXT("VK_NUMPAD1")); break;
    case 0x62: lstrcat(pch, TEXT("VK_NUMPAD2")); break;
    case 0x63: lstrcat(pch, TEXT("VK_NUMPAD3")); break;
    case 0x64: lstrcat(pch, TEXT("VK_NUMPAD4")); break;
    case 0x65: lstrcat(pch, TEXT("VK_NUMPAD5")); break;
    case 0x66: lstrcat(pch, TEXT("VK_NUMPAD6")); break;
    case 0x67: lstrcat(pch, TEXT("VK_NUMPAD7")); break;
    case 0x68: lstrcat(pch, TEXT("VK_NUMPAD8")); break;
    case 0x69: lstrcat(pch, TEXT("VK_NUMPAD9")); break;
    case 0x6A: lstrcat(pch, TEXT("VK_MULTIPLY")); break;
    case 0x6B: lstrcat(pch, TEXT("VK_ADD")); break;
    case 0x6C: lstrcat(pch, TEXT("VK_SEPARATOR")); break;
    case 0x6D: lstrcat(pch, TEXT("VK_SUBTRACT")); break;
    case 0x6E: lstrcat(pch, TEXT("VK_DECIMAL")); break;
    case 0x6F: lstrcat(pch, TEXT("VK_DIVIDE")); break;
    case 0x70: lstrcat(pch, TEXT("VK_F1")); break;
    case 0x71: lstrcat(pch, TEXT("VK_F2")); break;
    case 0x72: lstrcat(pch, TEXT("VK_F3")); break;
    case 0x73: lstrcat(pch, TEXT("VK_F4")); break;
    case 0x74: lstrcat(pch, TEXT("VK_F5")); break;
    case 0x75: lstrcat(pch, TEXT("VK_F6")); break;
    case 0x76: lstrcat(pch, TEXT("VK_F7")); break;
    case 0x77: lstrcat(pch, TEXT("VK_F8")); break;
    case 0x78: lstrcat(pch, TEXT("VK_F9")); break;
    case 0x79: lstrcat(pch, TEXT("VK_F10")); break;
    case 0x7A: lstrcat(pch, TEXT("VK_F11")); break;
    case 0x7B: lstrcat(pch, TEXT("VK_F12")); break;
    case 0x7C: lstrcat(pch, TEXT("VK_F13")); break;
    case 0x7D: lstrcat(pch, TEXT("VK_F14")); break;
    case 0x7E: lstrcat(pch, TEXT("VK_F15")); break;
    case 0x7F: lstrcat(pch, TEXT("VK_F16")); break;
    case 0x80: lstrcat(pch, TEXT("VK_F17")); break;
    case 0x81: lstrcat(pch, TEXT("VK_F18")); break;
    case 0x82: lstrcat(pch, TEXT("VK_F19")); break;
    case 0x83: lstrcat(pch, TEXT("VK_F20")); break;
    case 0x84: lstrcat(pch, TEXT("VK_F21")); break;
    case 0x85: lstrcat(pch, TEXT("VK_F22")); break;
    case 0x86: lstrcat(pch, TEXT("VK_F23")); break;
    case 0x87: lstrcat(pch, TEXT("VK_F24")); break;
    case 0x90: lstrcat(pch, TEXT("VK_NUMLOCK")); break;
    case 0x91: lstrcat(pch, TEXT("VK_SCROLL")); break;
    case 0x92: lstrcat(pch, TEXT("VK_OEM_NEC_EQUAL or VK_OEM_FJ_JISHO")); break;
    case 0x93: lstrcat(pch, TEXT("VK_OEM_FJ_MASSHOU")); break;
    case 0x94: lstrcat(pch, TEXT("VK_OEM_FJ_TOUROKU")); break;
    case 0x95: lstrcat(pch, TEXT("VK_OEM_FJ_LOYA")); break;
    case 0x96: lstrcat(pch, TEXT("VK_OEM_FJ_ROYA")); break;
    case 0xA0: lstrcat(pch, TEXT("VK_LSHIFT")); break;
    case 0xA1: lstrcat(pch, TEXT("VK_RSHIFT")); break;
    case 0xA2: lstrcat(pch, TEXT("VK_LCONTROL")); break;
    case 0xA3: lstrcat(pch, TEXT("VK_RCONTROL")); break;
    case 0xA4: lstrcat(pch, TEXT("VK_LMENU")); break;
    case 0xA5: lstrcat(pch, TEXT("VK_RMENU")); break;
    case 0xA6: lstrcat(pch, TEXT("VK_BROWSER_BACK")); break;
    case 0xA7: lstrcat(pch, TEXT("VK_BROWSER_FORWARD")); break;
    case 0xA8: lstrcat(pch, TEXT("VK_BROWSER_REFRESH")); break;
    case 0xA9: lstrcat(pch, TEXT("VK_BROWSER_STOP")); break;
    case 0xAA: lstrcat(pch, TEXT("VK_BROWSER_SEARCH")); break;
    case 0xAB: lstrcat(pch, TEXT("VK_BROWSER_FAVORITES")); break;
    case 0xAC: lstrcat(pch, TEXT("VK_BROWSER_HOME")); break;
    case 0xAD: lstrcat(pch, TEXT("VK_VOLUME_MUTE")); break;
    case 0xAE: lstrcat(pch, TEXT("VK_VOLUME_DOWN")); break;
    case 0xAF: lstrcat(pch, TEXT("VK_VOLUME_UP")); break;
    case 0xB0: lstrcat(pch, TEXT("VK_MEDIA_NEXT_TRACK")); break;
    case 0xB1: lstrcat(pch, TEXT("VK_MEDIA_PREV_TRACK")); break;
    case 0xB2: lstrcat(pch, TEXT("VK_MEDIA_STOP")); break;
    case 0xB3: lstrcat(pch, TEXT("VK_MEDIA_PLAY_PAUSE")); break;
    case 0xB4: lstrcat(pch, TEXT("VK_LAUNCH_MAIL")); break;
    case 0xB5: lstrcat(pch, TEXT("VK_LAUNCH_MEDIA_SELECT")); break;
    case 0xB6: lstrcat(pch, TEXT("VK_LAUNCH_APP1")); break;
    case 0xB7: lstrcat(pch, TEXT("VK_LAUNCH_APP2")); break;
    case 0xBA: lstrcat(pch, TEXT("VK_OEM_1")); break;
    case 0xBB: lstrcat(pch, TEXT("VK_OEM_PLUS")); break;
    case 0xBC: lstrcat(pch, TEXT("VK_OEM_COMMA")); break;
    case 0xBD: lstrcat(pch, TEXT("VK_OEM_MINUS")); break;
    case 0xBE: lstrcat(pch, TEXT("VK_OEM_PERIOD")); break;
    case 0xBF: lstrcat(pch, TEXT("VK_OEM_2")); break;
    case 0xC0: lstrcat(pch, TEXT("VK_OEM_3")); break;
    case 0xDB: lstrcat(pch, TEXT("VK_OEM_4")); break;
    case 0xDC: lstrcat(pch, TEXT("VK_OEM_5")); break;
    case 0xDD: lstrcat(pch, TEXT("VK_OEM_6")); break;
    case 0xDE: lstrcat(pch, TEXT("VK_OEM_7")); break;
    case 0xDF: lstrcat(pch, TEXT("VK_OEM_8")); break;
    case 0xE1: lstrcat(pch, TEXT("VK_OEM_AX")); break;
    case 0xE2: lstrcat(pch, TEXT("VK_OEM_102")); break;
    case 0xE3: lstrcat(pch, TEXT("VK_ICO_HELP")); break;
    case 0xE4: lstrcat(pch, TEXT("VK_ICO_00")); break;
    case 0xE5: lstrcat(pch, TEXT("VK_PROCESSKEY")); break;
    case 0xE6: lstrcat(pch, TEXT("VK_ICO_CLEAR")); break;
    case 0xE7: lstrcat(pch, TEXT("VK_PACKET")); break;
    case 0xE9: lstrcat(pch, TEXT("VK_OEM_RESET")); break;
    case 0xEA: lstrcat(pch, TEXT("VK_OEM_JUMP")); break;
    case 0xEB: lstrcat(pch, TEXT("VK_OEM_PA1")); break;
    case 0xEC: lstrcat(pch, TEXT("VK_OEM_PA2")); break;
    case 0xED: lstrcat(pch, TEXT("VK_OEM_PA3")); break;
    case 0xEE: lstrcat(pch, TEXT("VK_OEM_WSCTRL")); break;
    case 0xEF: lstrcat(pch, TEXT("VK_OEM_CUSEL")); break;
    case 0xF0: lstrcat(pch, TEXT("VK_OEM_ATTN")); break;
    case 0xF1: lstrcat(pch, TEXT("VK_OEM_FINISH")); break;
    case 0xF2: lstrcat(pch, TEXT("VK_OEM_COPY")); break;
    case 0xF3: lstrcat(pch, TEXT("VK_OEM_AUTO")); break;
    case 0xF4: lstrcat(pch, TEXT("VK_OEM_ENLW")); break;
    case 0xF5: lstrcat(pch, TEXT("VK_OEM_BACKTAB")); break;
    case 0xF6: lstrcat(pch, TEXT("VK_ATTN")); break;
    case 0xF7: lstrcat(pch, TEXT("VK_CRSEL")); break;
    case 0xF8: lstrcat(pch, TEXT("VK_EXSEL")); break;
    case 0xF9: lstrcat(pch, TEXT("VK_EREOF")); break;
    case 0xFA: lstrcat(pch, TEXT("VK_PLAY")); break;
    case 0xFB: lstrcat(pch, TEXT("VK_ZOOM")); break;
    case 0xFC: lstrcat(pch, TEXT("VK_NONAME")); break;
    case 0xFD: lstrcat(pch, TEXT("VK_PA1")); break;
    case 0xFE: lstrcat(pch, TEXT("VK_OEM_CLEAR")); break;
    default:
        /* alphabet or digit */
        if (TEXT('0') <= vk && vk <= TEXT('9') ||
            TEXT('A') <= vk && vk <= TEXT('Z'))
        {
            pch += lstrlen(pch);
            wsprintf(pch, TEXT("'%c'"), (UINT)vk);
            break;
        }

        // language-dependent codes
        if (vk == 0x15)
        {
            langid = GetUserDefaultLangID();
            switch (langid)
            {
            case LANG_JAPANESE:
                lstrcat(pch, TEXT("VK_KANA"));
                break;
            case LANG_KOREAN:
                lstrcat(pch, TEXT("VK_HANGUL"));
                break;
            default:
                lstrcat(pch, TEXT("VK_KANA or VK_HANGUL"));
                break;
            }
        }
        else if (vk == 0x19)
        {
            langid = GetUserDefaultLangID();
            switch (langid)
            {
            case LANG_JAPANESE:
                lstrcat(pch, TEXT("VK_KANJI"));
                break;
            case LANG_KOREAN:
                lstrcat(pch, TEXT("VK_HANJA"));
                break;
            default:
                lstrcat(pch, TEXT("VK_KANJI or VK_HANJA"));
                break;
            }
        }
        else
        {
            // unknown code
            wsprintf(pch, TEXT("0x%02X (%u)"), (UINT)(BYTE)vk, vk);
        }
    }

    return s_szBuf;
}

void MainWnd_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    LPTSTR pszName;
    RECT rcClient;

    pszName = GetVirutalKeyName(g_vk);
    GetClientRect(hwnd, &rcClient);

    hdc = BeginPaint(hwnd, &ps);
    if (hdc)
    {
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        SetBkMode(hdc, TRANSPARENT);
        SetBkColor(hdc, RGB(255, 255, 255));
        DrawText(hdc, pszName, -1, &rcClient,
                 DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        EndPaint(hwnd, &ps);
    }
}

void MainWnd_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    if (!fDown)
        return;

    g_vk = vk;
    InvalidateRect(hwnd, NULL, TRUE);
}

BOOL MainWnd_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    g_hMainWnd = hwnd;
    return TRUE;
}

void MainWnd_OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(hwnd, WM_CREATE, MainWnd_OnCreate);
    HANDLE_MSG(hwnd, WM_DESTROY, MainWnd_OnDestroy);
    HANDLE_MSG(hwnd, WM_PAINT, MainWnd_OnPaint);
    HANDLE_MSG(hwnd, WM_KEYDOWN, MainWnd_OnKey);
    HANDLE_MSG(hwnd, WM_KEYUP, MainWnd_OnKey);
#ifndef NO_SYSKEY
    HANDLE_MSG(hwnd, WM_SYSKEYDOWN, MainWnd_OnKey);
    HANDLE_MSG(hwnd, WM_SYSKEYUP, MainWnd_OnKey);
#endif
    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI _tWinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPTSTR      lpCmdLine,
    int         nCmdShow)
{
    WNDCLASS wc;
    MSG msg;

    g_hInstance = hInstance;

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = s_szName;
    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, TEXT("RegisterClass failed!"), s_szName,
                   MB_ICONERROR);
        return 1;
    }

    CreateWindow(s_szName, s_szName, WS_OVERLAPPEDWINDOW,
                 CW_USEDEFAULT, 0, 450, 100,
        NULL, NULL, hInstance, NULL);
    if (g_hMainWnd == NULL)
    {
        MessageBox(NULL, TEXT("CreateWindow failed!"), s_szName,
                   MB_ICONERROR);
        return 2;
    }

    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
