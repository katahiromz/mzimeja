// main.cpp --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include <shlobj.h>
#include <strsafe.h>
#include <algorithm>
#include <clocale>
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

// The window classes for mzimeja UI windows.
// mzimejaで使われるウィンドウクラスの名前。
const WCHAR szUIServerClassName[] = L"MZIMEUI";
const WCHAR szCompStrClassName[]  = L"MZIMECompStr";
const WCHAR szCandClassName[]     = L"MZIMECand";
const WCHAR szStatusClassName[]   = L"MZIMEStatus";
const WCHAR szGuideClassName[]    = L"MZIMEGuide";

// The table of guideline.
// ガイドラインのテーブル。
const MZGUIDELINE glTable[] = {
    {GL_LEVEL_ERROR, GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
    {GL_LEVEL_WARNING, GL_ID_TYPINGERROR, IDS_GL_TYPINGERROR, 0},
    {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST, IDS_GL_TESTGUIDELINESTR,
     IDS_GL_TESTGUIDELINEPRIVATE}
};

// The filename of the IME. IMEのファイル名。
const WCHAR szImeFileName[] = L"mzimeja.ime";

//////////////////////////////////////////////////////////////////////////////

// Adjust window position.
// ウィンドウ位置を画面内に補正。
void RepositionWindow(HWND hWnd)
{
    RECT rc, rcWorkArea;
    ::GetWindowRect(hWnd, &rc);
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
    SIZE siz;
    siz.cx = rc.right - rc.left;
    siz.cy = rc.bottom - rc.top;
    if (rc.right > rcWorkArea.right) {
        rc.right = rcWorkArea.right;
        rc.left = rcWorkArea.right - siz.cx;
    }
    if (rc.left < rcWorkArea.left) {
        rc.left = rcWorkArea.left;
        rc.right = rc.left + siz.cx;
    }
    if (rc.bottom > rcWorkArea.bottom) {
        rc.bottom = rcWorkArea.bottom;
        rc.top = rcWorkArea.bottom - siz.cy;
    }
    if (rc.top < rcWorkArea.top) {
        rc.top = rcWorkArea.top;
        rc.bottom = rc.top + siz.cy;
    }
    DPRINTA("%d, %d, %d, %d\n", rc.left, rc.top, siz.cx, siz.cy);
    ::MoveWindow(hWnd, rc.left, rc.top, siz.cx, siz.cy, TRUE);
}

//////////////////////////////////////////////////////////////////////////////

MzIme TheIME;

// mzimejaのコンストラクタ。
MzIme::MzIme()
{
    m_hInst = NULL;
    m_hMyKL = NULL;
    m_bWinLogOn = FALSE;

    m_lpCurTransKey = NULL;
    m_uNumTransKey = 0;

    m_fOverflowKey = FALSE;

    m_hIMC = NULL;
    m_lpIMC = NULL;
}

// mzimejaの辞書を読み込む。
BOOL MzIme::LoadDict()
{
    BOOL ret = TRUE;

    std::wstring basic;
    if (!Config_GetDWORD(L"BasicDictDisabled", FALSE)) {
        if (Config_GetSz(L"BasicDictPathName", basic)) {
            if (!g_basic_dict.Load(basic.c_str(), L"BasicDictObject")) {
                ret = FALSE;
            }
        }
    } else {
        g_basic_dict.Unload();
    }

    std::wstring name;
    if (!Config_GetDWORD(L"NameDictDisabled", FALSE)) {
        if (Config_GetSz(L"NameDictPathName", name)) {
            if (!g_name_dict.Load(name.c_str(), L"NameDictObject")) {
                ret = FALSE;
            }
        }
    } else {
        g_name_dict.Unload();
    }

    return ret;
}

// mzimejaの辞書をアンロードする。
void MzIme::UnloadDict()
{
    g_basic_dict.Unload();
    g_name_dict.Unload();
}

// mzimejaを初期化。
BOOL MzIme::Init(HINSTANCE hInstance)
{
    m_hInst = hInstance;
    //::InitCommonControls();

    MakeLiteralMaps();

    // load dict
    LoadDict();

    // register window classes for IME
    return RegisterClasses(m_hInst);
} // MzIme::Init

// mzimejaを逆初期化。
VOID MzIme::Uninit(VOID)
{
    UnregisterClasses();
    UnloadDict();
}

//////////////////////////////////////////////////////////////////////////////

// mzimejaのウィンドウクラスを登録する。
BOOL MzIme::RegisterClasses(HINSTANCE hInstance)
{
#define CS_MZIME (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_IME)
    WNDCLASSEX wcx;

    // Register the UI server window.
    // UIサーバーウィンドウを登録。
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = MZIMEWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 2 * sizeof(LONG_PTR);
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szUIServerClassName;
    wcx.hbrBackground = NULL;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the composition window.
    // 未確定文字列ウィンドウを登録。
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = CompWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szCompStrClassName;
    wcx.hbrBackground = NULL;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the candidate window.
    // 候補ウィンドウを登録。
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = CandWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szCandClassName;
    wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the status window.
    // 状態ウィンドウを登録。
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = StatusWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szStatusClassName;
    wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the guideline window.
    // ガイドラインウィンドウを登録。
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = GuideWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szGuideClassName;
    wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    return TRUE;
#undef CS_MZIME
} // MzIme::RegisterClasses

// キーボードレイアウトリストから自分のHKLを取得する。
HKL MzIme::GetHKL(VOID)
{
    HKL hKL = NULL;

    // get list size and allocate buffer for list
    DWORD dwCount = ::GetKeyboardLayoutList(0, NULL);
    HKL* pHKLs = (HKL*)new(std::nothrow) HKL[dwCount];
    if (pHKLs  == NULL) {
        ASSERT(0);
        return NULL;
    }

    // get the list of keyboard layouts
    ::GetKeyboardLayoutList(dwCount, pHKLs);

    // find hKL from the list
    TCHAR szFile[32];
    for (DWORD dwi = 0; dwi < dwCount; dwi++) {
        HKL hKLTemp = pHKLs[dwi];
        ::ImmGetIMEFileName(hKLTemp, szFile, _countof(szFile));

        if (::lstrcmp(szFile, szImeFileName) == 0) { // IMEファイル名が一致した？
            hKL = hKLTemp;
            break;
        }
    }

    // free the list
    delete[] pHKLs;
    return hKL;
}

// Update the translate key buffer.
// メッセージを生成する。
BOOL MzIme::GenerateMessage(LPTRANSMSG lpGeneMsg)
{
    BOOL ret = FALSE;
    FOOTMARK_FORMAT("(%u,%d,%d)\n",
                    lpGeneMsg->message, lpGeneMsg->wParam, lpGeneMsg->lParam);

    if (m_lpCurTransKey)
        FOOTMARK_RETURN_INT(GenerateMessageToTransKey(lpGeneMsg));

    if (m_lpIMC && ::IsWindow(m_lpIMC->hWnd)) {
        DWORD dwNewSize = sizeof(TRANSMSG) * (m_lpIMC->NumMsgBuf() + 1);
        m_lpIMC->hMsgBuf = ImmReSizeIMCC(m_lpIMC->hMsgBuf, dwNewSize);
        if (m_lpIMC->hMsgBuf) {
            LPTRANSMSG lpTransMsg = m_lpIMC->LockMsgBuf();
            if (lpTransMsg) {
                lpTransMsg[m_lpIMC->NumMsgBuf()++] = *lpGeneMsg;
                m_lpIMC->UnlockMsgBuf();
                ret = ImmGenerateMessage(m_hIMC);
            }
        }
    }
    FOOTMARK_RETURN_INT(ret);
}

// メッセージを生成する。
BOOL MzIme::GenerateMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    FOOTMARK_FORMAT("(%u, 0x%08lX, 0x%08lX)\n", message, wParam, lParam);
    TRANSMSG genmsg;
    genmsg.message = message;
    genmsg.wParam = wParam;
    genmsg.lParam = lParam;
    FOOTMARK_RETURN_INT(GenerateMessage(&genmsg));
}

