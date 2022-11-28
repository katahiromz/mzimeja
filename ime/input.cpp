// input.cpp --- mzimeja input context and related
// ���̓R���e�L�X�g�֘A�B
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////
// input modes

// ���̓��[�h����ϊ����[�h���J����Ă��邩�ǂ�����Ԃ��B
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

// IME�ϊ����[�h������̓��[�h��Ԃ��B
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

// ���̓��[�h����R�}���hID��Ԃ��B
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

// ���̓��[�h���擾����B
INPUT_MODE GetInputMode(HIMC hIMC) {
    if (hIMC) {
        DWORD dwConversion, dwSentence;
        ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
        BOOL bOpen = ::ImmGetOpenStatus(hIMC);
        return InputModeFromConversionMode(bOpen, dwConversion);
    }
    return IMODE_DISABLED;
}

// ���̓��̓��[�h�B
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

// ���̓��[�h��ݒ�B
void SetInputMode(HIMC hIMC, INPUT_MODE imode) {
    if (imode == IMODE_DISABLED) {
        return;
    }
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
    switch (imode) {
    case IMODE_FULL_HIRAGANA: // �S�p�Ђ炪�ȃ��[�h�B
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~IME_CMODE_KATAKANA;
        dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
        break;
    case IMODE_FULL_KATAKANA: // �S�p�J�^�J�i���[�h�B
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
        break;
    case IMODE_FULL_ASCII: // �S�p�p�����[�h�B
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~(IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
        dwConversion |= IME_CMODE_FULLSHAPE;
        break;
    case IMODE_HALF_KANA: // ���p�J�i���[�h�B
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~IME_CMODE_FULLSHAPE;
        dwConversion |= IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
        break;
    case IMODE_HALF_ASCII: // ���p�p�����[�h�B
        ImmSetOpenStatus(hIMC, FALSE);
        dwConversion &= ~(IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
        break;
    case IMODE_DISABLED: // �����B
        ASSERT(0);
        break;
    }
    // �ϊ����[�h���Z�b�g����B
    ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

// ���[�}�����̓��[�h���H
BOOL IsRomanMode(HIMC hIMC) {
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence); // �ϊ����[�h���擾�B
    return (dwConversion & IME_CMODE_ROMAN); // ���[�}�����H
}

// ���[�}�����̓��[�h��ݒ肷��B
void SetRomanMode(HIMC hIMC, BOOL bRoman) {
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence); // �ϊ����[�h���擾�B
    if (bRoman) {
        dwConversion |= IME_CMODE_ROMAN;
    } else {
        dwConversion &= ~IME_CMODE_ROMAN;
    }
    ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence); // �ϊ����[�h��ݒ�B
}

//////////////////////////////////////////////////////////////////////////////
// input context

// ���̓R���e�L�X�g������̓��[�h���擾�B
INPUT_MODE InputContext::GetInputMode() const {
    return InputModeFromConversionMode(fOpen, Conversion());
}

// ���̓R���e�L�X�g���烍�[�}�����̓��[�h���擾�B
BOOL InputContext::IsRomanMode() const {
    return Conversion() & IME_CMODE_ROMAN;
}

// ���̓R���e�L�X�g���̃_���v�B
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

// ���̓R���e�L�X�g�̏������B
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

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, NULL);
    hCandInfo = CandInfo::ReCreate(hCandInfo, NULL);
}

// ����񂪂��邩�H
BOOL InputContext::HasCandInfo() {
    BOOL fRet = FALSE;

    if (ImmGetIMCCSize(hCandInfo) < sizeof(CANDIDATEINFO)) { // �T�C�Y�͗L�����H
        return FALSE;
    }

    CandInfo *lpCandInfo = LockCandInfo(); // ���������b�N�B
    if (lpCandInfo) {
        fRet = (lpCandInfo->dwCount > 0); // �����̍��ڂ����邩�H
        UnlockCandInfo(); // �����̃��b�N�������B
    }
    return fRet;
}

// ���m�蕶���񂪂��邩�H
BOOL InputContext::HasCompStr() {
    if (ImmGetIMCCSize(hCompStr) <= sizeof(COMPOSITIONSTRING)) { // �T�C�Y�͗L�����H
        return FALSE;
    }

    BOOL ret = FALSE;
    CompStr *pCompStr = LockCompStr(); // ���m�蕶��������b�N�B
    if (pCompStr) {
        ret = (pCompStr->dwCompStrLen > 0); // ���m�蕶����̒��������邩�H
        UnlockCompStr(); // ���m�蕶����̃��b�N�������B
    }
    return ret;
}

