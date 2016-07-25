// convert.cpp --- mzimeja kana kanji conversion
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#include "mzimeja.h"

const DWORD c_dwMilliseconds = 8000;
const DWORD c_retry_count = 32;

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

static const wchar_t *BunruiToString(HinshiBunrui bunrui) {
  int index = int(bunrui) - int(HB_HEAD);
  static const wchar_t *s_array[] = {
    L"HB_HEAD",
    L"HB_TAIL",
    L"HB_UNKNOWN",
    L"HB_MEISHI",
    L"HB_IKEIYOUSHI",
    L"HB_NAKEIYOUSHI",
    L"HB_RENTAISHI",
    L"HB_FUKUSHI",
    L"HB_SETSUZOKUSHI",
    L"HB_KANDOUSHI",
    L"HB_KAKU_JOSHI",
    L"HB_SETSUZOKU_JOSHI",
    L"HB_FUKU_JOSHI",
    L"HB_SHUU_JOSHI",
    L"HB_JODOUSHI",
    L"HB_MIZEN_JODOUSHI",
    L"HB_RENYOU_JODOUSHI",
    L"HB_SHUUSHI_JODOUSHI",
    L"HB_RENTAI_JODOUSHI",
    L"HB_KATEI_JODOUSHI",
    L"HB_MEIREI_JODOUSHI",
    L"HB_GODAN_DOUSHI",
    L"HB_ICHIDAN_DOUSHI",
    L"HB_KAHEN_DOUSHI",
    L"HB_SAHEN_DOUSHI",
    L"HB_KANGO",
    L"HB_SETTOUJI",
    L"HB_SETSUBIJI",
    L"HB_PERIOD",
    L"HB_COMMA",
    L"HB_SYMBOLS"
  };
  return s_array[index];
}

// 品詞の連結コスト
static int
CandConnectCost(
  const std::wstring& tags1, HinshiBunrui bunrui1,
  const std::wstring& tags2, HinshiBunrui bunrui2)
{
  if (bunrui2 == HB_PERIOD || bunrui2 == HB_COMMA) return 0;
  if (bunrui2 == HB_TAIL) return 0;
  if (bunrui1 == HB_SYMBOLS || bunrui2 == HB_SYMBOLS) return 0;
  if (bunrui1 == HB_UNKNOWN || bunrui2 == HB_UNKNOWN) return 0;
  switch (bunrui1) {
  case HB_HEAD:
    switch (bunrui2) {
    case HB_JODOUSHI: case HB_SHUU_JOSHI:
      return 5;
    default:
      break;
    }
    break;
  case HB_MEISHI: // 名詞
    switch (bunrui2) {
    case HB_MEISHI:
      if (tags1.find(L"[数詞]") != std::wstring::npos) {
        if (tags2.find(L"[数詞]") != std::wstring::npos ||
            tags2.find(L"[数単位]") != std::wstring::npos)
        {
          return 0;
        }
      }
      return 10;
    case HB_SETTOUJI:
      return 10;
    case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
      return 5;
    default:
      break;
    }
    break;
  case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI: // い形容詞、な形容詞
    switch (bunrui1) {
    case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
      return 10;
    case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
    case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
      return 3;
    default:
      break;
    }
    break;
  case HB_MIZEN_JODOUSHI: case HB_RENYOU_JODOUSHI:
  case HB_SHUUSHI_JODOUSHI: case HB_RENTAI_JODOUSHI:
  case HB_KATEI_JODOUSHI: case HB_MEIREI_JODOUSHI:
    assert(0);
    break;
  case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI: case HB_KAHEN_DOUSHI:
  case HB_SAHEN_DOUSHI: case HB_SETTOUJI:
    switch (bunrui2) {
    case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI: case HB_KAHEN_DOUSHI:
    case HB_SAHEN_DOUSHI: case HB_SETTOUJI:
      return 5;
    }
    break;
  case HB_RENTAISHI: case HB_FUKUSHI: case HB_SETSUZOKUSHI:
  case HB_KANDOUSHI: case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
  case HB_FUKU_JOSHI: case HB_SHUU_JOSHI: case HB_JODOUSHI:
  case HB_SETSUBIJI: case HB_COMMA: case HB_PERIOD:
  default:
    break;
  }
  return 0;
} // CandConnectCost

