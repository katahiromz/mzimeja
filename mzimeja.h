// mzimeja.h --- MZ-IME Japanese Input
//////////////////////////////////////////////////////////////////////////////

#ifndef MZIMEJA_H_
#define MZIMEJA_H_

#ifndef _INC_WINDOWS
  #include <windows.h>
#endif
#ifndef _INC_TCHAR
  #include <tchar.h>
#endif

#ifndef RC_INVOKED
  #include <cstdio>
  #include <cassert>
  #include <cstring>
#endif

#include "indicml.h"
#include "immdev.h"

//////////////////////////////////////////////////////////////////////////////

#define MZIME_FILENAME  TEXT("mzimeja.ime")

// for limit of MZ-IME
#define MAXCOMPWND 10
#define MAXCOMPSIZE 128
#define MAXCLAUSESIZE 16
#define MAXCANDPAGESIZE 9
#define MAXCANDSTRSIZE 16
#define MAXGLCHAR 32
#define MAXCANDSTRNUM 32

// for GlobalAlloc
#define GHIME (GHND | GMEM_SHARE)

// special messages
#define WM_UI_UPDATE (WM_USER + 500)
#define WM_UI_HIDE (WM_USER + 501)

#define WM_UI_STATEMOVE (WM_USER + 601)
#define WM_UI_DEFCOMPMOVE (WM_USER + 602)
#define WM_UI_CANDMOVE (WM_USER + 603)
#define WM_UI_GUIDEMOVE (WM_USER + 604)

// Escape Functions
#define IME_ESC_PRI_GETDWORDTEST (IME_ESC_PRIVATE_FIRST + 0)

// special style
#define WS_COMPDEFAULT (WS_DISABLED | WS_POPUP)
#define WS_COMPNODEFAULT (WS_DISABLED | WS_POPUP)

// macro for me!
#define IsCTLPushed(x) ((x)[VK_CONTROL] & 0x80)
#define IsSHFTPushed(x) ((x)[VK_SHIFT] & 0x80)
#define IsALTPushed(x) ((x)[VK_ALT] & 0x80)

// update context
#define UPDATE_MODE 0x01
#define UPDATE_SYSTEM 0x02
#define UPDATE_TEXT 0x04
#define UPDATE_FORCE 0x08
#define UPDATE_ALL (UPDATE_MODE | UPDATE_SYSTEM | UPDATE_TEXT)

// advise context
#define ADVISE_MODE 0x0001   // advise about Mode requested  
#define ADVISE_ISOPEN 0x0002 // advise about IsOpen requested

// key state context
#define KS_SHIFT 0x01
#define KS_CONTROL 0x02

// ID of guideline table
#define MYGL_NODICTIONARY 0
#define MYGL_TYPINGERROR 1
#define MYGL_TESTGUIDELINE 2

// Change Mode index
#define TO_CMODE_ALPHANUMERIC 0x0001
#define TO_CMODE_KATAKANA 0x0002
#define TO_CMODE_HIRAGANA 0x0003
#define TO_CMODE_FULLSHAPE 0x0008
#define TO_CMODE_ROMAN 0x0010
#define TO_CMODE_CHARCODE 0x0020
#define TO_CMODE_TOOLBAR 0x0100

// WndExtra of child UI windows
#define FIGWL_MOUSE 0
#define FIGWL_SVRWND (FIGWL_MOUSE + sizeof(LONG))
#define FIGWL_FONT (FIGWL_SVRWND + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTSTR (FIGWL_FONT + sizeof(LONG_PTR))
#define FIGWL_COMPSTARTNUM (FIGWL_COMPSTARTSTR + sizeof(LONG))
#define FIGWL_STATUSBMP (FIGWL_COMPSTARTNUM + sizeof(LONG))
#define FIGWL_CLOSEBMP (FIGWL_STATUSBMP + sizeof(LONG_PTR))
#define FIGWL_PUSHSTATUS (FIGWL_CLOSEBMP + sizeof(LONG_PTR))
#define FIGWL_CHILDWND (FIGWL_PUSHSTATUS + sizeof(LONG))
#define UIEXTRASIZE (FIGWL_CHILDWND + sizeof(LONG_PTR))

// The flags of FIGWL_MOUSE
#define FIM_CAPUTURED 0x01
#define FIM_MOVED 0x02

// The flags of the button of Status Window
#define PUSHED_STATUS_HDR 0x01
#define PUSHED_STATUS_MODE 0x02
#define PUSHED_STATUS_ROMAN 0x04
#define PUSHED_STATUS_CLOSE 0x08

