// comp_str.cpp --- composition string of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

bool LogCompStrExtra::IsValid() {
  if (hiragana_phonemes.size() != typing_phonemes.size()) {
    FOOTMARK_PRINT_CALL_STACK();
    return false;
  }
  if (dwSelectedPhoneme > (DWORD)hiragana_phonemes.size()) {
    FOOTMARK_PRINT_CALL_STACK();
    return false;
  }
  for (size_t i = 0; i < hiragana_phonemes.size(); ++i) {
    if (hiragana_phonemes[i].empty()) {
      FOOTMARK_PRINT_CALL_STACK();
      return false;
    }
    if (typing_phonemes[i].empty()) {
      FOOTMARK_PRINT_CALL_STACK();
      return false;
    }
    if (hiragana_phonemes[i].size() > 5) {
      FOOTMARK_PRINT_CALL_STACK();
      return false;
    }
    if (typing_phonemes[i].size() > 5) {
      FOOTMARK_PRINT_CALL_STACK();
      return false;
    }
  }
  return true;
} // LogCompStrExtra::IsValid

void LogCompStrExtra::ErasePhoneme(DWORD iPhoneme) {
  assert(iPhoneme < GetPhonemeCount());
  hiragana_phonemes.erase(hiragana_phonemes.begin() + iPhoneme);
  typing_phonemes.erase(typing_phonemes.begin() + iPhoneme);
}

DWORD LogCompStrExtra::GetTotalSize() const {
  DWORD total = sizeof(COMPSTREXTRA);
  for (size_t i = 0; i < hiragana_phonemes.size(); ++i) {
    total += (hiragana_phonemes[i].size() + 1) * sizeof(WCHAR);
  }
  for (size_t i = 0; i < typing_phonemes.size(); ++i) {
    total += (typing_phonemes[i].size() + 1) * sizeof(WCHAR);
  }
  return total;
}

void LogCompStrExtra::clear() {
  FOOTMARK();
  dwSelectedClause = 0xFFFFFFFF;
  dwSelectedPhoneme = 0;
  hiragana_phonemes.clear();
  typing_phonemes.clear();
  phoneme_clauses.clear();
}

std::wstring
LogCompStrExtra::JoinLeft(const std::vector<std::wstring>& strs) const {
  FOOTMARK();
  std::wstring str;
  for (DWORD i = 0; i < dwSelectedPhoneme; ++i) {
    str += strs[i].c_str();
  }
  return str;
}

std::wstring
LogCompStrExtra::JoinRight(const std::vector<std::wstring>& strs) const {
  FOOTMARK();
  std::wstring str;
  const DWORD dwCount = (DWORD)strs.size();
  for (DWORD i = dwSelectedPhoneme; i < dwCount; ++i) {
    str += strs[i].c_str();
  }
  return str;
}

DWORD LogCompStrExtra::GetPhonemeCount() const {
  FOOTMARK();
  assert(hiragana_phonemes.size() == typing_phonemes.size());
  return (DWORD)hiragana_phonemes.size();
}

void LogCompStrExtra::InsertThere(
  std::vector<std::wstring>& strs, std::wstring& str)
{
  FOOTMARK();
  strs.insert(strs.begin() + dwSelectedPhoneme, str.c_str());
}

void LogCompStrExtra::InsertPhonemePair(
  std::wstring& typed, std::wstring& translated)
{
  FOOTMARK();
  InsertThere(typing_phonemes, typed);
  InsertThere(hiragana_phonemes, translated);
}

WCHAR LogCompStrExtra::GetPrevChar() const {
  FOOTMARK();
  if (dwSelectedPhoneme > 0) {
    std::wstring str = hiragana_phonemes[dwSelectedPhoneme - 1];
    assert(str.size());
    return str[str.size() - 1];
  }
  return 0;
}

std::wstring
LogCompStrExtra::Join(const std::vector<std::wstring>& strs) const {
  FOOTMARK();
  std::wstring str;
  for (size_t i = 0; i < strs.size(); ++i) {
    str += strs[i].c_str();
  }
  return str;
}

//////////////////////////////////////////////////////////////////////////////

void COMPSTREXTRA::GetLog(LogCompStrExtra& log) {
  FOOTMARK();
  log.clear();
  log.dwSelectedClause = dwSelectedClause;
  log.dwSelectedPhoneme = dwSelectedPhoneme;

  DWORD dwCount;
  LPWSTR pch = GetHiraganaPhonemes(dwCount);
  if (pch && dwCount) {
    while (dwCount--) {
      log.hiragana_phonemes.push_back(pch);
      pch += lstrlenW(pch) + 1;
    }
  }

  pch = GetTypingPhonemes(dwCount);
  if (pch && dwCount) {
    while (dwCount--) {
      log.typing_phonemes.push_back(pch);
      pch += lstrlenW(pch) + 1;
    }
  }
} // COMPSTREXTRA::GetLog

DWORD COMPSTREXTRA::Store(const LogCompStrExtra *log) {
  FOOTMARK();
  assert(this);
  assert(log);

  LPBYTE pb = GetBytes();
  dwSignature = 0xDEADFACE;
  dwSelectedClause = log->dwSelectedClause;
  dwSelectedPhoneme = log->dwSelectedPhoneme;
  pb += sizeof(COMPSTREXTRA);

  DWORD size;

  dwHiraganaPhonemeOffset = (DWORD)(pb - GetBytes());
  dwHiraganaPhonemeCount = 0;
  for (size_t i = 0; i < log->hiragana_phonemes.size(); ++i) {
    size = (log->hiragana_phonemes[i].size() + 1) * sizeof(WCHAR);
    memcpy(pb, &log->hiragana_phonemes[i][0], size);
    ++dwHiraganaPhonemeCount;
    pb += size;
  }

  dwTypingPhonemeOffset = (DWORD)(pb - GetBytes());
  dwTypingPhonemeCount = 0;
  for (size_t i = 0; i < log->typing_phonemes.size(); ++i) {
    size = (log->typing_phonemes[i].size() + 1) * sizeof(WCHAR);
    memcpy(pb, &log->typing_phonemes[i][0], size);
    ++dwTypingPhonemeCount;
    pb += size;
  }

  assert(log->GetTotalSize() == (DWORD)(pb - GetBytes()));
  return (DWORD)(pb - GetBytes());
} // COMPSTREXTRA::Store

