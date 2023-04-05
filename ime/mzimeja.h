// mzimeja.h --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)
// NOTE: This file uses Japanese cp932 encoding for historic reason.

#pragma once

#ifndef NDEBUG
    #error mzimeja is for Release only. Not for Debug. See HowToBuild.txt.
#endif

#define _CRT_SECURE_NO_WARNINGS   // use fopen

#include "../targetver.h"   // target Windows version

#ifndef _INC_WINDOWS
    #include <windows.h>    // Windows
#endif
#include <tchar.h>          // for Windows generic text
#include <shlwapi.h>        // Shell Light-Weight API
#include <strsafe.h>        // for StringC... functions

#include <string>           // for std::string, std::wstring, ...
#include <vector>           // for std::vector
#include <unordered_set>    // for std::unordered_set
#include <unordered_map>    // for std::unordered_map

#include "indicml.h"        // for system indicator
#include "immdev.h"         // for IME/IMM development
#include "input.h"          // for INPUT_MODE and InputContext

#include "../dict.hpp"      // for dictionary
#include "../str.hpp"       // for str_*

//////////////////////////////////////////////////////////////////////////////
// _countof macro --- get the number of elements in an array

#ifndef _countof
  #define _countof(array)   (sizeof(array) / sizeof(array[0]))
#endif

//////////////////////////////////////////////////////////////////////////////
// For debugging.
// �f�o�b�O�p�B

#ifndef MZIMEJA_DEBUG_OUTPUT
  #define DebugPrintA   (void)
  #define DebugPrintW   (void)
  #define DebugPrint    (void)
  #define DPRINT(fmt, ...)
  #define ASSERT(exp)
  #define TRACE_ON()
  #define TRACE_OFF()
#else
  extern "C" {
    extern BOOL g_bTrace;
    void DebugPrintA(const char *lpszFormat, ...);
    void DebugPrintW(const WCHAR *lpszFormat, ...);
    void DebugAssert(const char *file, int line, const char *exp);
  } // extern "C"
  #define DebugPrintA DebugPrintA
  #define DebugPrintW DebugPrintW
  #define DPRINT(fmt, ...) DebugPrintA("%s (%d): " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
  #define ASSERT(exp) ((exp) ? (void)0 : DebugAssert(__FILE__, __LINE__, #exp))
  #ifdef UNICODE
    #define DebugPrint DebugPrintW
  #else
    #define DebugPrint DebugPrintA
  #endif
  #define TRACE_ON()    do { g_bTrace = TRUE; } while (0)
  #define TRACE_OFF()   do { g_bTrace = FALSE; } while (0)
#endif

#include "footmark.hpp"   // for footmark++

//////////////////////////////////////////////////////////////////////////////

// For limit of MZ-IME.
// MZ-IME�̐����B
#define MAXCOMPWND  10  // maximum number of composition windows
#define MAXGLCHAR   32  // maximum number of guideline characters

// Special messages.
// ���ʂȃ��b�Z�[�W�B
#define WM_UI_UPDATE      (WM_USER + 500)
#define WM_UI_HIDE        (WM_USER + 501)
#define WM_UI_STATEMOVE   (WM_USER + 601)
#define WM_UI_DEFCOMPMOVE (WM_USER + 602)
#define WM_UI_CANDMOVE    (WM_USER + 603)
#define WM_UI_GUIDEMOVE   (WM_USER + 604)

// Escape Functions
#define IME_ESC_PRI_GETDWORDTEST (IME_ESC_PRIVATE_FIRST + 0)

// Special style.
// ���ʂȃE�B���h�E�X�^�C���B
#define WS_COMPDEFAULT    (WS_DISABLED | WS_POPUP)
#define WS_COMPNODEFAULT  (WS_DISABLED | WS_POPUP)

// The IDs of guideline entry.
// �K�C�h���C���G���g���[��ID�B
#define MYGL_NODICTIONARY   0
#define MYGL_TYPINGERROR    1
#define MYGL_TESTGUIDELINE  2

