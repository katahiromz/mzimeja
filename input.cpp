// input.cpp --- mzimeja input context and related
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

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
  case IMODE_DISABLED:
    assert(0);
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

BOOL InputContext::IsRomanMode() const {
  FOOTMARK();
  return Conversion() & IME_CMODE_ROMAN;
}

void InputContext::Initialize() {
  FOOTMARK();

  if (!HasLogFont()) {
    lfFont.W.lfCharSet = SHIFTJIS_CHARSET;
    lfFont.W.lfFaceName[0] = 0;
    fdwInit |= INIT_LOGFONT;
  }

  if (!HasConversion()) {
    fdwConversion =
      IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
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
  FOOTMARK();
  DebugPrint(TEXT("InputContext::LockCandInfo: locking %p\n"), hCandInfo);
  CandInfo *info = (CandInfo *)::ImmLockIMCC(hCandInfo);
  if (info) {
    DebugPrint(TEXT("InputContext::LockCandInfo: locked %p\n"), hCandInfo);
  } else {
    DebugPrint(TEXT("InputContext::LockCandInfo: not locked %p\n"), hCandInfo);
  }
  return info;
}

void InputContext::UnlockCandInfo() {
  FOOTMARK();
  DebugPrint(TEXT("InputContext::UnlockCandInfo: unlocking %p\n"), hCandInfo);
  BOOL b = ::ImmUnlockIMCC(hCandInfo);
  if (b) {
    DebugPrint(TEXT("InputContext::UnlockCandInfo: unlocked %p\n"), hCandInfo);
  } else {
    DebugPrint(TEXT("InputContext::UnlockCandInfo: not unlocked %p\n"), hCandInfo);
  }
}

CompStr *InputContext::LockCompStr() {
  FOOTMARK();
  DebugPrint(TEXT("InputContext::LockCompStr: locking %p\n"), hCompStr);
  CompStr *comp_str = (CompStr *)::ImmLockIMCC(hCompStr);
  if (comp_str) {
    DebugPrint(TEXT("InputContext::LockCompStr: locked %p\n"), hCompStr);
  } else {
    DebugPrint(TEXT("InputContext::LockCompStr: not locked %p\n"), hCompStr);
  }
  return comp_str;
}

void InputContext::UnlockCompStr() {
  FOOTMARK();
  DebugPrint(TEXT("InputContext::UnlockCompStr: unlocking %p\n"), hCompStr);
  BOOL b = ::ImmUnlockIMCC(hCompStr);
  if (b) {
    DebugPrint(TEXT("InputContext::UnlockCompStr: unlocked %p\n"), hCompStr);
  } else {
    DebugPrint(TEXT("InputContext::UnlockCompStr: not unlocked %p\n"), hCompStr);
  }
}

LPTRANSMSG InputContext::LockMsgBuf() {
  FOOTMARK();
  DebugPrint(TEXT("InputContext::LockMsgBuf: locking %p\n"), hMsgBuf);
  LPTRANSMSG lpTransMsg = (LPTRANSMSG)::ImmLockIMCC(hMsgBuf);
  if (lpTransMsg) {
    DebugPrint(TEXT("InputContext::LockMsgBuf: locked %p\n"), hMsgBuf);
  } else {
    DebugPrint(TEXT("InputContext::LockMsgBuf: not locked %p\n"), hMsgBuf);
  }
  return lpTransMsg;
}

void InputContext::UnlockMsgBuf() {
  FOOTMARK();
  DebugPrint(TEXT("InputContext::UnlockMsgBuf: unlocking %p\n"), hMsgBuf);
  BOOL b = ::ImmUnlockIMCC(hMsgBuf);
  if (b) {
    DebugPrint(TEXT("InputContext::UnlockMsgBuf: unlocked %p\n"), hMsgBuf);
  } else {
    DebugPrint(TEXT("InputContext::UnlockMsgBuf: not unlocked %p\n"), hMsgBuf);
  }
}

DWORD& InputContext::NumMsgBuf() { return dwNumMsgBuf; }

const DWORD& InputContext::NumMsgBuf() const { return dwNumMsgBuf; }

void InputContext::MakeGuideLine(DWORD dwID) {
  FOOTMARK();
  DWORD dwSize =
      sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(TCHAR)) * 2 * sizeof(TCHAR);
  WCHAR *lpStr;

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
  FOOTMARK();
  DebugPrint(TEXT("InputContext::LockGuideLine: locking %p\n"), hGuideLine);
  LPGUIDELINE guideline = (LPGUIDELINE)::ImmLockIMCC(hGuideLine);
  if (guideline) {
    DebugPrint(TEXT("InputContext::LockGuideLine: locked %p\n"), hGuideLine);
  } else {
    DebugPrint(TEXT("InputContext::LockGuideLine: not locked %p\n"), hGuideLine);
  }
  return guideline;
}

