// mzimeja.cpp --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#include "mzimeja.h"
#include <shlobj.h>
#include <strsafe.h>
#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

// The window classes for mzimeja UI windows.
// mzimeja�Ŏg����E�B���h�E�N���X�̖��O�B
const WCHAR szUIServerClassName[] = L"MZIMEUI";
const WCHAR szCompStrClassName[]  = L"MZIMECompStr";
const WCHAR szCandClassName[]     = L"MZIMECand";
const WCHAR szStatusClassName[]   = L"MZIMEStatus";
const WCHAR szGuideClassName[]    = L"MZIMEGuide";

// �K�C�h���C���̃e�[�u���B
const MZGUIDELINE glTable[] = {
    {GL_LEVEL_ERROR, GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
    {GL_LEVEL_WARNING, GL_ID_TYPINGERROR, IDS_GL_TYPINGERROR, 0},
    {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST, IDS_GL_TESTGUIDELINESTR,
     IDS_GL_TESTGUIDELINEPRIVATE}
};

// The filename of the IME.
// IME�̃t�@�C�����B
const WCHAR szImeFileName[] = L"mzimeja.ime";

//////////////////////////////////////////////////////////////////////////////

HFONT CheckNativeCharset(HDC hDC) {
    HFONT hOldFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);

    LOGFONT lfFont;
    GetObject(hOldFont, sizeof(LOGFONT), &lfFont);

    if (lfFont.lfCharSet != SHIFTJIS_CHARSET) {
        lfFont.lfWeight = FW_NORMAL;
        lfFont.lfCharSet = SHIFTJIS_CHARSET;
        lfFont.lfFaceName[0] = 0;
        SelectObject(hDC, CreateFontIndirect(&lfFont));
    } else {
        hOldFont = NULL;
    }
    return hOldFont;
} // CheckNativeCharset

// adjust window position
void RepositionWindow(HWND hWnd) {
    RECT rc, rcWorkArea;
    ::GetWindowRect(hWnd, &rc);
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
    SIZE siz;
    siz.cx = rc.right - rc.left;
    siz.cy = rc.bottom - rc.top;
    if (rc.right > rcWorkArea.right) {
        rc.right = rcWorkArea.right;
        rc.left = rcWorkArea.right - siz.cx;
    }
    if (rc.left < rcWorkArea.left) {
        rc.left = rcWorkArea.left;
        rc.right = rc.left + siz.cx;
    }
    if (rc.bottom > rcWorkArea.bottom) {
        rc.bottom = rcWorkArea.bottom;
        rc.top = rcWorkArea.bottom - siz.cy;
    }
    if (rc.top < rcWorkArea.top) {
        rc.top = rcWorkArea.top;
        rc.bottom = rc.top + siz.cy;
    }
    ::MoveWindow(hWnd, rc.left, rc.top, siz.cx, siz.cy, TRUE);
}

//////////////////////////////////////////////////////////////////////////////

MzIme TheIME;

MzIme::MzIme() {
    m_hInst = NULL;
    m_hMyKL = NULL;
    m_bWinLogOn = FALSE;

    m_lpCurTransKey = NULL;
    m_uNumTransKey = 0;

    m_fOverflowKey = FALSE;

    m_hIMC = NULL;
    m_lpIMC = NULL;
}

BOOL MzIme::LoadDict() {
    BOOL ret = TRUE;
    DWORD dw;

    std::wstring basic;
    if (!GetUserDword(L"BasicDictDisabled", &dw) || !dw) {
        if (GetUserString(L"BasicDictPathName", basic) ||
            GetComputerString(L"BasicDictPathName", basic))
        {
            if (!m_basic_dict.Load(basic.c_str(), L"BasicDictObject")) {
                ret = FALSE;
            }
        }
    } else {
        m_basic_dict.Unload();
    }

    std::wstring name;
    if (!GetUserDword(L"NameDictDisabled", &dw) || !dw) {
        if (GetUserString(L"NameDictPathName", name) ||
            GetComputerString(L"NameDictPathName", name))
        {
            if (!m_name_dict.Load(name.c_str(), L"NameDictObject")) {
                ret = FALSE;
            }
        }
    } else {
        m_name_dict.Unload();
    }

    return ret;
}

void MzIme::UnloadDict() {
    m_basic_dict.Unload();
    m_name_dict.Unload();
}

