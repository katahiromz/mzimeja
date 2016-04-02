// input_context.h --- input context
//////////////////////////////////////////////////////////////////////////////

#ifndef INPUT_CONTEXT_H_
#define INPUT_CONTEXT_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include "immdev.h"

//////////////////////////////////////////////////////////////////////////////

struct InputContext : public INPUTCONTEXT {
  void Initialize();
  BOOL IsCandidate();

  LPCANDIDATEINFO LockCandInfo();
  void UnlockCandInfo();

  LPCOMPOSITIONSTRING LockCompStr();
  void UnlockCompStr();

  LPTRANSMSG LockTransMsg();
  void UnlockTransMsg();

        DWORD& NumTransMsg();
  const DWORD& NumTransMsg() const;

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
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_CONTEXT_H_

//////////////////////////////////////////////////////////////////////////////
