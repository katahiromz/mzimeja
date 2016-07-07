// katsuyou.cpp
// (Japanese, Shift_JIS)
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <map>
using namespace std;

#define UNBOOST_USE_STRING_ALGORITHM
#include "..\\unboost.hpp"

enum HINSHI_BUNRUI {
  HB_MEISHI,            // ����
  HB_IKEIYOUSHI,        // ���`�e��
  HB_NAKEIYOUSHI,       // �Ȍ`�e��
  HB_RENTAISHI,         // �A�̎�
  HB_FUKUSHI,           // ����
  HB_SETSUZOKUSHI,      // �ڑ���
  HB_KANDOUSHI,         // ������
  HB_JOSHI,             // ����
  HB_MIZEN_JODOUSHI,    // ���R������
  HB_RENYOU_JODOUSHI,   // �A�p������
  HB_SHUUSHI_JODOUSHI,  // �I�~������
  HB_RENTAI_JODOUSHI,   // �A�̏�����
  HB_KATEI_JODOUSHI,    // ���菕����
  HB_MEIREI_JODOUSHI,   // ���ߏ�����
  HB_GODAN_DOUSHI,      // �ܒi����
  HB_ICHIDAN_DOUSHI,    // ��i����
  HB_KAHEN_DOUSHI,      // �J�ϓ���
  HB_SAHEN_DOUSHI,      // �T�ϓ���
  HB_KANGO,             // ����
  HB_SETTOUGO,          // �ړ���
  HB_SETSUBIGO,         // �ڔ���
};

struct ENTRY {
  std::wstring  pre;
  HINSHI_BUNRUI bunrui;
  std::wstring  post;
  std::wstring  tags;
};

std::vector<ENTRY> g_entries;
std::map<wchar_t,wchar_t> g_dan_map;
std::map<wchar_t,wchar_t> g_gyou_map;

const wchar_t g_table[][5] = {
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', 0, L'��', 0, L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', 0, 0, 0, L'��'},
  {L'��', 0, 0, 0, 0},
};

BOOL is_hiragana(WCHAR ch) {
  if (0x3040 <= ch && ch <= 0x309F) return TRUE;
  switch (ch) {
  case 0x3095: case 0x3096: case 0x3099: case 0x309A: case 0x309B:
  case 0x309C: case 0x309D: case 0x309E: case 0x30FC:
    return TRUE;
  default:
    return FALSE;
  }
}

BOOL is_zenkaku_katakana(WCHAR ch) {
  if (0x30A0 <= ch && ch <= 0x30FF) return TRUE;
  switch (ch) {
  case 0x30FD: case 0x30FE: case 0x3099: case 0x309A: case 0x309B:
  case 0x309C: case 0x30FC:
    return TRUE;
  default:
    return FALSE;
  }
}

std::wstring lcmap(const std::wstring& str, DWORD dwFlags) {
  WCHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
    str.c_str(), -1, szBuf, 1024);
  return std::wstring(szBuf);
}

inline bool entry_compare(const ENTRY& e1, const ENTRY& e2) {
  return (e1.pre < e2.pre);
}

