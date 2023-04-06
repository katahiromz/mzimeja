// input.cpp --- mzimeja input context and related
// 入力コンテキスト関連。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////
// input modes

// 入力モードから変換モードが開かれているかどうかを返す。
BOOL IsInputModeOpen(INPUT_MODE imode)
{
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

// IME変換モードから入力モードを返す。
INPUT_MODE InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion)
{
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

// 入力モードからコマンドIDを返す。
UINT CommandFromInputMode(INPUT_MODE imode)
{
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

// 入力モードを取得する。
INPUT_MODE GetInputMode(HIMC hIMC)
{
    if (hIMC) {
        DWORD dwConversion, dwSentence;
        ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
        BOOL bOpen = ::ImmGetOpenStatus(hIMC);
        return InputModeFromConversionMode(bOpen, dwConversion);
    }
    return IMODE_DISABLED;
}

// 次の入力モード。
INPUT_MODE NextInputMode(INPUT_MODE imode)
{
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

// 入力モードを設定。
void SetInputMode(HIMC hIMC, INPUT_MODE imode)
{
    if (imode == IMODE_DISABLED) {
        return;
    }
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
    switch (imode) {
    case IMODE_FULL_HIRAGANA: // 全角ひらがなモード。
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~IME_CMODE_KATAKANA;
        dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE;
        break;
    case IMODE_FULL_KATAKANA: // 全角カタカナモード。
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion |= IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
        break;
    case IMODE_FULL_ASCII: // 全角英数モード。
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~(IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
        dwConversion |= IME_CMODE_FULLSHAPE;
        break;
    case IMODE_HALF_KANA: // 半角カナモード。
        ImmSetOpenStatus(hIMC, TRUE);
        dwConversion &= ~IME_CMODE_FULLSHAPE;
        dwConversion |= IME_CMODE_JAPANESE | IME_CMODE_KATAKANA;
        break;
    case IMODE_HALF_ASCII: // 半角英数モード。
        ImmSetOpenStatus(hIMC, FALSE);
        dwConversion &= ~(IME_CMODE_FULLSHAPE | IME_CMODE_JAPANESE | IME_CMODE_KATAKANA);
        break;
    case IMODE_DISABLED: // 無効。
        ASSERT(0);
        break;
    }
    // 変換モードをセットする。
    ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
}

// ローマ字入力モードか？
BOOL IsRomanMode(HIMC hIMC)
{
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence); // 変換モードを取得。
    return (dwConversion & IME_CMODE_ROMAN); // ローマ字か？
}

// ローマ字入力モードを設定する。
void SetRomanMode(HIMC hIMC, BOOL bRoman)
{
    DWORD dwConversion, dwSentence;
    ::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence); // 変換モードを取得。
    if (bRoman) {
        dwConversion |= IME_CMODE_ROMAN;
    } else {
        dwConversion &= ~IME_CMODE_ROMAN;
    }
    ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence); // 変換モードを設定。
}

//////////////////////////////////////////////////////////////////////////////
// input context

// 入力コンテキストから入力モードを取得。
INPUT_MODE InputContext::GetInputMode() const
{
    return InputModeFromConversionMode(fOpen, Conversion());
}

// 入力コンテキストからローマ字入力モードを取得。
BOOL InputContext::IsRomanMode() const
{
    return Conversion() & IME_CMODE_ROMAN;
}

// 入力コンテキスト情報のダンプ。
void InputContext::Dump()
{
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

// 入力コンテキストの初期化。
void InputContext::Initialize()
{
    FOOTMARK();
    Dump();

    lfFont.W.lfCharSet = SHIFTJIS_CHARSET;
    lfFont.W.lfFaceName[0] = 0;
    fdwInit |= INIT_LOGFONT;

    fdwConversion = IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE |
                    IME_CMODE_JAPANESE;
    DWORD bIsNonRoman = Config_GetDWORD(L"IsNonRoman", FALSE);
    if (bIsNonRoman) {
        fdwConversion &= ~IME_CMODE_ROMAN;
    } else {
        fdwConversion |= IME_CMODE_ROMAN;
    }
    fdwInit |= INIT_CONVERSION;

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, NULL);
    hCandInfo = CandInfo::ReCreate(hCandInfo, NULL);
}

// 候補情報があるか？
BOOL InputContext::HasCandInfo()
{
    BOOL fRet = FALSE;

    if (ImmGetIMCCSize(hCandInfo) < sizeof(CANDIDATEINFO)) { // サイズは有効か？
        return FALSE;
    }

    CandInfo *lpCandInfo = LockCandInfo(); // 候補情報をロック。
    if (lpCandInfo) {
        fRet = (lpCandInfo->dwCount > 0); // 候補情報の項目があるか？
        UnlockCandInfo(); // 候補情報のロックを解除。
    }
    return fRet;
}

// 未確定文字列があるか？
BOOL InputContext::HasCompStr()
{
    if (ImmGetIMCCSize(hCompStr) <= sizeof(COMPOSITIONSTRING)) { // サイズは有効か？
        return FALSE;
    }

    BOOL ret = FALSE;
    CompStr *pCompStr = LockCompStr(); // 未確定文字列をロック。
    if (pCompStr) {
        ret = (pCompStr->dwCompStrLen > 0); // 未確定文字列の長さがあるか？
        UnlockCompStr(); // 未確定文字列のロックを解除。
    }
    return ret;
}

// 候補情報をロック。
CandInfo *InputContext::LockCandInfo()
{
    CandInfo *info = (CandInfo *)::ImmLockIMCC(hCandInfo);
    ASSERT(info);
    return info;
}

// 候補情報のロックを解除。
void InputContext::UnlockCandInfo()
{
    ::ImmUnlockIMCC(hCandInfo);
}

// 未確定文字列をロック。
CompStr *InputContext::LockCompStr()
{
    CompStr *comp_str = (CompStr *)::ImmLockIMCC(hCompStr);
    ASSERT(comp_str);
    return comp_str;
}

// 未確定文字列のロックを解除。
void InputContext::UnlockCompStr()
{
    ::ImmUnlockIMCC(hCompStr);
}

// メッセージバッファをロック。
LPTRANSMSG InputContext::LockMsgBuf()
{
    LPTRANSMSG lpTransMsg = (LPTRANSMSG) ::ImmLockIMCC(hMsgBuf);
    ASSERT(lpTransMsg);
    return lpTransMsg;
}

// メッセージバッファのロックを解除。
void InputContext::UnlockMsgBuf()
{
    ::ImmUnlockIMCC(hMsgBuf);
}

// メッセージバッファの要素の個数。
DWORD& InputContext::NumMsgBuf()
{
    return dwNumMsgBuf;
}

// メッセージバッファの要素の個数。
const DWORD& InputContext::NumMsgBuf() const
{
    return dwNumMsgBuf;
}

// ガイドラインを作成。
void InputContext::MakeGuideLine(DWORD dwID)
{
    DWORD dwSize = sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(TCHAR)) * 2 * sizeof(TCHAR);
    WCHAR *lpStr;

    hGuideLine = ImmReSizeIMCC(hGuideLine, dwSize); // ガイドラインを再作成。
    LPGUIDELINE lpGuideLine = LockGuideLine(); // ガイドラインをロック。

    // ガイドラインの基本情報を設定。
    lpGuideLine->dwSize = dwSize;
    lpGuideLine->dwLevel = glTable[dwID].dwLevel;
    lpGuideLine->dwIndex = glTable[dwID].dwIndex;
    lpGuideLine->dwStrOffset = sizeof(GUIDELINE);
    lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset);
    LoadString(TheIME.m_hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
    lpGuideLine->dwStrLen = lstrlen(lpStr);

    // ガイドラインの余剰情報を設定する。
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

    // ガイドラインのメッセージを生成。
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_GUIDELINE, 0);

    UnlockGuideLine(); // ガイドラインのロックを解除。
}

