// convert.cpp --- mzimeja kana kanji conversion
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#include "mzimeja.h"

const DWORD c_dwMilliseconds = 8000;

static const wchar_t s_hiragana_table[][5] = {    
  {L'あ', L'い', L'う', L'え', L'お'},   // GYOU_A
  {L'か', L'き', L'く', L'け', L'こ'},   // GYOU_KA
  {L'が', L'ぎ', L'ぐ', L'げ', L'ご'},   // GYOU_GA
  {L'さ', L'し', L'す', L'せ', L'そ'},   // GYOU_SA
  {L'ざ', L'じ', L'ず', L'ぜ', L'ぞ'},   // GYOU_ZA
  {L'た', L'ち', L'つ', L'て', L'と'},   // GYOU_TA
  {L'だ', L'ぢ', L'づ', L'で', L'ど'},   // GYOU_DA
  {L'な', L'に', L'ぬ', L'ね', L'の'},   // GYOU_NA
  {L'は', L'ひ', L'ふ', L'へ', L'ほ'},   // GYOU_HA
  {L'ば', L'び', L'ぶ', L'べ', L'ぼ'},   // GYOU_BA
  {L'ぱ', L'ぴ', L'ぷ', L'ぺ', L'ぽ'},   // GYOU_PA
  {L'ま', L'み', L'む', L'め', L'も'},   // GYOU_MA
  {L'や', 0, L'ゆ', 0, L'よ'},           // GYOU_YA
  {L'ら', L'り', L'る', L'れ', L'ろ'},   // GYOU_RA
  {L'わ', 0, 0, 0, L'を'},               // GYOU_WA
  {L'ん', 0, 0, 0, 0},                   // GYOU_NN
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
      if (bunrui_str == L"名詞")            entry.bunrui = HB_MEISHI;
      else if (bunrui_str == L"副詞")       entry.bunrui = HB_FUKUSHI;
      else if (bunrui_str == L"助詞")       entry.bunrui = HB_JOSHI;
      else if (bunrui_str == L"漢語")       entry.bunrui = HB_KANGO;
      else continue;
    } else if (bunrui_str.size() == 3) {
      if (bunrui_str == L"連体詞")          entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"接続詞")     entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"感動詞")     entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"接頭辞")     entry.bunrui = HB_SETTOUJI;
      else if (bunrui_str == L"接尾辞")     entry.bunrui = HB_SETSUBIJI;
      else continue;
    } else if (bunrui_str.size() == 4) {
      if (bunrui_str == L"い形容詞")        entry.bunrui = HB_IKEIYOUSHI;
      else if (bunrui_str == L"な形容詞")   entry.bunrui = HB_NAKEIYOUSHI;
      else if (bunrui_str == L"五段動詞")   entry.bunrui = HB_GODAN_DOUSHI;
      else if (bunrui_str == L"一段動詞")   entry.bunrui = HB_ICHIDAN_DOUSHI;
      else if (bunrui_str == L"カ変動詞")   entry.bunrui = HB_KAHEN_DOUSHI;
      else if (bunrui_str == L"サ変動詞")   entry.bunrui = HB_SAHEN_DOUSHI;
      else continue;
    } else if (bunrui_str.size() == 5) {
      if (bunrui_str == L"未然助動詞")      entry.bunrui = HB_MIZEN_JODOUSHI;
      else if (bunrui_str == L"連用助動詞") entry.bunrui = HB_RENYOU_JODOUSHI;
      else if (bunrui_str == L"終止助動詞") entry.bunrui = HB_SHUUSHI_JODOUSHI;
      else if (bunrui_str == L"連体助動詞") entry.bunrui = HB_RENTAI_JODOUSHI;
      else if (bunrui_str == L"仮定助動詞") entry.bunrui = HB_KATEI_JODOUSHI;
      else if (bunrui_str == L"命令助動詞") entry.bunrui = HB_MEIREI_JODOUSHI;
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
      if (fields[0][i] == L'な') fields[0].resize(i);
      i = fields[2].size() - 1;
      if (fields[2][i] == L'な') fields[2].resize(i);
      break;
    case HB_IKEIYOUSHI:
      i = fields[0].size() - 1;
      if (fields[0][i] == L'い') fields[0].resize(i);
      i = fields[2].size() - 1;
      if (fields[2][i] == L'い') fields[2].resize(i);
      break;
    case HB_ICHIDAN_DOUSHI:
      DebugPrintW(L"%s\n", fields[0].c_str());
      assert(fields[0][fields[0].size() - 1] == L'る');
      assert(fields[2][fields[2].size() - 1] == L'る');
      fields[0].resize(fields[0].size() - 1);
      fields[2].resize(fields[2].size() - 1);
      break;
    case HB_KAHEN_DOUSHI:
      if (fields[0] == L"くる") continue;
      substr = fields[0].substr(fields[0].size() - 2, 2);
      if (substr != L"くる") continue;
      fields[0] = substr;
      substr = fields[2].substr(fields[0].size() - 2, 2);
      fields[2] = substr;
      break;
    case HB_SAHEN_DOUSHI:
      if (fields[0] == L"する") continue;
      substr = fields[0].substr(fields[0].size() - 2, 2);
      if (substr == L"する") entry.gyou = GYOU_SA;
      else if (substr != L"ずる") entry.gyou = GYOU_ZA;
      else continue;
      fields[0] = substr;
      fields[2] = fields[2].substr(fields[0].size() - 2, 2);
      break;
    case HB_GODAN_DOUSHI:
      ch = fields[0][fields[0].size() - 1];
      if (m_vowel_map[ch] != L'う') continue;
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

  // 未然形
  do {
    if (str.substr(0, 2) != L"かろ") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"かろ";
    node.post = fields[2] + L"かろ";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 連用形
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.substr(0, 2) != L"かっ") break;
    node.pre = fields[0] + L"かっ";
    node.post = fields[2] + L"かっ";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str[0] != L'く') break;
    node.pre = fields[0] + L'く';
    node.post = fields[2] + L'く';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'う') break;
    node.pre = fields[0] + L'う';
    node.post = fields[2] + L'う';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    wchar_t ch0 = fields[0][fields[0].size() - 1];
    wchar_t ch1 = m_consonant_map[ch0];
    std::wstring addition;
    switch (m_vowel_map[ch0]) {
    case L'あ':
      str = fields[1].substr(0, fields[1].size() - 1);
      for (size_t i = 0; i < _countof(s_hiragana_table); ++i) {
        if (s_hiragana_table[i][0] == ch1) {
          addition = s_hiragana_table[i][DAN_O];;
          addition += L'う';
          str += addition;
          break;
        }
      }
      node.post = str;
      str = fields[0].substr(0, fields[0].size() - 1);
      for (size_t i = 0; i < _countof(s_hiragana_table); ++i) {
        if (s_hiragana_table[i][0] == ch1) {
          addition = s_hiragana_table[i][DAN_O];;
          addition += L'う';
          str += addition;
          break;
        }
      }
      node.pre = str;
      if (str.substr(0, addition.size()) != addition) break;
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + addition.size()]++;
      break;
    case L'い':
      if (str.substr(0, 2) != L"ゅう") break;
      node.pre = fields[0] + L"ゅう";
      node.post = fields[2] + L"ゅう";
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + 2]++;
    default:
      break;
    }
  } while(0);

  // 終止形
  node.katsuyou = SHUUSHI_KEI;
  do {
    if (str[0] != L'い') break;
    node.pre = fields[0] + L'い';
    node.post = fields[2] + L'い';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'し') {
      node.pre = fields[0] + L'し';
      node.post = fields[2] + L'し';
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size() + 1]++;
    } else {
      node.pre = fields[0];
      node.post = fields[2];
      lattice.chunks[index].push_back(node);
      lattice.refs[index + fields[0].size()]++;
    }
  } while(0);

  // 連体形
  node.katsuyou = RENTAI_KEI;
  do {
    if (str[0] != L'い') break;
    node.pre = fields[0] + L'い';
    node.post = fields[2] + L'い';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'き') break;
    node.pre = fields[0] + L'き';
    node.post = fields[2] + L'き';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (str.substr(0, 2) != L"けれ") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"けれ";
    node.post = fields[2] + L"けれ";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 名詞形
  node.katsuyou = MEISHI_KEI;
  do {
    if (str[0] != L'さ') break;
    node.pre = fields[0] + L'さ';
    node.post = fields[2] + L'さ';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'み') break;
    node.pre = fields[0] + L'み';
    node.post = fields[2] + L'み';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'め') break;
    node.pre = fields[0] + L'め';
    node.post = fields[2] + L'目';
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

  // 未然形
  do {
    if (str.substr(0, 2) != L"だろ") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"だろ";
    node.post = fields[2] + L"だろ";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 連用形
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.substr(0, 2) != L"だっ") break;
    node.pre = fields[0] + L"だっ";
    node.post = fields[2] + L"だっ";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str[0] != L'で') break;
    node.pre = fields[0] + L'で';
    node.post = fields[2] + L'で';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'に') break;
    node.pre = fields[0] + L'に';
    node.post = fields[2] + L'に';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  do {
    if (str[0] != L'だ') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'だ';
    node.post = fields[2] + L'だ';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 連体形
  do {
    if (str[0] != L'な') break;
    node.katsuyou = RENTAI_KEI;
    node.pre = fields[0] + L'な';
    node.post = fields[2] + L'な';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (str.substr(0, 2) != L"なら") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"なら";
    node.post = fields[2] + L"なら";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 名詞形
  do {
    if (str[0] != L'さ') break;
    node.katsuyou = MEISHI_KEI;
    node.pre = fields[0] + L'さ';
    node.post = fields[2] + L'さ';
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

  // 未然形
  do {
    node.katsuyou = MIZEN_KEI;
    if (node.gyou == GYOU_A) {
      if (str[0] != L'わ') break;
      node.pre = fields[0] + L'わ';
      node.post = fields[2] + L'わ';
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

  // 連用形
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
    case 1:   ch = L'い'; break;
    case 2:   ch = L'ん'; break;
    case 3:   ch = L'っ'; break;
    }
    if (str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  // 連体形
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

  // 仮定形
  // 命令形
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

  // 名詞形
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

  // 未然形
  // 連用形
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

  // 終止形
  // 連体形
  do {
    if (str[0] != L'る') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'る';
    node.post = fields[2] + L'る';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
    node.katsuyou = RENTAI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (str[0] != L'れ') break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L'れ';
    node.post = fields[2] + L'れ';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 命令形
  node.katsuyou = MEIREI_KEI;
  do {
    if (str[0] != L'ろ') break;
    node.pre = fields[0] + L'ろ';
    node.post = fields[2] + L'ろ';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str[0] != L'よ') break;
    node.pre = fields[0] + L'よ';
    node.post = fields[2] + L'よ';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 名詞形
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

  // 未然形
  do {
    if (str[0] != L'こ') break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L'こ';
    node.post = fields[2] + L'来';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 連用形
  do {
    if (str[0] != L'き') break;
    node.katsuyou = RENYOU_KEI;
    node.pre = fields[0] + L'き';
    node.post = fields[2] + L'来';
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  // 連用形
  do {
    if (str.substr(0, 2) != L"くる") break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L"くる";
    node.post = fields[2] + L"来る";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
    node.katsuyou = RENYOU_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 仮定形
  do {
    if (str.substr(0, 2) != L"くれ") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"くれ";
    node.post = fields[2] + L"来れ";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 命令形
  do {
    if (str.substr(0, 2) != L"こい") break;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0] + L"こい";
    node.post = fields[2] + L"来い";
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 名詞形
  do {
    if (str[0] != L'き') break;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0] + L'き';
    node.post = fields[2] + L'来';
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

  // 未然形
  node.katsuyou = MIZEN_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'ざ') break;
      node.pre = fields[0] + L'ざ';
      node.post = fields[2] + L'ざ';
    } else {
      if (str[0] != L'さ') break;
      node.pre = fields[0] + L'さ';
      node.post = fields[2] + L'さ';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'じ') break;
      node.pre = fields[0] + L'じ';
      node.post = fields[2] + L'じ';
    } else {
      if (str[0] != L'し') break;
      node.pre = fields[0] + L'し';
      node.post = fields[2] + L'し';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'ぜ') break;
      node.pre = fields[0] + L'ぜ';
      node.post = fields[2] + L'ぜ';
    } else {
      if (str[0] != L'せ') break;
      node.pre = fields[0] + L'せ';
      node.post = fields[2] + L'せ';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 連用形
  node.katsuyou = RENYOU_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str[0] != L'じ') break;
      node.pre = fields[0] + L'じ';
      node.post = fields[2] + L'じ';
    } else {
      if (str[0] != L'し') break;
      node.pre = fields[0] + L'し';
      node.post = fields[2] + L'し';
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  // 連用形
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"ずる") break;
      node.pre = fields[0] + L"ずる";
      node.post = fields[2] + L"ずる";
    } else {
      if (str.substr(0, 2) != L"する") break;
      node.pre = fields[0] + L"する";
      node.post = fields[2] + L"する";
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
      if (str[0] != L'ず') break;
      node.pre = fields[0] + L'ず';
      node.post = fields[2] + L'ず';
    } else {
      if (str[0] != L'す') break;
      node.pre = fields[0] + L'す';
      node.post = fields[2] + L'す';
    }
    node.katsuyou = SHUUSHI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"ずれ") break;
      node.pre = fields[0] + L"ずれ";
      node.post = fields[2] + L"ずれ";
    } else {
      if (str.substr(0, 2) != L"すれ") break;
      node.pre = fields[0] + L"すれ";
      node.post = fields[2] + L"すれ";
    }
    node.katsuyou = KATEI_KEI;
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);

  // 命令形
  node.katsuyou = MEIREI_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"じろ") break;
      node.pre = fields[0] + L"じろ";
      node.post = fields[2] + L"じろ";
    } else {
      if (str.substr(0, 2) != L"しろ") break;
      node.pre = fields[0] + L"しろ";
      node.post = fields[2] + L"しろ";
    }
    lattice.chunks[index].push_back(node);
    lattice.refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.substr(0, 2) != L"ぜよ") break;
      node.pre = fields[0] + L"ぜよ";
      node.post = fields[2] + L"ぜよ";
    } else {
      if (str.substr(0, 2) != L"せよ") break;
      node.pre = fields[0] + L"せよ";
      node.post = fields[2] + L"せよ";
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
      ::wsprintfW(sz, L"こうほ%u-%u", iClause, iCand);
      cand.hiragana = sz;
      ::wsprintfW(sz, L"候補%u-%u", iClause, iCand);
      cand.converted = sz;
      clause.candidates.push_back(cand);
    }
    result.clauses.push_back(clause);
  }
  result.clauses[0].candidates[0].hiragana = L"ひらりー";
  result.clauses[0].candidates[0].converted = L"ヒラリー";
  result.clauses[1].candidates[0].hiragana = L"とらんぷ";
  result.clauses[1].candidates[0].converted = L"トランプ";
  result.clauses[2].candidates[0].hiragana = L"さんだーす";
  result.clauses[2].candidates[0].converted = L"サンダース";
  result.clauses[3].candidates[0].hiragana = L"かたやま";
  result.clauses[3].candidates[0].converted = L"片山";
  result.clauses[4].candidates[0].hiragana = L"うちゅうじん";
  result.clauses[4].candidates[0].converted = L"宇宙人";
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
  cand.hiragana = L"たんいつぶんせつへんかん";
  cand.converted = L"単一文節変換1";
  result.candidates.push_back(cand);
  cand.hiragana = L"たんいつぶんせつへんかん";
  cand.converted = L"単一文節変換2";
  result.candidates.push_back(cand);
  cand.hiragana = L"たんいつぶんせつへんかん";
  cand.converted = L"単一文節変換3";
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
