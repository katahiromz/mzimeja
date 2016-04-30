// lcmap.cpp
// (Japanese, Shift_JIS)
#include <windows.h>
#include <string>
#include <iostream>

std::string lcmap(const std::string& str, DWORD dwFlags) {
  CHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  ::LCMapStringA(MAKELCID(langid, SORT_DEFAULT), dwFlags,
    str.c_str(), -1, szBuf, 1024);
  return std::string(szBuf);
}

// The result is:
// 
//   ��������������������`�a�bABCabc�B
//   �����A�C�E�A�C�E����`�a�bABCabc�B
//   �����������������������`�a�b�`�a�b�������B
//   �����A�C�E�A�C�E�A�C�E�`�a�b�`�a�b�������B
//   �����������A�C�E�A�C�E�`�a�b�`�a�b�������B
//   �������������ABCABCabc�
//   ����������������ABCABCabc�

int main(void) {
  std::string str, strMapped;
  DWORD dwFlags;

  str = "�����������A�C�E����`�a�bABCabc�B";

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
