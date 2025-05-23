﻿// immsec.cpp --- IMM security related
// IMMセキュリティ関連。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define MEMALLOC(x) LocalAlloc(LMEM_FIXED, x)
#define MEMFREE(x) LocalFree(x)

extern "C" {

//////////////////////////////////////////////////////////////////////////////
// internal functions

PSID MyCreateSid(VOID)
{
    PSID psid;
    BOOL fResult;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_WORLD_SID_AUTHORITY;

    // allocate and initialize an SID
    fResult = AllocateAndInitializeSid(&SidAuthority, 1, SECURITY_WORLD_RID, 0, 0,
                                       0, 0, 0, 0, 0, &psid);
    if (!fResult) {
        DPRINTA("MyCreateSid:AllocateAndInitializeSid failed");
        return NULL;
    }

    if (!IsValidSid(psid)) {
        DPRINTA("MyCreateSid:AllocateAndInitializeSid returns bogus sid");
        FreeSid(psid);
        return NULL;
    }

    return psid;
}

POSVERSIONINFO GetVersionInfo(VOID)
{
    static BOOL fFirstCall = TRUE;
    static OSVERSIONINFO os;

    if (fFirstCall) {
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx(&os)) {
            fFirstCall = FALSE;
        }
    }
    return &os;
}

// CreateSecurityAttributes()
//
// The purpose of this function:
//      Allocate and set the security attributes that is
//      appropriate for named objects created by an IME.
//      The security attributes will give GENERIC_ALL
//      access for everyone
//
// Return value:
//      If the function succeeds, the return value is a
//      pointer to SECURITY_ATTRIBUTES. If the function fails,
//      the return value is NULL. To get extended error
//      information, call GetLastError().
//
// Remarks:
//      FreeSecurityAttributes() should be called to free up the
//      SECURITY_ATTRIBUTES allocated by this function.
SECURITY_ATTRIBUTES *CreateSecurityAttributes(void)
{
    if (!IsNT()) return NULL;

    // create a sid for everyone access
    PSID psid = MyCreateSid();
    if (psid == NULL) {
        return NULL;
    }

    // allocate and initialize an access control list (ACL) that will
    // contain the SID we've just created.
    DWORD cbacl;
    cbacl = sizeof(ACL) + (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
            GetLengthSid(psid);

    PACL pacl;
    pacl = (PACL)MEMALLOC(cbacl);
    if (pacl == NULL) {
        DPRINTA("CreateSecurityAttributes:LocalAlloc for ACL failed");
        FreeSid(psid);
        return NULL;
    }

    BOOL fResult = InitializeAcl(pacl, cbacl, ACL_REVISION);
    if (!fResult) {
        DPRINTA("CreateSecurityAttributes:InitializeAcl failed");
        FreeSid(psid);
        MEMFREE(pacl);
        return NULL;
    }

    //
    // adds an access-allowed ACE for interactive users to the ACL
    //
    fResult = AddAccessAllowedAce(pacl, ACL_REVISION, GENERIC_ALL, psid);
    if (!fResult) {
        DPRINTA("CreateSecurityAttributes:AddAccessAllowedAce failed");
        MEMFREE(pacl);
        FreeSid(psid);
        return NULL;
    }

    // Those SIDs have been copied into the ACL. We don't need'em any more.
    FreeSid(psid);

    // Let's make sure that our ACL is valid.
    if (!IsValidAcl(pacl)) {
        DPRINTA("CreateSecurityAttributes:IsValidAcl returns FALSE!");
        MEMFREE(pacl);
        return NULL;
    }

    // allocate security attribute
    SECURITY_ATTRIBUTES *psa;
    psa = (PSECURITY_ATTRIBUTES)MEMALLOC(sizeof(SECURITY_ATTRIBUTES));
    if (psa == NULL) {
        DPRINTA("CreateSecurityAttributes:LocalAlloc for psa failed");
        MEMFREE(pacl);
        return NULL;
    }

    // allocate and initialize a new security descriptor
    PSECURITY_DESCRIPTOR psd;
    psd = MEMALLOC(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (psd == NULL) {
        DPRINTA("CreateSecurityAttributes:LocalAlloc for psd failed");
        MEMFREE(pacl);
        MEMFREE(psa);
        return NULL;
    }

    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION)) {
        DPRINTA("CreateSecurityAttributes:InitializeSecurityDescriptor failed");
        MEMFREE(pacl);
        MEMFREE(psa);
        MEMFREE(psd);
        return NULL;
    }

    // The discretionary ACL is referenced by, not copied
    // into, the security descriptor. We shouldn't free up ACL
    // after the SetSecurityDescriptorDacl call.
    fResult = SetSecurityDescriptorDacl(psd, TRUE, pacl, FALSE);
    if (!fResult) {
        DPRINTA("CreateSecurityAttributes:SetSecurityDescriptorDacl failed");
        MEMFREE(pacl);
        MEMFREE(psa);
        MEMFREE(psd);
        return NULL;
    }

    if (!IsValidSecurityDescriptor(psd)) {
        DPRINTA("CreateSecurityAttributes:IsValidSecurityDescriptor failed!");
        MEMFREE(pacl);
        MEMFREE(psa);
        MEMFREE(psd);
        return NULL;
    }

    // everything is done
    psa->nLength = sizeof(SECURITY_ATTRIBUTES);
    psa->lpSecurityDescriptor = psd;
    psa->bInheritHandle = TRUE;

    return psa;
}

// FreeSecurityAttributes()
//
// The purpose of this function:
//      Frees the memory objects allocated by previous
//      CreateSecurityAttributes() call.
void FreeSecurityAttributes(SECURITY_ATTRIBUTES *psa)
{
    if (psa == NULL) return;

    BOOL fResult;
    BOOL fDaclPresent;
    BOOL fDaclDefaulted;
    PACL pacl;
    fResult = GetSecurityDescriptorDacl(psa->lpSecurityDescriptor, &fDaclPresent,
                                        &pacl, &fDaclDefaulted);
    if (fResult) {
        if (pacl != NULL) MEMFREE(pacl);
    } else {
        DPRINTA("FreeSecurityAttributes:GetSecurityDescriptorDacl failed");
    }

    MEMFREE(psa->lpSecurityDescriptor);
    MEMFREE(psa);
}

// IsNT()
//
// Return value:
//      TRUE if the current system is Windows NT
//
// Remarks:
//      The implementation of this function is not multi-thread safe.
//      You need to modify the function if you call the function in
//      multi-thread environment.
BOOL IsNT(void)
{
    return GetVersionInfo()->dwPlatformId == VER_PLATFORM_WIN32_NT;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