// 品詞の連結可能性
static BOOL
IsNodeConnectable(const LatticeNode& node1, const LatticeNode& node2) {
  if (node2.bunrui == HB_PERIOD || node2.bunrui == HB_COMMA) return TRUE;
  if (node2.bunrui == HB_TAIL) return TRUE;
  if (node1.bunrui == HB_SYMBOLS || node2.bunrui == HB_SYMBOLS) return TRUE;
  if (node1.bunrui == HB_UNKNOWN || node2.bunrui == HB_UNKNOWN) return TRUE;

  switch (node1.bunrui) {
  case HB_HEAD:
    switch (node2.bunrui) {
    case HB_SHUU_JOSHI:
      return FALSE;
    default:
      break;
    }
    return TRUE;
  case HB_MEISHI: // 名詞
    switch (node2.bunrui) {
    case HB_SETTOUJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI: // い形容詞、な形容詞
    switch (node1.katsuyou) {
    case MIZEN_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[未然形に連結]")) {
          if (node2.pre[0] == L'な' || node2.pre == L"う") {
            return TRUE;
          }
        }
      }
      return FALSE;
    case RENYOU_KEI:
      switch (node2.bunrui) {
      case HB_JODOUSHI:
        if (node2.HasTag(L"[連用形に連結]")) {
          return TRUE;
        }
        return FALSE;
      case HB_MEISHI: case HB_SETTOUJI:
      case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
      case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
      case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
        return TRUE;
      default:
        return FALSE;
      }
      break;
    case SHUUSHI_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[終止形に連結]")) {
          return TRUE;
        }
        if (node2.HasTag(L"[種々の語]")) {
          return TRUE;
        }
      }
      if (node2.bunrui == HB_SHUU_JOSHI) {
        return TRUE;
      }
      return FALSE;
    case RENTAI_KEI:
      switch (node2.bunrui) {
      case HB_KANDOUSHI: case HB_JODOUSHI:
        return FALSE;
      default:
        return TRUE;
      }
    case KATEI_KEI:
      switch (node2.bunrui) {
      case HB_SETSUZOKU_JOSHI:
        if (node2.pre == L"ば" || node2.pre == L"ども" || node2.pre == L"ど") {
          return TRUE;
        }
      default:
        break;
      }
      return FALSE;
    case MEIREI_KEI:
      switch (node2.bunrui) {
      case HB_SHUU_JOSHI: case HB_MEISHI: case HB_SETTOUJI:
        return TRUE;
      default:
        break;
      }
      return FALSE;
    }
    break;
  case HB_RENTAISHI:  // 連体詞
    switch (node2.bunrui) {
    case HB_KANDOUSHI: case HB_JODOUSHI: case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_FUKUSHI:  // 副詞
    switch (node2.bunrui) {
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
    case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_SETSUZOKUSHI: // 接続詞
    switch (node2.bunrui) {
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI: case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_KANDOUSHI:  // 感動詞
    switch (node2.bunrui) {
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI: case HB_SETSUBIJI: case HB_JODOUSHI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
    // 終助詞以外の助詞
    switch (node2.bunrui) {
    case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_SHUU_JOSHI: // 終助詞
    switch (node2.bunrui) {
    case HB_MEISHI: case HB_SETTOUJI: case HB_SHUU_JOSHI:
      return TRUE;
    default:
      return FALSE;
    }
    break;
  case HB_JODOUSHI: // 助動詞
    switch (node1.katsuyou) {
    case MIZEN_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[未然形に連結]")) {
          return TRUE;
        }
      }
      return FALSE;
    case RENYOU_KEI:
      switch (node2.bunrui) {
      case HB_JODOUSHI:
        if (node2.HasTag(L"[連用形に連結]")) {
          return TRUE;
        }
        return FALSE;
      case HB_MEISHI: case HB_SETTOUJI:
      case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
      case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
      case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
        return TRUE;
      default:
        return FALSE;
      }
      break;
    case SHUUSHI_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[終止形に連結]")) {
          return TRUE;
        }
        if (node2.HasTag(L"[種々の語]")) {
          return TRUE;
        }
      }
      if (node2.bunrui == HB_SHUU_JOSHI) {
        return TRUE;
      }
      return FALSE;
    case RENTAI_KEI:
      switch (node2.bunrui) {
      case HB_KANDOUSHI: case HB_JODOUSHI:
        return FALSE;
      default:
        return TRUE;
      }
    case KATEI_KEI:
      switch (node2.bunrui) {
      case HB_SETSUZOKU_JOSHI:
        if (node2.pre == L"ば" || node2.pre == L"ども" || node2.pre == L"ど") {
          return TRUE;
        }
        break;
      default:
        break;
      }
      return FALSE;
    case MEIREI_KEI:
      switch (node2.bunrui) {
      case HB_SHUU_JOSHI: case HB_MEISHI: case HB_SETTOUJI:
        return TRUE;
      default:
        return FALSE;
      }
    }
    break;
  case HB_MIZEN_JODOUSHI: case HB_RENYOU_JODOUSHI:
  case HB_SHUUSHI_JODOUSHI: case HB_RENTAI_JODOUSHI:
  case HB_KATEI_JODOUSHI: case HB_MEIREI_JODOUSHI:
    assert(0);
    break;
  case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
  case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
    // 動詞
    switch (node1.katsuyou) {
    case MIZEN_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[未然形に連結]")) {
          return TRUE;
        }
      }
      return FALSE;
    case RENYOU_KEI:
      switch (node2.bunrui) {
      case HB_JODOUSHI:
        if (node2.HasTag(L"[連用形に連結]")) {
          return TRUE;
        }
        return FALSE;
      case HB_MEISHI: case HB_SETTOUJI:
      case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
      case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
      case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
        return TRUE;
      default:
        return FALSE;
      }
      break;
    case SHUUSHI_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[終止形に連結]")) {
          return TRUE;
        }
        if (node2.HasTag(L"[種々の語]")) {
          return TRUE;
        }
      }
      if (node2.bunrui == HB_SHUU_JOSHI) {
        return TRUE;
      }
      return FALSE;
    case RENTAI_KEI:
      switch (node2.bunrui) {
      case HB_KANDOUSHI: case HB_JODOUSHI:
        return FALSE;
      default:
        return TRUE;
      }
      break;
    case KATEI_KEI:
      switch (node2.bunrui) {
      case HB_SETSUZOKU_JOSHI:
        if (node2.pre == L"ば" || node2.pre == L"ども" || node2.pre == L"ど") {
          return TRUE;
        }
      default:
        break;
      }
      return FALSE;
    case MEIREI_KEI:
      switch (node2.bunrui) {
      case HB_SHUU_JOSHI: case HB_MEISHI: case HB_SETTOUJI:
        return TRUE;
      default:
        return FALSE;
      }
      break;
    }
    break;
  case HB_SETTOUJI:   // 接頭辞
    switch (node2.bunrui) {
    case HB_MEISHI:
      return TRUE;
    default:
      return FALSE;
    }
    break;
  case HB_SETSUBIJI:  // 接尾辞
    switch (node2.bunrui) {
    case HB_SETTOUJI:
      return FALSE;
    default:
      break;
    }
    break;
  case HB_COMMA: case HB_PERIOD:  // 、。
    switch (node2.bunrui) {
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
    case HB_SHUU_JOSHI: case HB_SETSUBIJI: case HB_JODOUSHI:
      return FALSE;
    default:
      break;
    }
    break;
  default:
    break;
  }
  return TRUE;
} // IsNodeConnectable

static size_t ScanDict(WStrings& records, const WCHAR *dict_data, WCHAR ch) {
  FOOTMARK();
  assert(dict_data);

  WCHAR sz[3] = {RECORD_SEP, ch, 0};
  const WCHAR *pch1 = wcsstr(dict_data, sz);
  if (pch1 == NULL) {
    return FALSE;
  }

  std::wstring str;
  const WCHAR *pch2 = pch1;
  const WCHAR *pch3;
  for (;;) {
    pch3 = wcsstr(pch2 + 1, sz);
    if (pch3 == NULL) break;
    pch2 = pch3;
  }
  pch3 = wcschr(pch2 + 1, RECORD_SEP);
  assert(pch3);
  str.assign(pch1 + 1, pch3);

  sz[0] = RECORD_SEP;
  sz[1] = 0;
  unboost::split(records, str, unboost::is_any_of(sz));
  assert(records.size());
  return records.size();
} // ScanDict

void MzIme::MakeLiteralMaps() {
  FOOTMARK();
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
} // MzIme::MakeLiteralMaps

//////////////////////////////////////////////////////////////////////////////
// Dict (dictionary)

Dict::Dict() {
  m_hMutex = NULL;
  m_hFileMapping = NULL;
}

Dict::~Dict() {
  Unload();
}

DWORD Dict::GetSize() const {
  DWORD ret = 0;
  WIN32_FIND_DATAW find;
  find.nFileSizeLow = 0;
  HANDLE hFind = ::FindFirstFileW(m_strFileName.c_str(), &find);
  if (hFind != INVALID_HANDLE_VALUE) {
    ::CloseHandle(hFind);
    ret = find.nFileSizeLow;
  }
  return ret;
}

BOOL Dict::Load(const wchar_t *file_name, const wchar_t *object_name) {
  if (IsLoaded()) {
    return TRUE;
  }

  m_strFileName = file_name;
  m_strObjectName = object_name;

  SECURITY_ATTRIBUTES *psa = CreateSecurityAttributes();
  assert(psa);

  if (m_hMutex == NULL) {
    m_hMutex = ::CreateMutexW(psa, FALSE, m_strObjectName.c_str());
  }
  if (m_hMutex == NULL) {
    // free sa
    FreeSecurityAttributes(psa);
    return FALSE;
  }

  // get file size
  DWORD cbSize = GetSize();
  if (cbSize == 0) return FALSE;

  BOOL ret = FALSE;
  DWORD wait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds);
  if (wait == WAIT_OBJECT_0) {
    // create a file mapping
    m_hFileMapping = ::CreateFileMappingW(
      INVALID_HANDLE_VALUE, psa, PAGE_READWRITE,
      0, cbSize, (m_strObjectName + L"FileMapping").c_str());
    if (m_hFileMapping != NULL) {
      // file mapping was created
      if (::GetLastError() == ERROR_ALREADY_EXISTS) {
        // already exists
        ret = TRUE;
      } else {
        // newly created, load from file
        FILE *fp = _wfopen(m_strFileName.c_str(), L"rb");
        if (fp) {
          wchar_t *pch = Lock();
          if (pch) {
            ret = fread(pch, cbSize, 1, fp);
            Unlock(pch);
          }
          fclose(fp);
        }
      }
    }
    ::ReleaseMutex(m_hMutex);
  }

  // free sa
  FreeSecurityAttributes(psa);

  return ret;
} // Dict::Load

