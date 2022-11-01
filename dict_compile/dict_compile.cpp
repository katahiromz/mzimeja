// dict_compile.cpp --- dictionary compiler of MZ-IME Japanese Input
/////////////////////////////////////////////////////////////////////////////
// WARNING: This file is Shift_JIS encoding for historical reason. Be careful.

#include "mzimeja.h"

using namespace std;

static const wchar_t s_hiragana_table[][5] = {    
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_A
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_KA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_GA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_SA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_ZA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_TA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_DA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_NA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_HA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_BA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_PA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_MA
  {L'��', 0, L'��', 0, L'��'},           // GYOU_YA
  {L'��', L'��', L'��', L'��', L'��'},   // GYOU_RA
  {L'��', 0, 0, 0, L'��'},               // GYOU_WA
  {L'��', 0, 0, 0, 0},                   // GYOU_NN
};

unboost::unordered_map<wchar_t,wchar_t>   g_vowel_map;
unboost::unordered_map<wchar_t,wchar_t>   g_consonant_map;

void MakeLiteralMaps() {
  if (g_consonant_map.size()) {
    return;
  }
  g_consonant_map.clear();
  g_vowel_map.clear();
  const size_t count = _countof(s_hiragana_table);
  for (size_t i = 0; i < count; ++i) {
    for (size_t k = 0; k < 5; ++k) {
      g_consonant_map[s_hiragana_table[i][k]] = s_hiragana_table[i][0];
    }
    for (size_t k = 0; k < 5; ++k) {
      g_vowel_map[s_hiragana_table[i][k]] = s_hiragana_table[0][k];
    }
  }
} // MzIme::MakeLiteralMaps

inline bool entry_compare_pre(const DictEntry& e1, const DictEntry& e2) {
  return (e1.pre < e2.pre);
}

inline BOOL is_fullwidth_katakana(WCHAR ch) {
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
  return szBuf;
}

