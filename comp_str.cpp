// comp_str.cpp --- composition string
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

DWORD LogCompStr::GetTotalSize() const {
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
  return total;
}

//////////////////////////////////////////////////////////////////////////////

void CompStr::GetLogCompStr(LogCompStr& log) {
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
}

/*static*/ HIMCC CompStr::ReAlloc(HIMCC hCompStr, const LogCompStr *log) {
  LogCompStr log_comp_str;
  if (log == NULL) {
    log = &log_comp_str;
  }
  DWORD total = log->GetTotalSize();
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
    memcpy(pb, &log->member[0], log->member.size() * sizeof(WCHAR)); \
    pb += log->member.size() * sizeof(WCHAR); \
  }

      lpCompStr->dwCompReadAttrLen = log->comp_read_attr.size() * sizeof(BYTE);
      lpCompStr->dwCompReadAttrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_BYTES(comp_read_attr);
      lpCompStr->dwCompReadClauseLen = log->comp_read_clause.size() * sizeof(DWORD);
      lpCompStr->dwCompReadClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_DWORDS(comp_read_clause);
      lpCompStr->dwCompReadStrLen = log->comp_read_str.size();
      lpCompStr->dwCompReadStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_STRING(comp_read_str);
      lpCompStr->dwCompAttrLen = log->comp_attr.size() * sizeof(BYTE);
      lpCompStr->dwCompAttrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_BYTES(comp_attr);
      lpCompStr->dwCompClauseLen = log->comp_clause.size() * sizeof(DWORD);
      lpCompStr->dwCompClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_DWORDS(comp_clause);
      lpCompStr->dwCompStrLen = log->comp_str.size();
      lpCompStr->dwCompStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_STRING(comp_str);
      lpCompStr->dwResultReadClauseLen = log->result_read_clause.size() * sizeof(DWORD);
      lpCompStr->dwResultReadClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_DWORDS(result_read_clause);
      lpCompStr->dwResultReadStrLen = log->result_read_str.size();
      lpCompStr->dwResultReadStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_STRING(result_read_str);
      lpCompStr->dwResultClauseLen = log->result_clause.size() * sizeof(DWORD);
      lpCompStr->dwResultClauseOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_DWORDS(result_clause);
      lpCompStr->dwResultStrLen = log->result_str.size();
      lpCompStr->dwResultStrOffset = (DWORD)(pb - lpCompStr->GetBytes());
      ADD_STRING(result_str);
      assert((DWORD)(pb - lpCompStr->GetBytes()) == total);

      ImmUnlockIMCC(hNewCompStr);
      hCompStr = hNewCompStr;
    }
  }
  return hCompStr;
}

void CompStr::Clear(DWORD dwClrFlag) {
  dwSize = sizeof(COMPOSITIONSTRING);

  if (dwClrFlag & CLR_UNDET) {
    dwCompReadAttrOffset = sizeof(COMPOSITIONSTRING);
    dwCompReadClauseOffset = sizeof(COMPOSITIONSTRING);
    dwCompReadStrOffset = sizeof(COMPOSITIONSTRING);
    dwCompAttrOffset = sizeof(COMPOSITIONSTRING);
    dwCompClauseOffset = sizeof(COMPOSITIONSTRING);
    dwCompStrOffset = sizeof(COMPOSITIONSTRING);

    dwCompStrLen = 0;
    dwCompReadStrLen = 0;
    dwCompAttrLen = 0;
    dwCompReadAttrLen = 0;
    dwCompClauseLen = 0;
    dwCompReadClauseLen = 0;

    dwCursorPos = 0;
  }

  if (dwClrFlag & CLR_RESULT) {
    dwResultStrOffset = sizeof(COMPOSITIONSTRING);
    dwResultClauseOffset = sizeof(COMPOSITIONSTRING);
    dwResultReadStrOffset = sizeof(COMPOSITIONSTRING);
    dwResultReadClauseOffset = sizeof(COMPOSITIONSTRING);

    dwResultStrLen = 0;
    dwResultClauseLen = 0;
    dwResultReadStrLen = 0;
    dwResultReadClauseLen = 0;
  }
}

BOOL CompStr::CheckAttr() {
  LPBYTE lpb = GetCompAttr();
  int len = dwCompAttrLen;
  for (int i = 0; i < len; i++)
    if (*lpb++ & ATTR_TARGET_CONVERTED) return TRUE;
  return FALSE;
}