void Dict::Unload() {
  if (m_hMutex) {
    if (m_hFileMapping) {
      DWORD wait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds);
      if (wait == WAIT_OBJECT_0) {
        if (m_hFileMapping) {
          ::CloseHandle(m_hFileMapping);
          m_hFileMapping = NULL;
        }
        ::ReleaseMutex(m_hMutex);
      }
    }
    ::CloseHandle(m_hMutex);
    m_hMutex = NULL;
  }
}

wchar_t *Dict::Lock() {
  if (m_hFileMapping == NULL) return NULL;
  DWORD cbSize = GetSize();
  void *pv = ::MapViewOfFile(m_hFileMapping,
                             FILE_MAP_ALL_ACCESS, 0, 0, cbSize);
  return reinterpret_cast<wchar_t *>(pv);
}

void Dict::Unlock(wchar_t *data) {
  ::UnmapViewOfFile(data);
}

BOOL Dict::IsLoaded() const {
  return (m_hMutex != NULL && m_hFileMapping != NULL);
}

//////////////////////////////////////////////////////////////////////////////
// MzConversionResult, MzConversionClause etc.

void MzConversionClause::add(const LatticeNode *node) {
  bool matched = false;
  for (size_t i = 0; i < candidates.size(); ++i) {
    if (candidates[i].converted == node->post) {
      if (candidates[i].cost > node->cost)  {
        candidates[i].cost = node->cost;
        candidates[i].bunruis.insert(node->bunrui);
        candidates[i].tags += node->tags;
      }
      matched = true;
      break;
    }
  }

  if (!matched) {
    MzConversionCandidate cand;
    cand.hiragana = node->pre;
    cand.converted = node->post;
    cand.cost = node->cost;
    cand.bunruis.insert(node->bunrui);
    cand.tags = node->tags;
    candidates.push_back(cand);
  }
}

static inline bool CandidateCompare(
  const MzConversionCandidate& cand1, const MzConversionCandidate& cand2)
{
  FOOTMARK();
  return cand1.cost < cand2.cost;
}

void MzConversionClause::sort() {
  FOOTMARK();
  std::sort(candidates.begin(), candidates.end(), CandidateCompare);
}

void MzConversionResult::sort() {
  FOOTMARK();

  for (size_t i = 0; i < clauses.size(); ++i) {
    clauses[i].sort();
  }

  for (size_t i = 1; i < clauses.size(); ++i) {
    for (size_t iCand1 = 0; iCand1 < clauses[i - 1].candidates.size(); ++iCand1) {
      for (size_t iCand2 = 0; iCand2 < clauses[i].candidates.size(); ++iCand2) {
        MzConversionCandidate& cand1 = clauses[i - 1].candidates[iCand1];
        MzConversionCandidate& cand2 = clauses[i].candidates[iCand2];
        int min_cost = 0x7FFF;
        std::set<HinshiBunrui>::iterator it1, end1 = cand1.bunruis.end();
        std::set<HinshiBunrui>::iterator it2, end2 = cand2.bunruis.end();
        for (it1 = cand1.bunruis.begin(); it1 != end1; ++it1)  {
          for (it2 = cand2.bunruis.begin(); it2 != end2; ++it2)  {
            int cost = CandConnectCost(cand1.tags, *it1, cand2.tags, *it2);
            if (cost < min_cost) {
              min_cost = cost;
            }
          }
        }
        cand2.cost += min_cost;
      }
    }
  }

  for (size_t i = 0; i < clauses.size(); ++i) {
    clauses[i].sort();
  }
}

//////////////////////////////////////////////////////////////////////////////
// LatticeNode

int LatticeNode::CalcCost() const {
  int ret = 0;
  if (bunrui == HB_KANGO) ret += 200;
  if (tags.size() != 0) {
    if (HasTag(L"[非標準]")) ret += 100;
    if (HasTag(L"[不謹慎]")) ret += 50;
    if (HasTag(L"[人名]")) ret += 30;
    else if (HasTag(L"[駅名]")) ret += 30;
    else if (HasTag(L"[地名]")) ret += 30;
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// Lattice

void Lattice::AddExtra() {
  if (pre == L"きょう") {
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    WCHAR sz[32];

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

    wsprintfW(sz, L"%u年%u月%u日", st.wYear, st.wMonth, st.wDay);
    fields[2] = sz;
    DoFields(0, fields);

    wsprintfW(sz, L"%04u/%02u/%02u", st.wYear, st.wMonth, st.wDay);
    fields[2] = sz;
    DoFields(0, fields);

    wsprintfW(sz, L"%02u/%02u/%04u", st.wMonth, st.wDay, st.wYear);
    fields[2] = sz;
    DoFields(0, fields);
    return;
  }
  if (pre == L"ことし") {
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    WCHAR sz[32];

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

    wsprintfW(sz, L"%u年", st.wYear);
    fields[2] = sz;
    DoFields(0, fields);
    return;
  }
  if (pre == L"じこく") {
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    WCHAR sz[32];

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

    wsprintfW(sz, L"%u時%u分%u秒", st.wHour, st.wMinute, st.wSecond);
    fields[2] = sz;
    DoFields(0, fields);

    wsprintfW(sz, L"%02u時%02u分%02u秒", st.wHour, st.wMinute, st.wSecond);
    fields[2] = sz;
    DoFields(0, fields);

    if (st.wHour >= 12) {
      wsprintfW(sz, L"午後%u時%u分%u秒", st.wHour - 12, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);

      wsprintfW(sz, L"午後%02u時%02u分%02u秒", st.wHour - 12, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);
    } else {
      wsprintfW(sz, L"午前%u時%u分%u秒", st.wHour, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);

      wsprintfW(sz, L"午前%02u時%02u分%02u秒", st.wHour, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);
    }

    wsprintfW(sz, L"%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
    fields[2] = sz;
    DoFields(0, fields);
    return;
  }
  if (pre == L"かっこ") {
    WStrings items;
    unboost::split(items, TheIME.LoadSTR(101), unboost::is_any_of(L"\t"));

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_SYMBOLS, 0));
    for (size_t i = 0; i < items.size(); ++i) {
      fields[2] = items[i];
      DoFields(0, fields);
    }
    return;
  }
  if (pre == L"けいせん") {
    WStrings items;
    unboost::split(items, TheIME.LoadSTR(100), unboost::is_any_of(L"\t"));

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_SYMBOLS, 0));
    for (size_t i = 0; i < items.size(); ++i) {
      fields[2] = items[i];
      DoFields(0, fields);
    }
    return;
  }
  if (pre == L"じぶん") {
    WCHAR sz[64];
    DWORD dwSize = _countof(sz);
    if (::GetUserNameW(sz, &dwSize)) {
      WStrings fields(4);
      fields[0] = pre;
      fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));
      fields[2] = sz;
      DoFields(0, fields);
    }
    return;
  }
}