BOOL LoadDictDataFile(const wchar_t *fname, std::vector<DictEntry>& entries) {
  FILE *fp = _wfopen(fname, L"rb");
  if (fp == NULL) {
    return FALSE;
  }

  // load each line
  int lineno = 0;
  char buf[256];
  wchar_t wbuf[256];
  while (fgets(buf, 256, fp) != NULL) {
    ++lineno;
    if (buf[0] == ';') continue;  // comment

    // convert to UTF-16
    ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
    std::wstring str = wbuf;

    // split to fields
    unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
    WStrings fields;
    unboost::split(fields, str, unboost::is_any_of(L"\t"));

    // is it an invalid line?
    if (fields.empty() || fields[0].empty()) {
      assert(0);
      continue;
    }

    // parse a record
    DictEntry entry;
    entry.gyou = GYOU_A;
    if (fields.size() == 1) {
      // only one field, it's a noun (HB_MEISHI)
      entry.post = str;
      entry.bunrui = HB_MEISHI;
      if (is_fullwidth_katakana(str[0])) {
        // convert to hiragana
        std::wstring hiragana = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
        entry.pre = hiragana;
      } else {
        entry.pre = str;
      }
      entries.push_back(entry);
      continue;
    }

    // more than 2 fields
    // classify by the fields[1] string
    const std::wstring& bunrui_str = fields[1];
    if (bunrui_str.empty()) {
      entry.bunrui = HB_MEISHI;
    } else if (bunrui_str.size() == 2) {
      if (bunrui_str == L"����")            entry.bunrui = HB_MEISHI;
      else if (bunrui_str == L"����")       entry.bunrui = HB_FUKUSHI;
      else if (bunrui_str == L"����")       entry.bunrui = HB_KANGO;
      else continue;
    } else if (bunrui_str.size() == 3) {
      if (bunrui_str == L"�A�̎�")          entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"�ڑ���")     entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"������")     entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"�ړ���")     entry.bunrui = HB_SETTOUJI;
      else if (bunrui_str == L"�ڔ���")     entry.bunrui = HB_SETSUBIJI;
      else if (bunrui_str == L"�i����")     entry.bunrui = HB_KAKU_JOSHI;
      else if (bunrui_str == L"������")     entry.bunrui = HB_FUKU_JOSHI;
      else if (bunrui_str == L"�I����")     entry.bunrui = HB_SHUU_JOSHI;
      else if (bunrui_str == L"�J���}")     entry.bunrui = HB_COMMA;
      else if (bunrui_str == L"�L����")     entry.bunrui = HB_SYMBOL;
      else continue;
    } else if (bunrui_str.size() == 4) {
      if (bunrui_str == L"���`�e��")        entry.bunrui = HB_IKEIYOUSHI;
      else if (bunrui_str == L"�Ȍ`�e��")   entry.bunrui = HB_NAKEIYOUSHI;
      else if (bunrui_str == L"�ܒi����")   entry.bunrui = HB_GODAN_DOUSHI;
      else if (bunrui_str == L"��i����")   entry.bunrui = HB_ICHIDAN_DOUSHI;
      else if (bunrui_str == L"�J�ϓ���")   entry.bunrui = HB_KAHEN_DOUSHI;
      else if (bunrui_str == L"�T�ϓ���")   entry.bunrui = HB_SAHEN_DOUSHI;
      else if (bunrui_str == L"�ڑ�����")   entry.bunrui = HB_SETSUZOKU_JOSHI;
      else if (bunrui_str == L"�s���I�h")   entry.bunrui = HB_PERIOD;
      else continue;
    } else if (bunrui_str.size() == 5) {
      if (bunrui_str == L"���R������")      entry.bunrui = HB_MIZEN_JODOUSHI;
      else if (bunrui_str == L"�A�p������") entry.bunrui = HB_RENYOU_JODOUSHI;
      else if (bunrui_str == L"�I�~������") entry.bunrui = HB_SHUUSHI_JODOUSHI;
      else if (bunrui_str == L"�A�̏�����") entry.bunrui = HB_RENTAI_JODOUSHI;
      else if (bunrui_str == L"���菕����") entry.bunrui = HB_KATEI_JODOUSHI;
      else if (bunrui_str == L"���ߏ�����") entry.bunrui = HB_MEIREI_JODOUSHI;
      else continue;
    } else {
      continue;
    }

    // complete field[2] if lacked
    if (fields.size() == 2) {
      fields.push_back(fields[0]);
    } else if (fields[2].empty()) {
      fields[2] = fields[0];
    }

    // optimize
    std::wstring substr;
    wchar_t ch;
    size_t i, ngyou;
    switch (entry.bunrui) {
    case HB_NAKEIYOUSHI:
      i = fields[0].size() - 1;
      if (fields[0][i] == L'��') fields[0].resize(i);
      i = fields[2].size() - 1;
      if (fields[2][i] == L'��') fields[2].resize(i);
      break;
    case HB_IKEIYOUSHI:
      i = fields[0].size() - 1;
      if (fields[0][i] == L'��') fields[0].resize(i);
      i = fields[2].size() - 1;
      if (fields[2][i] == L'��') fields[2].resize(i);
      break;
    case HB_ICHIDAN_DOUSHI:
      assert(fields[0][fields[0].size() - 1] == L'��');
      assert(fields[2][fields[2].size() - 1] == L'��');
      fields[0].resize(fields[0].size() - 1);
      fields[2].resize(fields[2].size() - 1);
      break;
    case HB_KAHEN_DOUSHI:
      if (fields[0] == L"����") continue;
      substr = fields[0].substr(fields[0].size() - 2, 2);
      if (substr != L"����") continue;
      fields[0] = substr;
      substr = fields[2].substr(fields[0].size() - 2, 2);
      fields[2] = substr;
      break;
    case HB_SAHEN_DOUSHI:
      if (fields[0] == L"����") continue;
      substr = fields[0].substr(fields[0].size() - 2, 2);
      if (substr == L"����") entry.gyou = GYOU_SA;
      else if (substr != L"����") entry.gyou = GYOU_ZA;
      else continue;
      fields[0] = substr;
      fields[2] = fields[2].substr(fields[0].size() - 2, 2);
      break;
    case HB_GODAN_DOUSHI:
      ch = fields[0][fields[0].size() - 1];
      if (g_vowel_map[ch] != L'��') continue;
      fields[0].resize(fields[0].size() - 1);
      fields[2].resize(fields[2].size() - 1);
      ch = g_consonant_map[ch];
      for (i = 0; i < _countof(s_hiragana_table); ++i) {
        if (s_hiragana_table[i][0] == ch) {
          ngyou = i;
          break;
        }
      }
      entry.gyou = (Gyou)ngyou;
      break;
    default:
      break;
    }

    // store the parsed record to entries
    entry.pre = fields[0];
    entry.post = fields[2];
    if (fields.size() >= 4) {
      entry.tags = fields[3];
    } else {
      entry.tags.clear();
    }
    entries.push_back(entry);
  }

  // close the file
  fclose(fp);

  // sort by preconversion string
  std::sort(entries.begin(), entries.end(), entry_compare_pre);
  return TRUE;  // success
} // LoadDictDataFile

