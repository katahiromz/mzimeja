// make_radical_image.cpp
// (Japanese, Shift_JIS)
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
using namespace std;

#define FONT_SIZE 24
#define ICON_SIZE 24

typedef struct tagBITMAPINFOEX
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
} BITMAPINFOEX, FAR * LPBITMAPINFOEX;
 
BOOL SaveBitmapToFile(LPCTSTR pszFileName, HBITMAP hbm)
{
    BOOL f;
    BITMAPFILEHEADER bf;
    BITMAPINFOEX bi;
    BITMAPINFOHEADER *pbmih;
    DWORD cb;
    DWORD cColors, cbColors;
    HDC hDC;
    HANDLE hFile;
    LPVOID pBits;
    BITMAP bm;
    DWORD dwError = 0;
 
    if (!GetObject(hbm, sizeof(BITMAP), &bm))
        return FALSE;
 
    pbmih = &bi.bmiHeader;
    ZeroMemory(pbmih, sizeof(BITMAPINFOHEADER));
    pbmih->biSize             = sizeof(BITMAPINFOHEADER);
    pbmih->biWidth            = bm.bmWidth;
    pbmih->biHeight           = bm.bmHeight;
    pbmih->biPlanes           = 1;
    pbmih->biBitCount         = bm.bmBitsPixel;
    pbmih->biCompression      = BI_RGB;
    pbmih->biSizeImage        = bm.bmWidthBytes * bm.bmHeight;
 
    if (bm.bmBitsPixel < 16)
        cColors = 1 << bm.bmBitsPixel;
    else
        cColors = 0;
    cbColors = cColors * sizeof(RGBQUAD);
 
    bf.bfType = 0x4d42;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    cb = sizeof(BITMAPFILEHEADER) + pbmih->biSize + cbColors;
    bf.bfOffBits = cb;
    bf.bfSize = cb + pbmih->biSizeImage;
 
    pBits = HeapAlloc(GetProcessHeap(), 0, pbmih->biSizeImage);
    if (pBits == NULL)
        return FALSE;
 
    f = FALSE;
    hDC = GetDC(NULL);
    if (hDC != NULL)
    {
        if (GetDIBits(hDC, hbm, 0, bm.bmHeight, pBits, (BITMAPINFO*)&bi,
            DIB_RGB_COLORS))
        {
            hFile = CreateFile(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL |
                               FILE_FLAG_WRITE_THROUGH, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                f = WriteFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL) &&
                    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &cb, NULL) &&
                    WriteFile(hFile, bi.bmiColors, cbColors, &cb, NULL) &&
                    WriteFile(hFile, pBits, pbmih->biSizeImage, &cb, NULL);
                if (!f)
                    dwError = GetLastError();
                CloseHandle(hFile);
 
                if (!f)
                    DeleteFile(pszFileName);
            }
            else
                dwError = GetLastError();
        }
        else
            dwError = GetLastError();
        ReleaseDC(NULL, hDC);
    }
    else
        dwError = GetLastError();
 
    HeapFree(GetProcessHeap(), 0, pBits);
    SetLastError(dwError);
    return f;
}

void DrawRectangle(HDC hDC, INT x, INT y) {
  HGDIOBJ hbrOld = SelectObject(hDC, GetStockObject(NULL_BRUSH));
  HGDIOBJ hPenOld = SelectObject(hDC, GetStockObject(BLACK_PEN));
  Rectangle(hDC, x, y, x + ICON_SIZE, y + ICON_SIZE);
  SelectObject(hDC, hPenOld);
  SelectObject(hDC, hbrOld);
}

int main(void) {
  std::vector<std::wstring> table;

  FILE *fp = fopen("..\\res\\radical.dat", "rb");
  char buf[256];
  wchar_t wbuf[256];
  while (fgets(buf, 256, fp) != NULL) {
    if (buf[0] == ';') continue;
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 64);
    table.push_back(wbuf);
  }
  fclose(fp);

  INT count = 258;
  INT cx = 258 * ICON_SIZE;
  INT cy = 1 * ICON_SIZE;
  HDC hDC = CreateCompatibleDC(NULL);

  BITMAPINFO bi;
  ZeroMemory(&bi, sizeof(bi));
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = cx;
  bi.bmiHeader.biHeight = cy;
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 24;

  LPVOID pvBits;
  HBITMAP hbm = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS,
    &pvBits, NULL, 0);
  HGDIOBJ hbmOld = SelectObject(hDC, hbm);

  LOGFONTW lf;
  ZeroMemory(&lf, sizeof(lf));
  lstrcpyW(lf.lfFaceName, L"‰Ô‰€–¾’©A");
  //lf.lfQuality = ANTIALIASED_QUALITY;
  lf.lfHeight = FONT_SIZE;
  //lf.lfWeight = FW_SEMIBOLD;
  HFONT hFont = CreateFontIndirectW(&lf);
  HGDIOBJ hFontOld = SelectObject(hDC, hFont);

  RECT rc;
  SetRect(&rc, 0, 0, cx, cy);
  FillRect(hDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

  for (size_t i = 0; i < count; ++i) {
    std::wstring str = table[i];
    size_t pos = str.find_first_not_of(L"\t0123456789\t\r\n");
    if (pos != std::wstring::npos) {
      INT x = i * ICON_SIZE;
      INT y = 0;
      INT delta = (ICON_SIZE - FONT_SIZE - 2) / 2;
      //DrawRectangle(hDC, x, y);
      TextOutW(hDC, x + delta + 3, y + delta + 1, &str[pos], 1);
    }
  }

  SelectObject(hDC, hFontOld);
  DeleteObject(hFont);
  SelectObject(hDC, hbmOld);

  SaveBitmapToFile(TEXT("radical.bmp"), hbm);

  DeleteObject(hbm);
  DeleteDC(hDC);
}
