// 郵便番号変換。
#include "mzimeja.h"

std::wstring postal_code(LPCWSTR code)
{
    DWORD dwTick1 = ::GetTickCount(); // 測定開始。

    // TODO: ここのコードをなるべく高速にせよ。
    std::wstring postal, ret;
    if (!Config_GetDWORD(L"PostalDictDisabled", FALSE)) {
        if (Config_GetSz(L"PostalDictPathName", postal)) {
            // 郵便番号データのファイルを開く。
            if (FILE *fin = _wfopen(postal.c_str(), L"rb")) {
                // 一行ずつ処理する。
                char buf[256];
                while (fgets(buf, _countof(buf), fin)) {
                    // 前後の空白を除去。
                    StrTrimA(buf, " \r\n");

                    // コメントの除去。
                    if (LPSTR pch = strchr(buf, ';')) {
                        *pch = 0;
                        StrTrimA(buf, " \r\n");
                    }

                    // UTF-8からUTF-16へ変換。
                    WCHAR szLine[256];
                    ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, szLine, _countof(szLine));
                    szLine[_countof(szLine) - 1] = 0; // Avoid buffer overflow

                    // タブ文字で分割。
                    std::wstring line = szLine;
                    std::vector<std::wstring> fields;
                    str_split(fields, line, std::wstring(L"\t"));

                    // フィールドが2個以下なら失敗。
                    if (fields.size() < 2)
                        continue;

                    // 第一フィールドがcodeでなければ失敗。
                    if (fields[0] != code)
                        continue;

                    // 発見。
                    ret = fields[1];
                    break;
                }

                // ファイルを閉じる。
                fclose(fin);
            }
        }
    }

    DWORD dwTick2 = ::GetTickCount(); // 測定終了。

    DPRINT("postal_code: %lu\n", dwTick2 - dwTick1);
    return ret;
}
