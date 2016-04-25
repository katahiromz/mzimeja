// mzimeja.h --- MZ-IME Japanese Input
//////////////////////////////////////////////////////////////////////////////

#ifndef MZIMEJA_H_
#define MZIMEJA_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#include <tchar.h>

#include <cstdio>
#include <cassert>
#include <cstring>

#include "indicml.h"        // for system indicator
#include "immdev.h"         // for IME/IMM development
#include "input_context.h"  // for InputContext
#include "comp_str.h"       // for LogCompStr and CompStr
#include "cand_info.h"      // for MZCAND, CandList, CandInfo

//////////////////////////////////////////////////////////////////////////////
// _countof macro --- get the number of elements in an array

#ifndef _countof
  #define _countof(array)   (sizeof(array) / sizeof(array[0]))
#endif

//////////////////////////////////////////////////////////////////////////////
// for debugging

#ifdef NDEBUG
  #define DebugPrintA (void)
  #define DebugPrintW (void)
  #define DebugPrint  (void)
#else
  extern "C" {
    int DebugPrintA(LPCSTR lpszFormat, ...);
    int DebugPrintW(LPCWSTR lpszFormat, ...);
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

#define MZIME_FILENAME    TEXT("mzimeja.ime")

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

// WndExtra of child UI windows
#define FIGWL_MOUSE 0
#define FIGWLP_SERVERWND    (FIGWL_MOUSE + sizeof(LONG))
#define FIGWLP_FONT         (FIGWLP_SERVERWND + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTSTR  (FIGWLP_FONT + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTNUM  (FIGWL_COMPSTARTSTR + sizeof(LONG))
#define FIGWLP_STATUSBMP    (FIGWL_COMPSTARTNUM + sizeof(LONG))
#define FIGWLP_CLOSEBMP     (FIGWLP_STATUSBMP + sizeof(LONG_PTR))
#define FIGWL_PUSHSTATUS    (FIGWLP_CLOSEBMP + sizeof(LONG_PTR))
#define FIGWLP_CHILDWND     (FIGWL_PUSHSTATUS + sizeof(LONG))
#define UIEXTRASIZE         (FIGWLP_CHILDWND + sizeof(LONG_PTR))

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
// input modes

enum InputMode {
  IMODE_ZEN_HIRAGANA,
  IMODE_ZEN_KATAKANA,
  IMODE_ZEN_EISUU,
  IMODE_HAN_KANA,
  IMODE_HAN_EISUU
};

//////////////////////////////////////////////////////////////////////////////
// Structures

// UICHILD
typedef struct {
  HWND hWnd;
  BOOL bShow;
  POINT pt;
} UICHILD, NEAR *PUICHILD, FAR *LPUICHILD;

// UICHILD2
typedef struct {
  HWND hWnd;
  BOOL bShow;
  RECT rc;
} UICHILD2, NEAR *PUICHILD2, FAR *LPUICHILD2;

// UIEXTRA
typedef struct {
  HIMC      hIMC;
  UICHILD   uiStatus;
  UICHILD   uiCand;
  DWORD     dwCompStyle;
  HFONT     hFont;
  BOOL      bVertical;
  UICHILD   uiDefComp;
  UICHILD2  uiComp[MAXCOMPWND];
  UICHILD   uiGuide;
} UIEXTRA,  NEAR *PUIEXTRA, FAR *LPUIEXTRA;

// MZGUIDELINE
typedef struct {
  DWORD dwLevel;
  DWORD dwIndex;
  DWORD dwStrID;
  DWORD dwPrivateID;
} MZGUIDELINE, NEAR *PMZGUIDELINE, FAR *LPMZGUIDELINE;

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
LONG NotifyCommand(HIMC hIMC, HWND hWnd, UINT message, WPARAM wParam,
                   LPARAM lParam);
LONG ControlCommand(HIMC hIMC, HWND hWnd, UINT message, WPARAM wParam,
                    LPARAM lParam);
void DrawUIBorder(LPRECT lprc);
void DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int GetCompFontHeight(LPUIEXTRA lpUIExtra);
BOOL IsImeMessage(UINT message);

// uistate.c
HWND StatusWnd_Create(HWND hWnd, LPUIEXTRA lpUIExtra);
LRESULT CALLBACK StatusWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void StatusWnd_Update(LPUIEXTRA lpUIExtra);

// uicand.c
LRESULT CALLBACK CandWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void CandWnd_Paint(HWND hCandWnd);
void CandWnd_Create(HWND hUIWnd, LPUIEXTRA lpUIExtra, InputContext *lpIMC);
void CandWnd_Resize(LPUIEXTRA lpUIExtra, InputContext *lpIMC);
void CandWnd_Hide(LPUIEXTRA lpUIExtra);
void CandWnd_Move(HWND hUIWnd, InputContext *lpIMC, LPUIEXTRA lpUIExtra,
                  BOOL fForceComp);

// uicomp.c
LRESULT CALLBACK CompWnd_WindowProc(HWND, UINT, WPARAM, LPARAM);
void CompWnd_Paint(HWND hCompWnd);
void CompWnd_Create(HWND hUIWnd, LPUIEXTRA lpUIExtra, InputContext *lpIMC);
void CompWnd_Move(LPUIEXTRA lpUIExtra, InputContext *lpIMC);
void CompWnd_Hide(LPUIEXTRA lpUIExtra);
void CompWnd_SetFont(LPUIEXTRA lpUIExtra);

// uiguide.c
LRESULT CALLBACK GuideWndProc(HWND, UINT, WPARAM, LPARAM);
void PASCAL PaintGuide(HWND hGuideWnd, HDC hDC, LPPOINT lppt,
                       DWORD dwPushedGuide);
void PASCAL ButtonGuide(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void PASCAL UpdateGuideWindow(LPUIEXTRA lpUIExtra);
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

// dic.c
void PASCAL RevertText(HIMC hIMC);
BOOL PASCAL ConvKanji(HIMC);
BOOL PASCAL MakeGuideLine(HIMC, DWORD);
int GetCandidateStringsFromDictionary(LPWSTR lpString, LPWSTR lpBuf,
                                      DWORD dwBufLen, LPTSTR szDicFileName);

// dic2.c
WORD PASCAL HanToZen(WORD, WORD, DWORD);
WORD PASCAL ZenToHan(WORD);
BOOL PASCAL IsTenten(WORD);
WORD PASCAL ConvTenten(WORD);
BOOL PASCAL IsMaru(WORD);
WORD PASCAL ConvMaru(WORD);
WORD PASCAL HiraToKata(WORD);
WORD PASCAL KataToHira(WORD);
void PASCAL lHanToZen(LPTSTR, LPTSTR, DWORD);

// mzimeja.cpp
BOOL IsInputModeOpen(InputMode imode);
InputMode InputModeFromConversionMode(BOOL bOpen, DWORD dwConversion);
InputMode NextInputMode(InputMode imode);
InputMode GetInputMode(HIMC hIMC);
void SetInputMode(HIMC hIMC, InputMode imode);
void RepositionWindow(HWND hWnd);
BOOL IsRomajiMode(HIMC hIMC);
void SetRomajiMode(HIMC hIMC, BOOL bRomaji);
UINT CommandFromInputMode(InputMode imode);
HFONT CheckNativeCharset(HDC hDC);

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
// C++ interface

struct MZIMEJA {
  HINSTANCE       m_hInst;
  HANDLE          m_hMutex;
  HKL             m_hMyKL;
  BOOL            m_bWinLogOn;
  LPTRANSMSGLIST  m_lpCurTransKey;
  UINT            m_uNumTransKey;
  BOOL            m_fOverTransKey;
  HIMC            m_hIMC;
  InputContext *  m_lpIMC;

  // dictionary file name
  TCHAR           m_szDicFileName[256];

  MZIMEJA();
  BOOL Init(HINSTANCE hInstance);
  VOID Destroy(VOID);
  HBITMAP LoadBMP(LPCTSTR pszName);
  HBITMAP LoadBMP(UINT nID) {
    return LoadBMP(MAKEINTRESOURCE(nID));
  }
  LPTSTR LoadSTR(INT nID);
  BOOL RegisterClasses(HINSTANCE hInstance);
  void UpdateIndicIcon(HIMC hIMC);
  HKL GetHKL(VOID);

  InputContext *LockIMC(HIMC hIMC);
  VOID UnlockIMC(HIMC hIMC);
  BOOL GenerateMessage(LPTRANSMSG lpGeneMsg);
  BOOL GenerateMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);
  BOOL GenerateMessageToTransKey(LPTRANSMSG lpGeneMsg);

  BOOL DoCommand(HIMC hIMC, DWORD dwCommand);
}; // struct MZIMEJA

extern MZIMEJA TheIME;

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef MZIMEJA_H_

//////////////////////////////////////////////////////////////////////////////