// Update the translate key buffer.
BOOL MzIme::GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg)
{
    // increment the number
    ++m_uNumTransKey;

    // check overflow
    if (m_uNumTransKey >= m_lpCurTransKey->uMsgCount) {
        m_fOverflowKey = TRUE;
        return FALSE;
    }

    // put one message to TRANSMSG buffer
    LPTRANSMSG lpgmT0 = m_lpCurTransKey->TransMsg + (m_uNumTransKey - 1);
    *lpgmT0 = *lpGeneMsg;

    return TRUE;
}

// mzimejaのコマンドを実行する。
BOOL MzIme::DoCommand(HIMC hIMC, DWORD dwCommand)
{
    switch (dwCommand) {
    case IDM_RECONVERT:
        break;
    case IDM_ABOUT:
        GenerateMessage(WM_IME_NOTIFY, IMN_PRIVATE, MAKELPARAM(0, 0xDEAD));
        break;
    case IDM_HIRAGANA:
        SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
        break;
    case IDM_FULL_KATAKANA:
        SetInputMode(hIMC, IMODE_FULL_KATAKANA);
        break;
    case IDM_FULL_ASCII:
        SetInputMode(hIMC, IMODE_FULL_ASCII);
        break;
    case IDM_HALF_KATAKANA:
        SetInputMode(hIMC, IMODE_HALF_KANA);
        break;
    case IDM_HALF_ASCII:
        SetInputMode(hIMC, IMODE_HALF_ASCII);
        break;
    case IDM_CANCEL:
        break;
    case IDM_ROMAN_INPUT:
        SetRomanMode(hIMC, TRUE);
        break;
    case IDM_KANA_INPUT:
        SetRomanMode(hIMC, FALSE);
        break;
    case IDM_HIDE:
        break;
    case IDM_PROPERTY:
        break;
    case IDM_ADD_WORD:
        break;
    case IDM_IME_PAD:
        GenerateMessage(WM_IME_NOTIFY, IMN_PRIVATE, MAKELPARAM(0, 0xFACE));
        break;
    case IDM_IME_PROPERTY:
        ImeConfigure(GetKeyboardLayout(0), NULL, IME_CONFIG_GENERAL, NULL);
        break;
    default:
        return FALSE;
    }
    return TRUE;
} // MzIme::DoCommand

