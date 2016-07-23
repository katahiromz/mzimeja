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

// 品詞の連結可能性
static BOOL
IsNodeConnectable(const LatticeNode& node1, const LatticeNode& node2) {
  if (node2.bunrui == HB_PERIOD || node2.bunrui == HB_COMMA) return TRUE;
  if (node1.bunrui == HB_HEAD || node2.bunrui == HB_TAIL) return TRUE;
  if (node1.bunrui == HB_TAIL || node2.bunrui == HB_HEAD) return FALSE;
  if (node1.bunrui == HB_SYMBOLS || node2.bunrui == HB_SYMBOLS) return TRUE;
  if (node1.bunrui == HB_UNKNOWN || node2.bunrui == HB_UNKNOWN) return TRUE;

  switch (node1.bunrui) {
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

  WCHAR sz[3] = {L'\n', ch, 0};
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
  pch3 = wcschr(pch2 + 1, L'\n');
  assert(pch3);
  str.assign(pch1 + 1, pch3);

  unboost::split(records, str, unboost::is_any_of(L"\n"));
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

inline bool entry_compare_pre(const DictEntry& e1, const DictEntry& e2) {
  return (e1.pre < e2.pre);
}

BOOL MzIme::LoadBasicDictFile(std::vector<DictEntry>& entries) {
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
      if (bunrui_str == L"名詞")            entry.bunrui = HB_MEISHI;
      else if (bunrui_str == L"副詞")       entry.bunrui = HB_FUKUSHI;
      else if (bunrui_str == L"漢語")       entry.bunrui = HB_KANGO;
      else continue;
    } else if (bunrui_str.size() == 3) {
      if (bunrui_str == L"連体詞")          entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"接続詞")     entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"感動詞")     entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"接頭辞")     entry.bunrui = HB_SETTOUJI;
      else if (bunrui_str == L"接尾辞")     entry.bunrui = HB_SETSUBIJI;
      else if (bunrui_str == L"格助詞")     entry.bunrui = HB_KAKU_JOSHI;
      else if (bunrui_str == L"副助詞")     entry.bunrui = HB_FUKU_JOSHI;
      else if (bunrui_str == L"終助詞")     entry.bunrui = HB_SHUU_JOSHI;
      else continue;
    } else if (bunrui_str.size() == 4) {
      if (bunrui_str == L"い形容詞")        entry.bunrui = HB_IKEIYOUSHI;
      else if (bunrui_str == L"な形容詞")   entry.bunrui = HB_NAKEIYOUSHI;
      else if (bunrui_str == L"五段動詞")   entry.bunrui = HB_GODAN_DOUSHI;
      else if (bunrui_str == L"一段動詞")   entry.bunrui = HB_ICHIDAN_DOUSHI;
      else if (bunrui_str == L"カ変動詞")   entry.bunrui = HB_KAHEN_DOUSHI;
      else if (bunrui_str == L"サ変動詞")   entry.bunrui = HB_SAHEN_DOUSHI;
      else if (bunrui_str == L"接続助詞")   entry.bunrui = HB_SETSUZOKU_JOSHI;
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

  // sort by preconversion string
  std::sort(entries.begin(), entries.end(), entry_compare_pre);
  return TRUE;  // success
} // MzIme::LoadBasicDictFile

BOOL MzIme::DeployDictData(
  ImageBase *data, SECURITY_ATTRIBUTES *psa,
  const std::vector<DictEntry>& entries)
{
  FOOTMARK();

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

  BOOL ret = FALSE;

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
      const DictEntry& entry = entries[i];
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
    #if 1
      FILE *fp = fopen("c:\\dictdata.txt", "wb");
      fwrite(pv, size, 1, fp);
      fclose(fp);
    #endif
    ::UnmapViewOfFile(pv);
    ret = TRUE; // success
  }

  // store the total size
  data->dwSharedDictDataSize = size;

  return ret;
} // MzIme::DeployDictData

//////////////////////////////////////////////////////////////////////////////

