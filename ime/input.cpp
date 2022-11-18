// input.cpp --- mzimeja input context and related
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////
// input modes

BOOL IsInputModeOpen(INPUT_MODE imode) {
    switch (imode) {
    case IMODE_FULL_HIRAGANA:
    case IMODE_FULL_KATAKANA:
    case IMODE_FULL_ASCII:
    case IMODE_HALF_KANA:
        return TRUE;
    case IMODE_HALF_ASCII:
    case IMODE_DISABLED:
    default:
        return FALSE;
    }
}

INPUT_MODE InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion) {
    if (bOpen) {
        if (dwConversion & IME_CMODE_FULLSHAPE) {
            if (dwConversion & IME_CMODE_JAPANESE) {
                if (dwConversion & IME_CMODE_KATAKANA) {
                    return IMODE_FULL_KATAKANA;
                } else {
                    return IMODE_FULL_HIRAGANA;
                }
            } else {
                return IMODE_FULL_ASCII;
            }
        } else {
            if (dwConversion & (IME_CMODE_JAPANESE | IME_CMODE_KATAKANA)) {
                return IMODE_HALF_KANA;
            } else {
                return IMODE_HALF_ASCII;
            }
        }
    } else {
        return IMODE_HALF_ASCII;
    }
}

UINT CommandFromInputMode(INPUT_MODE imode) {
    switch (imode) {
    case IMODE_FULL_HIRAGANA:
        return IDM_HIRAGANA;
    case IMODE_FULL_KATAKANA:
        return IDM_FULL_KATAKANA;
    case IMODE_FULL_ASCII:
        return IDM_FULL_ASCII;
    case IMODE_HALF_KANA:
        return IDM_HALF_KATAKANA;
    case IMODE_HALF_ASCII:
        return IDM_HALF_ASCII;
    default:
        return IDM_HALF_ASCII;
    }
}

INPUT_MODE GetInputMode(HIMC hIMC) {
    if (hIMC) {
        DWORD dwConversion, dwSentence;
        ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
        BOOL bOpen = ::ImmGetOpenStatus(hIMC);
        return InputModeFromConversionMode(bOpen, dwConversion);
    }
    return IMODE_DISABLED;
}

INPUT_MODE NextInputMode(INPUT_MODE imode) {
    switch (imode) {
    case IMODE_FULL_HIRAGANA:
        return IMODE_FULL_KATAKANA;
    case IMODE_FULL_KATAKANA:
        return IMODE_FULL_ASCII;
    case IMODE_FULL_ASCII:
        return IMODE_HALF_KANA;
    case IMODE_HALF_KANA:
        return IMODE_HALF_ASCII;
    case IMODE_DISABLED:
        return IMODE_DISABLED;
    case IMODE_HALF_ASCII:
    default:
        return IMODE_FULL_HIRAGANA;
    }
}

