#ifndef IMMSEC_H_
#define IMMSEC_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif

extern "C" {

PSECURITY_ATTRIBUTES CreateSecurityAttributes(void);
VOID FreeSecurityAttributes(PSECURITY_ATTRIBUTES psa);
BOOL IsNT(VOID);

} // extern "C"

#endif  // ndef IMMSEC_H_
