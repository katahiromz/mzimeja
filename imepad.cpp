// imepad.cpp --- mzimeja IME Pad UI
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS   // use fopen
#include "targetver.h"      // target Windows version

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>          // for Windows generic text

#include <string>           // for std::string, std::wstring, ...
#include <vector>           // for std::vector
#include <set>              // for std::set
#include <map>              // for std::map
#include <algorithm>        // for std::sort

#include <cstdlib>          // for C standard library
#include <cstdio>           // for C standard I/O
#include <cctype>           // for C character types
#include <cassert>          // for assert
#include <cstring>          // for C string

#define UNBOOST_USE_STRING_ALGORITHM
#define UNBOOST_USE_UNORDERED_MAP
#include "unboost.hpp"

#include "resource.h"

//////////////////////////////////////////////////////////////////////////////

#define INITIAL_WIDTH 380
#define INITIAL_HEIGHT 250
#define INTERVAL_MILLISECONDS   100

struct KANJI_ENTRY {
  WORD          kanji_id;
  WCHAR         kanji_char;
  WORD          radical_id2;
  WORD          strokes;
  std::wstring  readings;
};

struct RADICAL_ENTRY {
  WORD          radical_id;
  WORD          radical_id2;
  WORD          strokes;
  std::wstring  readings;
};

const WCHAR szImePadClassName[] = L"MZIMEPad";

//////////////////////////////////////////////////////////////////////////////
// IME Pad

class ImePad {
public:
  ImePad();
  ~ImePad();

  BOOL PrepareForKanji();
  static BOOL Create(HWND hwndParent);
  static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
  static LRESULT CALLBACK TabCtrlWndProc(HWND, UINT, WPARAM, LPARAM);

protected:
  HWND            m_hWnd;

  // data
  std::vector<KANJI_ENTRY>                          m_kanji_table;
  unboost::unordered_map<WORD, std::vector<WORD> >  m_kanji_stroke_map;
  std::vector<RADICAL_ENTRY>                        m_radical_table;
  unboost::unordered_map<WORD, std::vector<WORD> >  m_radical_stroke_map;
  unboost::unordered_map<WORD, WORD>                m_radical_id_map;
  unboost::unordered_map<WORD, std::vector<WORD> >  m_radical2_to_kanji_map;
  BOOL LoadKanjiData();
  BOOL LoadRadicalData();
  BOOL LoadKanjiAndRadical();

  // UI
  HWND            m_hTabCtrl;
  HWND            m_hListView;
  HWND            m_hListBox1;
  HWND            m_hListBox2;
  WNDPROC         m_fnTabCtrlOldWndProcOld;
  HWND            m_hwndOld;
  void MySendInput(WCHAR ch);

  // images
  HIMAGELIST      m_himlKanji;
  HIMAGELIST      m_himlRadical;
  HBITMAP         m_hbmRadical;
  BOOL LoadRadicalImage();
  BOOL CreateKanjiImageList();
  BOOL CreateRadicalImageList();
  void DeleteAllImages();

  // fonts
  HFONT           m_hSmallFont;
  HFONT           m_hNormalFont;
  HFONT           m_hLargeFont;
  BOOL CreateAllFonts();
  void DeleteAllFonts();

  BOOL OnCreate(HWND hWnd);
  void OnSize(HWND hWnd);
  void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
  void OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
  void OnDrawItem(HWND hWnd, LPDRAWITEMSTRUCT lpDraw);
  void OnEraseBkGnd(HWND hWnd);
  void OnLV1StrokesChanged(HWND hWnd);
  void OnLV2StrokesChanged(HWND hWnd);
  void OnTimer(HWND hWnd);
  void OnGetMinMaxInfo(LPMINMAXINFO pmmi);
}; // class ImePad

//////////////////////////////////////////////////////////////////////////////

HINSTANCE g_hInst;

//////////////////////////////////////////////////////////////////////////////

LPWSTR LoadStringDx(INT nStringID) {
  static WCHAR s_sz[MAX_PATH * 2];
  s_sz[0] = L'\0';
  ::LoadStringW(g_hInst, nStringID, s_sz, MAX_PATH * 2);
  return s_sz;
}

HBITMAP LoadBitmapDx(LPCWSTR pszName) {
  assert(g_hInst);
  return ::LoadBitmap(g_hInst, pszName);
}

