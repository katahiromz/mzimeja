// comp_str.cpp --- composition string
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

DWORD LogCompStrPrivate::GetTotalSize() const {
  DWORD total = sizeof(COMPSTRPRIVATE);
  for (size_t i = 0; i < hiragana_phonemes.size(); ++i) {
    total += (hiragana_phonemes[i].size() + 1) * sizeof(WCHAR);
  }
  for (size_t i = 0; i < typing_phonemes.size(); ++i) {
    total += (typing_phonemes[i].size() + 1) * sizeof(WCHAR);
  }
  return total;
}

void COMPSTRPRIVATE::GetLog(LogCompStrPrivate& log) {
  log.clear();
  log.dwPhonemeCursor = dwPhonemeCursor;

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
} // COMPSTRPRIVATE::GetLog

DWORD COMPSTRPRIVATE::Store(const LogCompStrPrivate *log) {
  assert(this);
  assert(log);
  dwSignature = 0xDEADFACE;
  dwPhonemeCursor = log->dwPhonemeCursor;

  LPBYTE pb = (LPBYTE)this;
  pb += sizeof(COMPSTRPRIVATE);

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
} // COMPSTRPRIVATE::Store

//////////////////////////////////////////////////////////////////////////////

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
  total += private_data.GetTotalSize();
  return total;
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
  COMPSTRPRIVATE *private_data = GetPrivateData();
  if (private_data && private_data->dwSignature == 0xDEADFACE) {
    private_data->GetLog(log.private_data);
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

      COMPSTRPRIVATE *private_data = (COMPSTRPRIVATE *)pb;
      pb += private_data->Store(&log->private_data);

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
