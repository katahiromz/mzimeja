// debug.cpp --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include <shlobj.h>
#include <strsafe.h>
#include <stdio.h>
#include "resource.h"

extern "C"
{

//////////////////////////////////////////////////////////////////////////////
// デバッグ用。

#ifndef NDEBUG
BOOL g_bTrace = TRUE;   // この変数がFALSEのときはデバッグ出力しない。

// printf関数と同じ文法でデバッグ出力を行う関数。
void DebugPrintA(const char *lpszFormat, ...)
{
    char szMsgA[1024];

    if (!g_bTrace)
        return;

    va_list marker;
    va_start(marker, lpszFormat);
    StringCchVPrintfA(szMsgA, _countof(szMsgA), lpszFormat, marker);
    va_end(marker);

#ifdef USE_LOGFILE
    FILE *fout = fopen("C:\\mzimeja.log", "a");
    fprintf("%s", szMsgA);
    fclose(fout);
#else
    OutputDebugStringA(szMsgA);
#endif
}

// wprintf関数と同じ文法でデバッグ出力を行う関数。
void DebugPrintW(const WCHAR *lpszFormat, ...)
{
    WCHAR szMsg[1024];

    if (!g_bTrace)
        return;

    va_list marker;
    va_start(marker, lpszFormat);
    StringCchVPrintfW(szMsg, _countof(szMsg), lpszFormat, marker);
    va_end(marker);

#ifdef USE_LOGFILE
    FILE *fout = fopen("C:\\mzimeja.log", "a");
    fprintf("%ls", szMsg);
    fclose(fout);
#else
    OutputDebugStringW(szMsg);
#endif
}

// ASSERT失敗時に呼び出される関数。
void DebugAssert(const char *file, int line, const char *exp)
{
    DebugPrintA("%s (%d): ASSERT(%s) failed\n", file, line, exp);

#if 0
    WCHAR szText[1024];
    StringCchPrintfW(szText, _countof(szText), L"%hs (%d): ASSERT(%hs) failed\n", file, line, exp);
    MessageBoxW(NULL, szText, L"Assertion Failure", MB_ICONERROR);
#endif
}
#endif  // ndef NDEBUG

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"