HBITMAP LoadBitmapDx(INT nBitmapID) {
  return LoadBitmapDx(MAKEINTRESOURCEW(nBitmapID));
}

static const WCHAR s_szRegKey[] = 
  L"SOFTWARE\\Katayama Hirofumi MZ\\mzimeja";

static std::wstring GetSettingString(LPCWSTR pszSettingName) {
  HKEY hKey;
  LONG result;
  WCHAR szValue[MAX_PATH * 2];
  result = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, s_szRegKey,
                           0, KEY_READ | KEY_WOW64_64KEY, &hKey);
  if (result != ERROR_SUCCESS) {
    result = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, s_szRegKey,
                             0, KEY_READ, &hKey);
  }
  if (result == ERROR_SUCCESS && hKey) {
    DWORD cbData = sizeof(szValue);
    result = ::RegQueryValueExW(hKey, pszSettingName, NULL, NULL, 
      reinterpret_cast<LPBYTE>(szValue), &cbData);
    ::RegCloseKey(hKey);
    if (result == ERROR_SUCCESS) {
      return std::wstring(szValue);
    }
  }
  assert(0);
  return std::wstring();
} // GetSettingString

// adjust window position
static void RepositionWindow(HWND hWnd) {
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

static HBITMAP Create24BppBitmap(HDC hDC, INT cx, INT cy) {
  BITMAPINFO bi;
  ZeroMemory(&bi, sizeof(bi));
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = cx;
  bi.bmiHeader.biHeight = cy;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 24;

  LPVOID pvBits;
  return ::CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
}

inline bool
radical_compare(const RADICAL_ENTRY& entry1, const RADICAL_ENTRY& entry2) {
  return entry1.strokes < entry2.strokes;
}

void ImePad::OnDrawItem(HWND hWnd, LPDRAWITEMSTRUCT lpDraw) {
  INT id = lpDraw->itemID;  
  const RADICAL_ENTRY& entry = m_radical_table[id];
  RECT rc = lpDraw->rcItem;
  if (lpDraw->itemState & ODS_SELECTED) {
    ::FillRect(lpDraw->hDC, &rc, (HBRUSH)(COLOR_HIGHLIGHT + 1));
    ImageList_Draw(m_himlRadical, entry.radical_id - 1, lpDraw->hDC,
      rc.left, rc.top, ILD_NORMAL);
    ::SelectObject(lpDraw->hDC, ::GetStockObject(BLACK_PEN));
    ::SelectObject(lpDraw->hDC, ::GetStockObject(NULL_BRUSH));
    ::Rectangle(lpDraw->hDC, rc.left, rc.top, rc.left + 24, rc.top + 24);
    rc.left += 24;
    ::SetBkMode(lpDraw->hDC, TRANSPARENT);
    ::SetTextColor(lpDraw->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    ::DrawText(lpDraw->hDC, entry.readings.c_str(), -1,
      &rc, DT_LEFT | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
  } else {
    ::FillRect(lpDraw->hDC, &rc, (HBRUSH)(COLOR_WINDOW + 1));
    ImageList_Draw(m_himlRadical, entry.radical_id - 1, lpDraw->hDC,
      rc.left, rc.top, ILD_NORMAL);
    ::SelectObject(lpDraw->hDC, ::GetStockObject(BLACK_PEN));
    ::SelectObject(lpDraw->hDC, ::GetStockObject(NULL_BRUSH));
    ::Rectangle(lpDraw->hDC, rc.left, rc.top, rc.left + 24, rc.top + 24);
    rc.left += 24;
    ::SetBkMode(lpDraw->hDC, TRANSPARENT);
    ::SetTextColor(lpDraw->hDC, GetSysColor(COLOR_WINDOWTEXT));
    ::DrawText(lpDraw->hDC, entry.readings.c_str(), -1,
      &rc, DT_LEFT | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
  }
}

void ImePad::OnEraseBkGnd(HWND hWnd) {
  RECT rc;
  ::GetClientRect(hWnd, &rc);

  HDC hDC = ::GetDC(hWnd);
  ::SetTextColor(hDC, GetSysColor(COLOR_BTNTEXT));
  ::SetBkMode(hDC, TRANSPARENT);
  ::SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
  ::DrawText(hDC, TEXT("(í—pŠ¿Žš‚Ì‚Ý‚Å‚·)"), -1,
    &rc, DT_RIGHT | DT_TOP | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
  ::ReleaseDC(hWnd, hDC);
}

/*static*/ LRESULT CALLBACK
ImePad::TabCtrlWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LPMEASUREITEMSTRUCT lpMeasure;
  LPDRAWITEMSTRUCT lpDraw;
  ImePad *pImePad;
  pImePad = (ImePad *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if (pImePad == NULL) return 0;

  switch (uMsg) {
  case WM_MEASUREITEM:
    lpMeasure = (LPMEASUREITEMSTRUCT)lParam;
    lpMeasure->itemWidth = 64;
    lpMeasure->itemHeight = 24;
    return TRUE;
  case WM_DRAWITEM:
    lpDraw = (LPDRAWITEMSTRUCT)lParam;
    pImePad->OnDrawItem(hWnd, lpDraw);
    return TRUE;
  case WM_COMMAND:
    if (HIWORD(wParam) == LBN_SELCHANGE) {
      ::PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
    }
    break;
  case WM_ERASEBKGND:
    ::CallWindowProc(
      pImePad->m_fnTabCtrlOldWndProcOld, hWnd, uMsg, wParam, lParam);
    pImePad->OnEraseBkGnd(hWnd);
    return 0;
  default:
    break;
  }
  return ::CallWindowProc(
    pImePad->m_fnTabCtrlOldWndProcOld, hWnd, uMsg, wParam, lParam);
} // ImePad::TabCtrlWndProc

//////////////////////////////////////////////////////////////////////////////
// create/destroy

/*static*/ BOOL ImePad::Create(HWND hwndParent) {
  RECT rc;
  ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

  rc.left = rc.right - INITIAL_WIDTH - 32;
  rc.top = rc.bottom - INITIAL_HEIGHT - 32;

  DWORD style = WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_ACTIVECAPTION;
  DWORD exstyle = WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST |
                  WS_EX_NOACTIVATE;
  HWND hImePad = ::CreateWindowEx(
    exstyle, szImePadClassName, LoadStringDx(IDM_IME_PAD),
    style, rc.left, rc.top, INITIAL_WIDTH, INITIAL_HEIGHT,
    hwndParent, NULL, g_hInst, NULL);
  assert(hImePad);

  RepositionWindow(hImePad);
  ::ShowWindow(hImePad, SW_SHOWNOACTIVATE);

  return hImePad != NULL;
} // ImePad_Create

ImePad::ImePad() {
  m_hWnd = NULL;
  m_himlKanji = NULL;
  m_himlRadical = NULL;
  m_hSmallFont = NULL;
  m_hNormalFont = NULL;
  m_hLargeFont = NULL;
  m_hbmRadical = NULL;
  m_hwndOld = NULL;
}

ImePad::~ImePad() {
  DeleteAllImages();
  DeleteAllFonts();
}

//////////////////////////////////////////////////////////////////////////////
// loading res/kanji.dat and res/radical.dat

BOOL ImePad::LoadKanjiData() {
  if (m_kanji_table.size()) {
    return TRUE;
  }
  char buf[256];
  wchar_t wbuf[256];
  std::wstring kanji_file = GetSettingString(L"KanjiDataFile");
  using namespace std;
  FILE *fp = _wfopen(kanji_file.c_str(), L"rb");
  if (fp) {
    KANJI_ENTRY entry;
    while (fgets(buf, 256, fp) != NULL) {
      if (buf[0] == ';') continue;
      ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
      std::wstring str = wbuf;
      unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
      std::vector<std::wstring> vec;
      unboost::split(vec, str, unboost::is_any_of(L"\t"));
      entry.kanji_id = _wtoi(vec[0].c_str());
      entry.kanji_char = vec[1][0];
      entry.radical_id2 = _wtoi(vec[2].c_str());
      entry.strokes = _wtoi(vec[3].c_str());
      entry.readings = vec[4];
      m_kanji_table.push_back(entry);
      m_kanji_stroke_map[entry.strokes].push_back(entry.kanji_id);
    }
    fclose(fp);
    return TRUE;
  }
  assert(0);
  return FALSE;
} // ImePad::LoadKanjiData

BOOL ImePad::LoadRadicalData() {
  if (m_radical_table.size()) {
    return TRUE;
  }
  char buf[256];
  wchar_t wbuf[256];
  std::wstring radical_file = GetSettingString(L"RadicalDataFile");
  using namespace std;
  FILE *fp = _wfopen(radical_file.c_str(), L"rb");
  if (fp) {
    RADICAL_ENTRY entry;
    while (fgets(buf, 256, fp) != NULL) {
      if (buf[0] == ';') continue;
      ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
      std::wstring str = wbuf;
      unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
      std::vector<std::wstring> vec;
      unboost::split(vec, str, unboost::is_any_of(L"\t"));
      entry.radical_id = _wtoi(vec[0].c_str());
      entry.radical_id2 = _wtoi(vec[1].c_str());
      entry.strokes = _wtoi(vec[3].c_str());
      entry.readings = vec[4];
      m_radical_table.push_back(entry);
      m_radical_stroke_map[entry.strokes].push_back(entry.radical_id);
      m_radical_id_map[entry.radical_id] = entry.radical_id2;
    }
    fclose(fp);
    return TRUE;
  }
  assert(0);
  return FALSE;
} // ImePad::LoadRadicalData

BOOL ImePad::LoadKanjiAndRadical() {
  if (LoadKanjiData() && LoadRadicalData()) {
    if (m_radical2_to_kanji_map.size()) {
      return TRUE;
    }

    for (size_t i = 0; i < m_kanji_table.size(); ++i) {
      const KANJI_ENTRY& kanji = m_kanji_table[i];
      m_radical2_to_kanji_map[kanji.radical_id2].push_back(kanji.kanji_id);
    }
    std::sort(m_radical_table.begin(), m_radical_table.end(),
              radical_compare);
    return TRUE;
  }
  assert(0);
  return FALSE;
}

void ImePad::DeleteAllImages() {
  if (m_hbmRadical) {
    ::DeleteObject(m_hbmRadical);
    m_hbmRadical = NULL;
  }
  if (m_himlKanji) {
    ImageList_Destroy(m_himlKanji);
    m_himlKanji = NULL;
  }
  if (m_himlRadical) {
    ImageList_Destroy(m_himlRadical);
    m_himlRadical = NULL;
  }
}

void ImePad::DeleteAllFonts() {
  if (m_hSmallFont) {
    ::DeleteObject(m_hSmallFont);
    m_hSmallFont = NULL;
  }
  if (m_hNormalFont) {
    ::DeleteObject(m_hNormalFont);
    m_hNormalFont = NULL;
  }
  if (m_hLargeFont) {
    ::DeleteObject(m_hLargeFont);
    m_hLargeFont = NULL;
  }
}

BOOL ImePad::CreateAllFonts() {
  if (m_hSmallFont) {
    DeleteAllFonts();
  }

  LOGFONT lf;
  ZeroMemory(&lf, sizeof(lf));
  lf.lfHeight = 28;
  lf.lfCharSet = SHIFTJIS_CHARSET;
  lf.lfQuality = ANTIALIASED_QUALITY;
  lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;

  lf.lfHeight = 10;
  m_hSmallFont = ::CreateFontIndirect(&lf);
  lf.lfHeight = 12;
  m_hNormalFont = ::CreateFontIndirect(&lf);
  lf.lfHeight = 28;
  m_hLargeFont = ::CreateFontIndirect(&lf);
  if (m_hSmallFont && m_hNormalFont && m_hLargeFont) {
    return TRUE;
  }
  assert(0);
  return FALSE;
} // ImePad::CreateAllFonts

BOOL ImePad::LoadRadicalImage() {
  if (m_hbmRadical == NULL) {
    m_hbmRadical = LoadBitmapDx(1);
  }
  assert(m_hbmRadical);
  return m_hbmRadical != NULL;
}

BOOL ImePad::CreateKanjiImageList() {
  if (!LoadKanjiAndRadical()) {
    return FALSE;
  }

  if (m_himlKanji) {
    ImageList_Destroy(m_himlKanji);
    m_himlKanji = NULL;
  }

  m_himlKanji = ImageList_Create(32, 32, ILC_COLOR,
    (INT)m_kanji_table.size(), 1);
  if (m_himlKanji == NULL) {
    return FALSE;
  }

  HDC hDC = ::CreateCompatibleDC(NULL);
  ::SelectObject(hDC, GetStockObject(WHITE_BRUSH));
  ::SelectObject(hDC, GetStockObject(BLACK_PEN));
  HGDIOBJ hFontOld = ::SelectObject(hDC, m_hLargeFont);
  for (size_t i = 0; i < m_kanji_table.size(); ++i) {
    HBITMAP hbm = Create24BppBitmap(hDC, 32, 32);
    HGDIOBJ hbmOld = ::SelectObject(hDC, hbm);
    {
      ::Rectangle(hDC, 0, 0, 32, 32);
      RECT rc;
      ::SetRect(&rc, 0, 0, 32, 32);
      ::DrawTextW(hDC, &m_kanji_table[i].kanji_char, 1, &rc,
        DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
    }
    ::SelectObject(hDC, hbmOld);
    ImageList_Add(m_himlKanji, hbm, NULL);
    ::DeleteObject(hbm);
  }
  ::SelectObject(hDC, hFontOld);

  return TRUE;
} // ImePad::CreateKanjiImageList

BOOL ImePad::CreateRadicalImageList() {
  if (!LoadKanjiAndRadical() || !LoadRadicalImage()) {
    return FALSE;
  }

  if (m_himlRadical) {
    ImageList_Destroy(m_himlRadical);
  }
  m_himlRadical = ImageList_Create(24, 24, ILC_COLOR,
    (INT)m_radical_table.size(), 1);
  if (m_himlRadical == NULL) {
    return FALSE;
  }

  assert(m_hbmRadical);
  HDC hDC = ::CreateCompatibleDC(NULL);
  HDC hDC2 = ::CreateCompatibleDC(NULL);
  HGDIOBJ hbm2Old = ::SelectObject(hDC2, m_hbmRadical);
  for (size_t i = 0; i < m_radical_table.size(); ++i) {
    HBITMAP hbm = Create24BppBitmap(hDC, 24, 24);
    HGDIOBJ hbmOld = ::SelectObject(hDC, hbm);
    {
      ::BitBlt(hDC, 0, 0, 24, 24, hDC2, i * 24, 0, SRCCOPY);
    }
    ::SelectObject(hDC, hbmOld);
    ImageList_Add(m_himlRadical, hbm, NULL);
    ::DeleteObject(hbm);
  }
  ::SelectObject(hDC2, hbm2Old);
  ::DeleteDC(hDC2);
  ::DeleteDC(hDC);

  return TRUE;
} // ImePad::CreateRadicalImageList

BOOL ImePad::PrepareForKanji() {
  if (CreateAllFonts() &&
      CreateKanjiImageList() &&
      CreateRadicalImageList())
  {
    return TRUE;
  }
  return FALSE;
}

BOOL ImePad::OnCreate(HWND hWnd) {
  if (!PrepareForKanji()) {
    return FALSE;
  }

  ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

  RECT rc;
  ::GetClientRect(hWnd, &rc);

  DWORD style, exstyle;

  // create tab control
  style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_FOCUSNEVER;
  exstyle = WS_EX_NOACTIVATE;
  m_hTabCtrl = ::CreateWindowEx(exstyle, WC_TABCONTROL, NULL, style,
    0, 0, rc.right - rc.left, rc.bottom - rc.top,
    hWnd, (HMENU)1, g_hInst, NULL);

  ::SetWindowLongPtr(m_hTabCtrl, GWLP_USERDATA, (LONG_PTR)this);
  m_fnTabCtrlOldWndProcOld = (WNDPROC)
    SetWindowLongPtr(m_hTabCtrl, GWLP_WNDPROC,
                     (LONG_PTR)ImePad::TabCtrlWndProc);

  // initialize tab control
  TC_ITEM item;
  item.mask = TCIF_TEXT;
  item.pszText = LoadStringDx(IDM_STROKES);
  TabCtrl_InsertItem(m_hTabCtrl, 0, &item);
  item.pszText = LoadStringDx(IDM_RADICALS);
  TabCtrl_InsertItem(m_hTabCtrl, 1, &item);

  // get inner area
  TabCtrl_AdjustRect(m_hTabCtrl, FALSE, &rc);

  // create list box (strokes)
  style = WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY;
  exstyle = WS_EX_NOACTIVATE | WS_EX_CLIENTEDGE;
  m_hListBox1 = ::CreateWindowEx(exstyle, TEXT("LISTBOX"), NULL, style,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    m_hTabCtrl, (HMENU)2, g_hInst, NULL);

  // create list box (radicals)
  style = WS_CHILD | WS_VSCROLL | LBS_OWNERDRAWFIXED |
          LBS_NOINTEGRALHEIGHT | LBS_NOTIFY;
  exstyle = WS_EX_NOACTIVATE | WS_EX_CLIENTEDGE;
  m_hListBox2 = ::CreateWindowEx(exstyle, TEXT("LISTBOX"), NULL, style,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    m_hTabCtrl, (HMENU)3, g_hInst, NULL);
  ::SendMessage(m_hListBox2, LB_SETITEMHEIGHT, 0, 24);

  // create list view
  style = WS_CHILD | LVS_ICON;
  exstyle = WS_EX_NOACTIVATE | WS_EX_CLIENTEDGE;
  m_hListView = ::CreateWindowEx(exstyle, WC_LISTVIEW, NULL, style,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    m_hTabCtrl, (HMENU)4, g_hInst, NULL);

  // insert items to for strokes
  std::set<WORD> strokes;
  for (size_t i = 0; i < m_kanji_table.size(); ++i) {
    strokes.insert(m_kanji_table[i].strokes);
  }
  std::set<WORD>::iterator it, end = strokes.end();
  TCHAR sz[128];
  for (it = strokes.begin(); it != end; ++it) {
    ::wsprintf(sz, TEXT("%u‰æ"), *it);
    ::SendMessage(m_hListBox1, LB_ADDSTRING, 0, (LPARAM)sz);
  }

  // fill radical list box
  for (size_t i = 0; i < m_radical_table.size(); ++i) {
    const RADICAL_ENTRY& entry = m_radical_table[i];
    LPCTSTR psz = entry.readings.c_str();
    ::SendMessage(m_hListBox2, LB_ADDSTRING, 0, (LPARAM)psz);
    ::SendMessage(m_hListBox2, LB_SETITEMDATA, i, (LPARAM)entry.radical_id);
  }

  // set image list to list view
  ListView_SetImageList(m_hListView, m_himlKanji, LVSIL_NORMAL);

  // set font
  ::SendMessage(m_hListBox2, WM_SETFONT, (WPARAM)m_hSmallFont, FALSE);
  ::SendMessage(m_hListView, WM_SETFONT, (WPARAM)m_hNormalFont, FALSE);
  ::SendMessage(m_hListBox1, WM_SETFONT, (WPARAM)m_hLargeFont, FALSE);

  // show child windows
  ::ShowWindow(m_hListBox1, SW_SHOWNOACTIVATE);
  ::ShowWindow(m_hListView, SW_SHOWNOACTIVATE);

  // highlight
  TabCtrl_HighlightItem(m_hTabCtrl, 0, TRUE);

  // set timer
  ::SetTimer(hWnd, 666, INTERVAL_MILLISECONDS, NULL);

  return TRUE;
} // ImePad::OnCreate

void ImePad::OnSize(HWND hWnd) {
  RECT rc;
  ::GetClientRect(m_hWnd, &rc);

  ::MoveWindow(m_hTabCtrl, rc.left, rc.top,
    rc.right - rc.left, rc.bottom - rc.top, TRUE);

  TabCtrl_AdjustRect(m_hTabCtrl, FALSE, &rc);

  const INT cx1 = 90;
  const INT cx2 = 140;
  if (::IsWindowVisible(m_hListBox1)) {
    ::MoveWindow(m_hListBox1, rc.left, rc.top,
      cx1, rc.bottom - rc.top, TRUE);
    ::MoveWindow(m_hListView, rc.left + cx1, rc.top,
      rc.right - rc.left - cx1, rc.bottom - rc.top, TRUE);
  } else {
    ::MoveWindow(m_hListBox2, rc.left, rc.top,
      cx2, rc.bottom - rc.top, TRUE);
    ::MoveWindow(m_hListView, rc.left + cx2, rc.top,
      rc.right - rc.left - cx2, rc.bottom - rc.top, TRUE);
  }
  ListView_Arrange(m_hListView, LVA_DEFAULT);
}

void ImePad::OnLV1StrokesChanged(HWND hWnd) {
  ::SendMessage(m_hListView, WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), 0);
  ::SendMessage(m_hListView, WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), 0);
  ListView_DeleteAllItems(m_hListView);

  INT i = SendMessage(m_hListBox1, LB_GETCURSEL, 0, 0);
  if (i == LB_ERR) {
    OnSize(m_hWnd);
    return;
  }

  TCHAR sz[32];
  SendMessage(m_hListBox1, LB_GETTEXT, i, (LPARAM)sz);
  int strokes = _ttoi(sz);

  LV_ITEMW lv_item;
  ZeroMemory(&lv_item, sizeof(lv_item));
  lv_item.mask = LVIF_TEXT | LVIF_IMAGE;
  for (size_t i = 0; i < m_kanji_table.size(); ++i) {
    const KANJI_ENTRY& entry = m_kanji_table[i];
    if (entry.strokes != strokes) {
      continue;
    }
    lv_item.iItem = (INT)i;
    lv_item.iSubItem = 0;
    lv_item.pszText = const_cast<WCHAR *>(entry.readings.c_str());
    lv_item.iImage = (int)i;
    ListView_InsertItem(m_hListView, &lv_item);
  }
  OnSize(m_hWnd);
}

void ImePad::OnLV2StrokesChanged(HWND hWnd) {
  ::SendMessage(m_hListView, WM_HSCROLL, MAKEWPARAM(SB_LEFT, 0), 0);
  ::SendMessage(m_hListView, WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), 0);
  ListView_DeleteAllItems(m_hListView);

  INT i = SendMessage(m_hListBox2, LB_GETCURSEL, 0, 0);
  if (i == LB_ERR) {
    OnSize(m_hWnd);
    return;
  }

  WORD radical_id2 = m_radical_table[i].radical_id2;

  LV_ITEMW lv_item;
  ZeroMemory(&lv_item, sizeof(lv_item));
  lv_item.mask = LVIF_TEXT | LVIF_IMAGE;
  for (size_t i = 0; i < m_kanji_table.size(); ++i) {
    const KANJI_ENTRY& entry = m_kanji_table[i];
    if (entry.radical_id2 != radical_id2) {
      continue;
    }
    lv_item.iItem = (INT)i;
    lv_item.iSubItem = 0;
    lv_item.pszText = const_cast<WCHAR *>(entry.readings.c_str());
    lv_item.iImage = (int)i;
    ListView_InsertItem(m_hListView, &lv_item);
  }
  OnSize(m_hWnd);
}

void ImePad::OnTimer(HWND hWnd) {
  static DWORD s_dwThreadID = ::GetWindowThreadProcessId(hWnd, NULL);
  HWND hwndTarget = ::GetForegroundWindow();
  if (hwndTarget == hWnd && m_hwndOld != NULL) {
    ::SetForegroundWindow(m_hwndOld);
  } else {
    if (hwndTarget != m_hwndOld) {
      DWORD dwTargetThreadID;
      dwTargetThreadID = ::GetWindowThreadProcessId(hwndTarget, NULL);
      ::AttachThreadInput(s_dwThreadID, dwTargetThreadID, TRUE);
      m_hwndOld = hwndTarget;
    }
  }
}

void ImePad::OnGetMinMaxInfo(LPMINMAXINFO pmmi) {
  pmmi->ptMinTrackSize.x = 200;
  pmmi->ptMinTrackSize.y = 200;
}

void ImePad::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {
  if (HIWORD(wParam) == LBN_SELCHANGE) {
    switch (LOWORD(wParam)) {
    case 2:
      OnLV1StrokesChanged(hWnd);
      break;
    case 3:
      OnLV2StrokesChanged(hWnd);
      break;
    }
    return;
  }
  if (LOWORD(wParam) == IDCANCEL) {
    EndDialog(hWnd, IDCANCEL);
    return;
  }
}

void ImePad::MySendInput(WCHAR ch) {
  if (::GetForegroundWindow() == m_hWnd) {
    ::SetForegroundWindow(m_hwndOld);
  }
  INPUT input;
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = 0;
  input.ki.wScan = ch;
  input.ki.dwFlags = KEYEVENTF_UNICODE;
  input.ki.time = 0;
  input.ki.dwExtraInfo = 0;
  ::SendInput(1, &input, sizeof(INPUT));
}

void ImePad::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) {
  NMHDR *pnmhdr = (NMHDR *)lParam;
  switch (pnmhdr->code) {
  case TCN_SELCHANGE:
    {
      INT iCurSel = TabCtrl_GetCurSel(m_hTabCtrl);
      TabCtrl_HighlightItem(m_hTabCtrl, 0, FALSE);
      TabCtrl_HighlightItem(m_hTabCtrl, 1, FALSE);
      TabCtrl_HighlightItem(m_hTabCtrl, iCurSel, TRUE);
      switch (iCurSel) {
      case 0:
        ::ShowWindow(m_hListBox1, SW_SHOWNOACTIVATE);
        ::ShowWindow(m_hListBox2, SW_HIDE);
        OnLV1StrokesChanged(hWnd);
        break;
      case 1:
        ::ShowWindow(m_hListBox1, SW_HIDE);
        ::ShowWindow(m_hListBox2, SW_SHOWNOACTIVATE);
        OnLV2StrokesChanged(hWnd);
        break;
      default:
        break;
      }
    }
    break;
  case NM_DBLCLK:
    if (pnmhdr->hwndFrom == m_hListView) {
      INT iItem = ListView_GetNextItem(m_hListView, -1, LVNI_ALL | LVNI_SELECTED);
      if (iItem == -1) {
        break;
      }
      LV_ITEM item;
      ZeroMemory(&item, sizeof(item));
      item.mask = LVIF_IMAGE;
      item.iItem = iItem;
      item.iSubItem = 0;
      ListView_GetItem(m_hListView, &item);
      WCHAR ch = m_kanji_table[item.iImage].kanji_char;
      MySendInput(ch);
    }
    break;
  }
}

/*static*/ LRESULT CALLBACK
ImePad::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  ImePad *pImePad;
  pImePad = (ImePad *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

  switch (uMsg) {
  case WM_CREATE:
    pImePad = new ImePad;
    if (!pImePad->OnCreate(hWnd)) {
      delete pImePad;
      return -1;
    }
    pImePad->m_hWnd = hWnd;
    break;

  case WM_SIZE:
    pImePad->OnSize(hWnd);
    break;

  case WM_NOTIFY:
    pImePad->OnNotify(hWnd, wParam, lParam);
    break;

  case WM_COMMAND:
    pImePad->OnCommand(hWnd, wParam, lParam);
    break;

  case WM_DESTROY:
    KillTimer(hWnd, 666);
    ::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
    ::SetWindowLongPtr(pImePad->m_hTabCtrl, GWLP_USERDATA, 0);
    delete pImePad;
    PostQuitMessage(0);
    break;

  case WM_MOUSEACTIVATE:
    return MA_NOACTIVATE;

  case WM_TIMER:
    if (wParam == 666) {
      pImePad->OnTimer(hWnd);
    }
    break;

  case WM_GETMINMAXINFO:
    pImePad->OnGetMinMaxInfo((LPMINMAXINFO)lParam);
    break;

  default:
    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return 0;
} // ImePad::WindowProc

//////////////////////////////////////////////////////////////////////////////

extern "C"
INT WINAPI
wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR    lpCmdLine,
  INT       nCmdShow)
{
  HWND hwndFound = ::FindWindowW(szImePadClassName, LoadStringDx(IDM_IME_PAD));
  if (hwndFound) {
    ::FlashWindow(hwndFound, TRUE);
    return 0;
  }

  g_hInst = hInstance;
  ::InitCommonControls();

  // register class of IME Pad window.
  WNDCLASSEX wcx;
  wcx.cbSize = sizeof(WNDCLASSEX);
#ifdef IME
  wcx.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_IME;
#else
  wcx.style = CS_DBLCLKS;
#endif
  wcx.lpfnWndProc = ImePad::WindowProc;
  wcx.cbClsExtra = 0;
  wcx.cbWndExtra = 0;
  wcx.hInstance = hInstance;
  wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcx.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  wcx.lpszMenuName = NULL;
  wcx.lpszClassName = szImePadClassName;
  wcx.hIconSm = NULL;
  if (!::RegisterClassEx(&wcx)) {
    MessageBoxA(NULL, "RegisterClassEx", NULL, MB_ICONERROR);
    return 1;
  }

  if (!ImePad::Create(NULL)) {
    MessageBoxA(NULL, "CreateWindowEx", NULL, MB_ICONERROR);
    return 2;
  }

  MSG msg;
  while (::GetMessage(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  return INT(msg.wParam);
} // wWinMain

//////////////////////////////////////////////////////////////////////////////
