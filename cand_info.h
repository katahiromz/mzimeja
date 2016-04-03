// cand_info.h --- candidate info
//////////////////////////////////////////////////////////////////////////////

#ifndef CAND_INFO_H_
#define CAND_INFO_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

#define MAXCANDPAGESIZE 9
#define MAXCANDSTRSIZE 16
#define MAXCANDSTRNUM 32

//////////////////////////////////////////////////////////////////////////////

struct MZCAND : public CANDIDATEINFO {
  CANDIDATELIST cl;
  DWORD offset[MAXCANDSTRNUM];
  TCHAR szCand[MAXCANDSTRNUM][MAXCANDSTRSIZE];
};
typedef MZCAND *LPMZCAND;

//////////////////////////////////////////////////////////////////////////////

struct CandList : public CANDIDATELIST {
  LPBYTE GetBytes() { return (LPBYTE)this; }
  LPTSTR GetCandString(DWORD i) { return LPTSTR(GetBytes() + dwOffset[i]); }
  LPTSTR GetCurString() { return GetCandString(dwSelection); }
  DWORD GetPageEnd() const { return dwPageStart + dwPageSize; }

private:
  CandList();
  CandList(const CandList&);
  CandList& operator=(const CandList&);
};

//////////////////////////////////////////////////////////////////////////////

struct CandInfo : public MZCAND {
  LPBYTE GetBytes() { return (LPBYTE)this; }
  CandList *GetList() { return (CandList *)(GetBytes() + cl.dwOffset[0]); }

  DWORD GetCandOffset(DWORD i, LPCANDIDATELIST lpCandList) {
    return DWORD(LPBYTE(szCand[i]) - LPBYTE(lpCandList));
  }

  void Clear() {
    dwSize = 0L;
    dwCount = 0L;
    dwOffset[0] = 0L;
    cl.dwSize = 0L;
    cl.dwStyle = 0L;
    cl.dwCount = 0L;
    cl.dwSelection = 0L;
    cl.dwPageStart = 0L;
    cl.dwPageSize = 0L;
    cl.dwOffset[0] = 0L;
  }

private:
  CandInfo();
  CandInfo(const CandInfo&);
  CandInfo& operator=(const CandInfo&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_CONTEXT_H_

//////////////////////////////////////////////////////////////////////////////
