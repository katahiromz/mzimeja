/*++

Copyright (c) 1990-1998 Microsoft Corporation, All Rights Reserved

Module Name:

    IMMSEC.H
    
++*/

extern "C" {

PSECURITY_ATTRIBUTES CreateSecurityAttributes(VOID);
VOID FreeSecurityAttributes( PSECURITY_ATTRIBUTES psa);
BOOL IsNT(VOID);

} // extern "C"
