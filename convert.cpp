// convert.cpp --- mzimeja kana kanji conversion
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#include "mzimeja.h"

const DWORD c_dwMilliseconds = 8000;

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

void MZIMEJA::MakeLiteralMaps() {
  if (m_consonant_map.size()) {
    return;
  }
  m_consonant_map.clear();
  m_vowel_map.clear();
  const size_t count = _countof(s_hiragana_table);
  for (size_t i = 0; i < count; ++i) {
    for (size_t k = 0; k < 5; ++k) {
      m_consonant_map[s_hiragana_table[i][k]] = s_hiragana_table[i][0];
    }
    for (size_t k = 0; k < 5; ++k) {
      m_vowel_map[s_hiragana_table[i][k]] = s_hiragana_table[0][k];
    }
  }
}

inline bool entry_compare_pre(const DICT_ENTRY& e1, const DICT_ENTRY& e2) {
  return (e1.pre < e2.pre);
}

BOOL MZIMEJA::LoadBasicDictFile(std::vector<DICT_ENTRY>& entries) {
  FOOTMARK();
  char buf[256];
  wchar_t wbuf[256];
  std::wstring str;
  using namespace std;

  entries.clear();
  entries.reserve(60000);

  // open basic dictionary
  std::wstring filename = GetComputerString(L"basic dictionary file");
  FILE *fp = _wfopen(filename.c_str(), L"rb");
  if (fp == NULL) {
    DebugPrintW(L"ERROR: cannot open dict: %s\n", filename.c_str());
    assert(0);
    return FALSE; // failure
  }

  // load each line
  int lineno = 0;
  while (fgets(buf, 256, fp) != NULL) {
    ++lineno;
    if (buf[0] == ';') continue;  // comment

    // convert to UTF-16
    ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
    std::wstring str = wbuf;

    // split to fields
    unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
    FIELDS fields;
    unboost::split(fields, str, unboost::is_any_of(L"\t"));

    // is it an invalid line?
    if (fields.empty() || fields[0].empty()) {
      assert(0);
      continue;
    }

    // parse a record
    DICT_ENTRY entry;
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
      else if (bunrui_str == L"����")       entry.bunrui = HB_JOSHI;
      else if (bunrui_str == L"����")       entry.bunrui = HB_KANGO;
      else continue;
    } else if (bunrui_str.size() == 3) {
      if (bunrui_str == L"�A�̎�")          entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"�ڑ���")     entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"������")     entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"�ړ���")     entry.bunrui = HB_SETTOUJI;
      else if (bunrui_str == L"�ڔ���")     entry.bunrui = HB_SETSUBIJI;
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
      DebugPrintW(L"%s\n", fields[0].c_str());
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
      if (m_vowel_map[ch] != L'��') continue;
      fields[0].resize(fields[0].size() - 1);
      fields[2].resize(fields[2].size() - 1);
      ch = m_consonant_map[ch];
      for (i = 0; i < _countof(s_hiragana_table); ++i) {
        if (s_hiragana_table[i][0] == ch) {
          ngyou = i;
          break;
        }
      }
      entry.gyou = (GYOU)ngyou;
      break;
    default:
      break;
    }

    // store the parsed record to entries
    entry.pre = fields[0];
    entry.post = fields[2];
    if (fields.size() >= 4) {
      entry.tags = fields[3];
    }
    entries.push_back(entry);
  }

  // sort by preconversion string
  std::sort(entries.begin(), entries.end(), entry_compare_pre);
  return TRUE;  // success
} // MZIMEJA::LoadBasicDictFile

