// cand_info.cpp --- candidate info of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

#define MAX_CANDSTRS 32

void LogCandInfo::clear() {
  dwStyle = IME_CAND_READ;
  dwSelection = 0;
  dwPageStart = 0;
  dwPageSize = 0;
  cand_strs.clear();
}

DWORD LogCandInfo::GetTotalSize() const {
  FOOTMARK();
  DWORD num_strs = (DWORD)cand_strs.size();
  if (num_strs > MAX_CANDSTRS) {
    num_strs = MAX_CANDSTRS;
  }
  DWORD total = sizeof(CANDIDATEINFO) + sizeof(CANDIDATELIST) - sizeof(DWORD);
  for (DWORD iStr = 0; iStr < num_strs; ++iStr) {
    total += sizeof(DWORD);
    total += (cand_strs[iStr].size() + 1) * sizeof(WCHAR);
  }
  return total;
}

//////////////////////////////////////////////////////////////////////////////

#define PAGE_SIZE 9

void CandInfo::GetLog(LogCandInfo& log) {
  FOOTMARK();
  log.dwStyle = IME_CAND_READ;
  log.dwSelection = 0;
  log.dwPageStart = 0;
  log.dwPageSize = 0;
  log.cand_strs.clear();
  if (dwCount > 0) {
    CandList *list = GetList();
    if (list) {
      log.dwStyle = list->dwStyle;
      log.dwSelection = list->dwSelection;
      log.dwPageStart = list->dwPageStart;
      log.dwPageSize = PAGE_SIZE;
      if (list->dwCount < log.dwPageSize) {
        log.dwPageSize = list->dwCount;
      }
      for (DWORD iStr = 0; iStr < list->dwCount; ++iStr) {
        log.cand_strs.push_back(list->GetCandString(iStr));
      }
    }
  }
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
      DWORD num_strs = (DWORD)log->cand_strs.size();
      if (num_strs > MAX_CANDSTRS) {
        num_strs = MAX_CANDSTRS;
      }

      LPBYTE pb = cand_info->GetBytes();

      // store candidate info header
      CANDIDATEINFO *info = (CANDIDATEINFO *)pb;
      info->dwSize = total;
      info->dwCount = 1;  // number of lists
      info->dwOffset[0] = (DWORD)sizeof(CANDIDATEINFO);
      info->dwPrivateSize = 0;
      info->dwPrivateOffset = 0;
      pb += sizeof(CANDIDATEINFO);

      // store candidate list header
      CANDIDATELIST *list = (CANDIDATELIST *)pb;
      list->dwSize = (DWORD)(total - sizeof(CANDIDATEINFO));
      list->dwStyle = log->dwStyle;
      list->dwCount = (DWORD)log->cand_strs.size();
      list->dwSelection = log->dwSelection;
      list->dwPageStart = log->dwPageStart;
      list->dwPageSize = log->dwPageSize;
      pb += sizeof(CANDIDATELIST) + (num_strs - 1) * sizeof(DWORD);

      // store candidate strings
      for (DWORD iStr = 0; iStr < num_strs; ++iStr) {
        list->dwOffset[iStr] = (DWORD)(pb - (LPBYTE)list);
        DWORD size = (log->cand_strs[iStr].size() + 1) * sizeof(WCHAR);
        memcpy(pb, log->cand_strs[iStr].c_str(), size);
        pb += size;
      }
      assert((DWORD)(pb - cand_info->GetBytes()) == total);

      ImmUnlockIMCC(hNewCandInfo);
      hCandInfo = hNewCandInfo;
    } else {
      DebugPrint(TEXT("CandInfo::ReCreate: failed #2"));
    }
  } else {
    DebugPrint(TEXT("CandInfo::ReCreate: failed"));
  }
  return hCandInfo;
} // CandInfo::ReCreate

void CandInfo::Dump() {
#ifndef NDEBUG
  DebugPrint(TEXT("### CandInfo ###\n"));
  DebugPrint(TEXT("+ dwSize: %u\n"), dwSize);
  DebugPrint(TEXT("+ dwCount: %u\n"), dwCount);
  for (DWORD i = 0; i < dwCount; ++i) {
    DebugPrint(TEXT("+ List #%u"), i);
    DebugPrint(TEXT("++ dwOffset: %08X\n"), dwOffset[i]);
    CandList *list = GetList(i);
    DebugPrint(TEXT("++ dwSize: %u\n"), list->dwSize);
    DebugPrint(TEXT("++ dwStyle: %08X\n"), list->dwStyle);
    DebugPrint(TEXT("++ dwCount: %08X\n"), list->dwCount);
    DebugPrint(TEXT("++ dwSelection: %08X\n"), list->dwSelection);
    DebugPrint(TEXT("++ dwPageStart: %u\n"), list->dwPageStart);
    DebugPrint(TEXT("++ dwPageSize: %u\n"), list->dwPageSize);
    DebugPrint(TEXT("++ dwOffset: %u\n"), list->dwOffset);
    for (DWORD k = 0; k < list->dwCount; ++k) {
      DebugPrint(TEXT("+++ %s"), list->GetCandString(k));
    }
  }
#endif
} // CandInfo::Dump

//////////////////////////////////////////////////////////////////////////////
