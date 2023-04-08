// 郵便番号変換。
#include "mzimeja.h"

// 郵便番号変換を行う関数。
std::wstring postal_code(LPCWSTR code)
{
    std::wstring postal, ret;
    if (Config_GetDWORD(L"PostalDictDisabled", FALSE)) // 無効化されている？
        return ret;

    if (!Config_GetSz(L"PostalDictPathName", postal)) // 郵便番号データのパス名を取得できない？
        return ret;

    DWORD dwTick1 = ::GetTickCount(); // 測定開始。

    CHAR szCodeA[16];
    StringCchPrintfA(szCodeA, _countof(szCodeA), "%07u", code);

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
    DPRINT("postal_code: %lu\n", dwTick2 - dwTick1);

    return ret;
}