// インジケーターのアイコンを更新。
void MzIme::UpdateIndicIcon(HIMC hIMC)
{
    if (m_hMyKL == NULL) {
        m_hMyKL = GetHKL();
        if (m_hMyKL == NULL) return;
    }

    // TODO: enable pen icon update
    HWND hwndIndicate = ::FindWindow(INDICATOR_CLASS, NULL);
    if (::IsWindow(hwndIndicate)) {
        BOOL fOpen = FALSE;
        if (hIMC) {
            fOpen = ImmGetOpenStatus(hIMC);
        }

        ATOM atomTip = ::GlobalAddAtom(TEXT("MZ-IME Open"));
        LPARAM lParam = (LPARAM)m_hMyKL;
        ::PostMessage(hwndIndicate, INDICM_SETIMEICON, fOpen ? 1 : (-1), lParam);
        ::PostMessage(hwndIndicate, INDICM_SETIMETOOLTIPS, (fOpen ? atomTip : (-1)),
                      lParam);
        ::PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS,
                      (fOpen ? (RDMI_LEFT) : 0), lParam);
    }
}

// ウィンドウクラスの登録を解除。
void MzIme::UnregisterClasses()
{
    ::UnregisterClass(szUIServerClassName, m_hInst);
    ::UnregisterClass(szCompStrClassName, m_hInst);
    ::UnregisterClass(szCandClassName, m_hInst);
    ::UnregisterClass(szStatusClassName, m_hInst);
}

// ビットマップをリソースから読み込む。
HBITMAP MzIme::LoadBMP(LPCTSTR pszName)
{
    return ::LoadBitmap(m_hInst, pszName);
}

// 文字列をリソースから読み込む。
WCHAR *MzIme::LoadSTR(INT nID)
{
    static WCHAR sz[512];
    sz[0] = 0;
    ::LoadStringW(m_hInst, nID, sz, _countof(sz));
    return sz;
}

// 入力コンテキストをロックする。
InputContext *MzIme::LockIMC(HIMC hIMC)
{
    InputContext *context = (InputContext *)::ImmLockIMC(hIMC);
    if (context) {
        m_hIMC = hIMC;
        m_lpIMC = context;
    }
    return context;
}

// 入力コンテキストのロックを解除。
VOID MzIme::UnlockIMC(HIMC hIMC)
{
    ::ImmUnlockIMC(hIMC);
    if (::ImmGetIMCLockCount(hIMC) == 0) {
        m_hIMC = NULL;
        m_lpIMC = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////
// UI extra related
// 余剰情報。

HGLOBAL GetUIExtraFromServerWnd(HWND hwndServer)
{
    return (HGLOBAL)GetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE);
}

void SetUIExtraToServerWnd(HWND hwndServer, HGLOBAL hUIExtra)
{
    ::SetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE, (LONG_PTR)hUIExtra);
}

