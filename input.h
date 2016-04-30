// input.h --- input
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_H_
#define INPUT_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

#include <string>
#include <vector>

#include "SmallWString.hpp"   // for SmallWString

//////////////////////////////////////////////////////////////////////////////
// input modes

enum INPUT_MODE {
  IMODE_ZEN_HIRAGANA,
  IMODE_ZEN_KATAKANA,
  IMODE_ZEN_EISUU,
  IMODE_HAN_KANA,
  IMODE_HAN_EISUU,
  IMODE_DISABLED
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
// composition string

struct LogCompStrExtra {
  DWORD                       dwPhonemeCursor;
  std::vector<SmallWString>   hiragana_phonemes;
  std::vector<SmallWString>   typing_phonemes;
  std::vector<DWORD>          phoneme_clauses;  // for future use
  LogCompStrExtra() : dwPhonemeCursor(0) {}
  DWORD GetTotalSize() const;
  void clear() {
    dwPhonemeCursor = 0;
    hiragana_phonemes.clear();
    typing_phonemes.clear();
    phoneme_clauses.clear();
  }
  void AssertValid() {
    assert(hiragana_phonemes.size() == typing_phonemes.size());
    assert(dwPhonemeCursor <= (DWORD)hiragana_phonemes.size());
  }
}; // struct LogCompStrExtra

struct COMPSTREXTRA {
  DWORD dwSignature;
  DWORD dwPhonemeCursor;
  DWORD dwHiraganaPhonemeLen;
  DWORD dwHiraganaPhonemeOffset;
  DWORD dwTypingPhonemeLen;
  DWORD dwTypingPhonemeOffset;
  LPBYTE GetBytes() { return (LPBYTE)this; }

  LPWSTR GetHiraganaPhonemes(DWORD& dwCount) {
    dwCount = dwHiraganaPhonemeLen / sizeof(WCHAR);
    if (dwCount) {
      return (LPWSTR)(GetBytes() + dwHiraganaPhonemeOffset);
    }
    return NULL;
  }
  LPWSTR GetTypingPhonemes(DWORD& dwCount) {
    dwCount = dwTypingPhonemeLen / sizeof(WCHAR);
    if (dwCount) {
      return (LPWSTR)(GetBytes() + dwTypingPhonemeOffset);
    }
    return NULL;
  }

  void GetLog(LogCompStrExtra& log);
  DWORD Store(const LogCompStrExtra *log);
}; // struct COMPSTREXTRA

struct LogCompStr {
  DWORD               dwCursorPos;
  DWORD               dwDeltaStart;
  std::vector<BYTE>   comp_read_attr;
  std::vector<DWORD>  comp_read_clause;
  std::wstring        comp_read_str;
  std::vector<BYTE>   comp_attr;
  std::vector<DWORD>  comp_clause;
  std::wstring        comp_str;
  std::vector<DWORD>  result_read_clause;
  std::wstring        result_read_str;
  std::vector<DWORD>  result_clause;
  std::wstring        result_str;
  LogCompStrExtra     extra;
  LogCompStr() {
    dwCursorPos = 0;
    dwDeltaStart = 0;
    comp_read_clause.resize(2, 0);
    comp_clause.resize(2, 0);
    result_read_clause.resize(2, 0);
    result_clause.resize(2, 0);
  }
  DWORD GetTotalSize() const;

  void AddChar(WCHAR chTyped, WCHAR chTranslated, INPUT_MODE imode);
  void Revert();
  void DeleteChar(BOOL bBackSpace = FALSE);
  void MakeResult();

  void MakeHiragana();
  void MakeKatakana();
  void MakeHankaku();
  void MakeZenEisuu();
  void MakeHanEisuu();
}; // struct LogCompStr

inline void SetClause(LPDWORD lpdw, DWORD num) {
  *lpdw = 0;
  *(lpdw + 1) = num;
}

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

struct LogCandInfo {
  DWORD   dwStyle;
  DWORD   dwSelection;
  DWORD   dwPageStart;
  DWORD   dwPageSize;
  std::vector<std::wstring> cand_strs;

