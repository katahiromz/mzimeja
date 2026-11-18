// main.cpp --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include <shlobj.h>
#include <strsafe.h>
#include <algorithm>
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

// The table of guideline.
// ガイドラインのテーブル。
const MZGUIDELINE guideline_table[] = {
    {GL_LEVEL_ERROR, GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
    {GL_LEVEL_WARNING, GL_ID_TYPINGERROR, IDS_GL_TYPINGERROR, 0},
    {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST, IDS_GL_TESTGUIDELINESTR, IDS_GL_TESTGUIDELINEPRIVATE}
};

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
    m_initialized = FALSE;

    m_lpCurTransKey = NULL;
    m_uNumTransKey = 0;

    m_fOverflowKey = FALSE;

    m_hIMC = NULL;
    m_lpIMC = NULL;
    ZeroMemory(m_atoms, sizeof(m_atoms));
}

void MzIme::Attach(HINSTANCE hInstance)
{
    m_hInst = hInstance;
    ::InitializeCriticalSection(&m_initLock);
}

BOOL MzIme::EnsureInitialized(DWORD dwSystemInfoFlags)
{
    ::EnterCriticalSection(&m_initLock);
    if (dwSystemInfoFlags & IME_SYSINFO_WINLOGON)
        m_bWinLogOn = TRUE;
    if (!m_initialized) {
        m_initialized = Init(m_hInst);
        if (!m_initialized)
            Uninit();
    }
    BOOL initialized = m_initialized;
    ::LeaveCriticalSection(&m_initLock);
    return initialized;
}

void MzIme::Detach()
{
    if (m_initialized)
        Uninit();
    ::DeleteCriticalSection(&m_initLock);
}

