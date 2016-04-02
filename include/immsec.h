/*++

Copyright (c) 1990-1998 Microsoft Corporation, All Rights Reserved

Module Name:

    IMMSEC.H
    
++*/

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif

extern "C" {

PSECURITY_ATTRIBUTES CreateSecurityAttributes(VOID);
VOID FreeSecurityAttributes( PSECURITY_ATTRIBUTES psa);
BOOL IsNT(VOID);

} // extern "C"