BOOL MzIme::Init(HINSTANCE hInstance) {
    m_hInst = hInstance;
    //::InitCommonControls();

    MakeLiteralMaps();

    // load dict
    LoadDict();

    // register window classes for IME
    return RegisterClasses(m_hInst);
} // MzIme::Init

VOID MzIme::Uninit(VOID) {
    UnregisterClasses();
    UnloadDict();
}

//////////////////////////////////////////////////////////////////////////////
// registry

LONG MzIme::OpenRegKey(
        HKEY hKey, LPCWSTR pszSubKey, BOOL bWrite, HKEY *phSubKey) const
{
    LONG result;
    REGSAM sam = (bWrite ? KEY_WRITE : KEY_READ);
    result = ::RegOpenKeyExW(hKey, pszSubKey, 0, sam | KEY_WOW64_64KEY, phSubKey);
    if (result != ERROR_SUCCESS) {
        result = ::RegOpenKeyExW(hKey, pszSubKey, 0, sam, phSubKey);
    }
    return result;
} // MzIme::OpenRegKey

LONG MzIme::CreateRegKey(HKEY hKey, LPCWSTR pszSubKey, HKEY *phSubKey) {
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
} // MzIme::CreateRegKey

//////////////////////////////////////////////////////////////////////////////
// settings

static const WCHAR s_szRegKey[] =
        L"SOFTWARE\\Katayama Hirofumi MZ\\mzimeja";

LONG MzIme::OpenComputerSettingKey(BOOL bWrite, HKEY *phKey) {
    LONG result;
    if (bWrite) {
        result = OpenRegKey(HKEY_LOCAL_MACHINE, s_szRegKey, TRUE, phKey);
    } else {
        result = OpenRegKey(HKEY_LOCAL_MACHINE, s_szRegKey, FALSE, phKey);
    }
    return result;
}

LONG MzIme::OpenUserSettingKey(BOOL bWrite, HKEY *phKey) {
    LONG result;
    if (bWrite) {
        HKEY hSoftware;
        result = OpenRegKey(HKEY_CURRENT_USER, L"Software", TRUE, &hSoftware);
        if (result == ERROR_SUCCESS) {
            HKEY hCompany;
            result = CreateRegKey(hSoftware, L"Katayama Hirofumi MZ", &hCompany);
            if (result == ERROR_SUCCESS) {
                HKEY hMZIMEJA;
                result = CreateRegKey(hCompany, L"mzimeja", &hMZIMEJA);
                if (result == ERROR_SUCCESS) {
                    ::RegCloseKey(hMZIMEJA);
                }
                ::RegCloseKey(hCompany);
            }
            ::RegCloseKey(hSoftware);
        }
        result = OpenRegKey(HKEY_CURRENT_USER, s_szRegKey, TRUE, phKey);
    } else {
        result = OpenRegKey(HKEY_CURRENT_USER, s_szRegKey, FALSE, phKey);
    }
    return result;
}

