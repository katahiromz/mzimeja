// input.h --- mzimeja input context and related
//////////////////////////////////////////////////////////////////////////////
// (Japanese, UTF-8)

#ifndef INPUT_H_
#define INPUT_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// 入力モード。

enum INPUT_MODE {
    IMODE_FULL_HIRAGANA,    // 全角ひらがな。
    IMODE_FULL_KATAKANA,    // 全角カタカナ。
    IMODE_FULL_ASCII,       // 全角英数。
    IMODE_HALF_KANA,        // 半角カナ。
    IMODE_HALF_ASCII,       // 半角英数。
    IMODE_DISABLED          // IMEが無効。
};

BOOL        IsInputModeOpen(INPUT_MODE imode);
INPUT_MODE  InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion);
INPUT_MODE  NextInputMode(INPUT_MODE imode);
INPUT_MODE  GetInputMode(HIMC hIMC);
void        SetInputMode(HIMC hIMC, INPUT_MODE imode);
BOOL        IsRomanMode(HIMC hIMC);
void        SetRomanMode(HIMC hIMC, BOOL bRoman);
UINT        CommandFromInputMode(INPUT_MODE imode);

//////////////////////////////////////////////////////////////////////////////
// 未確定文字列。

struct LogCandInfo;

// 未確定文字列の余剰情報の論理データ。
struct LogCompStrExtra {
    // 選択中の文節のインデックス。
    // assert(iClause <= GetClauseCount());
    DWORD iClause;
    // ひらがな文節文字列。
    std::vector<std::wstring>   hiragana_clauses;
    // 入力文節文字列。
    std::vector<std::wstring>   typing_clauses;
    // 未確定文字列の文節文字列。
    std::vector<std::wstring>   comp_str_clauses;

    LogCompStrExtra() {
        clear();
    }
    void clear();
    DWORD GetTotalSize() const;
}; // struct LogCompStrExtra

// 未確定文字列の余剰情報の物理データ。
struct COMPSTREXTRA {
    DWORD dwSignature;              // 確認のため。
    DWORD iClause;                  // 現在の文節のインデックス。
    DWORD dwHiraganaClauseCount;    // ひらがな文節の個数。
    DWORD dwHiraganaClauseOffset;   // ひらがな文節のオフセット。
    DWORD dwTypingClauseCount;      // 入力文節の個数。
    DWORD dwTypingClauseOffset;     // 入力文節のオフセット。

    BYTE *GetBytes() { return (LPBYTE) this; }  // バイト列の取得。
    WCHAR *GetHiraganaClauses(DWORD& dwCount);  // ひらがな文節の取得。
    WCHAR *GetTypingClauses(DWORD& dwCount);    // 入力文節の取得。

    void GetLog(LogCompStrExtra& log);          // 物理データから論理データへ。
    DWORD Store(const LogCompStrExtra *log);    // 論理データから物理データへ。
}; // struct COMPSTREXTRA

// 未確定文字列の論理データ。
struct LogCompStr {
    DWORD dwCursorPos;                      // 現在位置の文字のインデックス。
    DWORD dwDeltaStart;                     // 最初に変更された文字のインデックス。
    std::vector<BYTE>   comp_read_attr;     // we don't use this member: comp_read_attr
    std::vector<DWORD>  comp_read_clause;   // we don't use this member: comp_read_clause
    std::wstring comp_read_str;             // 読み文字列。
    std::vector<BYTE>   comp_attr;          // 文字属性。
    std::vector<DWORD>  comp_clause;        // 未確定文字列の文節インデックスから未確定文字列の文字インデックスへの写像。
    std::wstring comp_str;                  // 未確定文字列。
    std::vector<DWORD>  result_read_clause; // 結果読み文節インデックスから結果文字インデックスへの写像。
    std::wstring result_read_str;           // 結果読み文字列。
    std::vector<DWORD>  result_clause;      // 結果文節インデックスから結果文字インデックスへの写像。
    std::wstring result_str;                // 結果文字列。
    LogCompStrExtra extra;                  // 余剰情報。

    LogCompStr() {
        clear();
    }

    // クリア。
    void clear();
    void clear_read();
    void clear_comp();
    void clear_result();
    void clear_extra() { extra.clear(); }

    void fix(); // 補正。
    DWORD GetTotalSize() const; // 物理データの合計サイズ。

    BOOL IsBeingConverted();
    DWORD GetClauseCount() const;
    BOOL CompCharInClause(DWORD iCompChar, DWORD iClause,
                          BOOL bExcludeEnd = FALSE) const;
    BYTE GetClauseAttr(DWORD dwClauseIndex) const;
    void SetClauseAttr(DWORD dwClauseIndex, BYTE attr);
    BOOL IsClauseConverted() const;
    BOOL IsClauseConverted(DWORD dwClauseIndex) const;
    BOOL HasClauseSelected() const;
    std::wstring GetClauseCompString(DWORD dwClauseIndex) const;
    WCHAR PrevCharInClause() const;

