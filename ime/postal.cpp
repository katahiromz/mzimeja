// 郵便番号変換。
#include "mzimeja.h"

// 郵便番号を正規化する。
// 与えられた文字列が郵便番号ではない場合は空文字列を返す。
std::wstring normalize_postal_code(const std::wstring& str)
{
    // 半角に変換。
    std::wstring ret = lcmap(str, LCMAP_HALFWIDTH);

    // 郵便番号中のハイフンを取り除く。
    if (ret.size() >= 5 && is_hyphen(ret[3]))
        ret.erase(3, 1);

    // 全部数字でなければ失敗。
    if (!are_all_chars_numeric(ret))
        return L"";

    // 三桁や五桁の場合は七桁の省略形と見なす。
    if (ret.size() == 3)
        ret += L"00";
    if (ret.size() == 5)
        ret += L"00";

    // 最終的に七桁でなければ失敗。
    if (ret.size() != 7)
        return L"";

    return ret;
}

// 郵便番号変換を行う関数。
std::wstring convert_postal_code(LPCWSTR code)
{
    if (lstrlenW(code) != 7)
        return L""; // 正規化されていなければ失敗。

    std::wstring postal, ret;
    if (Config_GetDWORD(L"PostalDictDisabled", FALSE)) // 無効化されている？
        return ret;

    if (!Config_GetSz(L"PostalDictPathName", postal)) // 郵便番号データのパス名を取得できない？
        return ret;

    DWORD dwTick1 = ::GetTickCount(); // 測定開始。

    CHAR szCodeA[16];
    WideCharToMultiByte(CP_ACP, 0, code, -1, szCodeA, _countof(szCodeA), NULL, NULL);
    szCodeA[_countof(szCodeA) - 1] = 0; // Avoid buffer overflow

    // 郵便番号データのファイルを開く。
    if (FILE *fin = _wfopen(postal.c_str(), L"rb")) {
        // 一行ずつ処理する。
        char szLine[MAX_PATH];
        while (fgets(szLine, _countof(szLine), fin)) {
            // タブ文字以外の前後の空白を除去。
            StrTrimA(szLine, " \r\n");

            // コメントの除去。
            if (LPSTR pch1 = strchr(szLine, ';')) {
                *pch1 = 0;
                StrTrimA(szLine, " \r\n");
            }

            // タブ文字で分割。
            LPSTR pch2 = strchr(szLine, '\t');
            if (!pch2)
                continue; // タブがなければ次の行。
            *pch2++ = 0;

            // 各フィールドの前後の空白を除去。
            StrTrimA(szLine, " \t\r\n");
            StrTrimA(pch2, " \t\r\n");

            // 一致しなければ次の行。
            if (lstrcmpA(szCodeA, szLine) != 0 || *pch2 == 0)
                continue;

            // UTF-8からUTF-16へ変換。
            WCHAR szValue[MAX_PATH];
            ::MultiByteToWideChar(CP_UTF8, 0, pch2, -1, szValue, _countof(szValue));
            szValue[_countof(szValue) - 1] = 0; // Avoid buffer overflow

            // 結果をセット。
            ret = szValue;
            break;
        }

        // ファイルを閉じる。
        fclose(fin);
    }

    DWORD dwTick2 = ::GetTickCount(); // 測定終了。

    // 測定値をデバッグ出力。
    DPRINT("convert_postal_code: %lu\n", dwTick2 - dwTick1);

    return ret;
}