BOOL MzIme::GetComputerString(LPCWSTR pszSettingName, std::wstring& value) {
    HKEY hKey;
    WCHAR szValue[MAX_PATH * 2];
    LONG result = OpenComputerSettingKey(FALSE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = sizeof(szValue);
        result = ::RegQueryValueExW(hKey, pszSettingName, NULL, NULL,
                                    reinterpret_cast<LPBYTE>(szValue), &cbData);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            value = szValue;
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::GetComputerString

BOOL MzIme::SetComputerString(LPCWSTR pszSettingName, LPCWSTR pszValue) {
    HKEY hKey;
    LONG result = OpenComputerSettingKey(TRUE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = (::lstrlenW(pszValue) + 1) * sizeof(WCHAR);
        result = ::RegSetValueExW(hKey, pszSettingName, 0, REG_SZ,
                                  reinterpret_cast<const BYTE *>(pszValue), cbData);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::SetComputerString

BOOL
MzIme::GetComputerData(LPCWSTR pszSettingName, void *ptr, DWORD size) {
    HKEY hKey;
    LONG result = OpenComputerSettingKey(FALSE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = size;
        result = ::RegQueryValueExW(hKey, pszSettingName, NULL, NULL,
                                    reinterpret_cast<LPBYTE>(ptr), &cbData);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::GetComputerData

BOOL
MzIme::SetComputerData(LPCWSTR pszSettingName, const void *ptr, DWORD size) {
    HKEY hKey;
    LONG result = OpenComputerSettingKey(TRUE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        result = ::RegSetValueExW(hKey, pszSettingName, 0, REG_BINARY,
                                  reinterpret_cast<const BYTE *>(ptr), size);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::SetComputerData

BOOL MzIme::GetComputerDword(LPCWSTR pszSettingName, DWORD *ptr) {
    HKEY hKey;
    LONG result = OpenComputerSettingKey(FALSE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = sizeof(DWORD);
        result = ::RegQueryValueExW(hKey, pszSettingName, NULL, NULL,
                                    reinterpret_cast<LPBYTE>(ptr), &cbData);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::GetComputerData

BOOL MzIme::SetComputerDword(LPCWSTR pszSettingName, DWORD data) {
    HKEY hKey;
    DWORD dwData = data;
    LONG result = OpenComputerSettingKey(TRUE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD size = sizeof(DWORD);
        result = ::RegSetValueExW(hKey, pszSettingName, 0, REG_BINARY,
                                  reinterpret_cast<const BYTE *>(&dwData), size);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::SetComputerData

BOOL MzIme::GetUserString(LPCWSTR pszSettingName, std::wstring& value) {
    HKEY hKey;
    WCHAR szValue[MAX_PATH * 2];
    LONG result = OpenUserSettingKey(FALSE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = sizeof(szValue);
        result = ::RegQueryValueExW(hKey, pszSettingName, NULL, NULL,
                                    reinterpret_cast<LPBYTE>(szValue), &cbData);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            value = szValue;
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::GetUserString

BOOL MzIme::SetUserString(LPCWSTR pszSettingName, LPCWSTR pszValue) {
    HKEY hKey;
    LONG result = OpenUserSettingKey(TRUE, &hKey);
    ASSERT(result == ERROR_SUCCESS);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = (::lstrlenW(pszValue) + 1) * sizeof(WCHAR);
        result = ::RegSetValueExW(hKey, pszSettingName, 0, REG_SZ,
                                  reinterpret_cast<const BYTE *>(pszValue), cbData);
        ::RegCloseKey(hKey);
        ASSERT(result == ERROR_SUCCESS);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::SetUserString

BOOL MzIme::GetUserData(LPCWSTR pszSettingName, void *ptr, DWORD size) {
    HKEY hKey;
    LONG result = OpenUserSettingKey(FALSE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = size;
        result = ::RegQueryValueExW(hKey, pszSettingName, NULL, NULL,
                                    reinterpret_cast<LPBYTE>(ptr), &cbData);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::GetUserData

BOOL MzIme::SetUserData(LPCWSTR pszSettingName, const void *ptr, DWORD size) {
    HKEY hKey;
    LONG result = OpenUserSettingKey(TRUE, &hKey);
    ASSERT(result == ERROR_SUCCESS);
    if (result == ERROR_SUCCESS && hKey) {
        result = ::RegSetValueExW(hKey, pszSettingName, 0, REG_BINARY,
                                  reinterpret_cast<const BYTE *>(ptr), size);
        ::RegCloseKey(hKey);
        ASSERT(result == ERROR_SUCCESS);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::SetUserData

BOOL MzIme::GetUserDword(LPCWSTR pszSettingName, DWORD *ptr) {
    HKEY hKey;
    LONG result = OpenUserSettingKey(FALSE, &hKey);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD cbData = sizeof(DWORD);
        result = ::RegQueryValueExW(hKey, pszSettingName, NULL, NULL,
                                    reinterpret_cast<LPBYTE>(ptr), &cbData);
        ::RegCloseKey(hKey);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::GetUserData

BOOL MzIme::SetUserDword(LPCWSTR pszSettingName, DWORD data) {
    HKEY hKey;
    DWORD dwData = data;
    LONG result = OpenUserSettingKey(TRUE, &hKey);
    ASSERT(result == ERROR_SUCCESS);
    if (result == ERROR_SUCCESS && hKey) {
        DWORD size = sizeof(DWORD);
        result = ::RegSetValueExW(hKey, pszSettingName, 0, REG_DWORD,
                                  reinterpret_cast<const BYTE *>(&dwData), size);
        ::RegCloseKey(hKey);
        ASSERT(result == ERROR_SUCCESS);
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
    }
    return FALSE;
} // MzIme::SetUserData

//////////////////////////////////////////////////////////////////////////////

// mzimeja�̃E�B���h�E�N���X��o�^����B
BOOL MzIme::RegisterClasses(HINSTANCE hInstance) {
#define CS_MZIME (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_IME)
    WNDCLASSEX wcx;

    // Register the UI server window.
    // UI�T�[�o�[�E�B���h�E��o�^�B
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = MZIMEWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 2 * sizeof(LONG_PTR);
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szUIServerClassName;
    wcx.hbrBackground = NULL;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the composition window.
    // ���m�蕶����E�B���h�E��o�^�B
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = CompWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szCompStrClassName;
    wcx.hbrBackground = NULL;
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the candidate window.
    // ���E�B���h�E��o�^�B
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = CandWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szCandClassName;
    wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the status window.
    // ��ԃE�B���h�E��o�^�B
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = StatusWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szStatusClassName;
    wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    // Register the guideline window.
    // �K�C�h���C���E�B���h�E��o�^�B
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_MZIME;
    wcx.lpfnWndProc = GuideWnd_WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = UIEXTRASIZE;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hIcon = NULL;
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = szGuideClassName;
    wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcx.hIconSm = NULL;
    if (!RegisterClassEx(&wcx)) {
        ASSERT(0);
        return FALSE;
    }

    return TRUE;
#undef CS_MZIME
} // MzIme::RegisterClasses

// �L�[�{�[�h���C�A�E�g���X�g���玩����HKL���擾����B
HKL MzIme::GetHKL(VOID) {
    HKL hKL = NULL;

    // get list size and allocate buffer for list
    DWORD dwCount = ::GetKeyboardLayoutList(0, NULL);
    HKL* pHKLs = (HKL*)new(std::nothrow) HKL[dwCount];
    if (pHKLs  == NULL) {
        ASSERT(0);
        return NULL;
    }

    // get the list of keyboard layouts
    ::GetKeyboardLayoutList(dwCount, pHKLs);

    // find hKL from the list
    TCHAR szFile[32];
    for (DWORD dwi = 0; dwi < dwCount; dwi++) {
        HKL hKLTemp = pHKLs[dwi];
        ::ImmGetIMEFileName(hKLTemp, szFile, _countof(szFile));

        if (::lstrcmp(szFile, szImeFileName) == 0) { // IME�t�@�C��������v�����H
            hKL = hKLTemp;
            break;
        }
    }

    // free the list
    delete[] pHKLs;
    return hKL;
}

// Update the translate key buffer.
// ���b�Z�[�W�𐶐�����B
BOOL MzIme::GenerateMessage(LPTRANSMSG lpGeneMsg) {
    BOOL ret = FALSE;
    FOOTMARK_FORMAT("(%u,%d,%d)\n",
                    lpGeneMsg->message, lpGeneMsg->wParam, lpGeneMsg->lParam);

    if (m_lpCurTransKey)
        FOOTMARK_RETURN_INT(GenerateMessageToTransKey(lpGeneMsg));

    if (m_lpIMC && ::IsWindow(m_lpIMC->hWnd)) {
        DWORD dwNewSize = sizeof(TRANSMSG) * (m_lpIMC->NumMsgBuf() + 1);
        m_lpIMC->hMsgBuf = ImmReSizeIMCC(m_lpIMC->hMsgBuf, dwNewSize);
        if (m_lpIMC->hMsgBuf) {
            LPTRANSMSG lpTransMsg = m_lpIMC->LockMsgBuf();
            if (lpTransMsg) {
                lpTransMsg[m_lpIMC->NumMsgBuf()++] = *lpGeneMsg;
                m_lpIMC->UnlockMsgBuf();
                ret = ImmGenerateMessage(m_hIMC);
            }
        }
    }
    FOOTMARK_RETURN_INT(ret);
}

// ���b�Z�[�W�𐶐�����B
BOOL MzIme::GenerateMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    FOOTMARK_FORMAT("(%u, 0x%08lX, 0x%08lX)\n", message, wParam, lParam);
    TRANSMSG genmsg;
    genmsg.message = message;
    genmsg.wParam = wParam;
    genmsg.lParam = lParam;
    FOOTMARK_RETURN_INT(GenerateMessage(&genmsg));
}

// Update the translate key buffer.
BOOL MzIme::GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg) {
    // increment the number
    ++m_uNumTransKey;

    // check overflow
    if (m_uNumTransKey >= m_lpCurTransKey->uMsgCount) {
        m_fOverflowKey = TRUE;
        return FALSE;
    }

    // put one message to TRANSMSG buffer
    LPTRANSMSG lpgmT0 = m_lpCurTransKey->TransMsg + (m_uNumTransKey - 1);
    *lpgmT0 = *lpGeneMsg;

    return TRUE;
}

// mzimeja�̃R�}���h�����s����B
BOOL MzIme::DoCommand(HIMC hIMC, DWORD dwCommand) {
    switch (dwCommand) {
    case IDM_RECONVERT:
        break;
    case IDM_ABOUT:
        GenerateMessage(WM_IME_NOTIFY, IMN_PRIVATE, MAKELPARAM(0, 0xDEAD));
        break;
    case IDM_HIRAGANA:
        SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
        break;
    case IDM_FULL_KATAKANA:
        SetInputMode(hIMC, IMODE_FULL_KATAKANA);
        break;
    case IDM_FULL_ASCII:
        SetInputMode(hIMC, IMODE_FULL_ASCII);
        break;
    case IDM_HALF_KATAKANA:
        SetInputMode(hIMC, IMODE_HALF_KANA);
        break;
    case IDM_HALF_ASCII:
        SetInputMode(hIMC, IMODE_HALF_ASCII);
        break;
    case IDM_CANCEL:
        break;
    case IDM_ROMAN_INPUT:
        SetRomanMode(hIMC, TRUE);
        break;
    case IDM_KANA_INPUT:
        SetRomanMode(hIMC, FALSE);
        break;
    case IDM_HIDE:
        break;
    case IDM_PROPERTY:
        break;
    case IDM_ADD_WORD:
        break;
    case IDM_IME_PAD:
        GenerateMessage(WM_IME_NOTIFY, IMN_PRIVATE, MAKELPARAM(0, 0xFACE));
        break;
    default:
        return FALSE;
    }
    return TRUE;
} // MzIme::DoCommand

// �C���W�P�[�^�[�̃A�C�R�����X�V�B
void MzIme::UpdateIndicIcon(HIMC hIMC) {
    if (m_hMyKL == NULL) {
        m_hMyKL = GetHKL();
        if (m_hMyKL == NULL) return;
    }

    // TODO: enable pen icon update
    HWND hwndIndicate = ::FindWindow(INDICATOR_CLASS, NULL);
    if (::IsWindow(hwndIndicate)) {
        BOOL fOpen = FALSE;
        if (hIMC) {
            fOpen = ImmGetOpenStatus(hIMC);
        }

        ATOM atomTip = ::GlobalAddAtom(TEXT("MZ-IME Open"));
        LPARAM lParam = (LPARAM)m_hMyKL;
        ::PostMessage(hwndIndicate, INDICM_SETIMEICON, fOpen ? 1 : (-1), lParam);
        ::PostMessage(hwndIndicate, INDICM_SETIMETOOLTIPS, (fOpen ? atomTip : (-1)),
                      lParam);
        ::PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS,
                      (fOpen ? (RDMI_LEFT) : 0), lParam);
    }
}

// �E�B���h�E�N���X�̓o�^�������B
void MzIme::UnregisterClasses() {
    ::UnregisterClass(szUIServerClassName, m_hInst);
    ::UnregisterClass(szCompStrClassName, m_hInst);
    ::UnregisterClass(szCandClassName, m_hInst);
    ::UnregisterClass(szStatusClassName, m_hInst);
}

// �r�b�g�}�b�v�����\�[�X����ǂݍ��ށB
HBITMAP MzIme::LoadBMP(LPCTSTR pszName) {
    return ::LoadBitmap(m_hInst, pszName);
}

// ����������\�[�X����ǂݍ��ށB
WCHAR *MzIme::LoadSTR(INT nID) {
    static WCHAR sz[512];
    sz[0] = 0;
    ::LoadStringW(m_hInst, nID, sz, 512);
    return sz;
}

// ���̓R���e�L�X�g�����b�N����B
InputContext *MzIme::LockIMC(HIMC hIMC) {
    InputContext *context = (InputContext *)::ImmLockIMC(hIMC);
    if (context) {
        m_hIMC = hIMC;
        m_lpIMC = context;
    }
    ASSERT(context);
    return context;
}

// ���̓R���e�L�X�g�̃��b�N�������B
VOID MzIme::UnlockIMC(HIMC hIMC) {
    ::ImmUnlockIMC(hIMC);
    if (::ImmGetIMCLockCount(hIMC) == 0) {
        m_hIMC = NULL;
        m_lpIMC = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////
// UI extra related
// �]����B

HGLOBAL GetUIExtraFromServerWnd(HWND hwndServer) {
    return (HGLOBAL)GetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE);
}

void SetUIExtraToServerWnd(HWND hwndServer, HGLOBAL hUIExtra) {
    SetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE, (LONG_PTR)hUIExtra);
}

UIEXTRA *LockUIExtra(HWND hwndServer) {
    HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
    UIEXTRA *lpUIExtra = (UIEXTRA *)::GlobalLock(hUIExtra);
    ASSERT(lpUIExtra);
    return lpUIExtra;
}

void UnlockUIExtra(HWND hwndServer) {
    HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
    ::GlobalUnlock(hUIExtra);
}

void FreeUIExtra(HWND hwndServer) {
    HGLOBAL hUIExtra = GetUIExtraFromServerWnd(hwndServer);
    ::GlobalFree(hUIExtra);
    SetWindowLongPtr(hwndServer, IMMGWLP_PRIVATE, (LONG_PTR)NULL);
}

//////////////////////////////////////////////////////////////////////////////
// For debugging.
// �f�o�b�O�p�B

#ifdef MZIMEJA_DEBUG_OUTPUT
void DebugPrintA(const char *lpszFormat, ...) {
    char szMsgA[1024];

    va_list marker;
    va_start(marker, lpszFormat);
    StringCchVPrintfA(szMsgA, _countof(szMsgA), lpszFormat, marker);
    va_end(marker);

    INT cch = strlen(szMsgA);
    if (cch > 0) {
        if (szMsgA[cch - 1] == '\n') {
            szMsgA[cch - 1] = 0;
        }
        StringCchCatA(szMsgA, _countof(szMsgA), "\r\n");
    }

    WCHAR szMsgW[1024];
    szMsgW[0] = 0;
    ::MultiByteToWideChar(932, 0, szMsgA, -1, szMsgW, 1024);

    CHAR szLogFile[MAX_PATH];
    SHGetSpecialFolderPathA(NULL, szLogFile, CSIDL_DESKTOP, FALSE);
    StringCchCatA(szLogFile, _countof(szLogFile), "\\mzimeja.log");

    //OutputDebugString(szMsg);
    FILE *fp = fopen(szLogFile, "ab");
    if (fp) {
        INT len = lstrlenW(szMsgW);
        fwrite(szMsgW, len * sizeof(WCHAR), 1, fp);
        fclose(fp);
    }

    OutputDebugStringW(szMsgW);
}
void DebugPrintW(const WCHAR *lpszFormat, ...) {
    WCHAR szMsg[1024];

    va_list marker;
    va_start(marker, lpszFormat);
    StringCchVPrintfW(szMsg, _countof(szMsg), lpszFormat, marker);
    va_end(marker);

    INT cch = wcslen(szMsg);
    if (cch > 0) {
        if (szMsg[cch - 1] == L'\n') {
            szMsg[cch - 1] = 0;
        }
        StringCchCatW(szMsg, _countof(szMsg), L"\r\n");
    }

    CHAR szLogFile[MAX_PATH];
    SHGetSpecialFolderPathA(NULL, szLogFile, CSIDL_DESKTOP, FALSE);
    StringCchCatA(szLogFile, _countof(szLogFile), "\\mzimeja.log");

    FILE *fp = fopen(szLogFile, "ab");
    if (fp) {
        INT len = lstrlenW(szMsg);
        fwrite(szMsg, len * sizeof(WCHAR), 1, fp);
        fclose(fp);
    }

    OutputDebugStringW(szMsg);
}

void DebugAssert(const char *file, int line, const char *exp) {
    DebugPrintA("%s (%d): ASSERT(%s) failed\n", file, line, exp);
}
#endif  // def MZIMEJA_DEBUG_OUTPUT

//////////////////////////////////////////////////////////////////////////////
// DLL entry point

// IME��DLL�t�@�C���̈��ł��邩��AIME���ǂݍ��܂ꂽ��A�G���g���[�|�C���g��
// DllMain���Ăяo�����͂��B
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwFunction, LPVOID lpNot) {
    FOOTMARK_FORMAT("(%p, 0x%08lX, %p)\n", hInstDLL, dwFunction, lpNot);

    switch (dwFunction) {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hInstDLL);
        TheIME.Init(hInstDLL); // �������B
        break;

    case DLL_PROCESS_DETACH:
        TheIME.Uninit(); // �t�������B
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    FOOTMARK_RETURN_INT(TRUE);
} // DllMain

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