BOOL MZIMEJA::DeployDictData(
  IMAGE_BASE *data, SECURITY_ATTRIBUTES *psa,
  const std::vector<DICT_ENTRY>& entries)
{
  FOOTMARK();

  // calculate the total size
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

  // wait for locking dict data
  BOOL ret = FALSE;
  DWORD dwWait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds);
  if (dwWait != WAIT_OBJECT_0) {
    return ret;
  }

  // create shared dict data
  m_hBasicDictData = ::CreateFileMappingW(INVALID_HANDLE_VALUE, psa,
    PAGE_READWRITE, 0, size, L"mzimeja_basic_dict");
  LPVOID pv = ::MapViewOfFile(
    m_hBasicDictData, FILE_MAP_ALL_ACCESS, 0, 0, size);
  if (pv) {
    // store all data
    size_t cch;
    WCHAR *pch = reinterpret_cast<WCHAR *>(pv);
    *pch++ += L'\n';  // new line
    for (size_t i = 0; i < entries.size(); ++i) {
      // line format: pre \t MAKEWORD(bunrui, gyou) \t post \t tags \n
      const DICT_ENTRY& entry = entries[i];
      // pre \t
      cch = entry.pre.size();
      memcpy(pch, entry.pre.c_str(), cch * sizeof(WCHAR));
      pch += cch;
      *pch++ = L'\t';
      // MAKEWORD(bunrui, gyou) \t
      *pch++ = MAKEWORD(entry.bunrui, entry.gyou);
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
    assert(size / 2 == size_t(pch - reinterpret_cast<WCHAR *>(pv)));
    #if 0
      FILE *fp = fopen("c:\\dictdata.txt", "wb");
      fwrite(pv, size, 1, fp);
      fclose(fp);
    #endif
    ::UnmapViewOfFile(pv);
    ret = TRUE; // success
  }

  // store the total size
  data->dwSharedDictDataSize = size;

  // it's done, unlock the mutex
  ::ReleaseMutex(m_hMutex);

  return ret;
} // MZIMEJA::DeployDictData

//////////////////////////////////////////////////////////////////////////////

BOOL MZIMEJA::LoadBasicDict() {
  FOOTMARK();
  BOOL ret = FALSE;

  // get the base shared data
  IMAGE_BASE *data = LockImeBaseData();
  if (data == NULL) {
    assert(0);
    return ret; // failure
  }

  // create a security attributes
  SECURITY_ATTRIBUTES *psa = CreateSecurityAttributes();
  assert(psa);

  // is the signature valid?
  if (data->dwSignature == 0xDEADFACE) {
    // is it already stored?
    if (data->dwSharedDictDataSize == 0) {
      // no data, so deploy dict data
      std::vector<DICT_ENTRY> entries;
      if (LoadBasicDictFile(entries)) {
        ret = DeployDictData(data, psa, entries);
      }
      assert(ret);
    } else {
      // data is already stored. wait for locking dict data
      DWORD dwWait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds);
      if (dwWait == WAIT_OBJECT_0) {
        // open shared dict data
        m_hBasicDictData = ::CreateFileMappingW(INVALID_HANDLE_VALUE, psa,
          PAGE_READONLY, 0, data->dwSharedDictDataSize, L"mzimeja_basic_dict");
        if (m_hBasicDictData) {
          ret = TRUE; // success
        }
        assert(ret);
        // unlock the dict data
        ::ReleaseMutex(m_hMutex);
      }
    }
  }

  // release the security attributes
  FreeSecurityAttributes(psa);

  // unlock the base shared data
  UnlockImeBaseData(data);

  return ret;
} // MZIMEJA::LoadBasicDict

BOOL MZIMEJA::IsBasicDictLoaded() const {
  return m_hBasicDictData != NULL;
}

WCHAR *MZIMEJA::LockBasicDict() {
  // get size
  DWORD dwSize = 0;
  IMAGE_BASE *data = LockImeBaseData();
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
  return NULL;  // failure
}

void MZIMEJA::UnlockBasicDict(WCHAR *data) {
  ::UnmapViewOfFile(data);
}

//////////////////////////////////////////////////////////////////////////////
// making a lattice

void MZIMEJA::MakeLattice(LATTICE& lattice) {
  WCHAR *dict_data = LockBasicDict();
  MakeLattice(lattice, dict_data);
  UnlockBasicDict(dict_data);
}

