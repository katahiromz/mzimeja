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
  IMODE_DISABLED        // IME is disabled
};

BOOL IsInputModeOpen(INPUT_MODE imode);
INPUT_MODE InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion);
INPUT_MODE NextInputMode(INPUT_MODE imode);
INPUT_MODE GetInputMode(HIMC hIMC);
void SetInputMode(HIMC hIMC, INPUT_MODE imode);
BOOL IsRomanMode(HIMC hIMC);
void SetRomanMode(HIMC hIMC, BOOL bRoman);
UINT CommandFromInputMode(INPUT_MODE imode);

//////////////////////////////////////////////////////////////////////////////
// composition info

// logical comp info extra
struct LogCompStrExtra {
  // selected clause index (0xFFFFFFFF if clause was not selected)
  DWORD                       dwSelectedClause;
  // selected phoneme index
  DWORD                       dwSelectedPhoneme;
  // hiragana phonemes
  std::vector<std::wstring>   hiragana_phonemes;
  // typing phonemes
  std::vector<std::wstring>   typing_phonemes;
  // mapping from composition clause index to phoneme index
  std::vector<DWORD>          phoneme_clauses;

  LogCompStrExtra() { clear(); }
  void clear();
  DWORD GetTotalSize() const;
  std::wstring Join(const std::vector<std::wstring>& strs) const;
  std::wstring JoinLeft(const std::vector<std::wstring>& strs) const;
  std::wstring JoinRight(const std::vector<std::wstring>& strs) const;
  DWORD GetPhonemeCount() const;
  WCHAR GetPrevChar() const;
  void ErasePhoneme(DWORD iPhoneme);
  void InsertPhonemePair(std::wstring& typed, std::wstring& translated);
  void InsertThere(std::vector<std::wstring>& strs, std::wstring& str);
  bool IsValid();
}; // struct LogCompStrExtra

// physical comp info extar
struct COMPSTREXTRA {
  DWORD   dwSignature;
  DWORD   dwSelectedClause;
  DWORD   dwSelectedPhoneme;
  DWORD   dwHiraganaPhonemeCount;
  DWORD   dwHiraganaPhonemeOffset;
  DWORD   dwTypingPhonemeCount;
  DWORD   dwTypingPhonemeOffset;

  LPBYTE GetBytes() { return (LPBYTE)this; }

  LPWSTR GetHiraganaPhonemes(DWORD& dwCount) {
    dwCount = dwHiraganaPhonemeCount;
    if (dwCount) {
      return (LPWSTR)(GetBytes() + dwHiraganaPhonemeOffset);
    }
    return NULL;
  }
  LPWSTR GetTypingPhonemes(DWORD& dwCount) {
    dwCount = dwTypingPhonemeCount;
    if (dwCount) {
      return (LPWSTR)(GetBytes() + dwTypingPhonemeOffset);
    }
    return NULL;
  }

  // physical to logical
  void GetLog(LogCompStrExtra& log);
  // physical from logical
  DWORD Store(const LogCompStrExtra *log);
}; // struct COMPSTREXTRA

// logical composition info
struct LogCompStr {
  DWORD               dwCursorPos;
  DWORD               dwDeltaStart;
  // We don't use this member
  std::vector<BYTE>   comp_read_attr;
  // We don't use this member
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

  bool IsValid();
  void Dump();

  void AddChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConversion);
  void RevertText();
  void DeleteChar(BOOL bRoman, BOOL bBackSpace = FALSE);
  void MakeResult();

  void MoveLeft(BOOL bShift);
  void MoveRight(BOOL bShift);

  BOOL HasClauseSelected() const;
  DWORD GetClauseCount() const;
  BYTE GetClauseAttr(DWORD iClause) const;
  void SetClauseAttr(DWORD iClause, BYTE attr);
  BYTE GetCompCharAttr(DWORD ich) const;
  BOOL IsClauseConverted(DWORD iClause) const;

  DWORD GetPhonemeCount() const;
  DWORD GetCompCharCount() const;

  // conversion of composition
  void MakeHiragana();
  void MakeKatakana();
  void MakeHankaku();
  void MakeZenEisuu();
  void MakeHanEisuu();

  // conversion of locative information
  DWORD ClauseToPhoneme(DWORD iClause) const;
  DWORD ClauseToCompChar(DWORD iClause) const;
  DWORD PhonemeToClause(DWORD iPhoneme) const;
  DWORD PhonemeToCompChar(DWORD iPhoneme, DWORD dwDeltaChar) const;
  DWORD CompCharToClause(DWORD iCompChar) const;
  DWORD CompCharToPhoneme(DWORD iCompChar, DWORD& dwDeltaChar) const;

  // get clause information
  DWORD GetCurrentClause() const;
  std::wstring GetLeft(DWORD iClause) const;
  std::wstring GetRight(DWORD iClause) const;
  std::wstring GetClauseCompString(DWORD iClause) const;
  std::wstring GetClauseHiraganaString(DWORD iClause) const;
  std::wstring GetClauseTypingString(DWORD iClause) const;
  void SetClauseCompString(DWORD iClause, std::wstring& strClause, BOOL bConverted);
  BOOL IsBeingConverted();
  void MakeCompString(DWORD dwConversion);
  std::wstring Translate(
    const std::wstring& hiragana, const std::wstring& typing, DWORD dwConversion);

protected:
  void ExtraUpdated(INPUT_MODE imode);
  void AddKanaChar(std::wstring& typed, std::wstring& translated, DWORD dwConversion);
  void AddRomanChar(std::wstring& typed, std::wstring& translated, DWORD dwConversion);
}; // struct LogCompStr

inline void SetClause(LPDWORD lpdw, DWORD num) {
  *lpdw = 0;
  *(lpdw + 1) = num;
}

// physical composition info
struct CompStr : public COMPOSITIONSTRING {
  static HIMCC ReCreate(HIMCC hCompStr, const LogCompStr *log = NULL);

  void GetLog(LogCompStr& log);
  BOOL IsBeingConverted();

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

// logical candidate info
struct LogCandInfo {
  DWORD   dwStyle;
  DWORD   dwSelection;
  DWORD   dwPageStart;
  DWORD   dwPageSize;
  std::vector<std::wstring> cand_strs;
  LogCandInfo() { clear(); }
  void clear();
  DWORD GetTotalSize() const;
}; // struct LogCandInfo

// physical candidate list
struct CandList : public CANDIDATELIST {
  LPBYTE GetBytes() { return (LPBYTE)this; }
  LPTSTR GetCandString(DWORD i) { return LPTSTR(GetBytes() + dwOffset[i]); }
  LPTSTR GetCurString() { return GetCandString(dwSelection); }
  DWORD  GetPageEnd() const { return dwPageStart + dwPageSize; }

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

  LPBYTE GetBytes() { return (LPBYTE)this; }
  CandList *GetList(DWORD i = 0) {
    return (CandList *)(GetBytes() + dwOffset[i]);
  }

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
  void GetCands(LogCandInfo& log_cand_info, std::wstring& str);
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
