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

  LPGUIDELINE LockGuideLine();
  void UnlockGuideLine();
};

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef INPUT_CONTEXT_H_

//////////////////////////////////////////////////////////////////////////////
