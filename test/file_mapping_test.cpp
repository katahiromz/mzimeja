// file_mapping_test.cpp
#include <windows.h>
#include <stdio.h>

int main(void) {
  HANDLE hMapping;
  hMapping = CreateFileMapping(
    INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1,
    TEXT("file_mapping_test"));
  if (hMapping != NULL) {
    BOOL bAlreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);
    LPVOID pv = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 1);
    if (pv) {
      LPBYTE pb = (LPBYTE)pv;
      if (bAlreadyExists) {
        *pb += 1;
      } else {
        *pb = 0;
      }
      BYTE b = *pb;
      UnmapViewOfFile(pv);
      printf("%d\n", (INT)b);
    }
    getchar();
    CloseHandle(hMapping);
  }
  return 0;
}
