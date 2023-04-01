// config.cpp --- mzimeja configuration
// mzimejaの設定。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include <prsht.h>
#include "resource.h"

#define MAX_PAGES 5

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// ページをプロパティシートに追加。
void PASCAL AddPage(LPPROPSHEETHEADER ppsh, UINT id, DLGPROC pfn) {
    if (ppsh->nPages < MAX_PAGES) {
        PROPSHEETPAGE psp;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = TheIME.m_hInst;
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.pfnDlgProc = pfn;
        psp.lParam = 0;

        ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
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

    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hwndParent = hWnd;
    psh.hInstance = TheIME.m_hInst;
    psh.pszCaption = MAKEINTRESOURCE(IDS_CONFIGNAME);
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = rPages;

    switch (dwMode) {
    case IME_CONFIG_GENERAL:
        AddPage(&psh, IDD_GENERAL, GeneralDlgProc);
        AddPage(&psh, IDD_WORDLIST, RegWordDlgProc);
        AddPage(&psh, IDD_CHOOSEDICT, ChooseDictDlgProc);
        AddPage(&psh, IDD_ABOUTIME, AboutDlgProc);
#ifdef _DEBUG
        AddPage(&psh, IDD_DEBUG, DebugOptionDlgProc);
#endif
        PropertySheet(&psh);
        break;

    case IME_CONFIG_REGISTERWORD:
        AddPage(&psh, IDD_ADDDELWORD, RegWordDlgProc);
        AddPage(&psh, IDD_ABOUTIME, AboutDlgProc);
        PropertySheet(&psh);
        break;

    case IME_CONFIG_SELECTDICTIONARY:
        AddPage(&psh, IDD_CHOOSEDICT, ChooseDictDlgProc);
        AddPage(&psh, IDD_ABOUTIME, AboutDlgProc);
        PropertySheet(&psh);
        break;

    default:
        break;
    }

    return TRUE;
} // ImeConfigure

INT_PTR CALLBACK
WordListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch (lpnm->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_APPLY:
            break;

        case PSN_RESET:
            break;

        case PSN_HELP:
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        ::SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
RegWordDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    NMHDR FAR *lpnm;
    //LPPROPSHEETPAGE lpPropSheet =
    //    (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    UINT nItem;
    UINT i;
    LRESULT dwIndex;
    TCHAR szRead[128];
    TCHAR szString[128];
    LRESULT dwStyle;

    switch (uMsg) {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch (lpnm->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_APPLY:
            if (!GetDlgItemText(hDlg, ID_WR_READING, szRead, _countof(szRead))) {
                WCHAR *psz = TheIME.LoadSTR(IDS_NOREADING);
                ::MessageBoxW(hDlg, psz, NULL, MB_OK);
                return FALSE;
            }

            if (!GetDlgItemText(hDlg, ID_WR_STRING, szString, _countof(szString))) {
                WCHAR *psz = TheIME.LoadSTR(IDS_NOSTRING);
                ::MessageBoxW(hDlg, psz, NULL, MB_OK);
                return FALSE;
            }

            dwIndex = SendDlgItemMessage(hDlg, ID_WR_STYLE, CB_GETCURSEL, 0, 0);
            dwStyle =
                    SendDlgItemMessage(hDlg, ID_WR_STYLE, CB_GETITEMDATA, dwIndex, 0);

            if (!ImeRegisterWord(szRead, (DWORD)dwStyle, szString)) {
                WCHAR *psz = TheIME.LoadSTR(IDS_REGWORDRET);
                ::MessageBoxW(hDlg, psz, NULL, MB_OK);
            }
            break;

        case PSN_RESET:
            break;

        case PSN_HELP:
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        ::SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
        //lpPropSheet = (LPPROPSHEETPAGE)lParam;

        nItem = ImeGetRegisterWordStyle(0, NULL);
        if (nItem) {
            LPSTYLEBUF lpStyleBuf =
                    (LPSTYLEBUF)GlobalAlloc(GPTR, nItem * sizeof(STYLEBUF));

            if (!lpStyleBuf) {
                WCHAR *psz = TheIME.LoadSTR(IDS_NOMEMORY);
                ::MessageBoxW(hDlg, psz, NULL, MB_OK);
                return TRUE;
            }

            ImeGetRegisterWordStyle(nItem, lpStyleBuf);

            for (i = 0; i < nItem; i++) {
                dwIndex = SendDlgItemMessage(hDlg, ID_WR_STYLE, CB_ADDSTRING, 0,
                                             (LPARAM)lpStyleBuf->szDescription);
                SendDlgItemMessage(hDlg, ID_WR_STYLE, CB_SETITEMDATA, dwIndex,
                                   lpStyleBuf->dwStyle);
                lpStyleBuf++;
            }

            GlobalFree(lpStyleBuf);
        }
        break;

    case WM_DESTROY:
        break;

    case WM_HELP:
        break;

    case WM_CONTEXTMENU: // right mouse click
        break;

    case WM_COMMAND:
        break;

    default:
        return FALSE;
    }
    return TRUE;
} // RegWordDlgProc

INT_PTR CALLBACK
ChooseDictDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    NMHDR FAR *lpnm;
    //LPPROPSHEETPAGE lpPropSheet =
    //    (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (uMsg) {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch (lpnm->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_APPLY:
            break;

        case PSN_RESET:
            break;

        case PSN_HELP:
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        ::SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
        //lpPropSheet = (LPPROPSHEETPAGE)lParam;
        break;

    case WM_DESTROY:
        break;

    case WM_HELP:
        break;

    case WM_CONTEXTMENU: // right mouse click
        break;

    case WM_COMMAND:
        break;

    default:
        return FALSE;
    }
    return TRUE;
} // ChooseDictDlgProc