BOOL MzIme::LoadBasicDict() {
  FOOTMARK();
  if (IsBasicDictLoaded()) return TRUE;

  // get the base shared data
  BOOL ret = FALSE;
  ImageBase *data = LockImeBaseData();
  if (data == NULL) {
    assert(0);
    return ret; // failure
  }

  // create a security attributes
  SECURITY_ATTRIBUTES *psa = CreateSecurityAttributes();
  assert(psa);

  // is the signature valid?
  if (data->dwSignature == 0xDEADFACE) {
    // lock the dictionary
    DWORD dwWait = ::WaitForSingleObject(m_hDictLock, c_dwMilliseconds);
    if (dwWait == WAIT_OBJECT_0) {
      // is it already stored?
      if (data->dwSharedDictDataSize == 0) {
        // no data, so deploy dict data
        std::vector<DictEntry> entries;
        if (LoadBasicDictFile(entries)) {
          ret = DeployDictData(data, psa, entries);
        }
      } else {
        // open shared dict data
        m_hBasicDictData = ::CreateFileMappingW(INVALID_HANDLE_VALUE, psa,
          PAGE_READONLY, 0, data->dwSharedDictDataSize, L"mzimeja_basic_dict");
        if (m_hBasicDictData) {
          ret = TRUE; // success
        }
        assert(ret);
      }
      // unlock the dictionary
      ::ReleaseMutex(m_hDictLock);
    }
  }

  // release the security attributes
  FreeSecurityAttributes(psa);

  // unlock the base shared data
  UnlockImeBaseData(data);

  return ret;
} // MzIme::LoadBasicDict

BOOL MzIme::IsBasicDictLoaded() const {
  FOOTMARK();
  return m_hBasicDictData != NULL;
}

WCHAR *MzIme::LockBasicDict() {
  FOOTMARK();
  // get size
  DWORD dwSize = 0;
  ImageBase *data = LockImeBaseData();
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

void MzIme::UnlockBasicDict(WCHAR *data) {
  FOOTMARK();
  ::UnmapViewOfFile(data);
}

//////////////////////////////////////////////////////////////////////////////
// MzConversionResult, MzConversionClause etc.

void MzConversionClause::add(
  const std::wstring& pre, const std::wstring& post, int the_cost)
{
  bool matched = false;
  for (size_t i = 0; i < candidates.size(); ++i) {
    if (candidates[i].converted == post) {
      if (candidates[i].cost > the_cost)  {
        candidates[i].cost = the_cost;
      }
      matched = true;
      break;
    }
  }

  if (!matched) {
    MzConversionCandidate cand;
    cand.hiragana = pre;
    cand.converted = post;
    cand.cost = the_cost;
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
  std::sort(candidates.begin() + 1, candidates.end(), CandidateCompare);
}

void MzConversionResult::sort() {
  FOOTMARK();
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

void Lattice::AddNodes(size_t index, const WCHAR *dict_data) {
  FOOTMARK();
  const size_t length = pre.size();
  assert(length);

  WStrings fields, records;
  for (; index < length; ++index) {
    if (refs[index] == 0) continue;
    size_t count = ScanDict(records, dict_data, pre[index]);
    DebugPrintW(L"ScanDict(%c) count: %d\n", pre[index], count);
    for (size_t k = 0; k < records.size(); ++k) {
      const std::wstring& record = records[k];
      unboost::split(fields, record, unboost::is_any_of(L"\t"));
      DoFields(index, fields);
    }
  }
} // Lattice::AddNodes

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

void Lattice::AddComplement(size_t index) {
  FOOTMARK();
  const size_t length = pre.size();

  WStrings fields(4);
  fields[1].assign(1, MAKEWORD(HB_UNKNOWN, 0));
  //fields[3].clear();
  for (size_t count = 1; count <= 4; ++count) {
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
  do {
    if (str.size() && str[0] != L'し') {
      node.pre = fields[0] + L'し';
      node.post = fields[2] + L'し';
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + fields[0].size() + 1]++;
    } else {
      node.pre = fields[0];
      node.post = fields[2];
      chunks[index].push_back(unboost::make_shared(node));
      refs[index + fields[0].size()]++;
    }
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
      refs[index]++;

      node.cost += 30;
      node.pre = fields[0];
      node.post = fields[2];
      chunks[index].push_back(unboost::make_shared(node));
      refs[index]++;
    } else {
      node.pre = fields[0];
      node.post = fields[2];
      chunks[index].push_back(unboost::make_shared(node));
      refs[index]++;
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
  const size_t len = fields[0].size();
  if (index + len > pre.size()) {
    return;
  }
  if (pre.substr(index, len) != fields[0]) {
    return;
  }
  
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
  case HB_RENTAISHI:
  case HB_FUKUSHI:
  case HB_SETSUZOKUSHI:
  case HB_KANDOUSHI:
  case HB_KAKU_JOSHI:
  case HB_SETSUZOKU_JOSHI:
  case HB_FUKU_JOSHI:
  case HB_SHUU_JOSHI:
  case HB_KANGO:
  case HB_SETTOUJI:
  case HB_SETSUBIJI:
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index]++;
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
    refs[index]++;
    break;
  case HB_RENYOU_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = RENYOU_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index]++;
    break;
  case HB_SHUUSHI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = SHUUSHI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index]++;
    break;
  case HB_RENTAI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = RENTAI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index]++;
    break;
  case HB_KATEI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = KATEI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index]++;
    break;
  case HB_MEIREI_JODOUSHI:
    node.bunrui = HB_JODOUSHI;
    node.katsuyou = MEIREI_KEI;
    node.pre = fields[0];
    node.post = fields[2];
    chunks[index].push_back(unboost::make_shared(node));
    refs[index]++;
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
      if (k >= 15) {
        DebugPrintW(L" ...");
        break;
      }
    }
    DebugPrintW(L"\n");
  }