BOOL Lattice::AddNodes(size_t index, const WCHAR *dict_data) {
  FOOTMARK();
  const size_t length = pre.size();
  assert(length);

  std::wstring sep;
  sep += FIELD_SEP;

  WStrings fields, records;
  for (; index < length; ++index) {
    if (refs[index] == 0) continue;

    if (is_period(pre[index])) {
      size_t saved = index;
      do {
        ++index;
      } while (is_period(pre[index]));

      fields.resize(4);
      fields[0] = pre.substr(saved, index - saved);
      fields[1] = MAKEWORD(HB_PERIOD, 0);
      switch (index - saved) {
      case 2:
        fields[2] += L'‥';
        break;
      case 3:
        fields[2] += L'…';
        break;
      default:
        fields[2] = fields[0];
        break;
      }
      DoFields(saved, fields);
      --index;
      continue;
    }
    if (is_comma(pre[index])) {
      size_t saved = index;
      do {
        ++index;
      } while (is_comma(pre[index]));

      fields.resize(4);
      fields[0] = pre.substr(saved, index - saved);
      fields[1] = MAKEWORD(HB_COMMA, 0);
      fields[2] = fields[0];
      DoFields(saved, fields);
      --index;
      continue;
    }
    if (!is_hiragana(pre[index])) {
      size_t saved = index;
      do {
        ++index;
      } while (!is_hiragana(pre[index]) && pre[index]);

      fields.resize(4);
      fields[0] = pre.substr(saved, index - saved);
      fields[1] = MAKEWORD(HB_MEISHI, 0);
      fields[2] = fields[0];
      DoMeishi(saved, fields);
      --index;
      continue;
    }

    size_t count = ScanDict(records, dict_data, pre[index]);
    DebugPrintW(L"ScanDict(%c) count: %d\n", pre[index], count);
    for (size_t k = 0; k < records.size(); ++k) {
      const std::wstring& record = records[k];
      unboost::split(fields, record, unboost::is_any_of(sep));
      DoFields(index, fields);
    }

    switch (pre[index]) {
    case L'こ': case L'き': case L'く':
      fields.resize(4);
      fields[1].push_back(MAKEWORD(HB_KAHEN_DOUSHI, GYOU_KA));
      DoKahenDoushi(index, fields);
      break;
    case L'さ': case L'し': case L'せ': case L'す':
      fields.resize(4);
      fields[1].push_back(MAKEWORD(HB_SAHEN_DOUSHI, GYOU_SA));
      DoSahenDoushi(index, fields);
      break;
    default:
      break;
    }
  }

  return TRUE;
} // Lattice::AddNodes

struct DeleteDifferentSizeNode {
  size_t m_size;
  DeleteDifferentSizeNode(size_t size) {
    m_size = size;
  }
  bool operator()(const LatticeNodePtr& n) {
    return n->pre.size() != m_size;
  }
};

BOOL Lattice::AddNodesForSingle(const WCHAR *dict_data) {
  FOOTMARK();

  std::wstring sep;
  sep += FIELD_SEP;

  WStrings fields, records;
  size_t count = ScanDict(records, dict_data, pre[0]);
  DebugPrintW(L"ScanDict(%c) count: %d\n", pre[0], count);
  for (size_t k = 0; k < records.size(); ++k) {
    const std::wstring& record = records[k];
    unboost::split(fields, record, unboost::is_any_of(sep));
    DoFields(0, fields);
  }

  DeleteDifferentSizeNode del(pre.size());
  for (size_t i = 0; i < chunks[0].size(); ++i) {
    std::remove_if(chunks[0].begin(), chunks[0].end(), del);
  }

  return !chunks[0].empty();
}

void Lattice::UpdateRefs() {
  FOOTMARK();
  const size_t length = pre.size();
  refs.assign(length + 1, 0);
  refs[0] = 1;

  for (size_t index = 0; index < length; ++index) {
    if (refs[index] == 0) continue;
    LatticeChunk& chunk1 = chunks[index];
    for (size_t k = 0; k < chunk1.size(); ++k) {
      refs[index + chunk1[k]->pre.size()]++;
    }
  }
} // Lattice::UpdateRefs

void Lattice::UpdateLinks() {
  FOOTMARK();
  const size_t length = pre.size();
  assert(length);
  assert(length + 1 == chunks.size());
  assert(length + 1 == refs.size());

  UnlinkAllNodes();

  // add head and link to head
  {
    LatticeNode node;
    node.bunrui = HB_HEAD;
    node.linked = 1;
    LatticeChunk& chunk1 = chunks[0];
    for (size_t k = 0; k < chunk1.size(); ++k) {
      chunk1[k]->linked = 1;
      node.branches.push_back(chunk1[k]);
    }
    head = unboost::make_shared(node);
  }
  // add tail
  {
    LatticeNode node;
    node.bunrui = HB_TAIL;
    chunks[length].clear();
    chunks[length].push_back(unboost::make_shared(node));
  }

  // add links and branches
  size_t num_links = 0;
  for (size_t index = 0; index < length; ++index) {
    LatticeChunk& chunk1 = chunks[index];
    for (size_t k = 0; k < chunk1.size(); ++k) {
      if (!chunk1[k]->linked) continue;
      const std::wstring& pre = chunk1[k]->pre;
      LatticeChunk& chunk2 = chunks[index + pre.size()];
      for (size_t m = 0; m < chunk2.size(); ++m) {
        if (IsNodeConnectable(*chunk1[k].get(), *chunk2[m].get())) {
          chunk1[k]->branches.push_back(chunk2[m]);
          chunk2[m]->linked++;
          num_links++;
        }
      }
    }
  }
  DebugPrintA("num_links: %d\n", (int)num_links);
} // Lattice::UpdateLinks

void Lattice::UnlinkAllNodes() {
  FOOTMARK();
  const size_t length = pre.size();
  for (size_t index = 0; index < length; ++index) {
    LatticeChunk& chunk1 = chunks[index];
    for (size_t k = 0; k < chunk1.size(); ++k) {
      chunk1[k]->linked = 0;
      chunk1[k]->branches.clear();
    }
  }
} // Lattice::UnlinkAllNodes

void Lattice::AddComplement(size_t index, size_t min_size, size_t max_size) {
  FOOTMARK();
  const size_t length = pre.size();

  WStrings fields(4);
  fields[1].assign(1, MAKEWORD(HB_UNKNOWN, 0));
  //fields[3].clear();
  for (size_t count = min_size; count <= max_size; ++count) {
    if (length < index + count) continue;
    fields[0] = pre.substr(index, count);
    fields[2] = fields[0];
    DoFields(index, fields);
  }
} // Lattice::AddComplement

static inline bool IsNodeUnlinked(const LatticeNodePtr& node) {
  FOOTMARK();
  return node->linked == 0;
}

void Lattice::CutUnlinkedNodes() {
  FOOTMARK();
  const size_t length = pre.size();
  for (size_t index = 0; index < length; ++index) {
    LatticeChunk& chunk1 = chunks[index];
    std::remove_if(chunk1.begin(), chunk1.end(), IsNodeUnlinked);
  }
} // Lattice::CutUnlinkedNodes

size_t Lattice::GetLastLinkedIndex() const {
  FOOTMARK();
  // is the last node linked?
  const size_t length = pre.size();
  if (chunks[length][0]->linked) {
    return length;  // return the last index
  }

  // scan chunks in reverse order
  for (size_t index = length; index > 0; ) {
    --index;
    for (size_t k = 0; k < chunks[index].size(); ++k) {
      if (chunks[index][k]->linked) {
        return index; // the linked node was found
      }
    }
  }
  return 0;
} // Lattice::GetLastLinkedIndex

