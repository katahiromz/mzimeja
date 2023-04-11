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

// レジストリのアプリキーを開く。
HKEY Config_OpenAppKey(VOID)
{
    HKEY hAppKey;
    LONG error = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                                TEXT("SOFTWARE\\Katayama Hirofumi MZ\\mzimeja"),
                                0, KEY_READ, &hAppKey);
    if (error) {
        DPRINTA("0x%08lX\n", error);
        error = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               TEXT("SOFTWARE\\Katayama Hirofumi MZ\\mzimeja"),
                               0, KEY_READ, &hAppKey);
        if (error) {
            DPRINTA("0x%08lX\n", error);
            return NULL;
        }
    }
    return hAppKey;
}

// レジストリのアプリキーを作成する。
HKEY Config_CreateAppKey(VOID)
{
    // 会社名キーを作成する。
    HKEY hCompanyKey;
    LONG error = ::RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Katayama Hirofumi MZ"),
                                  0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hCompanyKey, NULL);
    if (error) {
        DPRINTA("error: 0x%08lX\n", error);
        return NULL;
    }

    // アプリキーを開く。
    HKEY hAppKey;
    error = ::RegCreateKeyEx(hCompanyKey, TEXT("mzimeja"), 0, NULL, 0,
                             KEY_READ | KEY_WRITE, NULL, &hAppKey, NULL);
    if (error) {
        DPRINTA("error: 0x%08lX\n", error);
        ::RegCloseKey(hCompanyKey);
        return NULL;
    }

    ::RegCloseKey(hCompanyKey);
    return hAppKey;
}

// レジストリからDWORD値を読み込む。
DWORD Config_GetDWORD(LPCTSTR name, DWORD dwDefault)
{
    HKEY hKey = Config_OpenAppKey();
    if (!hKey)
        return dwDefault;

    DWORD dwValue = dwDefault;
    DWORD cbValue = sizeof(dwValue);
    LONG error = ::RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE)&dwValue, &cbValue);
    ::RegCloseKey(hKey);
    if (error || cbValue != sizeof(DWORD)) {
        DPRINTA("error: 0x%08lX\n", error);
        return dwDefault;
    }

    return dwValue;
}

// レジストリにDWORD値を書き込む。
BOOL Config_SetDWORD(LPCTSTR name, DWORD dwValue)
{
    HKEY hKey = Config_CreateAppKey();
    if (!hKey)
        return FALSE;

    LONG error = ::RegSetValueEx(hKey, name, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
    ::RegCloseKey(hKey);
    if (error) {
        DPRINTA("error: 0x%08lX\n", error);
        return FALSE;
    }

    return TRUE;
}

// レジストリからバイナリ値を読み込む。
BOOL Config_GetData(LPCTSTR name, LPVOID pvData, DWORD cbData)
{
    HKEY hKey = Config_OpenAppKey();
    if (!hKey)
        return FALSE;

    DWORD cbDataOld = cbData;
    LONG error = ::RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE)pvData, &cbData);
    ::RegCloseKey(hKey);
    if (error || cbDataOld != cbData) {
        DPRINTA("error: 0x%08lX\n", error);
        return FALSE;
    }

    return TRUE;
}

// レジストリにバイナリ値を書き込む。
BOOL Config_SetData(LPCTSTR name, DWORD dwType, LPCVOID pvData, DWORD cbData)
{
    HKEY hKey = Config_CreateAppKey();
    if (!hKey)
        return FALSE;

    LONG error = ::RegSetValueEx(hKey, name, 0, dwType, (const BYTE*)pvData, cbData);
    ::RegCloseKey(hKey);
    if (error) {
        DPRINTA("error: 0x%08lX\n", error);
        return FALSE;
    }

    return TRUE;
}

// レジストリから文字列値を読み込む。
BOOL Config_GetSz(LPCTSTR name, std::wstring& str)
{
    str.clear();

    HKEY hKey = Config_OpenAppKey();
    if (!hKey)
        return FALSE;

    TCHAR szText[MAX_PATH];
    DWORD cbData = sizeof(szText);
    LONG error = ::RegQueryValueEx(hKey, name, NULL, NULL, (LPBYTE)szText, &cbData);
    szText[_countof(szText) - 1] = 0;
    ::RegCloseKey(hKey);
    if (error) {
        DPRINTA("error: 0x%08lX\n", error);
        return FALSE;
    }

    str = szText;
    return TRUE;
}

