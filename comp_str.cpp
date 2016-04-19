// comp_str.cpp --- composition string
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

DWORD LogCompStr::GetTotalSize() const {
  size_t total = sizeof(COMPOSITIONSTRING);
  total += comp_read_attr.size();
  total += comp_read_clause.size();
  total += comp_read_str.size() * sizeof(WCHAR);
  total += comp_attr.size();
  total += comp_clause.size();
  total += comp_str.size() * sizeof(WCHAR);
  total += result_read_clause.size();
  total += result_read_str.size() * sizeof(WCHAR);
  total += result_clause.size();
  total += result_str.size() * sizeof(WCHAR);
  return total;
}

//////////////////////////////////////////////////////////////////////////////

void CompStr::GetLog(LogCompStr& log) {
  log.dwCursorPos = dwCursorPos;
  log.dwDeltaStart = dwDeltaStart;
  log.comp_read_attr.assign(GetCompReadAttr(), dwCompReadAttrLen);
  log.comp_read_clause.assign((char *)GetCompReadClause(), dwCompReadClauseLen);
  log.comp_read_str.assign(GetCompReadStr(), dwCompReadStrLen);
  log.comp_attr.assign(GetCompAttr(), dwCompAttrLen);
  log.comp_clause.assign((char *)GetCompClause(), dwCompClauseLen);
  log.comp_str.assign(GetCompStr(), dwCompStrLen);
  log.result_read_clause.assign((char *)GetResultReadClause(), dwResultReadClauseLen);
  log.result_read_str.assign(GetResultReadStr(), dwResultReadStrLen);
  log.result_clause.assign((char *)GetResultClause(), dwResultClauseLen);
  log.result_str.assign(GetResultStr(), dwResultStrLen);
}

/*static*/ HIMCC CompStr::ReAlloc(HIMCC hCompStr, const LogCompStr *log) {
  DWORD total = (log ? log->GetTotalSize() : sizeof(COMPOSITIONSTRING));
  HIMCC hNewCompStr = ImmReSizeIMCC(hCompStr, total);
  if (hNewCompStr) {
    CompStr *lpCompStr = (CompStr *)ImmLockIMCC(hNewCompStr);
    if (lpCompStr) {
      lpCompStr->dwSize = total;
      lpCompStr->dwCursorPos = (log ? log->dwCursorPos : 0);
      lpCompStr->dwDeltaStart = (log ? log->dwDeltaStart : 0);

      char *pb = lpCompStr->GetBytes();
      pb += sizeof(COMPOSITIONSTRING);

#define ADD_BYTES(member) \
  memcpy(pb, &log->member[0], log->member.size()); \
  pb += log->member.size()

#define ADD_STRING(member) \
  memcpy(pb, &log->member[0], log->member.size() * sizeof(WCHAR)); \
  pb += log->member.size() * sizeof(WCHAR)

      ADD_BYTES(comp_read_attr);
      ADD_BYTES(comp_read_clause);
      ADD_STRING(comp_read_str);
      ADD_BYTES(comp_attr);
      ADD_BYTES(comp_clause);
      ADD_STRING(comp_str);
      ADD_BYTES(result_read_clause);
      ADD_STRING(result_read_str);
      ADD_BYTES(result_clause);
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
  char *lpb = GetCompAttr();
  int len = dwCompAttrLen;
  for (int i = 0; i < len; i++)
    if (*lpb++ & 0x01) return TRUE;
  return FALSE;
}

void CompStr::MakeAttrClause() {
  DWORD len = dwCompAttrLen;
  DWORD readlen = dwCompReadAttrLen;
  if (len != readlen) return;

  DWORD pos = dwCursorPos;
  char *lpb = GetCompAttr();
  for (DWORD i = 0; i < len; i++) {
    if (i < pos)
      *lpb++ = 0x10;
    else
      *lpb++ = 0x00;
  }

  lpb = GetCompReadAttr();
  for (DWORD i = 0; i < readlen; i++) {
    if (i < pos)
      *lpb++ = 0x10;
    else
      *lpb++ = 0x00;
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

//////////////////////////////////////////////////////////////////////////////