void SetInputMode(HIMC hIMC, INPUT_MODE imode) {
    if (imode == IMODE_DISABLED) {
        return;
    }
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
    switch (imode) {
    case IMODE_FULL_HIRAGANA:
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~IME_CMODE_KATAKANA;
        dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
        break;
    case IMODE_FULL_KATAKANA:
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
        break;
    case IMODE_FULL_ASCII:
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~(IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
        dwConversion |= IME_CMODE_FULLSHAPE;
        break;
    case IMODE_HALF_KANA:
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~IME_CMODE_FULLSHAPE;
        dwConversion |= IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
        break;
    case IMODE_HALF_ASCII:
        ImmSetOpenStatus(hIMC, FALSE);
        dwConversion &= ~(IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
        break;
    case IMODE_DISABLED:
        ASSERT(0);
        break;
    }
    ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

BOOL IsRomanMode(HIMC hIMC) {
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
    return (dwConversion & IME_CMODE_ROMAN);
}

void SetRomanMode(HIMC hIMC, BOOL bRoman) {
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
    return InputModeFromConversionMode(fOpen, Conversion());
}

BOOL InputContext::IsRomanMode() const {
    return Conversion() & IME_CMODE_ROMAN;
}

void InputContext::Dump() {
    DebugPrintA("### INPUTCONTEXT ###\n");
    DebugPrintA("hWnd: %p\n", hWnd);
    DebugPrintA("fOpen: %d\n", fOpen);
    DebugPrintA("ptStatusWndPos.x: %d\n", ptStatusWndPos.x);
    DebugPrintA("ptStatusWndPos.y: %d\n", ptStatusWndPos.y);
    DebugPrintA("ptSoftKbdPos.x: %d\n", ptSoftKbdPos.x);
    DebugPrintA("ptSoftKbdPos.y: %d\n", ptSoftKbdPos.y);
    DebugPrintA("fdwConversion: %08X\n", fdwConversion);
    DebugPrintA("fdwSentence: %08X\n", fdwSentence);
    DebugPrintA("lfFont.W.lfHeight: %d\n", lfFont.W.lfHeight);
    DebugPrintA("lfFont.W.lfCharSet: %d\n", lfFont.W.lfCharSet);
    DebugPrintW(L"lfFont.W.lfFaceName: %s\n", lfFont.W.lfFaceName);
    DebugPrintA("cfCompForm.dwStyle: %08X\n", cfCompForm.dwStyle);
    DebugPrintA("cfCompForm.ptCurrentPos.x: %d\n", cfCompForm.ptCurrentPos.x);
    DebugPrintA("cfCompForm.ptCurrentPos.y: %d\n", cfCompForm.ptCurrentPos.y);
    DebugPrintA("cfCompForm.rcArea.left: %d\n", cfCompForm.rcArea.left);
    DebugPrintA("cfCompForm.rcArea.top: %d\n", cfCompForm.rcArea.top);
    DebugPrintA("cfCompForm.rcArea.right: %d\n", cfCompForm.rcArea.right);
    DebugPrintA("cfCompForm.rcArea.bottom: %d\n", cfCompForm.rcArea.bottom);
    DebugPrintA("cfCandForm[0].dwIndex: %d\n", cfCandForm[0].dwIndex);
    DebugPrintA("cfCandForm[0].dwStyle: %08X\n", cfCandForm[0].dwStyle);
    DebugPrintA("cfCandForm[0].ptCurrentPos.x: %d\n", cfCandForm[0].ptCurrentPos.x);
    DebugPrintA("cfCandForm[0].ptCurrentPos.y: %d\n", cfCandForm[0].ptCurrentPos.y);
    DebugPrintA("cfCandForm[0].rcArea.left: %d\n", cfCandForm[0].rcArea.left);
    DebugPrintA("cfCandForm[0].rcArea.top: %d\n", cfCandForm[0].rcArea.top);
    DebugPrintA("cfCandForm[0].rcArea.right: %d\n", cfCandForm[0].rcArea.right);
    DebugPrintA("cfCandForm[0].rcArea.bottom: %d\n", cfCandForm[0].rcArea.bottom);
    DebugPrintA("hCompStr: %p\n", hCompStr);
    DebugPrintA("hCandInfo: %p\n", hCandInfo);
    DebugPrintA("hGuideLine: %p\n", hGuideLine);
    DebugPrintA("hPrivate: %p\n", hPrivate);
    DebugPrintA("dwNumMsgBuf: %d\n", dwNumMsgBuf);
    DebugPrintA("hMsgBuf: %p\n", hMsgBuf);
    DebugPrintA("fdwInit: %08X\n", fdwInit);
}

void InputContext::Initialize() {
    FOOTMARK();
    Dump();

    lfFont.W.lfCharSet = SHIFTJIS_CHARSET;
    lfFont.W.lfFaceName[0] = 0;
    fdwInit |= INIT_LOGFONT;

    fdwConversion = IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE |
                    IME_CMODE_JAPANESE;
    DWORD bIsNonRoman = FALSE;
    if (TheIME.GetUserDword(L"IsNonRoman", &bIsNonRoman)) {
        if (bIsNonRoman) {
            fdwConversion &= ~IME_CMODE_ROMAN;
        } else {
            fdwConversion |= IME_CMODE_ROMAN;
        }
    }
    fdwInit |= INIT_CONVERSION;

    hCompStr = CompStr::ReCreate(hCompStr, NULL);
    hCandInfo = CandInfo::ReCreate(hCandInfo, NULL);
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
    CandInfo *info = (CandInfo *)::ImmLockIMCC(hCandInfo);
    ASSERT(info);
    return info;
}

void InputContext::UnlockCandInfo() {
    ::ImmUnlockIMCC(hCandInfo);
}

CompStr *InputContext::LockCompStr() {
    CompStr *comp_str = (CompStr *)::ImmLockIMCC(hCompStr);
    ASSERT(comp_str);
    return comp_str;
}

void InputContext::UnlockCompStr() {
    ::ImmUnlockIMCC(hCompStr);
}

LPTRANSMSG InputContext::LockMsgBuf() {
    LPTRANSMSG lpTransMsg = (LPTRANSMSG) ::ImmLockIMCC(hMsgBuf);
    ASSERT(lpTransMsg);
    return lpTransMsg;
}

void InputContext::UnlockMsgBuf() {
    ::ImmUnlockIMCC(hMsgBuf);
}

DWORD& InputContext::NumMsgBuf() {
    return dwNumMsgBuf;
}

const DWORD& InputContext::NumMsgBuf() const {
    return dwNumMsgBuf;
}

void InputContext::MakeGuideLine(DWORD dwID) {
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
        lpGuideLine->dwPrivateOffset = 0;
        lpGuideLine->dwPrivateSize = 0;
    }

    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_GUIDELINE, 0);

    UnlockGuideLine();
}

LPGUIDELINE InputContext::LockGuideLine() {
    LPGUIDELINE guideline = (LPGUIDELINE) ::ImmLockIMCC(hGuideLine);
    ASSERT(guideline);
    return guideline;
}

void InputContext::UnlockGuideLine() {
    ::ImmUnlockIMCC(hGuideLine);
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

BOOL InputContext::SelectCand(UINT uCandIndex) {
    // get logical data
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // select a candidate
    if (cand.HasCandInfo() && comp.IsClauseConverted()) {
        if (cand.SelectCand(uCandIndex)) {
            std::wstring str = cand.GetString();
            comp.SetClauseCompString(comp.extra.iClause, str);

            // recreate
            hCompStr = CompStr::ReCreate(hCompStr, &comp);
            hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

            // update composition
            TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL);

            // update candidate
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
            return TRUE;
        }
    }
    return FALSE;
} // InputContext::SelectCand

void InputContext::AddChar(WCHAR chTyped, WCHAR chTranslated) {
    // get logical data
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // if the current clause is converted,
    BOOL bHasResult = FALSE;
    if (comp.IsClauseConverted()) {
        // determinate composition
        comp.MakeResult();
        CloseCandidate();
        bHasResult = TRUE;
    } else {
        // if there is not a composition string, then
        if (comp.comp_str.empty()) {
            // start composition
            TheIME.GenerateMessage(WM_IME_STARTCOMPOSITION);
        }
    }

    // add a character
    comp.AssertValid();
    if ((Conversion() & IME_CMODE_JAPANESE) && !::IsCharAlphaW(chTyped)) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.AddChar(chTyped, chTranslated, Conversion());
    comp.AssertValid();

    // recreate
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    if (bHasResult) {
        LPARAM lParam = GCS_COMPALL | GCS_RESULTALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    } else {
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    }
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

BOOL InputContext::CloseCandidate(BOOL bClearCandInfo /* = TRUE*/) {
    if (HasCandInfo()) {
        if (bClearCandInfo) {
            hCandInfo = CandInfo::ReCreate(hCandInfo, NULL);
        }
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
        return TRUE;
    }
    return FALSE;
}

BOOL InputContext::Convert(BOOL bShift) {
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
        if (comp.IsClauseConverted()) {
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
            TheIME.ConvertSingleClause(comp, cand, bRoman);
        }
    } else {
        if (Conversion() & IME_CMODE_JAPANESE) {
            if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
                comp.AddChar(L'n', L'n', Conversion());
            }
        }
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
        BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
        TheIME.ConvertMultiClause(comp, cand, bRoman);
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
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
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
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
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
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
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

BOOL InputContext::ConvertCode() {
    // get logical data
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // if there is no conposition, we cannot convert it
    if (!comp.HasCompStr()) {
        return FALSE;
    }

    // convert
    if (!cand.HasCandInfo()) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
    }
    TheIME.ConvertCode(comp, cand);

    // recreate candidate and generate message to change candidate
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

    // recreate composition
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // generate message to change composition
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

    return TRUE;
}

void InputContext::Escape() {
    // get logical data
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // if the current clause is converted,
    if (comp.IsClauseConverted()) {
        RevertText();
    } else {
        CancelText();
    }
} // InputContext::Escape

void InputContext::CancelText() {
    // close candidate
    CloseCandidate();

    // reset composition
    hCompStr = CompStr::ReCreate(hCompStr, NULL);

    // generate messages to end composition
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::CancelText

void InputContext::RevertText() {
    // close candidate
    CloseCandidate(FALSE);

    // get logical data
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // return if no comp str
    if (!comp.HasCompStr()) {
        return;
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
    // get logical data
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // delete char
    comp.AssertValid();
    comp.DeleteChar(bBackSpace, Conversion());
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
    // get logical data
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    BOOL bCandChanged = FALSE;
    if (bShift) {
        if (cand.HasCandInfo()) {
            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            if (!TheIME.StretchClauseLeft(comp, cand, bRoman)) {
                return;
            }
            bCandChanged = TRUE;
        }
    } else {
        // move left
        if (comp.MoveLeft()) {
            cand.iClause = comp.extra.iClause;
            bCandChanged = TRUE;
        }
        cand.Dump();
    }

    // recreate
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // update composition
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
    // update candidate
    if (bCandChanged) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveLeft

void InputContext::MoveRight(BOOL bShift) {
    // get logical data
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    BOOL bCandChanged = FALSE;
    if (bShift) {
        if (cand.HasCandInfo()) {
            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            if (!TheIME.StretchClauseRight(comp, cand, bRoman)) {
                return;
            }
            bCandChanged = TRUE;
        }
    } else {
        // move right
        if (comp.MoveRight()) {
            cand.iClause = comp.extra.iClause;
            bCandChanged = TRUE;
        }
        cand.Dump();
    }

    // recreate
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // update composition
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
    // update candidate
    if (bCandChanged) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveRight

void InputContext::MoveUp() {
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
    // get logical data
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // move to head
    comp.AssertValid();
    if (cand.HasCandInfo()) {
        cand.MoveHome();
    } else {
        comp.MoveHome();
    }
    comp.AssertValid();

    // recreate
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // update composition
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    if (cand.HasCandInfo()) {
        // update candidate
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveHome

void InputContext::MoveEnd() {
    // get logical data
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // move to tail
    comp.AssertValid();
    if (cand.HasCandInfo()) {
        cand.MoveEnd();
    } else {
        comp.MoveEnd();
    }
    comp.AssertValid();

    // recreate
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // update composition
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    if (cand.HasCandInfo() && comp.IsClauseConverted()) {
        // update candidate
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveEnd

void InputContext::PageUp() {
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
    CompStr *pCompStr = LockCompStr();
    if (pCompStr) {
        pCompStr->Dump();
        UnlockCompStr();
    } else {
        DebugPrintA("(no comp str)\n");
    }
} // InputContext::DumpCompStr

void InputContext::DumpCandInfo() {
    CandInfo *pCandInfo = LockCandInfo();
    if (pCandInfo) {
        pCandInfo->Dump();
        UnlockCandInfo();
    } else {
        DebugPrintA("(no cand info)\n");
    }
} // InputContext::DumpCandInfo

//////////////////////////////////////////////////////////////////////////////