// レジストリに文字列値を書き込む。
BOOL Config_SetSz(LPCTSTR name, LPCTSTR psz)
{
    return Config_SetData(name, REG_SZ, psz, (lstrlen(psz) + 1) * sizeof(TCHAR));
}

// IDD_GENERAL - 全般設定プロパティシートページ。
INT_PTR CALLBACK
GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmhdr = (LPNMHDR)lParam;
    switch (uMsg) {
    case WM_INITDIALOG:
        if (Config_GetDWORD(TEXT("bCommaPeriod"), FALSE))
            ::CheckDlgButton(hDlg, chx1, BST_CHECKED);
        else
            ::CheckDlgButton(hDlg, chx1, BST_UNCHECKED);
        if (Config_GetDWORD(TEXT("bNoZenkakuAscii"), FALSE))
            ::CheckDlgButton(hDlg, chx2, BST_CHECKED);
        else
            ::CheckDlgButton(hDlg, chx2, BST_UNCHECKED);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case chx1:
        case chx2:
            ::PropSheet_Changed(::GetParent(hDlg), hDlg);
            break;
        }
        break;

    case WM_NOTIFY:
        switch (pnmhdr->code) {
        case PSN_APPLY:
            if (::IsDlgButtonChecked(hDlg, chx1) == BST_CHECKED) {
                Config_SetDWORD(TEXT("bCommaPeriod"), TRUE);
            } else {
                Config_SetDWORD(TEXT("bCommaPeriod"), FALSE);
            }
            if (::IsDlgButtonChecked(hDlg, chx2) == BST_CHECKED) {
                Config_SetDWORD(TEXT("bNoZenkakuAscii"), TRUE);
            } else {
                Config_SetDWORD(TEXT("bNoZenkakuAscii"), FALSE);
            }
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
} // GeneralDlgProc

// 品詞リストを埋める。
void RegWord_PopulateHinshi(HWND hDlg)
{
    HWND hCmb1 = GetDlgItem(hDlg, cmb1);
    for (INT i = IDS_HINSHI_00; i <= IDS_HINSHI_26; ++i) {
        LPTSTR psz = TheIME.LoadSTR(i);
        ComboBox_AddString(hCmb1, psz);
    }
    ComboBox_SetCurSel(hCmb1, 0);
}

// 単語を追加する。
BOOL RegWord_AddWord(HWND hDlg, LPCTSTR pszWord OPTIONAL)
{
    // 単語を取得。
    TCHAR szWord[MAX_PATH];
    if (!pszWord) {
        ::GetDlgItemText(hDlg, edt1, szWord, _countof(szWord));
        pszWord = szWord;
    }

    // 品詞を取得。
    HWND hCmb1 = GetDlgItem(hDlg, cmb1);
    INT iHinshi = ComboBox_GetCurSel(hCmb1);
    if (iHinshi < 0)
        return FALSE;

    // 読みを取得。
    TCHAR szYomi[MAX_PATH];
    ::GetDlgItemText(hDlg, edt2, szYomi, _countof(szYomi));

    HinshiBunrui hinshi = (HinshiBunrui)(HB_MEISHI + iHinshi);

    BOOL bWrong = FALSE;
    INT len = lstrlen(pszWord);
    switch (hinshi) {
    case HB_NAKEIYOUSHI: // な形容詞
        if (len == 0 || pszWord[len - 1] != 0x306A) // 「な」で終わらなければならない。
            bWrong = TRUE;
        break;
    case HB_IKEIYOUSHI: // い形容詞
        if (len == 0 || pszWord[len - 1] != 0x3044) // 「い」で終わらなければならない。
            bWrong = TRUE;
        break;
    case HB_ICHIDAN_DOUSHI: // 一段動詞
        if (len == 0 || pszWord[len - 1] != 0x308B) // 「る」で終わらなければならない。
            bWrong = TRUE;
        break;
    case HB_KAHEN_DOUSHI: // カ変動詞
        if (len <= 1 ||
            (pszWord[len - 2] != 0x304F && pszWord[len - 1] != 0x308B)) // 「くる」で終わらなければならない。
        {
            bWrong = TRUE;
        }
        break;
    case HB_SAHEN_DOUSHI: // サ変動詞
        if (len <= 1 || 
            (pszWord[len - 2] != 0x3059 && pszWord[len - 2] != 0x305A) ||
            pszWord[len - 1] != 0x308B) // 「する」「ずる」で終わらなければならない。
        {
            bWrong = TRUE;
        }
        break;
    case HB_GODAN_DOUSHI: // 五段動詞
        MakeLiteralMaps();
        if (len == 0 || g_vowel_map[pszWord[len - 1]] != 0x3046) // う段で終わらなければならない。
            bWrong = TRUE;
        break;
    default:
        break;
    }

    if (bWrong) {
        MessageBoxW(hDlg, TheIME.LoadSTR(IDS_HINSHIWRONG), NULL, MB_ICONERROR);
        return FALSE;
    }

    return ImeRegisterWord(szYomi, HinshiToStyle(hinshi), pszWord);
}