// window extra of child UI windows
#define FIGWL_MOUSE 0
#define FIGWLP_SERVERWND        (FIGWL_MOUSE + sizeof(LONG))
#define FIGWLP_FONT             (FIGWLP_SERVERWND + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTSTR      (FIGWLP_FONT + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTNUM      (FIGWL_COMPSTARTSTR + sizeof(LONG))
#define FIGWLP_STATUSBMP        (FIGWL_COMPSTARTNUM + sizeof(LONG))
#define FIGWLP_CLOSEBMP         (FIGWLP_STATUSBMP + sizeof(LONG_PTR))
#define FIGWL_PUSHSTATUS        (FIGWLP_CLOSEBMP + sizeof(LONG_PTR))
#define FIGWLP_CHILDWND         (FIGWL_PUSHSTATUS + sizeof(LONG))
#define UIEXTRASIZE             (FIGWLP_CHILDWND + sizeof(LONG_PTR))

// The flags of FIGWL_MOUSE.
// FIGWL_MOUSE�̃t���O�B
#define FIM_CAPUTURED 0x01
#define FIM_MOVED 0x02

// The flags of the close button.
// ����{�^���̃t���O�B
#define PUSHED_STATUS_CLOSE 0x08

// Statue Close Button.
#define STCLBT_X  (BTX * 2 + 3)
#define STCLBT_Y  1
#define STCLBT_DX 12
#define STCLBT_DY 12

// The style flags of ImeRegisterWord
#define MZIME_REGWORD_STYLE (IME_REGWORD_STYLE_USER_FIRST | 0x40000000)

static inline HinshiBunrui StyleToHinshi(DWORD dwStyle) {
    ASSERT((dwStyle & MZIME_REGWORD_STYLE) == MZIME_REGWORD_STYLE);
    HinshiBunrui ret = (HinshiBunrui)((dwStyle & ~MZIME_REGWORD_STYLE) + HB_MEISHI);
    ASSERT(HB_MEISHI <= ret && ret <= HB_MAX);
    return ret;
}

static inline DWORD HinshiToStyle(HinshiBunrui hinshi) {
    ASSERT(HB_MEISHI <= hinshi && hinshi <= HB_MAX);
    return ((DWORD)hinshi - HB_MEISHI) | MZIME_REGWORD_STYLE;
}

//////////////////////////////////////////////////////////////////////////////
// additional GCS bits

#define GCS_COMPALL \
        (GCS_COMPSTR | GCS_COMPATTR | GCS_COMPREADSTR | GCS_COMPREADATTR | \
         GCS_COMPCLAUSE | GCS_COMPREADCLAUSE)

#define GCS_RESULTALL \
        (GCS_RESULTSTR | GCS_RESULTREADSTR | GCS_RESULTCLAUSE | GCS_RESULTREADCLAUSE)

//////////////////////////////////////////////////////////////////////////////
// Structures

// UIEXTRA
struct UIEXTRA {
    HIMC hIMC;
    HWND hwndStatus;
    HWND hwndCand;
    POINT ptCand;
    DWORD dwCompStyle;
    HFONT hFont;
    BOOL bVertical;
    HWND hwndDefComp;
    HWND hwndComp[MAXCOMPWND];
    RECT rcComp[MAXCOMPWND];
    HWND hwndGuide;
};

// MZGUIDELINE
struct MZGUIDELINE {
    DWORD dwLevel;
    DWORD dwIndex;
    DWORD dwStrID;
    DWORD dwPrivateID;
};

//////////////////////////////////////////////////////////////////////////////

// �E�B���h�E�N���X���B
extern const WCHAR szUIServerClassName[];   // UI�T�[�o�[�B
extern const WCHAR szCompStrClassName[];    // ���m�蕶����B
extern const WCHAR szCandClassName[];       // ���E�B���h�E�B
extern const WCHAR szStatusClassName[];     // ��ԃE�B���h�E�B
extern const WCHAR szGuideClassName[];      // �K�C�h���C���E�B���h�E�B

// �K�C�h���C���e�[�u���B
extern const MZGUIDELINE glTable[];

//////////////////////////////////////////////////////////////////////////////