BOOL CreateDictFile(
  const wchar_t *fname,
  const std::vector<DictEntry>& entries)
{
  // calculate the total size
  size_t size = 0;
  size += 1;  // \n
  for (size_t i = 0; i < entries.size(); ++i) {
    const DictEntry& entry = entries[i];
    size += entry.pre.size();
    //size += 3;  // \t hb \t
    size += entry.post.size();
    //size += 1;  // \t
    size += entry.tags.size();
    //size += 1;  // \n
    size += 3 + 1 + 1;
  }
  size += 1;  // \0
  size *= sizeof(WCHAR);
  printf("size: %d\n", size);

  void *pv = malloc(size);
  if (pv == NULL) {
    return FALSE;
  }

  wchar_t *pch = (wchar_t *)pv;

  size_t cch;
  *pch++ += RECORD_SEP;
  for (size_t i = 0; i < entries.size(); ++i) {
    // line format:
    // pre FIELD_SEP MAKEWORD(bunrui, gyou) FIELD_SEP post FIELD_SEP tags RECORD_SEP
    const DictEntry& entry = entries[i];
    // pre \t
    cch = entry.pre.size();
    memcpy(pch, entry.pre.c_str(), cch * sizeof(WCHAR));
    pch += cch;
    *pch++ = FIELD_SEP;
    // MAKEWORD(bunrui, gyou) \t
    *pch++ = MAKEWORD(entry.bunrui, entry.gyou);
    *pch++ = FIELD_SEP;
    // post \t
    cch = entry.post.size();
    memcpy(pch, entry.post.c_str(), cch * sizeof(WCHAR));
    pch += cch;
    *pch++ = FIELD_SEP;
    // tags
    cch = entry.tags.size();
    memcpy(pch, entry.tags.c_str(), cch * sizeof(WCHAR));
    pch += cch;
    // new line
    *pch++ = RECORD_SEP;
  }
  *pch++ = L'\0'; // NUL
  assert(size / 2 == size_t(pch - reinterpret_cast<WCHAR *>(pv)));

  pch = (wchar_t *)pv;
  *pch = RECORD_SEP;

  BOOL ret = FALSE;

  HANDLE hFile = ::CreateFileW(fname, GENERIC_WRITE, FILE_SHARE_READ,
    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
    NULL);
  if (hFile != INVALID_HANDLE_VALUE) {
    DWORD dwWritten;
    ret = WriteFile(hFile, pv, size, &dwWritten, NULL);
    CloseHandle(hFile);
  }

  free(pv);
  return ret;
} // CreateDictFile

extern "C"
int wmain(int argc, wchar_t **wargv) {
  if (argc != 3) {
    printf("ERROR: missing parameters\n");
    return 1;
  }

  MakeLiteralMaps();

  std::vector<DictEntry> entries;
  if (!LoadDictDataFile(wargv[1], entries)) {
    printf("ERROR: cannot load\n");
    return 2;
  }

  if (!CreateDictFile(wargv[2], entries)) {
    printf("ERROR: cannot create\n");
    return 3;
  }

  printf("success.\n");

  return 0;
} // wmain