bool do_load(void) {
  char buf[256];
  wchar_t wbuf[256];
  std::wstring str;

  FILE *fp = fopen("..\\mzimeja.dic", "rb");
  if (fp == NULL) return false;

  int lineno = 0;
  while (fgets(buf, 256, fp) != NULL) {
    ++lineno;
    if (buf[0] == ';') continue;
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);

    std::wstring str = wbuf;
    unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
    std::vector<std::wstring> fields;
    unboost::split(fields, str, unboost::is_any_of(L"\t"));

    if (fields.empty()) {
      fprintf(stderr, "WARNING: empty line at Line %d\n", lineno);
      continue;
    }

    ENTRY entry;
    if (fields.size() == 1) {
      entry.post = str;
      entry.bunrui = HB_MEISHI;
      if (is_zenkaku_katakana(str[0])) {
        std::wstring hiragana = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
        entry.pre = hiragana;
      } else {
        entry.pre = str;
      }
    } else {
      const std::wstring& bunrui_str = fields[1];
      if (bunrui_str == L"����")              entry.bunrui = HB_MEISHI;
      else if (bunrui_str == L"���`�e��")     entry.bunrui = HB_IKEIYOUSHI;
      else if (bunrui_str == L"�Ȍ`�e��")     entry.bunrui = HB_NAKEIYOUSHI;
      else if (bunrui_str == L"�A�̎�")       entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"����")         entry.bunrui = HB_FUKUSHI;
      else if (bunrui_str == L"�ڑ���")       entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"������")       entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"����")         entry.bunrui = HB_JOSHI;
      else if (bunrui_str == L"���R������")   entry.bunrui = HB_MIZEN_JODOUSHI;
      else if (bunrui_str == L"�A�p������")   entry.bunrui = HB_RENYOU_JODOUSHI;
      else if (bunrui_str == L"�I�~������")   entry.bunrui = HB_SHUUSHI_JODOUSHI;
      else if (bunrui_str == L"�A�̏�����")   entry.bunrui = HB_RENTAI_JODOUSHI;
      else if (bunrui_str == L"���菕����")   entry.bunrui = HB_KATEI_JODOUSHI;
      else if (bunrui_str == L"���ߏ�����")   entry.bunrui = HB_MEIREI_JODOUSHI;
      else if (bunrui_str == L"�ܒi����")     entry.bunrui = HB_GODAN_DOUSHI;
      else if (bunrui_str == L"��i����")     entry.bunrui = HB_ICHIDAN_DOUSHI;
      else if (bunrui_str == L"�J�ϓ���")     entry.bunrui = HB_KAHEN_DOUSHI;
      else if (bunrui_str == L"�T�ϓ���")     entry.bunrui = HB_SAHEN_DOUSHI;
      else if (bunrui_str == L"����")         entry.bunrui = HB_KANGO;
      else if (bunrui_str == L"�ړ���")       entry.bunrui = HB_SETTOUGO;
      else if (bunrui_str == L"�ڔ���")       entry.bunrui = HB_SETSUBIGO;
      else {
        fprintf(stderr, "WARNING: invalid bunrui at Line %d\n", lineno);
        continue;
      }

      entry.pre = fields[0];
      if (fields.size() < 3 || fields[2].empty()) {
        entry.post = fields[0];
      } else {
        entry.post = fields[2];
      }

      if (fields.size() >= 4) {
        entry.tags = fields[3];
      }
    }
    g_entries.push_back(entry);
  }

  std::sort(g_entries.begin(), g_entries.end(), entry_compare);
  return true;
} // do_load

void do_wprintf(const wchar_t *format, ...) {
  wchar_t wbuf[512];
  char buf[512];
  va_list va;
  va_start(va, format);
  wvsprintfW(wbuf, format, va);
  WideCharToMultiByte(CP_ACP, 0, wbuf, -1, buf, 512, NULL, NULL);
  fputs(buf, stdout);
  va_end(va);
}

