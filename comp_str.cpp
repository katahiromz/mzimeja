// comp_str.cpp --- composition string of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

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
  dwSelectedClause = 0xFFFFFFFF;
  dwSelectedPhoneme = 0;
  hiragana_phonemes.clear();
  typing_phonemes.clear();
  phoneme_clauses.clear();
}

std::wstring
LogCompStrExtra::JoinLeft(const std::vector<SmallWString>& strs) const {
  std::wstring str;
  for (DWORD i = 0; i < dwSelectedPhoneme; ++i) {
    str += strs[i].c_str();
  }
  return str;
}

std::wstring
LogCompStrExtra::JoinRight(const std::vector<SmallWString>& strs) const {
  std::wstring str;
  const DWORD dwCount = (DWORD)strs.size();
  for (DWORD i = dwSelectedPhoneme; i < dwCount; ++i) {
    str += strs[i].c_str();
  }
  return str;
}

DWORD LogCompStrExtra::GetPhonemeCount() const {
  assert(hiragana_phonemes.size() == typing_phonemes.size());
  return (DWORD)hiragana_phonemes.size();
}

void LogCompStrExtra::InsertThere(
  std::vector<SmallWString>& strs, std::wstring& str)
{
  strs.insert(strs.begin() + dwSelectedPhoneme, str.c_str());
}

WCHAR LogCompStrExtra::GetPrevChar() const {
  if (dwSelectedPhoneme > 0) {
    assert(hiragana_phonemes[dwSelectedPhoneme - 1].size());
    return hiragana_phonemes[dwSelectedPhoneme - 1].back();
  }
  return 0;
}

std::wstring
LogCompStrExtra::Join(const std::vector<SmallWString>& strs) const {
  std::wstring str;
  for (size_t i = 0; i < strs.size(); ++i) {
    str += strs[i].c_str();
  }
  return str;
}

//////////////////////////////////////////////////////////////////////////////

void COMPSTREXTRA::GetLog(LogCompStrExtra& log) {
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
  assert(this);
  assert(log);
  dwSignature = 0xDEADFACE;
  dwSelectedClause = log->dwSelectedClause;
  dwSelectedPhoneme = log->dwSelectedPhoneme;

  LPBYTE pb = (LPBYTE)this;
  pb += sizeof(COMPSTREXTRA);

  DWORD size, dwCount;
  LPWSTR pch;

  // calculate size and offset of hiragana phonemes
  dwHiraganaPhonemeOffset = (DWORD)(pb - (LPBYTE)this);
  size = 0;
  for (size_t i = 0; i < log->hiragana_phonemes.size(); ++i) {
    size += (log->hiragana_phonemes[i].size() + 1);
  }
  dwHiraganaPhonemeLen = size * sizeof(WCHAR);
  pb += dwHiraganaPhonemeLen;

  // calculate size and offset of typing phonemes
  dwTypingPhonemeOffset = (DWORD)(pb - (LPBYTE)this);
  size = 0;
  for (size_t i = 0; i < log->typing_phonemes.size(); ++i) {
    size += (log->typing_phonemes[i].size() + 1);
  }
  dwTypingPhonemeLen = size * sizeof(WCHAR);
  pb += dwTypingPhonemeLen;

  // store hiragana phonemes
  pch = GetHiraganaPhonemes(dwCount);
  for (size_t i = 0; i < log->hiragana_phonemes.size(); ++i) {
    size_t size = (log->hiragana_phonemes[i].size() + 1);
    memcpy(pch, &log->hiragana_phonemes[i][0], size * sizeof(WCHAR));
    pch += size;
  }

  // store typing phonemes
  pch = GetTypingPhonemes(dwCount);
  for (size_t i = 0; i < log->typing_phonemes.size(); ++i) {
    size_t size = (log->typing_phonemes[i].size() + 1);
    memcpy(pch, &log->typing_phonemes[i][0], size * sizeof(WCHAR));
    pch += size;
  }

  assert(log->GetTotalSize() == (DWORD)(pb - (LPBYTE)this));
  return (DWORD)(pb - (LPBYTE)this);
} // COMPSTREXTRA::Store