// ���������b�N�B
CandInfo *InputContext::LockCandInfo() {
    CandInfo *info = (CandInfo *)::ImmLockIMCC(hCandInfo);
    ASSERT(info);
    return info;
}

// �����̃��b�N�������B
void InputContext::UnlockCandInfo() {
    ::ImmUnlockIMCC(hCandInfo);
}

// ���m�蕶��������b�N�B
CompStr *InputContext::LockCompStr() {
    CompStr *comp_str = (CompStr *)::ImmLockIMCC(hCompStr);
    ASSERT(comp_str);
    return comp_str;
}

// ���m�蕶����̃��b�N�������B
void InputContext::UnlockCompStr() {
    ::ImmUnlockIMCC(hCompStr);
}

// ���b�Z�[�W�o�b�t�@�����b�N�B
LPTRANSMSG InputContext::LockMsgBuf() {
    LPTRANSMSG lpTransMsg = (LPTRANSMSG) ::ImmLockIMCC(hMsgBuf);
    ASSERT(lpTransMsg);
    return lpTransMsg;
}

// ���b�Z�[�W�o�b�t�@�̃��b�N�������B
void InputContext::UnlockMsgBuf() {
    ::ImmUnlockIMCC(hMsgBuf);
}

// ���b�Z�[�W�o�b�t�@�̗v�f�̌��B
DWORD& InputContext::NumMsgBuf() {
    return dwNumMsgBuf;
}

// ���b�Z�[�W�o�b�t�@�̗v�f�̌��B
const DWORD& InputContext::NumMsgBuf() const {
    return dwNumMsgBuf;
}

// �K�C�h���C�����쐬�B
void InputContext::MakeGuideLine(DWORD dwID) {
    DWORD dwSize = sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(TCHAR)) * 2 * sizeof(TCHAR);
    WCHAR *lpStr;

    hGuideLine = ImmReSizeIMCC(hGuideLine, dwSize); // �K�C�h���C�����č쐬�B
    LPGUIDELINE lpGuideLine = LockGuideLine(); // �K�C�h���C�������b�N�B

    // �K�C�h���C���̊�{����ݒ�B
    lpGuideLine->dwSize = dwSize;
    lpGuideLine->dwLevel = glTable[dwID].dwLevel;
    lpGuideLine->dwIndex = glTable[dwID].dwIndex;
    lpGuideLine->dwStrOffset = sizeof(GUIDELINE);
    lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset);
    LoadString(TheIME.m_hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
    lpGuideLine->dwStrLen = lstrlen(lpStr);

    // �K�C�h���C���̗]�����ݒ肷��B
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

    // �K�C�h���C���̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_GUIDELINE, 0);

    UnlockGuideLine(); // �K�C�h���C���̃��b�N�������B
}

// �K�C�h���C�������b�N�B
LPGUIDELINE InputContext::LockGuideLine() {
    LPGUIDELINE guideline = (LPGUIDELINE) ::ImmLockIMCC(hGuideLine);
    ASSERT(guideline);
    return guideline;
}

// �K�C�h���C���̃��b�N�������B
void InputContext::UnlockGuideLine() {
    ::ImmUnlockIMCC(hGuideLine);
}

// �_���I�u�W�F�N�g���擾����B
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

// ����I������B
BOOL InputContext::SelectCand(UINT uCandIndex) {
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand); // �_�������擾�B

    // select a candidate
    if (cand.HasCandInfo() && comp.IsClauseConverted()) {
        if (cand.SelectCand(uCandIndex)) {
            std::wstring str = cand.GetString();
            comp.SetClauseCompString(comp.extra.iClause, str);

            // ���m�蕶����ƌ����̍č쐬�B
            hCompStr = CompStr::ReCreate(hCompStr, &comp);
            hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

            // ���m�蕶����̃��b�Z�[�W�𐶐��B
            TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL);

            // update candidate
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
            return TRUE;
        }
    }
    return FALSE;
} // InputContext::SelectCand

