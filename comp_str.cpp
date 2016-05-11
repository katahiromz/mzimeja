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

LPWSTR COMPSTREXTRA::GetHiraganaClauses(DWORD& dwCount) {
  FOOTMARK();
  dwCount = dwHiraganaClauseCount;
  if (dwCount) {
    return (LPWSTR)(GetBytes() + dwHiraganaClauseOffset);
  }
  return NULL;
}

LPWSTR COMPSTREXTRA::GetTypingClauses(DWORD& dwCount) {
  FOOTMARK();
  dwCount = dwTypingClauseCount;
  if (dwCount) {
    return (LPWSTR)(GetBytes() + dwTypingClauseOffset);
  }
  return NULL;
}

void COMPSTREXTRA::GetLog(LogCompStrExtra& log) {
  FOOTMARK();
  log.clear();
  log.iClause = iClause;

  DWORD dwCount;
  LPWSTR pch = GetHiraganaClauses(dwCount);
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

  LPBYTE pb = GetBytes();
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

BOOL LogCompStr::IsBeingConverted() {
  FOOTMARK();
  for (size_t i = 0; i < comp_attr.size(); ++i) {
    if (comp_attr[i] != ATTR_INPUT) return TRUE;
  }
  return FALSE;
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
  if (dwClauseIndex + 1 <= GetClauseCount()) {
    DWORD ich0 = comp_clause[dwClauseIndex];
    DWORD ich1 = comp_clause[dwClauseIndex + 1];
    for (DWORD i = ich0; i < ich1; ++i) {
      comp_attr[i] = attr;
    }
  }
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
    str = zenkaku_eisuu_to_hankaku(str);
    extra.hiragana_clauses[iClause] = roman_to_hiragana(str);
  } else {
    extra.hiragana_clauses[iClause] = str;
    extra.typing_clauses[iClause] =
      lcmap(hiragana_to_typing(str), LCMAP_HALFWIDTH);
  }
} // LogCompStr::UpdateExtraClause

void LogCompStr::UpdateCompStr() {
  FOOTMARK();
  comp_str.clear();
  size_t ich = 0;
  size_t count = extra.comp_str_clauses.size();
  comp_clause.resize(count + 1);
  for (size_t i = 0; i < count; ++i) {
    comp_clause[i] = ich;
    comp_str += extra.comp_str_clauses[i];
    ich += extra.comp_str_clauses[i].size();
  }
  comp_clause[count] = ich;
}

