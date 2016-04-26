// cand_info.h --- candidate info
//////////////////////////////////////////////////////////////////////////////

#ifndef CAND_INFO_H_
#define CAND_INFO_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"
#include <vector>

//////////////////////////////////////////////////////////////////////////////

struct LogCandInfo {
  DWORD   dwStyle;
  DWORD   dwSelection;
  DWORD   dwPageStart;
  DWORD   dwPageSize;
  std::vector<std::wstring> cand_strs;

  LogCandInfo(DWORD dwCandStyle = IME_CAND_READ) {
    dwStyle = dwCandStyle;
    dwSelection = 0;
    dwPageStart = 0;
    dwPageSize = 0;
  }
  DWORD GetTotalSize() const;
};

//////////////////////////////////////////////////////////////////////////////

struct CandList : public CANDIDATELIST {
  LPBYTE GetBytes() { return (LPBYTE)this; }
  LPTSTR GetCandString(DWORD i) { return LPTSTR(GetBytes() + dwOffset[i]); }
  LPTSTR GetCurString() { return GetCandString(dwSelection); }
  DWORD  GetPageEnd() const { return dwPageStart + dwPageSize; }

private:
  // not implemented
  CandList();
  CandList(const CandList&);
  CandList& operator=(const CandList&);
};

//////////////////////////////////////////////////////////////////////////////

struct CandInfo : public CANDIDATEINFO {
  static HIMCC ReCreate(HIMCC hCandInfo, const LogCandInfo *log = NULL);
  void GetLogCandInfo(LogCandInfo& log);

  LPBYTE GetBytes() { return (LPBYTE)this; }
  CandList *GetList(DWORD i = 0) {
    return (CandList *)(GetBytes() + dwOffset[i]);
  }

  void Dump();

private:
  // not implement
  CandInfo();
  CandInfo(const CandInfo&);
  CandInfo& operator=(const CandInfo&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_CONTEXT_H_

//////////////////////////////////////////////////////////////////////////////
