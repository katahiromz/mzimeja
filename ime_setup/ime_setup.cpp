// ime_setup.cpp --- MZ-IME setup program
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS   // use fopen
#include "../targetver.h"       // target Windows version

#define NOMINMAX
#include <windows.h>
#include <shlwapi.h>
#include <dlgs.h>
#include <cstdlib>    // for __argc, __wargv
#include <cstring>    // for wcsrchr
#include <algorithm>  // for std::max

HINSTANCE g_hInstance;

//////////////////////////////////////////////////////////////////////////////

LPWSTR FindLocalFile(LPWSTR pszPath, LPCWSTR pszFileName)
{
    ::GetModuleFileNameW(NULL, pszPath, MAX_PATH);
    PathRemoveFileSpecW(pszPath);

    for (INT i = 0; i < 5; ++i)
    {
        size_t ich = wcslen(pszPath);
        {
            PathAppendW(pszPath, pszFileName);
            if (PathFileExistsW(pszPath))
                return pszPath;
        }
        pszPath[ich] = 0;
        {
            PathAppendW(pszPath, L"mzimeja");
            PathAppendW(pszPath, pszFileName);
            if (PathFileExistsW(pszPath))
                return pszPath;
        }
        pszPath[ich] = 0;
        PathRemoveFileSpecW(pszPath);
    }
    return NULL;
}

LPWSTR GetSrcImePathName(LPWSTR pszPath) {
    return FindLocalFile(pszPath, L"mzimeja.ime");
}

LPWSTR GetSystemImePathName(LPWSTR pszPath) {
    GetSystemDirectory(pszPath, MAX_PATH);
    wcscat(pszPath, L"\\mzimeja.ime");
    return pszPath;
}

LPWSTR GetBasicDictPathName(LPWSTR pszPath) {
    return FindLocalFile(pszPath, L"res\\mzimeja.dic");
}

LPWSTR GetNameDictPathName(LPWSTR pszPath) {
    return FindLocalFile(pszPath, L"res\\name.dic");
}

LPWSTR GetKanjiDataPathName(LPWSTR pszPath) {
    return FindLocalFile(pszPath, L"res\\kanji.dat");
}

LPWSTR GetRadicalDataPathName(LPWSTR pszPath) {
    return FindLocalFile(pszPath, L"res\\radical.dat");
}

LPWSTR GetImePadPathName(LPWSTR pszPath) {
    return FindLocalFile(pszPath, L"imepad.exe");
}

LPWSTR GetVerInfoPathName(LPWSTR pszPath) {
    return FindLocalFile(pszPath, L"verinfo.exe");
}

LPWSTR GetReadMePathName(LPWSTR pszPath) {
    LPWSTR ret = FindLocalFile(pszPath, L"mzimeja\\READMEJP.txt");
    if (ret)
        return ret;
    return FindLocalFile(pszPath, L"READMEJP.txt");
}

//////////////////////////////////////////////////////////////////////////////
// registry

LONG OpenRegKey(HKEY hKey, LPCWSTR pszSubKey, BOOL bWrite, HKEY *phSubKey) {
    LONG result;
    REGSAM sam = (bWrite ? KEY_WRITE : KEY_READ);
    result = ::RegOpenKeyExW(hKey, pszSubKey, 0, sam | KEY_WOW64_64KEY, phSubKey);
    if (result != ERROR_SUCCESS) {
        result = ::RegOpenKeyExW(hKey, pszSubKey, 0, sam, phSubKey);
    }
    return result;
} // OpenRegKey

LONG CreateRegKey(HKEY hKey, LPCWSTR pszSubKey, HKEY *phSubKey) {
    LONG result;
    DWORD dwDisposition;
    const REGSAM sam = KEY_WRITE;
    result = ::RegCreateKeyExW(hKey, pszSubKey, 0, NULL, 0, sam |
                               KEY_WOW64_64KEY, NULL, phSubKey, &dwDisposition);
    if (result != ERROR_SUCCESS) {
        result = ::RegCreateKeyExW(hKey, pszSubKey, 0, NULL, 0, sam, NULL,
                                   phSubKey, &dwDisposition);
    }
    return result;
} // CreateRegKey

