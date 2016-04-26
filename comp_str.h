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
  static HIMCC ReAlloc(HIMCC hCompStr, const LogCompStr *log);

  void GetLogCompStr(LogCompStr& log);
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
