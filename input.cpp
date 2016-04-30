// input.cpp --- input
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////
// input modes

BOOL IsInputModeOpen(INPUT_MODE imode) {
  FOOTMARK();
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
  case IMODE_ZEN_KATAKANA:
  case IMODE_ZEN_EISUU:
  case IMODE_HAN_KANA:
    return TRUE;
  case IMODE_HAN_EISUU:
  case IMODE_DISABLED:
  default:
    return FALSE;
  }
}

INPUT_MODE InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion) {
  FOOTMARK();
  if (bOpen) {
    if (dwConversion & IME_CMODE_FULLSHAPE) {
      if (dwConversion & IME_CMODE_JAPANESE) {
        if (dwConversion & IME_CMODE_KATAKANA) {
          return IMODE_ZEN_KATAKANA;
        } else {
          return IMODE_ZEN_HIRAGANA;
        }
      } else {
        return IMODE_ZEN_EISUU;
      }
    } else {
      if (dwConversion & (IME_CMODE_JAPANESE | IME_CMODE_KATAKANA)) {
        return IMODE_HAN_KANA;
      } else {
        return IMODE_HAN_EISUU;
      }
    }
  } else {
    return IMODE_HAN_EISUU;
  }
}

UINT CommandFromInputMode(INPUT_MODE imode) {
  FOOTMARK();
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    return IDM_HIRAGANA;
  case IMODE_ZEN_KATAKANA:
    return IDM_ZEN_KATAKANA;
  case IMODE_ZEN_EISUU:
    return IDM_ZEN_ALNUM;
  case IMODE_HAN_KANA:
    return IDM_HAN_KATAKANA;
  case IMODE_HAN_EISUU:
    return IDM_ALNUM;
  default:
    return IDM_ALNUM;
  }
}

INPUT_MODE GetInputMode(HIMC hIMC) {
  FOOTMARK();
  if (hIMC) {
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
    BOOL bOpen = ::ImmGetOpenStatus(hIMC);
    return InputModeFromConversionMode(bOpen, dwConversion);
  }
  return IMODE_DISABLED;
}

INPUT_MODE NextInputMode(INPUT_MODE imode) {
  FOOTMARK();
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    return IMODE_ZEN_KATAKANA;
  case IMODE_ZEN_KATAKANA:
    return IMODE_ZEN_EISUU;
  case IMODE_ZEN_EISUU:
    return IMODE_HAN_KANA;
  case IMODE_HAN_KANA:
    return IMODE_HAN_EISUU;
  case IMODE_DISABLED:
    return IMODE_DISABLED;
  case IMODE_HAN_EISUU:
  default:
    return IMODE_ZEN_HIRAGANA;
  }
}