// ������ǉ��B
void InputContext::AddChar(WCHAR chTyped, WCHAR chTranslated) {
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr(); // ���m�蕶��������b�N�B
    if (lpCompStr) {
        lpCompStr->GetLog(comp); // ���m�蕶����̘_���f�[�^���擾�B
        UnlockCompStr(); // ���m�蕶����̃��b�N�������B
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

    // �P������ǉ��B
    comp.AssertValid();
    if ((Conversion() & IME_CMODE_JAPANESE) && !::IsCharAlphaW(chTyped)) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.AddChar(chTyped, chTranslated, Conversion());
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    if (bHasResult) { // ���ʂ�����H
        LPARAM lParam = GCS_COMPALL | GCS_RESULTALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    } else { // ���ʂ��Ȃ��H
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    }
} // InputContext::AddChar

// ���E�B���h�E���J���B
BOOL InputContext::OpenCandidate() {
    BOOL ret = FALSE;
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr(); // ���m�蕶��������b�N�B
    if (!lpCompStr)
        return ret;

    lpCompStr->GetLog(comp); // ���m�蕶����̘_���f�[�^���擾�B
    UnlockCompStr(); // ���m�蕶����̃��b�N�������B

    LogCandInfo cand;
    CandInfo *lpCandInfo = LockCandInfo(); // ���������b�N�B
    if (lpCandInfo) {
        lpCandInfo->GetLog(cand); // �����̘_���f�[�^���擾�B
        UnlockCandInfo(); // �����̃��b�N�������B

        // �����J�����b�Z�[�W�𐶐��B
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
        // �������č쐬�B
        hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
        // �����̕ύX���b�Z�[�W�𐶐��B
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

        ret = TRUE; // �����B
    }
    return ret;
}

// ���E�B���h�E�����B
BOOL InputContext::CloseCandidate(BOOL bClearCandInfo /* = TRUE*/) {
    if (HasCandInfo()) { // ����񂪂��邩�H
        if (bClearCandInfo) { // �����N���A���邩�H
            hCandInfo = CandInfo::ReCreate(hCandInfo, NULL); // �����̍č쐬�B
        }
        // ������郁�b�Z�[�W�𐶐��B
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
        return TRUE;
    }
    return FALSE;
}

// �ϊ��B
BOOL InputContext::Convert(BOOL bShift) {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // ���m�蕶���񂪂Ȃ���Εϊ��ł��Ȃ��B
    if (!comp.HasCompStr()) {
        return FALSE;
    }

    // �ϊ��B
    if (cand.HasCandInfo()) { // ����񂪂��邩�H
        if (comp.IsClauseConverted()) { // ���߂��ϊ�����Ă��邩�H
            LogCandList& cand_list = cand.cand_lists[comp.extra.iClause];
            if (bShift) {
                cand_list.MovePrev();
            } else {
                cand_list.MoveNext();
            }
            std::wstring str = cand_list.cand_strs[cand_list.dwSelection];
            comp.SetClauseCompString(comp.extra.iClause, str);
        } else {
            // �����J�����b�Z�[�W�𐶐��B
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);

            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            TheIME.ConvertSingleClause(comp, cand, bRoman);
        }
    } else { // ����񂪂Ȃ��B
        if (Conversion() & IME_CMODE_JAPANESE) {
            if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
                comp.AddChar(L'n', L'n', Conversion());
            }
        }
        // �����J�����b�Z�[�W�𐶐��B
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);

        BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
        TheIME.ConvertMultiClause(comp, cand, bRoman);
    }

    // �������č쐬�B
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
    // ���̕ύX���b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

    return TRUE;
} // InputContext::Convert

// �ϊ����ʂ��쐬�B
void InputContext::MakeResult() {
    // �������B
    CloseCandidate();

    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // ���ʂ��쐬�B
    comp.AssertValid();
    comp.MakeResult();
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_RESULTALL);
    // ���m�蕶����̊m�胁�b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::MakeResult

