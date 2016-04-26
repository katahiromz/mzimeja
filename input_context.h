// input_context.h --- input context
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_CONTEXT_H_
#define INPUT_CONTEXT_H_

#include "comp_str.h"
#include "cand_info.h"

//////////////////////////////////////////////////////////////////////////////

struct InputContext : public INPUTCONTEXT {
  void Initialize();

        BOOL& IsOpen()       { return fOpen; }
  const BOOL& IsOpen() const { return fOpen; }
        DWORD& Conversion()       { return fdwConversion; }
  const DWORD& Conversion() const { return fdwConversion; }
        DWORD& Sentence()       { return fdwSentence; }
  const DWORD& Sentence() const { return fdwSentence; }

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

  void AddChar(WCHAR ch);
  void CancelText();
  void RevertText();
  void DeleteChar(BOOL bBackSpace = FALSE);
  BOOL DoConvert();
  void GetCands(LogCandInfo& log_cand_info, std::wstring& str);
  void MakeResult();

  void DumpCompStr();

private:
  InputContext();
  InputContext(const InputContext&);
  InputContext& operator=(const InputContext&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_CONTEXT_H_

//////////////////////////////////////////////////////////////////////////////