INT_PTR CALLBACK
AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    NMHDR FAR *lpnm;
    //LPPROPSHEETPAGE lpPropSheet =
    //    (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (uMsg) {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch (lpnm->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_APPLY:
            break;

        case PSN_RESET:
            break;

        case PSN_HELP:
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        ::SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
        //lpPropSheet = (LPPROPSHEETPAGE)lParam;
        break;

    case WM_DESTROY:
        break;

    case WM_HELP:
        break;

    case WM_CONTEXTMENU: // right mouse click
        break;

    case WM_COMMAND:
        break;

    default:
        return FALSE;
    }
    return TRUE;
} // AboutDlgProc

INT_PTR CALLBACK
GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    NMHDR FAR *lpnm;
    //LPPROPSHEETPAGE lpPropSheet =
    //    (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (uMsg) {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch (lpnm->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_APPLY:
            break;

        case PSN_RESET:
            break;

        case PSN_HELP:
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        ::SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
        //lpPropSheet = (LPPROPSHEETPAGE)lParam;
        break;

    case WM_DESTROY:
        break;

    case WM_HELP:
        break;

    case WM_CONTEXTMENU: // right mouse click
        break;

    case WM_COMMAND:
        break;

    default:
        return FALSE;
    }
    return TRUE;
} // GeneralDlgProc

INT_PTR CALLBACK
DebugOptionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //DWORD dwTemp;
    //TCHAR szBuf[128];
    NMHDR FAR *lpnm;
    //LPPROPSHEETPAGE lpPropSheet =
    //    (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (uMsg) {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch (lpnm->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_APPLY:
            break;

        case PSN_RESET:
            break;

        case PSN_HELP:
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        ::SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
        //lpPropSheet = (LPPROPSHEETPAGE)lParam;
        break;

    case WM_DESTROY:
        break;

    case WM_HELP:
        break;

    case WM_CONTEXTMENU: // right mouse click
        break;

    case WM_COMMAND:
        break;

    default:
        return FALSE;
    }
    return TRUE;
} // DebugOptionDlgProc

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