void SetInputMode(HIMC hIMC, INPUT_MODE imode) {
  FOOTMARK();
  if (imode == IMODE_DISABLED) {
    return;
  }
  DWORD dwConversion, dwSentence;
  ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion &= ~IME_CMODE_KATAKANA;
    dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
    break;
  case IMODE_ZEN_KATAKANA:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
    break;
  case IMODE_ZEN_EISUU:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion &= ~(IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
    dwConversion |= IME_CMODE_FULLSHAPE;
    break;
  case IMODE_HAN_KANA:
    ImmSetOpenStatus(hIMC, TRUE);
    dwConversion &= ~IME_CMODE_FULLSHAPE;
    dwConversion |= IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
    break;
  case IMODE_HAN_EISUU:
    ImmSetOpenStatus(hIMC, FALSE);
    dwConversion &= ~(IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
    break;
  }
  ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

BOOL IsRomanMode(HIMC hIMC) {
  FOOTMARK();
  DWORD dwConversion, dwSentence;
  ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
  return (dwConversion & IME_CMODE_ROMAN);
}

void SetRomanMode(HIMC hIMC, BOOL bRoman) {
  FOOTMARK();
  DWORD dwConversion, dwSentence;
  ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
  if (bRoman) {
    dwConversion |= IME_CMODE_ROMAN;
  } else {
    dwConversion &= ~IME_CMODE_ROMAN;
  }
  ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

//////////////////////////////////////////////////////////////////////////////
// input context

INPUT_MODE InputContext::GetInputMode() const {
  FOOTMARK();
  return InputModeFromConversionMode(fOpen, Conversion());
}

BOOL InputContext::IsRoman() const {
  return Conversion() & IME_CMODE_ROMAN;
}

void InputContext::Initialize() {
  FOOTMARK();
  if (!HasLogFont()) {
    lfFont.W.lfCharSet = SHIFTJIS_CHARSET;
    lfFont.W.lfFaceName[0] = L'\0';
    fdwInit |= INIT_LOGFONT;
  }

  if (!HasConversion()) {
    fdwConversion = IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
    fdwInit |= INIT_CONVERSION;
  }

  hCompStr = CompStr::ReCreate(hCompStr);
  hCandInfo = CandInfo::ReCreate(hCandInfo);
}

BOOL InputContext::HasCandInfo() {
  FOOTMARK();
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
  FOOTMARK();
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

void InputContext::MakeGuideLine(DWORD dwID) {
  FOOTMARK();
  DWORD dwSize =
      sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(TCHAR)) * 2 * sizeof(TCHAR);
  LPTSTR lpStr;

  hGuideLine = ImmReSizeIMCC(hGuideLine, dwSize);
  LPGUIDELINE lpGuideLine = LockGuideLine();

  lpGuideLine->dwSize = dwSize;
  lpGuideLine->dwLevel = glTable[dwID].dwLevel;
  lpGuideLine->dwIndex = glTable[dwID].dwIndex;
  lpGuideLine->dwStrOffset = sizeof(GUIDELINE);
  lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset);
  LoadString(TheIME.m_hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
  lpGuideLine->dwStrLen = lstrlen(lpStr);

  if (glTable[dwID].dwPrivateID) {
    lpGuideLine->dwPrivateOffset =
        sizeof(GUIDELINE) + (MAXGLCHAR + 1) * sizeof(TCHAR);
    lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwPrivateOffset);
    LoadString(TheIME.m_hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
    lpGuideLine->dwPrivateSize = lstrlen(lpStr) * sizeof(TCHAR);
  } else {
    lpGuideLine->dwPrivateOffset = 0L;
    lpGuideLine->dwPrivateSize = 0L;
  }

  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_GUIDELINE, 0);

  UnlockGuideLine();
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

void InputContext::AddChar(WCHAR chTyped, WCHAR chTranslated) {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  if (log.comp_str.empty()) {
    // start composition
    TheIME.GenerateMessage(WM_IME_STARTCOMPOSITION);
  }

  // fix cursor pos
  std::wstring& comp_str = log.comp_str;
  if ((DWORD)comp_str.size() < log.dwCursorPos) {
    log.dwCursorPos = (DWORD)comp_str.size();
  }

  INPUT_MODE imode = GetInputMode();
  log.AddChar(chTyped, chTranslated, imode);

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::AddChar

void InputContext::GetCands(LogCandInfo& log_cand_info, std::wstring& str) {
  DWORD dwCount = (DWORD)log_cand_info.cand_strs.size();
  if (dwCount > 0) {
    log_cand_info.dwSelection++;
    if (log_cand_info.dwSelection >= dwCount) {
      log_cand_info.dwSelection = 0;
    }
  } else {
    log_cand_info.dwSelection = 0;
    log_cand_info.cand_strs.push_back(L"これは");
    log_cand_info.cand_strs.push_back(L"テスト");
    log_cand_info.cand_strs.push_back(L"です。");
  }
  str = log_cand_info.cand_strs[log_cand_info.dwSelection];
}

BOOL InputContext::DoConvert() {
  FOOTMARK();

  if ((GetFileAttributes(TheIME.m_szDicFileName) == 0xFFFFFFFF) ||
      (GetFileAttributes(TheIME.m_szDicFileName) & FILE_ATTRIBUTE_DIRECTORY)) {
    MakeGuideLine(MYGL_NODICTIONARY);
    return FALSE;
  }

  BOOL bHasCompStr = FALSE, bIsBeingConverted = FALSE;
  LogCompStr log_comp_str;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log_comp_str);
    bIsBeingConverted = lpCompStr->IsBeingConverted();
    UnlockCompStr();
    bHasCompStr = (log_comp_str.comp_str.size() > 0);
  }

  if (!bHasCompStr) {
    return FALSE;
  }

  if (bIsBeingConverted) {
    // the composition string is being converted.
    if (!HasCandInfo()) {
      // there is no candidate.
      // open candidate
      TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
    }
  }

  // get logical data of candidate info
  LogCandInfo log_cand_info;
  CandInfo *lpCandInfo = LockCandInfo();
  if (lpCandInfo) {
    lpCandInfo->GetLog(log_cand_info);
    UnlockCandInfo();
  }

  // get candidates
  std::wstring str = log_comp_str.comp_str;
  GetCands(log_cand_info, str);
  log_comp_str.comp_str = str;

  // recreate candidate
  hCandInfo = CandInfo::ReCreate(hCandInfo, &log_cand_info);
  // generate message to change candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

  // set composition string
  log_comp_str.comp_str = str;
  log_comp_str.comp_attr.assign(str.size(), ATTR_TARGET_CONVERTED);
  log_comp_str.comp_clause.resize(2);
  log_comp_str.comp_clause[0] = 0;
  log_comp_str.comp_clause[1] = str.size();

  // recreate composition
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log_comp_str);
  DumpCompStr();

  // generate message to change composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

  return TRUE;
} // InputContext::DoConvert

void InputContext::MakeResult() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
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

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // generate messages to end composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_RESULTALL);
  TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::MakeResult

void InputContext::MakeHiragana() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.MakeHiragana();

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // generate messages to update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_RESULTALL);
}