void LogCompStr::MakeHiragana() {
  FOOTMARK();
  extra.comp_str_clauses[extra.iClause] =
    lcmap(extra.hiragana_clauses[extra.iClause], LCMAP_HIRAGANA);
  UpdateCompStr();
  SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeKatakana() {
  FOOTMARK();
  extra.comp_str_clauses[extra.iClause] =
    lcmap(extra.hiragana_clauses[extra.iClause], LCMAP_KATAKANA);
  UpdateCompStr();
  SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeHankaku() {
  FOOTMARK();
  extra.comp_str_clauses[extra.iClause] =
    lcmap(extra.hiragana_clauses[extra.iClause],
          LCMAP_HALFWIDTH | LCMAP_KATAKANA);
  UpdateCompStr();
  SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeZenEisuu() {
  FOOTMARK();
  extra.comp_str_clauses[extra.iClause] =
    lcmap(extra.typing_clauses[extra.iClause], LCMAP_FULLWIDTH);
  UpdateCompStr();
  SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::MakeHanEisuu() {
  FOOTMARK();
  extra.comp_str_clauses[extra.iClause] =
    lcmap(extra.typing_clauses[extra.iClause], LCMAP_HALFWIDTH);
  UpdateCompStr();
  SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
  dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

void LogCompStr::AddCharToEnd(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv) {
  FOOTMARK();
  BOOL bRoman = (dwConv & IME_CMODE_ROMAN);
  std::wstring str, typed, translated;
  typed += chTyped;
  translated += chTranslated;
  if (is_zenkaku_katakana(chTranslated)) {
    translated = lcmap(translated, LCMAP_HIRAGANA);
    chTranslated = translated[0];
  }
  int len = 0;
  INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConv);
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
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
            hiragana_to_roman(zenkaku_eisuu_to_hankaku(translated));
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
  case IMODE_ZEN_KATAKANA:
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
            hiragana_to_roman(zenkaku_eisuu_to_hankaku(translated));
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
  case IMODE_ZEN_EISUU:
    if (is_hiragana(chTranslated)) {
      // set comp str and get delta length
      len = 1;
      extra.comp_str_clauses[extra.iClause] += chTranslated;
      // set hiragana
      extra.hiragana_clauses[extra.iClause] +=
        roman_to_hiragana(zenkaku_eisuu_to_hankaku(translated));
      // set typing
      if (chTyped == chTranslated) {
        if (bRoman) {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_roman(zenkaku_eisuu_to_hankaku(translated));
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
        roman_to_hiragana(zenkaku_eisuu_to_hankaku(str));
      // set typing
      extra.typing_clauses[extra.iClause] += chTyped;
    }
    break;
  case IMODE_HAN_KANA:
    if (is_hiragana(chTranslated)) {
      // set comp str and get delta length
      len = (int)translated.size();
      extra.comp_str_clauses[extra.iClause] += translated;
      // set hiragana
      extra.hiragana_clauses[extra.iClause] += translated;
      // set typing
      if (bRoman) {
        if (bRoman) {
          extra.typing_clauses[extra.iClause] +=
            hiragana_to_roman(zenkaku_eisuu_to_hankaku(translated));
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
      str += chTranslated;
      str = roman_to_hankaku_katakana(str, str.size());
      len = (int)str.size() - len;
      // set hiragana
      extra.comp_str_clauses[extra.iClause] = str;
      str = extra.hiragana_clauses[extra.iClause];
      str += chTranslated;
      str = roman_to_hiragana(str, str.size());
      extra.hiragana_clauses[extra.iClause] = str;
      // set typing
      extra.typing_clauses[extra.iClause] += chTyped;
    }
    break;
  case IMODE_HAN_EISUU:
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
  if (is_zenkaku_katakana(chTranslated)) {
    translated = lcmap(translated, LCMAP_HIRAGANA);
    chTranslated = translated[0];
  }
  int len = 0;
  std::wstring str = extra.comp_str_clauses[extra.iClause];
  INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConv);
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
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
  case IMODE_ZEN_KATAKANA:
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
  case IMODE_ZEN_EISUU:
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
  case IMODE_HAN_KANA:
    translated = lcmap(translated, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    if (is_hiragana(chTranslated)) {
      str.insert(dwIndexInClause, translated);
      len = 1;
    } else {
      len = (int)str.size();
      str.insert(dwIndexInClause, translated);
      str = roman_to_hankaku_katakana(str);
      len = (int)str.size() - len;
    }
    break;
  case IMODE_HAN_EISUU:
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
} // LogCompStr::AddChar

void LogCompStr::DeleteChar(BOOL bBackSpace/* = FALSE*/, DWORD dwConv) {
  FOOTMARK();
  // is the current clause being converted?
  if (IsClauseConverted(extra.iClause)) { // being converted
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
      DWORD dwIndex = dwCursorPos - ClauseToCompChar(extra.iClause);
      extra.comp_str_clauses[extra.iClause].erase(dwIndex, 1);
      // update extra clause
      UpdateExtraClause(extra.iClause, dwConv);
      // update composition string
      UpdateCompStr();
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
    // update composition string
    extra.comp_str_clauses[extra.iClause] =
      lcmap(extra.hiragana_clauses[extra.iClause],
            LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
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
  result_read_clause = comp_read_clause;  // TODO: generate reading
  result_read_str = comp_read_str;
  result_clause = comp_clause;
  result_str = comp_str;
  clear_read();
  clear_comp();
  clear_extra();
}

void LogCompStr::MoveLeft(BOOL bShift) {
  FOOTMARK();
  // TODO: bShift
  // is the current clause being converted?
  if (IsClauseConverted(extra.iClause)) { // being converted
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
  } else {
    // move cursor
    if (dwCursorPos > 0) {
      --dwCursorPos;
      // went out of clause?
      if (!CompCharInClause(dwCursorPos, extra.iClause)) {
        // set the current clause
        extra.iClause = CompCharToClause(dwCursorPos);
        // is the clause converted?
        if (IsClauseConverted(extra.iClause)) {
          // retarget
          SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
          // move cursor
          dwCursorPos = GetCompCharCount();
        } else {
          // move cursor
          dwCursorPos = ClauseToCompChar(extra.iClause + 1);
        }
      }
    }
  }
} // LogCompStr::MoveLeft

void LogCompStr::MoveRight(BOOL bShift) {
  FOOTMARK();
  // TODO: bShift
  // is the current clause being converted?
  if (IsClauseConverted(extra.iClause)) { // being converted
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
  } else {  // not being converted
    // move cursor
    if (dwCursorPos + 1 <= GetCompCharCount()) {
      ++dwCursorPos;
      // went out of clause?
      if (!CompCharInClause(dwCursorPos, extra.iClause)) {
        // set current clause
        extra.iClause = CompCharToClause(dwCursorPos);
        // is the clause converted?
        if (IsClauseConverted(extra.iClause)) {
          // retarget
          SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
          // move cursor
          dwCursorPos = GetCompCharCount();
        } else {
          // move cursor
          dwCursorPos = ClauseToCompChar(extra.iClause + 1);
        }
      }
    }
  }
} // LogCompStr::MoveRight

DWORD LogCompStr::GetClauseCompStrLen(DWORD dwClauseIndex) const {
  return (DWORD)extra.comp_str_clauses[dwClauseIndex].size();
}

void LogCompStr::SetClauseCompString(DWORD iClause, std::wstring& str) {
  FOOTMARK();
  if (iClause < GetClauseCount()) {
    extra.comp_str_clauses[iClause] = str;
    UpdateCompStr();
    SetClauseAttr(extra.iClause, ATTR_CONVERTED);
    extra.iClause = iClause;
    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
  }
}

//////////////////////////////////////////////////////////////////////////////

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

    log.extra.comp_str_clauses.clear();
    size_t count = log.comp_clause.size();
    if (count >= 1) {
      std::wstring str;
      for (size_t i = 0; i < count - 1; ++i) {
        str = log.GetClauseCompString(i);
        log.extra.comp_str_clauses.push_back(str);
      }
    } else {
      log.extra.comp_str_clauses.push_back(L"");
    }
    if (log.extra.hiragana_clauses.empty()) {
      log.extra.hiragana_clauses.push_back(L"");
    }
    if (log.extra.typing_clauses.empty()) {
      log.extra.typing_clauses.push_back(L"");
    }
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

#undef ADD_BYTES
#undef ADD_DWORDS
#undef ADD_STRING

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

//////////////////////////////////////////////////////////////////////////////
// for debugging

void LogCompStr::AssertValid() {
  if (dwCursorPos > GetCompCharCount()) {
    Dump();
    assert(0);
  }
  if (comp_attr.size()) {
    if (comp_attr.size() != comp_str.size()) {
      Dump();
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
    if (result_read_clause[result_read_clause.size() - 1] != 0) {
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

  DebugPrint(TEXT("+ result_str: %ls\n"), result_str.c_str());

  DebugPrint(TEXT("+ extra.iClause: %08X\n"), extra.iClause);

  DebugPrint(TEXT("+ extra.hiragana_clauses: "));
  for (size_t i = 0; i < extra.hiragana_clauses.size(); ++i) {
    DebugPrint(TEXT("%ls "), extra.hiragana_clauses[i].c_str());
  }
  DebugPrint(TEXT("\n"));

  DebugPrint(TEXT("+ extra.typing_clauses: "));
  for (size_t i = 0; i < extra.typing_clauses.size(); ++i) {
    DebugPrint(TEXT("%ls "), extra.typing_clauses[i].c_str());
  }
  DebugPrint(TEXT("\n"));
#endif  // ndef NDEBUG
}

//////////////////////////////////////////////////////////////////////////////