extern "C" {

// convert.c
extern std::unordered_map<wchar_t,wchar_t>   g_vowel_map;      // �ꉹ�ʑ��B
extern std::unordered_map<wchar_t,wchar_t>   g_consonant_map;  // �q���ʑ��B
void MakeLiteralMaps(); // �q���̎ʑ��ƕꉹ�̎ʑ����쐬����B
WCHAR ConvertCommaPeriod(WCHAR ch); // ��Ǔ_��ϊ�����B
LPCWSTR BunruiToString(HinshiBunrui bunrui);

// notify.c
BOOL PASCAL NotifyUCSetOpen(HIMC hIMC);
BOOL PASCAL NotifyUCConversionMode(HIMC hIMC);
BOOL PASCAL NotifyUCSetCompositionWindow(HIMC hIMC);

// ui.c
LRESULT CALLBACK MZIMEWndProc(HWND, UINT, WPARAM, LPARAM);
LONG NotifyCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam);
LONG ControlCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam);
void DrawUIBorder(LPRECT lprc);
void DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int GetCompFontHeight(UIEXTRA *lpUIExtra);
BOOL IsImeMessage(UINT message);
BOOL IsImeMessage2(UINT message);

// uistate.c
HWND StatusWnd_Create(HWND hWnd, UIEXTRA *lpUIExtra);
LRESULT CALLBACK StatusWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void StatusWnd_Update(UIEXTRA *lpUIExtra);

