// regword.cpp --- registering words
// 辞書登録。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

//  ImeRegisterWord
//  ImeRegisterWord 関数は現在のIMEの辞書に文字列を登録します。
//  BOOL WINAPI
//    ImeRegisterWord(
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString
//  )
//  (パラメータ)
//    lpszReading
//      登録される文字列の読み。
//    dwStyle
//      登録される文字列のスタイル。次のような値が用意されています。
//
//      IME_REGWORD_STYLE_EUDC
//        文字列は EUDC の範囲にある。
//      IME_REGWORD_STYLE_USER_FIRST から IME_REGWORD_STYLE_USER_LAST
//        IME_REGWORD_STYLE_USER_FIRST から
//        IME_REGWORD_STYLE_USER_LAST の範囲は IME ISV の private
//        style に使われる。IME ISV は自由に独自のスタイルを定義して
//        よい。
//        例えば、
//          #define MSIME_NOUN  (IME_REGWORD_STYLE_USER_FIRST)
//          #define MSIME_VERB  (IME_REGWORD_STYLE_USER_FIRST+1)
//    lpszString
//      登録される文字列。
//  (返り値)
//    成功したならば、TRUE を。さもなくば FALSE を返す。
BOOL WINAPI ImeRegisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
    TCHAR szRead[MAX_PATH];
    TCHAR szStr[MAX_PATH];
    FOOTMARK();

    if (!lpRead || !lpStr) {
        DPRINT("!lpRead || !lpStr\n");
        return FALSE;
    }
    if ((dw & MZIME_REGWORD_STYLE) != MZIME_REGWORD_STYLE) {
        DPRINT("dw:0x%08lX\n", dw);
        return FALSE;
    }

    StringCchCopy(szStr, _countof(szStr), lpStr);
    LCMapString(0, LCMAP_HIRAGANA | LCMAP_FULLWIDTH, lpRead, -1, szRead, _countof(szRead));
    StrTrimW(szRead, L" \t\r\n\x3000");
    StrTrimW(szStr, L" \t\r\n\x3000");
    if (!szRead[0] || !szStr[0]) {
        DPRINT("'%ls', '%ls'\n", szRead, szStr);
        return FALSE;
    }

    HinshiBunrui hinshi = StyleToHinshi(dw);
    if (!(HB_MEISHI <= hinshi && hinshi <= HB_MAX)) {
        DPRINT("%d\n", hinshi);
        return FALSE;
    }

    LPCTSTR pszHinshi = HinshiToString(hinshi);
    if (!pszHinshi || !pszHinshi[0]) {
        DPRINT("%d\n", hinshi);
        return FALSE;
    }

    // アプリキーを作成する。
    HKEY hAppKey = Config_CreateAppKey();
    if (!hAppKey) {
        return FALSE;
    }

    // ユーザー辞書キーを作成する。
    HKEY hUserDict;
    LONG error = ::RegCreateKeyEx(hAppKey, TEXT("UserDict"), 0, NULL, 0, 
                                  KEY_READ | KEY_WRITE, NULL, &hUserDict, NULL);
    if (error) {
        DPRINT("error: 0x%08lX\n", error);
        ::RegCloseKey(hAppKey);
        return FALSE;
    }

    // 値名文字列は、"読み:単語:品詞"の形。
    TCHAR szName[MAX_PATH];
    StringCchCopy(szName, _countof(szName), szRead);
    StringCchCat(szName, _countof(szName), TEXT(":"));
    StringCchCat(szName, _countof(szName), szStr);
    StringCchCat(szName, _countof(szName), TEXT(":"));
    StringCchCat(szName, _countof(szName), pszHinshi);

    // 値文字列は、現状では空文字列とする。
    TCHAR szValue[MAX_PATH];
    StringCchCopy(szValue, _countof(szValue), TEXT(""));
    INT cchValue = lstrlen(szValue);

    // レジストリに値をセット。
    error = ::RegSetValueEx(hUserDict, szName, 0, REG_SZ, (LPBYTE)szValue, (cchValue + 1) * sizeof(TCHAR));
    if (error) {
        DPRINT("error: 0x%08lX\n", error);
    }
    BOOL ret = (error == ERROR_SUCCESS);

    // レジストリキーを閉じる。
    ::RegCloseKey(hUserDict);
    ::RegCloseKey(hAppKey);

    return ret;
}