void Lattice::DoIkeiyoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  std::wstring str = pre.substr(index + fields[0].size());

  LatticeNode node;
  node.bunrui = HB_IKEIYOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // 未然形
  do {
    if (str.empty() || str.substr(0, 2) != L"かろ") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"かろ";
    node.post = fields[2] + L"かろ";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 連用形
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.empty() || str.substr(0, 2) != L"かっ") break;
    node.pre = fields[0] + L"かっ";
    node.post = fields[2] + L"かっ";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'く') break;
    node.pre = fields[0] + L'く';
    node.post = fields[2] + L'く';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'う') break;
    node.pre = fields[0] + L'う';
    node.post = fields[2] + L'う';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (fields[0].empty() || str.empty()) break;
    wchar_t ch0 = fields[0][fields[0].size() - 1];
    wchar_t ch1 = TheIME.m_consonant_map[ch0];
    std::wstring addition;
    switch (TheIME.m_vowel_map[ch0]) {
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
      if (str.empty() || str.substr(0, addition.size()) != addition) break;
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + fields[0].size() + addition.size()]++;
      break;
    case L'い':
      if (str.empty() || str.substr(0, 2) != L"ゅう") break;
      node.pre = fields[0] + L"ゅう";
      node.post = fields[2] + L"ゅう";
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + fields[0].size() + 2]++;
    default:
      break;
    }
  } while(0);

  // 終止形
  node.katsuyou = SHUUSHI_KEI;
  do {
    if (str.empty() || str[0] != L'い') break;
    node.pre = fields[0] + L'い';
    node.post = fields[2] + L'い';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 連体形
  node.katsuyou = RENTAI_KEI;
  do {
    if (str.empty() || str[0] != L'い') break;
    node.pre = fields[0] + L'い';
    node.post = fields[2] + L'い';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'き') break;
    node.pre = fields[0] + L'き';
    node.post = fields[2] + L'き';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (str.empty() || str.substr(0, 2) != L"けれ") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"けれ";
    node.post = fields[2] + L"けれ";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 名詞形
  node.bunrui = HB_MEISHI;
  do {
    if (str.empty() || str[0] != L'さ') break;
    node.pre = fields[0] + L'さ';
    node.post = fields[2] + L'さ';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'み') break;
    node.pre = fields[0] + L'み';
    node.post = fields[2] + L'み';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'め') break;
    node.pre = fields[0] + L'め';
    node.post = fields[2] + L'目';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // (い形容詞の語幹)+"そうだ"
  if (str.size() >= 2 && str[0] == L'そ' && str[1] == L'う') {
    WStrings new_fields = fields;
    new_fields[0] = fields[0] + L"そう";
    new_fields[2] = fields[2] + L"そう";
    DoNakeiyoushi(index, new_fields);
  }

  // 「痛。」「寒。」など
  if (str.size()) {
    switch (str[0]) {
    case L'。': case L'、': case L'，': case L'．': case 0:
      DoMeishi(index, fields);
      break;
    }
  }
} // Lattice::DoIkeiyoushi

void Lattice::DoNakeiyoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  std::wstring str = pre.substr(index + fields[0].size());

  LatticeNode node;
  node.bunrui = HB_NAKEIYOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // 未然形
  do {
    if (str.empty() || str.substr(0, 2) != L"だろ") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"だろ";
    node.post = fields[2] + L"だろ";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 連用形
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.empty() || str.substr(0, 2) != L"だっ") break;
    node.pre = fields[0] + L"だっ";
    node.post = fields[2] + L"だっ";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'で') break;
    node.pre = fields[0] + L'で';
    node.post = fields[2] + L'で';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'に') break;
    node.pre = fields[0] + L'に';
    node.post = fields[2] + L'に';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  do {
    if (str.empty() || str[0] != L'だ') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'だ';
    node.post = fields[2] + L'だ';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 連体形
  do {
    if (str.empty() || str[0] != L'な') break;
    node.katsuyou = RENTAI_KEI;
    node.pre = fields[0] + L'な';
    node.post = fields[2] + L'な';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (str.empty() || str.substr(0, 2) != L"なら") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"なら";
    node.post = fields[2] + L"なら";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 名詞形
  node.bunrui = HB_MEISHI;
  do {
    if (str.empty() || str[0] != L'さ') break;
    node.pre = fields[0] + L'さ';
    node.post = fields[2] + L'さ';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 「きれい。」「静か。」など
  switch (str[0]) {
  case L'。': case L'、': case L'，': case L'．': case 0:
    DoMeishi(index, fields);
    break;
  }
} // Lattice::DoNakeiyoushi

void Lattice::DoGodanDoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  std::wstring str = pre.substr(index + length);
  DebugPrintW(L"DoGodanDoushi: %s, %s\n", fields[0].c_str(), str.c_str());

  LatticeNode node;
  node.bunrui = HB_GODAN_DOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  WORD w = fields[1][0];
  node.gyou = (Gyou)HIBYTE(w);

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
      if (str.empty() || str[0] != L'わ') break;
      node.pre = fields[0] + L'わ';
      node.post = fields[2] + L'わ';
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + fields[0].size() + 1]++;
    } else {
      wchar_t ch = s_hiragana_table[node.gyou][DAN_A];
      if (str.empty() || str[0] != ch) break;
      node.pre = fields[0] + ch;
      node.post = fields[2] + ch;
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + fields[0].size() + 1]++;
    }
  } while(0);

  // 連用形
  node.katsuyou = RENYOU_KEI;
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
    if (str.empty() || str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (type == 0) break;
    wchar_t ch;
    switch (type) {
    case 1:   ch = L'い'; break;
    case 2:   ch = L'ん'; break;
    case 3:   ch = L'っ'; break;
    }
    if (str.empty() || str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  // 連体形
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_U];
    if (str.empty() || str[0] != ch) break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
    node.katsuyou = RENTAI_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  // 命令形
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_E];
    if (str.empty() || str[0] != ch) break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
    node.katsuyou = MEIREI_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 名詞形
  node.bunrui = HB_MEISHI;
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
    if (str.empty() || str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 「動く(五段)」→「動ける(一段)」、
  // 「聞く(五段)」→「聞ける(一段)」など
  {
    WStrings new_fields = fields;
    new_fields[0] += s_hiragana_table[node.gyou][DAN_I];
    new_fields[2] += s_hiragana_table[node.gyou][DAN_I];
    DoIchidanDoushi(index, new_fields);
  }
} // Lattice::DoGodanDoushi

void Lattice::DoIchidanDoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_ICHIDAN_DOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // 未然形
  // 連用形
  do {
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size()]++;
    node.katsuyou = RENYOU_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size()]++;
  } while(0);

  // 終止形
  // 連体形
  do {
    if (str.empty() || str[0] != L'る') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'る';
    node.post = fields[2] + L'る';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
    node.katsuyou = RENTAI_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (str.empty() || str[0] != L'れ') break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L'れ';
    node.post = fields[2] + L'れ';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 命令形
  node.katsuyou = MEIREI_KEI;
  do {
    if (str.empty() || str[0] != L'ろ') break;
    node.pre = fields[0] + L'ろ';
    node.post = fields[2] + L'ろ';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'よ') break;
    node.pre = fields[0] + L'よ';
    node.post = fields[2] + L'よ';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 名詞形
  node.bunrui = HB_MEISHI;
  do {
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size()]++;
  } while(0);
} // Lattice::DoIchidanDoushi

