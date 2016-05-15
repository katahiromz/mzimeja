// input.h --- mzimeja input context and related
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_H_
#define INPUT_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// input modes

enum INPUT_MODE {
  IMODE_ZEN_HIRAGANA,   // zenkaku hiragana
  IMODE_ZEN_KATAKANA,   // zenkaku katakana
  IMODE_ZEN_EISUU,      // zenkaku alphanumeric
  IMODE_HAN_KANA,       // hankaku katakana
  IMODE_HAN_EISUU,      // hankaku alphanumeric
  IMODE_DISABLED        // the IME is disabled
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
// composition info

// logical comp info extra
struct LogCompStrExtra {
  // selected composition clause index
  // assert(iClause <= GetClauseCount());
  DWORD                       iClause;
  // hiragana clause strings
  std::vector<std::wstring>   hiragana_clauses;
  // typing clause strings
  std::vector<std::wstring>   typing_clauses;
  // composition clause strings
  std::vector<std::wstring>   comp_str_clauses;

  LogCompStrExtra() { clear(); }
  void clear();
  DWORD GetTotalSize() const;
}; // struct LogCompStrExtra

// physical comp info extar
struct COMPSTREXTRA {
  DWORD   dwSignature;
  DWORD   iClause;
  DWORD   dwHiraganaClauseCount;
  DWORD   dwHiraganaClauseOffset;
  DWORD   dwTypingClauseCount;
  DWORD   dwTypingClauseOffset;

  LPBYTE GetBytes() { return (LPBYTE)this; }

  LPWSTR GetHiraganaClauses(DWORD& dwCount);
  LPWSTR GetTypingClauses(DWORD& dwCount);

  // physical to logical
  void GetLog(LogCompStrExtra& log);
  // physical from logical
  DWORD Store(const LogCompStrExtra *log);
}; // struct COMPSTREXTRA

// logical composition info
struct LogCompStr {
  // composition character index of the current position
  DWORD               dwCursorPos;
  // index of first changed composition character
  DWORD               dwDeltaStart;
  // we don't use this member: comp_read_attr
  std::vector<BYTE>   comp_read_attr;
  // we don't use this member: comp_read_clause
  std::vector<DWORD>  comp_read_clause;
  // composition reading string
  std::wstring        comp_read_str;
  // composition attributes (per composition character)
  std::vector<BYTE>   comp_attr;
  // mapping from composition clause index to composition character index
  std::vector<DWORD>  comp_clause;
  // composition string
  std::wstring        comp_str;
  // mapping from result reading clause index to result reading character index
  std::vector<DWORD>  result_read_clause;
  // result reading string
  std::wstring        result_read_str;
  // mapping from result clause index to result character index
  std::vector<DWORD>  result_clause;
  // result string
  std::wstring        result_str;
  // extra information
  LogCompStrExtra     extra;

  LogCompStr() { clear(); }
  void clear();
  void clear_read();
  void clear_comp();
  void clear_result();
  void clear_extra() { extra.clear(); }
  DWORD GetTotalSize() const;

  BOOL IsBeingConverted();
  DWORD GetClauseCount() const;
  BOOL CompCharInClause(DWORD iCompChar, DWORD iClause,
                        BOOL bExcludeEnd = FALSE) const;
  BYTE GetClauseAttr(DWORD dwClauseIndex) const;
  void SetClauseAttr(DWORD dwClauseIndex, BYTE attr);
  BOOL IsClauseConverted(DWORD dwClauseIndex) const;
  BOOL HasClauseSelected() const;
  std::wstring GetClauseCompString(DWORD dwClauseIndex) const;
  WCHAR PrevCharInClause() const;

  BYTE GetCompCharAttr(DWORD ich) const;
  DWORD GetCompCharCount() const;

  DWORD ClauseToCompChar(DWORD dwClauseIndex) const;
  DWORD CompCharToClause(DWORD iCompChar) const;

  // conversion of composition
  void MakeHiragana();
  void MakeKatakana();
  void MakeHankaku();
  void MakeZenEisuu();
  void MakeHanEisuu();

  void AddCharToEnd(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
  void InsertChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
  void AddDakuonChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);

  void AddChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv);
  void DeleteChar(BOOL bBackSpace/* = FALSE*/, DWORD dwConv);
  void RevertText();
  void MakeResult();

  void MoveLeft(BOOL bShift);
  void MoveRight(BOOL bShift);

  DWORD GetClauseCompStrLen(DWORD dwClauseIndex) const;
  void SetClauseCompString(DWORD iClause, std::wstring& str);

  // for debugging
  void AssertValid();
  void Dump();

protected:
  void MergeAt(std::vector<std::wstring>& strs, DWORD istr);
  void UpdateExtraClause(DWORD iClause, DWORD dwConversion);
  void UpdateCompStr();
}; // struct LogCompStr

inline void SetClause(LPDWORD lpdw, DWORD num) {
  *lpdw = 0;
  *(lpdw + 1) = num;
}

// physical composition info
struct CompStr : public COMPOSITIONSTRING {
  static HIMCC ReCreate(HIMCC hCompStr, const LogCompStr *log = NULL);

  void GetLog(LogCompStr& log);

  LPBYTE GetBytes() { return (LPBYTE)this; }