//  ImeUnregisterWord
//  ImeUnregisterWord は現在のIMEの辞書から登録されている文字列を削除しま
//  す。
//  BOOL WINAPI
//    ImeUnregisterWord(
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString
//  )
//  (パラメータ)
//    lpszReading
//      登録されている文字列の読み。
//    dwStyle
//      登録されている文字列のスタイル。dwStyle の中身については
//      ImeRegisterWord の説明を参照のこと。
//    lpszString
//      削除される文字列。
//  (返り値)
//    成功したならば、TRUE を。さもなくば FALSE を返す。
BOOL WINAPI ImeUnregisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
    TCHAR szRead[MAX_PATH];
    TCHAR szStr[MAX_PATH];

    FOOTMARK();

    if (!lpRead || !lpStr) {
        DPRINT("!lpRead || !lpStr\n");
        return FALSE;
    }

    if ((dw & MZIME_REGWORD_STYLE) != MZIME_REGWORD_STYLE) {
        DPRINT("%08lX\n", dw);
        return FALSE;
    }

    StringCchCopy(szRead, _countof(szRead), lpRead);
    StringCchCopy(szStr, _countof(szStr), lpStr);
    StrTrimW(szRead, L" \t\r\n\x3000");
    StrTrimW(szStr, L" \t\r\n\x3000");
    if (!szRead[0] || !szStr[0]) {
        DPRINT("'%ls', '%ls'\n", szRead, szStr);
        return FALSE;
    }

    HinshiBunrui hinshi = StyleToHinshi(dw);
    if (!(HB_MEISHI <= hinshi && hinshi <= HB_MAX)) {
        DPRINT("%d\n", hinshi);
        return FALSE;
    }
    LPCTSTR pszHinshi = HinshiToString(hinshi);
    if (!pszHinshi || !pszHinshi[0]) {
        DPRINT("%d\n", hinshi);
        return FALSE;
    }

    // レジストリのアプリキーを作成する。
    HKEY hAppKey = Config_CreateAppKey();
    if (!hAppKey)
        return FALSE;

    // ユーザー辞書キーを開く。
    HKEY hUserDict;
    LONG error = ::RegOpenKeyEx(hAppKey, TEXT("UserDict"), 0, KEY_READ | KEY_WRITE, &hUserDict);
    if (error) {
        DPRINT("error: 0x%08lX\n", error);
        ::RegCloseKey(hAppKey);
        return FALSE;
    }

    // 値名文字列は、"読み:単語:品詞"の形。
    TCHAR szName[MAX_PATH];
    StringCchCopy(szName, _countof(szName), szRead);
    StringCchCat(szName, _countof(szName), TEXT(":"));
    StringCchCat(szName, _countof(szName), szStr);
    StringCchCat(szName, _countof(szName), TEXT(":"));
    StringCchCat(szName, _countof(szName), pszHinshi);

    // レジストリの値を削除。
    BOOL ret = (::RegDeleteValue(hUserDict, szName) == ERROR_SUCCESS);
    if (!ret) {
        DPRINT("%ls\n", szName);
    }

    // レジストリキーを閉じる。
    ::RegCloseKey(hAppKey);
    ::RegCloseKey(hUserDict);

    return ret;
}

//  ImeGetRegisterWordStyle
//  ImeGetRegisterWordStyle は現在のIMEで利用可能なスタイルを得ます。
//  UINT WINAPI
//    ImeGetRegisterWordStyle(
//    UINT nItem,
//    LPSTYLEBUF lpStyleBuf
//  )
//  (パラメータ)
//    nItem
//      バッファに保存可能なスタイルの最大数。
//    lpStyleBuf
//      満たされるべきバッファ。
//  (返り値)
//    バッファにコピーされたスタイルの数が返り値です。もし、nItems が零
//    ならば、このIMEですべての可能なスタイルを受け取るのに必要な配列の
//    要素数になります。
UINT WINAPI ImeGetRegisterWordStyle(UINT u, LPSTYLEBUF lp)
{
    FOOTMARK();
    UINT uMax = (HB_MAX - HB_MEISHI) + 1;
    if (u == 0)
        return uMax;
    if (u > uMax)
        u = uMax;
    UINT i;
    for (i = 0; i < uMax; ++i) {
        HinshiBunrui hinshi = (HinshiBunrui)(i + HB_MEISHI);
        if (i >= u)
            break;
        lp[i].dwStyle = HinshiToStyle(hinshi);
        StringCchCopy(lp[i].szDescription, _countof(lp[i].szDescription), HinshiToString(hinshi));
    }
    return i;
}