void InputContext::UnlockGuideLine() {
  FOOTMARK();
  DebugPrint(TEXT("InputContext::UnlockGuideLine: unlocking %p\n"), hGuideLine);
  BOOL b = ::ImmUnlockIMCC(hGuideLine);
  if (b) {
    DebugPrint(TEXT("InputContext::UnlockGuideLine: unlocked %p\n"), hGuideLine);
  } else {
    DebugPrint(TEXT("InputContext::UnlockGuideLine: not unlocked %p\n"), hGuideLine);
  }
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

  // if the current position has a converted character, then
  if (log.GetCompCharAttr(log.dwCursorPos) != ATTR_INPUT) {
    // determinate composition
    log.MakeResult();
    LPARAM lParam = GCS_COMPALL | GCS_RESULTALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
  }

  // if there is not a composition string, then
  if (log.comp_str.empty()) {
    // start composition
    TheIME.GenerateMessage(WM_IME_STARTCOMPOSITION);
  }

  // add a character
  log.AssertValid();
  log.AddChar(chTyped, chTranslated, Conversion());
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::AddChar

void InputContext::GetCands(LogCandInfo& log, std::wstring& str) {
  FOOTMARK();
  // TODO:
  DWORD dwCount = (DWORD)log.cand_lists.size();
  if (dwCount > 0) {
    DWORD& dwSelection = log.cand_lists[0].dwSelection;
    dwSelection++;
    if (dwSelection >= dwCount) {
      dwSelection = 0;
    }
  } else {
    LogCandList list;
    list.cand_strs.push_back(L"これは");
    list.cand_strs.push_back(L"テスト");
    list.cand_strs.push_back(L"です。");
    log.cand_lists.push_back(list);
  }
  str = log.cand_lists[0].cand_strs[log.cand_lists[0].dwSelection];
}

BOOL InputContext::OpenCandidate() {
  BOOL ret = FALSE;
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();

    LogCandInfo cand;
    CandInfo *cand_info = LockCandInfo();
    if (cand_info) {
      cand_info->GetLog(cand);
      UnlockCandInfo();

      // get candidates
      comp.AssertValid();
      GetCands(cand, comp.comp_str);
      comp.AssertValid();

      // generate message to open candidate
      TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
      // reset candidates
      hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
      // generate message to change candidate
      TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

      ret = TRUE;
    }
  }
  return ret;
}

BOOL InputContext::CloseCandidate() {
  if (HasCandInfo()) {
    hCandInfo = CandInfo::ReCreate(hCandInfo);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
    return TRUE;
  }
  return FALSE;
}

BOOL InputContext::Convert(BOOL bShift) {
  FOOTMARK();

  //// check the presence of dictionary
  //if ((GetFileAttributes(TheIME.m_szDicFileName) == 0xFFFFFFFF) ||
  //    (GetFileAttributes(TheIME.m_szDicFileName) & FILE_ATTRIBUTE_DIRECTORY)) {
  //  MakeGuideLine(MYGL_NODICTIONARY);
  //  return FALSE;
  //}

  // get logical data of composition info
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // if there is no conposition, we cannot convert it
  BOOL bHasCompStr = (comp.comp_str.size() > 0);
  if (!bHasCompStr) {
    return FALSE;
  }

  // get logical data of candidate info
  LogCandInfo cand;
  CandInfo *lpCandInfo = LockCandInfo();
  if (lpCandInfo) {
    lpCandInfo->GetLog(cand);
    UnlockCandInfo();
  }

  // convert
  if (HasCandInfo()) {
    LogCandList& cand_list = cand.cand_lists[comp.extra.iClause];
    if (bShift) {
      cand_list.MovePrev();
    } else {
      cand_list.MoveNext();
    }
    std::wstring str = cand_list.cand_strs[cand_list.dwSelection];
    comp.SetClauseCompString(comp.extra.iClause, str);
  } else {
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
    TheIME.FreeClauseConversion(comp, cand);
  }

  // recreate candidate and generate message to change candidate
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

  // recreate composition
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  // generate message to change composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

  return TRUE;
} // InputContext::Convert

void InputContext::MakeResult() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // set result
  log.AssertValid();
  log.MakeResult();
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  // generate messages to set composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_RESULTALL);
  TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::MakeResult

void InputContext::MakeHiragana() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.AssertValid();
  log.MakeHiragana();
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeKatakana() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.AssertValid();
  log.MakeKatakana();
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeHankaku() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.AssertValid();
  log.MakeHankaku();
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeZenEisuu() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.AssertValid();
  log.MakeZenEisuu();
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeHanEisuu() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // update composition
  log.AssertValid();
  log.MakeHanEisuu();
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::CancelText() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // reset composition
  hCompStr = CompStr::ReCreate(hCompStr);

  // generate messages to end composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
  TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::CancelText

void InputContext::RevertText() {
  FOOTMARK();

  // close candidate if any
  CloseCandidate();

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

  // reset composition of selected clause
  log.AssertValid();
  log.RevertText();
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
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
  log.AssertValid();
  log.DeleteChar(IsRomanMode(), bBackSpace);
  log.AssertValid();

  // if there is no composition, then
  if (log.comp_str.empty()) {
    // close candidate if any
    CloseCandidate();

    // clear composition
    hCompStr = CompStr::ReCreate(hCompStr);

    // generate messages to end composition
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
  } else {
    // recreate
    hCompStr = CompStr::ReCreate(hCompStr, &log);

    // update composition
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
  }
} // InputContext::DeleteChar

void InputContext::MoveLeft(BOOL bShift) {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // move left
  log.AssertValid();
  log.MoveLeft(bShift);
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
} // InputContext::MoveLeft

void InputContext::MoveRight(BOOL bShift) {
  FOOTMARK();

  // get logical data
  LogCompStr log;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(log);
    UnlockCompStr();
  }

  // move right
  log.AssertValid();
  log.MoveRight(bShift);
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

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

  // move to the beginning
  log.AssertValid();
  if (log.HasClauseSelected()) {
    log.extra.iClause = 0;
  } else {
    log.dwCursorPos = 0;
  }
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

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

  // move to the end
  log.AssertValid();
  if (log.HasClauseSelected()) {
    log.extra.iClause = log.comp_clause.size() - 1;
  } else {
    log.dwCursorPos = (DWORD)log.comp_str.size();
  }
  log.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &log);

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
