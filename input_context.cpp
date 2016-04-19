// input_context.cpp --- input context
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

void InputContext::Initialize() {
  if (!HasLogFont()) {
    lfFont.W.lfCharSet = SHIFTJIS_CHARSET;
    SetsLogFont();
  }

  if (!HasConversion()) {
    fdwConversion = IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
    SetsConversion();
  }

  hCompStr = CompStr::ReAlloc(hCompStr, NULL);
  hCandInfo = CandInfo::ReAlloc(hCandInfo);
}

BOOL InputContext::HasCandidate() {
  BOOL fRet = FALSE;

  if (ImmGetIMCCSize(hCandInfo) < sizeof(CANDIDATEINFO)) return FALSE;

  CandInfo *lpCandInfo = LockCandInfo();
  if (lpCandInfo) {
    fRet = (lpCandInfo->dwCount > 0);
    UnlockCandInfo();
  }
  return fRet;
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

//////////////////////////////////////////////////////////////////////////////
