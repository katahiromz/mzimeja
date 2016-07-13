#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
using namespace std;

#define UNBOOST_USE_STRING_ALGORITHM
#include "../../include/unboost.hpp"

HINSTANCE g_hInstance;
HWND  g_hTabCtrl;
HWND  g_hListView;
HWND  g_hListBox1;
HWND  g_hListBox2;
HBITMAP g_hbmRadical;
HFONT g_hSmallFont;
HFONT g_hNormalFont;
HFONT g_hLargeFont;

struct KANJI_ENTRY {
  WORD          kanji_id;
  WCHAR         kanji_char;
  WORD          radical_id2;
  WORD          strokes;
  std::wstring  readings;
};
std::vector<KANJI_ENTRY> kanji_table;

// strokes |--> kanji_ids
std::map<WORD, std::vector<WORD> > kanji_stroke_map;

struct RADICAL_ENTRY {
  WORD          radical_id;
  WORD          radical_id2;
  WORD          strokes;
  std::wstring  readings;
};
std::vector<RADICAL_ENTRY> radical_table;

// strokes |--> radical_id's
std::map<WORD, std::vector<WORD> > radical_stroke_map;

// radical_id |--> radical_id2
std::map<WORD, WORD> radical_id_map;

// radical_id2 |--> kanji_id's
std::map<WORD, std::vector<WORD> > radical2_to_kanji_map;

HIMAGELIST g_himlKanji;
HIMAGELIST g_himlRadical;

HBITMAP Create24BppBitmap(HDC hDC, INT cx, INT cy) {
  BITMAPINFO bi;
  ZeroMemory(&bi, sizeof(bi));
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = cx;
  bi.bmiHeader.biHeight = cy;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 24;

  LPVOID pvBits;
  return CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
}

inline bool radical_compare(const RADICAL_ENTRY& entry1, const RADICAL_ENTRY& entry2) {
  return entry1.strokes < entry2.strokes;
}

void CreateAllFonts() {
  LOGFONT lf;
  ZeroMemory(&lf, sizeof(lf));
  lf.lfHeight = 28;
  lf.lfCharSet = SHIFTJIS_CHARSET;
  lf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
  HFONT hFont = CreateFontIndirect(&lf);

  lf.lfHeight = 10;
  g_hSmallFont = CreateFontIndirect(&lf);
  lf.lfHeight = 12;
  g_hNormalFont = CreateFontIndirect(&lf);
  lf.lfHeight = 28;
  g_hLargeFont = CreateFontIndirect(&lf);
}