void InputContext::MakeKatakana() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.MakeKatakana();

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // generate messages to update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_RESULTALL);
}

void InputContext::MakeHankaku() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.MakeHankaku();

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // generate messages to update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_RESULTALL);
}

void InputContext::MakeZenEisuu() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.MakeZenEisuu();

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // generate messages to update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_RESULTALL);
}

void InputContext::MakeHanEisuu() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.MakeHanEisuu();

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // generate messages to update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_RESULTALL);
}

void InputContext::CancelText() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr);
  DumpCompStr();

  // generate messages to end composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL);
  TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::CancelText

void InputContext::RevertText() {
  FOOTMARK();

  // close candidate
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  // return if no composition string
  if (!HasCompStr()) {
    return;
  }

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // reset composition
  log.Revert();

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::RevertText

void InputContext::DeleteChar(BOOL bBackSpace) {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // delete char
  log.DeleteChar(bBackSpace);

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  if (log.comp_str.empty()) {
    // close candidate if any
    if (HasCandInfo()) {
      hCandInfo = CandInfo::ReCreate(hCandInfo);
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

void InputContext::MoveLeft(BOOL bShift) {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  BOOL bIsBeingConverted = FALSE;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    bIsBeingConverted = lpCompStr->IsBeingConverted();
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  log.MoveLeft(bShift);

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
} // InputContext::MoveLeft

void InputContext::MoveRight(BOOL bShift) {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  BOOL bIsBeingConverted = FALSE;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    bIsBeingConverted = lpCompStr->IsBeingConverted();
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  log.MoveRight(bShift);

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
} // InputContext::MoveRight

void InputContext::MoveToBeginning() {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  log.dwCursorPos = 0;
  log.extra.dwPhonemeCursor = 0;
  log.extra.dwCharExtra = 0;

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
}

void InputContext::MoveToEnd() {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  log.dwCursorPos = (DWORD)log.comp_str.size();
  log.extra.dwPhonemeCursor = (DWORD)extra.hiragana_phonemes.size();
  log.extra.dwCharExtra = 0;

  // recreate
  DumpCompStr();
  hCompStr = CompStr::ReCreate(hCompStr, &log);
  DumpCompStr();

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
}

void InputContext::DumpCompStr() {
  FOOTMARK();
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

void InputContext::DumpCandInfo() {
  FOOTMARK();
#ifndef NDEBUG
  CandInfo *pCandInfo = LockCandInfo();
  if (pCandInfo) {
    pCandInfo->Dump();
    UnlockCandInfo();
  } else {
    DebugPrint(TEXT("(no cand info)\n"));
  }
#endif
} // InputContext::DumpCandInfo

//////////////////////////////////////////////////////////////////////////////