void MZIMEJA::MakeLattice(LATTICE& lattice, const WCHAR *dict_data) {
  const std::wstring& pre = lattice.pre;
  const size_t length = pre.size();
  assert(length);

  lattice.chunks.resize(length);
  lattice.refs.resize(length + 1, 0);
  lattice.refs[0] = 1;

  for (size_t index = 0; index < length; ++index) {
    if (lattice.refs[index] == 0) continue;
    RECORDS records;
    ScanDict(records, dict_data, pre[index]);
    for (size_t k = 0; k < records.size(); ++k) {
      const std::wstring& record = records[k];
      FIELDS fields;
      unboost::split(fields, record, unboost::is_any_of(L"\t"));
      ParseFields(lattice, index, fields);
    }
  }
}

void MZIMEJA::CutExtraNodes(LATTICE& lattice) {
  const std::wstring& pre = lattice.pre;
  const size_t length = pre.size();
  assert(length);
  assert(length == lattice.chunks.size());
  assert(length + 1 == lattice.refs.size());
  for (size_t index = 0; index < length; ++index) {
    if (lattice.refs[index] == 0) continue;
    // TODO:
  }
}

BOOL MZIMEJA::ScanDict(RECORDS& records, const WCHAR *dict_data, WCHAR ch) {
  WCHAR sz[3] = {ch, L'\n', 0};
  const WCHAR *pch1 = wcsstr(dict_data, sz);
  if (pch1 == NULL) {
    return FALSE;
  }

  std::wstring str;
  const WCHAR *pch2;
  const WCHAR *pch3;
  pch2 = pch1;
  for (;;) {
    pch3 = wcsstr(pch1, pch2 + 1);
    if (pch3 == NULL) break;
    pch2 = pch3;
  }
  pch3 = wcschr(pch2, L'\n');
  assert(pch3);
  str.assign(pch1 + 1, pch3 - 1);

  unboost::split(records, str, unboost::is_any_of(L"\n"));
  assert(records.size());
  return TRUE;
}

void
MZIMEJA::ParseIkeiyoushi(LATTICE& lattice, size_t index, const FIELDS& fields) {
  assert(fields.size() == 4);
  assert(fields[0].size());
  std::wstring str = lattice.pre.substr(index + fields[0].size());
  LATTICE_NODE node;
  node.bunrui = HB_IKEIYOUSHI;
  node.cost = 0;

  // ���R�`
  do {
    if (str.substr(0, 2) != L"����") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.substr(0, 2) != L"����") break;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    wchar_t ch0 = fields[0][fields[0].size() - 1];
    wchar_t ch1 = m_consonant_map[ch0];
    std::wstring addition;
    switch (m_vowel_map[ch0]) {
    case L'��':
      str = fields[1].substr(0, fields[1].size() - 1);
      for (size_t i = 0; i < _countof(s_hiragana_table); ++i) {
        if (s_hiragana_table[i][0] == ch1) {
          addition = s_hiragana_table[i][DAN_O];;
          addition += L'��';
          str += addition;
          break;
        }
      }
      node.post = str;
      str = fields[0].substr(0, fields[0].size() - 1);
      for (size_t i = 0; i < _countof(s_hiragana_table); ++i) {
        if (s_hiragana_table[i][0] == ch1) {
          addition = s_hiragana_table[i][DAN_O];;
          addition += L'��';
          str += addition;
          break;
        }
      }
      node.pre = str;
      if (str.substr(0, addition.size()) != addition) break;
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + addition.size()]++;
      break;
    case L'��':
      if (str.substr(0, 2) != L"�イ") break;
      node.pre = fields[0] + L"�イ";
      node.post = fields[2] + L"�イ";
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + 2]++;
    default:
      break;
    }
  } while(0);

  // �I�~�`
  node.katsuyou = SHUUSHI_KEI;
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'��') {
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + 1]++;
    } else {
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size()]++;
    }
  } while(0);

  // �A�̌`
  node.katsuyou = RENTAI_KEI;
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (str.substr(0, 2) != L"����") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // �����`
  node.katsuyou = MEISHI_KEI;
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
}

void
MZIMEJA::ParseNakeiyoushi(LATTICE& lattice, size_t index, const FIELDS& fields) {
  assert(fields.size() == 4);
  assert(fields[0].size());
  std::wstring str = lattice.pre.substr(index + fields[0].size());
  LATTICE_NODE node;
  node.bunrui = HB_NAKEIYOUSHI;
  node.cost = 0;

  // ���R�`
  do {
    if (str.substr(0, 2) != L"����") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.substr(0, 2) != L"����") break;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �A�̌`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = RENTAI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (str.substr(0, 2) != L"�Ȃ�") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"�Ȃ�";
    node.post = fields[2] + L"�Ȃ�";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // �����`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = MEISHI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
}