void Lattice::DoKahenDoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_KAHEN_DOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // 未然形
  do {
    if (str.empty() || str[0] != L'こ') break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L'こ';
    node.post = fields[2] + L'来';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 連用形
  do {
    if (str.empty() || str[0] != L'き') break;
    node.katsuyou = RENYOU_KEI;
    node.pre = fields[0] + L'き';
    node.post = fields[2] + L'来';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  // 連用形
  do {
    if (str.empty() || str.substr(0, 2) != L"くる") break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L"くる";
    node.post = fields[2] + L"来る";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
    node.katsuyou = RENYOU_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 仮定形
  do {
    if (str.empty() || str.substr(0, 2) != L"くれ") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"くれ";
    node.post = fields[2] + L"来れ";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 命令形
  do {
    if (str.empty() || str.substr(0, 2) != L"こい") break;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0] + L"こい";
    node.post = fields[2] + L"来い";
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 名詞形
  node.bunrui = HB_MEISHI;
  do {
    if (str.empty() || str[0] != L'き') break;
    node.pre = fields[0] + L'き';
    node.post = fields[2] + L'来';
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
} // Lattice::DoKahenDoushi

void Lattice::DoSahenDoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_SAHEN_DOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  WORD w = fields[1][0];
  node.gyou = (Gyou)HIBYTE(w);

  // 未然形
  node.katsuyou = MIZEN_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'ざ') break;
      node.pre = fields[0] + L'ざ';
      node.post = fields[2] + L'ざ';
    } else {
      if (str.empty() || str[0] != L'さ') break;
      node.pre = fields[0] + L'さ';
      node.post = fields[2] + L'さ';
    }
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'じ') break;
      node.pre = fields[0] + L'じ';
      node.post = fields[2] + L'じ';
    } else {
      if (str.empty() || str[0] != L'し') break;
      node.pre = fields[0] + L'し';
      node.post = fields[2] + L'し';
    }
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'ぜ') break;
      node.pre = fields[0] + L'ぜ';
      node.post = fields[2] + L'ぜ';
    } else {
      if (str.empty() || str[0] != L'せ') break;
      node.pre = fields[0] + L'せ';
      node.post = fields[2] + L'せ';
    }
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 連用形
  node.katsuyou = RENYOU_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'じ') break;
      node.pre = fields[0] + L'じ';
      node.post = fields[2] + L'じ';
    } else {
      if (str.empty() || str[0] != L'し') break;
      node.pre = fields[0] + L'し';
      node.post = fields[2] + L'し';
    }
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 終止形
  // 連用形
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"ずる") break;
      node.pre = fields[0] + L"ずる";
      node.post = fields[2] + L"ずる";
    } else {
      if (str.empty() || str.substr(0, 2) != L"する") break;
      node.pre = fields[0] + L"する";
      node.post = fields[2] + L"する";
    }
    node.katsuyou = SHUUSHI_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;

    node.katsuyou = RENYOU_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'ず') break;
      node.pre = fields[0] + L'ず';
      node.post = fields[2] + L'ず';
    } else {
      if (str.empty() || str[0] != L'す') break;
      node.pre = fields[0] + L'す';
      node.post = fields[2] + L'す';
    }
    node.katsuyou = SHUUSHI_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // 仮定形
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"ずれ") break;
      node.pre = fields[0] + L"ずれ";
      node.post = fields[2] + L"ずれ";
    } else {
      if (str.empty() || str.substr(0, 2) != L"すれ") break;
      node.pre = fields[0] + L"すれ";
      node.post = fields[2] + L"すれ";
    }
    node.katsuyou = KATEI_KEI;
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // 命令形
  node.katsuyou = MEIREI_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"じろ") break;
      node.pre = fields[0] + L"じろ";
      node.post = fields[2] + L"じろ";
    } else {
      if (str.empty() || str.substr(0, 2) != L"しろ") break;
      node.pre = fields[0] + L"しろ";
      node.post = fields[2] + L"しろ";
    }
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"ぜよ") break;
      node.pre = fields[0] + L"ぜよ";
      node.post = fields[2] + L"ぜよ";
    } else {
      if (str.empty() || str.substr(0, 2) != L"せよ") break;
      node.pre = fields[0] + L"せよ";
      node.post = fields[2] + L"せよ";
    }
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
} // Lattice::DoSahenDoushi

void Lattice::DoMeishi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());

  size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_MEISHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  if (pre.substr(index, length) == fields[0]) {
    if (node.HasTag(L"[動植物]")) {
      // animals or plants are written in katakana
      node.pre = fields[0];
      node.post = lcmap(fields[0], LCMAP_KATAKANA | LCMAP_FULLWIDTH);
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + length]++;

      node.cost += 30;
      node.pre = fields[0];
      node.post = fields[2];
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + length]++;
    } else {
      node.pre = fields[0];
      node.post = fields[2];
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + length]++;
    }
  }

  if (str.size() >= 2 && str[0] == L'っ' && str[1] == L'ぽ') {
    WStrings new_fields = fields;
    new_fields[0] += L"っぽ";
    new_fields[2] += L"っぽ";
    DoIkeiyoushi(index, new_fields);
  }
} // Lattice::DoMeishi

void Lattice::DoFields(size_t index, const WStrings& fields) {
  assert(fields.size() == 4);
  const size_t length = fields[0].size();
  if (index + length > pre.size()) {
    return;
  }
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  DebugPrintW(L"DoFields: %s\n", fields[0].c_str());

  WORD w = fields[1][0];

  LatticeNode node;
  node.bunrui = (HinshiBunrui)LOBYTE(w);
  node.gyou = (Gyou)HIBYTE(w);
  node.tags = fields[3];
  node.cost = node.CalcCost();

  switch (node.bunrui) {
  case HB_MEISHI:
    DoMeishi(index, fields);
    break;
  case HB_PERIOD: case HB_COMMA: case HB_SYMBOLS:
  case HB_RENTAISHI: case HB_FUKUSHI:
  case HB_SETSUZOKUSHI: case HB_KANDOUSHI:
  case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
  case HB_FUKU_JOSHI: case HB_SHUU_JOSHI:
  case HB_KANGO: case HB_SETTOUJI: case HB_SETSUBIJI:
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + length]++;
    break;
  case HB_IKEIYOUSHI:
    DoIkeiyoushi(index, fields);
    break;
  case HB_NAKEIYOUSHI:
    DoNakeiyoushi(index, fields);
    break;
  case HB_MIZEN_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + length]++;
    break;
  case HB_RENYOU_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = RENYOU_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + length]++;
    break;
  case HB_SHUUSHI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + length]++;
    break;
  case HB_RENTAI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = RENTAI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + length]++;
    break;
  case HB_KATEI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + length]++;
    break;
  case HB_MEIREI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index + length]++;
    break;
  case HB_GODAN_DOUSHI:
    DoGodanDoushi(index, fields);
    break;
  case HB_ICHIDAN_DOUSHI:
    DoIchidanDoushi(index, fields);
    break;
  case HB_KAHEN_DOUSHI:
    DoKahenDoushi(index, fields);
    break;
  case HB_SAHEN_DOUSHI:
    DoSahenDoushi(index, fields);
    break;
  default:
    break;
  }
} // Lattice::DoFields

