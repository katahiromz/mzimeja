// input_context.cpp --- input context
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////

void InputContext::Initialize() {
  if (!HasLogFont()) {
    lfFont.W.lfCharSet = SHIFTJIS_CHARSET;
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

//////////////////////////////////////////////////////////////////////////////