//////////////////////////////////////////////////////////////////////////////

void LogCompStr::clear() {
  dwCursorPos = 0;
  clear_read();
  clear_comp();
  clear_result();
  clear_extra();
}

BOOL LogCompStr::HasClauseSelected() const {
  if (comp_clause.size() && extra.dwSelectedClause != 0xFFFFFFFF) {
    assert(extra.dwSelectedClause < GetClauseCount());
    return TRUE;
  }
  return FALSE;
}

BYTE LogCompStr::GetCharAttr(DWORD ich) const {
  if (ich < (DWORD)comp_attr.size()) {
    return comp_attr[ich];
  }
  return ATTR_INPUT;
}

DWORD LogCompStr::GetCurrentClause() const {
  if (extra.dwSelectedClause != 0xFFFFFFFF)
    return extra.dwSelectedClause;
  return 0;
}

DWORD LogCompStr::ClauseToPhoneme(DWORD iClause) const {
  if (iClause == 0xFFFFFFFF) return CompCharToPhoneme(dwCursorPos);
  assert(iClause < (DWORD)extra.phoneme_clauses.size());
  return extra.phoneme_clauses[iClause];
}

DWORD LogCompStr::PhonemeToClause(DWORD iPhoneme) const {
  DWORD ich = PhonemeToCompChar(iPhoneme);
  return CompCharToClause(ich);
}

DWORD LogCompStr::ClauseToCompChar(DWORD iClause) const {
  if (iClause == 0xFFFFFFFF) return GetCharCount();
  assert(iClause < (DWORD)comp_clause.size());
  return comp_clause[iClause];
}

DWORD LogCompStr::CompCharToClause(DWORD iCompChar) const {
  DWORD iClause = 0xFFFFFFFF;
  for (size_t i = 0; i < comp_clause.size(); ++i) {
    if (iCompChar < comp_clause[i]) {
      iClause = (DWORD)comp_clause[i];
    }
  }
  return iClause;
}

DWORD LogCompStr::PhonemeToCompChar(DWORD iPhoneme) const {
  assert(iPhoneme < (DWORD)comp_clause.size());
  return comp_clause[iPhoneme];
}

DWORD LogCompStr::CompCharToPhoneme(DWORD iCompChar) const {
  DWORD iPhoneme = 0;
  for (DWORD i = 0; i < (DWORD)comp_clause.size(); ++i) {
    if (iCompChar < comp_clause[i]) {
      iPhoneme = i;
    }
  }
  return iPhoneme;
}

DWORD LogCompStr::GetClauseCount() const {
  if (comp_clause.size() < 2) return 0;
  return (DWORD)(comp_clause.size() - 1);
}

void LogCompStr::clear_read() {
  comp_read_attr.clear();
  comp_read_clause.clear();
  comp_read_str.clear();
}

void LogCompStr::clear_comp() {
  comp_attr.clear();
  comp_clause.clear();
  comp_str.clear();
}

