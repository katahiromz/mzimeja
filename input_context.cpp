// input_context.cpp --- input context
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

void InputContext::Initialize() {
  if (!HasLogFont()) {
    lfFont.W.lfCharSet = SHIFTJIS_CHARSET;
    lfFont.W.lfFaceName[0] = L'\0';
    fdwInit |= INIT_LOGFONT;
  }

  if (!HasConversion()) {
    fdwConversion = IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
    fdwInit |= INIT_CONVERSION;
  }

  hCompStr = CompStr::ReAlloc(hCompStr, NULL);
  hCandInfo = CandInfo::ReAlloc(hCandInfo);
}

BOOL InputContext::HasCandInfo() {
  BOOL fRet = FALSE;

  if (ImmGetIMCCSize(hCandInfo) < sizeof(CANDIDATEINFO)) return FALSE;

  CandInfo *lpCandInfo = LockCandInfo();
  if (lpCandInfo) {
    fRet = (lpCandInfo->dwCount > 0);
    UnlockCandInfo();
  }
  return fRet;
}

BOOL InputContext::HasCompStr() {
  if (ImmGetIMCCSize(hCompStr) <= sizeof(COMPOSITIONSTRING)) return FALSE;

  CompStr *pCompStr = LockCompStr();
  BOOL ret = (pCompStr->dwCompStrLen > 0);
  UnlockCompStr();

  return ret;
}

CandInfo *InputContext::LockCandInfo() {
  DebugPrint(TEXT("InputContext::LockCandInfo: locking %p\n"), hCandInfo);
  CandInfo *info = (CandInfo *)ImmLockIMCC(hCandInfo);
  DebugPrint(TEXT("InputContext::LockCandInfo: locked %p\n"), hCandInfo);
  return info;
}

void InputContext::UnlockCandInfo() {
  DebugPrint(TEXT("InputContext::UnlockCandInfo: unlocking %p\n"), hCandInfo);
  ImmUnlockIMCC(hCandInfo);
  DebugPrint(TEXT("InputContext::UnlockCandInfo: unlocked %p\n"), hCandInfo);
}

CompStr *InputContext::LockCompStr() {
  DebugPrint(TEXT("InputContext::LockCompStr: locking %p\n"), hCompStr);
  CompStr *comp_str = (CompStr *)ImmLockIMCC(hCompStr);
  DebugPrint(TEXT("InputContext::LockCompStr: locked %p\n"), hCompStr);
  return comp_str;
}

void InputContext::UnlockCompStr() {
  DebugPrint(TEXT("InputContext::UnlockCompStr: unlocking %p\n"), hCompStr);
  ImmUnlockIMCC(hCompStr);
  DebugPrint(TEXT("InputContext::UnlockCompStr: unlocked %p\n"), hCompStr);
}

LPTRANSMSG InputContext::LockMsgBuf() {
  DebugPrint(TEXT("InputContext::LockMsgBuf: locking %p\n"), hMsgBuf);
  LPTRANSMSG lpTransMsg = (LPTRANSMSG)ImmLockIMCC(hMsgBuf);
  DebugPrint(TEXT("InputContext::LockMsgBuf: locked %p\n"), hMsgBuf);
  return lpTransMsg;
}

void InputContext::UnlockMsgBuf() {
  DebugPrint(TEXT("InputContext::UnlockMsgBuf: unlocking %p\n"), hMsgBuf);
  ImmUnlockIMCC(hMsgBuf);
  DebugPrint(TEXT("InputContext::UnlockMsgBuf: unlocked %p\n"), hMsgBuf);
}

DWORD& InputContext::NumMsgBuf() {
  return dwNumMsgBuf;
}

const DWORD& InputContext::NumMsgBuf() const {
  return dwNumMsgBuf;
}

LPGUIDELINE InputContext::LockGuideLine() {
  DebugPrint(TEXT("InputContext::LockGuideLine: locking %p\n"), hGuideLine);
  LPGUIDELINE guideline = (LPGUIDELINE)ImmLockIMCC(hGuideLine);
  DebugPrint(TEXT("InputContext::LockGuideLine: locked %p\n"), hGuideLine);
  return guideline;
}

void InputContext::UnlockGuideLine() {
  DebugPrint(TEXT("InputContext::UnlockGuideLine: unlocking %p\n"), hGuideLine);
  ImmUnlockIMCC(hGuideLine);
  DebugPrint(TEXT("InputContext::UnlockGuideLine: unlocked %p\n"), hGuideLine);
}