//////////////////////////////////////////////////////////////////////////////

bool LogCompStr::IsValid() {
  bool ret = true;
  if (dwCursorPos > GetCompCharCount()) {
    FOOTMARK_PRINT_CALL_STACK();
    ret = false;
  }
  if (comp_clause.size()) {
    if (comp_clause[0] != 0) {
      FOOTMARK_PRINT_CALL_STACK();
      ret = false;
    }
    if (comp_clause[comp_clause.size() - 1] != GetCompCharCount()) {
      FOOTMARK_PRINT_CALL_STACK();
      ret = false;
    }
    if (extra.dwSelectedClause > (DWORD)comp_clause.size()) {
      FOOTMARK_PRINT_CALL_STACK();
      ret = false;
    }
    for (size_t i = 1; i < comp_clause.size(); ++i) {
      if (comp_clause[i] > GetCompCharCount()) {
        FOOTMARK_PRINT_CALL_STACK();
        ret = false;
      }
      if (comp_clause[i - 1] > comp_clause[i]) {
        FOOTMARK_PRINT_CALL_STACK();
        ret = false;
      }
    }
  }
  if (result_read_clause.size()) {
    if (result_read_clause[0] != 0) {
      FOOTMARK_PRINT_CALL_STACK();
      ret = false;
    }
    if (result_read_clause[result_read_clause.size() - 1] != 0) {
      FOOTMARK_PRINT_CALL_STACK();
      ret = false;
    }
    for (size_t i = 1; i < result_read_clause.size(); ++i) {
      if (result_read_clause[i] > (DWORD)result_read_str.size()) {
        FOOTMARK_PRINT_CALL_STACK();
        ret = false;
      }
      if (result_read_clause[i - 1] > result_read_clause[i]) {
        FOOTMARK_PRINT_CALL_STACK();
        ret = false;
      }
    }
  }
  if (result_clause.size()) {
    if (result_clause[0] != 0) {
      FOOTMARK_PRINT_CALL_STACK();
      ret = false;
    }
    if (result_clause[result_clause.size() - 1] != 0) {
      FOOTMARK_PRINT_CALL_STACK();
      ret = false;
    }
    for (size_t i = 1; i < result_clause.size(); ++i) {
      if (result_clause[i] > (DWORD)result_str.size()) {
        FOOTMARK_PRINT_CALL_STACK();
        ret = false;
      }
      if (result_clause[i - 1] > result_clause[i]) {
        FOOTMARK_PRINT_CALL_STACK();
        ret = false;
      }
    }
  }
  if (ret) {
    ret = extra.IsValid();
  }
  if (!ret) {
    Dump();
  }
  return ret;
} // LogCompStr::IsValid

void LogCompStr::clear() {
  FOOTMARK();
  clear_read();
  clear_comp();
  clear_result();
  clear_extra();
}

BOOL LogCompStr::HasClauseSelected() const {
  FOOTMARK();
  if (comp_clause.size() && extra.dwSelectedClause != 0xFFFFFFFF) {
    assert(extra.dwSelectedClause < GetClauseCount());
    return TRUE;
  }
  return FALSE;
}

BYTE LogCompStr::GetCompCharAttr(DWORD ich) const {
  FOOTMARK();
  BYTE ret = ATTR_INPUT;
  if (ich < (DWORD)comp_attr.size()) {
    ret = comp_attr[ich];
  }
  return ret;
}

DWORD LogCompStr::GetCurrentClause() const {
  FOOTMARK();
  DWORD ret = 0;
  if (extra.dwSelectedClause != 0xFFFFFFFF) ret = extra.dwSelectedClause;
  return ret;
}

DWORD LogCompStr::ClauseToPhoneme(DWORD iClause) const {
  FOOTMARK();
  if (iClause == 0xFFFFFFFF) return CompCharToPhoneme(dwCursorPos);
  assert(iClause < (DWORD)extra.phoneme_clauses.size());
  return extra.phoneme_clauses[iClause];
}

DWORD LogCompStr::PhonemeToClause(DWORD iPhoneme) const {
  FOOTMARK();
  DWORD ich = PhonemeToCompChar(iPhoneme);
  return CompCharToClause(ich, dwDeltaChar);
}

DWORD LogCompStr::ClauseToCompChar(DWORD iClause) const {
  FOOTMARK();
  if (iClause == 0xFFFFFFFF) return GetCompCharCount();
  assert(iClause < (DWORD)comp_clause.size());
  return comp_clause[iClause];
}

DWORD LogCompStr::CompCharToClause(DWORD iCompChar) const {
  FOOTMARK();
  DWORD iClause = 0xFFFFFFFF;
  for (size_t i = 0; i < comp_clause.size(); ++i) {
    if (iCompChar < comp_clause[i]) {
      iClause = (DWORD)comp_clause[i];
    }
  }
  return iClause;
}

DWORD LogCompStr::PhonemeToCompChar(DWORD iPhoneme, DWORD dwDeltaChar) const {
  FOOTMARK();
  // TODO:
  ...
}

