// input_context.cpp --- input context
//////////////////////////////////////////////////////////////////////////////

#include "input_context.h"

//////////////////////////////////////////////////////////////////////////////

void InputContext::Initialize() {
  // TODO:
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

LPGUIDELINE InputContext::LockGuideLine() {
  return (LPGUIDELINE)ImmLockIMCC(hGuideLine);
}

void InputContext::UnlockGuideLine() {
  ImmUnlockIMCC(hGuideLine);
}

//////////////////////////////////////////////////////////////////////////////
