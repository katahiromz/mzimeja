// disabled_window.cpp
#include <windows.h>
#include <stdio.h>

const TCHAR g_szName[] = TEXT("disabled_window");

HINSTANCE g_hInstance;
HWND g_hMainWnd;
HWND g_hListBox;
BOOL g_bCaptured = FALSE;
POINT g_ptPrev = {-1, -1};

enum MY_HITTEST {
  MH_NONE,
  MH_CAPTION,
  MH_CLIENT,
  MH_CHILDWND
};

void WriteLog(char *format, ...) {
  char buf[1024];
  va_list va;
  va_start(va, format);
  wvsprintfA(buf, format, va);
  va_end(va);
  FILE *fp = fopen("disabled_window.log", "a");
  fputs(buf, fp);
  fclose(fp);
}

BOOL OnCreate(HWND hWnd) {
  g_hListBox = CreateWindow(TEXT("LISTBOX"), NULL,
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER,
    10, 10, 100, 120, hWnd, (HMENU)1, g_hInstance, NULL);
  if (g_hListBox == NULL) {
    return FALSE;
  }

  TCHAR sz[32];
  for (INT i = 0; i < 100; ++i) {
    wsprintf(sz, TEXT("item #%u"), i);
    SendMessage(g_hListBox, LB_ADDSTRING, 0, (LPARAM)sz);
  }
  return TRUE;
}

void OnWrappedMouseMove(HWND hWnd, POINT pt, BOOL bDown) {
  POINT ptScreen = pt;
  ClientToScreen(hWnd, &ptScreen);
  WriteLog("OnWrappedMouseMove: pt: (%d, %d)\n", pt.x, pt.y);
  WriteLog("OnWrappedMouseMove: ptScreen: (%d, %d)\n", ptScreen.x, ptScreen.y);
  if (g_bCaptured) {
    if (bDown && GetCapture() == hWnd) {
      if (g_ptPrev.x != -1 && g_ptPrev.y != -1) {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        MoveWindow(hWnd,
          rc.left + (ptScreen.x - g_ptPrev.x),
          rc.top + (ptScreen.y - g_ptPrev.y),
          rc.right - rc.left, rc.bottom - rc.top,
          TRUE);
      }
      g_ptPrev = ptScreen;
    } else {
      g_ptPrev.x = -1;
      g_ptPrev.y = -1;
      ReleaseCapture();
      g_bCaptured = FALSE;
    }
  }
}

MY_HITTEST GetHitTest(HWND hWnd, POINT pt, HWND *phChild) {
  HWND hChild;
  hChild = ChildWindowFromPointEx(hWnd, pt, CWP_SKIPINVISIBLE);
  if (hChild && hWnd != hChild) {
    *phChild = hChild;
    return MH_CHILDWND;
  } else {
    *phChild = NULL;
    return MH_CAPTION;
  }
}

void OnWrappedLButton(HWND hWnd, POINT pt, BOOL bDown) {
  HWND hChild;
  MY_HITTEST nHitTest = GetHitTest(hWnd, pt, &hChild);

  switch (nHitTest) {
  case MH_CAPTION:
    WriteLog("OnWrappedLButton: MH_CAPTION\n");
    g_ptPrev.x = g_ptPrev.y = -1;
    if (bDown) {
      SetCapture(hWnd);
      g_bCaptured = TRUE;
    } else {
      ReleaseCapture();
      g_bCaptured = FALSE;
    }
    break;
  case MH_CHILDWND:
    // TODO: accessing child controls
    WriteLog("OnWrappedLButton: MH_CHILDWND\n");
    break;
  case MH_CLIENT:
    WriteLog("OnWrappedLButton: MH_CLIENT\n");
    break;
  default:
    break;
  }
}

void OnEmulatedMouseUI(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(hWnd, &pt);

  switch (uMsg) {
  case WM_SETCURSOR:
    switch (HIWORD(lParam)) {
    case WM_MOUSEMOVE:
      if (g_bCaptured) {
        OnWrappedMouseMove(hWnd, pt, GetAsyncKeyState(VK_LBUTTON) < 0);
      }
      break;
    case WM_LBUTTONDOWN:
      OnWrappedLButton(hWnd, pt, TRUE);
      break;
    case WM_LBUTTONUP:
      OnWrappedLButton(hWnd, pt, FALSE);
      break;
    case WM_RBUTTONDOWN:
      break;
    case WM_RBUTTONUP:
      break;
    }
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    break;
  case WM_MOUSEMOVE:
    // This message comes from the captured window.
    if (g_bCaptured) {
      OnWrappedMouseMove(hWnd, pt, GetAsyncKeyState(VK_LBUTTON) < 0);
    }
    break;
  case WM_LBUTTONDOWN:
    // This message comes from the captured window.
    OnWrappedLButton(hWnd, pt, TRUE);
    break;
  case WM_LBUTTONUP:
    // This message comes from the captured window.
    OnWrappedLButton(hWnd, pt, FALSE);
    break;
  case WM_RBUTTONDOWN:
    // This message comes from the captured window.
    break;
  case WM_RBUTTONUP:
    // This message comes from the captured window.
    break;
  }
}

LRESULT CALLBACK
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_CREATE:
    WriteLog("WM_CREATE\n");
    if (!OnCreate(hWnd)) {
      return -1;
    }
    break;
  case WM_MOUSEMOVE:
    WriteLog("WM_MOUSEMOVE\n");
    OnEmulatedMouseUI(hWnd, uMsg, wParam, lParam);
    break;
  case WM_LBUTTONUP:
    WriteLog("WM_LBUTTONUP\n");
    OnEmulatedMouseUI(hWnd, uMsg, wParam, lParam);
    break;
  case WM_LBUTTONDOWN:
    WriteLog("WM_LBUTTONDOWN\n");
    OnEmulatedMouseUI(hWnd, uMsg, wParam, lParam);
    break;
  case WM_RBUTTONDOWN:
    WriteLog("WM_RBUTTONDOWN\n");
    OnEmulatedMouseUI(hWnd, uMsg, wParam, lParam);
    break;
  case WM_RBUTTONUP:
    WriteLog("WM_RBUTTONUP\n");
    OnEmulatedMouseUI(hWnd, uMsg, wParam, lParam);
    break;
  case WM_SETCURSOR:
    WriteLog("WM_SETCURSOR\n");
    OnEmulatedMouseUI(hWnd, uMsg, wParam, lParam);
    break;
  case WM_MOVE:
    break;
  default:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return 0;
}

INT WINAPI WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  INT nCmdShow)
{
  g_hInstance = hInstance;

  WNDCLASS wc;
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = WindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = g_szName;
  if (!RegisterClass(&wc)) {
    MessageBox(NULL, TEXT("RegisterClass"), NULL, MB_ICONERROR);
    return 1;
  }

  DWORD style =
    WS_POPUP | WS_DISABLED | WS_CLIPSIBLINGS |
    WS_BORDER | WS_THICKFRAME;
  DWORD exstyle =
    WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;
  g_hMainWnd = CreateWindowEx(exstyle, g_szName, g_szName,
    style, 100, 100, 200, 200, NULL, NULL, hInstance, NULL);
  if (g_hMainWnd == NULL) {
    MessageBox(NULL, TEXT("CreateWindowEx"), NULL, MB_ICONERROR);
    return 2;
  }

  ShowWindow(g_hMainWnd, nCmdShow);
  UpdateWindow(g_hMainWnd);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (INT)msg.wParam;
}