DWORD LogCompStr::CompCharToPhoneme(DWORD iCompChar, DWORD& dwDeltaChar) const {
  FOOTMARK();
  DWORD iPhoneme = 0;
  dwDeltaChar = 0;
  for (DWORD i = 0; i < (DWORD)comp_clause.size(); ++i) {
    if (iCompChar < comp_clause[i]) {
      dwDeltaChar = comp_clause[i] - iCompChar;
      iPhoneme = i;
    }
  }
  return iPhoneme;
}

DWORD LogCompStr::GetClauseCount() const {
  FOOTMARK();
  if (comp_clause.size() < 2) return 0;
  return (DWORD)(comp_clause.size() - 1);
}

void LogCompStr::clear_read() {
  FOOTMARK();
  comp_read_attr.clear();
  comp_read_clause.clear();
  comp_read_str.clear();
}

void LogCompStr::clear_comp() {
  FOOTMARK();
  dwCursorPos = 0;
  comp_attr.clear();
  comp_clause.clear();
  comp_str.clear();
}

void LogCompStr::clear_result() {
  FOOTMARK();
  result_read_clause.clear();
  result_read_str.clear();
  result_clause.clear();
  result_str.clear();
}

DWORD LogCompStr::GetTotalSize() const {
  FOOTMARK();
  size_t total = sizeof(COMPOSITIONSTRING);
  total += comp_read_attr.size() * sizeof(BYTE);
  total += comp_read_clause.size() * sizeof(DWORD);
  total += (comp_read_str.size() + 1) * sizeof(WCHAR);
  total += comp_attr.size() * sizeof(BYTE);
  total += comp_clause.size() * sizeof(DWORD);
  total += (comp_str.size() + 1) * sizeof(WCHAR);
  total += result_read_clause.size() * sizeof(DWORD);
  total += (result_read_str.size() + 1) * sizeof(WCHAR);
  total += result_clause.size() * sizeof(DWORD);
  total += (result_str.size() + 1) * sizeof(WCHAR);
  total += extra.GetTotalSize();
  return total;
}

void LogCompStr::SetClauseCompString(
  DWORD iClause, std::wstring& strNew, BOOL bConverted)
{
  FOOTMARK();
  if (iClause == 0xFFFFFFFF) return;

  // replace old with new
  std::wstring strOld = GetClauseCompString(iClause);
  std::wstring strLeft = GetLeft(iClause);
  comp_str.replace(strLeft.size(), strOld.size(), strNew);

  // untarget
  for (size_t i = 0; i < comp_attr.size(); ++i) {
    if (comp_attr[i] == ATTR_TARGET_CONVERTED) comp_attr[i] = ATTR_CONVERTED;
  }

  // replace attr
  comp_attr.erase(
    comp_attr.begin() + strLeft.size(),
    comp_attr.begin() + strLeft.size() + strOld.size());
  comp_attr.insert(
    comp_attr.begin() + strLeft.size(), strNew.size(),
    (bConverted ? ATTR_TARGET_CONVERTED : ATTR_INPUT));

  // update clause
  if (strNew.size() > strOld.size()) {
    for (size_t i = iClause + 1; i < comp_clause.size(); ++i) {
      comp_clause[i] += (DWORD)(strNew.size() - strOld.size());
    }
  } else {
    if (strNew.size() < strOld.size()) {
      for (size_t i = iClause + 1; i < comp_clause.size(); ++i) {
        comp_clause[i] -= (DWORD)(strOld.size() - strNew.size());
      }
    }
  }

  if (bConverted) {
    dwCursorPos = (DWORD)comp_str.size();
    extra.dwSelectedClause = iClause;
  } else {
    dwCursorPos = (DWORD)(strLeft.size() + strNew.size());
    extra.dwSelectedClause = 0xFFFFFFFF;
  }
} // LogCompStr::SetClauseCompString

std::wstring LogCompStr::GetLeft(DWORD iClause) const {
  FOOTMARK();
  assert(iClause < (DWORD)comp_clause.size());
  DWORD ich = comp_clause[iClause];
  return comp_str.substr(0, ich);
}

std::wstring LogCompStr::GetRight(DWORD iClause) const {
  FOOTMARK();
  assert(iClause < (DWORD)comp_clause.size());
  DWORD ich = comp_clause[iClause];
  return comp_str.substr(ich);
}

DWORD LogCompStr::GetPhonemeCount() const {
  FOOTMARK();
  return extra.GetPhonemeCount();
}

std::wstring LogCompStr::GetClauseCompString(DWORD iClause) const {
  FOOTMARK();
  std::wstring ret;
  assert(iClause != 0xFFFFFFFF);
  if (iClause + 1 < (DWORD)comp_clause.size()) {
    DWORD ich = comp_clause[iClause], ichNext = comp_clause[iClause + 1];
    ret = comp_str.substr(ich, ichNext - ich);
  }
  return ret;
}

std::wstring LogCompStr::GetClauseHiraganaString(DWORD iClause) const {
  FOOTMARK();
  std::wstring ret;
  assert(iClause != 0xFFFFFFFF);
  if (iClause + 1 < GetPhonemeCount()) {
    DWORD iph = extra.phoneme_clauses[iClause];
    DWORD iphNext = extra.phoneme_clauses[iClause + 1];
    for (DWORD i = iph; i < iphNext; ++i) {
      ret += extra.hiragana_phonemes[i];
    }
  }
  return ret;
}