bool do_katsuyou_ikeiyoushi(const ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'��') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2;
  temp0 = str;
  temp0 += L"����";
  do_wprintf(L"���R�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"����";
  temp1 = str;
  temp1 += L"��";
  temp2 = str;
  temp2 += L"��";
  do_wprintf(L"�A�p�`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"����";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  do_wprintf(L"���ߌ`: (�Ȃ�)\n");

  temp0 = str;
  temp0 += L"��";
  temp1 = str;
  temp1 += L"��";
  temp2 = str;
  temp2 += L"��";
  do_wprintf(L"�����`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  return true;
}

bool do_katsuyou_nakeiyoushi(const ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'��') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp0 += L"����";
  do_wprintf(L"���R�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"����";
  temp1 = str;
  temp1 += L"��";
  temp2 = str;
  temp2 += L"��";
  do_wprintf(L"�A�p�`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"�Ȃ�";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  do_wprintf(L"���ߌ`: (�Ȃ�)\n");

  temp0 = str;
  temp1 = str;
  temp1 += L"��";
  do_wprintf(L"�����`: %s %s\n", temp0.c_str(), temp1.c_str());

  return true;
}

bool do_katsuyou_godan_doushi(const ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty()) return false;
  wchar_t ch = str[str.size() - 1];
  if (g_dan_map[ch] != L'��') return false;
  str.resize(str.size() - 1);

  wchar_t gyou = g_gyou_map[ch];
  size_t ngyou = 0;
  const size_t count = sizeof(g_table) / sizeof(g_table[0]);
  for (size_t i = 0; i < count; ++i) {
    if (g_table[i][0] == gyou) {
      ngyou = i;
      break;
    }
  }

  int type;
  switch (g_gyou_map[ch]) {
  case L'��': case L'��':              type = 1; break;
  case L'��': case L'��': case L'��':  type = 2; break;
  case L'��': case L'��': case L'��':  type = 3; break;
  default:                             type = 0; break;
  }

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp0 += g_table[ngyou][0];
  temp1 = str;
  temp1 += g_table[ngyou][4];
  do_wprintf(L"���R�`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  temp1 = str;
  switch (type) {
  case 1: temp1 += L'��'; break;
  case 2: temp1 += L'��'; break;
  case 3: temp1 += L'��'; break;
  default: temp1.clear(); break;
  }
  if (temp1.empty()) {
    do_wprintf(L"�A�p�`: %s\n", temp0.c_str());
  } else {
    do_wprintf(L"�A�p�`: %s %s\n", temp0.c_str(), temp1.c_str());
  }

  temp0 = str;
  temp0 += g_table[ngyou][2];
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][3];
  do_wprintf(L"����`: %s\n", temp0.c_str());
  do_wprintf(L"���ߌ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  do_wprintf(L"�����`: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_ichidan_doushi(const ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'��') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1;

  temp0 = str;
  do_wprintf(L"���R�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�p�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  temp1 = str;
  temp1 += L"��";
  do_wprintf(L"���ߌ`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  do_wprintf(L"�����`: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_kahen_doushi(const ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str.substr(str.size() - 2) != L"����") {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0;

  temp0 = str;
  do_wprintf(L"���R�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�p�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"���ߌ`: %s\n", temp0.c_str());

  do_wprintf(L"�����`: (�Ȃ�)\n");
  
  return true;
}

bool do_katsuyou_sahen_doushi(const ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.size() < 2) {
    return false;
  }
  bool flag = str.substr(str.size() - 2) == L"����";
  if (str.substr(str.size() - 2) != L"����" && !flag) {
    return false;
  }
  str.resize(str.size() - 2);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp1 = str;
  temp2 = str;
  if (flag) {
    temp0 += L"��";
    temp1 += L"��";
    temp2 += L"��";
  } else {
    temp0 += L"��";
    temp1 += L"��";
    temp2 += L"��";
  }
  do_wprintf(L"���R�`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"��";
  } else {
    temp0 += L"��";
  }
  do_wprintf(L"�A�p�`: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"����";
    temp1 += L"��";
  } else {
    temp0 += L"����";
    temp1 += L"��";
  }
  do_wprintf(L"�I�~�`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"����";
  } else {
    temp0 += L"����";
  }
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"����";
  } else {
    temp0 += L"����";
  }
  do_wprintf(L"����`: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"����";
    temp1 += L"����";
  } else {
    temp0 += L"����";
    temp1 += L"����";
  }
  do_wprintf(L"���ߌ`: %s %s\n", temp0.c_str(), temp1.c_str());

  do_wprintf(L"�����`: (�Ȃ�)\n");

  return true;
}

bool do_katsuyou(const wchar_t *data) {
  size_t count = g_entries.size();
  for (size_t i = 0; i < count; ++i) {
    const ENTRY& entry = g_entries[i];
    if (data[0] != entry.pre[0]) continue;
    if (entry.pre == data) {
      if (entry.tags.find(L"[��W��]") != std::wstring::npos) continue;
      switch (entry.bunrui) {
      case HB_MEISHI:
        do_wprintf(L"����: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return true;
      case HB_IKEIYOUSHI:
        do_wprintf(L"���`�e��: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return do_katsuyou_ikeiyoushi(entry);
      case HB_NAKEIYOUSHI:
        do_wprintf(L"�Ȍ`�e��: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return do_katsuyou_nakeiyoushi(entry);
      case HB_RENTAISHI:
        do_wprintf(L"�A�̎�: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return true;
      case HB_GODAN_DOUSHI:
        do_wprintf(L"�ܒi����: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return do_katsuyou_godan_doushi(entry);
      case HB_ICHIDAN_DOUSHI:
        do_wprintf(L"��i����: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return do_katsuyou_ichidan_doushi(entry);
      case HB_KAHEN_DOUSHI:
        do_wprintf(L"�J�ϓ���: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return do_katsuyou_kahen_doushi(entry);
      case HB_SAHEN_DOUSHI:
        do_wprintf(L"�T�ϓ���: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        return do_katsuyou_sahen_doushi(entry);
      default:
        break;
      }
    }
  }
  return false;
}

void make_maps(void) {
  const size_t count = sizeof(g_table) / sizeof(g_table[0]);
  for (size_t i = 0; i < count; ++i) {
    for (size_t k = 0; k < 5; ++k) {
      g_gyou_map[g_table[i][k]] = g_table[i][0];
    }
    for (size_t k = 0; k < 5; ++k) {
      g_dan_map[g_table[i][k]] = g_table[0][k];
    }
  }
} // make_maps

int main(int argc, char **argv) {
  make_maps();

  if (!do_load()) {
    fprintf(stderr, "ERROR: cannot load dictionary\n");
    return 1;
  }

  wchar_t data[256];
  if (argc >= 2) {
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, data, 256);
    do_katsuyou(data);
  }
  return 0;
}
