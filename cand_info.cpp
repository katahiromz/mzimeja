// cand_info.cpp --- candidate info of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define MAX_CANDLISTS   32
#define CANDPAGE_SIZE       8

//////////////////////////////////////////////////////////////////////////////
// LogCandList

void LogCandList::clear() {
  FOOTMARK();
  dwStyle = IME_CAND_READ;
  dwSelection = 0;
  dwPageStart = 0;
  dwPageSize = CANDPAGE_SIZE;
  cand_strs.clear();
}

DWORD LogCandList::GetTotalSize() const {
  FOOTMARK();
  DWORD total = sizeof(CANDIDATELIST);
  total += DWORD(cand_strs.size() * sizeof(DWORD));
  for (size_t iCand = 0; iCand < cand_strs.size(); ++iCand) {
    total += DWORD((cand_strs[iCand].size() + 1) * sizeof(WCHAR));
  }
  return total;
}

DWORD LogCandList::GetCandCount() const {
  return (DWORD)cand_strs.size();
}

void LogCandList::MoveNext() {
  ++dwSelection;
  if (dwSelection >= GetCandCount()) {
    dwSelection = 0;
  }
  dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

void LogCandList::MovePrev() {
  if (dwSelection > 0) {
    --dwSelection;
  } else {
    dwSelection = GetCandCount() - 1;
  }
  dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

void LogCandList::PageUp() {
  if (dwPageStart >= dwPageSize) {
    dwSelection -= dwPageSize;
  } else {
    dwSelection = 0;
  }
  dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

void LogCandList::PageDown() {
  if (dwPageStart + dwPageSize < GetCandCount()) {
    dwSelection += dwPageSize;
  } else {
    dwSelection = GetCandCount() - 1;
  }
  dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

std::wstring LogCandList::GetString(DWORD iCand) const {
  return cand_strs[iCand];
}

std::wstring LogCandList::GetString() const {
  return GetString(dwSelection);
}

//////////////////////////////////////////////////////////////////////////////
// LogCandInfo

void LogCandInfo::clear() {
  FOOTMARK();
  cand_lists.clear();
  iClause = 0;
}

BOOL LogCandInfo::HasCandInfo() const {
  return cand_lists.size() > 0;
}

DWORD LogCandInfo::GetClauseCount() const {
  return DWORD(cand_lists.size());
}

void LogCandInfo::MoveLeft() {
  if (iClause > 0) {
    --iClause;
  } else {
    iClause = GetClauseCount() - 1;
  }
}

void LogCandInfo::MoveRight() {
  if (iClause + 1 < GetClauseCount()) {
    ++iClause;
  } else {
    iClause = 0;
  }
}

void LogCandInfo::SelectCand(UINT uCandIndex) {
  DWORD dwPageStart = cand_lists[iClause].dwPageStart;
  if (dwPageStart + uCandIndex < cand_lists[iClause].dwPageSize) {
    cand_lists[iClause].dwSelection = dwPageStart + uCandIndex;
  }
}

void LogCandInfo::MoveNext() {
  cand_lists[iClause].MoveNext();
}

void LogCandInfo::MovePrev() {
  cand_lists[iClause].MovePrev();
}

void LogCandInfo::MoveHome() {
  iClause = 0;
}

void LogCandInfo::MoveEnd() {
  iClause = GetClauseCount() - 1;
}

void LogCandInfo::PageUp() {
  cand_lists[iClause].PageUp();
}

void LogCandInfo::PageDown() {
  cand_lists[iClause].PageDown();
}

std::wstring LogCandInfo::GetString() const {
  return cand_lists[iClause].GetString(cand_lists[iClause].dwSelection);
}

std::wstring LogCandInfo::GetString(DWORD iCand) const {
  return cand_lists[iClause].GetString(iCand);
}

DWORD LogCandInfo::GetTotalSize() const {
  FOOTMARK();
  DWORD total = sizeof(CANDIDATEINFO);
  for (size_t i = 0; i < cand_lists.size(); ++i) {
    total += cand_lists[i].GetTotalSize();
  }
  total += sizeof(CANDINFOEXTRA);
  return total;
}

void LogCandInfo::Dump() {
#ifndef NDEBUG
  DebugPrintA("LogCandInfo::Dump\n");
  for (size_t i = 0; i < cand_lists.size(); ++i) {
    DebugPrintA("### CandList %u ###\n", i);
    DebugPrintA("+ dwStyle: %08X", cand_lists[i].dwStyle);
    DebugPrintA("+ dwSelection: %08X", cand_lists[i].dwSelection);
    DebugPrintA("+ dwPageStart: %08X", cand_lists[i].dwPageStart);
    DebugPrintA("+ dwPageSize: %08X", cand_lists[i].dwPageSize);
    DebugPrintA("+ cand_strs: ");
    for (size_t k = 0; k < cand_lists[i].cand_strs.size(); ++k) {
      DebugPrintA("%ls ", cand_lists[i].cand_strs[k].c_str());
    }
    DebugPrintA("\n");
    DebugPrintA("+ iClause: %u\n", iClause);
  }
#endif
} // LogCandInfo::Dump

//////////////////////////////////////////////////////////////////////////////
// CandList

DWORD CandList::GetPageEnd() const {
  DWORD dw = dwPageStart + dwPageSize;
  if (dw > dwCount) dw = dwCount;
  return dw;
}

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
  if (dwCount < dwPageSize) dwPageSize = dwCount;

  BYTE *pb = GetBytes();
  pb += sizeof(CANDIDATELIST);
  pb += dwCount * sizeof(DWORD);

  for (DWORD iCand = 0; iCand < dwCount; ++iCand) {
    dwOffset[iCand] = DWORD(pb - GetBytes());
    const std::wstring& str = log->cand_strs[iCand];
    DWORD cb = DWORD((str.size() + 1) * sizeof(WCHAR));
    memcpy(pb, &str[0], cb);
    pb += cb;
  }

  DebugPrintA("%u, %u\n", dwSize, DWORD(pb - GetBytes()));
  assert(dwSize == DWORD(pb - GetBytes()));
  return DWORD(pb - GetBytes());
}

//////////////////////////////////////////////////////////////////////////////
// CandInfo

CandList *CandInfo::GetList(DWORD i) {
  FOOTMARK();
  assert(i < dwCount);
  return (CandList *)(GetBytes() + dwOffset[i]);
}

void CandInfo::GetLog(LogCandInfo& log) {
  FOOTMARK();
  log.clear();

  LogCandList cand;
  for (DWORD iList = 0; iList < dwCount; ++iList) {
    CandList *pList = GetList(iList);
    pList->GetLog(cand);
    log.cand_lists.push_back(cand);
  }

  CANDINFOEXTRA *extra = GetExtra();
  if (extra && extra->dwSignature == 0xDEADFACE) {
    log.iClause = extra->iClause;
  } else {
    log.iClause = 0;
  }
}

DWORD CandInfo::Store(const LogCandInfo *log) {
  FOOTMARK();
  dwSize = log->GetTotalSize();
  dwCount = (DWORD)log->cand_lists.size();
  if (MAX_CANDLISTS < dwCount) {
    dwCount = MAX_CANDLISTS;
  }

  BYTE *pb = GetBytes();
  pb += sizeof(CANDIDATEINFO);

  for (DWORD iList = 0; iList < dwCount; ++iList) {
    dwOffset[iList] = DWORD(pb - GetBytes());
    CandList *pList = GetList(iList);
    pb += pList->Store(&log->cand_lists[iList]);
  }

  dwPrivateSize = sizeof(CANDINFOEXTRA);
  dwPrivateOffset = DWORD(pb - GetBytes());

  CANDINFOEXTRA *extra = (CANDINFOEXTRA *)pb;
  extra->dwSignature = 0xDEADFACE;
  extra->iClause = log->iClause;
  pb += sizeof(CANDINFOEXTRA);

  assert(dwSize == DWORD(pb - GetBytes()));
  return DWORD(pb - GetBytes());
}

CANDINFOEXTRA *CandInfo::GetExtra() {
  if (dwPrivateSize >= sizeof(CANDINFOEXTRA)) {
    BYTE *pb = GetBytes();
    pb += dwPrivateOffset;
    CANDINFOEXTRA *extra = (CANDINFOEXTRA *)pb;
    if (extra->dwSignature == 0xDEADFACE) {
      return extra;
    }
  }
  return NULL;
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
      assert(cb == total);

      ImmUnlockIMCC(hNewCandInfo);
      hCandInfo = hNewCandInfo;
    } else {
      DebugPrintA("CandInfo::ReCreate: failed #2");
      assert(0);
    }
  } else {
    DebugPrintA("CandInfo::ReCreate: failed");
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
    DebugPrintA("\n");
  }
#endif
} // CandInfo::Dump

//////////////////////////////////////////////////////////////////////////////