std::wstring LogCompStr::GetClauseTypingString(DWORD iClause) const {
  FOOTMARK();
  std::wstring ret;
  assert(iClause != 0xFFFFFFFF);
  if (iClause + 1 < GetPhonemeCount()) {
    DWORD iph = extra.phoneme_clauses[iClause];
    DWORD iphNext = extra.phoneme_clauses[iClause + 1];
    for (DWORD i = iph; i < iphNext; ++i) {
      ret += extra.typing_phonemes[i];
    }
  }
  return ret;
}

void LogCompStr::MakeHiragana() {
  FOOTMARK();
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = GetClauseHiraganaString(iClause);
  SetClauseCompString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeKatakana() {
  FOOTMARK();
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseHiraganaString(iClause), LCMAP_KATAKANA);
  SetClauseCompString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeHankaku() {
  FOOTMARK();
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseTypingString(iClause),
                           LCMAP_HALFWIDTH | LCMAP_KATAKANA);
  SetClauseCompString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeZenEisuu() {
  FOOTMARK();
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseTypingString(iClause), LCMAP_FULLWIDTH);
  SetClauseCompString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeHanEisuu() {
  FOOTMARK();
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseTypingString(iClause), LCMAP_HALFWIDTH);
  SetClauseCompString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeResult() {
  FOOTMARK();
  result_read_clause = comp_read_clause;  // TODO: generate reading
  result_read_str = comp_read_str;
  result_clause = comp_clause;
  result_str = comp_str;
  clear_read();
  clear_comp();
  clear_extra();
  dwCursorPos = 0;
}

BYTE LogCompStr::GetClauseAttr(DWORD iClause) const {
  FOOTMARK();
  BYTE ret = ATTR_INPUT;
  if (iClause != 0xFFFFFFFF) {
    assert(iClause < GetClauseCount());
    DWORD ich = ClauseToCompChar(iClause);
    ret = GetCompCharAttr(ich);
  }
  return ret;
}

BOOL LogCompStr::IsClauseConverted(DWORD iClause) const {
  return GetClauseAttr(iClause) != ATTR_INPUT;
}

void LogCompStr::SetClauseAttr(DWORD iClause, BYTE attr) {
  FOOTMARK();
  if (iClause != 0xFFFFFFFF) {
    DWORD ich = ClauseToCompChar(iClause);
    DWORD ichEnd = ClauseToCompChar(iClause + 1);
    for (DWORD i = ich; i < ichEnd; ++i) {
      comp_attr[i] = attr;
    }
  }
}

BOOL LogCompStr::IsBeingConverted() {
  FOOTMARK();
  for (size_t i = 0; i < comp_attr.size(); ++i) {
    if (comp_attr[i] != ATTR_INPUT) return TRUE;
  }
  return FALSE;
}

DWORD LogCompStr::GetCompCharCount() const {
  FOOTMARK();
  return (DWORD)comp_str.size();
}

void LogCompStr::ExtraUpdated(INPUT_MODE imode) {
  FOOTMARK();
  std::wstring strLeft, strRight;
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    strLeft = extra.JoinLeft(extra.hiragana_phonemes);
    strRight = extra.JoinRight(extra.hiragana_phonemes);
    break;
  case IMODE_ZEN_KATAKANA:
    strLeft = lcmap(extra.JoinLeft(extra.hiragana_phonemes), LCMAP_KATAKANA);
    strRight = lcmap(extra.JoinRight(extra.hiragana_phonemes), LCMAP_KATAKANA);
    break;
  case IMODE_HAN_KANA:
    strLeft = lcmap(extra.JoinLeft(extra.hiragana_phonemes),
                    LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    strRight = lcmap(extra.JoinRight(extra.hiragana_phonemes),
                     LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    break;
  case IMODE_ZEN_EISUU:
    strLeft = lcmap(extra.JoinLeft(extra.typing_phonemes), LCMAP_FULLWIDTH);
    strRight = lcmap(extra.JoinRight(extra.typing_phonemes), LCMAP_FULLWIDTH);
    break;
  case IMODE_HAN_EISUU:
    strLeft = lcmap(extra.JoinLeft(extra.hiragana_phonemes), LCMAP_HALFWIDTH);
    strRight = lcmap(extra.JoinRight(extra.hiragana_phonemes), LCMAP_HALFWIDTH);
    break;
  default:
    assert(0);
    break;
  }
  dwCursorPos = (DWORD)strLeft.size();
  comp_str = strLeft + strRight;
}

void LogCompStr::AddKanaChar(
  std::wstring& typed, std::wstring& translated, DWORD dwConversion)
{
  FOOTMARK();
  // if there was dakuon combination
  WCHAR chDakuon = dakuon_shori(extra.GetPrevChar(), translated[0]);
  if (chDakuon) {
    // store a dakuon combination
    std::wstring str;
    str += chDakuon;
    extra.hiragana_phonemes[extra.dwSelectedPhoneme - 1] = str;
    extra.typing_phonemes[extra.dwSelectedPhoneme - 1] += L'Þ';
  } else {
    // insert the typed and translated strings
    extra.InsertPhonemePair(typed, translated);
    // move cursor
    ++dwCursorPos;
    ++extra.dwSelectedPhoneme;
  }
  // create the composition string
  INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConversion);
  ExtraUpdated(imode);
} // LogCompStr::AddKanaChar

void LogCompStr::AddRomanChar(
  std::wstring& typed, std::wstring& translated, DWORD dwConversion)
{
  FOOTMARK();
  const WCHAR chTyped = typed[0];
  // create the typed string and translated string
  if (is_hiragana(chTyped)) {
    // a hiragana character was typed. do reverse roman conversion
    translated = typed;
    typed = hiragana_to_roman(typed);
    // insert phonemes
    extra.InsertPhonemePair(typed, translated);
    ++extra.dwSelectedPhoneme;
  } else if (is_zenkaku_katakana(chTyped)) {
    // a katakana character was typed. make it hiragana
    translated = lcmap(typed, LCMAP_HIRAGANA);
    // do reverse roman conversion
    typed = hiragana_to_roman(translated);
    // insert phonemes
    extra.InsertPhonemePair(typed, translated);
    ++extra.dwSelectedPhoneme;
  } else if (is_kanji(chTyped)) {
    // a kanji (Chinese-oriented) character was typed
    translated = typed;
    // insert phonemes without conversion
    extra.InsertPhonemePair(typed, translated);
    ++extra.dwSelectedPhoneme;
  } else if (std::isalnum(chTyped) || chTyped == L'\'') {
    // an alphabet or apostorophe was typed. get previous char
    WCHAR chPrev = extra.GetPrevChar();
    // if previous one is hiragana or apostorophe or none, then
    if (is_hiragana(chPrev) || chPrev == L'\'' || chPrev == 0) {
      // insert phonemes without conversion
      translated = typed;
      extra.InsertPhonemePair(typed, translated);
      ++extra.dwSelectedPhoneme;
    } else {
      // if the char was not Japanese, then
      if (!(dwConversion & IME_CMODE_JAPANESE)) {
        // insert phonemes without conversion
        translated = typed;
        extra.InsertPhonemePair(typed, translated);
        ++extra.dwSelectedPhoneme;
      } else {
        // make hiragana phoneme from roman
        std::wstring str;
        str = extra.hiragana_phonemes[extra.dwSelectedPhoneme - 1];
        str += typed;
        std::wstring strConverted = roman_to_hiragana(str);
        if (is_hiragana(strConverted[0]) && strConverted.size() == 1) {
          // strConverted was one hiragana char
          extra.hiragana_phonemes[extra.dwSelectedPhoneme - 1] = strConverted;
          extra.typing_phonemes[extra.dwSelectedPhoneme - 1] += typed;
        } else { // otherwise,
          // get the difference of str and strConverted
          std::wstring part;
          for (size_t i = 0; i < strConverted.size(); ++i) {
            if (str[i] == strConverted[i]) {
              part += str[i];
            } else {
              break;
            }
          }
          // if no match,
          if (part.empty()) {
            // insert phonemes without conversion
            translated = typed;
            extra.InsertPhonemePair(typed, translated);
            ++extra.dwSelectedPhoneme;
          } else {  // there was match
            // if matching part was whole and match was longer than five, then
            if (part.size() == str.size() && part.size() > 5) {
              // shrink part to split phoneme
              part = part.substr(0, 1);
            }
            // set part to phonemes
            extra.hiragana_phonemes[extra.dwSelectedPhoneme - 1] = part;
            extra.typing_phonemes[extra.dwSelectedPhoneme - 1] = part;
            // get remainder and insert it
            str = str.substr(part.size());
            strConverted = strConverted.substr(part.size());
            extra.InsertPhonemePair(str, strConverted);
            ++extra.dwSelectedPhoneme;
          }
        }
      }
    }
  } else {
    // otherwise insert phonemes without conversion
    translated = typed;
    extra.InsertPhonemePair(typed, translated);
    ++extra.dwSelectedPhoneme;
  }

  // create the composition string
  INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConversion);
  ExtraUpdated(imode);
} // LogCompStr::AddRomanChar