//////////////////////////////////////////////////////////////////////////////

LPCTSTR DoLoadString(INT nID) {
    static WCHAR s_szBuf[1024];
    s_szBuf[0] = 0;
    LoadStringW(g_hInstance, nID, s_szBuf, 1024);
    return s_szBuf[0] ? s_szBuf : L"Internal Error";
}

INT DoCopyFiles(VOID) {
    WCHAR szPathSrc[MAX_PATH], szPathDest[MAX_PATH];

    //////////////////////////////////////////////////////////
    // {app}\mzimeja.ime --> C:\Windows\system32\mzimeja.ime

    // source
    GetSrcImePathName(szPathSrc);
    // dest
    GetSystemImePathName(szPathDest);
    // copy
    BOOL b1 = CopyFile(szPathSrc, szPathDest, FALSE);
    if (!b1) {
        return 2;
    }

    return 0;
} // DoCopyFiles

INT DoDeleteFiles(VOID) {
    WCHAR szPath[MAX_PATH];
    if (!DeleteFileW(GetSystemImePathName(szPath))) {
        return 1;
    }
    return 0;
}

BOOL DoSetRegSz(HKEY hKey, const WCHAR *pszName, const WCHAR *pszValue) {
    DWORD cbData = (lstrlenW(pszValue) + 1) * sizeof(WCHAR);
    LONG result;
    result = RegSetValueExW(hKey, pszName, 0, REG_SZ, (BYTE *)pszValue, cbData);
    return result == ERROR_SUCCESS;
}

static const WCHAR s_szKeyboardLayouts[] =
        L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts";

INT DoSetRegistry1(VOID) {
    BOOL ret = FALSE;
    HKEY hKey;
    LONG result = OpenRegKey(HKEY_LOCAL_MACHINE, s_szKeyboardLayouts, TRUE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        HKEY hkLayouts;
        result = CreateRegKey(hKey, L"E0120411", &hkLayouts);
        if (result == ERROR_SUCCESS && hkLayouts) {
            if (DoSetRegSz(hkLayouts, L"Layout File", L"kbdjpn.kbd") &&
                DoSetRegSz(hkLayouts, L"Layout Text", DoLoadString(4)) &&
                DoSetRegSz(hkLayouts, L"IME File", L"mzimeja.ime"))
            {
                ret = TRUE;
            }
            RegCloseKey(hkLayouts);
        }
        RegCloseKey(hKey);
    }
    return (ret ? 0 : -1);
} // DoSetRegistry1