    BYTE GetCompCharAttr(DWORD ich) const;
    DWORD GetCompCharCount() const;

    DWORD ClauseToCompChar(DWORD dwClauseIndex) const;
    DWORD CompCharToClause(DWORD iCompChar) const;
    BOOL HasCompStr() const;

    // 未確定文字列の文字種変換。
    void MakeHiragana();
    void MakeKatakana();
    void MakeHankaku();
    void MakeZenEisuu();
    void MakeHanEisuu();

    void AddCharToEnd(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
    void InsertChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
    void AddDakuonChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);

    void AddChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
    void DeleteChar(BOOL bBackSpace /* = FALSE*/, DWORD dwConv);
    void RevertText();
    void MakeResult();

    BOOL MoveLeft();
    BOOL MoveRight();
    BOOL MoveHome();
    BOOL MoveEnd();

    DWORD GetClauseCompStrLen(DWORD dwClauseIndex) const;
    void SetClauseCompString(DWORD iClause, std::wstring& str);
    void SetClauseCompHiragana(DWORD iClause, std::wstring& str);
    void SetClauseCompHiragana(DWORD iClause, std::wstring& str, BOOL bRoman);
    void UpdateExtraClause(DWORD iClause, DWORD dwConversion);
    void UpdateFromExtra(BOOL bRoman);

    // デバッグ用。
    void AssertValid();
    void Dump();

protected:
    void MergeAt(std::vector<std::wstring>& strs, DWORD istr);
    void UpdateCompStr();
}; // struct LogCompStr

inline void SetClause(DWORD *lpdw, DWORD num) {
    *lpdw = 0;
    *(lpdw + 1) = num;
}

// 未確定文字列の物理データ。
struct CompStr : public COMPOSITIONSTRING {
    static HIMCC ReCreate(HIMCC hCompStr, const LogCompStr *log = NULL);

    void GetLog(LogCompStr& log);
    DWORD Store(const LogCompStr *log);

    BYTE *GetBytes() {
        return (LPBYTE) this;
    }

    BYTE *GetCompReadAttr() {
        return GetBytes() + dwCompReadAttrOffset;
    }
    DWORD *GetCompReadClause() {
        return (DWORD *)(GetBytes() + dwCompReadClauseOffset);
    }
    WCHAR *GetCompReadStr() {
        return (LPTSTR)(GetBytes() + dwCompReadStrOffset);
    }
    BYTE *GetCompAttr() {
        return GetBytes() + dwCompAttrOffset;
    }
    DWORD *GetCompClause() {
        return (DWORD *)(GetBytes() + dwCompClauseOffset);
    }
    WCHAR *GetCompStr() {
        return (LPTSTR)(GetBytes() + dwCompStrOffset);
    }
    DWORD *GetResultReadClause() {
        return (DWORD *)(GetBytes() + dwResultReadClauseOffset);
    }
    WCHAR *GetResultReadStr() {
        return (LPTSTR)(GetBytes() + dwResultReadStrOffset);
    }
    DWORD *GetResultClause() {
        return (DWORD *)(GetBytes() + dwResultClauseOffset);
    }
    WCHAR *GetResultStr() {
        return (LPTSTR)(GetBytes() + dwResultStrOffset);
    }

    // extension
    COMPSTREXTRA *GetExtra();

    void Dump();

private:
    CompStr();
    CompStr(const CompStr&);
    CompStr& operator=(const CompStr&);
}; // struct CompStr

//////////////////////////////////////////////////////////////////////////////
// 候補情報。

// private data of CANDIDATEINFO
struct CANDINFOEXTRA {
    DWORD dwSignature; // must be 0xDEADFACE
    DWORD iClause; // index of selected clause
};

// 候補リストの論理データ。
struct LogCandList {
    DWORD dwStyle;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    std::vector<std::wstring> cand_strs;

    LogCandList() {
        clear();
    }
    void clear();
    DWORD GetTotalSize() const;

    void MoveNext();
    void MovePrev();
    void PageUp();
    void PageDown();
    void MoveHome();
    void MoveEnd();

    DWORD GetCandCount() const;
    std::wstring GetString() const;
    std::wstring GetString(DWORD iCand) const;
};

// 候補情報の論理データ。
struct LogCandInfo {
    std::vector<LogCandList>  cand_lists;
    DWORD iClause;

