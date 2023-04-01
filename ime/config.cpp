// config.cpp --- mzimeja configuration
// mzimejaの設定。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include <prsht.h>
#include "resource.h"

#define MAX_PAGES 5

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// IDD_GENERAL - 全般設定プロパティシート。
INT_PTR CALLBACK
GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    default:
        break;
    }
    return FALSE;
} // GeneralDlgProc

// IDD_ADDDELWORD - 単語の登録ダイアログ。
INT_PTR CALLBACK
RegWordDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static BOOL s_bAdd = TRUE;

    switch (uMsg) {
    case WM_INITDIALOG:
        s_bAdd = !!lParam;
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            // TODO:
            ::EndDialog(hDlg, IDOK);
            break;
        case IDCANCEL:
            ::EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
} // RegWordDlgProc

// IDD_CHOOSEDICT - 辞書の選択ダイアログ。
INT_PTR CALLBACK
ChooseDictDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            ::EndDialog(hDlg, IDOK);
            break;
        case IDCANCEL:
            ::EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
} // ChooseDictDlgProc

// IDD_WORDLIST - 単語の一覧プロパティシート。
INT_PTR CALLBACK
WordListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case psh1: // 追加。
            ::DialogBoxParam(TheIME.m_hInst, MAKEINTRESOURCE(IDD_ADDDELWORD), hDlg, RegWordDlgProc, TRUE);
            break;
        case psh2: // 削除。
            ::DialogBoxParam(TheIME.m_hInst, MAKEINTRESOURCE(IDD_ADDDELWORD), hDlg, RegWordDlgProc, FALSE);
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
}

// IDD_ABOUTIME - IMEのバージョン情報プロパティシート。
INT_PTR CALLBACK
AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    default:
        break;
    }
    return FALSE;
} // AboutDlgProc

// IDD_DEBUG - デバッグオプション。
INT_PTR CALLBACK
DebugOptionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        break;

    default:
        break;
    }
    return FALSE;
} // DebugOptionDlgProc

// ページをプロパティシートに追加するヘルパー関数。
static void AddPage(LPPROPSHEETHEADER ppsh, UINT id, DLGPROC pfn) {
    if (ppsh->nPages < MAX_PAGES) {
        PROPSHEETPAGE psp = { sizeof(psp) };
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = TheIME.m_hInst;
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.pfnDlgProc = pfn;
        ppsh->phpage[ppsh->nPages] = ::CreatePropertySheetPage(&psp);
        if (ppsh->phpage[ppsh->nPages]) ppsh->nPages++;
    }
} // AddPage

//  ImeConfigure
//    ImeConfigure 関数は IME に対して補足的な情報を要求するために使う
//    Dialog Box を提供する？
//  BOOL
//    ImeConfigure(
//    HKL hKL,
//    HWND hWnd,
//    DWORD dwMode,
//    LPVOID lpData
//    )
//  Parameters
//    hKL
//      この IME の入力言語のハンドル。
//    hWnd
//      親 Window のハンドル。
//    dwMode
//      Dialog のモード。以下のようなフラグが与えられる。
//      IME_CONFIG_GENERAL      一般 configuration のための Dialog
//      IME_CONFIG_REGWORD      単語登録のための Dialog
//      IME_CONFIG_SELECTDICTIONARY IME 辞書選択のための Dialog
//    lpData
//      VOID 型のポインタ。もし dwMode == IME_CONFIG_REGISTERWORD なら、
//      REGISTERWORD 構造体へのポインタとなる。さもなくば無視される。
//      initial string が与えられなかったら、IME_CONFIG_REGISTER モード
//      であっても、NULL であってかまわない。
//    Return Values
//      この関数が成功したら、TRUE。さもなくば FALSE。
//  Comments
//    IME は次のような擬似コードでもって lpData をチェックする。
//
//  if (dwmode != IME_CONFIG_REGISTERWORD){
//    // Does original execution
//  } else if (IsBadReadPtr(lpdata, sizeof(REGISTERWORD))==FALSE){
//    if (IsBadStringPtr(PREGISTERWORD(lpdata)->lpReading, (UINT)-1)==FALSE){
//      // Set the reading string to word registering dialogbox
//    }
//    if (IsBadStringPtr(PREGISTERWORD(lpdata)->lpWord, (UINT)-1)==FALSE){
//      // Set the word string to word registering dialogbox
//    }
//  }
BOOL WINAPI ImeConfigure(HKL hKL, HWND hWnd, DWORD dwMode, LPVOID lpData) {
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;
    FOOTMARK();

    // プロパティシートを開く準備。
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hwndParent = hWnd;
    psh.hInstance = TheIME.m_hInst;
    psh.pszCaption = MAKEINTRESOURCE(IDS_CONFIGNAME);
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = rPages;

    // モードに応じてプロパティシートを選ぶ。
    switch (dwMode) {
    case IME_CONFIG_GENERAL: // 全般設定。
        AddPage(&psh, IDD_GENERAL, GeneralDlgProc);
        AddPage(&psh, IDD_WORDLIST, WordListDlgProc);
        AddPage(&psh, IDD_ABOUTIME, AboutDlgProc);
#ifdef _DEBUG
        AddPage(&psh, IDD_DEBUG, DebugOptionDlgProc);
#endif
        ::PropertySheet(&psh);
        break;

    case IME_CONFIG_REGISTERWORD: // 単語登録。
        ::DialogBoxParam(TheIME.m_hInst, MAKEINTRESOURCE(IDD_ADDDELWORD), hWnd, RegWordDlgProc, TRUE);
        break;

    case IME_CONFIG_SELECTDICTIONARY: // 辞書の選択。
        ::DialogBox(TheIME.m_hInst, MAKEINTRESOURCE(IDD_CHOOSEDICT), hWnd, ChooseDictDlgProc);
        break;

    default:
        break;
    }

    return TRUE;
} // ImeConfigure

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