void LoadData() {
  char buf[256];
  wchar_t wbuf[256];

  CreateAllFonts();

  FILE *fp = fopen("..\\..\\res\\kanji.dat", "rb");
  while (fgets(buf, 256, fp) != NULL) {
    if (buf[0] == ';') continue;
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
    std::wstring str = wbuf;
    unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
    std::vector<std::wstring> vec;
    unboost::split(vec, str, unboost::is_any_of(L"\t"));
    KANJI_ENTRY entry;
    entry.kanji_id = _wtoi(vec[0].c_str());
    entry.kanji_char = vec[1][0];
    entry.radical_id2 = _wtoi(vec[2].c_str());
    entry.strokes = _wtoi(vec[3].c_str());
    entry.readings = vec[4];
    kanji_table.push_back(entry);
    kanji_stroke_map[entry.strokes].push_back(entry.kanji_id);
  }
  fclose(fp);

  fp = fopen("..\\..\\res\\radical.dat", "rb");
  while (fgets(buf, 256, fp) != NULL) {
    if (buf[0] == ';') continue;
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
    std::wstring str = wbuf;
    unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
    std::vector<std::wstring> vec;
    unboost::split(vec, str, unboost::is_any_of(L"\t"));
    RADICAL_ENTRY entry;
    entry.radical_id = _wtoi(vec[0].c_str());
    entry.radical_id2 = _wtoi(vec[1].c_str());
    entry.strokes = _wtoi(vec[3].c_str());
    entry.readings = vec[4];
    radical_table.push_back(entry);
    radical_stroke_map[entry.strokes].push_back(entry.radical_id);
    radical_id_map[entry.radical_id] = entry.radical_id2;
  }
  fclose(fp);

  for (size_t i = 0; i < kanji_table.size(); ++i) {
    const KANJI_ENTRY& kanji = kanji_table[i];
    radical2_to_kanji_map[kanji.radical_id2].push_back(kanji.kanji_id);
  }

  std::sort(radical_table.begin(), radical_table.end(), radical_compare);

  g_himlKanji = ImageList_Create(32, 32, ILC_COLOR,
    (INT)kanji_table.size(), 1);
  g_himlRadical = ImageList_Create(24, 24, ILC_COLOR,
    (INT)radical_table.size(), 1);

  HDC hDC = CreateCompatibleDC(NULL);
  SelectObject(hDC, GetStockObject(WHITE_BRUSH));
  SelectObject(hDC, GetStockObject(BLACK_PEN));
  HGDIOBJ hFontOld = SelectObject(hDC, g_hLargeFont);
  for (size_t i = 0; i < kanji_table.size(); ++i) {
    HBITMAP hbm = Create24BppBitmap(hDC, 32, 32);
    HGDIOBJ hbmOld = SelectObject(hDC, hbm);
    {
      Rectangle(hDC, 0, 0, 32, 32);
      RECT rc;
      SetRect(&rc, 0, 0, 32, 32);
      DrawTextW(hDC, &kanji_table[i].kanji_char, 1, &rc,
        DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
    }
    SelectObject(hDC, hbmOld);
    ImageList_Add(g_himlKanji, hbm, NULL);
    DeleteObject(hbm);
  }
  SelectObject(hDC, hFontOld);

  HDC hDC2 = CreateCompatibleDC(NULL);
  HGDIOBJ hbm2Old = SelectObject(hDC2, g_hbmRadical);
  for (size_t i = 0; i < radical_table.size(); ++i) {
    HBITMAP hbm = Create24BppBitmap(hDC, 24, 24);
    HGDIOBJ hbmOld = SelectObject(hDC, hbm);
    {
      BitBlt(hDC, 0, 0, 24, 24, hDC2, i * 24, 0, SRCCOPY);
    }
    SelectObject(hDC, hbmOld);
    ImageList_Add(g_himlRadical, hbm, NULL);
    DeleteObject(hbm);
  }
  SelectObject(hDC2, hbm2Old);
  DeleteDC(hDC2);

  DeleteDC(hDC);
}

void OnLV1StrokesChanged(HWND hWnd) {
  ListView_DeleteAllItems(g_hListView);

  INT i = SendMessage(g_hListBox1, LB_GETCURSEL, 0, 0);
  if (i == LB_ERR) {
    return;
  }

  TCHAR sz[32];
  SendMessage(g_hListBox1, LB_GETTEXT, i, (LPARAM)sz);
  int strokes = _ttoi(sz);

  LV_ITEMW lv_item;
  ZeroMemory(&lv_item, sizeof(lv_item));
  lv_item.mask = LVIF_TEXT | LVIF_IMAGE;
  for (size_t i = 0; i < kanji_table.size(); ++i) {
    if (kanji_table[i].strokes != strokes) {
      continue;
    }
    lv_item.iItem = (INT)i;
    lv_item.iSubItem = 0;
    lv_item.pszText = (WCHAR *)kanji_table[i].readings.c_str();
    lv_item.iImage = (int)i;
    ListView_InsertItem(g_hListView, &lv_item);
  }
}

void OnLV2StrokesChanged(HWND hWnd) {
  ListView_DeleteAllItems(g_hListView);

  INT i = SendMessage(g_hListBox2, LB_GETCURSEL, 0, 0);
  if (i == LB_ERR) {
    return;
  }

  WORD radical_id2 = radical_table[i].radical_id2;

  LV_ITEMW lv_item;
  ZeroMemory(&lv_item, sizeof(lv_item));
  lv_item.mask = LVIF_TEXT | LVIF_IMAGE;
  for (size_t i = 0; i < kanji_table.size(); ++i) {
    if (kanji_table[i].radical_id2 != radical_id2) {
      continue;
    }
    lv_item.iItem = (INT)i;
    lv_item.iSubItem = 0;
    lv_item.pszText = (WCHAR *)kanji_table[i].readings.c_str();
    lv_item.iImage = (int)i;
    ListView_InsertItem(g_hListView, &lv_item);
  }
}

WNDPROC fnWndProcOld;

void OnDrawItem(HWND hWnd, LPDRAWITEMSTRUCT lpDraw) {
  INT id = lpDraw->itemID;  
  const RADICAL_ENTRY& entry = radical_table[id];
  RECT rc = lpDraw->rcItem;
  if (lpDraw->itemState & ODS_SELECTED) {
    FillRect(lpDraw->hDC, &rc, (HBRUSH)(COLOR_HIGHLIGHT + 1));
    ImageList_Draw(g_himlRadical, entry.radical_id - 1, lpDraw->hDC,
      rc.left, rc.top, ILD_NORMAL);
    rc.left += 24;
    SetBkMode(lpDraw->hDC, TRANSPARENT);
    SetTextColor(lpDraw->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    DrawText(lpDraw->hDC, entry.readings.c_str(), -1,
      &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE |
      DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
  } else {
    FillRect(lpDraw->hDC, &rc, (HBRUSH)(COLOR_WINDOW + 1));
    ImageList_Draw(g_himlRadical, entry.radical_id - 1, lpDraw->hDC,
      rc.left, rc.top, ILD_NORMAL);
    rc.left += 24;
    SetBkMode(lpDraw->hDC, TRANSPARENT);
    SetTextColor(lpDraw->hDC, GetSysColor(COLOR_WINDOWTEXT));
    DrawText(lpDraw->hDC, entry.readings.c_str(), -1,
      &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE |
      DT_NOCLIP | DT_NOPREFIX | DT_END_ELLIPSIS);
  }
}

void OnEraseBkGnd(HWND hWnd) {
  RECT rc;
  GetClientRect(hWnd, &rc);

  HDC hDC = GetDC(hWnd);
  SetTextColor(hDC, GetSysColor(COLOR_BTNTEXT));
  SetBkMode(hDC, TRANSPARENT);
  SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
  DrawText(hDC, TEXT("(èÌópäøéöÇÃÇ›Ç≈Ç∑)"), -1,
    &rc, DT_RIGHT | DT_TOP | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
  ReleaseDC(hWnd, hDC);
}

LRESULT CALLBACK
MyTabCtrlWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LPMEASUREITEMSTRUCT lpMeasure;
  LPDRAWITEMSTRUCT lpDraw;
  switch (uMsg) {
  case WM_MEASUREITEM:
    lpMeasure = (LPMEASUREITEMSTRUCT)lParam;
    lpMeasure->itemWidth = 64;
    lpMeasure->itemHeight = 24;
    break;
  case WM_DRAWITEM:
    lpDraw = (LPDRAWITEMSTRUCT)lParam;
    OnDrawItem(hWnd, lpDraw);
    break;
  case WM_COMMAND:
    if (HIWORD(wParam) == LBN_SELCHANGE) {
      PostMessage(GetParent(hWnd), uMsg, wParam, lParam);
    }
    break;
  case WM_ERASEBKGND:
    CallWindowProc(fnWndProcOld, hWnd, uMsg, wParam, lParam);
    OnEraseBkGnd(hWnd);
    return 0;
  default:
    break;
  }
  return CallWindowProc(fnWndProcOld, hWnd, uMsg, wParam, lParam);
}

BOOL OnInitDialog(HWND hWnd) {
  g_hbmRadical = LoadBitmap(g_hInstance, MAKEINTRESOURCE(1));

  RECT rc;
  GetClientRect(hWnd, &rc);

  LoadData();

  g_hTabCtrl = CreateWindow(WC_TABCONTROL, NULL,
    WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TCS_FOCUSNEVER,
    0, 0, rc.right - rc.left, rc.bottom - rc.top,
    hWnd, (HMENU)1, g_hInstance, NULL);
  fnWndProcOld = (WNDPROC)
    SetWindowLongPtr(g_hTabCtrl, GWLP_WNDPROC, (LONG_PTR)MyTabCtrlWndProc);

  TC_ITEM item;
  item.mask = TCIF_TEXT;
  item.pszText = TEXT("ëçéöâÊ");
  TabCtrl_InsertItem(g_hTabCtrl, 0, &item);
  item.pszText = TEXT("ïîéÒ");
  TabCtrl_InsertItem(g_hTabCtrl, 1, &item);

  TabCtrl_AdjustRect(g_hTabCtrl, FALSE, &rc);

  g_hListBox1 = CreateWindowEx(WS_EX_CLIENTEDGE,
    TEXT("LISTBOX"), NULL, WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    g_hTabCtrl, (HMENU)2, g_hInstance, NULL);

  g_hListBox2 = CreateWindowEx(WS_EX_CLIENTEDGE,
    TEXT("LISTBOX"), NULL, WS_CHILD | WS_HSCROLL | WS_VSCROLL | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    g_hTabCtrl, (HMENU)3, g_hInstance, NULL);
  SendMessage(g_hListBox2, LB_SETITEMHEIGHT, 0, 24);

  g_hListView = CreateWindowEx(WS_EX_CLIENTEDGE,
    WC_LISTVIEW, NULL, WS_CHILD | LVS_ICON,
    rc.left, rc.top, 120, rc.bottom - rc.top,
    g_hTabCtrl, (HMENU)4, g_hInstance, NULL);

  std::set<WORD> strokes;
  for (size_t i = 0; i < kanji_table.size(); ++i) {
    strokes.insert(kanji_table[i].strokes);
  }

  std::set<WORD>::iterator it, end = strokes.end();
  TCHAR sz[128];
  for (it = strokes.begin(); it != end; ++it) {
    wsprintf(sz, TEXT("%uâÊ"), *it);
    SendMessage(g_hListBox1, LB_ADDSTRING, 0, (LPARAM)sz);
  }

  for (size_t i = 0; i < radical_table.size(); ++i) {
    const RADICAL_ENTRY& entry = radical_table[i];
    LPCTSTR psz = entry.readings.c_str();
    SendMessage(g_hListBox2, LB_ADDSTRING, 0, (LPARAM)psz);
    SendMessage(g_hListBox2, LB_SETITEMDATA, i, (LPARAM)entry.radical_id);
  }

  ListView_SetImageList(g_hListView, g_himlKanji, LVSIL_NORMAL);

  SendMessage(g_hListBox2, WM_SETFONT, (WPARAM)g_hSmallFont, FALSE);
  SendMessage(g_hListView, WM_SETFONT, (WPARAM)g_hNormalFont, FALSE);
  SendMessage(g_hListBox1, WM_SETFONT, (WPARAM)g_hLargeFont, FALSE);
  ShowWindow(g_hListBox1, SW_SHOWNOACTIVATE);
  ShowWindow(g_hListView, SW_SHOWNOACTIVATE);
  TabCtrl_HighlightItem(g_hTabCtrl, 0, TRUE);

  return TRUE;
}

void OnSize(HWND hWnd) {
  RECT rc;
  GetClientRect(hWnd, &rc);

  MoveWindow(g_hTabCtrl, rc.left, rc.top,
    rc.right - rc.left, rc.bottom - rc.top, TRUE);

  TabCtrl_AdjustRect(g_hTabCtrl, FALSE, &rc);

  MoveWindow(g_hListBox1, rc.left, rc.top,
    120, rc.bottom - rc.top, TRUE);
  MoveWindow(g_hListBox2, rc.left, rc.top,
    120, rc.bottom - rc.top, TRUE);
  MoveWindow(g_hListView, rc.left + 120, rc.top,
    rc.right - rc.left - 120, rc.bottom - rc.top, TRUE);
}

void OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) {
  TCHAR sz[32];
  NMHDR *pnmhdr = (NMHDR *)lParam;
  switch (pnmhdr->code) {
  case TCN_SELCHANGE:
    {
      INT iCurSel = TabCtrl_GetCurSel(g_hTabCtrl);
      TabCtrl_HighlightItem(g_hTabCtrl, 0, FALSE);
      TabCtrl_HighlightItem(g_hTabCtrl, 1, FALSE);
      TabCtrl_HighlightItem(g_hTabCtrl, iCurSel, TRUE);
      switch (iCurSel) {
      case 0:
        ShowWindow(g_hListBox1, SW_SHOWNOACTIVATE);
        ShowWindow(g_hListBox2, SW_HIDE);
        OnLV1StrokesChanged(hWnd);
        break;
      case 1:
        ShowWindow(g_hListBox1, SW_HIDE);
        ShowWindow(g_hListBox2, SW_SHOWNOACTIVATE);
        OnLV2StrokesChanged(hWnd);
        break;
      default:
        break;
      }
    }
    break;
  }
}

void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {
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

INT_PTR CALLBACK
DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_INITDIALOG:
    return OnInitDialog(hWnd);
  case WM_COMMAND:
    OnCommand(hWnd, wParam, lParam);
    break;
  case WM_SIZE:
    OnSize(hWnd);
    break;
  case WM_NOTIFY:
    OnNotify(hWnd, wParam, lParam);
    break;
  }
  return FALSE;
}

extern "C"
INT WINAPI wWinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPWSTR lpCmdLine,
  INT nCmdShow)
{
  g_hInstance = hInstance;
  InitCommonControls();
  DialogBox(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc);
  return 0;
}
