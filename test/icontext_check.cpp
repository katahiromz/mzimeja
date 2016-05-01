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
std::vector<DWORD> g_vecReadClause;
std::vector<DWORD> g_vecCompClause;
std::vector<DWORD> g_vecResultReadClause;
std::vector<DWORD> g_vecResultClause;

static const TCHAR s_szName[] = TEXT("katahiromz's input context checker");

void MainWnd_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rcClient;

    GetClientRect(hwnd, &rcClient);
    WCHAR sz[256];

    std::wstring strReadAttr, strCompAttr;
    for (size_t i = 0; i < g_vecReadAttr.size(); ++i) {
        wsprintfW(sz, TEXT("%02X "), g_vecReadAttr[i]);
        strReadAttr += sz;
    }
    for (size_t i = 0; i < g_vecCompAttr.size(); ++i) {
        wsprintfW(sz, TEXT("%02X "), g_vecCompAttr[i]);
        strCompAttr += sz;
    }

    std::wstring strReadClause, strCompClause;
    for (size_t i = 0; i < g_vecReadClause.size(); ++i) {
        wsprintfW(sz, TEXT("%08X "), g_vecReadClause[i]);
        strReadClause += sz;
    }
    for (size_t i = 0; i < g_vecCompClause.size(); ++i) {
        wsprintfW(sz, TEXT("%08X "), g_vecCompClause[i]);
        strCompClause += sz;
    }

    std::wstring strResultReadClause, strResultClause;
    for (size_t i = 0; i < g_vecResultReadClause.size(); ++i) {
        wsprintfW(sz, TEXT("%08X "), g_vecResultReadClause[i]);
        strResultReadClause += sz;
    }
    for (size_t i = 0; i < g_vecResultClause.size(); ++i) {
        wsprintfW(sz, TEXT("%08X "), g_vecResultClause[i]);
        strResultClause += sz;
    }

    WCHAR szBuf[1024];
    wsprintfW(szBuf,
        L"dwCursorPos: %u\r\n"
        L"dwDeltaStart: %u\r\n"
        L"reading: %s\r\n"
        L"reading clause: %s\r\n"
        L"reading attributes: %s\r\n"
        L"composition: %s\r\n"
        L"composition clause: %s\r\n"
        L"composition attributes: %s\r\n"
        L"result reading: %s\r\n"
        L"result reading clause: %s\r\n"
        L"result: %s\r\n"
        L"result clause: %s",
        g_dwCursorPos, g_dwDeltaStart,
        g_strRead.c_str(), strReadClause.c_str(), strReadAttr.c_str(),
        g_strComp.c_str(), strCompClause.c_str(), strCompAttr.c_str(),
        g_strResultRead.c_str(), strResultReadClause.c_str(),
        g_strResult.c_str(), strResultClause.c_str());

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
            g_vecReadClause.clear();
            if (lpCompStr->dwCompReadClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwCompReadClauseOffset);
                DWORD count = lpCompStr->dwCompReadClauseLen / sizeof(DWORD);
                separate(g_strRead, pdw, count);
                g_vecReadClause.assign(pdw, pdw + count);
            }
            LPBYTE pbReadAttr = LPBYTE(lpCompStr) + lpCompStr->dwCompReadAttrOffset;
            g_vecReadAttr.assign(pbReadAttr, pbReadAttr + lpCompStr->dwCompReadAttrLen);

            // get comp info
            LPTSTR pchComp = (LPTSTR)(LPBYTE(lpCompStr) + lpCompStr->dwCompStrOffset);
            g_strComp.assign(pchComp, lpCompStr->dwCompStrLen);
            g_vecCompClause.clear();
            if (lpCompStr->dwCompClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwCompClauseOffset);
                DWORD count = lpCompStr->dwCompClauseLen / sizeof(DWORD);
                separate(g_strComp, pdw, count);
                g_vecCompClause.assign(pdw, pdw + count);
            }
            LPBYTE pbCompAttr = LPBYTE(lpCompStr) + lpCompStr->dwCompAttrOffset;
            g_vecCompAttr.assign(pbCompAttr, pbCompAttr + lpCompStr->dwCompAttrLen);

            // get result reading
            LPTSTR pchResultRead = (LPTSTR)(LPBYTE(lpCompStr) + lpCompStr->dwResultReadStrOffset);
            g_strResultRead.assign(pchResultRead, lpCompStr->dwResultReadStrLen);
            g_vecResultReadClause.clear();
            if (lpCompStr->dwResultReadClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwResultReadClauseOffset);
                DWORD count = lpCompStr->dwResultReadClauseLen / sizeof(DWORD);
                separate(g_strResultRead, pdw, count);
                g_vecResultReadClause.assign(pdw, pdw + count);
            }

            // get result
            LPTSTR pchResult = (LPTSTR)(LPBYTE(lpCompStr) + lpCompStr->dwResultStrOffset);
            g_strResult.assign(pchResult, lpCompStr->dwResultStrLen);
            g_vecResultClause.clear();
            if (lpCompStr->dwResultClauseLen) {
                LPDWORD pdw = (LPDWORD)(LPBYTE(lpCompStr) + lpCompStr->dwResultClauseOffset);
                DWORD count = lpCompStr->dwResultClauseLen / sizeof(DWORD);
                separate(g_strResult, pdw, count);
                g_vecResultClause.assign(pdw, pdw + count);
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