void LogCompStr::AddChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConversion) {
  FOOTMARK();
  std::wstring strTyped, strTranslated;
  if (chTranslated && !(dwConversion & IME_CMODE_ROMAN)) {   // kana input
    assert(is_hiragana(chTranslated));
    strTyped += chTyped;
    strTranslated += chTranslated;
    AddKanaChar(strTyped, strTranslated, dwConversion);
  } else {  // roman input
    strTyped += chTyped;
    strTranslated = strTyped;
    AddRomanChar(strTyped, strTranslated, dwConversion);
  }
  // create the reading string
  std::wstring str = extra.Join(extra.hiragana_phonemes);
  comp_read_str = lcmap(str, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
} // LogCompStr::AddChar

void LogCompStr::RevertText() {
  FOOTMARK();
  // reset composition
  assert(extra.dwSelectedClause != 0xFFFFFFFF);
  // get a hiragana string of the current clause
  std::wstring str = GetClauseHiraganaString(extra.dwSelectedClause);
  // set the hiragana string to the current clause
  SetClauseCompString(extra.dwSelectedClause, str, FALSE);
  // set delta start
  dwDeltaStart = ClauseToCompChar(extra.dwSelectedClause);
}

void LogCompStr::DeleteChar(BOOL bRoman, BOOL bBackSpace/* = FALSE*/) {
  FOOTMARK();
  // is the current clause being converted?
  if (IsClauseConverted(extra.dwSelectedClause) != ATTR_INPUT) { // being converted
    // set the hiragana string to the clause
    std::wstring str = GetClauseHiraganaString(extra.dwSelectedClause);
    SetClauseCompString(extra.dwSelectedClause, str, FALSE);
  } else {  // not being converted
    BOOL flag = FALSE;
    // is it back space?
    if (bBackSpace) { // back space
      if (0 < dwCursorPos && dwCursorPos <= GetCompCharCount()) {
        --dwCursorPos;  // move left
        extra.dwSelectedPhoneme = CompCharToPhoneme(dwCursorPos);
        flag = TRUE;
      }
    } else {  // not back space
      if (0 <= dwCursorPos && dwCursorPos < GetCompCharCount()) {
        assert(extra.dwSelectedPhoneme < GetPhonemeCount());
        flag = TRUE;
      }
    }
    if (flag) {
      // get the current phoneme string as str
      std::wstring& str = extra.hiragana_phonemes[extra.dwSelectedPhoneme];
      if (str.size() <= 1) {
        extra.ErasePhoneme(extra.dwSelectedPhoneme);  // erase phoneme
        comp_str.erase(dwCursorPos, 1);   // delete one char
      } else {
        comp_str.erase(dwCursorPos, 1);   // delete one char
        // scan and compare comp_str and str
        for (size_t i = 0; i < str.size(); ++i) {
          if (str[i] != comp_str[dwCursorPos + i]) {
            // erase one char of phoneme
            str.erase(i, 1);
            // generate typing phonemes
            if (bRoman) {
              extra.typing_phonemes[extra.dwSelectedPhoneme] =
                hiragana_to_roman(str);
            } else {
              extra.typing_phonemes[extra.dwSelectedPhoneme] =
                lcmap(str, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
            }
            break;
          }
        }
      }
    }
  }
} // LogCompStr::DeleteChar

void LogCompStr::MoveLeft(BOOL bShift) {
  FOOTMARK();
  // TODO: bShift
  // is the current clause being converted?
  if (extra.dwSelectedClause(extra.dwSelectedClause)) { // being converted
    // untarget
    SetClauseAttr(extra.dwSelectedClause, ATTR_CONVERTED);
    // set the current clause
    if (extra.dwSelectedClause > 0) {
      --extra.dwSelectedClause;
    } else {
      assert(comp_clause.size() >= 2);
      extra.dwSelectedClause = GetClauseCount() - 1;
    }
    // retarget
    SetClauseAttr(extra.dwSelectedClause, ATTR_TARGET_CONVERTED);
    // set the current phoneme
    extra.dwSelectedPhoneme = ClauseToPhoneme(extra.dwSelectedClause);
  } else {
    // move cursor
    if (dwCursorPos > 0) --dwCursorPos;
    // did it enter the converted?
    if (GetCompCharAttr(dwCursorPos) != ATTR_INPUT) { // entered the converted
      // untarget
      SetClauseAttr(extra.dwSelectedClause, ATTR_CONVERTED);
      // set the current clause
      extra.dwSelectedClause = CompCharToClause(dwCursorPos);
      // retarget
      SetClauseAttr(extra.dwSelectedClause, ATTR_TARGET_CONVERTED);
    }
    // set the current phoneme
    extra.dwSelectedPhoneme = CompCharToPhoneme(dwCursorPos);
  }
} // LogCompStr::MoveLeft

void LogCompStr::MoveRight(BOOL bShift) {
  FOOTMARK();
  // TODO: bShift
  // is the current clause being converted?
  if (extra.dwSelectedClause(extra.dwSelectedClause)) { // being converted
    // untarget
    SetClauseAttr(extra.dwSelectedClause, ATTR_CONVERTED);
    // set current clause
    ++extra.dwSelectedClause;
    if (extra.dwSelectedClause >= GetClauseCount()) { // exceeded
      extra.dwSelectedClause = 0;
    }
    // retarget
    SetClauseAttr(extra.dwSelectedClause, ATTR_TARGET_CONVERTED);
    // set current phoneme
    extra.dwSelectedPhoneme = ClauseToPhoneme(extra.dwSelectedClause);
  } else {  // not being converted
    // move cursor
    if (dwCursorPos + 1 < GetCompCharCount()) {
      ++dwCursorPos;
    }
    // did it enter the converted?
    if (GetCompCharAttr(dwCursorPos) != ATTR_INPUT) { // entered the converted
      // untarget
      SetClauseAttr(extra.dwSelectedClause, ATTR_CONVERTED);
      // set current clause
      extra.dwSelectedClause = CompCharToClause(dwCursorPos);
      // retarget
      SetClauseAttr(extra.dwSelectedClause, ATTR_TARGET_CONVERTED);
    }
    // set current phoneme
    extra.dwSelectedPhoneme = CompCharToPhoneme(dwCursorPos);
  }
} // LogCompStr::MoveRight

std::wstring
LogCompStr::Translate(
  const std::wstring& hiragana, const std::wstring& typing,
  DWORD dwConversion)
{
  InputMode imode = InputModeFromConversionMode(TRUE, dwConversion);
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    return hiragana;
  case IMODE_ZEN_KATAKANA:
    return lcmap(hiragana, LCMAP_KATAKANA);
  case IMODE_ZEN_EISUU:
    return lcmap(typing, LCMAP_FULLWIDTH);
  case IMODE_HAN_KANA:
    return lcmap(typing, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
  case IMODE_HAN_EISUU:
    return lcmap(typing, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
  default:
    return typing;
  }
} // LogCompStr::Transfer

void LogCompStr::MakeCompString(DWORD dwConversion) {
  std::wstring new_comp_str;
  DWORD i, count = GetClauseCount();
  for (i = 0; i < count; ++i) {
    DWORD ich = ClauseToCompChar(i);
    DWORD ichNext = ClauseToCompChar(i + 1);
    if (GetCompCharAttr(ich) == ATTR_INPUT) {
      std::wstring hiragana, typing;
      DWORD iph = CompCharToPhoneme(ich);
      DWORD iphNext = CompCharToPhoneme(ichNext);
      for (DWORD k = iph; k < iphNext; ++k) {
        hiragana += extra.hiragana_phonemes[k];
        typing += extra.typing_phonemes[k];
      }
      new_comp_str += Translate(hiragana, typing, dwConversion);
    } else {
      new_comp_str += GetClauseCompString(i);
    }
  }
  for (i = 0; i < count; ++i) {
    DWORD ich = ClauseToCompChar(i);
    comp_clause[i] = ich;
  }
  comp_clause[i] = GetCompCharCount();
  comp_str = new_comp_str;
  dwCursorPos = PhonemeToCompChar(extra.dwSelectedPhoneme);
}

void LogCompStr::Dump() {
#ifndef NDEBUG
  DebugPrint(TEXT("### LogCompStr ###\n"));
  DebugPrint(TEXT("+ dwCursorPos: %08X\n"), dwCursorPos);
  DebugPrint(TEXT("+ dwDeltaStart: %08X\n"), dwDeltaStart);

  DebugPrint(TEXT("+ comp_read_attr: "));
  for (size_t i = 0; i < comp_read_attr.size(); ++i) {
    DebugPrint(TEXT("%02X "), comp_read_attr[i]);
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ comp_read_clause: "));
  for (size_t i = 0; i < comp_read_clause.size(); ++i) {
    DebugPrint(TEXT("%08X "), comp_read_clause[i]);
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ comp_read_str: %ls\n"), comp_read_str.c_str());
  DebugPrint(TEXT("+ comp_attr: "));
  for (size_t i = 0; i < comp_attr.size(); ++i) {
    DebugPrint(TEXT("%02X "), comp_attr[i]);
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ comp_clause: "));
  for (size_t i = 0; i < comp_clause.size(); ++i) {
    DebugPrint(TEXT("%08X "), comp_clause[i]);
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ comp_str: %ls\n"), comp_str.c_str());

  DebugPrint(TEXT("+ result_read_clause: "));
  for (size_t i = 0; i < result_read_clause.size(); ++i) {
    DebugPrint(TEXT("%08X "), result_read_clause[i]);
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ result_read_str: %ls\n"), result_read_str.c_str());

  DebugPrint(TEXT("+ result_clause: "));
  for (size_t i = 0; i < result_clause.size(); ++i) {
    DebugPrint(TEXT("%08X "), result_clause[i]);
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ extra.dwSelectedClause: %08X\n"), extra.dwSelectedClause);
  DebugPrint(TEXT("+ extra.dwSelectedPhoneme: %08X\n"), extra.dwSelectedPhoneme);

  DebugPrint(TEXT("+ extra.hiragana_phonemes: "));
  for (size_t i = 0; i < extra.hiragana_phonemes.size(); ++i) {
    DebugPrint(TEXT("%ls "), extra.hiragana_phonemes[i].c_str());
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ extra.typing_phonemes: "));
  for (size_t i = 0; i < extra.typing_phonemes.size(); ++i) {
    DebugPrint(TEXT("%ls "), extra.typing_phonemes[i].c_str());
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ extra.phoneme_clauses: "));
  for (size_t i = 0; i < extra.phoneme_clauses.size(); ++i) {
    DebugPrint(TEXT("%08X "), extra.phoneme_clauses[i]);
  }
  DebugPrint(TEXT("\n"));
#endif  // ndef NDEBUG
}

//////////////////////////////////////////////////////////////////////////////

BOOL CompStr::IsBeingConverted() {
  return (dwCompStrLen > 0 && dwCompAttrLen > 0 &&
          GetCompAttr()[0] != ATTR_INPUT);
}

void CompStr::GetLog(LogCompStr& log) {
  FOOTMARK();
  log.dwCursorPos = dwCursorPos;
  log.dwDeltaStart = dwDeltaStart;
  log.comp_read_attr.assign(GetCompReadAttr(), GetCompReadAttr() + dwCompReadAttrLen);
  log.comp_read_clause.assign(GetCompReadClause(), GetCompReadClause() + dwCompReadClauseLen / sizeof(DWORD));
  log.comp_read_str.assign(GetCompReadStr(), dwCompReadStrLen);
  log.comp_attr.assign(GetCompAttr(), GetCompAttr() + dwCompAttrLen);
  log.comp_clause.assign(GetCompClause(), GetCompClause() + dwCompClauseLen / sizeof(DWORD));
  log.comp_str.assign(GetCompStr(), dwCompStrLen);
  log.result_read_clause.assign(GetResultReadClause(), GetResultReadClause() + dwResultReadClauseLen / sizeof(DWORD));
  log.result_read_str.assign(GetResultReadStr(), dwResultReadStrLen);
  log.result_clause.assign(GetResultClause(), GetResultClause() + dwResultClauseLen / sizeof(DWORD));
  log.result_str.assign(GetResultStr(), dwResultStrLen);
  COMPSTREXTRA *extra = GetExtra();
  if (extra && extra->dwSignature == 0xDEADFACE) {
    extra->GetLog(log.extra);
  } else {
    // TODO: do something
  }
}

/*static*/ HIMCC CompStr::ReCreate(HIMCC hCompStr, const LogCompStr *log) {
  FOOTMARK();
  LogCompStr log_comp_str;
  if (log == NULL) {
    log = &log_comp_str;
  }
  const DWORD total = log->GetTotalSize();
  HIMCC hNewCompStr = ::ImmReSizeIMCC(hCompStr, total);
  if (hNewCompStr) {
    CompStr *lpCompStr = (CompStr *)::ImmLockIMCC(hNewCompStr);
    if (lpCompStr) {
      LPBYTE pb = lpCompStr->GetBytes();
      lpCompStr->dwSize = total;
      lpCompStr->dwCursorPos = log->dwCursorPos;
      lpCompStr->dwDeltaStart = log->dwDeltaStart;
      pb += sizeof(COMPOSITIONSTRING);

#define ADD_BYTES(member) \
  if (log->member.size()) { \
    memcpy(pb, &log->member[0], log->member.size() * sizeof(BYTE)); \
    pb += log->member.size() * sizeof(BYTE); \
  }

#define ADD_DWORDS(member) \
  if (log->member.size()) { \
    memcpy(pb, &log->member[0], log->member.size() * sizeof(DWORD)); \
    pb += log->member.size() * sizeof(DWORD); \
  }

#define ADD_STRING(member) \
  if (log->member.size()) { \
    memcpy(pb, &log->member[0], (log->member.size() + 1) * sizeof(WCHAR)); \
    pb += (log->member.size() + 1) * sizeof(WCHAR); \
  } else { \
    pb[0] = pb[1] = 0; \
    pb += 1 * sizeof(WCHAR); \
  }

      lpCompStr->dwCompReadAttrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwCompReadAttrLen = log->comp_read_attr.size() * sizeof(BYTE);
      ADD_BYTES(comp_read_attr);
      lpCompStr->dwCompReadClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwCompReadClauseLen = log->comp_read_clause.size() * sizeof(DWORD);
      ADD_DWORDS(comp_read_clause);
      lpCompStr->dwCompReadStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwCompReadStrLen = log->comp_read_str.size();
      ADD_STRING(comp_read_str);
      lpCompStr->dwCompAttrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwCompAttrLen = log->comp_attr.size() * sizeof(BYTE);
      ADD_BYTES(comp_attr);
      lpCompStr->dwCompClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwCompClauseLen = log->comp_clause.size() * sizeof(DWORD);
      ADD_DWORDS(comp_clause);
      lpCompStr->dwCompStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwCompStrLen = log->comp_str.size();
      ADD_STRING(comp_str);
      lpCompStr->dwResultReadClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwResultReadClauseLen = log->result_read_clause.size() * sizeof(DWORD);
      ADD_DWORDS(result_read_clause);
      lpCompStr->dwResultReadStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwResultReadStrLen = log->result_read_str.size();
      ADD_STRING(result_read_str);
      lpCompStr->dwResultClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwResultClauseLen = log->result_clause.size() * sizeof(DWORD);
      ADD_DWORDS(result_clause);
      lpCompStr->dwResultStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      lpCompStr->dwResultStrLen = log->result_str.size();
      ADD_STRING(result_str);

      COMPSTREXTRA *extra = (COMPSTREXTRA *)pb;
      lpCompStr->dwPrivateSize = log->extra.GetTotalSize();
      lpCompStr->dwPrivateOffset = (DWORD)(pb - lpCompStr->GetBytes());
      pb += extra->Store(&log->extra);

      assert((DWORD)(pb - lpCompStr->GetBytes()) == total);

      ::ImmUnlockIMCC(hNewCompStr);
      hCompStr = hNewCompStr;
    } else {
      DebugPrint(TEXT("CompStr::ReCreate: failed #2"));
    }
  } else {
    DebugPrint(TEXT("CompStr::ReCreate: failed"));
  }
  return hCompStr;
} // CompStr::ReCreate

void CompStr::Dump() {
  FOOTMARK();
#ifndef NDEBUG
  DebugPrint(TEXT("dwSize: %08X\n"), dwSize);
  DebugPrint(TEXT("dwCursorPos: %08X\n"), dwCursorPos);
  DebugPrint(TEXT("dwDeltaStart: %08X\n"), dwDeltaStart);
  DebugPrint(TEXT("CompReadAttr: "));
  if (dwCompReadAttrLen) {
    LPBYTE attrs = GetCompReadAttr();
    for (DWORD i = 0; i < dwCompReadAttrLen; ++i) {
      DebugPrint(TEXT("%02X "), attrs[i]);
    }
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("CompReadClause: "));
  if (dwCompReadClauseLen) {
    LPDWORD clauses = GetCompReadClause();
    for (DWORD i = 0; i < dwCompReadClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X "), clauses[i]);
    }
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("CompReadStr: "));
  if (dwCompReadStrLen) {
    LPTSTR str = GetCompReadStr();
    DebugPrint(TEXT("%s"), str);
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("CompAttr: "));
  if (dwCompAttrLen) {
    LPBYTE attrs = GetCompAttr();
    for (DWORD i = 0; i < dwCompAttrLen; ++i) {
      DebugPrint(TEXT("%02X "), attrs[i]);
    }
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("CompClause: "));
  if (dwCompClauseLen) {
    LPDWORD clauses = GetCompClause();
    for (DWORD i = 0; i < dwCompClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X "), clauses[i]);
    }
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("CompStr: "));
  if (dwCompStrLen) {
    LPTSTR str = GetCompStr();
    DebugPrint(TEXT("%s"), str);
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("ResultReadClause: "));
  if (dwResultReadClauseLen) {
    LPDWORD clauses = GetResultReadClause();
    for (DWORD i = 0; i < dwResultReadClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X "), clauses[i]);
    }
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("ResultReadStr: "));
  if (dwResultReadStrLen) {
    LPTSTR str = GetResultReadStr();
    DebugPrint(TEXT("%s"), str);
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("ResultClause: "));
  if (dwResultClauseLen) {
    LPDWORD clauses = GetResultClause();
    for (DWORD i = 0; i < dwResultClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X "), clauses[i]);
    }
  }
  DebugPrint(TEXT("\n"));
  DebugPrint(TEXT("ResultStr: "));
  if (dwResultStrLen) {
    LPTSTR str = GetResultStr();
    DebugPrint(TEXT("%s"), str);
  }
  DebugPrint(TEXT("\n"));
#endif  // ndef NDEBUG
} // CompStr::Dump

//////////////////////////////////////////////////////////////////////////////
