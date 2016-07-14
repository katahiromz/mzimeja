// uipad.cpp --- mzimeja IME Pad UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

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
    rc.left += 24;
    ::SetBkMode(lpDraw->hDC, TRANSPARENT);
    ::SetTextColor(lpDraw->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    ::DrawText(lpDraw->hDC, entry.readings.c_str(), -1,
      &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE |
      DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
  } else {
    ::FillRect(lpDraw->hDC, &rc, (HBRUSH)(COLOR_WINDOW + 1));
    ImageList_Draw(m_himlRadical, entry.radical_id - 1, lpDraw->hDC,
      rc.left, rc.top, ILD_NORMAL);
    rc.left += 24;
    ::SetBkMode(lpDraw->hDC, TRANSPARENT);
    ::SetTextColor(lpDraw->hDC, GetSysColor(COLOR_WINDOWTEXT));
    ::DrawText(lpDraw->hDC, entry.readings.c_str(), -1,
      &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE |
      DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
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

  switch (uMsg) {
  case WM_MEASUREITEM:
    lpMeasure = (LPMEASUREITEMSTRUCT)lParam;
    lpMeasure->itemWidth = 64;
    lpMeasure->itemHeight = 24;
    break;
  case WM_DRAWITEM:
    lpDraw = (LPDRAWITEMSTRUCT)lParam;
    pImePad->OnDrawItem(hWnd, lpDraw);
    break;
  case WM_COMMAND:
    if (HIWORD(wParam) == LBN_SELCHANGE) {
      ::PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
    }
    break;
  case WM_ERASEBKGND:
    ::CallWindowProc(
      pImePad->m_fnWndProcOld, hWnd, uMsg, wParam, lParam);
    pImePad->OnEraseBkGnd(hWnd);
    return 0;
  default:
    break;
  }
  return ::CallWindowProc(
    pImePad->m_fnWndProcOld, hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////
// create/destroy

/*static*/ BOOL ImePad::Create(HWND hwndParent) {
  FOOTMARK();

  UIEXTRA *lpUIExtra = LockUIExtra(hwndParent);
  if (lpUIExtra == NULL) {
    return FALSE;
  }

  HWND hImePad = lpUIExtra->hImePad;
  if (!::IsWindow(hImePad)) {
    RECT rc;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

    rc.left = rc.right - 200 - 32;
    rc.top = rc.bottom - 200 - 32;

    DWORD style = WS_DISABLED | WS_POPUP | WS_CAPTION |
                  WS_SYSMENU | WS_SIZEBOX | WS_ACTIVECAPTION;
    DWORD exstyle = WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW;
    hImePad = ::CreateWindowEx(
      exstyle, szImePadClassName, TheIME.LoadSTR(IDM_IME_PAD),
      style, rc.left, rc.top, 200, 200,
      hwndParent, NULL, TheIME.m_hInst, NULL);
    lpUIExtra->hImePad = hImePad;
  }
  RepositionWindow(hImePad);
  ::ShowWindow(hImePad, SW_SHOWNOACTIVATE);
  SetWindowLongPtr(hImePad, FIGWLP_SERVERWND, (LONG_PTR)hwndParent);

  UnlockUIExtra(hwndParent);
  return hImePad != NULL;
} // ImePad_Create

ImePad::ImePad() {
  m_hWnd = NULL;
  m_himlKanji = NULL;
  m_himlRadical = NULL;
  m_hSmallFont = NULL;
  m_hNormalFont = NULL;
  m_hLargeFont = NULL;
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
  std::wstring kanji_file = TheIME.GetSettingString(L"kanji data file");
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
  return FALSE;
} // ImePad::LoadKanjiData

BOOL ImePad::LoadRadicalData() {
  if (m_radical_table.size()) {
    return TRUE;
  }
  char buf[256];
  wchar_t wbuf[256];
  std::wstring radical_file = TheIME.GetSettingString(L"radical data file");
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
  return FALSE;
} // ImePad::CreateAllFonts

BOOL ImePad::LoadRadicalImage() {
  if (m_hbmRadical == NULL) {
    m_hbmRadical = TheIME.LoadBMP(L"RADICALBMP");
  }
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
    m_himlRadical = NULL;
  }

  m_himlRadical = ImageList_Create(24, 24, ILC_COLOR,
    (INT)m_radical_table.size(), 1);
  if (m_himlRadical == NULL) {
    return FALSE;
  }

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

  // create tab control
  m_hTabCtrl = ::CreateWindow(WC_TABCONTROL, NULL,
    WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_FOCUSNEVER,
    0, 0, rc.right - rc.left, rc.bottom - rc.top,
    hWnd, (HMENU)1, TheIME.m_hInst, NULL);

  ::SetWindowLongPtr(m_hTabCtrl, GWLP_USERDATA, (LONG_PTR)this);
  m_fnWndProcOld = (WNDPROC)
    SetWindowLongPtr(m_hTabCtrl, GWLP_WNDPROC,
                     (LONG_PTR)ImePad::TabCtrlWndProc);

  // initialize tab control
  TC_ITEM item;
  item.mask = TCIF_TEXT;
  item.pszText = TheIME.LoadSTR(IDM_STROKES);
  TabCtrl_InsertItem(m_hTabCtrl, 0, &item);
  item.pszText = TheIME.LoadSTR(IDM_RADICALS);
  TabCtrl_InsertItem(m_hTabCtrl, 1, &item);

  // get inner area
  TabCtrl_AdjustRect(m_hTabCtrl, FALSE, &rc);

  // create list box (strokes)
  m_hListBox1 = ::CreateWindowEx(WS_EX_CLIENTEDGE,
    TEXT("LISTBOX"), NULL, WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    m_hTabCtrl, (HMENU)2, TheIME.m_hInst, NULL);

  // create list box (radicals)
  m_hListBox2 = ::CreateWindowEx(WS_EX_CLIENTEDGE,
    TEXT("LISTBOX"), NULL, WS_CHILD | WS_HSCROLL | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    m_hTabCtrl, (HMENU)3, TheIME.m_hInst, NULL);
  ::SendMessage(m_hListBox2, LB_SETITEMHEIGHT, 0, 24);

  // create list view
  m_hListView = ::CreateWindowEx(WS_EX_CLIENTEDGE,
    WC_LISTVIEW, NULL, WS_CHILD | LVS_ICON,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    m_hTabCtrl, (HMENU)4, TheIME.m_hInst, NULL);

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

  return TRUE;
} // ImePad::OnCreate

void ImePad::OnSize(HWND hWnd) {
  RECT rc;
  ::GetClientRect(hWnd, &rc);

  ::MoveWindow(m_hTabCtrl, rc.left, rc.top,
    rc.right - rc.left, rc.bottom - rc.top, TRUE);

  TabCtrl_AdjustRect(m_hTabCtrl, FALSE, &rc);

  ::MoveWindow(m_hListBox1, rc.left, rc.top,
    120, rc.bottom - rc.top, TRUE);
  ::MoveWindow(m_hListBox2, rc.left, rc.top,
    120, rc.bottom - rc.top, TRUE);
  ::MoveWindow(m_hListView, rc.left + 120, rc.top,
    rc.right - rc.left - 120, rc.bottom - rc.top, TRUE);
}

void ImePad::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {
}

void ImePad::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) {
}

/*static*/ LRESULT CALLBACK
ImePad::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  FOOTMARK();
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
    delete pImePad;
    break;

  default:
    if (!IsImeMessage(uMsg))
      return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    break;
  }
  return 0;
} // ImePad::WindowProc

//////////////////////////////////////////////////////////////////////////////