// Status Button Pos
#define BTX 20
#define BTY 20
#define BTFHIRA 20
#define BTFKATA 40
#define BTFALPH 60
#define BTHKATA 80
#define BTHALPH 100
#define BTROMA 120
#define BTEMPT 140

// Statue Close Button
#define STCLBT_X (BTX * 2 + 3)
#define STCLBT_Y 1
#define STCLBT_DX 12
#define STCLBT_DY 12

// define Shift Arrow right-left
#define ARR_RIGHT 1
#define ARR_LEFT 2

// Init or Clear Structure Flag
#define CLR_RESULT 1
#define CLR_UNDET 2
#define CLR_RESULT_AND_UNDET 3

//////////////////////////////////////////////////////////////////////////////

// define GET LP for COMPOSITIONSTRING members.
#define GETLPCOMPREADATTR(lpcs) \
  (LPBYTE)((LPBYTE)(lpcs) + (lpcs)->dwCompReadAttrOffset)
#define GETLPCOMPREADCLAUSE(lpcs) \
  (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwCompReadClauseOffset)
#define GETLPCOMPREADSTR(lpcs) \
  (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwCompReadStrOffset)
#define GETLPCOMPATTR(lpcs) (LPBYTE)((LPBYTE)(lpcs) + (lpcs)->dwCompAttrOffset)
#define GETLPCOMPCLAUSE(lpcs) \
  (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwCompClauseOffset)
#define GETLPCOMPSTR(lpcs) (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwCompStrOffset)
#define GETLPRESULTREADCLAUSE(lpcs) \
  (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwResultReadClauseOffset)
#define GETLPRESULTREADSTR(lpcs) \
  (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwResultReadStrOffset)
#define GETLPRESULTCLAUSE(lpcs) \
  (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwResultClauseOffset)
#define GETLPRESULTSTR(lpcs) \
  (LPTSTR)((LPBYTE)(lpcs) + (lpcs)->dwResultStrOffset)

#define SetClause(lpdw, num)      \
  {                               \
    *((LPDWORD)(lpdw)) = 0;       \
    *((LPDWORD)(lpdw) + 1) = num; \
  }

//////////////////////////////////////////////////////////////////////////////

#define GCS_COMPALL                                                  \
  (GCS_COMPSTR | GCS_COMPATTR | GCS_COMPREADSTR | GCS_COMPREADATTR | \
   GCS_COMPCLAUSE | GCS_COMPREADCLAUSE)
#define GCS_RESULTALL \
  (GCS_RESULTSTR | GCS_RESULTREADSTR | GCS_RESULTCLAUSE | GCS_RESULTREADCLAUSE)

//////////////////////////////////////////////////////////////////////////////

// ImeMenu Define
#define NUM_ROOT_MENU_L 3
#define NUM_ROOT_MENU_R 3
#define NUM_SUB_MENU_L 2
#define NUM_SUB_MENU_R 2

#define IDIM_ROOT_ML_1 0x10
#define IDIM_ROOT_ML_2 0x11
#define IDIM_ROOT_ML_3 0x12
#define IDIM_SUB_ML_1 0x20
#define IDIM_SUB_ML_2 0x21
#define IDIM_ROOT_MR_1 0x30
#define IDIM_ROOT_MR_2 0x31
#define IDIM_ROOT_MR_3 0x32
#define IDIM_SUB_MR_1 0x40
#define IDIM_SUB_MR_2 0x41

#define NATIVE_CHARSET SHIFTJIS_CHARSET

//////////////////////////////////////////////////////////////////////////////
// Structures

typedef struct _tagMZCOMPSTR {
  COMPOSITIONSTRING cs;
  TCHAR   szCompReadStr[MAXCOMPSIZE];
  BYTE    bCompReadAttr[MAXCOMPSIZE];
  DWORD   dwCompReadClause[MAXCLAUSESIZE];
  TCHAR   szCompStr[MAXCOMPSIZE];
  BYTE    bCompAttr[MAXCOMPSIZE];
  DWORD   dwCompClause[MAXCLAUSESIZE];
  char    szTypeInfo[MAXCOMPSIZE];
  TCHAR   szResultReadStr[MAXCOMPSIZE];
  DWORD   dwResultReadClause[MAXCOMPSIZE];
  TCHAR   szResultStr[MAXCOMPSIZE];
  DWORD   dwResultClause[MAXCOMPSIZE];
} MZCOMPSTR, NEAR *PMZCOMPSTR, FAR *LPMZCOMPSTR;

