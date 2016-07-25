
// comp_str.cpp --- composition string of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

DWORD LogCompStrExtra::GetTotalSize() const {
  FOOTMARK();
  DWORD total = sizeof(COMPSTREXTRA);
  for (size_t i = 0; i < hiragana_clauses.size(); ++i) {
    total += (hiragana_clauses[i].size() + 1) * sizeof(WCHAR);
  }
  for (size_t i = 0; i < typing_clauses.size(); ++i) {
    total += (typing_clauses[i].size() + 1) * sizeof(WCHAR);
  }
  return total;
}

void LogCompStrExtra::clear() {
  FOOTMARK();
  iClause = 0;
  hiragana_clauses.clear();
  typing_clauses.clear();
}

//////////////////////////////////////////////////////////////////////////////

WCHAR *COMPSTREXTRA::GetHiraganaClauses(DWORD& dwCount) {
  FOOTMARK();
  dwCount = dwHiraganaClauseCount;
  if (dwCount) {
    return (WCHAR *)(GetBytes() + dwHiraganaClauseOffset);
  }
  return NULL;
}

WCHAR *COMPSTREXTRA::GetTypingClauses(DWORD& dwCount) {
  FOOTMARK();
  dwCount = dwTypingClauseCount;
  if (dwCount) {
    return (WCHAR *)(GetBytes() + dwTypingClauseOffset);
  }
  return NULL;
}

void COMPSTREXTRA::GetLog(LogCompStrExtra& log) {
  FOOTMARK();
  log.clear();
  log.iClause = iClause;

  DWORD dwCount;
  WCHAR *pch = GetHiraganaClauses(dwCount);
  if (pch && dwCount) {
    while (dwCount--) {
      log.hiragana_clauses.push_back(pch);
      pch += lstrlenW(pch) + 1;
    }
  }

  pch = GetTypingClauses(dwCount);
  if (pch && dwCount) {
    while (dwCount--) {
      log.typing_clauses.push_back(pch);
      pch += lstrlenW(pch) + 1;
    }
  }
} // COMPSTREXTRA::GetLog

DWORD COMPSTREXTRA::Store(const LogCompStrExtra *log) {
  FOOTMARK();
  assert(this);
  assert(log);

  BYTE *pb = GetBytes();
  dwSignature = 0xDEADFACE;
  iClause = log->iClause;
  pb += sizeof(COMPSTREXTRA);

  DWORD size;

  dwHiraganaClauseOffset = (DWORD)(pb - GetBytes());
  dwHiraganaClauseCount = 0;
  for (size_t i = 0; i < log->hiragana_clauses.size(); ++i) {
    size = (log->hiragana_clauses[i].size() + 1) * sizeof(WCHAR);
    memcpy(pb, &log->hiragana_clauses[i][0], size);
    ++dwHiraganaClauseCount;
    pb += size;
  }

  dwTypingClauseOffset = (DWORD)(pb - GetBytes());
  dwTypingClauseCount = 0;
  for (size_t i = 0; i < log->typing_clauses.size(); ++i) {
    size = (log->typing_clauses[i].size() + 1) * sizeof(WCHAR);
    memcpy(pb, &log->typing_clauses[i][0], size);
    ++dwTypingClauseCount;
    pb += size;
  }

  assert(log->GetTotalSize() == (DWORD)(pb - GetBytes()));
  return (DWORD)(pb - GetBytes());
} // COMPSTREXTRA::Store

//////////////////////////////////////////////////////////////////////////////

void LogCompStr::clear() {
  FOOTMARK();
  clear_read();
  clear_comp();
  clear_result();
  clear_extra();
  fix();
}

void LogCompStr::fix() {
  extra.comp_str_clauses.clear();
  size_t count = comp_clause.size();
  if (count >= 1) {
    std::wstring str;
    for (size_t i = 0; i < count - 1; ++i) {
      str = GetClauseCompString(i);
      extra.comp_str_clauses.push_back(str);
    }
  } else {
    extra.comp_str_clauses.push_back(L"");
  }
  if (extra.hiragana_clauses.empty()) {
    extra.hiragana_clauses.push_back(L"");
  }
  if (extra.typing_clauses.empty()) {
    extra.typing_clauses.push_back(L"");
  }
} // LogCompStr::fix

void LogCompStr::clear_read() {
  FOOTMARK();
  comp_read_attr.clear();
  comp_read_clause.clear();
  comp_read_str.clear();
}

