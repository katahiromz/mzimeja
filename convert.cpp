// convert.cpp --- mzimeja kana kanji conversion
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#include "mzimeja.h"

static const wchar_t s_hiragana_table[][5] = {
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

void MZIMEJA::MakeLiteralMaps() {
  if (m_consonant_map.size()) {
    return;
  }
  m_consonant_map.clear();
  m_vowel_map.clear();
  const size_t count = sizeof(s_hiragana_table) / sizeof(s_hiragana_table[0]);
  for (size_t i = 0; i < count; ++i) {
    for (size_t k = 0; k < 5; ++k) {
      m_consonant_map[s_hiragana_table[i][k]] = s_hiragana_table[i][0];
    }
    for (size_t k = 0; k < 5; ++k) {
      m_vowel_map[s_hiragana_table[i][k]] = s_hiragana_table[0][k];
    }
  }
}

inline bool entry_compare(const DICT_ENTRY& e1, const DICT_ENTRY& e2) {
  return (e1.pre < e2.pre);
}

BOOL MZIMEJA::LoadBasicDictFile(std::vector<DICT_ENTRY>& entries) {
  char buf[256];
  wchar_t wbuf[256];
  std::wstring str;

  entries.clear();
  entries.reserve(60000);

  std::wstring filename = GetComputerString(L"basic dictionary file");
  FILE *fp = fopen("..\\mzimeja.dic", "rb");
  if (fp == NULL) return false;

  int lineno = 0;
  while (fgets(buf, 256, fp) != NULL) {
    ++lineno;
    if (buf[0] == ';') continue;
    ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);

    std::wstring str = wbuf;

    unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
    std::vector<std::wstring> fields;
    unboost::split(fields, str, unboost::is_any_of(L"\t"));

    if (fields.empty()) {
      continue;
    }

    DICT_ENTRY entry;
    if (fields.size() == 1) {
      entry.post = str;
      entry.bunrui = HB_MEISHI;
      if (is_fullwidth_katakana(str[0])) {
        std::wstring hiragana = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
        entry.pre = hiragana;
      } else {
        entry.pre = str;
      }
    } else {
      const std::wstring& bunrui_str = fields[1];
      if (bunrui_str.empty()) {
        entry.bunrui = HB_MEISHI;
      } else if (bunrui_str.size() == 2) {
        if (bunrui_str == L"����")            entry.bunrui = HB_MEISHI;
        else if (bunrui_str == L"����")       entry.bunrui = HB_FUKUSHI;
        else if (bunrui_str == L"����")       entry.bunrui = HB_JOSHI;
        else if (bunrui_str == L"����")       entry.bunrui = HB_KANGO;
        else continue;
      } else if (bunrui_str.size() == 3) {
        if (bunrui_str == L"�A�̎�")          entry.bunrui = HB_RENTAISHI;
        else if (bunrui_str == L"�ڑ���")     entry.bunrui = HB_SETSUZOKUSHI;
        else if (bunrui_str == L"������")     entry.bunrui = HB_KANDOUSHI;
        else if (bunrui_str == L"�ړ���")     entry.bunrui = HB_SETTOUGO;
        else if (bunrui_str == L"�ڔ���")     entry.bunrui = HB_SETSUBIGO;
        else continue;
      } else if (bunrui_str.size() == 4) {
        if (bunrui_str == L"���`�e��")        entry.bunrui = HB_IKEIYOUSHI;
        else if (bunrui_str == L"�Ȍ`�e��")   entry.bunrui = HB_NAKEIYOUSHI;
        else if (bunrui_str == L"�ܒi����")   entry.bunrui = HB_GODAN_DOUSHI;
        else if (bunrui_str == L"��i����")   entry.bunrui = HB_ICHIDAN_DOUSHI;
        else if (bunrui_str == L"�J�ϓ���")   entry.bunrui = HB_KAHEN_DOUSHI;
        else if (bunrui_str == L"�T�ϓ���")   entry.bunrui = HB_SAHEN_DOUSHI;
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
    entries.push_back(entry);
  }

  std::sort(entries.begin(), entries.end(), entry_compare);
  return TRUE;
} // MZIMEJA::LoadBasicDictFile

BOOL MZIMEJA::DeployDictData(
  ImeBaseData *data, SECURITY_ATTRIBUTES *psa,
  const std::vector<DICT_ENTRY>& entries)
{
  size_t size = 0;
  size += 1;  // \n
  for (size_t i = 0; i < entries.size(); ++i) {
    const DICT_ENTRY& entry = entries[i];
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

  BOOL ret = FALSE;
  if (::WaitForSingleObject(m_hMutex, 5000) == WAIT_OBJECT_0) {
    // create shared dict data
    m_hBasicDictData = ::CreateFileMappingW(INVALID_HANDLE_VALUE, psa,
      PAGE_READWRITE, 0, size, L"mzimeja_basic_dict");
    LPVOID pv = ::MapViewOfFile(
      m_hBasicDictData, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (pv) {
      size_t cch;
      WCHAR *pch = reinterpret_cast<WCHAR *>(pv);
      *pch += L'\n';  // new line
      for (size_t i = 0; i < entries.size(); ++i) {
        // line format: pre \t hb \t post \t tags \n
        const DICT_ENTRY& entry = entries[i];
        // pre \t
        cch = entry.pre.size();
        memcpy(pch, entry.pre.c_str(), cch * sizeof(WCHAR));
        pch += cch;
        *pch++ = L'\t';
        // hb \t
        *pch++ = WCHAR(entry.bunrui);
        *pch++ = L'\t';
        // post \t
        cch = entry.post.size();
        memcpy(pch, entry.post.c_str(), cch * sizeof(WCHAR));
        pch += cch;
        *pch++ = L'\t';
        // tags
        cch = entry.tags.size();
        memcpy(pch, entry.tags.c_str(), cch * sizeof(WCHAR));
        pch += cch;
        // new line
        *pch++ = L'\n';
      }
      *pch++ = L'\0'; // NUL
      assert(size == size_t(pch - reinterpret_cast<WCHAR *>(pv)));
      ::UnmapViewOfFile(pv);
      ret = TRUE; // success
    }
    data->dwSharedDictDataSize = size;
    ::ReleaseMutex(m_hMutex);
  }
  return ret;
} // MZIMEJA::DeployDictData

//////////////////////////////////////////////////////////////////////////////

BOOL MZIMEJA::LoadBasicDict() {
  BOOL ret = FALSE;
  ImeBaseData *data = LockImeBaseData();
  if (data) {
    if (data->dwSignature == 0xDEADFACE) {
      SECURITY_ATTRIBUTES *psa = CreateSecurityAttributes();
      assert(psa);

      if (data->dwSharedDictDataSize != 0) {
        // get shared data
        if (WaitForSingleObject(m_hMutex, 5000) == WAIT_OBJECT_0) {
          m_hBasicDictData = ::CreateFileMappingW(INVALID_HANDLE_VALUE, psa,
            PAGE_READWRITE, 0, data->dwSharedDictDataSize, L"mzimeja_basic_dict");
          if (m_hBasicDictData) {
            ret = TRUE;
          }
          ::ReleaseMutex(m_hMutex);
        }
      } else {
        // create shared data
        std::vector<DICT_ENTRY> entries;
        if (LoadBasicDictFile(entries)) {
          ret = DeployDictData(data, psa, entries);
        }
      }
      FreeSecurityAttributes(psa);
    }
    UnlockImeBaseData(data);
  }

  return ret;
} // MZIMEJA::LoadBasicDict

BOOL MZIMEJA::IsBasicDictLoaded() const {
  return m_hBasicDictData != NULL;
}

WCHAR *MZIMEJA::LockBasicDict() {
  // get size
  DWORD dwSize = 0;
  ImeBaseData *data = LockImeBaseData();
  if (data) {
    dwSize = data->dwSharedDictDataSize;
    UnlockImeBaseData(data);
  }

  // map the view
  if (dwSize) {
    LPVOID pv;
    pv = ::MapViewOfFile(m_hBasicDictData, FILE_MAP_READ, 0, 0, dwSize);
    return reinterpret_cast<WCHAR *>(pv);
  }
  return NULL;
}

void MZIMEJA::UnlockBasicDict(WCHAR *data) {
  ::UnmapViewOfFile(data);
}

void MZIMEJA::PluralClauseConversion(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  MzConversionResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[comp.extra.iClause];
  PluralClauseConversion(strHiragana, result);

  // TODO:
  comp.comp_str.clear();
  comp.extra.clear();
  comp.comp_clause.resize(result.clauses.size() + 1);
  for (size_t k = 0; k < result.clauses.size(); ++k) {
    MzConversionClause& clause = result.clauses[k];
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause.candidates[i];
      comp.comp_clause[k] = (DWORD)comp.comp_str.size();
      comp.extra.hiragana_clauses.push_back(cand.hiragana);
      std::wstring typing;
      if (bRoman) {
        typing = hiragana_to_typing(cand.hiragana);
      } else {
        typing = hiragana_to_roman(cand.hiragana);
      }
      comp.extra.typing_clauses.push_back(typing);
      comp.comp_str += cand.converted;
      break;
    }
  }
  comp.comp_clause[result.clauses.size()] = (DWORD)comp.comp_str.size();
  comp.comp_attr.assign(comp.comp_str.size(), ATTR_CONVERTED);
  comp.extra.iClause = 0;
  comp.SetClauseAttr(comp.extra.iClause, ATTR_TARGET_CONVERTED);
  comp.dwCursorPos = (DWORD)comp.comp_str.size();
  comp.dwDeltaStart = 0;

  // setting cand
  cand.clear();
  for (size_t k = 0; k < result.clauses.size(); ++k) {
    MzConversionClause& clause = result.clauses[k];
    LogCandList cand_list;
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists.push_back(cand_list);
  }
  cand.iClause = 0;
} // MZIMEJA::PluralClauseConversion

void MZIMEJA::PluralClauseConversion(const std::wstring& strHiragana,
                                     MzConversionResult& result)
{
  FOOTMARK();
  // TODO:
  WCHAR sz[64];
  result.clauses.clear();
  for (DWORD iClause = 0; iClause < 5; ++iClause) {
    MzConversionClause clause;
    for (DWORD iCand = 0; iCand < 18; ++iCand) {
      MzConversionCandidate cand;
      ::wsprintfW(sz, L"������%u-%u", iClause, iCand);
      cand.hiragana = sz;
      ::wsprintfW(sz, L"���%u-%u", iClause, iCand);
      cand.converted = sz;
      clause.candidates.push_back(cand);
    }
    result.clauses.push_back(clause);
  }

  result.clauses[0].candidates[0].hiragana = L"�Ђ��[";
  result.clauses[0].candidates[0].converted = L"�q�����[";
  result.clauses[1].candidates[0].hiragana = L"�Ƃ���";
  result.clauses[1].candidates[0].converted = L"�g�����v";
  result.clauses[2].candidates[0].hiragana = L"���񂾁[��";
  result.clauses[2].candidates[0].converted = L"�T���_�[�X";
  result.clauses[3].candidates[0].hiragana = L"�������";
  result.clauses[3].candidates[0].converted = L"�ЎR";
  result.clauses[4].candidates[0].hiragana = L"�����イ����";
  result.clauses[4].candidates[0].converted = L"�F���l";
} // MZIMEJA::PluralClauseConversion

void MZIMEJA::SingleClauseConversion(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  DWORD iClause = comp.extra.iClause;

  MzConversionClause result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[iClause];
  SingleClauseConversion(strHiragana, result);

  comp.SetClauseCompString(iClause, result.candidates[0].converted);
  comp.SetClauseCompHiragana(iClause, result.candidates[0].hiragana, bRoman);

  // setting cand
  LogCandList cand_list;
  for (size_t i = 0; i < result.candidates.size(); ++i) {
    MzConversionCandidate& cand = result.candidates[i];
    cand_list.cand_strs.push_back(cand.converted);
  }
  cand.cand_lists[iClause] = cand_list;
  cand.iClause = iClause;

  comp.extra.iClause = iClause;
}

void MZIMEJA::SingleClauseConversion(const std::wstring& strHiragana,
                                     MzConversionClause& result)
{
  FOOTMARK();
  result.clear();

  // TODO:
  MzConversionCandidate cand;
  cand.hiragana = L"���񂢂Ԃ񂹂ւ񂩂�";
  cand.converted = L"�P�ꕶ�ߕϊ�1";
  result.candidates.push_back(cand);
  cand.hiragana = L"���񂢂Ԃ񂹂ւ񂩂�";
  cand.converted = L"�P�ꕶ�ߕϊ�2";
  result.candidates.push_back(cand);
  cand.hiragana = L"���񂢂Ԃ񂹂ւ񂩂�";
  cand.converted = L"�P�ꕶ�ߕϊ�3";
  result.candidates.push_back(cand);
} // MZIMEJA::SingleClauseConversion

BOOL MZIMEJA::StretchClauseLeft(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  // TODO:
  return FALSE;
} // MZIMEJA::StretchClauseLeft

BOOL MZIMEJA::StretchClauseRight(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  // TODO:
  return FALSE;
} // MZIMEJA::StretchClauseRight

//////////////////////////////////////////////////////////////////////////////
