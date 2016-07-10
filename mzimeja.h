// mzimeja.h --- MZ-IME Japanese Input (mzimeja)
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)

#ifndef MZIMEJA_H_
#define MZIMEJA_H_

#define _CRT_SECURE_NO_WARNINGS

#ifndef _INC_WINDOWS
  #include <windows.h>      // Windows
#endif
#include <tchar.h>          // for Windows generic text

#include <string>           // for std::string, std::wstring, ...
#include <vector>           // for std::vector
#include <set>              // for std::set
#include <map>              // for std::map
#include <algorithm>        // for std::sort

#include <cstdio>           // for C standard I/O
#include <cctype>           // for C character types
#include <cassert>          // for assert
#include <cstring>          // for C string

#include "indicml.h"        // for system indicator
#include "immdev.h"         // for IME/IMM development
#include "immsec.h"         // for IMM security
#include "input.h"          // for INPUT_MODE and InputContext

#define UNBOOST_USE_STRING_ALGORITHM
#define UNBOOST_USE_UNORDERED_MAP
#include "unboost.hpp"      // Unboost

//////////////////////////////////////////////////////////////////////////////
// _countof macro --- get the number of elements in an array

#ifndef _countof
  #define _countof(array)   (sizeof(array) / sizeof(array[0]))
#endif

//////////////////////////////////////////////////////////////////////////////
// for debugging

#ifdef NDEBUG
  #define DebugPrintA   (void)
  #define DebugPrintW   (void)
  #define DebugPrint    (void)
#else
  extern "C" {
    int DebugPrintA(const char *lpszFormat, ...);
    int DebugPrintW(const WCHAR *lpszFormat, ...);
  } // extern "C"
  #define DebugPrintA DebugPrintA
  #define DebugPrintW DebugPrintW
  #ifdef UNICODE
    #define DebugPrint DebugPrintW
  #else
    #define DebugPrint DebugPrintA
  #endif
#endif

#include "footmark.hpp"   // for FOOTMARK and FOOTMARK_PRINT_CALL_STACK

//////////////////////////////////////////////////////////////////////////////

// for limit of MZ-IME
#define MAXCOMPWND  10  // maximum number of composition windows
#define MAXGLCHAR   32  // maximum number of guideline characters

// special messages
#define WM_UI_UPDATE      (WM_USER + 500)
#define WM_UI_HIDE        (WM_USER + 501)

#define WM_UI_STATEMOVE   (WM_USER + 601)
#define WM_UI_DEFCOMPMOVE (WM_USER + 602)
#define WM_UI_CANDMOVE    (WM_USER + 603)
#define WM_UI_GUIDEMOVE   (WM_USER + 604)

// Escape Functions
#define IME_ESC_PRI_GETDWORDTEST (IME_ESC_PRIVATE_FIRST + 0)

// special style
#define WS_COMPDEFAULT    (WS_DISABLED | WS_POPUP)
#define WS_COMPNODEFAULT  (WS_DISABLED | WS_POPUP)

// ID of guideline table
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

// The flags of FIGWL_MOUSE
#define FIM_CAPUTURED 0x01
#define FIM_MOVED 0x02

// The flags of the close button
#define PUSHED_STATUS_CLOSE 0x08

// Statue Close Button
#define STCLBT_X  (BTX * 2 + 3)
#define STCLBT_Y  1
#define STCLBT_DX 12
#define STCLBT_DY 12

//////////////////////////////////////////////////////////////////////////////
// additional GCS bits

#define GCS_COMPALL                                                  \
  (GCS_COMPSTR | GCS_COMPATTR | GCS_COMPREADSTR | GCS_COMPREADATTR | \
   GCS_COMPCLAUSE | GCS_COMPREADCLAUSE)

#define GCS_RESULTALL \
  (GCS_RESULTSTR | GCS_RESULTREADSTR | GCS_RESULTCLAUSE | GCS_RESULTREADCLAUSE)

//////////////////////////////////////////////////////////////////////////////
// Structures

// UICHILD
struct UICHILD {
  HWND hWnd;
  BOOL bShow;
  POINT pt;
};

// UICHILD2
struct UICHILD2 {
  HWND hWnd;
  BOOL bShow;
  RECT rc;
};

// UIEXTRA
struct UIEXTRA {
  HIMC      hIMC;
  UICHILD   uiStatus;
  UICHILD   uiCand;
  DWORD     dwCompStyle;
  HFONT     hFont;
  BOOL      bVertical;
  UICHILD   uiDefComp;
  UICHILD2  uiComp[MAXCOMPWND];
  UICHILD   uiGuide;
};

// MZGUIDELINE
struct MZGUIDELINE {
  DWORD dwLevel;
  DWORD dwIndex;
  DWORD dwStrID;
  DWORD dwPrivateID;
};

//////////////////////////////////////////////////////////////////////////////
// externs

extern const TCHAR szUIServerClassName[];
extern const TCHAR szCompStrClassName[];
extern const TCHAR szCandClassName[];
extern const TCHAR szStatusClassName[];
extern const TCHAR szGuideClassName[];
extern const MZGUIDELINE glTable[];

//////////////////////////////////////////////////////////////////////////////