void LogCompStr::clear_comp() {
  FOOTMARK();
  dwCursorPos = 0;
  dwDeltaStart = 0;
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

DWORD LogCompStr::GetClauseCount() const {
  FOOTMARK();
  if (comp_clause.size() < 2) return 0;
  return (DWORD)(comp_clause.size() - 1);
}

DWORD LogCompStr::GetTotalSize() const {
  FOOTMARK();
  size_t total = sizeof(COMPOSITIONSTRING);
  total += comp_read_attr.size() * sizeof(BYTE);
  total += comp_read_clause.size() * sizeof(DWORD);
  total += comp_read_str.size() * sizeof(WCHAR);
  total += comp_attr.size() * sizeof(BYTE);
  total += comp_clause.size() * sizeof(DWORD);
  total += comp_str.size() * sizeof(WCHAR);
  total += result_read_clause.size() * sizeof(DWORD);
  total += result_read_str.size() * sizeof(WCHAR);
  total += result_clause.size() * sizeof(DWORD);
  total += result_str.size() * sizeof(WCHAR);
  total += extra.GetTotalSize();
  return total;
}

BOOL LogCompStr::IsBeingConverted() {
  FOOTMARK();
  for (size_t i = 0; i < comp_attr.size(); ++i) {
    if (comp_attr[i] != ATTR_INPUT) return TRUE;
  }
  return FALSE;
}

BOOL LogCompStr::HasCompStr() const {
  return comp_str.size() > 0;
}

BOOL LogCompStr::CompCharInClause(
  DWORD iCompChar, DWORD iClause, BOOL bExcludeEnd/* = FALSE */) const {
  FOOTMARK();
  if (bExcludeEnd) {
    return (iClause < GetClauseCount() &&
            comp_clause[iClause] <= iCompChar &&
            iCompChar < comp_clause[iClause + 1]);
  } else {
    return (iClause < GetClauseCount() &&
            comp_clause[iClause] <= iCompChar &&
            iCompChar <= comp_clause[iClause + 1]);
  }
}

BYTE LogCompStr::GetClauseAttr(DWORD dwClauseIndex) const {
  FOOTMARK();
  BYTE ret = ATTR_INPUT;
  if (dwClauseIndex < GetClauseCount()) {
    DWORD ich = ClauseToCompChar(dwClauseIndex);
    ret = GetCompCharAttr(ich);
  }
  return ret;
}

void LogCompStr::SetClauseAttr(DWORD dwClauseIndex, BYTE attr) {
  FOOTMARK();
  if (dwClauseIndex < GetClauseCount()) {
    DWORD ich0 = comp_clause[dwClauseIndex];
    DWORD ich1 = comp_clause[dwClauseIndex + 1];
    for (DWORD i = ich0; i < ich1; ++i) {
      if (i < comp_attr.size()) {
        comp_attr[i] = attr;
      }
    }
  }
}

BOOL LogCompStr::IsClauseConverted() const {
  FOOTMARK();
  return IsClauseConverted(extra.iClause);
}

BOOL LogCompStr::IsClauseConverted(DWORD dwClauseIndex) const {
  FOOTMARK();
  return GetClauseAttr(dwClauseIndex) != ATTR_INPUT;
}

BOOL LogCompStr::HasClauseSelected() const {
  FOOTMARK();
  return IsClauseConverted(extra.iClause);
}

BYTE LogCompStr::GetCompCharAttr(DWORD ich) const {
  FOOTMARK();
  BYTE ret = ATTR_INPUT;
  if (ich < (DWORD)comp_attr.size()) {
    ret = comp_attr[ich];
  }
  return ret;
}

DWORD LogCompStr::GetCompCharCount() const {
  FOOTMARK();
  return (DWORD)comp_str.size();
}

DWORD LogCompStr::ClauseToCompChar(DWORD dwClauseIndex) const {
  FOOTMARK();
  if (dwClauseIndex >= GetClauseCount()) return GetCompCharCount();
  return comp_clause[dwClauseIndex];
}

DWORD LogCompStr::CompCharToClause(DWORD iCompChar) const {
  FOOTMARK();
  DWORD dwClauseIndex = 0;
  const DWORD cClause = GetClauseCount();
  for (DWORD iClause = 0; iClause <= cClause; ++iClause) {
    if (comp_clause[iClause] <= iCompChar) {
      dwClauseIndex = iClause;
    } else {
      break;
    }
  }
  return dwClauseIndex;
}

std::wstring LogCompStr::GetClauseCompString(DWORD dwClauseIndex) const {
  FOOTMARK();
  std::wstring ret;
  if (dwClauseIndex + 1 <= GetClauseCount()) {
    DWORD ich0 = comp_clause[dwClauseIndex];
    DWORD ich1 = comp_clause[dwClauseIndex + 1];
    ret = comp_str.substr(ich0, ich1 - ich0);
  }
  return ret;
}

void LogCompStr::MergeAt(std::vector<std::wstring>& strs, DWORD istr) {
  FOOTMARK();
  strs[istr - 1] += strs[istr];
  strs.erase(strs.begin() + istr);
}

WCHAR LogCompStr::PrevCharInClause() const {
  FOOTMARK();
  if (dwCursorPos > 0) {
    if (CompCharInClause(dwCursorPos - 1, extra.iClause)) {
      return comp_str[dwCursorPos - 1];
    }
  }
  return 0;
}

void LogCompStr::UpdateExtraClause(DWORD iClause, DWORD dwConversion) {
  FOOTMARK();
  // fix extra info
  BOOL bRoman = (dwConversion & IME_CMODE_ROMAN);
  std::wstring str = extra.comp_str_clauses[iClause];
  str = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  if (bRoman) {
    extra.typing_clauses[iClause] =
      lcmap(hiragana_to_roman(str), LCMAP_HALFWIDTH);
    str = fullwidth_ascii_to_halfwidth(str);
    extra.hiragana_clauses[iClause] = roman_to_hiragana(str);
  } else {
    extra.hiragana_clauses[iClause] = str;
    extra.typing_clauses[iClause] =
      lcmap(hiragana_to_typing(str), LCMAP_HALFWIDTH);
  }
} // LogCompStr::UpdateExtraClause

void LogCompStr::UpdateCompStr() {
  FOOTMARK();
  std::wstring str;
  size_t ich = 0;
  size_t count = extra.comp_str_clauses.size();
  comp_clause.resize(count + 1);
  for (size_t i = 0; i < count; ++i) {
    comp_clause[i] = ich;
    str += extra.comp_str_clauses[i];
    ich += extra.comp_str_clauses[i].size();
  }
  comp_str = str;
  comp_clause[count] = ich;
}

void LogCompStr::UpdateFromExtra(BOOL bRoman) {
  UpdateCompStr();
  extra.typing_clauses.resize(extra.hiragana_clauses.size());
  if (bRoman) {
    for (DWORD i = 0; i < GetClauseCount(); ++i) {
      std::wstring& hira = extra.hiragana_clauses[i];
      extra.typing_clauses[i] =
        lcmap(hiragana_to_typing(hira), LCMAP_HALFWIDTH);
    }
  } else {
    for (DWORD i = 0; i < GetClauseCount(); ++i) {
      std::wstring& hira = extra.hiragana_clauses[i];
      extra.typing_clauses[i] =
        lcmap(hiragana_to_roman(hira), LCMAP_HALFWIDTH);
    }
  }
  comp_attr.assign(comp_str.size(), ATTR_CONVERTED);
}

void LogCompStr::MakeHiragana() {
  FOOTMARK();
  SetClauseCompString(extra.iClause,
    lcmap(extra.hiragana_clauses[extra.iClause], LCMAP_HIRAGANA));
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeKatakana() {
  FOOTMARK();
  SetClauseCompString(extra.iClause,
    lcmap(extra.hiragana_clauses[extra.iClause], LCMAP_KATAKANA));
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeHankaku() {
  FOOTMARK();
  SetClauseCompString(extra.iClause,
    lcmap(extra.hiragana_clauses[extra.iClause],
          LCMAP_HALFWIDTH | LCMAP_KATAKANA));
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeZenEisuu() {
  FOOTMARK();
  SetClauseCompString(extra.iClause,
    lcmap(extra.typing_clauses[extra.iClause], LCMAP_FULLWIDTH));
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeHanEisuu() {
  FOOTMARK();
  SetClauseCompString(extra.iClause,
    lcmap(extra.typing_clauses[extra.iClause], LCMAP_HALFWIDTH));
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::AddCharToEnd(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv) {
  FOOTMARK();
  BOOL bRoman = (dwConv & IME_CMODE_ROMAN);
  std::wstring str, typed, translated;
  typed += chTyped;
  translated += chTranslated;
  if (is_fullwidth_katakana(chTranslated)) {
    translated = lcmap(translated, LCMAP_HIRAGANA);
    chTranslated = translated[0];
  }
  int len = 0;
  INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConv);
  switch (imode) {
  case IMODE_FULL_HIRAGANA:
    if (is_hiragana(chTranslated)) {
      // set comp str and get delta length
      len = 1;
      extra.comp_str_clauses[extra.iClause] += chTranslated;
      // set hiragana
      extra.hiragana_clauses[extra.iClause] += chTranslated;
      // set typing
      if (chTyped == chTranslated) {
        if (bRoman) {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_roman(fullwidth_ascii_to_halfwidth(translated));
        } else {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_typing(translated);
        }
      } else {
        extra.typing_clauses[extra.iClause] += chTyped;
      }
    } else {
      // set comp str and get delta length
      str = extra.comp_str_clauses[extra.iClause];
      len = (int)str.size();
      str += chTyped;
      str = roman_to_hiragana(str, str.size());
      extra.comp_str_clauses[extra.iClause] = str;
      len = (int)str.size() - len;
      // set hiragana
      str = extra.hiragana_clauses[extra.iClause];
      str += chTyped;
      str = roman_to_hiragana(str, str.size());
      extra.hiragana_clauses[extra.iClause] = str;
      // set typing
      extra.typing_clauses[extra.iClause] += chTyped;
    }
    break;
  case IMODE_FULL_KATAKANA:
    if (is_hiragana(chTranslated)) {
      // set comp str and get delta length
      len = 1;
      extra.comp_str_clauses[extra.iClause] += lcmap(translated, LCMAP_KATAKANA);
      // set hiragana
      extra.hiragana_clauses[extra.iClause] += chTranslated;
      // set typing
      if (chTyped == chTranslated) {
        if (bRoman) {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_roman(fullwidth_ascii_to_halfwidth(translated));
        } else {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_typing(translated);
        }
      } else {
        extra.typing_clauses[extra.iClause] += typed;
      }
    } else {
      // set comp str and get delta length
      str = extra.comp_str_clauses[extra.iClause];
      len = (int)str.size();
      str += chTyped;
      str = roman_to_katakana(str, str.size());
      extra.comp_str_clauses[extra.iClause] = str;
      len = (int)str.size() - len;
      // set hiragana
      str = extra.hiragana_clauses[extra.iClause];
      str += chTyped;
      str = roman_to_hiragana(str, str.size());
      extra.hiragana_clauses[extra.iClause] = str;
      // set typing
      extra.typing_clauses[extra.iClause] += chTyped;
    }
    break;
  case IMODE_FULL_ASCII:
    if (is_hiragana(chTranslated)) {
      // set comp str and get delta length
      len = 1;
      extra.comp_str_clauses[extra.iClause] += chTranslated;
      // set hiragana
      extra.hiragana_clauses[extra.iClause] +=
        roman_to_hiragana(fullwidth_ascii_to_halfwidth(translated));
      // set typing
      if (chTyped == chTranslated) {
        if (bRoman) {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_roman(fullwidth_ascii_to_halfwidth(translated));
        } else {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_typing(translated);
        }
      } else {
        extra.typing_clauses[extra.iClause] += typed;
      }
    } else {
      // set comp str and get delta length
      str = extra.comp_str_clauses[extra.iClause];
      len = (int)str.size();
      str += chTyped;
      str = lcmap(str, LCMAP_FULLWIDTH);
      extra.comp_str_clauses[extra.iClause] = str;
      len = (int)str.size() - len;
      // set hiragana
      str = extra.hiragana_clauses[extra.iClause];
      str += chTyped;
      extra.hiragana_clauses[extra.iClause] =
        roman_to_hiragana(fullwidth_ascii_to_halfwidth(str));
      // set typing
      extra.typing_clauses[extra.iClause] += chTyped;
    }
    break;
  case IMODE_HALF_KANA:
    if (is_hiragana(chTranslated)) {
      // set comp str and get delta length
      len = (int)translated.size();
      extra.comp_str_clauses[extra.iClause] +=
        lcmap(translated, LCMAP_KATAKANA | LCMAP_HALFWIDTH);
      // set hiragana
      extra.hiragana_clauses[extra.iClause] += chTranslated;
      // set typing
      if (chTyped == chTranslated) {
        if (bRoman) {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_roman(fullwidth_ascii_to_halfwidth(translated));
        } else {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_typing(translated);
        }
      } else {
        extra.typing_clauses[extra.iClause] +=
          hiragana_to_typing(translated);
      }
    } else {
      // set comp str and get delta length
      str = extra.comp_str_clauses[extra.iClause];
      len = (int)str.size();
      str += chTyped;
      str = roman_to_halfwidth_katakana(str, str.size());
      len = (int)str.size() - len;
      // set hiragana
      extra.comp_str_clauses[extra.iClause] = str;
      str = extra.hiragana_clauses[extra.iClause];
      str += chTyped;
      str = roman_to_hiragana(str, str.size());
      extra.hiragana_clauses[extra.iClause] = str;
      // set typing
      extra.typing_clauses[extra.iClause] += chTyped;
    }
    break;
  case IMODE_HALF_ASCII:
  case IMODE_DISABLED:
    break;
  }
  dwCursorPos += len;
  UpdateCompStr();
} // LogCompStr::AddCharToEnd

void LogCompStr::InsertChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv) {
  FOOTMARK();
  std::wstring typed, translated;
  typed += chTyped;
  translated += chTranslated;
  DWORD dwIndexInClause = dwCursorPos - ClauseToCompChar(extra.iClause);
  if (is_fullwidth_katakana(chTranslated)) {
    translated = lcmap(translated, LCMAP_HIRAGANA);
    chTranslated = translated[0];
  }
  int len = 0;
  std::wstring str = extra.comp_str_clauses[extra.iClause];
  INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConv);
  switch (imode) {
  case IMODE_FULL_HIRAGANA:
    if (is_hiragana(chTranslated)) {
      str.insert(dwIndexInClause, translated);
      len = 1;
    } else {
      len = (int)str.size();
      str.insert(dwIndexInClause, typed);
      str = roman_to_hiragana(str);
      len = (int)str.size() - len;
    }
    break;
  case IMODE_FULL_KATAKANA:
    translated = lcmap(translated, LCMAP_KATAKANA);
    if (is_hiragana(chTranslated)) {
      str.insert(dwIndexInClause, translated);
      len = 1;
    } else {
      len = (int)str.size();
      str.insert(dwIndexInClause, typed);
      str = roman_to_katakana(str);
      len = (int)str.size() - len;
    }
    break;
  case IMODE_FULL_ASCII:
    translated = lcmap(translated, LCMAP_FULLWIDTH);
    if (is_hiragana(chTranslated)) {
      str.insert(dwIndexInClause, translated);
      len = 1;
    } else {
      len = (int)str.size();
      str.insert(dwIndexInClause, typed);
      str = lcmap(str, LCMAP_FULLWIDTH);
      len = (int)str.size() - len;
    }
    break;
  case IMODE_HALF_KANA:
    translated = lcmap(translated, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    if (is_hiragana(chTranslated)) {
      str.insert(dwIndexInClause, translated);
      len = 1;
    } else {
      len = (int)str.size();
      str.insert(dwIndexInClause, translated);
      str = roman_to_halfwidth_katakana(str);
      len = (int)str.size() - len;
    }
    break;
  case IMODE_HALF_ASCII:
  case IMODE_DISABLED:
    break;
  }
  extra.comp_str_clauses[extra.iClause] = str;
  dwCursorPos += len;
  UpdateCompStr();
  UpdateExtraClause(extra.iClause, dwConv);
} // LogCompStr::InsertChar

void
LogCompStr::AddDakuonChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv) {
  FOOTMARK();
  DWORD dwIndexInClause = dwCursorPos - ClauseToCompChar(extra.iClause);
  std::wstring str = extra.comp_str_clauses[extra.iClause];
  if (dwIndexInClause - 1 < str.size()) {
    str[dwIndexInClause - 1] = chTranslated;
  }
  extra.comp_str_clauses[extra.iClause] = str;
  UpdateCompStr();
  UpdateExtraClause(extra.iClause, dwConv);
}

void LogCompStr::AddChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv) {
  FOOTMARK();
  size_t size0 = comp_str.size();
  WCHAR ch = PrevCharInClause();
  if (ch) ch = dakuon_shori(ch, chTranslated);
  if (ch) {
    chTyped = '@';
    chTranslated = ch;
    AddDakuonChar(chTyped, chTranslated, dwConv);
  } else if (!CompCharInClause(dwCursorPos, extra.iClause, TRUE)) {
    AddCharToEnd(chTyped, chTranslated, dwConv);
  } else {
    InsertChar(chTyped, chTranslated, dwConv);
  }
  size_t size1 = comp_str.size();
  DWORD ich = ClauseToCompChar(extra.iClause);
  if (size0 < size1) {
    std::vector<BYTE> addition(size1 - size0);
    comp_attr.insert(comp_attr.begin() + ich, addition.begin(), addition.end());
  } else if (size1 < size0) {
    comp_attr.erase(comp_attr.begin() + ich,
                    comp_attr.begin() + ich + DWORD(size0 - size1));
  }
} // LogCompStr::AddChar

void LogCompStr::DeleteChar(BOOL bBackSpace/* = FALSE*/, DWORD dwConv) {
  FOOTMARK();
  // is the current clause being converted?
  if (IsClauseConverted()) { // being converted
    // set hiragana string to current clause
    extra.comp_str_clauses[extra.iClause] =
      extra.hiragana_clauses[extra.iClause];
    UpdateCompStr();
    SetClauseAttr(extra.iClause, ATTR_INPUT);
    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
  } else {  // not being converted
    BOOL flag = FALSE;
    // is it back space?
    if (bBackSpace) { // back space
      if (CompCharInClause(dwCursorPos - 1, extra.iClause)) {
        --dwCursorPos;  // move left
        flag = TRUE;
      }
    } else {  // not back space
      if (CompCharInClause(dwCursorPos, extra.iClause, TRUE)) {
        flag = TRUE;
      }
    }
    if (flag) {
      // erase the character
      DWORD ich = ClauseToCompChar(extra.iClause);
      DWORD delta = dwCursorPos - ich;
      extra.comp_str_clauses[extra.iClause].erase(delta, 1);
      // update extra clause
      UpdateExtraClause(extra.iClause, dwConv);
      // update composition string
      UpdateCompStr();
      // update comp_attr
      comp_attr.erase(comp_attr.begin() + ich);
    }
  }
} // LogCompStr::DeleteChar

void LogCompStr::RevertText() {
  FOOTMARK();
  // reset composition
  if (extra.iClause < GetClauseCount()) {
    // merge adjacent not converted clauses
    if (extra.iClause > 0) {
      if (!IsClauseConverted(extra.iClause - 1)) {
        MergeAt(extra.hiragana_clauses, extra.iClause);
        MergeAt(extra.typing_clauses, extra.iClause);
        MergeAt(extra.comp_str_clauses, extra.iClause);
        comp_clause.erase(comp_clause.begin() + extra.iClause);
        --extra.iClause;
      }
    }
    if (extra.iClause + 1 < GetClauseCount()) {
      if (!IsClauseConverted(extra.iClause + 1)) {
        MergeAt(extra.hiragana_clauses, extra.iClause + 1);
        MergeAt(extra.typing_clauses, extra.iClause + 1);
        MergeAt(extra.comp_str_clauses, extra.iClause + 1);
        comp_clause.erase(comp_clause.begin() + extra.iClause + 1);
      }
    }
    // compare old and new string
    std::wstring old_str = extra.comp_str_clauses[extra.iClause];
    std::wstring str = lcmap(extra.hiragana_clauses[extra.iClause],
                             LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
    DWORD ich = ClauseToCompChar(extra.iClause);
    if (old_str.size() < str.size()) {
      size_t diff = str.size() - old_str.size();
      std::vector<BYTE> addition(diff, ATTR_CONVERTED);
      comp_attr.insert(
        comp_attr.begin() + ich, addition.begin(), addition.end());
    } else if (old_str.size() > str.size()) {
      size_t diff = old_str.size() - str.size();
      comp_attr.erase(
        comp_attr.begin() + ich, comp_attr.begin() + ich + diff);
    }
    // update composition string
    extra.comp_str_clauses[extra.iClause] = str;
    UpdateCompStr();
    // set cursor position
    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
    // set delta start
    dwDeltaStart = ClauseToCompChar(extra.iClause);
    // untarget
    SetClauseAttr(extra.iClause, ATTR_INPUT);
  }
} // LogCompStr::RevertText

void LogCompStr::MakeResult() {
  FOOTMARK();

  // setting result_read_clause and result_read_str
  result_read_str.clear();
  const size_t count = extra.hiragana_clauses.size();
  result_read_clause.resize(count + 1);
  for (size_t i = 0; i < count; ++i) {
    result_read_clause[i] = DWORD(result_read_str.size());
    result_read_str +=
      lcmap(extra.hiragana_clauses[i], LCMAP_HALFWIDTH | LCMAP_KATAKANA);
  }
  result_read_clause[count] = DWORD(result_read_str.size());

  result_clause = comp_clause;
  result_str = comp_str;
  clear_read();
  clear_comp();
  clear_extra();
  fix();
}

BOOL LogCompStr::MoveLeft() {
  FOOTMARK();
  // is the current clause being converted?
  if (IsClauseConverted()) { // being converted
    // untarget
    SetClauseAttr(extra.iClause, ATTR_CONVERTED);
    // set the current clause
    if (extra.iClause > 0) {
      --extra.iClause;
    } else {
      extra.iClause = GetClauseCount() - 1;
    }
    // retarget
    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
    // move cursor
    dwCursorPos = GetCompCharCount();
    return TRUE;
  } else {
    // move cursor
    if (dwCursorPos > 0) {
      --dwCursorPos;
      // went out of clause?
      if (!CompCharInClause(dwCursorPos, extra.iClause)) {
        // set the current clause
        extra.iClause = CompCharToClause(dwCursorPos);
        // is the clause converted?
        if (IsClauseConverted()) {
          // retarget
          SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
          // move cursor
          dwCursorPos = GetCompCharCount();
          return TRUE;
        } else {
          // move cursor
          dwCursorPos = ClauseToCompChar(extra.iClause + 1);
        }
      }
    }
  }
  return FALSE;
} // LogCompStr::MoveLeft

BOOL LogCompStr::MoveRight() {
  FOOTMARK();
  // is the current clause being converted?
  if (IsClauseConverted()) { // being converted
    // untarget
    SetClauseAttr(extra.iClause, ATTR_CONVERTED);
    // set current clause
    ++extra.iClause;
    if (extra.iClause >= GetClauseCount()) { // exceeded
      extra.iClause = 0;
    }
    // retarget
    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
    // move cursor
    dwCursorPos = GetCompCharCount();
    return TRUE;
  } else {  // not being converted
    // move cursor
    if (dwCursorPos + 1 <= GetCompCharCount()) {
      ++dwCursorPos;
      // went out of clause?
      if (!CompCharInClause(dwCursorPos, extra.iClause)) {
        // set current clause
        extra.iClause = CompCharToClause(dwCursorPos);
        // is the clause converted?
        if (IsClauseConverted()) {
          // retarget
          SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
          // move cursor
          dwCursorPos = GetCompCharCount();
          return TRUE;
        } else {
          // move cursor
          dwCursorPos = ClauseToCompChar(extra.iClause + 1);
        }
      }
    }
  }
  return FALSE;
} // LogCompStr::MoveRight

BOOL LogCompStr::MoveHome() {
  FOOTMARK();
  // is the current clause being converted?
  if (IsClauseConverted()) { // being converted
    // untarget
    SetClauseAttr(extra.iClause, ATTR_CONVERTED);
    // set the current clause to first
    extra.iClause = 0;
    // retarget
    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
    // move cursor
    dwCursorPos = GetCompCharCount();
    return TRUE;
  } else {
    // move cursor to head
    dwCursorPos = 0;
    return FALSE;
  }
} // LogCompStr::MoveHome

BOOL LogCompStr::MoveEnd() {
  FOOTMARK();
  // is the current clause being converted?
  if (IsClauseConverted()) { // being converted
    // untarget
    SetClauseAttr(extra.iClause, ATTR_CONVERTED);
    // set the current clause to last
    extra.iClause = GetClauseCount() - 1;
    // retarget
    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
    // move cursor
    dwCursorPos = GetCompCharCount();
    return TRUE;
  } else {
    // move cursor to tail
    dwCursorPos = GetCompCharCount();
    return FALSE;
  }
} // LogCompStr::MoveEnd

DWORD LogCompStr::GetClauseCompStrLen(DWORD dwClauseIndex) const {
  return (DWORD)extra.comp_str_clauses[dwClauseIndex].size();
}

void LogCompStr::SetClauseCompString(DWORD iClause, std::wstring& str) {
  FOOTMARK();
  if (iClause < GetClauseCount()) {
    // fix comp_attr
    std::wstring old_str = extra.comp_str_clauses[iClause];
    DWORD ich = ClauseToCompChar(iClause);
    if (old_str.size() < str.size()) {
      size_t diff = str.size() - old_str.size();
      std::vector<BYTE> addition(diff, ATTR_CONVERTED);
      comp_attr.insert(
        comp_attr.begin() + ich, addition.begin(), addition.end());
    } else if (old_str.size() > str.size()) {
      size_t diff = old_str.size() - str.size();
      comp_attr.erase(
        comp_attr.begin() + ich, comp_attr.begin() + ich + diff);
    }
    // update comp str
    extra.comp_str_clauses[iClause] = str;
    UpdateCompStr();
    // untarget
    SetClauseAttr(extra.iClause, ATTR_CONVERTED);
    extra.iClause = iClause;
    // set target
    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
    // update cursor pos
    dwCursorPos = (DWORD)comp_str.size();
  }
} // LogCompStr::SetClauseCompString

void LogCompStr::SetClauseCompHiragana(DWORD iClause, std::wstring& str) {
  FOOTMARK();
  if (iClause < GetClauseCount()) {
    extra.hiragana_clauses[iClause] = str;
  }
}

void LogCompStr::SetClauseCompHiragana(
  DWORD iClause, std::wstring& str, BOOL bRoman)
{
  FOOTMARK();
  if (iClause < GetClauseCount()) {
    if (bRoman) {
      extra.typing_clauses[iClause] =
        hiragana_to_roman(fullwidth_ascii_to_halfwidth(str));
    } else {
      extra.typing_clauses[iClause] = hiragana_to_typing(str);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

DWORD CompStr::Store(const LogCompStr *log) {
  const DWORD total = log->GetTotalSize();

  BYTE *pb = GetBytes();
  dwSize = total;
  dwCursorPos = log->dwCursorPos;
  dwDeltaStart = log->dwDeltaStart;
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
    memcpy(pb, &log->member[0], log->member.size() * sizeof(WCHAR)); \
    pb += log->member.size() * sizeof(WCHAR); \
  }

  dwCompReadAttrOffset = DWORD(pb - GetBytes());
  dwCompReadAttrLen = log->comp_read_attr.size() * sizeof(BYTE);
  ADD_BYTES(comp_read_attr);

  dwCompReadClauseOffset = DWORD(pb - GetBytes());
  dwCompReadClauseLen = log->comp_read_clause.size() * sizeof(DWORD);
  ADD_DWORDS(comp_read_clause);

  dwCompReadStrOffset = DWORD(pb - GetBytes());
  dwCompReadStrLen = log->comp_read_str.size();
  ADD_STRING(comp_read_str);

  dwCompAttrOffset = DWORD(pb - GetBytes());
  dwCompAttrLen = log->comp_attr.size() * sizeof(BYTE);
  ADD_BYTES(comp_attr);

  dwCompClauseOffset = DWORD(pb - GetBytes());
  dwCompClauseLen = log->comp_clause.size() * sizeof(DWORD);
  ADD_DWORDS(comp_clause);

  dwCompStrOffset = DWORD(pb - GetBytes());
  dwCompStrLen = log->comp_str.size();
  ADD_STRING(comp_str);

  dwResultReadClauseOffset = DWORD(pb - GetBytes());
  dwResultReadClauseLen = log->result_read_clause.size() * sizeof(DWORD);
  ADD_DWORDS(result_read_clause);

  dwResultReadStrOffset = DWORD(pb - GetBytes());
  dwResultReadStrLen = log->result_read_str.size();
  ADD_STRING(result_read_str);

  dwResultClauseOffset = DWORD(pb - GetBytes());
  dwResultClauseLen = log->result_clause.size() * sizeof(DWORD);
  ADD_DWORDS(result_clause);

  dwResultStrOffset = DWORD(pb - GetBytes());
  dwResultStrLen = log->result_str.size();
  ADD_STRING(result_str);

  COMPSTREXTRA *pExtra = (COMPSTREXTRA *)pb;
  dwPrivateSize = log->extra.GetTotalSize();
  dwPrivateOffset = DWORD(pb - GetBytes());
  pb += pExtra->Store(&log->extra);

#undef ADD_BYTES
#undef ADD_DWORDS
#undef ADD_STRING
  assert(DWORD(pb - GetBytes()) == total);

  return DWORD(pb - GetBytes());
} // CompStr::Store

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
    log.fix();
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
      DWORD size = lpCompStr->Store(log);
      assert(size == total);

      ::ImmUnlockIMCC(hNewCompStr);
      hCompStr = hNewCompStr;
    } else {
      DebugPrintA("CompStr::ReCreate: failed #2");
      assert(0);
    }
  } else {
    DebugPrintA("CompStr::ReCreate: failed");
    assert(0);
  }
  return hCompStr;
} // CompStr::ReCreate

// extension
COMPSTREXTRA *CompStr::GetExtra() {
  if (dwPrivateSize > sizeof(COMPSTREXTRA)) {
    BYTE *pb = GetBytes();
    pb += dwPrivateOffset;
    COMPSTREXTRA *pExtra = (COMPSTREXTRA *)pb;
    if (pExtra->dwSignature == 0xDEADFACE) {
      return pExtra;
    }
  }
  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// for debugging

void LogCompStr::AssertValid() {
  if (dwCursorPos > GetCompCharCount()) {
    Dump();
    DebugPrintA("dwCursorPos: %u\n", dwCursorPos);
    DebugPrintA("GetCompCharCount(): %u\n", GetCompCharCount());
    assert(0);
  }
  if (comp_attr.size()) {
    if (comp_attr.size() != comp_str.size()) {
      Dump();
      DebugPrintA("comp_attr.size(): %u\n", (int)comp_attr.size());
      DebugPrintA("comp_str.size(): %u\n", (int)comp_str.size());
      assert(0);
    }
  }
  if (comp_clause.size()) {
    if (comp_clause[0] != 0) {
      Dump();
      assert(0);
    }
    if (comp_clause[comp_clause.size() - 1] != GetCompCharCount()) {
      Dump();
      assert(0);
    }
    if (extra.iClause > (DWORD)comp_clause.size()) {
      Dump();
      DebugPrintA("extra.iClause: %u\n", extra.iClause);
      DebugPrintA("comp_clause.size(): %u\n", (int)comp_clause.size());
      assert(0);
    }
    for (size_t i = 1; i < comp_clause.size(); ++i) {
      if (comp_clause[i] > GetCompCharCount()) {
        Dump();
        assert(0);
      }
      if (comp_clause[i - 1] > comp_clause[i]) {
        Dump();
        assert(0);
      }
    }
  }
  if (result_read_clause.size()) {
    if (result_read_clause[0] != 0) {
      Dump();
      assert(0);
    }
    for (size_t i = 1; i < result_read_clause.size(); ++i) {
      if (result_read_clause[i] > (DWORD)result_read_str.size()) {
        Dump();
        assert(0);
      }
      if (result_read_clause[i - 1] > result_read_clause[i]) {
        Dump();
        assert(0);
      }
    }
  }
  if (result_clause.size()) {
    if (result_clause[0] != 0) {
      Dump();
      assert(0);
    }
    for (size_t i = 1; i < result_clause.size(); ++i) {
      if (result_clause[i] > (DWORD)result_str.size()) {
        Dump();
        assert(0);
      }
      if (result_clause[i - 1] > result_clause[i]) {
        Dump();
        assert(0);
      }
    }
  }
  if (extra.hiragana_clauses.size() != extra.typing_clauses.size()) {
    Dump();
    assert(0);
  }
} // LogCompStr::AssertValid

void CompStr::Dump() {
  FOOTMARK();
#ifndef NDEBUG
  DebugPrintA("dwSize: %08X\n", dwSize);
  DebugPrintA("dwCursorPos: %08X\n", dwCursorPos);
  DebugPrintA("dwDeltaStart: %08X\n", dwDeltaStart);
  DebugPrintA("CompReadAttr: ");
  if (dwCompReadAttrLen) {
    BYTE *attrs = GetCompReadAttr();
    for (DWORD i = 0; i < dwCompReadAttrLen; ++i) {
      DebugPrintA("%02X ", attrs[i]);
    }
  }
  DebugPrintA("\n");
  DebugPrintA("CompReadClause: ");
  if (dwCompReadClauseLen) {
    DWORD *clauses = GetCompReadClause();
    for (DWORD i = 0; i < dwCompReadClauseLen / 4; ++i) {
      DebugPrintA("%08X ", clauses[i]);
    }
  }
  DebugPrintA("\n");
  DebugPrintA("CompReadStr: ");
  if (dwCompReadStrLen) {
    WCHAR *str = GetCompReadStr();
    DebugPrintA("%ls", str);
  }
  DebugPrintA("\n");
  DebugPrintA("CompAttr: ");
  if (dwCompAttrLen) {
    BYTE *attrs = GetCompAttr();
    for (DWORD i = 0; i < dwCompAttrLen; ++i) {
      DebugPrintA("%02X ", attrs[i]);
    }
  }
  DebugPrintA("\n");
  DebugPrintA("CompClause: ");
  if (dwCompClauseLen) {
    DWORD *clauses = GetCompClause();
    for (DWORD i = 0; i < dwCompClauseLen / 4; ++i) {
      DebugPrintA("%08X ", clauses[i]);
    }
  }
  DebugPrintA("\n");
  DebugPrintA("CompStr: ");
  if (dwCompStrLen) {
    WCHAR *str = GetCompStr();
    DebugPrintA("%ls", str);
  }
  DebugPrintA("\n");
  DebugPrintA("ResultReadClause: ");
  if (dwResultReadClauseLen) {
    DWORD *clauses = GetResultReadClause();
    for (DWORD i = 0; i < dwResultReadClauseLen / 4; ++i) {
      DebugPrintA("%08X ", clauses[i]);
    }
  }
  DebugPrintA("\n");
  DebugPrintA("ResultReadStr: ");
  if (dwResultReadStrLen) {
    WCHAR *str = GetResultReadStr();
    DebugPrintA("%ls", str);
  }
  DebugPrintA("\n");
  DebugPrintA("ResultClause: ");
  if (dwResultClauseLen) {
    DWORD *clauses = GetResultClause();
    for (DWORD i = 0; i < dwResultClauseLen / 4; ++i) {
      DebugPrintA("%08X ", clauses[i]);
    }
  }
  DebugPrintA("\n");
  DebugPrintA("ResultStr: ");
  if (dwResultStrLen) {
    WCHAR *str = GetResultStr();
    DebugPrintA("%ls", str);
  }
  DebugPrintA("\n");
#endif  // ndef NDEBUG
} // CompStr::Dump

void LogCompStr::Dump() {
#ifndef NDEBUG
  DebugPrintA("### LogCompStr ###\n");
  DebugPrintA("+ dwCursorPos: %08X\n", dwCursorPos);
  DebugPrintA("+ dwDeltaStart: %08X\n", dwDeltaStart);

  DebugPrintA("+ comp_read_attr: ");
  for (size_t i = 0; i < comp_read_attr.size(); ++i) {
    DebugPrintA("%02X ", comp_read_attr[i]);
  }
  DebugPrintA("\n");

  DebugPrintA("+ comp_read_clause: ");
  for (size_t i = 0; i < comp_read_clause.size(); ++i) {
    DebugPrintA("%08X ", comp_read_clause[i]);
  }
  DebugPrintA("\n");

  DebugPrintA("+ comp_read_str: %ls\n", comp_read_str.c_str());
  DebugPrintA("+ comp_attr: ");
  for (size_t i = 0; i < comp_attr.size(); ++i) {
    DebugPrintA("%02X ", comp_attr[i]);
  }
  DebugPrintA("\n");

  DebugPrintA("+ comp_clause: ");
  for (size_t i = 0; i < comp_clause.size(); ++i) {
    DebugPrintA("%08X ", comp_clause[i]);
  }
  DebugPrintA("\n");

  DebugPrintA("+ comp_str: %ls\n", comp_str.c_str());

  DebugPrintA("+ result_read_clause: ");
  for (size_t i = 0; i < result_read_clause.size(); ++i) {
    DebugPrintA("%08X ", result_read_clause[i]);
  }
  DebugPrintA("\n");

  DebugPrintA("+ result_read_str: %ls\n", result_read_str.c_str());

  DebugPrintA("+ result_clause: ");
  for (size_t i = 0; i < result_clause.size(); ++i) {
    DebugPrintA("%08X ", result_clause[i]);
  }
  DebugPrintA("\n");

  DebugPrintA("+ result_str: %ls\n", result_str.c_str());

  DebugPrintA("+ extra.iClause: %08X\n", extra.iClause);

  DebugPrintA("+ extra.hiragana_clauses: ");
  for (size_t i = 0; i < extra.hiragana_clauses.size(); ++i) {
    DebugPrintA("%ls ", extra.hiragana_clauses[i].c_str());
  }
  DebugPrintA("\n");

  DebugPrintA("+ extra.typing_clauses: ");
  for (size_t i = 0; i < extra.typing_clauses.size(); ++i) {
    DebugPrintA("%ls ", extra.typing_clauses[i].c_str());
  }
  DebugPrintA("\n");
#endif  // ndef NDEBUG
} // LogCompStr::Dump

//////////////////////////////////////////////////////////////////////////////