typedef struct _tagMZCAND {
  CANDIDATEINFO ci;
  CANDIDATELIST cl;
  DWORD offset[MAXCANDSTRNUM];
  TCHAR szCand[MAXCANDSTRNUM][MAXCANDSTRSIZE];
} MZCAND, NEAR *PMZCAND, FAR *LPMZCAND;

typedef struct _tagUICHILD {
  HWND hWnd;
  BOOL bShow;
  POINT pt;
} UICHILD, NEAR *PUICHILD, FAR *LPUICHILD;

typedef struct _tagUICHILD2 {
  HWND hWnd;
  BOOL bShow;
  RECT rc;
} UICHILD2, NEAR *PUICHILD2, FAR *LPUICHILD2;

typedef struct _tagUIEXTRA {
  HIMC hIMC;
  UICHILD uiStatus;
  UICHILD uiCand;
  DWORD dwCompStyle;
  HFONT hFont;
  BOOL bVertical;
  UICHILD uiDefComp;
  UICHILD2 uiComp[MAXCOMPWND];
  UICHILD uiGuide;
} UIEXTRA, NEAR *PUIEXTRA, FAR *LPUIEXTRA;

typedef struct _tagMZGUIDELINE {
  DWORD dwLevel;
  DWORD dwIndex;
  DWORD dwStrID;
  DWORD dwPrivateID;
} MZGUIDELINE, NEAR *PMZGUIDELINE, FAR *LPMZGUIDELINE;

//////////////////////////////////////////////////////////////////////////////
// externs

extern HINSTANCE hInst;
extern HKL hMyKL;
extern BOOL bWinLogOn;
extern LPTRANSMSGLIST lpCurTransKey;
extern UINT uNumTransKey;
extern BOOL fOverTransKey;
extern TCHAR szUIClassName[];
extern TCHAR szCompStrClassName[];
extern TCHAR szCandClassName[];
extern TCHAR szStatusClassName[];
extern TCHAR szGuideClassName[];
extern MZGUIDELINE glTable[];
extern TCHAR szDicFileName[];
extern BYTE bComp[];
extern BYTE bCompCtl[];
extern BYTE bCompSht[];
extern BYTE bCompAlt[];
extern BYTE bNoComp[];
extern BYTE bNoCompCtl[];
extern BYTE bNoCompSht[];
extern BYTE bNoCompAlt[];

//////////////////////////////////////////////////////////////////////////////