void Lattice::Dump(int num) {
#ifndef NDEBUG
  const size_t length = pre.size();
  DebugPrintW(L"### Lattice::Dump(%d) ###\n", num);
  DebugPrintW(L"Lattice length: %d\n", int(length));
  for (size_t i = 0; i < length; ++i) {
    DebugPrintW(L"Lattice chunk #%d:", int(i));
    for (size_t k = 0; k < chunks[i].size(); ++k) {
      DebugPrintW(L" %s(%s)", chunks[i][k]->post.c_str(),
        BunruiToString(chunks[i][k]->bunrui));
    }
    DebugPrintW(L"\n");
  }
#endif  // ndef NDEBUG
} // Lattice::Dump

//////////////////////////////////////////////////////////////////////////////

BOOL MzIme::MakeLattice(Lattice& lattice, const std::wstring& pre) {
  FOOTMARK();

  if (!m_basic_dict.IsLoaded()) {
    return FALSE;
  }

  // initialize lattice
  assert(pre.size() != 0);
  const size_t length = pre.size();
  lattice.pre = pre;
  lattice.chunks.resize(length + 1);
  lattice.refs.assign(length + 1, 0);
  lattice.refs[0] = 1;

  // lock the dictionary
  WCHAR *dict_data = m_basic_dict.Lock();
  size_t count = 0;
  if (dict_data) {
    // add nodes
    lattice.AddNodes(0, dict_data);
    // dump
    lattice.Dump(1);
    // repeat until linked to tail
    for (;;) {
      // link and cut not linked
      lattice.UpdateLinks();
      lattice.CutUnlinkedNodes();
      // dump
      lattice.Dump(2);
      // does it reach the last?
      size_t index = lattice.GetLastLinkedIndex();
      if (index == length) break;
      // add complement
      lattice.UpdateRefs();
      lattice.AddComplement(index, 1, 5);
      lattice.AddNodes(index + 1, dict_data);
      // dump
      lattice.Dump(3);

      ++count;
      if (count >= c_retry_count) break;
    }
    // unlock the dictionary
    m_basic_dict.Unlock(dict_data);
    if (count < c_retry_count) {
      return TRUE;  // success
    }
  }

  // dump
  lattice.Dump(4);
  return FALSE; // failure
} // MzIme::MakeLattice

BOOL MzIme::MakeLatticeForSingle(Lattice& lattice, const std::wstring& pre) {
  FOOTMARK();

  if (!m_basic_dict.IsLoaded()) {
    return FALSE;
  }

  // initialize lattice
  assert(pre.size() != 0);
  const size_t length = pre.size();
  lattice.pre = pre;
  lattice.chunks.resize(length + 1);
  lattice.refs.assign(length + 1, 0);
  lattice.refs[0] = 1;

  // lock the dictionary
  WCHAR *dict_data = m_basic_dict.Lock();
  if (dict_data) {
    // add nodes
    if (!lattice.AddNodesForSingle(dict_data)) {
      lattice.AddComplement(0, pre.size(), pre.size());
    }

    // unlock the dictionary
    m_basic_dict.Unlock(dict_data);
    return TRUE;  // success
  }

  // dump
  lattice.Dump(4);
  return FALSE; // failure
} // MzIme::MakeLatticeForSingle

