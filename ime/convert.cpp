// convert.cpp --- mzimeja kana kanji conversion
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#include "../mzimeja.h"

const DWORD c_dwMilliseconds = 8000;

// hiragana table
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

// convert the classification (bunrui) to string (for debugging)
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
    L"HB_SYMBOL"
  };
  return s_array[index];
} // BunruiToString

// �i���̘A���R�X�g
static int
CandConnectCost(HinshiBunrui bunrui1, HinshiBunrui bunrui2)
{
  if (bunrui2 == HB_PERIOD || bunrui2 == HB_COMMA) return 0;
  if (bunrui2 == HB_TAIL) return 0;
  if (bunrui1 == HB_SYMBOL || bunrui2 == HB_SYMBOL) return 0;
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
  case HB_MEISHI: // ����
    switch (bunrui2) {
    case HB_MEISHI:
      return 10;
    case HB_SETTOUJI:
      return 10;
    case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
      return 5;
    default:
      break;
    }
    break;
  case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI: // ���`�e���A�Ȍ`�e��
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
    default:
      break;
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

// �i���̘A���\��
static BOOL
IsNodeConnectable(const LatticeNode& node1, const LatticeNode& node2) {
  if (node2.bunrui == HB_PERIOD || node2.bunrui == HB_COMMA) return TRUE;
  if (node2.bunrui == HB_TAIL) return TRUE;
  if (node1.bunrui == HB_SYMBOL || node2.bunrui == HB_SYMBOL) return TRUE;
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
  case HB_MEISHI: // ����
    switch (node2.bunrui) {
    case HB_SETTOUJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI: // ���`�e���A�Ȍ`�e��
    switch (node1.katsuyou) {
    case MIZEN_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[���R�`�ɘA��]")) {
          if (node2.pre[0] == L'��' || node2.pre == L"��") {
            return TRUE;
          }
        }
      }
      return FALSE;
    case RENYOU_KEI:
      switch (node2.bunrui) {
      case HB_JODOUSHI:
        if (node2.HasTag(L"[�A�p�`�ɘA��]")) {
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
        if (node2.HasTag(L"[�I�~�`�ɘA��]")) {
          return TRUE;
        }
        if (node2.HasTag(L"[��X�̌�]")) {
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
        if (node2.pre == L"��" || node2.pre == L"�ǂ�" || node2.pre == L"��") {
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
  case HB_RENTAISHI:  // �A�̎�
    switch (node2.bunrui) {
    case HB_KANDOUSHI: case HB_JODOUSHI: case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_FUKUSHI:  // ����
    switch (node2.bunrui) {
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
    case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_SETSUZOKUSHI: // �ڑ���
    switch (node2.bunrui) {
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI: case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_KANDOUSHI:  // ������
    switch (node2.bunrui) {
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI: case HB_SETSUBIJI: case HB_JODOUSHI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
    // �I�����ȊO�̏���
    switch (node2.bunrui) {
    case HB_SETSUBIJI:
      return FALSE;
    default:
      return TRUE;
    }
    break;
  case HB_SHUU_JOSHI: // �I����
    switch (node2.bunrui) {
    case HB_MEISHI: case HB_SETTOUJI: case HB_SHUU_JOSHI:
      return TRUE;
    default:
      return FALSE;
    }
    break;
  case HB_JODOUSHI: // ������
    switch (node1.katsuyou) {
    case MIZEN_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[���R�`�ɘA��]")) {
          return TRUE;
        }
      }
      return FALSE;
    case RENYOU_KEI:
      switch (node2.bunrui) {
      case HB_JODOUSHI:
        if (node2.HasTag(L"[�A�p�`�ɘA��]")) {
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
        if (node2.HasTag(L"[�I�~�`�ɘA��]")) {
          return TRUE;
        }
        if (node2.HasTag(L"[��X�̌�]")) {
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
        if (node2.pre == L"��" || node2.pre == L"�ǂ�" || node2.pre == L"��") {
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
    // ����
    switch (node1.katsuyou) {
    case MIZEN_KEI:
      if (node2.bunrui == HB_JODOUSHI) {
        if (node2.HasTag(L"[���R�`�ɘA��]")) {
          return TRUE;
        }
      }
      return FALSE;
    case RENYOU_KEI:
      switch (node2.bunrui) {
      case HB_JODOUSHI:
        if (node2.HasTag(L"[�A�p�`�ɘA��]")) {
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
        if (node2.HasTag(L"[�I�~�`�ɘA��]")) {
          return TRUE;
        }
        if (node2.HasTag(L"[��X�̌�]")) {
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
        if (node2.pre == L"��" || node2.pre == L"�ǂ�" || node2.pre == L"��") {
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
  case HB_SETTOUJI:   // �ړ���
    switch (node2.bunrui) {
    case HB_MEISHI:
      return TRUE;
    default:
      return FALSE;
    }
    break;
  case HB_SETSUBIJI:  // �ڔ���
    switch (node2.bunrui) {
    case HB_SETTOUJI:
      return FALSE;
    default:
      break;
    }
    break;
  case HB_COMMA: case HB_PERIOD:  // �A�B
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
// MzConvResult, MzConvClause etc.

void MzConvClause::add(const LatticeNode *node) {
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
    MzConvCandidate cand;
    cand.hiragana = node->pre;
    cand.converted = node->post;
    cand.cost = node->cost;
    cand.bunruis.insert(node->bunrui);
    cand.tags = node->tags;
    candidates.push_back(cand);
  }
}

static inline bool CandidateCompare(
  const MzConvCandidate& cand1, const MzConvCandidate& cand2)
{
  FOOTMARK();
  return cand1.cost < cand2.cost;
}

void MzConvClause::sort() {
  FOOTMARK();
  std::sort(candidates.begin(), candidates.end(), CandidateCompare);
}

void MzConvResult::sort() {
  FOOTMARK();

  for (size_t i = 1; i < clauses.size(); ++i) {
    for (size_t iCand1 = 0; iCand1 < clauses[i - 1].candidates.size(); ++iCand1) {
      for (size_t iCand2 = 0; iCand2 < clauses[i].candidates.size(); ++iCand2) {
        MzConvCandidate& cand1 = clauses[i - 1].candidates[iCand1];
        MzConvCandidate& cand2 = clauses[i].candidates[iCand2];
        int min_cost = 0x7FFF;
        std::set<HinshiBunrui>::iterator it1, end1 = cand1.bunruis.end();
        std::set<HinshiBunrui>::iterator it2, end2 = cand2.bunruis.end();
        for (it1 = cand1.bunruis.begin(); it1 != end1; ++it1)  {
          for (it2 = cand2.bunruis.begin(); it2 != end2; ++it2)  {
            int cost = CandConnectCost(*it1, *it2);
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
  if (bunrui == HB_SYMBOL) ret += 120;
  if (tags.size() != 0) {
    if (HasTag(L"[��W��]")) ret += 100;
    if (HasTag(L"[�s�ސT]")) ret += 50;
    if (HasTag(L"[�l��]")) ret += 30;
    else if (HasTag(L"[�w��]")) ret += 30;
    else if (HasTag(L"[�n��]")) ret += 30;
  }
  return ret;
}

bool LatticeNode::IsDoushi() const {
  switch (bunrui) {
  case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
  case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
    return true;
  default:
    break;
  }
  return false;
}

bool LatticeNode::IsJodoushi() const {
  switch (bunrui) {
  case HB_JODOUSHI:
  case HB_MIZEN_JODOUSHI: case HB_RENYOU_JODOUSHI:
  case HB_SHUUSHI_JODOUSHI: case HB_RENTAI_JODOUSHI:
  case HB_KATEI_JODOUSHI: case HB_MEIREI_JODOUSHI:
    return true;
  default:
    break;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// Lattice

void Lattice::AddExtra() {
  FOOTMARK();
  if (pre == L"���傤") {   // today
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    WCHAR sz[32];

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

    wsprintfW(sz, L"%u�N%u��%u��", st.wYear, st.wMonth, st.wDay);
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
  if (pre == L"���Ƃ�") {   // this year
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    WCHAR sz[32];

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

    wsprintfW(sz, L"%u�N", st.wYear);
    fields[2] = sz;
    DoFields(0, fields);
    return;
  }
  if (pre == L"������" || pre == L"��������") {   // now time
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    WCHAR sz[32];

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

    if (pre == L"��������") {
      fields[2] = L"������";
      DoFields(0, fields);
      fields[2] = L"����";
      DoFields(0, fields);
    }

    wsprintfW(sz, L"%u��%u��%u�b", st.wHour, st.wMinute, st.wSecond);
    fields[2] = sz;
    DoFields(0, fields);

    wsprintfW(sz, L"%02u��%02u��%02u�b", st.wHour, st.wMinute, st.wSecond);
    fields[2] = sz;
    DoFields(0, fields);

    if (st.wHour >= 12) {
      wsprintfW(sz, L"�ߌ�%u��%u��%u�b", st.wHour - 12, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);

      wsprintfW(sz, L"�ߌ�%02u��%02u��%02u�b", st.wHour - 12, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);
    } else {
      wsprintfW(sz, L"�ߑO%u��%u��%u�b", st.wHour, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);

      wsprintfW(sz, L"�ߑO%02u��%02u��%02u�b", st.wHour, st.wMinute, st.wSecond);
      fields[2] = sz;
      DoFields(0, fields);
    }

    wsprintfW(sz, L"%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
    fields[2] = sz;
    DoFields(0, fields);
    return;
  }
  if (pre == L"���Ԃ�") {   // myself
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
  if (pre == L"������") {   // kakko (parens, brackets, braces, ...)
    WStrings items;
    unboost::split(items, TheIME.LoadSTR(100), unboost::is_any_of(L"\t"));

    WStrings fields(4);
    fields[0] = pre;
    fields[1].assign(1, MAKEWORD(HB_SYMBOL, 0));
    for (size_t i = 0; i < items.size(); ++i) {
      fields[2] = items[i];
      DoFields(0, fields);
    }
    return;
  }
  // symbols
  static const wchar_t *s_words[] = {
    L"������", L"��������", L"��������", L"���񂩂�",
    L"������", L"������", L"�܂�", L"�ق�", L"�Ђ�",
    L"�Ă�", L"����", L"�ӂƂ�����",
    L"����", L"���ĂЂ���", L"���Ă݂�", L"�Ђ��肤��",
    L"�Ђ��肵��", L"�ӂƂ킭", L"�ق��킭", L"�܂�Ȃ�",
    L"�݂�����", L"�݂�����", L"�悱", L"�悱����", L"�悱����",
    L"���Ȃ�", L"�₶�邵", L"���肵��",
    L"����", L"����", L"�Ђ���", L"�݂�"
  };
  for (size_t i = 0; i < _countof(s_words); ++i) {
    if (pre == s_words[i]) {
      WStrings items;
      WCHAR *pch = TheIME.LoadSTR(101 + i);
      WStrings fields(4);
      fields[0] = pre;
      fields[1].assign(1, MAKEWORD(HB_SYMBOL, 0));
      int cost = 0;
      while (*pch) {
        fields[2].assign(1, *pch++);
        DoFields(0, fields, cost);
        ++cost;
      }
      return;
    }
  }
} // Lattice::AddExtra

BOOL Lattice::AddNodes(size_t index, const WCHAR *dict_data) {
  FOOTMARK();
  const size_t length = pre.size();
  assert(length);

  // separator
  std::wstring sep;
  sep += FIELD_SEP;

  WStrings fields, records;
  for (; index < length; ++index) {
    if (refs[index] == 0) continue;

    // �B�B�B
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
        fields[2] += L'�d';
        break;
      case 3:
        fields[2] += L'�c';
        break;
      default:
        fields[2] = fields[0];
        break;
      }
      DoFields(saved, fields);
      --index;
      continue;
    }
    // �E�E�E
    if (pre[index] == L'�E') {
      size_t saved = index;
      do {
        ++index;
      } while (pre[index] == L'�E');

      fields.resize(4);
      fields[0] = pre.substr(saved, index - saved);
      fields[1] = MAKEWORD(HB_SYMBOL, 0);
      switch (index - saved) {
      case 2:
        fields[2] += L'�d';
        break;
      case 3:
        fields[2] += L'�c';
        break;
      default:
        fields[2] = fields[0];
        break;
      }
      DoFields(saved, fields);
      --index;
      continue;
    }
    // �A�A�A
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
    // ��
    if ((pre[index] == L'-' || pre[index] == L'�|' || pre[index] == L'�[') &&
        (pre[index + 1] == L'>' || pre[index + 1] == L'��'))
    {
      fields.resize(4);
      fields[0] = pre.substr(index, 2);
      fields[1] = MAKEWORD(HB_SYMBOL, 0);
      fields[2] += L'��';
      DoFields(index, fields);
      ++index;
      continue;
    }
    // ��
    if ((pre[index] == L'<' || pre[index] == L'��') &&
        (pre[index + 1] == L'-' || pre[index + 1] == L'�|' ||
         pre[index + 1] == L'�['))
    {
      fields.resize(4);
      fields[0] = pre.substr(index, 2);
      fields[1] = MAKEWORD(HB_SYMBOL, 0);
      fields[2] += L'��';
      DoFields(index, fields);
      ++index;
      continue;
    }
    // other non-hiragana
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

      if (are_all_chars_numeric(fields[0])) {
        fields[2] = convert_to_kansuuji(fields[0]);
        DoMeishi(saved, fields);
        fields[2] = convert_to_kansuuji_brief(fields[0]);
        DoMeishi(saved, fields);
        fields[2] = convert_to_kansuuji_formal(fields[0]);
        DoMeishi(saved, fields);
      }

      --index;
      continue;
    }

    // scan dictionary
    size_t count = ScanDict(records, dict_data, pre[index]);
    DebugPrintW(L"ScanDict(%c) count: %d\n", pre[index], count);

    // store data for each record
    for (size_t k = 0; k < records.size(); ++k) {
      const std::wstring& record = records[k];
      unboost::split(fields, record, unboost::is_any_of(sep));
      DoFields(index, fields);
    }

    // special cases
    switch (pre[index]) {
    case L'��': case L'��': case L'��':   // KURU
      fields.resize(4);
      fields[1].push_back(MAKEWORD(HB_KAHEN_DOUSHI, GYOU_KA));
      DoKahenDoushi(index, fields);
      break;
    case L'��': case L'��': case L'��': case L'��':   // SURU
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

// for deleting the nodes of different size
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

  // scan dictionary
  WStrings fields, records;
  size_t count = ScanDict(records, dict_data, pre[0]);
  DebugPrintW(L"ScanDict(%c) count: %d\n", pre[0], count);

  // store data for each record
  for (size_t k = 0; k < records.size(); ++k) {
    const std::wstring& record = records[k];
    unboost::split(fields, record, unboost::is_any_of(sep));
    DoFields(0, fields);
  }

  // delete the nodes of different size
  DeleteDifferentSizeNode del(pre.size());
  for (size_t i = 0; i < chunks[0].size(); ++i) {
    std::remove_if(chunks[0].begin(), chunks[0].end(), del);
  }

  return !chunks[0].empty();
}

void Lattice::UpdateRefs() {
  FOOTMARK();
  const size_t length = pre.size();

  // initialize the reference counts
  refs.assign(length + 1, 0);
  refs[0] = 1;

  // update the reference counts
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
    head = unboost::make_shared<LatticeNode>(node);
  }
  // add tail
  {
    LatticeNode node;
    node.bunrui = HB_TAIL;
    chunks[length].clear();
    chunks[length].push_back(unboost::make_shared<LatticeNode>(node));
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
  // clear the branch links and the linked counts
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
  // add the undefined words on failure of conversion
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
  return 0;   // not found
} // Lattice::GetLastLinkedIndex

void Lattice::DoIkeiyoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();

  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  // get the right substring
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_IKEIYOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // ���R�`
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (fields[0].empty() || str.empty()) break;
    wchar_t ch0 = fields[0][fields[0].size() - 1];
    wchar_t ch1 = TheIME.m_consonant_map[ch0];
    std::wstring addition;
    switch (TheIME.m_vowel_map[ch0]) {
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
      if (str.empty() || str.substr(0, addition.size()) != addition) break;
      chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
      refs[index + fields[0].size() + addition.size()]++;
      break;
    case L'��':
      if (str.empty() || str.substr(0, 2) != L"�イ") break;
      node.pre = fields[0] + L"�イ";
      node.post = fields[2] + L"�イ";
      chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
      refs[index + fields[0].size() + 2]++;
    default:
      break;
    }
  } while(0);

  // �I�~�`
  node.katsuyou = SHUUSHI_KEI;
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �A�̌`
  node.katsuyou = RENTAI_KEI;
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // �����`
  node.bunrui = HB_MEISHI;
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // (���`�e���̌ꊲ)+"������"
  if (str.size() >= 2 && str[0] == L'��' && str[1] == L'��') {
    WStrings new_fields = fields;
    new_fields[0] = fields[0] + L"����";
    new_fields[2] = fields[2] + L"����";
    DoNakeiyoushi(index, new_fields);
  }

  // �u�ɁB�v�u���B�v�Ȃ�
  if (str.size()) {
    switch (str[0]) {
    case L'�B': case L'�A': case L'�C': case L'�D': case 0:
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
  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  // get the right substring
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_NAKEIYOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // ���R�`
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  do {
    if (str.empty() || str[0] != L'��') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �A�̌`
  do {
    if (str.empty() || str[0] != L'��') break;
    node.katsuyou = RENTAI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (str.empty() || str.substr(0, 2) != L"�Ȃ�") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"�Ȃ�";
    node.post = fields[2] + L"�Ȃ�";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // �����`
  node.bunrui = HB_MEISHI;
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �u���ꂢ�B�v�u�Â��B�v�Ȃ�
  switch (str[0]) {
  case L'�B': case L'�A': case L'�C': case L'�D': case 0:
    DoMeishi(index, fields);
    break;
  }
} // Lattice::DoNakeiyoushi

void Lattice::DoGodanDoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  // get the right substring
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

  // ���R�`
  do {
    node.katsuyou = MIZEN_KEI;
    if (node.gyou == GYOU_A) {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
      chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
      refs[index + fields[0].size() + 1]++;
    } else {
      wchar_t ch = s_hiragana_table[node.gyou][DAN_A];
      if (str.empty() || str[0] != ch) break;
      node.pre = fields[0] + ch;
      node.post = fields[2] + ch;
      chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
      refs[index + fields[0].size() + 1]++;
    }
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
    if (str.empty() || str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (type == 0) break;
    wchar_t ch;
    switch (type) {
    case 1:   ch = L'��'; break;
    case 2:   ch = L'��'; break;
    case 3:   ch = L'��'; break;
    }
    if (str.empty() || str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  // �A�̌`
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_U];
    if (str.empty() || str[0] != ch) break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
    node.katsuyou = RENTAI_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  // ���ߌ`
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_E];
    if (str.empty() || str[0] != ch) break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
    node.katsuyou = MEIREI_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �����`
  node.bunrui = HB_MEISHI;
  do {
    wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
    if (str.empty() || str[0] != ch) break;
    node.pre = fields[0] + ch;
    node.post = fields[2] + ch;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �u����(�ܒi)�v���u������(��i)�v�A
  // �u����(�ܒi)�v���u������(��i)�v�Ȃ�
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
  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  // get the right substring
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_ICHIDAN_DOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // ���R�`
  // �A�p�`
  do {
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size()]++;
    node.katsuyou = RENYOU_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size()]++;
  } while(0);

  // �I�~�`
  // �A�̌`
  do {
    if (str.empty() || str[0] != L'��') break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
    node.katsuyou = RENTAI_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (str.empty() || str[0] != L'��') break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // ���ߌ`
  node.katsuyou = MEIREI_KEI;
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �����`
  node.bunrui = HB_MEISHI;
  do {
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size()]++;
  } while(0);
} // Lattice::DoIchidanDoushi

void Lattice::DoKahenDoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  // get the right substring
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_KAHEN_DOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  // ���R�`
  do {
    if (str.empty() || str[0] != L'��') break;
    node.katsuyou = MIZEN_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �A�p�`
  do {
    if (str.empty() || str[0] != L'��') break;
    node.katsuyou = RENYOU_KEI;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  // �A�p�`
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
    node.katsuyou = RENYOU_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // ����`
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // ���ߌ`
  do {
    if (str.empty() || str.substr(0, 2) != L"����") break;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0] + L"����";
    node.post = fields[2] + L"����";
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // �����`
  node.bunrui = HB_MEISHI;
  do {
    if (str.empty() || str[0] != L'��') break;
    node.pre = fields[0] + L'��';
    node.post = fields[2] + L'��';
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
} // Lattice::DoKahenDoushi

void Lattice::DoSahenDoushi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());
  size_t length = fields[0].size();
  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  // get the right substring
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_SAHEN_DOUSHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  WORD w = fields[1][0];
  node.gyou = (Gyou)HIBYTE(w);

  // ���R�`
  node.katsuyou = MIZEN_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �A�p�`
  node.katsuyou = RENYOU_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // �I�~�`
  // �A�p�`
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    node.katsuyou = SHUUSHI_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;

    node.katsuyou = RENYOU_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    } else {
      if (str.empty() || str[0] != L'��') break;
      node.pre = fields[0] + L'��';
      node.post = fields[2] + L'��';
    }
    node.katsuyou = SHUUSHI_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 1]++;
  } while(0);

  // ����`
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    node.katsuyou = KATEI_KEI;
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);

  // ���ߌ`
  node.katsuyou = MEIREI_KEI;
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
  do {
    if (node.gyou == GYOU_ZA) {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    } else {
      if (str.empty() || str.substr(0, 2) != L"����") break;
      node.pre = fields[0] + L"����";
      node.post = fields[2] + L"����";
    }
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + fields[0].size() + 2]++;
  } while(0);
} // Lattice::DoSahenDoushi

void Lattice::DoMeishi(size_t index, const WStrings& fields) {
  FOOTMARK();
  assert(fields.size() == 4);
  assert(fields[0].size());

  size_t length = fields[0].size();
  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  // get the right substring
  std::wstring str = pre.substr(index + length);

  LatticeNode node;
  node.bunrui = HB_MEISHI;
  node.tags = fields[3];
  node.cost = node.CalcCost();

  if (pre.substr(index, length) == fields[0]) {
    if (node.HasTag(L"[���A��]")) {
      // animals or plants can be written in katakana
      node.pre = fields[0];
      node.post = lcmap(fields[0], LCMAP_KATAKANA | LCMAP_FULLWIDTH);
      chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
      refs[index + length]++;

      node.cost += 30;
      node.pre = fields[0];
      node.post = fields[2];
      chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
      refs[index + length]++;
    } else {
      node.pre = fields[0];
      node.post = fields[2];
      chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
      refs[index + length]++;
    }
  }

  // �����{�u���ۂ��v�Ō`�e����
  if (str.size() >= 2 && str[0] == L'��' && str[1] == L'��') {
    WStrings new_fields = fields;
    new_fields[0] += L"����";
    new_fields[2] += L"����";
    DoIkeiyoushi(index, new_fields);
  }
} // Lattice::DoMeishi

void Lattice::DoFields(size_t index, const WStrings& fields, int cost/* = 0*/) {
  assert(fields.size() == 4);
  const size_t length = fields[0].size();
  // boundary check
  if (index + length > pre.size()) {
    return;
  }
  // check text matching
  if (pre.substr(index, length) != fields[0]) {
    return;
  }
  DebugPrintW(L"DoFields: %s\n", fields[0].c_str());

  // initialize the node
  LatticeNode node;
  WORD w = fields[1][0];
  node.bunrui = (HinshiBunrui)LOBYTE(w);
  node.gyou = (Gyou)HIBYTE(w);
  node.tags = fields[3];
  node.cost = node.CalcCost() + cost;

  // add new entries by node classification (BUNRUI)
  switch (node.bunrui) {
  case HB_MEISHI:
    DoMeishi(index, fields);
    break;
  case HB_PERIOD: case HB_COMMA: case HB_SYMBOL:
  case HB_RENTAISHI: case HB_FUKUSHI:
  case HB_SETSUZOKUSHI: case HB_KANDOUSHI:
  case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
  case HB_FUKU_JOSHI: case HB_SHUU_JOSHI:
  case HB_KANGO: case HB_SETTOUJI: case HB_SETSUBIJI:
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
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
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + length]++;
    break;
  case HB_RENYOU_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = RENYOU_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + length]++;
    break;
  case HB_SHUUSHI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + length]++;
    break;
  case HB_RENTAI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = RENTAI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + length]++;
    break;
  case HB_KATEI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
    refs[index + length]++;
    break;
  case HB_MEIREI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared<LatticeNode>(node));
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
  const DWORD c_retry_count = 32;

  // failure if the dictionary not loaded
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

  // failure if the dictionary not loaded
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

void MzIme::MakeResult(MzConvResult& result, Lattice& lattice) {
  FOOTMARK();
  result.clear();

  // 2���ߍŒ���v�@�E��
  const size_t length = lattice.pre.size();
  LatticeNodePtr node1 = lattice.head;
  LatticeNodePtr tail = lattice.chunks[length][0];
  while (node1 != tail) {
    size_t kb1 = 0, max_len = 0, max_len1 = 0;
    for (size_t ib1 = 0; ib1 < node1->branches.size(); ++ib1) {
      LatticeNodePtr& node2 = node1->branches[ib1];
      if (node2->branches.empty()) {
        size_t len = node2->pre.size();
        // (doushi or jodoushi) + jodoushi
        if ((node1->IsDoushi() || node1->IsJodoushi()) && node2->IsJodoushi()) {
          ++len;
          node2->cost -= 80;
        }
        // jodoushi + shuu joshi
        if (node1->IsJodoushi() && node2->bunrui == HB_SHUU_JOSHI) {
          ++len;
          node2->cost -= 30;
        }
        // suushi + (suushi or number unit)
        if (node1->HasTag(L"[����]")) {
          if (node2->HasTag(L"[����]") || node2->HasTag(L"[���P��]")) {
            ++len;
            node2->cost -= 100;
          }
        }
        if (max_len < len) {
          max_len1 = node2->pre.size();
          max_len = len;
          kb1 = ib1;
        }
      } else {
        for (size_t ib2 = 0; ib2 < node2->branches.size(); ++ib2) {
          LatticeNodePtr& node3 = node2->branches[ib2];
          size_t len = node2->pre.size() + node3->pre.size();
          // (doushi or jodoushi) + jodoushi
          if ((node1->IsDoushi() || node1->IsJodoushi()) && node2->IsJodoushi()) {
            ++len;
            node2->cost -= 80;
          } else {
            if ((node2->IsDoushi() || node2->IsJodoushi()) && node3->IsJodoushi()) {
              ++len;
              node2->cost -= 80;
            }
          }
          // jodoushu + shuu joshi
          if (node1->IsJodoushi() && node2->bunrui == HB_SHUU_JOSHI) {
            ++len;
            node2->cost -= 30;
          } else {
            if (node2->IsJodoushi() && node3->bunrui == HB_SHUU_JOSHI) {
              ++len;
              node2->cost -= 30;
            }
          }
          // suushi + (suushi or number unit)
          if (node1->HasTag(L"[����]")) {
            if (node2->HasTag(L"[����]") || node2->HasTag(L"[���P��]")) {
              ++len;
              node2->cost -= 100;
            }
          } else {
            if (node2->HasTag(L"[����]")) {
              if (node3->HasTag(L"[����]") || node3->HasTag(L"[���P��]")) {
                ++len;
                node2->cost -= 100;
              }
            }
          }
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
      MzConvClause clause;
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
    MzConvClause& clause = result.clauses[iClause];

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

      // add hiragana
      node.pre = hiragana;
      node.post = hiragana;
      clause.add(&node);

      // add katakana
      std::wstring katakana;
      katakana = lcmap(hiragana, LCMAP_FULLWIDTH | LCMAP_KATAKANA);
      node.post = katakana;
      clause.add(&node);
    } else {
      LatticeNode node;
      node.bunrui = HB_UNKNOWN;
      node.cost = 10;
      node.pre = hiragana;

      // add the lowercase and fullwidth
      node.post = lcmap(hiragana, LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
      clause.add(&node);

      // add the uppercase and fullwidth
      node.post = lcmap(hiragana, LCMAP_UPPERCASE | LCMAP_FULLWIDTH);
      clause.add(&node);

      // add the capital and fullwidth
      node.post = node.post[0] + lcmap(hiragana.substr(1), LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
      clause.add(&node);

      // add the lowercase and halfwidth
      node.post = lcmap(hiragana, LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
      clause.add(&node);

      // add the uppercase and halfwidth
      node.post = lcmap(hiragana, LCMAP_UPPERCASE | LCMAP_HALFWIDTH);
      clause.add(&node);

      // add the capital and halfwidth
      node.post = node.post[0] + lcmap(hiragana.substr(1), LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
      clause.add(&node);
    }

    // go to the next clause
    index += size;
    ++iClause;
  }

  // sort by cost
  result.sort();
} // MzIme::MakeResult

void MzIme::MakeResultOnFailure(MzConvResult& result, const std::wstring& pre) {
  FOOTMARK();
  MzConvClause clause;
  result.clear();

  // initialize the node
  LatticeNode node;
  node.pre = pre;
  node.cost = 0;
  node.bunrui = HB_MEISHI;

  // hiragana
  node.post = pre;
  clause.add(&node);

  // katakana
  node.post = lcmap(pre, LCMAP_KATAKANA | LCMAP_FULLWIDTH);
  clause.add(&node);

  result.clauses.push_back(clause);
} // MzIme::MakeResultOnFailure

void MzIme::MakeResultForSingle(MzConvResult& result, Lattice& lattice) {
  FOOTMARK();
  result.clear();
  const size_t length = lattice.pre.size();

  // add other candidates
  MzConvClause clause;
  assert(lattice.chunks.size());
  const LatticeChunk& chunk = lattice.chunks[0];
  for (size_t i = 0; i < chunk.size(); ++i) {
    if (chunk[i]->pre.size() == length) {
      // add a candidate of same size
      clause.add(chunk[i].get());
    }
  }

  // initialize the node
  std::wstring hiragana = lattice.pre;
  LatticeNode node;
  node.pre = hiragana;
  node.bunrui = HB_UNKNOWN;
  node.cost = 10;

  // add hiragana
  node.post = hiragana;
  clause.add(&node);

  // add katakana
  node.post = lcmap(hiragana, LCMAP_FULLWIDTH | LCMAP_KATAKANA);
  clause.add(&node);

  // add the clause
  result.clauses.push_back(clause);
  assert(result.clauses[0].candidates.size());

  // sort by cost
  result.sort();
  assert(result.clauses[0].candidates.size());
} // MzIme::MakeResultForSingle

BOOL MzIme::ConvertMultiClause(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  MzConvResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[comp.extra.iClause];
  if (!ConvertMultiClause(strHiragana, result)) {
    return FALSE;
  }
  return StoreResult(result, comp, cand);
} // MzIme::ConvertMultiClause

BOOL MzIme::ConvertMultiClause(const std::wstring& strHiragana,
                               MzConvResult& result)
{
  FOOTMARK();

#if 1
  // failure if the dictionary not loaded
  if (!m_basic_dict.IsLoaded()) {
    return FALSE;
  }

  // make lattice and make result
  Lattice lattice;
  std::wstring pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  if (MakeLattice(lattice, pre)) {
    lattice.AddExtra();
    MakeResult(result, lattice);
  } else {
    lattice.AddExtra();
    MakeResultOnFailure(result, pre);
  }
#else
  // dummy sample
  WCHAR sz[64];
  result.clauses.clear();
  for (DWORD iClause = 0; iClause < 5; ++iClause) {
    MzConvClause clause;
    for (DWORD iCand = 0; iCand < 18; ++iCand) {
      MzConvCandidate cand;
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

  return TRUE;
} // MzIme::ConvertMultiClause

BOOL MzIme::ConvertSingleClause(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  DWORD iClause = comp.extra.iClause;

  // convert
  MzConvResult result;
  std::wstring strHiragana = comp.extra.hiragana_clauses[iClause];
  if (!ConvertSingleClause(strHiragana, result)) {
    return FALSE;
  }

  // setting composition
  result.clauses.resize(1);
  MzConvClause& clause = result.clauses[0];
  comp.SetClauseCompString(iClause, clause.candidates[0].converted);
  comp.SetClauseCompHiragana(iClause, clause.candidates[0].hiragana, bRoman);

  // setting cand
  LogCandList cand_list;
  for (size_t i = 0; i < clause.candidates.size(); ++i) {
    MzConvCandidate& cand = clause.candidates[i];
    cand_list.cand_strs.push_back(cand.converted);
  }
  cand.cand_lists[iClause] = cand_list;
  cand.iClause = iClause;

  comp.extra.iClause = iClause;

  return TRUE;
} // MzIme::ConvertSingleClause

BOOL MzIme::ConvertSingleClause(const std::wstring& strHiragana,
                                MzConvResult& result)
{
  FOOTMARK();
  result.clear();

#if 1
  // make lattice and make result
  Lattice lattice;
  std::wstring pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  MakeLatticeForSingle(lattice, pre);
  lattice.AddExtra();
  MakeResultForSingle(result, lattice);
#else
  // dummy sample
  MzConvCandidate cand;
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
  return TRUE;
} // MzIme::ConvertSingleClause

BOOL MzIme::StretchClauseLeft(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();

  DWORD iClause = comp.extra.iClause;

  // get the clause string
  std::wstring str1 = comp.extra.hiragana_clauses[iClause];
  if (str1.size() <= 1) return FALSE;

  // get the last character of this clause
  wchar_t ch = str1[str1.size() - 1];
  // shrink
  str1.resize(str1.size() - 1);

  // get the next clause string and add the character
  std::wstring str2;
  BOOL bSplitted = FALSE;
  if (iClause + 1 < comp.GetClauseCount()) {
    str2 = ch + comp.extra.hiragana_clauses[iClause + 1];
  } else {
    str2 += ch;
    bSplitted = TRUE;
  }

  // convert two clauses
  MzConvResult result1, result2;
  if (!ConvertSingleClause(str1, result1)) {
    return FALSE;
  }
  if (!ConvertSingleClause(str2, result2)) {
    return FALSE;
  }

  // add one clause if the clause was splitted
  if (bSplitted) {
    std::wstring str;
    comp.extra.hiragana_clauses.insert(
      comp.extra.hiragana_clauses.begin() + iClause + 1, str);
    comp.extra.comp_str_clauses.insert(
      comp.extra.comp_str_clauses.begin() + iClause + 1, str);
  }

  // seting composition
  MzConvClause& clause1 = result1.clauses[0];
  MzConvClause& clause2 = result2.clauses[0];
  comp.extra.hiragana_clauses[iClause] = str1;
  comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].converted;
  comp.extra.hiragana_clauses[iClause + 1] = str2;
  comp.extra.comp_str_clauses[iClause + 1] = clause2.candidates[0].converted;
  comp.UpdateFromExtra(bRoman);

  // seting the candidate list
  {
    LogCandList cand_list;
    for (size_t i = 0; i < clause1.candidates.size(); ++i) {
      MzConvCandidate& cand = clause1.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists[iClause] = cand_list;
  }
  {
    LogCandList cand_list;
    for (size_t i = 0; i < clause2.candidates.size(); ++i) {
      MzConvCandidate& cand = clause2.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    if (bSplitted) {
      cand.cand_lists.push_back(cand_list);
    } else {
      cand.cand_lists[iClause + 1] = cand_list;
    }
  }

  // set the current clause
  cand.iClause = iClause;
  comp.extra.iClause = iClause;
  // set the clause attributes
  comp.SetClauseAttr(iClause, ATTR_TARGET_CONVERTED);

  return TRUE;
} // MzIme::StretchClauseLeft

BOOL MzIme::StretchClauseRight(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();

  DWORD iClause = comp.extra.iClause;

  // get the current clause
  std::wstring str1 = comp.extra.hiragana_clauses[iClause];
  // we cannot stretch right if the clause was the right end
  if (iClause == comp.GetClauseCount() - 1) return FALSE;

  // get the next clause
  std::wstring str2 = comp.extra.hiragana_clauses[iClause + 1];
  if (str2.empty()) return FALSE;

  // get the first character of the second clause
  wchar_t ch = str2[0];
  // add the character to the first clause
  str1 += ch;
  if (str2.size() == 1) {
    str2.clear();
  } else {
    str2 = str2.substr(1);
  }

  // convert
  MzConvResult result1, result2;
  if (!ConvertSingleClause(str1, result1)) {
    return FALSE;
  }
  if (str2.size() && !ConvertSingleClause(str2, result2)) {
    return FALSE;
  }

  MzConvClause& clause1 = result1.clauses[0];

  // if the second clause was joined?
  if (str2.empty()) {
    // delete the joined clause
    comp.extra.hiragana_clauses.erase(
      comp.extra.hiragana_clauses.begin() + iClause + 1);
    comp.extra.comp_str_clauses.erase(
      comp.extra.comp_str_clauses.begin() + iClause + 1);
    comp.extra.hiragana_clauses[iClause] = str1;
    comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].converted;
  } else {
    // seting two clauses
    MzConvClause& clause2 = result2.clauses[0];
    comp.extra.hiragana_clauses[iClause] = str1;
    comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].converted;
    comp.extra.hiragana_clauses[iClause + 1] = str2;
    comp.extra.comp_str_clauses[iClause + 1] = clause2.candidates[0].converted;
  }
  // update composition by extra
  comp.UpdateFromExtra(bRoman);

  // seting the candidate list
  {
    LogCandList cand_list;
    for (size_t i = 0; i < clause1.candidates.size(); ++i) {
      MzConvCandidate& cand = clause1.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists[iClause] = cand_list;
  }
  if (str2.size()) {
    MzConvClause& clause2 = result2.clauses[0];
    LogCandList cand_list;
    for (size_t i = 0; i < clause2.candidates.size(); ++i) {
      MzConvCandidate& cand = clause2.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists[iClause + 1] = cand_list;
  }

  // set the current clause
  cand.iClause = iClause;
  comp.extra.iClause = iClause;
  // set the clause attributes
  comp.SetClauseAttr(iClause, ATTR_TARGET_CONVERTED);

  return TRUE;
} // MzIme::StretchClauseRight

inline bool is_sjis_lead(BYTE ch) {
  return (
    ((0x81 <= ch) && (ch <= 0x9F)) ||
    ((0xE0 <= ch) && (ch <= 0xEF))
  );
}

inline bool is_sjis_trail(BYTE ch) {
  return (
    ((0x40 <= ch) && (ch <= 0x7E)) ||
    ((0x80 <= ch) && (ch <= 0xFC))
  );
}

inline bool is_jis_byte(BYTE ch) {
  return ((0x21 <= ch) && (ch <= 0x7E));
}

inline bool is_jis_code(WORD w) {
  BYTE ch0 = BYTE(w >> 8);
  BYTE ch1 = BYTE(w);
  return (is_jis_byte(ch0) && is_jis_byte(ch1));
}

inline WORD jis2sjis(BYTE c0, BYTE c1) {
  if (c0 & 0x01) {
    c0 >>= 1;
    if (c0 < 0x2F) {
      c0 += 0x71;
    } else {
      c0 -= 0x4F;
    }
    if (c1 > 0x5F) {
      c1 += 0x20;
    } else {
      c1 += 0x1F;
    }
  } else {
    c0 >>= 1;
    if (c0 < 0x2F) {
      c0 += 0x70;
    } else {
      c0 -= 0x50;
    }
    c1 += 0x7E;
  }
  WORD sjis_code = WORD((c0 << 8) | c1);
  return sjis_code;
} // jis2sjis

inline WORD jis2sjis(WORD jis_code) {
    BYTE c0 = BYTE(jis_code >> 8);
    BYTE c1 = BYTE(jis_code);
    return jis2sjis(c0, c1);
}

inline bool is_sjis_code(WORD w) {
  return (
    is_sjis_lead(BYTE(w >> 8)) && is_sjis_trail(BYTE(w))
  );
}

inline WORD kuten_to_jis(const std::wstring& str) {
  if (str.size() != 5) return 0;
  std::wstring ku_bangou = str.substr(0, 3);
  std::wstring ten_bangou = str.substr(3, 2);
  WORD ku = WORD(wcstoul(ku_bangou.c_str(), NULL, 10));
  WORD ten = WORD(wcstoul(ten_bangou.c_str(), NULL, 10));
  WORD jis_code = (ku + 32) * 256 + ten + 32;
  return jis_code;
}

BOOL MzIme::ConvertCode(const std::wstring& strTyping,
                        MzConvResult& result)
{
  result.clauses.clear();
  MzConvClause clause;

  // initialize the node
  LatticeNode node;
  node.pre = strTyping;
  node.bunrui = HB_UNKNOWN;
  node.cost = 0;

  ULONG hex_code = wcstoul(strTyping.c_str(), NULL, 16);
  WCHAR szUnicode[2];
  szUnicode[0] = WCHAR(hex_code);
  szUnicode[1] = 0;

  // Unicode
  node.post = szUnicode;
  clause.add(&node);
  node.cost++;

  // Shift_JIS
  CHAR szSJIS[8];
  WORD wSJIS = WORD(hex_code);
  if (is_sjis_code(wSJIS)) {
    szSJIS[0] = HIBYTE(wSJIS);
    szSJIS[1] = LOBYTE(wSJIS);
    szSJIS[2] = 0;
    ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
    node.post = szUnicode;
    node.cost++;
    clause.add(&node);
  }

  // JIS
  if (is_jis_code(WORD(hex_code))) {
    wSJIS = jis2sjis(WORD(hex_code));
    if (is_sjis_code(wSJIS)) {
      szSJIS[0] = HIBYTE(wSJIS);
      szSJIS[1] = LOBYTE(wSJIS);
      szSJIS[2] = 0;
      ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
      node.post = szUnicode;
      node.cost++;
      clause.add(&node);
    }
  }

  // KUTEN code
  WORD wJIS = kuten_to_jis(strTyping);
  if (is_jis_code(wJIS)) {
    wSJIS = jis2sjis(wJIS);
    if (is_sjis_code(wSJIS)) {
      szSJIS[0] = HIBYTE(wSJIS);
      szSJIS[1] = LOBYTE(wSJIS);
      szSJIS[2] = 0;
      ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
      node.post = szUnicode;
      node.cost++;
      clause.add(&node);
    }
  }

  // original
  node.post = strTyping;
  node.cost++;
  clause.add(&node);

  result.clauses.push_back(clause);
  return TRUE;
} // MzIme::ConvertCode

BOOL MzIme::ConvertCode(LogCompStr& comp, LogCandInfo& cand) {
  FOOTMARK();
  MzConvResult result;
  std::wstring strTyping = comp.extra.typing_clauses[comp.extra.iClause];
  if (!ConvertCode(strTyping, result)) {
    return FALSE;
  }
  return StoreResult(result, comp, cand);
} // MzIme::ConvertCode

BOOL MzIme::StoreResult(
  const MzConvResult& result, LogCompStr& comp, LogCandInfo& cand)
{
  comp.comp_str.clear();
  comp.extra.clear();

  // setting composition
  comp.comp_clause.resize(result.clauses.size() + 1);
  for (size_t k = 0; k < result.clauses.size(); ++k) {
    const MzConvClause& clause = result.clauses[k];
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
      const MzConvCandidate& cand = clause.candidates[i];
      comp.comp_clause[k] = (DWORD)comp.comp_str.size();
      comp.extra.hiragana_clauses.push_back(cand.hiragana);
      std::wstring typing;
      typing = hiragana_to_typing(cand.hiragana);
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
    const MzConvClause& clause = result.clauses[k];
    LogCandList cand_list;
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
      const MzConvCandidate& cand = clause.candidates[i];
      cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists.push_back(cand_list);
  }
  cand.iClause = 0;
  return TRUE;
} // MzIme::StoreResult

//////////////////////////////////////////////////////////////////////////////