extern "C" {

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
INT_PTR CALLBACK RegWordDlgProc(HWND hDlg, UINT message, WPARAM wParam,
                                LPARAM lParam);
INT_PTR CALLBACK SelectDictionaryDlgProc(HWND hDlg, UINT message, WPARAM wParam,
                                         LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam,
                              LPARAM lParam);
INT_PTR CALLBACK GeneralDlgProc(HWND hDlg, UINT message, WPARAM wParam,
                                LPARAM lParam);
INT_PTR CALLBACK DebugOptionDlgProc(HWND hDlg, UINT message, WPARAM wParam,
                                    LPARAM lParam);

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

//////////////////////////////////////////////////////////////////////////////

struct MzConversionCandidate {
  std::wstring hiragana;
  std::wstring converted;
  void clear() {
    hiragana.clear();
    converted.clear();
  }
};

struct MzConversionClause {
  std::vector<MzConversionCandidate> candidates;
  void clear() { candidates.clear(); }
};

struct MzConversionResult {
  std::vector<MzConversionClause> clauses;
  void clear() { clauses.clear(); }
};

//////////////////////////////////////////////////////////////////////////////

struct ImeBaseData {
  DWORD   dwSignature;
  DWORD   dwSharedDictDataSize;
};

enum HINSHI_BUNRUI {
  HB_START_NODE,        // �J�n�m�[�h
  HB_MEISHI,            // ����
  HB_IKEIYOUSHI,        // ���`�e��
  HB_NAKEIYOUSHI,       // �Ȍ`�e��
  HB_RENTAISHI,         // �A�̎�
  HB_FUKUSHI,           // ����
  HB_SETSUZOKUSHI,      // �ڑ���
  HB_KANDOUSHI,         // ������
  HB_JOSHI,             // ����
  HB_MIZEN_JODOUSHI,    // ���R������
  HB_RENYOU_JODOUSHI,   // �A�p������
  HB_SHUUSHI_JODOUSHI,  // �I�~������
  HB_RENTAI_JODOUSHI,   // �A�̏�����
  HB_KATEI_JODOUSHI,    // ���菕����
  HB_MEIREI_JODOUSHI,   // ���ߏ�����
  HB_GODAN_DOUSHI,      // �ܒi����
  HB_ICHIDAN_DOUSHI,    // ��i����
  HB_KAHEN_DOUSHI,      // �J�ϓ���
  HB_SAHEN_DOUSHI,      // �T�ϓ���
  HB_KANGO,             // ����
  HB_SETTOUGO,          // �ړ���
  HB_SETSUBIGO,         // �ڔ���
  HB_END_NODE           // �I���m�[�h
};

struct DICT_ENTRY {
  std::wstring  pre;
  HINSHI_BUNRUI bunrui;
  std::wstring  post;
  std::wstring  tags;
};

const wchar_t g_table[][5] = {
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', 0, L'��', 0, L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', 0, 0, 0, L'��'},
  {L'��', 0, 0, 0, 0},
};

//////////////////////////////////////////////////////////////////////////////

// The IME
class MZIMEJA {
public:
  HINSTANCE       m_hInst;
  HKL             m_hMyKL;
  BOOL            m_bWinLogOn;
  LPTRANSMSGLIST  m_lpCurTransKey;
  UINT            m_uNumTransKey;
  BOOL            m_fOverflowKey;

public:
  MZIMEJA();

  // initialize the IME
  BOOL Init(HINSTANCE hInstance);

  // register classes
  BOOL RegisterClasses(HINSTANCE hInstance);

  // uninitialize
  VOID Uninit(VOID);

  // load a bitmap from resource
  HBITMAP LoadBMP(LPCTSTR pszName);
  HBITMAP LoadBMP(UINT nID) {
    return LoadBMP(MAKEINTRESOURCE(nID));
  }

  // load a string from resource
  WCHAR *LoadSTR(INT nID);

  // update the indicator icon
  void UpdateIndicIcon(HIMC hIMC);

  // get the keyboard layout handle
  HKL GetHKL(VOID);

  // lock the input context
  InputContext *LockIMC(HIMC hIMC);
  // unlock the input context
  VOID UnlockIMC(HIMC hIMC);

  // generate a message
  BOOL GenerateMessage(LPTRANSMSG lpGeneMsg);
  BOOL GenerateMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
  BOOL GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg);

  // do command
  BOOL DoCommand(HIMC hIMC, DWORD dwCommand);

  // basic dictionary
  BOOL LoadBasicDict();
  BOOL IsBasicDictLoaded() const;
  WCHAR *LockBasicDict();
  void UnlockBasicDict(WCHAR *data);

  // convert
  void PluralClauseConversion(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
  void PluralClauseConversion(const std::wstring& strHiragana,
                              MzConversionResult& result);
  void SingleClauseConversion(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
  void SingleClauseConversion(const std::wstring& strHiragana,
                              MzConversionClause& result);
  BOOL StretchClauseLeft(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);
  BOOL StretchClauseRight(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman);

  // settings
  std::wstring GetSettingString(LPCWSTR szSettingName) const;

protected:
  HANDLE          m_hMutex;         // mutex
  HANDLE          m_hBaseData;      // file mapping
  HIMC            m_hIMC;
  InputContext *  m_lpIMC;
  HANDLE          m_hBasicDictData; // file mapping
  unboost::unordered_map<wchar_t,wchar_t>   m_vowel_map;
  unboost::unordered_map<wchar_t,wchar_t>   m_consonant_map;

  ImeBaseData *LockImeBaseData();
  void UnlockImeBaseData(ImeBaseData *data);
  void MakeMaps();
  BOOL LoadBasicDictFile(std::vector<DICT_ENTRY>& entries);
  BOOL DeployDictData(ImeBaseData *data, SECURITY_ATTRIBUTES *psa,
                      const std::vector<DICT_ENTRY>& entries);
}; // class MZIMEJA

extern MZIMEJA TheIME;

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef MZIMEJA_H_

//////////////////////////////////////////////////////////////////////////////