// mzimejaの辞書を読み込む。
BOOL MzIme::LoadDict()
{
    BOOL ret = TRUE;

    std::wstring basic;
    if (!Config_GetDWORD(L"BasicDictDisabled", FALSE)) {
        if (FindLocalFile(basic, L"basic.dic") ||
            FindLocalFile(basic, L"res\\basic.dic") ||
            FindAppFile(basic, L"basic.dic") ||
            FindAppFile(basic, L"res\\basic.dic") ||
            Config_GetSz(L"BasicDictPathName", basic))
        {
            if (!g_basic_dict.Load(basic.c_str(), L"BasicDictObject")) {
                ret = FALSE;
            }
        }
    } else {
        g_basic_dict.Unload();
    }

    std::wstring name;
    if (!Config_GetDWORD(L"NameDictDisabled", FALSE)) {
        if (FindLocalFile(name, L"name.dic") ||
            FindLocalFile(name, L"res\\name.dic") ||
            FindAppFile(name, L"name.dic") ||
            FindAppFile(name, L"res\\name.dic") ||
            Config_GetSz(L"NameDictPathName", name))
        {
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

    mz_make_literal_maps();

    // load dict
    LoadDict();

#ifdef HAVE_VIBRATO
    // Vibrato engine initialization
    std::wstring vibrato_dict_path;
    if (FindLocalFile(vibrato_dict_path, L"vibrato\\ipadic.vibrato") ||
        FindLocalFile(vibrato_dict_path, L"res\\vibrato\\ipadic.vibrato") ||
        FindAppFile(vibrato_dict_path, L"vibrato\\ipadic.vibrato") ||
        FindAppFile(vibrato_dict_path, L"res\\vibrato\\ipadic.vibrato"))
    {
        if (g_vibrato_engine.Initialize(vibrato_dict_path)) {
            DPRINTW(L"Vibrato engine enabled\n");
        } else {
            DPRINTW(L"Vibrato engine initialization failed\n");
        }
    } else {
        DPRINTW(L"Vibrato dictionary not found, using legacy engine\n");
    }
#endif

    // Load atoms
    LoadAtoms();

    // register window classes for IME
    return RegisterClasses(m_hInst);
} // MzIme::Init

// mzimejaを逆初期化。
VOID MzIme::Uninit(VOID)
{
    UnregisterClasses();
    UnloadDict();
    UnloadAtoms();
}

BOOL MzIme::LoadAtoms()
{
    TCHAR szText[64];
    INT i = 0;
    // アトムを追加する。
    LoadString(m_hInst, IDS_HIRAGANA, szText, _countof(szText));
    m_atoms[i++] = ::GlobalAddAtom(szText);
    LoadString(m_hInst, IDS_ZENKAKU_KATAKANA, szText, _countof(szText));
    m_atoms[i++] = ::GlobalAddAtom(szText);
    LoadString(m_hInst, IDS_ZENKAKU_EISUU, szText, _countof(szText));
    m_atoms[i++] = ::GlobalAddAtom(szText);
    LoadString(m_hInst, IDS_HANKAKU_KANA, szText, _countof(szText));
    m_atoms[i++] = ::GlobalAddAtom(szText);
    LoadString(m_hInst, IDS_HANKAKU_EISUU, szText, _countof(szText));
    m_atoms[i++] = ::GlobalAddAtom(szText);
    LoadString(m_hInst, IDS_IMEDISABLED, szText, _countof(szText));
    m_atoms[i++] = ::GlobalAddAtom(szText);
    ASSERT(i == _countof(m_atoms));
    return TRUE;
}

void MzIme::UnloadAtoms()
{
    // アトムを削除する。
    for (size_t i = 0; i < _countof(m_atoms); ++i)
    {
        ::GlobalDeleteAtom(m_atoms[i]);
        m_atoms[i] = 0;
    }
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
        DPRINTA("IDM_ABOUT\n");
        {
            std::wstring verinfo_file;
            if (FindAppFile(verinfo_file, L"verinfo.exe") ||
                Config_GetSz(L"VerInfoFile", verinfo_file))
            {
                ::ShellExecuteW(NULL, NULL, verinfo_file.c_str(), NULL, NULL, SW_SHOWNOACTIVATE);
            }
        }
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
        DPRINTA("IDM_IME_PAD\n");
        {
            std::wstring imepad_file;
            if (FindLocalFile(imepad_file, L"imepad.exe") ||
                FindAppFile(imepad_file, L"imepad.exe") ||
                Config_GetSz(L"ImePadFile", imepad_file))
            {
                ::ShellExecuteW(NULL, NULL, imepad_file.c_str(), NULL, NULL, SW_SHOWNOACTIVATE);
            }
        }
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

        LPARAM lParam = (LPARAM)m_hMyKL;
        ::PostMessage(hwndIndicate, INDICM_SETIMEICON, fOpen ? 1 : (-1), lParam);
        ::PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS, 0, lParam);
    }
}

// ウィンドウクラスの登録を解除。
void MzIme::UnregisterClasses()
{
    ::UnregisterClass(szUIServerClassName, m_hInst);
    ::UnregisterClass(szCompStrClassName, m_hInst);
    ::UnregisterClass(szCandClassName, m_hInst);
    ::UnregisterClass(szStatusClassName, m_hInst);
    ::UnregisterClass(szGuideClassName, m_hInst);
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

//////////////////////////////////////////////////////////////////////////////

// ローカルファイルを検索する。
LPCTSTR mz_find_local_file(LPCTSTR name)
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

    return NULL;
}

// グラフを描画するプログラムGraphvizを探す。
LPCTSTR mz_find_graphviz(void)
{
    static std::wstring s_strPath;
    LPCTSTR str1 = TEXT("C:\\Program Files\\Graphviz\\bin\\dot.exe");
    LPCTSTR str2 = TEXT("C:\\Program Files (x86)\\Graphviz\\bin\\dot.exe");
    if (s_strPath.empty()) {
        if (PathFileExists(str1))
            s_strPath = str1;
        else if (PathFileExists(str2))
            s_strPath = str2;
    }
    return s_strPath.c_str();
}

//////////////////////////////////////////////////////////////////////////////

// IMEはDLLファイルの一種であるから、IMEが読み込まれたら、エントリーポイントの
// DllMainが呼び出されるはず。
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hInstDLL);
        TheIME.Attach(hInstDLL);
        break;

    case DLL_PROCESS_DETACH:
        TheIME.Detach();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
} // DllMain

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"