UIEXTRA *LockUIExtra(HWND hwndServer)
{
    HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
    UIEXTRA *lpUIExtra = (UIEXTRA *)::GlobalLock(hUIExtra);
    ASSERT(lpUIExtra);
    return lpUIExtra;
}

void UnlockUIExtra(HWND hwndServer)
{
    HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
    ::GlobalUnlock(hUIExtra);
}

void FreeUIExtra(HWND hwndServer)
{
    HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
    ::GlobalFree(hUIExtra);
    ::SetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE, (LONG_PTR)NULL);
}

LPCTSTR findLocalFile(LPCTSTR name)
{
    TCHAR szDir[MAX_PATH];
    ::GetModuleFileName(NULL, szDir, _countof(szDir));
    ::PathRemoveFileSpec(szDir);

    static TCHAR s_szPath[MAX_PATH];
    StringCchCopy(s_szPath, _countof(s_szPath), szDir);
    ::PathAppend(s_szPath, name);
    if (::PathFileExists(s_szPath))
        return s_szPath;

    StringCchCopy(s_szPath, _countof(s_szPath), szDir);
    ::PathAppend(s_szPath, TEXT(".."));
    ::PathAppend(s_szPath, name);
    if (::PathFileExists(s_szPath))
        return s_szPath;

    StringCchCopy(s_szPath, _countof(s_szPath), szDir);
    ::PathAppend(s_szPath, TEXT(".."));
    ::PathAppend(s_szPath, TEXT(".."));
    ::PathAppend(s_szPath, name);
    if (::PathFileExists(s_szPath))
        return s_szPath;

    ASSERT(0);
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////

// IMEはDLLファイルの一種であるから、IMEが読み込まれたら、エントリーポイントの
// DllMainが呼び出されるはず。
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    FOOTMARK_FORMAT("(%p, 0x%08lX, %p)\n", hInstDLL, fdwReason, lpvReserved);

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hInstDLL);
        TheIME.Init(hInstDLL); // 初期化。
        break;

    case DLL_PROCESS_DETACH:
        TheIME.Uninit(); // 逆初期化。
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    FOOTMARK_RETURN_INT(TRUE);
} // DllMain

//////////////////////////////////////////////////////////////////////////////

void DoIt(const std::wstring& pre)
{
    MzConvResult result;
    TheIME.ConvertMultiClause(pre, result);
    printf("%ls\n\n", result.get_str(false).c_str());
    printf("%ls\n\n", result.get_str(true).c_str());
}

// mzimejaのテスト。
void IME_Test1(void)
{
    DoIt(L"てすとです");
    DoIt(L"そこではなしはおわりになった");
    DoIt(L"わたしがわたしたわたをわたがしみたいにたべないでくださいませんか");
}

BOOL OnOK(HWND hwnd)
{
    WCHAR szText[1024];
    GetDlgItemTextW(hwnd, edt1, szText, _countof(szText));
    StrTrimW(szText, L" \t\r\n");
    if (szText[0] == 0) {
        MessageBoxW(hwnd, L"空ではない文字列を入力して下さい", NULL, 0);
        return FALSE;
    }
    DoIt(szText);
    return TRUE;
}

static INT_PTR CALLBACK
InputDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (OnOK(hwnd)) {
                EndDialog(hwnd, IDOK);
            }
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
    return 0;
}

void IME_Test2(void)
{
    while (::DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INPUTBOX),
                       NULL, InputDialogProc) == IDOK)
    {
        ;
    }
}

// Unicode版のmain関数。
int wmain(int argc, wchar_t **argv)
{
    // Unicode出力を可能に。
    std::setlocale(LC_CTYPE, "");

    LPCTSTR pathname = findLocalFile(L"res\\mzimeja.dic");
    //LPCTSTR pathname = findLocalFile(L"res\\testdata.dic");
    if (!g_basic_dict.Load(pathname, L"BasicDictObject")) {
        ASSERT(0);
        return 1;
    }

    // テスト1。
    IME_Test1();

    // テスト2。
    IME_Test2();

    g_basic_dict.Unload();

    return 0;
}

// 古いコンパイラのサポートのため。
int main(void)
{
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    int ret = wmain(argc, argv);
    LocalFree(argv);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"