void
MZIMEJA::ParseGodanDoushi(LATTICE& lattice, size_t index, const FIELDS& fields) {
  assert(fields.size() == 4);
  assert(fields[0].size());
  std::wstring str = lattice.pre.substr(index + fields[0].size());
  LATTICE_NODE node;
  node.bunrui = HB_GODAN_DOUSHI;
  node.cost = 0;

  WORD w = fields[1][0];
  node.gyou = (GYOU)HIBYTE(w);

  int type;
  switch (node.gyou) {
  case GYOU_KA: case GYOU_GA:                 type = 1; break;
  case GYOU_NA: case GYOU_BA: case GYOU_MA:   type = 2; break;
  case GYOU_TA: case GYOU_RA: case GYOU_WA:   type = 3; break;
  default:                                    type = 0; break;
  }

  // ���R�`
  do {
    node.katsuyou = MIZEN_KEI;
    if (node.gyou == GYOU_A) {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + 1]++;
    } else {
      wchar_t ch = s_hiragana_table[node.gyou][DAN_A];
      if (str[0] != ch) break;
      node.pre = fields[0] + ch;
      node.post = fields[2] + ch;
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + 1]++;
    }
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
    if (str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (type == 0) break;
    wchar_t ch;
    switch (type) {
    case 1:   ch = L'��'; break;
    case 2:   ch = L'��'; break;
    case 3:   ch = L'��'; break;
    }
    if (str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  // �A�̌`
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_U];
    if (str[0] != ch) break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
    node.katsuyou = RENTAI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  // ���ߌ`
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_E];
    if (str[0] != ch) break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
    node.katsuyou = MEIREI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �����`
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
    if (str[0] != ch) break;
    node.katsuyou = MEISHI_KEI;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
}

void
MZIMEJA::ParseIchidanDoushi(LATTICE& lattice, size_t index, const FIELDS& fields) {
  assert(fields.size() == 4);
  assert(fields[0].size());
  std::wstring str = lattice.pre.substr(index + fields[0].size());
  LATTICE_NODE node;
  node.bunrui = HB_ICHIDAN_DOUSHI;
  node.cost = 0;

  // ���R�`
  // �A�p�`
  do {
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size()]++;
    node.katsuyou = RENYOU_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size()]++;
  } while(0);

  // �I�~�`
  // �A�̌`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
    node.katsuyou = RENTAI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // ���ߌ`
  node.katsuyou = MEIREI_KEI;
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �����`
  do {
    node.katsuyou = MEISHI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size()]++;
  } while(0);
}

void
MZIMEJA::ParseKahenDoushi(LATTICE& lattice, size_t index, const FIELDS& fields) {
  assert(fields.size() == 4);
  assert(fields[0].size());
  std::wstring str = lattice.pre.substr(index + fields[0].size());
  LATTICE_NODE node;
  node.bunrui = HB_KAHEN_DOUSHI;
  node.cost = 0;

  // ���R�`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �A�p�`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = RENYOU_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  // �A�p�`
  do {
    if (str.substr(0, 2) != L"����") break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
    node.katsuyou = RENYOU_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // ����`
  do {
    if (str.substr(0, 2) != L"����") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // ���ߌ`
  do {
    if (str.substr(0, 2) != L"����") break;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // �����`
  do {
    if (str[0] != L'��') break;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
}

void
MZIMEJA::ParseSahenDoushi(LATTICE& lattice, size_t index, const FIELDS& fields) {
  assert(fields.size() == 4);
  assert(fields[0].size());
  std::wstring str = lattice.pre.substr(index + fields[0].size());
  LATTICE_NODE node;
  node.bunrui = HB_SAHEN_DOUSHI;
  node.cost = 0;

  WORD w = fields[1][0];
  node.gyou = (GYOU)HIBYTE(w);

  // ���R�`
  node.katsuyou = MIZEN_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  // �A�p�`
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    node.katsuyou = SHUUSHI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;

    node.katsuyou = RENYOU_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    node.katsuyou = SHUUSHI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    node.katsuyou = KATEI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // ���ߌ`
  node.katsuyou = MEIREI_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
}

void
MZIMEJA::ParseFields(LATTICE& lattice, size_t index, const FIELDS& fields) {
  assert(fields.size() == 4);
  size_t length = fields[0].size();
  WORD w = fields[1][0];
  LATTICE_NODE node;
  node.bunrui = (HINSHI_BUNRUI)LOBYTE(w);
  node.gyou = (GYOU)HIBYTE(w);
  node.cost = 0;
  switch (node.bunrui) {
  case HB_MEISHI:
  case HB_RENTAISHI:
  case HB_FUKUSHI:
  case HB_SETSUZOKUSHI:
  case HB_KANDOUSHI:
  case HB_JOSHI:
  case HB_KANGO:
  case HB_SETTOUJI:
  case HB_SETSUBIJI:
    if (lattice.pre.substr(index, length) == fields[0]) {
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index]++;
    }
    break;
  case HB_IKEIYOUSHI:
    ParseIkeiyoushi(lattice, index, fields);
    break;
  case HB_NAKEIYOUSHI:
    ParseNakeiyoushi(lattice, index, fields);
    break;
  case HB_MIZEN_JODOUSHI:
    if (lattice.pre.substr(index, length) == fields[0]) {
      node.bunrui = HB_JODOUSHI;
      node.katsuyou = MIZEN_KEI;
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index]++;
    }
    break;
  case HB_RENYOU_JODOUSHI:
    if (lattice.pre.substr(index, length) == fields[0]) {
      node.bunrui = HB_JODOUSHI;
      node.katsuyou = RENYOU_KEI;
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index]++;
    }
    break;
  case HB_SHUUSHI_JODOUSHI:
    if (lattice.pre.substr(index, length) == fields[0]) {
      node.bunrui = HB_JODOUSHI;
      node.katsuyou = SHUUSHI_KEI;
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index]++;
    }
    break;
  case HB_RENTAI_JODOUSHI:
    if (lattice.pre.substr(index, length) == fields[0]) {
      node.bunrui = HB_JODOUSHI;
      node.katsuyou = RENTAI_KEI;
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index]++;
    }
    break;
  case HB_KATEI_JODOUSHI:
    if (lattice.pre.substr(index, length) == fields[0]) {
      node.bunrui = HB_JODOUSHI;
      node.katsuyou = KATEI_KEI;
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index]++;
    }
    break;
  case HB_MEIREI_JODOUSHI:
    if (lattice.pre.substr(index, length) == fields[0]) {
      node.bunrui = HB_JODOUSHI;
      node.katsuyou = MEIREI_KEI;
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index]++;
    }
    break;
  case HB_GODAN_DOUSHI:
    ParseGodanDoushi(lattice, index, fields);
    break;
  case HB_ICHIDAN_DOUSHI:
    ParseIchidanDoushi(lattice, index, fields);
    break;
  case HB_KAHEN_DOUSHI:
    ParseKahenDoushi(lattice, index, fields);
    break;
  case HB_SAHEN_DOUSHI:
    ParseSahenDoushi(lattice, index, fields);
    break;
  default:
    break;
  }
}

void MZIMEJA::PluralClauseConversion(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  MzConversionResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[comp.extra.iClause];
  PluralClauseConversion(strHiragana, result);

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

void MZIMEJA::MakeResult(MzConversionResult& result, LATTICE& lattice) {
  // TODO:
}

void MZIMEJA::PluralClauseConversion(const std::wstring& strHiragana,
                                     MzConversionResult& result)
{
  FOOTMARK();

#if 0
  // TODO:
  LATTICE lattice;
  lattice.pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  MakeLattice(lattice);
  CutExtraNodes(lattice);
  MakeResult(result, lattice);
#else
  // dummy sample
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
#endif
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

#if 0
  // TODO:
#else
  // dummy sample
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
#endif
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