// ガイドラインをロック。
LPGUIDELINE InputContext::LockGuideLine()
{
    LPGUIDELINE guideline = (LPGUIDELINE) ::ImmLockIMCC(hGuideLine);
    ASSERT(guideline);
    return guideline;
}

// ガイドラインのロックを解除。
void InputContext::UnlockGuideLine()
{
    ::ImmUnlockIMCC(hGuideLine);
}

// 論理オブジェクトを取得する。
void InputContext::GetLogObjects(LogCompStr& comp, LogCandInfo& cand)
{
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

// 候補を選択する。
BOOL InputContext::SelectCand(UINT uCandIndex)
{
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand); // 論理情報を取得。

    // select a candidate
    if (cand.HasCandInfo() && comp.IsClauseConverted()) {
        if (cand.SelectCand(uCandIndex)) {
            std::wstring str = cand.GetString();
            comp.SetClauseCompString(comp.extra.iClause, str);

            // 未確定文字列と候補情報の再作成。
            hCompStr = CompStr::ReCreate(hCompStr, &comp);
            hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

            // 未確定文字列のメッセージを生成。
            TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL);

            // update candidate
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
            return TRUE;
        }
    }
    return FALSE;
} // InputContext::SelectCand

// 文字を追加。
void InputContext::AddChar(WCHAR chTyped, WCHAR chTranslated)
{
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr(); // 未確定文字列をロック。
    if (lpCompStr) {
        lpCompStr->GetLog(comp); // 未確定文字列の論理データを取得。
        UnlockCompStr(); // 未確定文字列のロックを解除。
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

    // １文字を追加。
    comp.AssertValid();
    if ((Conversion() & IME_CMODE_JAPANESE) && !::IsCharAlphaW(chTyped)) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.AddChar(chTyped, chTranslated, Conversion());
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    if (bHasResult) { // 結果がある？
        LPARAM lParam = GCS_COMPALL | GCS_RESULTALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    } else { // 結果がない？
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    }
} // InputContext::AddChar

// 候補ウィンドウを開く。
BOOL InputContext::OpenCandidate()
{
    BOOL ret = FALSE;
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr(); // 未確定文字列をロック。
    if (!lpCompStr)
        return ret;

    lpCompStr->GetLog(comp); // 未確定文字列の論理データを取得。
    UnlockCompStr(); // 未確定文字列のロックを解除。

    LogCandInfo cand;
    CandInfo *lpCandInfo = LockCandInfo(); // 候補情報をロック。
    if (lpCandInfo) {
        lpCandInfo->GetLog(cand); // 候補情報の論理データを取得。
        UnlockCandInfo(); // 候補情報のロックを解除。

        // 候補を開くメッセージを生成。
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
        // 候補情報を再作成。
        hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
        // 候補情報の変更メッセージを生成。
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

        ret = TRUE; // 成功。
    }
    return ret;
}

// 候補ウィンドウを閉じる。
BOOL InputContext::CloseCandidate(BOOL bClearCandInfo /* = TRUE*/)
{
    if (HasCandInfo()) { // 候補情報があるか？
        if (bClearCandInfo) { // 候補をクリアするか？
            hCandInfo = CandInfo::ReCreate(hCandInfo, NULL); // 候補情報の再作成。
        }
        // 候補を閉じるメッセージを生成。
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
        return TRUE;
    }
    return FALSE;
}

// 変換。
BOOL InputContext::Convert(BOOL bShift)
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // 未確定文字列がなければ変換できない。
    if (!comp.HasCompStr()) {
        return FALSE;
    }

    // 変換。
    if (cand.HasCandInfo()) { // 候補情報があるか？
        if (comp.IsClauseConverted()) { // 文節が変換されているか？
            LogCandList& cand_list = cand.cand_lists[comp.extra.iClause];
            if (bShift) {
                cand_list.MovePrev();
            } else {
                cand_list.MoveNext();
            }
            std::wstring str = cand_list.cand_strs[cand_list.dwSelection];
            comp.SetClauseCompString(comp.extra.iClause, str);
        } else {
            // 候補を開くメッセージを生成。
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);

            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            TheIME.ConvertSingleClause(comp, cand, bRoman);
        }
    } else { // 候補情報がない。
        if (Conversion() & IME_CMODE_JAPANESE) {
            if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
                comp.AddChar(L'n', L'n', Conversion());
            }
        }
        // 候補を開くメッセージを生成。
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);

        BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
        TheIME.ConvertMultiClause(comp, cand, bRoman);
    }

    // 候補情報を再作成。
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
    // 候補の変更メッセージを生成。
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

    return TRUE;
} // InputContext::Convert