INT DoSetRegistry2(VOID) {
    BOOL ret = FALSE;
    HKEY hKey;
    LONG result = OpenRegKey(HKEY_LOCAL_MACHINE, L"SOFTWARE", TRUE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        HKEY hkCompany;
        result = CreateRegKey(hKey, L"Katayama Hirofumi MZ", &hkCompany);
        if (result == ERROR_SUCCESS && hkCompany) {
            HKEY hkSoftware;
            result = CreateRegKey(hkCompany, L"mzimeja", &hkSoftware);
            if (result == ERROR_SUCCESS && hkSoftware) {
                TCHAR szBasicDictPath[MAX_PATH];
                TCHAR szNameDictPath[MAX_PATH];
                TCHAR szKanjiPath[MAX_PATH];
                TCHAR szRadicalPath[MAX_PATH];
                TCHAR szImePadPath[MAX_PATH];
                TCHAR szVerInfoPath[MAX_PATH];
                TCHAR szReadMePath[MAX_PATH];

                GetBasicDictPathName(szBasicDictPath);
                GetNameDictPathName(szNameDictPath);
                GetKanjiDataPathName(szKanjiPath);
                GetRadicalDataPathName(szRadicalPath);
                GetImePadPathName(szImePadPath);
                GetVerInfoPathName(szVerInfoPath);
                GetReadMePathName(szReadMePath);

                if (
                        DoSetRegSz(hkSoftware, L"BasicDictPathName", szBasicDictPath) &&
                        DoSetRegSz(hkSoftware, L"NameDictPathName", szNameDictPath) &&
                        DoSetRegSz(hkSoftware, L"KanjiDataFile", szKanjiPath) &&
                        DoSetRegSz(hkSoftware, L"RadicalDataFile", szRadicalPath) &&
                        DoSetRegSz(hkSoftware, L"ImePadFile", szImePadPath) &&
                        DoSetRegSz(hkSoftware, L"VerInfoFile", szVerInfoPath) &&
                        DoSetRegSz(hkSoftware, L"ReadMeFile", szReadMePath))
                {
                    ret = TRUE;
                }
                RegCloseKey(hkSoftware);
            }
            RegCloseKey(hkCompany);
        }
        RegCloseKey(hKey);
    }
    return (ret ? 0 : -1);
} // DoSetRegistry2

LONG MyDeleteRegKey(HKEY hKey, LPCTSTR pszSubKey)
{
    LONG ret;
    DWORD cchSubKeyMax, cchValueMax;
    DWORD cchMax, cch;
    TCHAR szNameBuf[MAX_PATH], *pszName = szNameBuf;
    HKEY hSubKey = hKey;

    if (pszSubKey != NULL)
    {
        ret = OpenRegKey(hKey, pszSubKey, FALSE, &hSubKey);
        if (ret) return ret;
    }

    ret = RegQueryInfoKey(hSubKey, NULL, NULL, NULL, NULL,
                          &cchSubKeyMax, NULL, NULL, &cchValueMax, NULL, NULL, NULL);
    if (ret) goto cleanup;

    cchSubKeyMax++;
    cchValueMax++;
    cchMax = std::max(cchSubKeyMax, cchValueMax);
    if (cchMax > sizeof(szNameBuf) / sizeof(TCHAR))
    {
        pszName = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, cchMax *
                                    sizeof(TCHAR));
        if (pszName == NULL)
        {
            ret = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
    }

    while(TRUE)
    {
        cch = cchMax;
        if (RegEnumKeyEx(hSubKey, 0, pszName, &cch, NULL,
                         NULL, NULL, NULL)) break;

        ret = MyDeleteRegKey(hSubKey, pszName);
        if (ret) goto cleanup;
    }

    if (pszSubKey != NULL)
        ret = RegDeleteKey(hKey, pszSubKey);
    else
        while(TRUE)
        {
            cch = cchMax;
            if (RegEnumValue(hKey, 0, pszName, &cch,
                             NULL, NULL, NULL, NULL)) break;

            ret = RegDeleteValue(hKey, pszName);
            if (ret) goto cleanup;
        }

cleanup:
    if (pszName != szNameBuf)
        HeapFree(GetProcessHeap(), 0, pszName);
    if (pszSubKey != NULL)
        RegCloseKey(hSubKey);
    return ret;
} // MyDeleteRegKey

INT DoUnsetRegistry1(VOID) {
    BOOL ret = FALSE;
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                s_szKeyboardLayouts, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
    if (result != ERROR_SUCCESS) {
        result = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                               s_szKeyboardLayouts, 0, KEY_ALL_ACCESS, &hKey);
    }
    if (result == ERROR_SUCCESS && hKey) {
        result = MyDeleteRegKey(hKey, L"E0120411");
        if (result == ERROR_SUCCESS) {
            ret = TRUE;
        }
        RegCloseKey(hKey);
    }
    return (ret ? 0 : -1);
} // DoUnsetRegistry1

