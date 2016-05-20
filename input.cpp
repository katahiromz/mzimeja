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

  hCompStr = CompStr::ReCreate(hCompStr, NULL);
  hCandInfo = CandInfo::ReCreate(hCandInfo, NULL);
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

void InputContext::GetLogObjects(LogCompStr& comp, LogCandInfo& cand) {
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  CandInfo *lpCandInfo = LockCandInfo();
  if (lpCandInfo) {
    lpCandInfo->GetLog(cand);
    UnlockCandInfo();
  }
} // InputContext::GetLogObjects

void InputContext::AddChar(WCHAR chTyped, WCHAR chTranslated) {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // if the current position has a converted character, then
  if (comp.GetClauseAttr(comp.extra.iClause) != ATTR_INPUT) {
    // determinate composition
    MakeResult();

    lpCompStr = LockCompStr();
    if (lpCompStr) {
      lpCompStr->GetLog(comp);
      UnlockCompStr();
    }
  }

  // if there is not a composition string, then
  if (comp.comp_str.empty()) {
    // start composition
    TheIME.GenerateMessage(WM_IME_STARTCOMPOSITION);
  }

  // add a character
  comp.AssertValid();
  comp.AddChar(chTyped, chTranslated, Conversion());
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::AddChar

BOOL InputContext::OpenCandidate() {
  BOOL ret = FALSE;
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();

    LogCandInfo cand;
    CandInfo *lpCandInfo = LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->GetLog(cand);
      UnlockCandInfo();

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
    hCandInfo = CandInfo::ReCreate(hCandInfo, NULL);
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

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // if there is no conposition, we cannot convert it
  if (!comp.HasCompStr()) {
    return FALSE;
  }

  // convert
  if (cand.HasCandInfo()) {
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
    BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
    TheIME.PluralClauseConversion(comp, cand, bRoman);
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
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // set result
  comp.AssertValid();
  comp.MakeResult();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  // generate messages to set composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_RESULTALL);
  TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::MakeResult

void InputContext::MakeHiragana() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // update composition
  comp.AssertValid();
  comp.MakeHiragana();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeKatakana() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // update composition
  comp.AssertValid();
  comp.MakeKatakana();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeHankaku() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // update composition
  comp.AssertValid();
  comp.MakeHankaku();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeZenEisuu() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // update composition
  comp.AssertValid();
  comp.MakeZenEisuu();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::MakeHanEisuu() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // get logical data
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // update composition
  comp.AssertValid();
  comp.MakeHanEisuu();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  // generate messages to update composition
  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

void InputContext::CancelText() {
  FOOTMARK();

  // close candidate
  CloseCandidate();

  // reset composition
  hCompStr = CompStr::ReCreate(hCompStr, NULL);

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
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // reset composition of selected clause
  comp.AssertValid();
  comp.RevertText();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);

  LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::RevertText

void InputContext::DeleteChar(BOOL bBackSpace) {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  CompStr *lpCompStr = LockCompStr();
  if (lpCompStr) {
    lpCompStr->GetLog(comp);
    UnlockCompStr();
  }

  // delete char
  comp.AssertValid();
  comp.DeleteChar(IsRomanMode(), bBackSpace);
  comp.AssertValid();

  // if there is no composition, then
  if (comp.comp_str.empty()) {
    // close candidate if any
    CloseCandidate();

    // clear composition
    hCompStr = CompStr::ReCreate(hCompStr, NULL);

    // generate messages to end composition
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
  } else {
    // recreate
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // update composition
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
  }
} // InputContext::DeleteChar

void InputContext::MoveLeft(BOOL bShift) {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // move left
  comp.AssertValid();
  comp.MoveLeft(bShift);
  if (!bShift) cand.MoveLeft();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);

  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::MoveLeft

void InputContext::MoveRight(BOOL bShift) {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // move right
  comp.AssertValid();
  comp.MoveRight(bShift);
  if (!bShift) cand.MoveRight();
  comp.AssertValid();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::MoveRight

void InputContext::MoveUp() {
  FOOTMARK();
  if (!HasCandInfo()) return;

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // candidate up
  cand.MovePrev();
  std::wstring str = cand.GetString();
  comp.SetClauseCompString(cand.iClause, str);

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
}

void InputContext::MoveDown() {
  FOOTMARK();
  if (!HasCandInfo()) return;

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // candidate down
  cand.MoveNext();
  std::wstring str = cand.GetString();
  comp.SetClauseCompString(cand.iClause, str);

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
}

void InputContext::MoveHome() {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // move to head
  comp.MoveHome();
  cand.MoveHome();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::MoveHome

void InputContext::MoveEnd() {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // move to tail
  comp.MoveEnd();
  cand.MoveEnd();

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::MoveEnd

void InputContext::PageUp() {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // go to previous page
  cand.PageUp();
  std::wstring str = cand.GetString();
  comp.SetClauseCompString(cand.iClause, str);

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::PageUp

void InputContext::PageDown() {
  FOOTMARK();

  // get logical data
  LogCompStr comp;
  LogCandInfo cand;
  GetLogObjects(comp, cand);

  // move to next page
  cand.PageDown();
  std::wstring str = cand.GetString();
  comp.SetClauseCompString(cand.iClause, str);

  // recreate
  hCompStr = CompStr::ReCreate(hCompStr, &comp);
  hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

  // update composition
  TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
  // update candidate
  TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::PageDown

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
