// 郵便番号変換。
#include "mzimeja.h"

std::wstring postal_code(LPCWSTR code)
{
    std::wstring postal, ret;
    DWORD dw = 0;
    if (!Config_GetDWORD(L"PostalDictDisabled", FALSE)) {
        if (Config_GetSz(L"PostalDictPathName", postal)) {
            if (FILE *fin = _wfopen(postal.c_str(), L"rb")) {
                char buf[256];
                while (fgets(buf, _countof(buf), fin)) {
                    StrTrimA(buf, " \r\n");
                    if (LPSTR pch = strchr(buf, ';')) {
                        *pch = 0;
                        StrTrimA(buf, " \r\n");
                    }

                    WCHAR szLine[256];
                    ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, szLine, _countof(szLine));
                    szLine[_countof(szLine) - 1] = 0;

                    std::wstring line = szLine;

                    std::vector<std::wstring> fields;
                    str_split(fields, line, std::wstring(L"\t"));
                    if (fields.size() < 2)
                        continue;

                    if (fields[0] != code)
                        continue;

                    ret = fields[1];
                    break;
                }

                fclose(fin);
            }
        }
    }

    return ret;
}
