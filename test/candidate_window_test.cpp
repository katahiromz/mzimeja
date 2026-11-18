#include "../ime/mzimeja.h"
#include "../ime/candidate_position.h"
#include <cstdio>

static bool Check(bool condition, const char *message)
{
    if (!condition)
        std::fprintf(stderr, "%s\n", message);
    return condition;
}

static LRESULT CALLBACK TestWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_IME_REQUEST && wp == IMR_QUERYCHARPOSITION) {
        IMECHARPOSITION *pos = (IMECHARPOSITION *)lp;
        POINT pt = { 40, 50 };
        ClientToScreen(hwnd, &pt);
        pos->pt = pt;
        pos->cLineHeight = 20;
        return TRUE;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int wmain()
{
    const RECT work = { 0, 0, 800, 600 };
    const RECT exclude = { 100, 100, 200, 120 };
    const SIZE size = { 150, 100 };
    POINT pt = { 100, 100 };
    POINT actual = CandidatePosExclude(pt, exclude, work, size, FALSE);
    bool ok = Check(actual.x == 100 && actual.y == 120, "horizontal placement");
    RECT bottom = { 100, 550, 200, 570 };
    actual = CandidatePosExclude(pt, bottom, work, size, FALSE);
    ok &= Check(actual.y == 450, "placement above exclusion");
    actual = CandidatePosExclude(pt, exclude, work, size, TRUE);
    ok &= Check(actual.x == 200, "vertical placement to right");
    RECT right = { 600, 100, 700, 120 };
    pt.x = 600;
    actual = CandidatePosExclude(pt, right, work, size, TRUE);
    ok &= Check(actual.x == 450, "vertical placement to left");
    pt.x = 790;
    actual = CandidatePosExclude(pt, exclude, work, size, FALSE);
    ok &= Check(actual.x == 650, "placement clamped to work area");

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = TestWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("MzimejaCandidatePositionTest");
    wc.cbWndExtra = UIEXTRASIZE;
    if (!Check(RegisterClass(&wc) != 0, "RegisterClass failed"))
        return 1;
    HWND client = CreateWindow(wc.lpszClassName, TEXT(""), WS_OVERLAPPEDWINDOW,
                               50, 50, 400, 300, NULL, NULL, wc.hInstance, NULL);
    HWND candidate = CreateWindow(wc.lpszClassName, TEXT(""), WS_POPUP,
                                  0, 0, 150, 100, NULL, NULL, wc.hInstance, NULL);
    if (!Check(client && candidate, "CreateWindow failed"))
        return 1;
    ShowWindow(client, SW_SHOWNOACTIVATE);

    HIMC context = ImmCreateContext();
    InputContext *imc = (InputContext *)ImmLockIMC(context);
    if (!Check(imc != NULL, "ImmCreateContext or ImmLockIMC failed"))
        return 1;
    UIEXTRA ui = { 0 };
    imc->hWnd = client;
    imc->hCompStr = CompStr::ReCreate(imc->hCompStr);
    LogCandInfo info;
    LogCandList list;
    list.cand_strs.push_back(L"candidate");
    info.cand_lists.push_back(list);
    imc->hCandInfo = CandInfo::ReCreate(imc->hCandInfo, &info);
    if (!Check(imc->hCompStr && imc->hCandInfo, "IME context allocation failed"))
        return 1;
    ui.hwndCand = candidate;
    imc->cfCandForm[0].dwIndex = 0;
    imc->cfCandForm[0].dwStyle = CFS_EXCLUDE;
    imc->cfCandForm[0].ptCurrentPos.x = 40;
    imc->cfCandForm[0].ptCurrentPos.y = 50;
    SetRect(&imc->cfCandForm[0].rcArea, 40, 50, 80, 70);
    CandWnd_Move(&ui, imc);
    RECT rect;
    GetWindowRect(candidate, &rect);
    POINT expected = { 40, 70 };
    ClientToScreen(client, &expected);
    ok &= Check(IsWindowVisible(candidate) && rect.left == expected.x &&
                rect.top == expected.y, "CFS_EXCLUDE candidate placement");

    ShowWindow(candidate, SW_HIDE);
    imc->cfCandForm[0].dwIndex = (DWORD)-1;
    CandWnd_Move(&ui, imc);
    GetWindowRect(candidate, &rect);
    ok &= Check(IsWindowVisible(candidate) && rect.left == expected.x &&
                rect.top == expected.y, "hidden composition position fallback");

    ShowWindow(candidate, SW_HIDE);
    imc->cfCandForm[0].dwIndex = 0;
    imc->cfCandForm[0].dwStyle = CFS_CANDIDATEPOS;
    CandWnd_Move(&ui, imc);
    GetWindowRect(candidate, &rect);
    expected.y -= 20;
    ok &= Check(IsWindowVisible(candidate) && rect.left == expected.x &&
                rect.top == expected.y, "CFS_CANDIDATEPOS placement");

    ImmUnlockIMC(context);
    ImmDestroyContext(context);
    DestroyWindow(candidate);
    DestroyWindow(client);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return ok ? 0 : 1;
}