    LogCandInfo() {
        clear();
    }
    void clear();
    DWORD GetTotalSize() const;
    DWORD GetClauseCount() const;
    BOOL HasCandInfo() const;

    void MoveNext();
    void MovePrev();
    void MoveHome();
    void MoveEnd();
    void PageUp();
    void PageDown();
    BOOL SelectCand(UINT uCandIndex);

    std::wstring GetString() const;
    std::wstring GetString(DWORD iCand) const;

    void Dump();
}; // struct LogCandInfo

// 候補リストの物理データ。
struct CandList : public CANDIDATELIST {
    BYTE *GetBytes()                { return (BYTE *)this; }
    WCHAR *GetCandString(DWORD i)   { return LPTSTR(GetBytes() + dwOffset[i]); }
    WCHAR *GetCurString()           { return GetCandString(dwSelection); }
    DWORD  GetPageEnd() const;
    void GetLog(LogCandList& log);
    DWORD Store(const LogCandList *log);

private:
    // never be implemented
    CandList();
    CandList(const CandList&);
    CandList& operator=(const CandList&);
}; // struct CandList

// 候補情報の物理データ。
struct CandInfo : public CANDIDATEINFO {
    static HIMCC ReCreate(HIMCC hCandInfo, const LogCandInfo *log = NULL);
    void GetLog(LogCandInfo& log);
    DWORD Store(const LogCandInfo *log);

    BYTE *GetBytes() { return (BYTE *)this; }
    CandList *GetList(DWORD i);
    CANDINFOEXTRA *GetExtra();

    void Dump();

private:
    // never be implemented
    CandInfo();
    CandInfo(const CandInfo&);
    CandInfo& operator=(const CandInfo&);
}; // struct CandInfo

//////////////////////////////////////////////////////////////////////////////
// 入力コンテキスト。

struct InputContext : public INPUTCONTEXT {
    void Initialize();

    // 状態を取得・設定する。
    BOOL& IsOpen()                  { return fOpen; }
    const BOOL& IsOpen() const      { return fOpen; }
    DWORD& Conversion()             { return fdwConversion; }
    const DWORD& Conversion() const { return fdwConversion; }
    DWORD& Sentence()               { return fdwSentence; }
    const DWORD& Sentence() const   { return fdwSentence; }
    BOOL HasStatusWndPos() const    { return (fdwInit & INIT_STATUSWNDPOS); }
    BOOL HasConversion() const      { return (fdwInit & INIT_CONVERSION); }
    BOOL HasSentence() const        { return (fdwInit & INIT_SENTENCE); }
    BOOL HasLogFont() const         { return (fdwInit & INIT_LOGFONT); }
    BOOL HasCompForm() const        { return (fdwInit & INIT_COMPFORM); }
    BOOL HasSoftKbdPos() const      { return (fdwInit & INIT_SOFTKBDPOS); }
    INPUT_MODE GetInputMode() const;
    BOOL IsRomanMode() const;

    // 候補情報。
    BOOL HasCandInfo();
    CandInfo *LockCandInfo();
    void UnlockCandInfo();

    // 未確定文字列。
    BOOL HasCompStr();
    CompStr *LockCompStr();
    void UnlockCompStr();

    // メッセージバッファ。
    LPTRANSMSG LockMsgBuf();
    void UnlockMsgBuf();
    DWORD& NumMsgBuf();
    const DWORD& NumMsgBuf() const;

    // ガイドライン。
    void MakeGuideLine(DWORD dwID);
    LPGUIDELINE LockGuideLine();
    void UnlockGuideLine();

    // actions
    BOOL SelectCand(UINT uCandIndex);
    void AddChar(WCHAR chTyped, WCHAR chTranslated);
    void Escape();
    void CancelText();
    void RevertText();
    void DeleteChar(BOOL bBackSpace = FALSE);
    BOOL Convert(BOOL bShift);
    BOOL OpenCandidate();
    BOOL CloseCandidate(BOOL bClearCandInfo = TRUE);

    void MoveLeft(BOOL bShift);
    void MoveRight(BOOL bShift);
    void MoveUp();
    void MoveDown();
    void MoveHome();
    void MoveEnd();
    void PageUp();
    void PageDown();

    void MakeResult();
    void MakeHiragana();
    void MakeKatakana();
    void MakeHankaku();
    void MakeZenEisuu();
    void MakeHanEisuu();
    BOOL ConvertCode();

    // 論理データを取得。
    void GetLogObjects(LogCompStr& comp, LogCandInfo& cand);

    // デバッグ用。
    void DumpCandInfo();
    void DumpCompStr();
    void Dump();

private:
    // never be implemented
    InputContext();
    InputContext(const InputContext&);
    InputContext& operator=(const InputContext&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_H_
