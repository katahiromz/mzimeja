// lcmap.cpp
// (Japanese, UTF-8)
#include <windows.h>
#include <string>
#include <iostream>

std::string lcmap(const std::string& str, DWORD dwFlags)
{
    CHAR szBuf[1024];
    const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
    ::LCMapStringA(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                   str.c_str(), -1, szBuf, 1024);
    return std::string(szBuf);
}

// The result is:
// 
//   漢字あいうあいうｱｲｳＡＢＣABCabc。
//   漢字アイウアイウｱｲｳＡＢＣABCabc。
//   漢字あいうあいうあいうＡＢＣＡＢＣａｂｃ。
//   漢字アイウアイウアイウＡＢＣＡＢＣａｂｃ。
//   漢字あいうアイウアイウＡＢＣＡＢＣａｂｃ。
//   漢字ｱｲｳｱｲｳｱｲｳABCABCabc｡
//   漢字あいうｱｲｳｱｲｳABCABCabc｡

int main(void)
{
    std::string str, strMapped;
    DWORD dwFlags;
    CHAR buf[MAX_PATH];
    WideCharToMultiByte(932, L"漢字あいうアイウｱｲｳＡＢＣABCabc。", -1,
                        buf, _countof(buf), NULL, NULL);
    str = buf;

    dwFlags = LCMAP_HIRAGANA;
    strMapped = lcmap(str, dwFlags);
    std::cout << strMapped << std::endl;

    dwFlags = LCMAP_KATAKANA;
    strMapped = lcmap(str, dwFlags);
    std::cout << strMapped << std::endl;

    dwFlags = LCMAP_FULLWIDTH | LCMAP_HIRAGANA;
    strMapped = lcmap(str, dwFlags);
    std::cout << strMapped << std::endl;

    dwFlags = LCMAP_FULLWIDTH | LCMAP_KATAKANA;
    strMapped = lcmap(str, dwFlags);
    std::cout << strMapped << std::endl;

    dwFlags = LCMAP_FULLWIDTH;
    strMapped = lcmap(str, dwFlags);
    std::cout << strMapped << std::endl;

    dwFlags = LCMAP_HALFWIDTH | LCMAP_KATAKANA;
    strMapped = lcmap(str, dwFlags);
    std::cout << strMapped << std::endl;

    dwFlags = LCMAP_HALFWIDTH;
    strMapped = lcmap(str, dwFlags);
    std::cout << strMapped << std::endl;

    return 0;
}