void CompStr::MakeAttrClause() {
  DWORD len = dwCompAttrLen;
  DWORD readlen = dwCompReadAttrLen;
  if (len != readlen) return;

  DWORD pos = dwCursorPos;
  LPBYTE lpb = GetCompAttr();
  for (DWORD i = 0; i < len; i++) {
    if (i < pos)
      *lpb++ = 0x10;
    else
      *lpb++ = ATTR_INPUT;
  }

  lpb = GetCompReadAttr();
  for (DWORD i = 0; i < readlen; i++) {
    if (i < pos)
      *lpb++ = 0x10;
    else
      *lpb++ = ATTR_INPUT;
  }

  LPDWORD lpdw = GetCompClause();
  *lpdw++ = 0;
  *lpdw++ = (BYTE)pos;
  *lpdw++ = len;

  lpdw = GetCompReadClause();
  *lpdw++ = 0;
  *lpdw++ = (BYTE)pos;
  *lpdw = len;
}

void CompStr::Dump() {
#ifndef NDEBUG
  DebugPrint(TEXT("dwSize: %08X"), dwSize);
  DebugPrint(TEXT("dwCursorPos: %08X"), dwCursorPos);
  DebugPrint(TEXT("dwDeltaStart: %08X"), dwDeltaStart);
  DebugPrint(TEXT("CompReadAttr: "));
  if (dwCompReadAttrLen) {
    LPBYTE attrs = GetCompReadAttr();
    for (DWORD i = 0; i < dwCompReadAttrLen; ++i) {
      DebugPrint(TEXT("%02X"), attrs[i]);
    }
  }
  DebugPrint(TEXT("CompReadClause: "));
  if (dwCompReadClauseLen) {
    LPDWORD clauses = GetCompReadClause();
    for (DWORD i = 0; i < dwCompReadClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X"), clauses[i]);
    }
  }
  DebugPrint(TEXT("CompReadStr: "));
  if (dwCompReadStrLen) {
    LPTSTR str = GetCompReadStr();
    for (DWORD i = 0; i < dwCompReadStrLen; ++i) {
      DebugPrint(TEXT("%*s"), dwCompReadStrLen, str);
    }
  }
  DebugPrint(TEXT("CompAttr: "));
  if (dwCompAttrLen) {
    LPBYTE attrs = GetCompAttr();
    for (DWORD i = 0; i < dwCompAttrLen; ++i) {
      DebugPrint(TEXT("%02X"), attrs[i]);
    }
  }
  DebugPrint(TEXT("CompClause: "));
  if (dwCompClauseLen) {
    LPDWORD clauses = GetCompClause();
    for (DWORD i = 0; i < dwCompClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X"), clauses[i]);
    }
  }
  DebugPrint(TEXT("CompStr: "));
  if (dwCompStrLen) {
    LPTSTR str = GetCompStr();
    for (DWORD i = 0; i < dwCompStrLen; ++i) {
      DebugPrint(TEXT("%*s"), dwCompStrLen, str);
    }
  }
  DebugPrint(TEXT("ResultReadClause: "));
  if (dwResultReadClauseLen) {
    LPDWORD clauses = GetResultReadClause();
    for (DWORD i = 0; i < dwResultReadClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X"), clauses[i]);
    }
  }
  DebugPrint(TEXT("ResultReadStr: "));
  if (dwResultReadStrLen) {
    LPTSTR str = GetResultReadStr();
    for (DWORD i = 0; i < dwResultReadStrLen; ++i) {
      DebugPrint(TEXT("%*s"), dwResultReadStrLen, str);
    }
  }
  DebugPrint(TEXT("ResultClause: "));
  if (dwResultClauseLen) {
    LPDWORD clauses = GetResultClause();
    for (DWORD i = 0; i < dwResultClauseLen / 4; ++i) {
      DebugPrint(TEXT("%08X"), clauses[i]);
    }
  }
  DebugPrint(TEXT("ResultStr: "));
  if (dwResultStrLen) {
    LPTSTR str = GetResultStr();
    for (DWORD i = 0; i < dwResultStrLen; ++i) {
      DebugPrint(TEXT("%*s"), dwResultStrLen, str);
    }
  }
#endif
} // CompStr::Dump

//////////////////////////////////////////////////////////////////////////////
