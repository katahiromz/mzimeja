// comp_str.h --- composition string
//////////////////////////////////////////////////////////////////////////////

#ifndef COMP_STR_H_
#define COMP_STR_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

#ifdef __cplusplus
  #include <string>
  #include <vector>
#endif

//////////////////////////////////////////////////////////////////////////////

struct LogCompStrPrivate {
  DWORD               dwReadCursor;
  std::wstring        spell;
  std::vector<DWORD>  index_map_comp2read;
  std::vector<DWORD>  index_map_read2spell;
  LogCompStrPrivate() : dwReadCursor(0) {}
  DWORD GetTotalSize() const;
  void clear() {
    dwReadCursor = 0;
    spell.clear();
    index_map_comp2read.clear();
    index_map_read2spell.clear();
  }
}; // struct LogCompStrPrivate

//////////////////////////////////////////////////////////////////////////////

struct COMPSTRPRIVATE {
  DWORD dwSignature;
  DWORD dwReadCursor;
  DWORD dwSpellStrLen;
  DWORD dwSpellStrOffset;
  DWORD dwComp2ReadMapLen;
  DWORD dwComp2ReadMapOffset;
  DWORD dwRead2SpellMapLen;
  DWORD dwRead2SpellMapOffset;
  LPBYTE GetBytes() { return (LPBYTE)this; }

  LPWSTR GetSpellStr() {
    if (dwSpellStrLen) {
      return (LPWSTR)(GetBytes() + dwSpellStrOffset);
    }
    return NULL;
  }
  LPDWORD GetComp2Read(DWORD& dwCount) {
    dwCount = dwComp2ReadMapLen / sizeof(DWORD);
    if (dwCount) {
      return (LPDWORD)(GetBytes() + dwComp2ReadMapOffset);
    }
    return NULL;
  }
  LPDWORD GetRead2Spell(DWORD& dwCount) {
    dwCount = dwRead2SpellMapLen / sizeof(DWORD);
    if (dwCount) {
      return (LPDWORD)(GetBytes() + dwRead2SpellMapOffset);
    }
    return NULL;
  }

  void GetLog(LogCompStrPrivate& log);
  DWORD Store(const LogCompStrPrivate *log);
}; // struct COMPSTRPRIVATE

//////////////////////////////////////////////////////////////////////////////
// logical composition string

struct LogCompStr {
  DWORD               dwCursorPos;
  DWORD               dwDeltaStart;
  std::vector<BYTE>   comp_read_attr;
  std::vector<DWORD>  comp_read_clause;
  std::wstring        comp_read_str;
  std::vector<BYTE>   comp_attr;
  std::vector<DWORD>  comp_clause;
  std::wstring        comp_str;
  std::vector<DWORD>  result_read_clause;
  std::wstring        result_read_str;
  std::vector<DWORD>  result_clause;
  std::wstring        result_str;
  LogCompStrPrivate   private_data;
  LogCompStr() {
    dwCursorPos = 0;
    dwDeltaStart = 0;
    comp_read_clause.resize(2, 0);
    comp_clause.resize(2, 0);
    result_read_clause.resize(2, 0);
    result_clause.resize(2, 0);
  }
  DWORD GetTotalSize() const;
};

inline void SetClause(LPDWORD lpdw, DWORD num) {
  *lpdw = 0;
  *(lpdw + 1) = num;
}

//////////////////////////////////////////////////////////////////////////////

struct CompStr : public COMPOSITIONSTRING {
  static HIMCC ReCreate(HIMCC hCompStr, const LogCompStr *log = NULL);

  void GetLog(LogCompStr& log);
  BOOL IsBeingConverted();

  LPBYTE GetBytes() { return (LPBYTE)this; }

  LPBYTE GetCompReadAttr() {
    return GetBytes() + dwCompReadAttrOffset;
  }
  LPDWORD GetCompReadClause() {
    return (LPDWORD)(GetBytes() + dwCompReadClauseOffset);
  }
  LPTSTR GetCompReadStr() {
    return (LPTSTR)(GetBytes() + dwCompReadStrOffset);
  }
  LPBYTE GetCompAttr() {
    return GetBytes() + dwCompAttrOffset;
  }
  LPDWORD GetCompClause() {
    return (LPDWORD)(GetBytes() + dwCompClauseOffset);
  }
  LPTSTR GetCompStr() {
    return (LPTSTR)(GetBytes() + dwCompStrOffset);
  }
  LPDWORD GetResultReadClause() {
    return (LPDWORD)(GetBytes() + dwResultReadClauseOffset);
  }
  LPTSTR GetResultReadStr() {
    return (LPTSTR)(GetBytes() + dwResultReadStrOffset);
  }
  LPDWORD GetResultClause() {
    return (LPDWORD)(GetBytes() + dwResultClauseOffset);
  }
  LPTSTR GetResultStr() {
    return (LPTSTR)(GetBytes() + dwResultStrOffset);
  }

  // extension
  COMPSTRPRIVATE *GetPrivateData() {
    if (dwPrivateSize) return (COMPSTRPRIVATE *)(GetBytes() + dwPrivateOffset);
    return NULL;
  }

  void Dump();

private:
  CompStr();
  CompStr(const CompStr&);
  CompStr& operator=(const CompStr&);
};

//////////////////////////////////////////////////////////////////////////////

#if 0
  #define GETLPCOMPREADATTR(lpcs) \
    (LPBYTE)((LPBYTE)(lpcs) + (lpcs)->dwCompReadAttrOffset)
  #define GETLPCOMPREADCLAUSE(lpcs) \
    (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwCompReadClauseOffset)
  #define GETLPCOMPREADSTR(lpcs) \
    (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwCompReadStrOffset)
  #define GETLPCOMPATTR(lpcs) (LPBYTE)((LPBYTE)(lpcs) + (lpcs)->dwCompAttrOffset)
  #define GETLPCOMPCLAUSE(lpcs) \
    (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwCompClauseOffset)
  #define GETLPCOMPSTR(lpcs) (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwCompStrOffset)
  #define GETLPRESULTREADCLAUSE(lpcs) \
    (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwResultReadClauseOffset)
  #define GETLPRESULTREADSTR(lpcs) \
    (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwResultReadStrOffset)
  #define GETLPRESULTCLAUSE(lpcs) \
    (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwResultClauseOffset)
  #define GETLPRESULTSTR(lpcs) \
    (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwResultStrOffset)
#endif

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef COMP_STR_H_

//////////////////////////////////////////////////////////////////////////////