// 単語を削除する。
BOOL RegWord_DeleteWord(HWND hDlg, INT iItem)
{
    HWND hLst1 = GetDlgItem(hDlg, lst1);

    // 選択項目のテキストを習得する。
    TCHAR szText1[MAX_PATH];
    TCHAR szText2[MAX_PATH];
    TCHAR szText3[MAX_PATH];
    szText1[0] = szText2[0] = szText3[0] = 0;
    LV_ITEM item = { LVIF_TEXT };
    item.iItem = iItem;
    item.iSubItem = 0;
    item.pszText = szText1;
    item.cchTextMax = _countof(szText1);
    ListView_GetItem(hLst1, &item);
    item.iSubItem = 1;
    item.pszText = szText2;
    item.cchTextMax = _countof(szText2);
    ListView_GetItem(hLst1, &item);
    item.iSubItem = 2;
    item.pszText = szText3;
    item.cchTextMax = _countof(szText3);
    ListView_GetItem(hLst1, &item);
    if (!szText1[0] || !szText2[0] || !szText3[0])
        return FALSE;

    HinshiBunrui hinshi = StringToHinshi(szText3);
    if (hinshi == HB_UNKNOWN)
        return FALSE;

    // 削除するか確認。
    if (MessageBox(hDlg, TheIME.LoadSTR(IDS_WANNADELETEWORD),
                   szText2, MB_ICONINFORMATION | MB_YESNOCANCEL) != IDYES)
    {
        return FALSE;
    }

    return ImeUnregisterWord(szText1, HinshiToStyle(hinshi), szText2);
}


