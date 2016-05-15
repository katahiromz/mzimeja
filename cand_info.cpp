// cand_info.cpp --- candidate info of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define MAX_CANDLISTS   32
#define PAGE_SIZE       8

//////////////////////////////////////////////////////////////////////////////
// LogCandList

void LogCandList::clear() {
  FOOTMARK();
  dwStyle = IME_CAND_READ;
  dwSelection = 0;
  dwPageStart = 0;
  dwPageSize = PAGE_SIZE;
  cand_strs.clear();
}

DWORD LogCandList::GetTotalSize() const {
  FOOTMARK();
  DWORD total = sizeof(CANDIDATELIST);
  total += (DWORD)((cand_strs.size() - 1) * sizeof(DWORD));
  for (size_t i = 0; i < cand_strs.size(); ++i) {
    total += DWORD((cand_strs.size() + 1) * sizeof(WCHAR));
  }
  return total;
}

//////////////////////////////////////////////////////////////////////////////
// LogCandInfo

void LogCandInfo::clear() {
  FOOTMARK();
  cand_lists.clear();
}

DWORD LogCandInfo::GetTotalSize() const {
  FOOTMARK();
  DWORD total = sizeof(CANDIDATEINFO);
  for (size_t i = 0; i < cand_lists.size(); ++i) {
    total += cand_lists[i].GetTotalSize();
  }
  return total;
}

//////////////////////////////////////////////////////////////////////////////
// CandList

void CandList::GetLog(LogCandList& log) {
  FOOTMARK();
  log.dwStyle = dwStyle;
  log.dwSelection = dwSelection;
  log.dwPageStart = dwPageStart;
  log.dwPageSize = dwPageSize;
  log.cand_strs.clear();
  for (DWORD iCand = 0; iCand < dwCount; ++iCand) {
    log.cand_strs.push_back(GetCandString(iCand));
  }
}

DWORD CandList::Store(const LogCandList *log) {
  FOOTMARK();

  dwSize = log->GetTotalSize();
  dwStyle = log->dwStyle;
  dwCount = DWORD(log->cand_strs.size());
  dwSelection = log->dwSelection;
  dwPageStart = log->dwPageStart;
  dwPageSize = log->dwPageSize;

  BYTE *pb = GetBytes();
  pb += sizeof(CANDIDATELIST);
  pb += (dwCount - 1) * sizeof(DWORD);

  for (DWORD iCand = 0; iCand < dwCount; ++iCand) {
    dwOffset[iCand] = DWORD(pb - GetBytes());
    const std::wstring& str = log->cand_strs[iCand];
    DWORD cb = DWORD((str.size() + 1) * sizeof(WCHAR));
    memcpy(pb, &str[0], cb);
    pb += cb;
  }
  assert(dwSize == DWORD(pb - GetBytes()));
  return DWORD(pb - GetBytes());
}

//////////////////////////////////////////////////////////////////////////////
// CandInfo

void CandInfo::GetLog(LogCandInfo& log) {
  FOOTMARK();
  log.clear();
  LogCandList log_cand_list;
  for (DWORD iList = 0; iList < dwCount; ++iList) {
    CandList *pList = GetList(iList);
    pList->GetLog(log_cand_list);
    log.cand_lists.push_back(log_cand_list);
  }
}

DWORD CandInfo::Store(const LogCandInfo *log) {
  FOOTMARK();
  dwSize = log->GetTotalSize();
  dwCount = (DWORD)log->cand_lists.size();
  if (MAX_CANDLISTS < dwCount) {
    dwCount = MAX_CANDLISTS;
  }
  dwPrivateSize = 0;
  dwPrivateOffset = 0;

  BYTE *pb = GetBytes();
  pb += sizeof(CANDIDATEINFO);

  for (DWORD iList = 0; iList < dwCount; ++iList) {
    dwOffset[iList] = DWORD(pb - GetBytes());
    CandList *pList = GetList(iList);
    pb += pList->Store(&log->cand_lists[iList]);
  }
  assert(dwSize == DWORD(pb - GetBytes()));
  return DWORD(pb - GetBytes());
}

/*static*/ HIMCC CandInfo::ReCreate(HIMCC hCandInfo, const LogCandInfo *log) {
  FOOTMARK();
  LogCandInfo log_cand_info;
  if (log == NULL) {
    log = &log_cand_info;
  }

  const DWORD total = log->GetTotalSize();
  HIMCC hNewCandInfo = ::ImmReSizeIMCC(hCandInfo, total);
  if (hNewCandInfo) {
    CandInfo *cand_info = (CandInfo *)::ImmLockIMCC(hNewCandInfo);
    if (cand_info) {
      DWORD cb = cand_info->Store(log);
      assert(cb == log->GetTotalSize());

      ImmUnlockIMCC(hNewCandInfo);
      hCandInfo = hNewCandInfo;
    } else {
      DebugPrint(TEXT("CandInfo::ReCreate: failed #2"));
      assert(0);
    }
  } else {
    DebugPrint(TEXT("CandInfo::ReCreate: failed"));
    assert(0);
  }
  return hCandInfo;
} // CandInfo::ReCreate

void CandInfo::Dump() {
#ifndef NDEBUG
  DebugPrintA("### CandInfo ###\n");
  DebugPrintA("+ dwSize: %u\n", dwSize);
  DebugPrintA("+ dwCount: %u\n", dwCount);
  DebugPrintA("+ dwPrivateSize: %u\n", dwPrivateSize);
  DebugPrintA("+ dwPrivateOffset: %u\n", dwPrivateOffset);
  for (DWORD i = 0; i < dwCount; ++i) {
    DebugPrintA("+ List #%u\n", i);
    DebugPrintA("++ dwOffset: %08X\n", dwOffset[i]);
    CandList *list = GetList(i);
    DebugPrintA("++ dwSize: %u\n", list->dwSize);
    DebugPrintA("++ dwStyle: %08X\n", list->dwStyle);
    DebugPrintA("++ dwCount: %08X\n", list->dwCount);
    DebugPrintA("++ dwSelection: %08X\n", list->dwSelection);
    DebugPrintA("++ dwPageStart: %u\n", list->dwPageStart);
    DebugPrintA("++ dwPageSize: %u\n", list->dwPageSize);
    DebugPrintA("++ Cand Strings: ");
    for (DWORD k = 0; k < list->dwCount; ++k) {
      DebugPrintA("%ls ", list->GetCandString(k));
    }
  }
#endif
} // CandInfo::Dump

//////////////////////////////////////////////////////////////////////////////