//  ImeEnumRegisterWord
//  ImeEnumRegisterWord は特定された読み、スタイル、登録された文字列データ
//  でもって登録されている文字列の情報を列挙します。
//  UINT WINAPI
//    ImeEnumRegisterWord(
//    hKL,
//    REGISTERWORDENUMPROC lpfnEnumProc,
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString,
//    LPVOID lpData
//  )
//  (パラメータ)
//    hKL
//      Input language handle.
//    lpfnEnumProc
//      callback 関数のアドレス。
//    lpszReading
//      列挙される読みを特定します。NULLならば、dwStyle 及び lpszString
//      パラメータに一致する全ての可能な読みを列挙します。
//    dwStyle
//      列挙されるスタイルを特定します。NULLならば、lpszReading 及び
//      lpszString パラメータに一致する全ての可能なスタイルを列挙しま
//      す。
//    lpszString
//      列挙される文字列を特定します。NULLならば、ImeEnumRegisterWord
//      は lpszReading 及び dwStyle パラメータが一致する全ての文字列を
//      列挙します。
//    lpData
//      アプリケーション側が用意したデータのアドレス
//  (返り値)
//    成功ならば、callback function に最後に返された値が返り値になります。
//    アプリケーションによってその意味は決められます。
//  (コメント)
//    全てのパラメータが NULL だったら、IME 辞書の全ての登録されている
//    文字列を列挙します。（うげげ）
//    もじ入力パラメータの２つがNULLだった場合には、残った３つ目のパラメー
//    タに適合する登録されている文字列を全て列挙します。
//  (個人的なコメント)
//    辞書の構造をかなりうまく決めていないと、こんなことをされたら重く
//    てやってられなくなります。正引きは勿論逆引きも可能である、という
//    のは最低条件みたいです。
//    SKK辞書との相性は最悪では？
UINT WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROC lpfn, LPCTSTR lpRead,
                                DWORD dw, LPCTSTR lpStr, LPVOID lpData)
{
    UINT ret;
    FOOTMARK();

    if (!lpfn || (dw && (dw & MZIME_REGWORD_STYLE) != MZIME_REGWORD_STYLE)) {
        DPRINT("%p, %08lX\n", lpfn, dw);
        return 0;
    }

    HinshiBunrui hinshi;
    std::wstring strHinshi;
    if (dw) {
        if ((dw & MZIME_REGWORD_STYLE) != MZIME_REGWORD_STYLE) {
            DPRINT("%08lX\n", dw);
            return 0;
        }
        hinshi = StyleToHinshi(dw);
        if (!(HB_MEISHI <= hinshi && hinshi <= HB_MAX)) {
            DPRINT("%d\n", hinshi);
            return FALSE;
        }
        strHinshi = HinshiToString(hinshi);
    }

    // レジストリキーを開く。
    HKEY hUserDict;
    LONG error = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                                TEXT("SOFTWARE\\Katayama Hirofumi MZ\\mzimeja\\UserDict"),
                                0, KEY_READ, &hUserDict);
    if (error) {
        DPRINT("error: 0x%08lX\n", error);
        return 0;
    }

    // 値を列挙する。
    for (DWORD dwIndex = 0; dwIndex < 0x1000; ++dwIndex) {
        // 値の名前を取得する。
        TCHAR szValueName[MAX_PATH];
        DWORD cchValueName = _countof(szValueName);
        TCHAR szValue[MAX_PATH];
        DWORD cbValue = sizeof(szValue);
        DWORD dwType;
        error = ::RegEnumValue(hUserDict, dwIndex, szValueName, &cchValueName, NULL,
                               &dwType, (LPBYTE)szValue, &cbValue);
        if (error) {
            if (error != ERROR_NO_MORE_ITEMS) {
                DPRINT("error: 0x%08lX\n", error);
            }
            break;
        }
        if (dwType != REG_SZ)
            continue;
        szValueName[_countof(szValueName) - 1] = 0; // avoid buffer overrun
        szValue[_countof(szValue) - 1] = 0; // avoid buffer overrun

        // コロンで値の文字列を分割する。
        LPTSTR pch1 = wcschr(szValueName, L':');
        if (pch1 == NULL)
            continue;
        *pch1++ = 0;
        LPTSTR pch2 = wcschr(pch1, L':');
        if (pch2 == NULL)
            continue;
        *pch2++ = 0;

        if (lpRead && lpRead[0] && lstrcmpi(szValueName, lpRead) != 0)
            continue;
        if (lpStr && lpStr[0] && lstrcmpi(pch1, lpStr) != 0)
            continue;
        if (dw && strHinshi[0] && lstrcmpi(pch2, strHinshi.c_str()) != 0)
            continue;

        HinshiBunrui hinshi = StringToHinshi(pch2);
        if (hinshi == HB_UNKNOWN)
            continue;
        DWORD dwStyle = HinshiToStyle(hinshi);
        ret = lpfn(szValueName, dwStyle, pch1, lpData);
    }

    // レジストリキーを閉じる。
    ::RegCloseKey(hUserDict);

    return ret;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