// uicand.c
LRESULT CALLBACK CandWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void CandWnd_Paint(HWND hCandWnd);
void CandWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CandWnd_Resize(UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CandWnd_Hide(UIEXTRA *lpUIExtra);
void CandWnd_Move(HWND hUIWnd, InputContext *lpIMC, UIEXTRA *lpUIExtra,
                  BOOL fForceComp);

// uicomp.c
LRESULT CALLBACK CompWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void CompWnd_Paint(HWND hCompWnd);
void CompWnd_Create(HWND hUIWnd, UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CompWnd_Move(UIEXTRA *lpUIExtra, InputContext *lpIMC);
void CompWnd_Hide(UIEXTRA *lpUIExtra);
void CompWnd_SetFont(UIEXTRA *lpUIExtra);
HWND GetCandPosHintFromComp(UIEXTRA *lpUIExtra, InputContext *lpIMC,
                            DWORD iClause, LPPOINT ppt);

// uiguide.c
LRESULT CALLBACK GuideWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void GuideWnd_Paint(HWND hGuideWnd, HDC hDC, LPPOINT lppt, DWORD dwPushedGuide);
void GuideWnd_Button(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void GuideWnd_Update(UIEXTRA *lpUIExtra);
LRESULT CALLBACK LineWndProc(HWND, UINT, WPARAM, LPARAM);

// config.c
LPCTSTR HinshiToString(HinshiBunrui hinshi);
HinshiBunrui StringToHinshi(LPCTSTR str);
HKEY Config_OpenAppKey(VOID);
HKEY Config_CreateAppKey(VOID);
DWORD Config_GetDWORD(LPCTSTR name, DWORD dwDefault);
BOOL Config_SetDWORD(LPCTSTR name, DWORD dwValue);
INT_PTR CALLBACK WordListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RegWordDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ChooseDictDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DebugOptionDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// immsec.cpp
SECURITY_ATTRIBUTES *CreateSecurityAttributes(void);
void FreeSecurityAttributes(SECURITY_ATTRIBUTES *psa);
BOOL IsNT(void);

// mzimeja.cpp
void      RepositionWindow(HWND hWnd);
HFONT     CheckNativeCharset(HDC hDC);
HGLOBAL   GetUIExtraFromServerWnd(HWND hwndServer);
void      SetUIExtraToServerWnd(HWND hwndServer, HGLOBAL hUIExtra);
UIEXTRA * LockUIExtra(HWND hwndServer);
void      UnlockUIExtra(HWND hwndServer);
void      FreeUIExtra(HWND hwndServer);

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
// keychar.cpp

// conversion between roman and hiragana
std::wstring hiragana_to_roman(std::wstring hiragana);
std::wstring roman_to_hiragana(std::wstring roman);
std::wstring roman_to_hiragana(std::wstring roman, size_t ichTarget);
// conversion between roman and katakana
std::wstring roman_to_katakana(std::wstring roman);
std::wstring roman_to_katakana(std::wstring roman, size_t ichTarget);
// conversion between roman and halfwidth katakana
std::wstring roman_to_halfwidth_katakana(std::wstring roman);
std::wstring roman_to_halfwidth_katakana(std::wstring roman, size_t ichTarget);

// character map for kana input
WCHAR vkey_to_hiragana(BYTE vk, BOOL bShift);
// character map for typing keys
WCHAR typing_key_to_char(BYTE vk, BOOL bShift, BOOL bCapsLock);
// dakuon (voiced consonant) processor
WCHAR dakuon_shori(WCHAR ch0, WCHAR ch1);
// locale-dependent string conversion
std::wstring lcmap(const std::wstring& str, DWORD dwFlags);
// convert hiragana to typing characters
std::wstring hiragana_to_typing(std::wstring hiragana);
// convert fullwidth ascii to halfwidth
std::wstring fullwidth_ascii_to_halfwidth(const std::wstring& str);

// is the character hiragana?
BOOL is_hiragana(WCHAR ch);
// is the character fullwidth katakana?
BOOL is_fullwidth_katakana(WCHAR ch);
// is the character halfwidth katakana?
BOOL is_halfwidth_katakana(WCHAR ch);
// is the character kanji?
BOOL is_kanji(WCHAR ch);
// is the character the education kanji?
BOOL is_education_kanji(WCHAR ch);
// is the character the common use kanji?
BOOL is_common_use_kanji(WCHAR ch);
// is the character fullwidth ASCII?
BOOL is_fullwidth_ascii(WCHAR ch);
// are all the characters numeric?
BOOL are_all_chars_numeric(const std::wstring& str);
// convert to kansuuji (Kanji number)
std::wstring convert_to_kansuuji_1(wchar_t ch, size_t digit_level);
std::wstring convert_to_kansuuji_4(const std::wstring& halfwidth);
std::wstring convert_to_kansuuji(const std::wstring& str);
std::wstring convert_to_kansuuji_brief(const std::wstring& str);
std::wstring convert_to_kansuuji_formal(const std::wstring& str);
// is the character a period?
BOOL is_period(WCHAR ch);
// is the character a comma?
BOOL is_comma(WCHAR ch);

//////////////////////////////////////////////////////////////////////////////

// �ϊ����B
struct MzConvCandidate {
    std::wstring hiragana;              // �Ђ炪�ȁB
    std::wstring converted;             // �ϊ���B
    int cost;                           // �R�X�g�B
    std::unordered_set<HinshiBunrui>  bunruis;    // �i�����ޏW���B
    std::wstring tags;                  // �^�O�B

    void clear() {
        hiragana.clear();
        converted.clear();
        cost = 0;
        bunruis.clear();
        tags.clear();
    }
};

// �ϊ����߁B
struct MzConvClause {
    std::vector<MzConvCandidate> candidates;    // ���Q�B
    void sort();                                // �\�[�g����B
    void add(const LatticeNode *node);          // �m�[�h��ǉ�����B

    void clear() {
        candidates.clear();
    }
};

// �ϊ����ʁB
struct MzConvResult {
    std::vector<MzConvClause> clauses;      // ���ߌQ�B
    void sort();                            // �\�[�g����B
    void clear() { clauses.clear(); }       // �N���A����B
};

//////////////////////////////////////////////////////////////////////////////
// dictionary - ����

class Dict {
public:
    Dict();
    ~Dict();

    // ������ǂݍ��ށB
    BOOL Load(const wchar_t *file_name, const wchar_t *object_name);
    // �������A�����[�h����B
    void Unload();

    BOOL IsLoaded() const;  // �ǂݍ��ݍς݂��H
    DWORD GetSize() const;  // �T�C�Y���擾����B

    wchar_t *Lock();            // ���b�N���ēǂݍ��݂��J�n����B
    void Unlock(wchar_t *data); // ���b�N���������ēǂݍ��݂��I������B

protected:
    std::wstring m_strFileName;     // �t�@�C�����B
    std::wstring m_strObjectName;   // �����̎������g���̂ŁA�I�u�W�F�N�g���ŋ�ʂ���B
    HANDLE m_hMutex;                // �r������p�B
    HANDLE m_hFileMapping;          // �t�@�C���}�b�s���O�B
};

//////////////////////////////////////////////////////////////////////////////
// MZ-IME

class MzIme {
    public:
    HINSTANCE m_hInst;
    HKL m_hMyKL;
    BOOL m_bWinLogOn;
    LPTRANSMSGLIST m_lpCurTransKey;
    UINT m_uNumTransKey;
    BOOL m_fOverflowKey;

public:
    MzIme();

    // �������B
    BOOL Init(HINSTANCE hInstance);
    // �E�B���h�E�N���X�̓o�^�B
    BOOL RegisterClasses(HINSTANCE hInstance);
    void UnregisterClasses();
    // �t�������B
    VOID Uninit(VOID);

    // ���\�[�X����r�b�g�}�b�v��ǂݍ��ށB
    HBITMAP LoadBMP(LPCTSTR pszName);
    HBITMAP LoadBMP(UINT nID) { return LoadBMP(MAKEINTRESOURCE(nID)); }
    // ���\�[�X���當�����ǂݍ��ށB
    WCHAR *LoadSTR(INT nID);

    void UpdateIndicIcon(HIMC hIMC);    // �C���W�P�[�^�[�A�C�R�����X�V����B

    HKL GetHKL(VOID);                   // �L�[�{�[�h���C�A�E�g�̃n���h�����擾����B

    InputContext *LockIMC(HIMC hIMC);   // ���̓R���e�L�X�g�����b�N����B
    VOID UnlockIMC(HIMC hIMC);          // ���̓R���e�L�X�g�̃��b�N����������B

    // ���b�Z�[�W�̐����B
    BOOL GenerateMessage(LPTRANSMSG lpGeneMsg);
    BOOL GenerateMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
    BOOL GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg);

    BOOL DoCommand(HIMC hIMC, DWORD dwCommand);     // �R�}���h�����s����B

    // ��{�����B
    BOOL LoadBasicDict();                   // ��{������ǂݍ��ށB
    BOOL IsBasicDictLoaded() const;         // ��{�������ǂݍ��܂ꂽ���H
    WCHAR *LockBasicDict();                 // ��{���������b�N����B
    void UnlockBasicDict(WCHAR *data);      // ��{�����̃��b�N����������B

    // make lattice
    BOOL MakeLatticeForMulti(Lattice& lattice, const std::wstring& pre);
    BOOL MakeLatticeForSingle(Lattice& lattice, const std::wstring& pre);
    void MakeResultOnFailure(MzConvResult& result, const std::wstring& pre);
    void MakeResultForMulti(MzConvResult& result, Lattice& lattice);
    void MakeResultForSingle(MzConvResult& result, Lattice& lattice);
    int CalcCost(const std::wstring& tags) const;

    // convert
    BOOL ConvertMultiClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL ConvertMultiClause(const std::wstring& strHiragana,
                            MzConvResult& result);
    BOOL ConvertSingleClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL ConvertSingleClause(const std::wstring& strHiragana,
                             MzConvResult& result);
    BOOL StretchClauseLeft(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL StretchClauseRight(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
    BOOL ConvertCode(const std::wstring& strTyping, MzConvResult& result);
    BOOL ConvertCode(LogCompStr& comp, LogCandInfo& cand);
    BOOL StoreResult(
            const MzConvResult& result, LogCompStr& comp, LogCandInfo& cand);

    // ���[�U�[���̐ݒ�B
    BOOL GetUserString(LPCWSTR pszSettingName, std::wstring& value);
    BOOL SetUserString(LPCWSTR pszSettingName, LPCWSTR pszValue);
    BOOL GetUserData(LPCWSTR pszSettingName, void *ptr, DWORD size);
    BOOL SetUserData(LPCWSTR pszSettingName, const void *ptr, DWORD size);

protected:
    // input context
    HIMC m_hIMC;
    InputContext *  m_lpIMC;

    // dictionary
    Dict m_basic_dict;
    Dict m_name_dict;
    BOOL LoadDict();
    void UnloadDict();

    // registry
    LONG OpenRegKey(HKEY hKey, LPCWSTR pszSubKey, BOOL bWrite, HKEY *phSubKey) const;
    LONG CreateRegKey(HKEY hKey, LPCWSTR pszSubKey, HKEY *phSubKey);
    LONG OpenComputerSettingKey(BOOL bWrite, HKEY *phKey);
    LONG OpenUserSettingKey(BOOL bWrite, HKEY *phKey);
}; // class MzIme

extern MzIme TheIME;

// postal.cpp
std::wstring postal_code(LPCWSTR code);

//////////////////////////////////////////////////////////////////////////////