  LogCandInfo(DWORD dwCandStyle = IME_CAND_READ) {
    dwStyle = dwCandStyle;
    dwSelection = 0;
    dwPageStart = 0;
    dwPageSize = 0;
  }
  DWORD GetTotalSize() const;
}; // struct LogCandInfo

struct CandList : public CANDIDATELIST {
  LPBYTE GetBytes() { return (LPBYTE)this; }
  LPTSTR GetCandString(DWORD i) { return LPTSTR(GetBytes() + dwOffset[i]); }
  LPTSTR GetCurString() { return GetCandString(dwSelection); }
  DWORD  GetPageEnd() const { return dwPageStart + dwPageSize; }

private:
  // not implemented
  CandList();
  CandList(const CandList&);
  CandList& operator=(const CandList&);
}; // struct CandList

struct CandInfo : public CANDIDATEINFO {
  static HIMCC ReCreate(HIMCC hCandInfo, const LogCandInfo *log = NULL);
  void GetLog(LogCandInfo& log);

  LPBYTE GetBytes() { return (LPBYTE)this; }
  CandList *GetList(DWORD i = 0) {
    return (CandList *)(GetBytes() + dwOffset[i]);
  }

  void Dump();

private:
  // not implement
  CandInfo();
  CandInfo(const CandInfo&);
  CandInfo& operator=(const CandInfo&);
}; // struct CandInfo

//////////////////////////////////////////////////////////////////////////////
// input context

struct InputContext : public INPUTCONTEXT {
  void Initialize();

        BOOL& IsOpen()       { return fOpen; }
  const BOOL& IsOpen() const { return fOpen; }
        DWORD& Conversion()       { return fdwConversion; }
  const DWORD& Conversion() const { return fdwConversion; }
        DWORD& Sentence()       { return fdwSentence; }
  const DWORD& Sentence() const { return fdwSentence; }
  INPUT_MODE GetInputMode() const;
  BOOL IsRoman() const;

  BOOL HasCandInfo();
  CandInfo *LockCandInfo();
  void UnlockCandInfo();

  BOOL HasCompStr();
  CompStr *LockCompStr();
  void UnlockCompStr();

  LPTRANSMSG LockMsgBuf();
  void UnlockMsgBuf();

        DWORD& NumMsgBuf();
  const DWORD& NumMsgBuf() const;

  void MakeGuideLine(DWORD dwID);
  LPGUIDELINE LockGuideLine();
  void UnlockGuideLine();

  BOOL HasStatusWndPos() const  { return (fdwInit & INIT_STATUSWNDPOS); }
  BOOL HasConversion() const    { return (fdwInit & INIT_CONVERSION); }
  BOOL HasSentence() const      { return (fdwInit & INIT_SENTENCE); }
  BOOL HasLogFont() const       { return (fdwInit & INIT_LOGFONT); }
  BOOL HasCompForm() const      { return (fdwInit & INIT_COMPFORM); }
  BOOL HasSoftKbdPos() const    { return (fdwInit & INIT_SOFTKBDPOS); }

  void AddChar(WCHAR chTyped, WCHAR chTranslated);
  void CancelText();
  void RevertText();
  void DeleteChar(BOOL bBackSpace = FALSE);
  BOOL DoConvert();
  void GetCands(LogCandInfo& log_cand_info, std::wstring& str);

  void MakeResult();
  void MakeHiragana();
  void MakeKatakana();
  void MakeHankaku();
  void MakeZenEisuu();
  void MakeHanEisuu();

  void MoveLeft();
  void MoveRight();
  void MoveToBeginning();
  void MoveToEnd();

  void DumpCandInfo();
  void DumpCompStr();

private:
  InputContext();
  InputContext(const InputContext&);
  InputContext& operator=(const InputContext&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_H_

//////////////////////////////////////////////////////////////////////////////
