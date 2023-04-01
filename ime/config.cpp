// config.cpp --- mzimeja configuration
// mzimejaの設定。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include <prsht.h>
#include <commctrl.h>
#include <windowsx.h>
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

// 品詞リストを埋める。
void RegWord_PopulateHinshi(HWND hDlg) {
    // TODO:
}

// 単語を追加または削除する。
BOOL RegWord_AddWord(HWND hDlg, BOOL bAdd) {
    // 単語を取得。
    TCHAR szWord[MAX_PATH];
    ::GetDlgItemText(hDlg, edt1, szWord, _countof(szWord));

    // 品詞を取得。
    HWND hCmb1 = GetDlgItem(hDlg, cmb1);
    TCHAR szHinshi[MAX_PATH];
    ComboBox_GetText(hCmb1, szHinshi, _countof(szHinshi));

    // 読みを取得。
    TCHAR szYomi[MAX_PATH];
    ::GetDlgItemText(hDlg, edt2, szYomi, _countof(szYomi));

    // 会社名キーを開く。
    HKEY hKey;
    LONG error = ::RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Katayama Hirofumi MZ"), 0, NULL,
                                  0, KEY_WRITE, NULL, &hKey, NULL);
    if (error) {
        DPRINT("error: 0x%08lX", error);
        return TRUE;
    }

    // アプリキーを開く。
    HKEY hAppKey;
    error = ::RegCreateKeyEx(hKey, TEXT("mzimeja"), 0, NULL, 0, KEY_WRITE, NULL, &hAppKey, NULL);
    if (error) {
        DPRINT("error: 0x%08lX", error);
        ::RegCloseKey(hKey);
        return TRUE;
    }

    if (bAdd) {
        // 値文字列は、"品詞:読み"の形。
        TCHAR szValue[MAX_PATH];
        lstrcpyn(szValue, szHinshi, _countof(szValue));
        INT cchValue;
        cchValue = lstrlen(szValue);
        lstrcpyn(szValue + cchValue, TEXT(":"), _countof(szValue) - cchValue);
        cchValue = lstrlen(szValue);
        lstrcpyn(szValue + cchValue, szYomi, _countof(szValue) - cchValue);
        cchValue = lstrlen(szValue);

        // レジストリに値をセット。
        ::RegSetValueEx(hAppKey, szWord, 0, REG_SZ, (LPBYTE)szValue, (cchValue + 1) * sizeof(TCHAR));
    } else {
        // レジストリの値を削除。
        ::RegDeleteValue(hAppKey, szWord);
    }

    // レジストリキーを閉じる。
    ::RegCloseKey(hAppKey);
    ::RegCloseKey(hKey);

    return TRUE;
}

// IDD_ADDDELWORD - 単語の登録ダイアログ。
INT_PTR CALLBACK
RegWordDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    BOOL bAdd = !!GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg) {
    case WM_INITDIALOG:
        bAdd = !!lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, bAdd);
        RegWord_PopulateHinshi(hDlg);
        if (bAdd) {
            CheckRadioButton(hDlg, rad1, rad2, rad1);
        } else {
            CheckRadioButton(hDlg, rad1, rad2, rad2);
            EnableWindow(GetDlgItem(hDlg, cmb1), FALSE);
            EnableWindow(GetDlgItem(hDlg, edt2), FALSE);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (RegWord_AddWord(hDlg, bAdd))
            {
                ::EndDialog(hDlg, IDOK);
            }
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

// 単語リストを埋める。
void WordList_PopulateList(HWND hDlg)
{
    // レジストリキーを開く。
    HKEY hKey;
    LONG error = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                                TEXT("SOFTWARE\\Katayama Hirofumi MZ\\mzimeja"),
                                0, KEY_READ, &hKey);
    if (error) {
        DPRINT("error: 0x%08lX", error);
        return;
    }

    // 値を列挙する。
    for (DWORD dwIndex = 0; dwIndex < 0x1000; ++dwIndex)
    {
        // 値の名前を取得する。
        TCHAR szValueName[MAX_PATH];
        DWORD cchValueName = sizeof(szValueName);
        error = ::RegEnumValue(hKey, dwIndex, szValueName, &cchValueName, NULL, NULL, NULL, NULL);
        if (error) {
            break;
        }
        szValueName[_countof(szValueName) - 1] = 0;

        // 値を取得する。
        TCHAR szValue[MAX_PATH];
        DWORD cbValue = sizeof(szValue);
        error = ::RegQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE)szValue, &cbValue);
        if (error) {
            DPRINT("error: 0x%08lX", error);
            break;
        }
        szValue[_countof(szValue) - 1] = 0;

        // コロンで値の文字列を分割する。
        LPWSTR pch = wcschr(szValue, L':');
        if (pch == NULL)
            continue;
        *pch++ = 0;

        // 単語。
        LV_ITEM item = { LVIF_TEXT, -1, 0 };
        item.pszText = szValueName;
        INT iItem = ListView_InsertItem(hDlg, &item);

        // 読み。
        item.iItem = iItem;
        item.iSubItem = 1;
        item.pszText = pch;
        ListView_SetItem(hDlg, &item);

        // 品詞。
        item.iItem = iItem;
        item.iSubItem = 2;
        item.pszText = szValue;
        ListView_SetItem(hDlg, &item);
    }

    // レジストリキーを閉じる。
    ::RegCloseKey(hKey);
}

// IDD_WORDLIST - 単語の一覧プロパティシート。
INT_PTR CALLBACK
WordListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        WordList_PopulateList(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case psh1: // 追加。
            if (::DialogBoxParam(TheIME.m_hInst, MAKEINTRESOURCE(IDD_ADDDELWORD),
                                 hDlg, RegWordDlgProc, TRUE) == IDOK)
            {
                // リストを更新。
                HWND hLst1 = ::GetDlgItem(hDlg, lst1);
                SetWindowRedraw(hLst1, FALSE);
                ListView_DeleteAllItems(hLst1);
                WordList_PopulateList(hDlg);
                SetWindowRedraw(hLst1, TRUE);
                InvalidateRect(hLst1, NULL, TRUE);
            }
            break;
        case psh2: // 削除。
            if (::DialogBoxParam(TheIME.m_hInst, MAKEINTRESOURCE(IDD_ADDDELWORD),
                                 hDlg, RegWordDlgProc, FALSE) == IDOK)
            {
                // リストを更新。
                HWND hLst1 = ::GetDlgItem(hDlg, lst1);
                SetWindowRedraw(hLst1, FALSE);
                ListView_DeleteAllItems(hLst1);
                WordList_PopulateList(hDlg);
                SetWindowRedraw(hLst1, TRUE);
                InvalidateRect(hLst1, NULL, TRUE);
            }
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