  LPBYTE GetCompReadAttr() {
    return GetBytes() + dwCompReadAttrOffset;
  }
  LPDWORD GetCompReadClause() {
    return (LPDWORD)(GetBytes() + dwCompReadClauseOffset);
  }
  LPTSTR GetCompReadStr() {
    return (LPTSTR)(GetBytes() + dwCompReadStrOffset);
  }
  LPBYTE GetCompAttr() {
    return GetBytes() + dwCompAttrOffset;
  }
  LPDWORD GetCompClause() {
    return (LPDWORD)(GetBytes() + dwCompClauseOffset);
  }
  LPTSTR GetCompStr() {
    return (LPTSTR)(GetBytes() + dwCompStrOffset);
  }
  LPDWORD GetResultReadClause() {
    return (LPDWORD)(GetBytes() + dwResultReadClauseOffset);
  }
  LPTSTR GetResultReadStr() {
    return (LPTSTR)(GetBytes() + dwResultReadStrOffset);
  }
  LPDWORD GetResultClause() {
    return (LPDWORD)(GetBytes() + dwResultClauseOffset);
  }
  LPTSTR GetResultStr() {
    return (LPTSTR)(GetBytes() + dwResultStrOffset);
  }

  // extension
  COMPSTREXTRA *GetExtra() {
    if (dwPrivateSize) return (COMPSTREXTRA *)(GetBytes() + dwPrivateOffset);
    return NULL;
  }

  void Dump();

private:
  CompStr();
  CompStr(const CompStr&);
  CompStr& operator=(const CompStr&);
}; // struct CompStr

//////////////////////////////////////////////////////////////////////////////
// candidate info

// logical candidate list
struct LogCandList {
  DWORD   dwStyle;
  DWORD   dwSelection;
  DWORD   dwPageStart;
  DWORD   dwPageSize;
  std::vector<std::wstring> cand_strs;

  LogCandList() { clear(); }
  void clear();
  DWORD GetTotalSize() const;
};

// logical candidate info
struct LogCandInfo {
  std::vector<LogCandList>  cand_lists;

  LogCandInfo() { clear(); }
  void clear();
  DWORD GetTotalSize() const;
}; // struct LogCandInfo

// physical candidate list
struct CandList : public CANDIDATELIST {
  LPBYTE GetBytes()             { return (LPBYTE)this; }
  LPTSTR GetCandString(DWORD i) { return LPTSTR(GetBytes() + dwOffset[i]); }
  LPTSTR GetCurString()         { return GetCandString(dwSelection); }
  DWORD  GetPageEnd() const     { return dwPageStart + dwPageSize; }
  void GetLog(LogCandList& log);
  DWORD Store(const LogCandList *log);

private:
  // never be implemented
  CandList();
  CandList(const CandList&);
  CandList& operator=(const CandList&);
}; // struct CandList

// logical candidate info
struct CandInfo : public CANDIDATEINFO {
  static HIMCC ReCreate(HIMCC hCandInfo, const LogCandInfo *log = NULL);
  void GetLog(LogCandInfo& log);
  DWORD Store(const LogCandInfo *log);

  LPBYTE GetBytes()          { return (LPBYTE)this; }
  CandList *GetList(DWORD i) { return (CandList *)(GetBytes() + dwOffset[i]); }

  void Dump();

private:
  // never be implemented
  CandInfo();
  CandInfo(const CandInfo&);
  CandInfo& operator=(const CandInfo&);
}; // struct CandInfo

//////////////////////////////////////////////////////////////////////////////
// input context

struct InputContext : public INPUTCONTEXT {
  void Initialize();

  // get status information
        BOOL& IsOpen()       { return fOpen; }
  const BOOL& IsOpen() const { return fOpen; }
        DWORD& Conversion()       { return fdwConversion; }
  const DWORD& Conversion() const { return fdwConversion; }
        DWORD& Sentence()       { return fdwSentence; }
  const DWORD& Sentence() const { return fdwSentence; }
  INPUT_MODE GetInputMode() const;
  BOOL IsRomanMode() const;
  BOOL HasStatusWndPos() const  { return (fdwInit & INIT_STATUSWNDPOS); }
  BOOL HasConversion() const    { return (fdwInit & INIT_CONVERSION); }
  BOOL HasSentence() const      { return (fdwInit & INIT_SENTENCE); }
  BOOL HasLogFont() const       { return (fdwInit & INIT_LOGFONT); }
  BOOL HasCompForm() const      { return (fdwInit & INIT_COMPFORM); }
  BOOL HasSoftKbdPos() const    { return (fdwInit & INIT_SOFTKBDPOS); }

  // candidate info
  BOOL HasCandInfo();
  CandInfo *LockCandInfo();
  void UnlockCandInfo();

  // composition info
  BOOL HasCompStr();
  CompStr *LockCompStr();
  void UnlockCompStr();

  // message buffer
  LPTRANSMSG LockMsgBuf();
  void UnlockMsgBuf();
        DWORD& NumMsgBuf();
  const DWORD& NumMsgBuf() const;

  // guideline
  void MakeGuideLine(DWORD dwID);
  LPGUIDELINE LockGuideLine();
  void UnlockGuideLine();

  // actions
  void AddChar(WCHAR chTyped, WCHAR chTranslated);
  void CancelText();
  void RevertText();
  void DeleteChar(BOOL bBackSpace = FALSE);
  BOOL DoConvert();
  BOOL OpenCandidate();
  BOOL CloseCandidate();
  void GetCands(LogCandInfo& log, std::wstring& str);
  void MoveLeft(BOOL bShift);
  void MoveRight(BOOL bShift);
  void MoveToBeginning();
  void MoveToEnd();
  void MakeResult();
  void MakeHiragana();
  void MakeKatakana();
  void MakeHankaku();
  void MakeZenEisuu();
  void MakeHanEisuu();

  // for debugging
  void DumpCandInfo();
  void DumpCompStr();

private:
  // never be implemented
  InputContext();
  InputContext(const InputContext&);
  InputContext& operator=(const InputContext&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_H_

//////////////////////////////////////////////////////////////////////////////