// ���m�蕶������Ђ炪�Ȃɂ���B
void InputContext::MakeHiragana() {
    // �������B
    CloseCandidate();

    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // ���m�蕶����̍X�V�B
    comp.AssertValid();
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.MakeHiragana(); // �Ђ炪�Ȃɂ���B
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// ���m�蕶������J�^�J�i�ɂ���B
void InputContext::MakeKatakana() {
    // �������B
    CloseCandidate();

    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // ���m�蕶����̍X�V�B
    comp.AssertValid();
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.MakeKatakana(); // �J�^�J�i�ɂ���B
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// ���m�蕶����𔼊p�ɂ���B
void InputContext::MakeHankaku() {
    // �������B
    CloseCandidate();

    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // ���m�蕶����̍X�V�B
    comp.AssertValid();
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.MakeHankaku(); // ���p�ɂ���B
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// ���m�蕶�����S�p�p���ɂ���B
void InputContext::MakeZenEisuu() {
    // �������B
    CloseCandidate();

    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // ���m�蕶������X�V�B
    comp.AssertValid();
    comp.MakeZenEisuu(); // �S�p�p���ɂ���B
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// ���m�蕶����𔼊p�p���ɂ���B
void InputContext::MakeHanEisuu() {
    // �������B
    CloseCandidate();

    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // ���m�蕶������X�V�B
    comp.AssertValid();
    comp.MakeHanEisuu(); // ���p�p���ɂ���B
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// �R�[�h���͂�ϊ�����B
BOOL InputContext::ConvertCode() {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // ���m�蕶���񂪂Ȃ���Εϊ��ł��Ȃ��B
    if (!comp.HasCompStr())
        return FALSE;

    // ����񂪂Ȃ���Ό����J���B
    if (!cand.HasCandInfo()) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
    }

    // �R�[�h�ϊ�����B
    TheIME.ConvertCode(comp, cand);

    // �����̍č쐬�B
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

    return TRUE;
}

// �L�[�{�[�h��Esc�L�[����������B
void InputContext::Escape() {
    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    if (comp.IsClauseConverted()) { // ���݂̕��߂��ϊ��ς݂Ȃ�
        RevertText(); // �Ђ炪�Ȃɖ߂��B
    } else {
        CancelText(); // �ϊ����L�����Z������B
    }
} // InputContext::Escape

// �ϊ����L�����Z������B
void InputContext::CancelText() {
    // �������B
    CloseCandidate();

    // ���m�蕶��������Z�b�g����B
    hCompStr = CompStr::ReCreate(hCompStr, NULL);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // ���m�蕶����̏I�����b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::CancelText

// �ĕϊ�����B
void InputContext::RevertText() {
    // �������B
    CloseCandidate(FALSE);

    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // ���m�蕶���񂪂Ȃ���ΏI���B
    if (!comp.HasCompStr()) {
        return;
    }

    // ���߂��Ђ炪�Ȃɖ߂��B
    comp.AssertValid();
    comp.RevertText();
    comp.AssertValid();

    // ���m�蕶����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::RevertText

// �������폜����B
void InputContext::DeleteChar(BOOL bBackSpace) {
    // ���m�蕶����̘_���f�[�^���擾�B
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // �������폜�B
    comp.AssertValid();
    comp.DeleteChar(bBackSpace, Conversion());
    comp.AssertValid();

    if (comp.comp_str.empty()) { // ���m�蕶���񂪋�ł����
        // �������B
        CloseCandidate();

        // ���m�蕶��������Z�b�g�B
        hCompStr = CompStr::ReCreate(hCompStr, NULL);

        // ���m�蕶����̃��b�Z�[�W�𐶐��B
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
        // ���m�蕶����̏I�����b�Z�[�W�𐶐��B
        TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
    } else {
        // ���m�蕶����̍č쐬�B
        hCompStr = CompStr::ReCreate(hCompStr, &comp);

        // ���m�蕶����̃��b�Z�[�W�𐶐��B
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    }
} // InputContext::DeleteChar

// ���Ɉړ�����B
void InputContext::MoveLeft(BOOL bShift) {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    BOOL bCandChanged = FALSE;
    if (bShift) { // Shift�L�[��������Ă��邩�H
        if (cand.HasCandInfo()) { // ��₪���邩�H
            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            if (!TheIME.StretchClauseLeft(comp, cand, bRoman)) { // ���߂�L�k����B
                return;
            }
            bCandChanged = TRUE; // ��₪�ύX���ꂽ�B
        }
    } else {
        if (comp.MoveLeft()) { // ���Ɉړ��B
            cand.iClause = comp.extra.iClause; // ���߂��擾�B
            bCandChanged = TRUE; // ��₪�ύX���ꂽ�B
        }
        cand.Dump();
    }

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
    // ���̕ύX���b�Z�[�W�𐶐��B
    if (bCandChanged) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveLeft

// �E�Ɉړ�����B
void InputContext::MoveRight(BOOL bShift) {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    BOOL bCandChanged = FALSE;
    if (bShift) { // Shift�L�[��������Ă��邩�H
        if (cand.HasCandInfo()) { // ��₪���邩�H
            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            if (!TheIME.StretchClauseRight(comp, cand, bRoman)) { // ���߂��E�ɐL�k�B
                return;
            }
            bCandChanged = TRUE; // ��₪�ύX���ꂽ�B
        }
    } else {
        // move right
        if (comp.MoveRight()) { // �E�Ɉړ��B
            cand.iClause = comp.extra.iClause; // ���߂��擾�B
            bCandChanged = TRUE; // ��₪�ύX���ꂽ�B
        }
        cand.Dump();
    }

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
    // ���̕ύX���b�Z�[�W�𐶐��B
    if (bCandChanged) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveRight

// ��Ɉړ�����B
void InputContext::MoveUp() {
    if (!HasCandInfo()) return; // ��₪���邩�H

    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // ��O�Ɉړ��B
    cand.MovePrev();
    // ���߂̖��m�蕶����̐ݒ�B
    std::wstring str = cand.GetString();
    comp.SetClauseCompString(cand.iClause, str);

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // ���̕ύX���b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
}

// ���Ɉړ�����B
void InputContext::MoveDown() {
    if (!HasCandInfo()) return; // ��₪���邩�H

    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // ����ֈړ��B
    cand.MoveNext();
    std::wstring str = cand.GetString();
    // ���߂̖��m�蕶����̐ݒ�B
    comp.SetClauseCompString(cand.iClause, str);

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // ���̕ύX���b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
}

// �L�[�{�[�h��Home�L�[�̏����B
void InputContext::MoveHome() {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // �擪�Ɉړ��B
    comp.AssertValid();
    if (cand.HasCandInfo()) { // ��₪���邩�H
        cand.MoveHome(); // �ŏ��̌��Ɉړ��B
    } else {
        comp.MoveHome(); // ���m�蕶����̍ŏ��Ɉړ��B
    }
    comp.AssertValid();

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    if (cand.HasCandInfo()) {
        // ���̕ύX���b�Z�[�W�𐶐��B
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveHome

// �L�[�{�[�h��End�L�[�̏����B
void InputContext::MoveEnd() {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // �����Ɉړ��B
    comp.AssertValid();
    if (cand.HasCandInfo()) { // ��₪���邩�H
        cand.MoveEnd(); // �Ō�̌��Ɉړ��B
    } else {
        comp.MoveEnd(); // ���m�蕶����̍Ō�Ɉړ��B
    }
    comp.AssertValid();

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    if (cand.HasCandInfo() && comp.IsClauseConverted()) {
        // ���̕ύX���b�Z�[�W�𐶐��B
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveEnd

// �L�[�{�[�h��PageUp�L�[�̏����B
void InputContext::PageUp() {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // ��O�̃y�[�W�Ɉړ��B
    cand.PageUp();
    // ���߂̖��m�蕶�����ݒ�B
    std::wstring str = cand.GetString();
    comp.SetClauseCompString(cand.iClause, str);

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m�蕶����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // ���̕ύX���b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::PageUp

// �L�[�{�[�h��PageDown�L�[�̏����B
void InputContext::PageDown() {
    // ���m�蕶����ƌ����̘_���f�[�^���擾�B
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    cand.PageDown(); // ���̈ʒu�ցB

    // ���߂̕�������w��B
    std::wstring str = cand.GetString();
    comp.SetClauseCompString(cand.iClause, str);

    // ���m�蕶����ƌ����̍č쐬�B
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // ���m����ƃJ�[�\���ʒu�̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // �����̃��b�Z�[�W�𐶐��B
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::PageDown

// ���m�蕶����̏����_���v����B
void InputContext::DumpCompStr() {
    CompStr *pCompStr = LockCompStr(); // ���m�蕶��������b�N�B
    if (pCompStr) {
        pCompStr->Dump(); // �_���v�B
        UnlockCompStr(); // ���m�蕶����̃��b�N�������B
    } else {
        DebugPrintA("(no comp str)\n");
    }
} // InputContext::DumpCompStr

// �������_���v����B
void InputContext::DumpCandInfo() {
    CandInfo *pCandInfo = LockCandInfo(); // ���������b�N�B
    if (pCandInfo) {
        pCandInfo->Dump(); // �_���v�B
        UnlockCandInfo(); // �����̃��b�N�������B
    } else {
        DebugPrintA("(no cand info)\n");
    }
} // InputContext::DumpCandInfo

//////////////////////////////////////////////////////////////////////////////