static const WCHAR s_sz_katahiromz[] =
        L"SOFTWARE\\Katayama Hirofumi MZ";

INT DoUnsetRegistry2(VOID) {
    BOOL ret = FALSE;
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                s_sz_katahiromz, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
    if (result != ERROR_SUCCESS) {
        result = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                               s_sz_katahiromz, 0, KEY_ALL_ACCESS, &hKey);
    }
    if (result == ERROR_SUCCESS && hKey) {
        result = MyDeleteRegKey(hKey, L"mzimeja");
        if (result == ERROR_SUCCESS) {
            ret = TRUE;
        }
        RegCloseKey(hKey);
    }
    return (ret ? 0 : -1);
} // DoUnsetRegistry2

//////////////////////////////////////////////////////////////////////////////

INT DoInstall(VOID) {
    if (0 != DoSetRegistry1()) {
        // failure
        ::MessageBoxW(NULL, DoLoadString(2), NULL, MB_ICONERROR);
        return 2;
    }
    if (0 != DoSetRegistry2()) {
        // failure
        ::MessageBoxW(NULL, DoLoadString(8), NULL, MB_ICONERROR);
        return 2;
    }

    if (0 != DoCopyFiles()) {
        // failure
        ::MessageBoxW(NULL, DoLoadString(3), NULL, MB_ICONERROR);
        return 1;
    }

    WCHAR szPath[MAX_PATH];
    ::GetSystemDirectoryW(szPath, MAX_PATH);
    wcscat(szPath, L"\\mzimeja.ime");
    if (!ImmInstallIME(szPath, DoLoadString(4))) {
        // failure
        WCHAR szMsg[128];
        DWORD dwError = ::GetLastError();
        ::wsprintfW(szMsg, DoLoadString(5), dwError);
        ::MessageBoxW(NULL, szMsg, NULL, MB_ICONERROR);
        return 3;
    }
    ShellExecuteW(NULL, NULL, L"control.exe", L"input.dll", NULL, SW_SHOWNORMAL);
    return 0;
} // DoInstall

INT DoUninstall(VOID) {
    if (0 != DoDeleteFiles()) {
        // failure
        ::MessageBoxW(NULL, DoLoadString(7), NULL, MB_ICONERROR);
        return 1;
    }

    if (0 != DoUnsetRegistry1() || 0 != DoUnsetRegistry2()) {
        // failure
        ::MessageBoxW(NULL, DoLoadString(6), NULL, MB_ICONERROR);
        return 2;
    }

    return 0;
} // DoUninstall

//////////////////////////////////////////////////////////////////////////////

extern "C"
INT_PTR CALLBACK
DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (::IsDlgButtonChecked(hWnd, rad1) == BST_CHECKED) {
                ::EndDialog(hWnd, rad1);
                break;
            }
            if (::IsDlgButtonChecked(hWnd, rad2) == BST_CHECKED) {
                ::EndDialog(hWnd, rad2);
                break;
            }
            break;
        case IDCANCEL:
            ::EndDialog(hWnd, IDCANCEL);
            break;
        }
    }
    return FALSE;
}
//////////////////////////////////////////////////////////////////////////////

extern "C"
INT WINAPI
wWinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPWSTR lpCmdLine,
        INT nCmdShow)
{
    g_hInstance = hInstance;

    int ret;
    switch (__argc) {
    case 2:
        if (lstrcmpiW(__wargv[1], L"/i") == 0) {
            return DoInstall();
        }
        if (lstrcmpiW(__wargv[1], L"/u") == 0) {
            return DoUninstall();
        }
        break;
    default:
        ret = ::DialogBoxW(hInstance, MAKEINTRESOURCEW(1), NULL, DialogProc);
        switch (ret) {
        case rad1:
            return DoInstall();
        case rad2:
            return DoUninstall();
        default:
            break;
        }
        break;
    }

    return 0;
} // wWinMain

//////////////////////////////////////////////////////////////////////////////