void MzIme::MakeResult(MzConversionResult& result, Lattice& lattice) {
  FOOTMARK();
  result.clear();

  const size_t length = lattice.pre.size();
  LatticeNodePtr node1 = lattice.head;
  LatticeNodePtr tail = lattice.chunks[length][0];
  while (node1 != tail) {
    size_t kb1 = 0, max_len = 0, max_len1 = 0;
    for (size_t ib1 = 0; ib1 < node1->branches.size(); ++ib1) {
      LatticeNodePtr& node2 = node1->branches[ib1];
      if (node2->branches.empty()) {
        const size_t len = node2->pre.size();
        if (max_len < len) {
          max_len1 = node2->pre.size();
          max_len = len;
          kb1 = ib1;
        }
      } else {
        for (size_t ib2 = 0; ib2 < node2->branches.size(); ++ib2) {
          LatticeNodePtr& node3 = node2->branches[ib2];
          const size_t len = node2->pre.size() + node3->pre.size();
          if (max_len < len) {
            max_len1 = node2->pre.size();
            max_len = len;
            kb1 = ib1;
          } else if (max_len == len) {
            if (max_len1 < node2->pre.size()) {
              max_len1 = node2->pre.size();
              max_len = len;
              kb1 = ib1;
            }
          }
        }
      }
    }

    // add clause
    if (node1->branches[kb1]->pre.size()) {
      MzConversionClause clause;
      clause.add(node1->branches[kb1].get());
      result.clauses.push_back(clause);
    }

    // go next
    node1 = node1->branches[kb1];
  }

  // add other candidates
  size_t index = 0, iClause = 0;
  while (index < length && iClause < result.clauses.size()) {
    const LatticeChunk& chunk = lattice.chunks[index];
    MzConversionClause& clause = result.clauses[iClause];

    std::wstring hiragana = clause.candidates[0].hiragana;
    const size_t size = hiragana.size();
    for (size_t i = 0; i < chunk.size(); ++i) {
      if (chunk[i]->pre.size() == size) {
        // add a candidate of same size
        clause.add(chunk[i].get());
      }
    }

    if (is_hiragana(hiragana[0])) {
      LatticeNode node;
      node.bunrui = HB_UNKNOWN;
      node.cost = 10;

      node.pre = hiragana;
      node.post = hiragana;
      clause.add(&node);

      std::wstring katakana;
      katakana = lcmap(hiragana, LCMAP_FULLWIDTH | LCMAP_KATAKANA);
      node.post = katakana;
      clause.add(&node);
    } else {
      LatticeNode node;
      node.bunrui = HB_UNKNOWN;
      node.cost = 10;

      node.pre = hiragana;
      node.post = lcmap(hiragana, LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
      clause.add(&node);
      node.post = lcmap(hiragana, LCMAP_UPPERCASE | LCMAP_FULLWIDTH);
      clause.add(&node);
      node.post = node.post[0] + lcmap(hiragana.substr(1), LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
      clause.add(&node);
      node.post = lcmap(hiragana, LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
      clause.add(&node);
      node.post = lcmap(hiragana, LCMAP_UPPERCASE | LCMAP_HALFWIDTH);
      clause.add(&node);
      node.post = node.post[0] + lcmap(hiragana.substr(1), LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
      clause.add(&node);
    }

    index += size;
    ++iClause;
  }

  result.sort();
} // MzIme::MakeResult

void MzIme::MakeResult(MzConversionResult& result, const std::wstring& pre) {
  FOOTMARK();
  result.clear();

  LatticeNode node;
  node.pre = pre;
  node.post = pre;
  node.cost = 0;
  node.bunrui = HB_MEISHI;
  
  MzConversionClause clause;
  clause.add(&node);
  node.post = lcmap(pre, LCMAP_KATAKANA | LCMAP_FULLWIDTH);
  clause.add(&node);

  result.clauses.push_back(clause);
}

void MzIme::MakeResultForSingle(MzConversionResult& result, Lattice& lattice) {
  FOOTMARK();
  result.clear();

  const size_t length = lattice.pre.size();

  // add other candidates
  MzConversionClause clause;
  assert(lattice.chunks.size());
  const LatticeChunk& chunk = lattice.chunks[0];
  for (size_t i = 0; i < chunk.size(); ++i) {
    if (chunk[i]->pre.size() == length) {
      // add a candidate of same size
      clause.add(chunk[i].get());
    }
  }

  std::wstring hiragana = lattice.pre;
  {
    LatticeNode node;
    node.pre = hiragana;
    node.post = hiragana;
    node.bunrui = HB_UNKNOWN;
    node.cost = 10;
    clause.add(&node);
  }

  {
    std::wstring katakana;
    katakana = lcmap(hiragana, LCMAP_FULLWIDTH | LCMAP_KATAKANA);

    LatticeNode node;
    node.pre = hiragana;
    node.post = katakana;
    node.bunrui = HB_UNKNOWN;
    node.cost = 10;
    clause.add(&node);
  }

  result.clauses.push_back(clause);

  result.sort();
} // MzIme::MakeResultForSingle

BOOL MzIme::ConvertMultiClause(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  MzConversionResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[comp.extra.iClause];
  if (!ConvertMultiClause(strHiragana, result)) {
    return FALSE;
  }

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
  return TRUE;
} // MzIme::ConvertMultiClause

BOOL MzIme::ConvertMultiClause(const std::wstring& strHiragana,
                               MzConversionResult& result)
{
  FOOTMARK();

#if 1
  if (!m_basic_dict.IsLoaded()) {
    return FALSE;
  }

  Lattice lattice;
  std::wstring pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  if (MakeLattice(lattice, pre)) {
    lattice.AddExtra();
    MakeResult(result, lattice);
  } else {
    lattice.AddExtra();
    MakeResult(result, pre);
  }
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

  return TRUE;
} // MzIme::ConvertMultiClause

BOOL MzIme::ConvertSingleClause(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  DWORD iClause = comp.extra.iClause;

  MzConversionResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[iClause];
  if (!ConvertSingleClause(strHiragana, result)) {
    return FALSE;
  }

  result.clauses.resize(1);
  MzConversionClause& clause = result.clauses[0];
  comp.SetClauseCompString(iClause, clause.candidates[0].converted);
  comp.SetClauseCompHiragana(iClause, clause.candidates[0].hiragana, bRoman);

  // setting cand
  LogCandList cand_list;
  for (size_t i = 0; i < clause.candidates.size(); ++i) {
    MzConversionCandidate& cand = clause.candidates[i];
    cand_list.cand_strs.push_back(cand.converted);
  }
  cand.cand_lists[iClause] = cand_list;
  cand.iClause = iClause;

  comp.extra.iClause = iClause;

  return TRUE;
} // MzIme::ConvertSingleClause

BOOL MzIme::ConvertSingleClause(const std::wstring& strHiragana,
                                MzConversionResult& result)
{
  FOOTMARK();
  result.clear();

#if 1
  Lattice lattice;
  std::wstring pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  MakeLatticeForSingle(lattice, pre);
  lattice.AddExtra();
  MakeResultForSingle(result, lattice);
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
  return TRUE;
} // MzIme::ConvertSingleClause

BOOL MzIme::StretchClauseLeft(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();

  DWORD iClause = comp.extra.iClause;

  std::wstring str1 = comp.extra.hiragana_clauses[iClause];
  if (str1.empty()) return FALSE;

  wchar_t ch = str1[str1.size() - 1];
  str1.resize(str1.size() - 1);

  BOOL bSplitted = FALSE;
  std::wstring str2;
  if (iClause + 1 < comp.GetClauseCount()) {
    str2 = ch + comp.extra.hiragana_clauses[iClause + 1];
  } else {
    bSplitted = TRUE;
    str2 += ch;
  }

  MzConversionResult result1, result2;
  if (!ConvertSingleClause(str1, result1)) {
    return FALSE;
  }

  if (!ConvertSingleClause(str2, result2)) {
    return FALSE;
  }

  if (bSplitted) {
    comp.comp_clause.push_back((DWORD)comp.comp_str.size());
    comp.extra.hiragana_clauses.push_back(L"");
    comp.extra.typing_clauses.push_back(L"");
    comp.extra.comp_str_clauses.push_back(L"");
  }

  MzConversionClause& clause2 = result2.clauses[0];
  comp.SetClauseCompString(iClause + 1, clause2.candidates[0].converted);
  comp.SetClauseCompHiragana(iClause + 1, clause2.candidates[0].hiragana, bRoman);
  {
    LogCandList cand_list;
    for (size_t i = 0; i < clause2.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause2.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists[iClause + 1] = cand_list;
  }

  MzConversionClause& clause1 = result1.clauses[0];
  comp.SetClauseCompString(iClause, clause1.candidates[0].converted);
  comp.SetClauseCompHiragana(iClause, clause1.candidates[0].hiragana, bRoman);
  {
    LogCandList cand_list;
    for (size_t i = 0; i < clause1.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause1.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists[iClause] = cand_list;
  }

  cand.iClause = iClause;
  comp.extra.iClause = iClause;

  return TRUE;
} // MzIme::StretchClauseLeft

BOOL MzIme::StretchClauseRight(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();

  DWORD iClause = comp.extra.iClause;

  std::wstring str1 = comp.extra.hiragana_clauses[iClause];
  if (iClause == comp.GetClauseCount() - 1) return FALSE;

  std::wstring str2 = comp.extra.hiragana_clauses[iClause + 1];
  if (str2.empty()) return FALSE;

  wchar_t ch = str2[0];
  str1 += ch;

  BOOL bJoined = FALSE;
  if (str2.size() <= 1) {
    bJoined = TRUE;
  }
  str2.resize(str2.size() - 1);

  MzConversionResult result1, result2;
  if (!ConvertSingleClause(str1, result1)) {
    return FALSE;
  }

  if (str2.size() && !ConvertSingleClause(str2, result2)) {
    return FALSE;
  }

  if (bJoined) {
    comp.extra.hiragana_clauses.resize(iClause);
    comp.extra.typing_clauses.resize(iClause);
    comp.extra.comp_str_clauses.resize(iClause);
  } else {
    MzConversionClause& clause2 = result2.clauses[0];
    comp.SetClauseCompString(iClause + 1, clause2.candidates[0].converted);
    comp.SetClauseCompHiragana(iClause + 1, clause2.candidates[0].hiragana, bRoman);
    {
      LogCandList cand_list;
      for (size_t i = 0; i < clause2.candidates.size(); ++i) {
        MzConversionCandidate& cand = clause2.candidates[i];
        cand_list.cand_strs.push_back(cand.converted);
      }
      cand.cand_lists[iClause + 1] = cand_list;
    }
  }

  MzConversionClause& clause1 = result1.clauses[0];
  comp.SetClauseCompString(iClause, clause1.candidates[0].converted);
  comp.SetClauseCompHiragana(iClause, clause1.candidates[0].hiragana, bRoman);
  {
    LogCandList cand_list;
    for (size_t i = 0; i < clause1.candidates.size(); ++i) {
      MzConversionCandidate& cand = clause1.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists[iClause] = cand_list;
  }

  cand.iClause = iClause;
  comp.extra.iClause = iClause;

  return TRUE;
} // MzIme::StretchClauseRight

//////////////////////////////////////////////////////////////////////////////