void InputContext::AddChar(WCHAR ch) {
  DebugPrint(TEXT("InputContext::AddChar\n"));

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLogCompStr(log);
    UnlockCompStr();
  }

  if (log.comp_str.empty()) {
    // start composition
    TheIME.GenerateMessage(WM_IME_STARTCOMPOSITION);
  }

  //DWORD dwConversion = lpIMC->Conversion();

  if (log.dwCursorPos > log.comp_str.size()) {
    log.dwCursorPos = log.comp_str.size();
  }
  log.comp_str.insert(log.dwCursorPos, 1, ch);
  log.dwDeltaStart = log.dwCursorPos;
  ++log.dwCursorPos;

  // update info
  log.comp_read_str = log.comp_str;
  log.comp_read_attr.resize(log.comp_str.size(), 0);
  log.comp_attr.resize(log.comp_str.size(), 0);
  log.comp_read_clause.resize(2);
  log.comp_read_clause[0] = 0;
  log.comp_read_clause[1] = (DWORD)log.comp_read_str.size();

  log.comp_clause.resize(2);
  log.comp_clause[0] = 0;
  log.comp_clause[1] = (DWORD)log.comp_str.size();

  // realloc
  DumpCompStr();
  hCompStr = CompStr::ReAlloc(hCompStr, &log);
  DumpCompStr();

  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::AddChar

void InputContext::MakeResult() {
  DebugPrint(TEXT("InputContext::MakeResult\n"));

  // close candidate
  if (HasCandInfo()) {
    CandInfo *lpCandInfo = LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      UnlockCandInfo();
    }
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLogCompStr(log);
    UnlockCompStr();
  }

  // set result
  log.result_read_clause = log.comp_read_clause;
  log.result_read_str = log.comp_read_str;
  log.result_clause = log.comp_clause;
  log.result_str = log.comp_str;

  // clear compostion
  log.dwCursorPos = 0;
  log.dwDeltaStart = 0;
  log.comp_read_attr.clear();
  log.comp_read_clause.clear();
  log.comp_read_str.clear();
  log.comp_attr.clear();
  log.comp_clause.clear();
  log.comp_str.clear();

  // realloc
  DumpCompStr();
  hCompStr = CompStr::ReAlloc(hCompStr, &log);
  DumpCompStr();

  // generate messages to end composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_RESULTALL);
  TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::MakeResult

void InputContext::CancelText() {
  DebugPrint(TEXT("InputContext::CancelText\n"));

  // close candidate
  if (HasCandInfo()) {
    CandInfo *lpCandInfo = LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      UnlockCandInfo();
    }
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  DumpCompStr();
  hCompStr = CompStr::ReAlloc(hCompStr, NULL);
  DumpCompStr();

  // generate messages to end composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL);
  TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::CancelText

void InputContext::DeleteChar(BOOL bBackSpace) {
  DebugPrint(TEXT("InputContext::DeleteChar(%d)\n"), bBackSpace);

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLogCompStr(log);
    UnlockCompStr();
  }

  // delete char
  if (bBackSpace) {
    if (log.dwCursorPos == 0) {
      DebugPrint(TEXT("log.dwCursorPos == 0\n"));
      return;
    } else if (log.dwCursorPos <= log.comp_str.size()) {
      --log.dwCursorPos;
      log.comp_str.erase(log.dwCursorPos);
      log.dwDeltaStart = log.dwCursorPos;
    } else {
      log.dwCursorPos = (DWORD)log.comp_str.size();
      log.dwDeltaStart = log.dwCursorPos;
    }
  } else {
    if (log.dwCursorPos >= log.comp_str.size()) {
      DebugPrint(TEXT("log.dwCursorPos >= log.comp_str.size()\n"));
      return;
    } else {
      log.comp_str.erase(log.dwCursorPos);
      log.dwCursorPos = log.dwDeltaStart;
    }
  }

  // update info
  log.comp_read_str = log.comp_str;
  log.comp_read_attr.resize(log.comp_str.size(), 0);
  log.comp_attr.resize(log.comp_str.size(), 0);
  log.comp_read_clause.resize(2);
  log.comp_read_clause[0] = 0;
  log.comp_read_clause[1] = (DWORD)log.comp_read_str.size();
  log.comp_clause.resize(2);
  log.comp_clause[0] = 0;
  log.comp_clause[1] = (DWORD)log.comp_str.size();

  // realloc
  DumpCompStr();
  hCompStr = CompStr::ReAlloc(hCompStr, &log);
  DumpCompStr();

  if (log.comp_str.empty()) {
    // close candidate if any
    if (HasCandInfo()) {
      CandInfo *lpCandInfo = LockCandInfo();
      if (lpCandInfo) {
        lpCandInfo->Clear();
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
        UnlockCandInfo();
      }
    }

    // generate messages to end composition
    TheIME.GenerateMessage(WM_IME_COMPOSITION);
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
  } else {
    // update composition
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
  }
} // InputContext::DeleteChar

void InputContext::DumpCompStr() {
#ifndef NDEBUG
  CompStr *pCompStr = LockCompStr();
  if (pCompStr) {
    pCompStr->Dump();
    UnlockCompStr();
  } else {
    DebugPrint(TEXT("(no comp str)\n"));
  }
#endif
} // InputContext::DumpCompStr

//////////////////////////////////////////////////////////////////////////////
