// comp_str.cpp --- composition string
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

#define member_offset(struct_name,member) \
  ((LONG)(LONG_PTR)&((reinterpret_cast<struct_name*>(NULL))->member))

void CompStr::Init(DWORD dwClrFlag) {
  dwSize = sizeof(MZCOMPSTR);

  if (dwClrFlag & CLR_UNDET) {
    dwCompReadAttrOffset = member_offset(MZCOMPSTR, bCompReadAttr);
    dwCompReadClauseOffset = member_offset(MZCOMPSTR, dwCompReadClause);
    dwCompReadStrOffset = member_offset(MZCOMPSTR, szCompReadStr);
    dwCompAttrOffset = member_offset(MZCOMPSTR, bCompAttr);
    dwCompClauseOffset = member_offset(MZCOMPSTR, dwCompClause);
    dwCompStrOffset = member_offset(MZCOMPSTR, szCompStr);

    dwCompStrLen = 0;
    dwCompReadStrLen = 0;
    dwCompAttrLen = 0;
    dwCompReadAttrLen = 0;
    dwCompClauseLen = 0;
    dwCompReadClauseLen = 0;

    *GetCompStr() = 0;
    *GetCompReadStr() = 0;

    dwCursorPos = 0;
  }

  if (dwClrFlag & CLR_RESULT) {
    dwResultStrOffset = member_offset(MZCOMPSTR, szResultStr);
    dwResultClauseOffset = member_offset(MZCOMPSTR, dwResultClause);
    dwResultReadStrOffset = member_offset(MZCOMPSTR, szResultReadStr);
    dwResultReadClauseOffset = member_offset(MZCOMPSTR, dwResultReadClause);

    dwResultStrLen = 0;
    dwResultClauseLen = 0;
    dwResultReadStrLen = 0;
    dwResultReadClauseLen = 0;

    *GetResultStr() = 0;
    *GetResultReadStr() = 0;
  }
}

void CompStr::Clear(DWORD dwClrFlag) {
  dwSize = sizeof(MZCOMPSTR);

  if (dwClrFlag & CLR_UNDET) {
    dwCompStrOffset = 0;
    dwCompClauseOffset = 0;
    dwCompAttrOffset = 0;
    dwCompReadStrOffset = 0;
    dwCompReadClauseOffset = 0;
    dwCompReadAttrOffset = 0;
    dwCompStrLen = 0;
    dwCompClauseLen = 0;
    dwCompAttrLen = 0;
    dwCompReadStrLen = 0;
    dwCompReadClauseLen = 0;
    dwCompReadAttrLen = 0;
    szCompStr[0] = 0;
    szCompReadStr[0] = 0;
    dwCursorPos = 0;
  }

  if (dwClrFlag & CLR_RESULT) {
    dwResultStrOffset = 0;
    dwResultClauseOffset = 0;
    dwResultReadStrOffset = 0;
    dwResultReadClauseOffset = 0;
    dwResultStrLen = 0;
    dwResultClauseLen = 0;
    dwResultReadStrLen = 0;
    dwResultReadClauseLen = 0;
    szResultStr[0] = 0;
    szResultReadStr[0] = 0;
  }
}

BOOL CompStr::CheckAttr() {
  LPBYTE lpb = GetCompAttr();
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
  LPBYTE lpb = GetCompAttr();
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
