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
    fdwConversion = IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
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

BOOL InputContext::HasConvertedCompStr() {
  if (ImmGetIMCCSize(hCompStr) <= sizeof(COMPOSITIONSTRING)) return FALSE;

  CompStr *pCompStr = LockCompStr();
  BOOL ret = (pCompStr->dwCompStrLen > 0) &&
             (pCompStr->GetCompAttr()[0] != ATTR_INPUT);
  UnlockCompStr();

  return ret;
}

CandInfo *InputContext::LockCandInfo() {
  return (CandInfo *)ImmLockIMCC(hCandInfo);
}

void InputContext::UnlockCandInfo() {
  ImmUnlockIMCC(hCandInfo);
}

CompStr *InputContext::LockCompStr() {
  return (CompStr *)ImmLockIMCC(hCompStr);
}

void InputContext::UnlockCompStr() {
  ImmUnlockIMCC(hCompStr);
}

LPTRANSMSG InputContext::LockMsgBuf() {
  return (LPTRANSMSG)ImmLockIMCC(hMsgBuf);
}

void InputContext::UnlockMsgBuf() {
    ImmUnlockIMCC(hMsgBuf);
}

DWORD& InputContext::NumMsgBuf() {
  return dwNumMsgBuf;
}

const DWORD& InputContext::NumMsgBuf() const {
  return dwNumMsgBuf;
}

LPGUIDELINE InputContext::LockGuideLine() {
  return (LPGUIDELINE)ImmLockIMCC(hGuideLine);
}

void InputContext::UnlockGuideLine() {
  ImmUnlockIMCC(hGuideLine);
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
    TheApp.GenerateMessage(WM_IME_STARTCOMPOSITION);
  }

  //DWORD dwConversion = lpIMC->Conversion();

  if (log.dwCursorPos > log.comp_str.size()) {
    log.dwCursorPos = log.comp_str.size();
  }
  log.comp_str.insert(log.dwCursorPos, 1, ch);
  log.dwDeltaStart = log.dwCursorPos;
  ++log.dwCursorPos;

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
  TheApp.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::AddChar

void InputContext::CancelText() {
  DebugPrint(TEXT("InputContext::CancelText\n"));

  // close candidate
  if (HasCandInfo()) {
    CandInfo *lpCandInfo = LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      UnlockCandInfo();
    }
    TheApp.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  DumpCompStr();
  hCompStr = CompStr::ReAlloc(hCompStr, NULL);
  DumpCompStr();

  // generate messages to end composition
  TheApp.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL);
  TheApp.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::CancelText

void InputContext::DeleteChar(BOOL bBackSpace) {
  DebugPrint(TEXT("InputContext::DeleteChar\n"));

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
      return;
    } else if (log.dwCursorPos < log.comp_str.size()) {
      --log.dwCursorPos;
      log.comp_str.erase(log.dwCursorPos);
    } else {
      log.dwCursorPos = (DWORD)log.comp_str.size();
    }
  } else {
    if (log.dwCursorPos >= log.comp_str.size()) {
      return;
    } else {
      log.comp_str.erase(log.dwCursorPos);
    }
  }

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
        TheApp.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
        UnlockCandInfo();
      }
    }

    // generate messages to end composition
    TheApp.GenerateMessage(WM_IME_COMPOSITION);
    TheApp.GenerateMessage(WM_IME_ENDCOMPOSITION);
  } else {
    // update composition
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
    TheApp.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
  }
} // InputContext::DeleteChar

void InputContext::DumpCompStr() {
#ifndef NDEBUG
  CompStr *pCompStr = LockCompStr();
  if (pCompStr) {
    pCompStr->Dump();
  }
  UnlockCompStr();
#endif
} // InputContext::DumpCompStr

//////////////////////////////////////////////////////////////////////////////
