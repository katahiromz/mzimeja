// input_context.h --- input context
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_CONTEXT_H_
#define INPUT_CONTEXT_H_

#include "comp_str.h"
#include "cand_info.h"

//////////////////////////////////////////////////////////////////////////////

struct InputContext : public INPUTCONTEXT {
  void Initialize();
  BOOL HasCandInfo();
  BOOL HasCompStr();

  CandInfo *LockCandInfo();
  void UnlockCandInfo();

  CompStr *LockCompStr();
  void UnlockCompStr();

  LPTRANSMSG LockMsgBuf();
  void UnlockMsgBuf();

        DWORD& NumMsgBuf();
  const DWORD& NumMsgBuf() const;

  LPGUIDELINE LockGuideLine();
  void UnlockGuideLine();

  BOOL HasStatusWndPos() const { return (fdwInit & INIT_STATUSWNDPOS); }
  BOOL HasConversion() const { return (fdwInit & INIT_CONVERSION); }
  BOOL HasSentence() const { return (fdwInit & INIT_SENTENCE); }
  BOOL HasLogFont() const { return (fdwInit & INIT_LOGFONT); }
  BOOL HasCompForm() const { return (fdwInit & INIT_COMPFORM); }
  BOOL HasSoftKbdPos() const { return (fdwInit & INIT_SOFTKBDPOS); }

  void SetsStatusWndPos() { fdwInit |= INIT_STATUSWNDPOS; }
  void SetsConversion() { fdwInit |= INIT_CONVERSION; }
  void SetsSentence() { fdwInit |= INIT_SENTENCE; }
  void SetsLogFont() { fdwInit |= INIT_LOGFONT; }
  void SetsCompForm() { fdwInit |= INIT_COMPFORM; }
  void SetsSoftKbdPos() { fdwInit |= INIT_SOFTKBDPOS; }

private:
  InputContext();
  InputContext(const InputContext&);
  InputContext& operator=(const InputContext&);
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_CONTEXT_H_

//////////////////////////////////////////////////////////////////////////////