// IDD_ADDWORD - 単語の登録ダイアログ。
INT_PTR CALLBACK
RegWordDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        RegWord_PopulateHinshi(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (RegWord_AddWord(hDlg, NULL)) {
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
ChooseDictDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

static INT CALLBACK EnumRegWordProc(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr, LPVOID lpData){
    HWND hLst1 = (HWND)lpData;

    // 読み。
    LV_ITEM item = { LVIF_TEXT };
    item.iItem = ListView_GetItemCount(hLst1);
    item.iSubItem = 0;
    item.pszText = const_cast<LPTSTR>(lpRead);
    INT iItem = ListView_InsertItem(hLst1, &item);

    // 単語。
    item.iItem = iItem;
    item.iSubItem = 1;
    item.pszText = const_cast<LPTSTR>(lpStr);
    ListView_SetItem(hLst1, &item);

    // 品詞。
    if ((dw & MZIME_REGWORD_STYLE) != MZIME_REGWORD_STYLE)
        return TRUE;
    HinshiBunrui hinshi = StyleToHinshi(dw);
    std::wstring strHinshi = HinshiToString(hinshi);
    item.iItem = iItem;
    item.iSubItem = 2;
    item.pszText = &strHinshi[0];
    ListView_SetItem(hLst1, &item);

    return TRUE;
}

// 単語リストを埋める。
void WordList_PopulateList(HWND hDlg)
{
    HWND hLst1 = GetDlgItem(hDlg, lst1);

    DWORD exstyle = ListView_GetExtendedListViewStyle(hLst1);
    exstyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
    ListView_SetExtendedListViewStyle(hLst1, exstyle);

    // すべてを削除。
    ListView_DeleteColumn(hLst1, 2);
    ListView_DeleteColumn(hLst1, 1);
    ListView_DeleteColumn(hLst1, 0);
    ListView_DeleteAllItems(hLst1);

    // カラムを追加。
    LV_COLUMN column = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 100 };
    column.pszText = TheIME.LoadSTR(IDS_READING);
    ListView_InsertColumn(hLst1, 0, &column);
    column.pszText = TheIME.LoadSTR(IDS_WORD);
    ListView_InsertColumn(hLst1, 1, &column);
    column.pszText = TheIME.LoadSTR(IDS_HINSHI);
    ListView_InsertColumn(hLst1, 2, &column);

    // 項目を追加する。
    ImeEnumRegisterWord(EnumRegWordProc, NULL, 0, NULL, hLst1);
}

// IDD_WORDLIST - 単語の一覧プロパティシートページ。
INT_PTR CALLBACK
WordListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        WordList_PopulateList(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case psh1: // 追加。
            if (::DialogBox(TheIME.m_hInst, MAKEINTRESOURCE(IDD_ADDWORD),
                            hDlg, RegWordDlgProc) == IDOK)
            {
                // リストを更新。
                HWND hLst1 = ::GetDlgItem(hDlg, lst1);
                SetWindowRedraw(hLst1, FALSE);
                WordList_PopulateList(hDlg);
                SetWindowRedraw(hLst1, TRUE);
                InvalidateRect(hLst1, NULL, TRUE);
            }
            break;
        case psh2: // 削除。
            {
                // 選択項目を取得する。
                HWND hLst1 = GetDlgItem(hDlg, lst1);
                INT iItem = ListView_GetNextItem(hLst1, -1, LVNI_SELECTED);
                if (iItem == -1)
                    return 0;

                if (RegWord_DeleteWord(hDlg, iItem)) {
                    // 単語を削除。
                    ListView_DeleteItem(hLst1, iItem);
                }
            }
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
}

// IDD_ABOUTIME - IMEのバージョン情報プロパティシートページ。
INT_PTR CALLBACK
AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
DebugOptionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szText[MAX_PATH];
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case psh1: // [Test 1] button
            GetDlgItemText(hDlg, edt1, szText, _countof(szText));
            MessageBox(hDlg, szText, TEXT("Test 1"), 0);
            break;
        case psh2: // [Test 2] button
            GetDlgItemText(hDlg, edt1, szText, _countof(szText));
            MessageBox(hDlg, szText, TEXT("Test 2"), 0);
            break;
        case psh3: // [Test 3] button
            GetDlgItemText(hDlg, edt1, szText, _countof(szText));
            MessageBox(hDlg, szText, TEXT("Test 3"), 0);
            break;
        }
        break;

    default:
        break;
    }
    return FALSE;
} // DebugOptionDlgProc

// ページをプロパティシートに追加するヘルパー関数。
static void AddPage(LPPROPSHEETHEADER ppsh, UINT id, DLGPROC pfn)
{
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
//      IME_CONFIG_GENERAL          一般 configuration のための Dialog
//      IME_CONFIG_REGISTERWORD     単語登録のための Dialog
//      IME_CONFIG_SELECTDICTIONARY IME 辞書選択のための Dialog
//    lpData
//      VOID 型のポインタ。もし dwMode == IME_CONFIG_REGISTERWORD なら、
//      REGISTERWORD 構造体へのポインタとなる。さもなくば無視される。
//      initial string が与えられなかったら、IME_CONFIG_REGISTER モード
//      であっても、NULL であってかまわない。
//    Return Values
//      この関数が成功したら、TRUE。さもなくば FALSE。
BOOL WINAPI ImeConfigure(HKL hKL, HWND hWnd, DWORD dwMode, LPVOID lpData)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;
    FOOTMARK();

    // プロパティシートを開く準備。
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW;
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
#ifndef NDEBUG
        AddPage(&psh, IDD_DEBUG, DebugOptionDlgProc);
#endif
        ::PropertySheet(&psh);
        break;

    case IME_CONFIG_REGISTERWORD: // 単語登録。
        ::DialogBox(TheIME.m_hInst, MAKEINTRESOURCE(IDD_ADDWORD), hWnd, RegWordDlgProc);
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
