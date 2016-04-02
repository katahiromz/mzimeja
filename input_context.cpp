// input_context.cpp --- input context
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "input_context.h"

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

  hCompStr = ImmReSizeIMCC(hCompStr, sizeof(MZCOMPSTR));
  LPCOMPOSITIONSTRING lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->dwSize = sizeof(MZCOMPSTR);
    UnlockCompStr();
  }

  hCandInfo = ImmReSizeIMCC(hCandInfo, sizeof(MZCAND));
  LPCANDIDATEINFO lpCandInfo = LockCandInfo();
  if (lpCandInfo) {
    lpCandInfo->dwSize = sizeof(MZCAND);
    UnlockCandInfo();
  }
}

BOOL InputContext::IsCandidate() {
  BOOL fRet = FALSE;

  if (ImmGetIMCCSize(hCandInfo) < sizeof(CANDIDATEINFO)) return FALSE;

  LPCANDIDATEINFO lpCandInfo = LockCandInfo();
  if (lpCandInfo) {
    fRet = (lpCandInfo->dwCount > 0);
    UnlockCandInfo();
  }
  return fRet;
}

LPCANDIDATEINFO InputContext::LockCandInfo() {
  return (LPCANDIDATEINFO)ImmLockIMCC(hCandInfo);
}

void InputContext::UnlockCandInfo() {
  ImmUnlockIMCC(hCandInfo);
}

LPCOMPOSITIONSTRING InputContext::LockCompStr() {
  return (LPCOMPOSITIONSTRING)ImmLockIMCC(hCompStr);
}

void InputContext::UnlockCompStr() {
  ImmUnlockIMCC(hCompStr);
}

LPTRANSMSG InputContext::LockTransMsg() {
  return (LPTRANSMSG)ImmLockIMCC(hMsgBuf);
}

void InputContext::UnlockTransMsg() {
    ImmUnlockIMCC(hMsgBuf);
}

DWORD& InputContext::NumTransMsg() {
  return dwNumMsgBuf;
}

const DWORD& InputContext::NumTransMsg() const {
  return dwNumMsgBuf;
}

LPGUIDELINE InputContext::LockGuideLine() {
  return (LPGUIDELINE)ImmLockIMCC(hGuideLine);
}

void InputContext::UnlockGuideLine() {
  ImmUnlockIMCC(hGuideLine);
}

//////////////////////////////////////////////////////////////////////////////
