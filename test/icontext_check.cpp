// icontext_check.c --- Windows Input Context checker
// Copyright (C) 2016 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
// All Rights Reserved.

#if !defined(UNICODE) || !defined(_UNICODE)
    #error You lose.
#endif
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "../include/immdev.h"
#include <string>
#include <vector>

typedef std::wstring tstring;

HINSTANCE   g_hInstance;
HWND        g_hMainWnd;
DWORD       g_dwCursorPos = 0;
DWORD       g_dwDeltaStart = 0;
tstring     g_strRead;
tstring     g_strComp;
tstring     g_strResultRead;
tstring     g_strResult;
std::vector<BYTE> g_vecReadAttr;
std::vector<BYTE> g_vecCompAttr;

static const TCHAR s_szName[] = TEXT("katahiromz's input context checker");

void MainWnd_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rcClient;

    GetClientRect(hwnd, &rcClient);
    std::wstring strReadAttr, strCompAttr;
    WCHAR sz[256];
    for (size_t i = 0; i < g_vecReadAttr.size(); ++i) {
        wsprintfW(sz, TEXT("%02X "), g_vecReadAttr[i]);
        strReadAttr += sz;
    }
    for (size_t i = 0; i < g_vecCompAttr.size(); ++i) {
        wsprintfW(sz, TEXT("%02X "), g_vecCompAttr[i]);
        strCompAttr += sz;
    }

    WCHAR szBuf[1024];
    wsprintfW(szBuf,
        L"dwCursorPos: %u\r\n"
        L"dwDeltaStart: %u\r\n"
        L"reading: %s\r\n"
        L"reading attributes: %s\r\n"
        L"composition: %s\r\n"
        L"composition attributes: %s\r\n"
        L"result reading: %s\r\n"
        L"result: %s",
        g_dwCursorPos, g_dwDeltaStart,
        g_strRead.c_str(), strReadAttr.c_str(),
        g_strComp.c_str(), strCompAttr.c_str(),
        g_strResultRead.c_str(), g_strResult.c_str());

    hdc = BeginPaint(hwnd, &ps);
    if (hdc)
    {
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        SetBkMode(hdc, TRANSPARENT);
        SetBkColor(hdc, RGB(255, 255, 255));
        DrawText(hdc, szBuf, -1, &rcClient,
                 DT_LEFT | DT_VCENTER | DT_NOPREFIX);
        EndPaint(hwnd, &ps);
    }
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

void separate(std::wstring& str, LPDWORD pdw, DWORD count) {
    for (LONG i = count - 1; i >= 0; --i) {
        str.insert(pdw[i], 1, L'|');
    }
}

void MainWnd_OnImeComposition(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    HIMC hIMC = ImmGetContext(hwnd);
    LPINPUTCONTEXT pContext = ImmLockIMC(hIMC);
    if (pContext)
    {
        HIMCC hCompStr = pContext->hCompStr;
        LPCOMPOSITIONSTRING lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(hCompStr);
        if (lpCompStr != NULL)
        {
            g_dwCursorPos = lpCompStr->dwCursorPos;
            g_dwDeltaStart = lpCompStr->dwDeltaStart;

            // get read info
            LPTSTR pchRead = (LPTSTR)(LPBYTE(lpCompStr) + lpCompStr->dwCompReadStrOffset);
            g_strRead.assign(pchRead, lpCompStr->dwCompReadStrLen);
            if (lpCompStr->dwCompReadClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwCompReadClauseOffset);
                separate(g_strRead, pdw, lpCompStr->dwCompReadClauseLen / sizeof(DWORD));
            }
            LPBYTE pbReadAttr = LPBYTE(lpCompStr) + lpCompStr->dwCompReadAttrOffset;
            g_vecReadAttr.assign(pbReadAttr, pbReadAttr + lpCompStr->dwCompReadAttrLen);

            // get comp info
            LPTSTR pchComp = (LPTSTR)(LPBYTE(lpCompStr) + lpCompStr->dwCompStrOffset);
            g_strComp.assign(pchComp, lpCompStr->dwCompStrLen);
            if (lpCompStr->dwCompClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwCompClauseOffset);
                separate(g_strComp, pdw, lpCompStr->dwCompClauseLen / sizeof(DWORD));
            }
            LPBYTE pbCompAttr = LPBYTE(lpCompStr) + lpCompStr->dwCompAttrOffset;
            g_vecCompAttr.assign(pbCompAttr, pbCompAttr + lpCompStr->dwCompAttrLen);

            // get result reading
            LPTSTR pchResultRead = (LPTSTR)(LPBYTE(lpCompStr) + lpCompStr->dwResultReadStrOffset);
            g_strResultRead.assign(pchResultRead, lpCompStr->dwResultReadStrLen);
            if (lpCompStr->dwResultReadClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwResultReadClauseOffset);
                separate(g_strResultRead, pdw, lpCompStr->dwResultReadClauseLen / sizeof(DWORD));
            }

            // get result
            LPTSTR pchResult = (LPTSTR)(LPBYTE(lpCompStr) + lpCompStr->dwResultStrOffset);
            g_strResult.assign(pchResult, lpCompStr->dwResultStrLen);
            if (lpCompStr->dwResultClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwResultClauseOffset);
                separate(g_strResult, pdw, lpCompStr->dwResultClauseLen / sizeof(DWORD));
            }

            ImmUnlockIMCC(hCompStr);
        }
        ImmUnlockIMC(hIMC);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(hwnd, WM_CREATE, MainWnd_OnCreate);
    HANDLE_MSG(hwnd, WM_DESTROY, MainWnd_OnDestroy);
    HANDLE_MSG(hwnd, WM_PAINT, MainWnd_OnPaint);
    case WM_IME_COMPOSITION:
        MainWnd_OnImeComposition(hwnd, wParam, lParam);
        // FALL THROUGH
    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

extern "C"
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
                 CW_USEDEFAULT, 0, 450, 350,
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