// 変換結果を作成。
void InputContext::MakeResult()
{
    // 候補を閉じる。
    CloseCandidate();

    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 結果を作成。
    comp.AssertValid();
    comp.MakeResult();
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_RESULTALL);
    // 未確定文字列の確定メッセージを生成。
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::MakeResult

// 未確定文字列をひらがなにする。
void InputContext::MakeHiragana()
{
    // 候補を閉じる。
    CloseCandidate();

    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 未確定文字列の更新。
    comp.AssertValid();
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.MakeHiragana(); // ひらがなにする。
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// 未確定文字列をカタカナにする。
void InputContext::MakeKatakana()
{
    // 候補を閉じる。
    CloseCandidate();

    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 未確定文字列の更新。
    comp.AssertValid();
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.MakeKatakana(); // カタカナにする。
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// 未確定文字列を半角にする。
void InputContext::MakeHankaku()
{
    // 候補を閉じる。
    CloseCandidate();

    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 未確定文字列の更新。
    comp.AssertValid();
    if (Conversion() & IME_CMODE_JAPANESE) {
        if (IsRomanMode() && comp.PrevCharInClause() == L'n') {
            comp.AddChar(L'n', L'n', Conversion());
        }
    }
    comp.AssertValid();
    comp.MakeHankaku(); // 半角にする。
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// 未確定文字列を全角英数にする。
void InputContext::MakeZenEisuu()
{
    // 候補を閉じる。
    CloseCandidate();

    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 未確定文字列を更新。
    comp.AssertValid();
    comp.MakeZenEisuu(); // 全角英数にする。
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// 未確定文字列を半角英数にする。
void InputContext::MakeHanEisuu()
{
    // 候補を閉じる。
    CloseCandidate();

    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 未確定文字列を更新。
    comp.AssertValid();
    comp.MakeHanEisuu(); // 半角英数にする。
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
}

// コード入力を変換する。
BOOL InputContext::ConvertCode()
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // 未確定文字列がなければ変換できない。
    if (!comp.HasCompStr())
        return FALSE;

    // 候補情報がなければ候補を開く。
    if (!cand.HasCandInfo()) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
    }

    // コード変換する。
    TheIME.ConvertCode(comp, cand);

    // 候補情報の再作成。
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

    return TRUE;
}

// キーボードのEscキーを処理する。
void InputContext::Escape()
{
    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    if (comp.IsClauseConverted()) { // 現在の文節が変換済みなら
        RevertText(); // ひらがなに戻す。
    } else {
        CancelText(); // 変換をキャンセルする。
    }
} // InputContext::Escape

// 変換をキャンセルする。
void InputContext::CancelText()
{
    // 候補を閉じる。
    CloseCandidate();

    // 未確定文字列をリセットする。
    hCompStr = CompStr::ReCreate(hCompStr, NULL);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // 未確定文字列の終了メッセージを生成。
    TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
} // InputContext::CancelText

// 再変換する。
void InputContext::RevertText()
{
    // 候補を閉じる。
    CloseCandidate(FALSE);

    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 未確定文字列がなければ終了。
    if (!comp.HasCompStr()) {
        return;
    }

    // 文節をひらがなに戻す。
    comp.AssertValid();
    comp.RevertText();
    comp.AssertValid();

    // 未確定文字列の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);

    // 未確定文字列のメッセージを生成。
    LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
} // InputContext::RevertText

// 文字を削除する。
void InputContext::DeleteChar(BOOL bBackSpace)
{
    // 未確定文字列の論理データを取得。
    LogCompStr comp;
    CompStr *lpCompStr = LockCompStr();
    if (lpCompStr) {
        lpCompStr->GetLog(comp);
        UnlockCompStr();
    }

    // 文字を削除。
    comp.AssertValid();
    comp.DeleteChar(bBackSpace, Conversion());
    comp.AssertValid();

    if (comp.comp_str.empty()) { // 未確定文字列が空であれば
        // 候補を閉じる。
        CloseCandidate();

        // 未確定文字列をリセット。
        hCompStr = CompStr::ReCreate(hCompStr, NULL);

        // 未確定文字列のメッセージを生成。
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
        // 未確定文字列の終了メッセージを生成。
        TheIME.GenerateMessage(WM_IME_ENDCOMPOSITION);
    } else {
        // 未確定文字列の再作成。
        hCompStr = CompStr::ReCreate(hCompStr, &comp);

        // 未確定文字列のメッセージを生成。
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS;
        TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    }
} // InputContext::DeleteChar

// 左に移動する。
void InputContext::MoveLeft(BOOL bShift)
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    BOOL bCandChanged = FALSE;
    if (bShift) { // Shiftキーが押されているか？
        if (cand.HasCandInfo()) { // 候補があるか？
            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            if (!TheIME.StretchClauseLeft(comp, cand, bRoman)) { // 文節を伸縮する。
                return;
            }
            bCandChanged = TRUE; // 候補が変更された。
        }
    } else {
        if (comp.MoveLeft()) { // 左に移動。
            cand.iClause = comp.extra.iClause; // 文節を取得。
            bCandChanged = TRUE; // 候補が変更された。
        }
        cand.Dump();
    }

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
    // 候補の変更メッセージを生成。
    if (bCandChanged) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveLeft

// 右に移動する。
void InputContext::MoveRight(BOOL bShift)
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    BOOL bCandChanged = FALSE;
    if (bShift) { // Shiftキーが押されているか？
        if (cand.HasCandInfo()) { // 候補があるか？
            BOOL bRoman = (Conversion() & IME_CMODE_ROMAN);
            if (!TheIME.StretchClauseRight(comp, cand, bRoman)) { // 文節を右に伸縮。
                return;
            }
            bCandChanged = TRUE; // 候補が変更された。
        }
    } else {
        // move right
        if (comp.MoveRight()) { // 右に移動。
            cand.iClause = comp.extra.iClause; // 文節を取得。
            bCandChanged = TRUE; // 候補が変更された。
        }
        cand.Dump();
    }

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_CURSORPOS);
    // 候補の変更メッセージを生成。
    if (bCandChanged) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveRight

// 上に移動する。
void InputContext::MoveUp()
{
    if (!HasCandInfo()) return; // 候補があるか？

    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // 一つ前に移動。
    cand.MovePrev();
    // 文節の未確定文字列の設定。
    std::wstring str = cand.GetString();
    comp.SetClauseCompString(cand.iClause, str);

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // 候補の変更メッセージを生成。
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
}

// 下に移動する。
void InputContext::MoveDown()
{
    if (!HasCandInfo()) return; // 候補があるか？

    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // 一つ次へ移動。
    cand.MoveNext();
    std::wstring str = cand.GetString();
    // 文節の未確定文字列の設定。
    comp.SetClauseCompString(cand.iClause, str);

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // 候補の変更メッセージを生成。
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
}

// キーボードのHomeキーの処理。
void InputContext::MoveHome()
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // 先頭に移動。
    comp.AssertValid();
    if (cand.HasCandInfo()) { // 候補があるか？
        cand.MoveHome(); // 最初の候補に移動。
    } else {
        comp.MoveHome(); // 未確定文字列の最初に移動。
    }
    comp.AssertValid();

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    if (cand.HasCandInfo()) {
        // 候補の変更メッセージを生成。
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveHome

// キーボードのEndキーの処理。
void InputContext::MoveEnd()
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // 末尾に移動。
    comp.AssertValid();
    if (cand.HasCandInfo()) { // 候補があるか？
        cand.MoveEnd(); // 最後の候補に移動。
    } else {
        comp.MoveEnd(); // 未確定文字列の最後に移動。
    }
    comp.AssertValid();

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    if (cand.HasCandInfo() && comp.IsClauseConverted()) {
        // 候補の変更メッセージを生成。
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
    }
} // InputContext::MoveEnd

// キーボードのPageUpキーの処理。
void InputContext::PageUp()
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    // 一つ前のページに移動。
    cand.PageUp();
    // 文節の未確定文字列を設定。
    std::wstring str = cand.GetString();
    comp.SetClauseCompString(cand.iClause, str);

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定文字列のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // 候補の変更メッセージを生成。
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::PageUp

// キーボードのPageDownキーの処理。
void InputContext::PageDown()
{
    // 未確定文字列と候補情報の論理データを取得。
    LogCompStr comp;
    LogCandInfo cand;
    GetLogObjects(comp, cand);

    cand.PageDown(); // 次の位置へ。

    // 文節の文字列を指定。
    std::wstring str = cand.GetString();
    comp.SetClauseCompString(cand.iClause, str);

    // 未確定文字列と候補情報の再作成。
    hCompStr = CompStr::ReCreate(hCompStr, &comp);
    hCandInfo = CandInfo::ReCreate(hCandInfo, &cand);

    // 未確定情報とカーソル位置のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_COMPALL | GCS_CURSORPOS);
    // 候補情報のメッセージを生成。
    TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
} // InputContext::PageDown

// 未確定文字列の情報をダンプする。
void InputContext::DumpCompStr()
{
    CompStr *pCompStr = LockCompStr(); // 未確定文字列をロック。
    if (pCompStr) {
        pCompStr->Dump(); // ダンプ。
        UnlockCompStr(); // 未確定文字列のロックを解除。
    } else {
        DebugPrintA("(no comp str)\n");
    }
} // InputContext::DumpCompStr

// 候補情報をダンプする。
void InputContext::DumpCandInfo()
{
    CandInfo *pCandInfo = LockCandInfo(); // 候補情報をロック。
    if (pCandInfo) {
        pCandInfo->Dump(); // ダンプ。
        UnlockCandInfo(); // 候補情報のロックを解除。
    } else {
        DebugPrintA("(no cand info)\n");
    }
} // InputContext::DumpCandInfo

//////////////////////////////////////////////////////////////////////////////