#endif  // ndef NDEBUG
} // Lattice::Dump

//////////////////////////////////////////////////////////////////////////////

BOOL MzIme::MakeLattice(Lattice& lattice, const std::wstring& pre) {
  FOOTMARK();

  // initialize lattice
  assert(pre.size() != 0);
  const size_t length = pre.size();
  lattice.pre = pre;
  lattice.chunks.resize(length + 1);
  lattice.refs.assign(length + 1, 0);
  lattice.refs[0] = 1;

  // lock the dictionary
  WCHAR *dict_data = LockBasicDict();
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
      lattice.AddComplement(index);
      lattice.AddNodes(index + 1, dict_data);
      // dump
      lattice.Dump(3);

      ++count;
      if (count >= 256) break;
    }
    // unlock the dictionary
    UnlockBasicDict(dict_data);
    return TRUE;  // success
  }
  // dump
  lattice.Dump(4);
  return FALSE; // failure
} // MzIme::MakeLattice

void MzIme::MakeResult(MzConversionResult& result, Lattice& lattice) {
  FOOTMARK();
  result.clear();

  const size_t length = lattice.pre.size();
  LatticeNodePtr node1 = lattice.head;
  LatticeNodePtr tail = lattice.chunks[length][0];
  while (node1 != tail) {
    size_t kb1 = 0, max_len = 0, last_len1 = 0;
    for (size_t ib1 = 0; ib1 < node1->branches.size(); ++ib1) {
      LatticeNodePtr& node2 = node1->branches[ib1];
      for (size_t ib2 = 0; ib2 < node2->branches.size(); ++ib2) {
        LatticeNodePtr& node3 = node2->branches[ib2];
        size_t len = node2->pre.size() + node3->pre.size();
        if (max_len < len) {
          last_len1 = node2->pre.size();
          max_len = len;
          kb1 = ib1;
        } else if (max_len == len) {
          if (last_len1 > node2->pre.size()) {
            last_len1 = node2->pre.size();
            max_len = len;
            kb1 = ib1;
          }
        }
      }
    }
    // make cand
    MzConversionCandidate cand;
    cand.hiragana = node1->branches[kb1]->pre;
    cand.converted = node1->branches[kb1]->post;
    cand.cost = node1->cost;
    // make clause
    MzConversionClause clause;
    clause.candidates.push_back(cand);
    // add clause
    result.clauses.push_back(clause);
    // go next
    node1 = node1->branches[kb1];
  }

  // add other candidates
  size_t index = 0, iClause = 0;
  while (index < length && iClause < result.clauses.size()) {
    const LatticeChunk& chunk = lattice.chunks[index];
    MzConversionClause& clause = result.clauses[iClause];
    const size_t size = clause.candidates[0].hiragana.size();
    for (size_t i = 0; i < chunk.size(); ++i) {
      if (chunk[i]->pre.size() == size) {
        // add a candidate of same size
        clause.add(chunk[i]->pre, chunk[i]->post, chunk[i]->cost);
      }
    }

    const std::wstring& hiragana = chunk[0]->pre;
    clause.add(hiragana, hiragana, 10);

    std::wstring katakana;
    katakana = lcmap(hiragana, LCMAP_FULLWIDTH | LCMAP_KATAKANA);
    clause.add(katakana, katakana, 10);

    index += size;
    ++iClause;
  }

  result.sort();
} // MzIme::MakeResult

void MzIme::PluralClauseConversion(
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
} // MzIme::PluralClauseConversion

void MzIme::PluralClauseConversion(const std::wstring& strHiragana,
                                   MzConversionResult& result)
{
  FOOTMARK();

#if 1
  if (!LoadBasicDict()) {
    ::MessageBoxA(NULL, "ERROR", NULL, 0);
    return;
  }

  Lattice lattice;
  std::wstring pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  MakeLattice(lattice, pre);
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
} // MzIme::PluralClauseConversion

void MzIme::SingleClauseConversion(
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
} // MzIme::SingleClauseConversion

void MzIme::SingleClauseConversion(const std::wstring& strHiragana,
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
} // MzIme::SingleClauseConversion

BOOL MzIme::StretchClauseLeft(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  // TODO:
  return FALSE;
} // MzIme::StretchClauseLeft

BOOL MzIme::StretchClauseRight(
  LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
  FOOTMARK();
  // TODO:
  return FALSE;
} // MzIme::StretchClauseRight

//////////////////////////////////////////////////////////////////////////////