extern "C" {

// subs.c
void PASCAL InitCompStr(LPCOMPOSITIONSTRING lpCompStr, DWORD dwClrFlag);
void PASCAL ClearCompStr(LPCOMPOSITIONSTRING lpCompStr, DWORD dwClrFlag);
void PASCAL ClearCandidate(LPCANDIDATEINFO lpCandInfo);
void PASCAL ChangeMode(HIMC hIMC, DWORD dwToMode);
void PASCAL ChangeCompStr(HIMC hIMC, DWORD dwToMode);
BOOL PASCAL IsCompStr(HIMC hIMC);
BOOL PASCAL IsConvertedCompStr(HIMC hIMC);
BOOL PASCAL IsCandidate(LPINPUTCONTEXT lpIMC);
void PASCAL UpdateIndicIcon(HIMC hIMC);
HFONT CheckNativeCharset(HDC hDC);

// toascii.c
BOOL PASCAL GenerateMessageToTransKey(LPTRANSMSGLIST lpTrabsKey,
                                      LPTRANSMSG lpGeneMsg);
BOOL PASCAL GenerateOverFlowMessage(LPTRANSMSGLIST lpTransKey);

// notify.c
BOOL PASCAL NotifyUCSetOpen(HIMC hIMC);
BOOL PASCAL NotifyUCConversionMode(HIMC hIMC);
BOOL PASCAL NotifyUCSetCompositionWindow(HIMC hIMC);

// input.c
BOOL PASCAL DicKeydownHandler(HIMC, WPARAM, LPARAM, LPBYTE);
BOOL PASCAL IMEKeydownHandler(HIMC, WPARAM, LPARAM, LPBYTE);
BOOL PASCAL IMEKeyupHandler(HIMC, WPARAM, LPARAM, LPBYTE);

// ui.c
BOOL IMERegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK MZIMEWndProc(HWND, UINT, WPARAM, LPARAM);
LONG PASCAL NotifyCommand(HIMC hUICurIMC, HWND hWnd, UINT message,
                          WPARAM wParam, LPARAM lParam);
LONG PASCAL ControlCommand(HIMC hUICurIMC, HWND hWnd, UINT message,
                           WPARAM wParam, LPARAM lParam);
void PASCAL DrawUIBorder(LPRECT lprc);
void PASCAL DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int PASCAL GetCompFontHeight(LPUIEXTRA lpUIExtra);

// uistate.c
LRESULT CALLBACK StatusWndProc(HWND, UINT, WPARAM, LPARAM);
void PASCAL PaintStatus(HWND hStatusWnd, HDC hDC, LPPOINT lppt,
                        DWORD dwPushedStatus);
void PASCAL ButtonStatus(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL PASCAL MyIsIMEMessage(UINT message);
void PASCAL UpdateStatusWindow(LPUIEXTRA lpUIExtra);

// uicand.c
LRESULT CALLBACK CandWndProc(HWND, UINT, WPARAM, LPARAM);
void PASCAL PaintCandWindow(HWND hCandWnd);
BOOL PASCAL GetCandPosFromComp(LPINPUTCONTEXT lpIMC, LPUIEXTRA lpUIExtra,
                               LPPOINT lppt);
void PASCAL CreateCandWindow(HWND hUIWnd, LPUIEXTRA lpUIExtra,
                             LPINPUTCONTEXT lpIMC);
void PASCAL ResizeCandWindow(LPUIEXTRA lpUIExtra, LPINPUTCONTEXT lpIMC);
void PASCAL HideCandWindow(LPUIEXTRA lpUIExtra);
void PASCAL MoveCandWindow(HWND hUIWnd, LPINPUTCONTEXT lpIMC,
                           LPUIEXTRA lpUIExtra, BOOL fForceComp);

// uicomp.c
LRESULT CALLBACK CompStrWndProc(HWND, UINT, WPARAM, LPARAM);
void PASCAL PaintCompWindow(HWND hCompWnd);
void PASCAL CreateCompWindow(HWND hUIWnd, LPUIEXTRA lpUIExtra,
                             LPINPUTCONTEXT lpIMC);
void PASCAL MoveCompWindow(LPUIEXTRA lpUIExtra, LPINPUTCONTEXT lpIMC);
void PASCAL HideCompWindow(LPUIEXTRA lpUIExtra);
void PASCAL SetFontCompWindow(LPUIEXTRA lpUIExtra);

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
void PASCAL DeleteChar(HIMC hIMC, UINT uVKey);
void PASCAL FlushText(HIMC hIMC);
void PASCAL RevertText(HIMC hIMC);
void PASCAL AddChar(HIMC, WORD);
BOOL PASCAL ConvKanji(HIMC);
BOOL WINAPI MakeResultString(HIMC, BOOL);
BOOL PASCAL MakeGuideLine(HIMC, DWORD);
BOOL PASCAL GenerateMessage(HIMC, LPINPUTCONTEXT, LPTRANSMSGLIST, LPTRANSMSG);
BOOL PASCAL CheckAttr(LPCOMPOSITIONSTRING lpCompStr);
void PASCAL MakeAttrClause(LPCOMPOSITIONSTRING lpCompStr);
void PASCAL HandleShiftArrow(HIMC hIMC, BOOL fArrow);
int GetCandidateStringsFromDictionary(LPWSTR lpString, LPWSTR lpBuf,
                                      DWORD dwBufLen, LPTSTR szDicFileName);

// dic2.c
BOOL OneCharZenToHan(WCHAR, WCHAR *, WCHAR *);
WORD PASCAL HanToZen(WORD, WORD, DWORD);
WORD PASCAL ZenToHan(WORD);
int PASCAL IsFirst(WORD);
int PASCAL IsSecond(WORD);
WORD PASCAL ConvChar(HIMC, WORD, WORD);
BOOL PASCAL IsTenten(WORD);
WORD PASCAL ConvTenten(WORD);
BOOL PASCAL IsMaru(WORD);
WORD PASCAL ConvMaru(WORD);
WORD PASCAL HiraToKata(WORD);
WORD PASCAL KataToHira(WORD);
void PASCAL lZenToHan(LPTSTR, LPTSTR);
void PASCAL lHanToZen(LPTSTR, LPTSTR, DWORD);

#ifdef _DEBUG
  int DebugPrint(LPCTSTR lpszFormat, ...);
  VOID WarnOut(LPCTSTR pStr);
  VOID ErrorOut(LPCTSTR pStr);
  #define ERROROUT(x) ErrorOut(x)
  #define WARNOUT(x) WarnOut(x)
#else
  #define DebugPrint (void)
  #define ERROROUT(x)
  #define WARNOUT(x)
#endif

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

#endif  // ndef MZIMEJA_H_

//////////////////////////////////////////////////////////////////////////////
