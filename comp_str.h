// comp_str.h --- composition string
//////////////////////////////////////////////////////////////////////////////

#ifndef COMP_STR_H_
#define COMP_STR_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

//////////////////////////////////////////////////////////////////////////////

#define MAXCOMPSIZE 128
#define MAXCLAUSESIZE 16

struct MZCOMPSTR : public COMPOSITIONSTRING {
  TCHAR   szCompReadStr[MAXCOMPSIZE];
  BYTE    bCompReadAttr[MAXCOMPSIZE];
  DWORD   dwCompReadClause[MAXCLAUSESIZE];
  TCHAR   szCompStr[MAXCOMPSIZE];
  BYTE    bCompAttr[MAXCOMPSIZE];
  DWORD   dwCompClause[MAXCLAUSESIZE];
  BYTE    szTypeInfo[MAXCOMPSIZE];
  TCHAR   szResultReadStr[MAXCOMPSIZE];
  DWORD   dwResultReadClause[MAXCOMPSIZE];
  TCHAR   szResultStr[MAXCOMPSIZE];
  DWORD   dwResultClause[MAXCOMPSIZE];
};
typedef MZCOMPSTR *LPMZCOMPSTR;

inline void SetClause(LPDWORD lpdw, DWORD num) {
  *lpdw = 0;
  *(lpdw + 1) = num;
}

//////////////////////////////////////////////////////////////////////////////

struct CompStr : public MZCOMPSTR {
  void Init(DWORD dwClrFlag);
  void Clear(DWORD dwClrFlag);
  BOOL CheckAttr();
  void MakeAttrClause();

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
    return (LPBYTE)(GetBytes() + dwCompAttrOffset);
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