void LogCompStr::clear_result() {
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

void LogCompStr::SetClauseString(
  DWORD iClause, std::wstring& strNew, BOOL bConverted)
{
  assert(iClause < GetClauseCount());

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
} // LogCompStr::SetClauseString

std::wstring LogCompStr::GetLeft(DWORD iClause) const {
  assert(iClause < (DWORD)comp_clause.size());
  DWORD ich = comp_clause[iClause];
  return comp_str.substr(0, ich);
}

std::wstring LogCompStr::GetRight(DWORD iClause) const {
  assert(iClause < (DWORD)comp_clause.size());
  DWORD ich = comp_clause[iClause];
  return comp_str.substr(ich);
}

DWORD LogCompStr::GetPhonemeCount() const {
  return extra.GetPhonemeCount();
}

std::wstring LogCompStr::GetClauseCompString(DWORD iClause) const {
  std::wstring ret;
  assert(iClause != 0xFFFFFFFF);
  if (iClause + 1 < (DWORD)comp_clause.size()) {
    DWORD ich = comp_clause[iClause], ichNext = comp_clause[iClause + 1];
    ret = comp_str.substr(ich, ichNext - ich);
  }
  return ret;
}

std::wstring LogCompStr::GetClauseHiraganaString(DWORD iClause) const {
  std::wstring ret;
  assert(iClause != 0xFFFFFFFF);
  if (iClause + 1 < GetPhonemeCount()) {
    DWORD iph = extra.phoneme_clauses[i];
    DWORD iphNext = extra.phoneme_clauses[i + 1];
    for (DWORD i = iph; i < iphNext; ++i) {
      ret += extra.hiragana_phonemes[i];
    }
  }
  return ret;
}

std::wstring LogCompStr::GetClauseTypingString(DWORD iClause) const {
  std::wstring ret;
  assert(iClause != 0xFFFFFFFF);
  if (iClause + 1 < GetPhonemeCount()) {
    DWORD iph = extra.typing_phonemes[i];
    DWORD iphNext = extra.typing_phonemes[i + 1];
    for (DWORD i = iph; i < iphNext; ++i) {
      ret += extra.typing_phonemes[i];
    }
  }
  return ret;
}

void LogCompStr::MakeHiragana() {
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = GetClauseHiraganaString(iClause);
  SetClauseString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeKatakana() {
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseHiraganaString(iClause), LCMAP_KATAKANA);
  SetClauseString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeHankaku() {
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseTypingString(iClause),
                           LCMAP_HALFWIDTH | LCMAP_KATAKANA);
  SetClauseString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeZenEisuu() {
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseTypingString(iClause), LCMAP_FULLWIDTH);
  SetClauseString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeHanEisuu() {
  DWORD iClause = 0;
  if (HasClauseSelected()) {
    iClause = extra.dwSelectedClause;
  }
  std::wstring str = lcmap(GetClauseTypingString(iClause), LCMAP_HALFWIDTH);
  SetClauseString(extra.dwSelectedClause, str, TRUE);
}

void LogCompStr::MakeResult() {
  result_read_clause = comp_read_clause;  // TODO: generate reading
  result_read_str = comp_read_str;
  result_clause = comp_clause;
  result_str = comp_str;
  clear_read();
  clear_comp();
  clear_extra();
}

void LogCompStr::SetClauseAttribute(DWORD iClause, BYTE attr) {
  if (iClause != 0xFFFFFFFF) {
    DWORD ich = ClauseToCompChar(iClause);
    DWORD ichEnd = ClauseToCompChar(iClause + 1);
    for (DWORD i = ich; i < ichEnd; ++i) {
      comp_attr[i] = attr;
    }
  }
}

BOOL LogCompStr::IsBeingConverted() {
  for (size_t i = 0; i < comp_attr.size(); ++i) {
    if (comp_attr[i] != ATTR_INPUT) return TRUE;
  }
  return FALSE;
}

DWORD LogCompStr::GetCharCount() const {
  return (DWORD)comp_str.size();
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
  HIMCC hNewCompStr = ImmReSizeIMCC(hCompStr, total);
  if (hNewCompStr) {
    CompStr *lpCompStr = (CompStr *)ImmLockIMCC(hNewCompStr);
    if (lpCompStr) {
      lpCompStr->dwSize = total;
      lpCompStr->dwCursorPos = log->dwCursorPos;
      lpCompStr->dwDeltaStart = log->dwDeltaStart;

      LPBYTE pb = lpCompStr->GetBytes();
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
      lpCompStr->dwPrivateSize = extra->GetTotalSize();
      lpCompStr->dwPrivateOffset = (DWORD)(pb - lpCompStr->GetBytes());
      pb += extra->Store(&log->extra);

      assert((DWORD)(pb - lpCompStr->GetBytes()) == total);

      ImmUnlockIMCC(hNewCompStr);
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
