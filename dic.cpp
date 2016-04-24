// dic.cpp
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "mzimeja.h"
#include "immsec.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

void PASCAL RevertText(HIMC hIMC) {
  CompStr *lpCompStr;
  LPTSTR lpread, lpstr;

  InputContext *lpIMC = TheApp.LockIMC(hIMC);
  if (!lpIMC) return;

  if (lpIMC->HasCandInfo()) {
    // Flush candidate lists.
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      lpIMC->UnlockCandInfo();
    }
    TheApp.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  if (lpIMC->HasCompStr()) {
    lpCompStr = lpIMC->LockCompStr();
    if (lpCompStr) {
      lpstr = lpCompStr->GetCompStr();
      lpread = lpCompStr->GetCompReadStr();
      lHanToZen(lpstr, lpread, lpIMC->Conversion());

      // make attribute
      lpCompStr->dwCursorPos = lstrlen(lpstr);
      // DeltaStart is 0 at RevertText time.
      lpCompStr->dwDeltaStart = 0;

      memset(lpCompStr->GetCompAttr(), 0, lstrlen(lpstr));
      memset(lpCompStr->GetCompReadAttr(), 0, lstrlen(lpread));

      SetClause(lpCompStr->GetCompClause(), lstrlen(lpstr));
      SetClause(lpCompStr->GetCompReadClause(), lstrlen(lpread));
      lpCompStr->dwCompClauseLen = 8;
      lpCompStr->dwCompReadClauseLen = 8;

      // make length
      lpCompStr->dwCompStrLen = lstrlen(lpstr);
      lpCompStr->dwCompReadStrLen = lstrlen(lpread);
      lpCompStr->dwCompAttrLen = lstrlen(lpstr);
      lpCompStr->dwCompReadAttrLen = lstrlen(lpread);

      // Generate messages.
      LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
      TheApp.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

      lpIMC->UnlockCompStr();
    }
  }
  TheApp.UnlockIMC();
}

BOOL PASCAL ConvKanji(HIMC hIMC) {
  BOOL bRc = FALSE;

  if ((GetFileAttributes(TheApp.m_szDicFileName) == 0xFFFFFFFF) ||
      (GetFileAttributes(TheApp.m_szDicFileName) & FILE_ATTRIBUTE_DIRECTORY)) {
    MakeGuideLine(hIMC, MYGL_NODICTIONARY);
  }

  InputContext *lpIMC = TheApp.LockIMC(hIMC);
  if (NULL == lpIMC) return FALSE;

  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (NULL == lpCompStr) {
    TheApp.UnlockIMC();
    return bRc;
  }

  CandInfo *lpCandInfo = lpIMC->LockCandInfo();
  if (NULL == lpCandInfo) {
    lpIMC->UnlockCompStr();
    TheApp.UnlockIMC();
    return bRc;
  }

  // Since IME handles all string as Unicode, convert the CompReadStr
  // from Unicode into multibyte string. Also the dictionary holdsits data
  // as Hiragana, so map the string from Katakana to Hiragana.
  LPTSTR lpT2 = lpCompStr->GetCompReadStr();

  // Get the candidate strings from dic file.
  TCHAR szBuf[256 + 2];
  szBuf[256] = 0;  // Double NULL-terminate
  szBuf[257] = 0;  // Double NULL-terminate
  int nBufLen =
    GetCandidateStringsFromDictionary(lpT2, szBuf, 256, TheApp.m_szDicFileName);

  // Check the result of dic. Because my candidate list has only MAXCANDSTRNUM
  // candidate strings.
  LPTSTR lpT = &szBuf[0];
  int cnt = 0;
  while (*lpT) {
    cnt++;
    lpT += (lstrlen(lpT) + 1);

    if (cnt > MAXCANDSTRNUM) {
      // The dic is too big....
      lpIMC->UnlockCandInfo();
      lpIMC->UnlockCompStr();
      TheApp.UnlockIMC();
      return bRc;
    }
  }

  LPBYTE lpb = lpCompStr->GetCompAttr();
  if (nBufLen < 1) {
    if (!*lpb) {
      // make attribute
      memset(lpCompStr->GetCompAttr(), 1, lstrlen(lpCompStr->GetCompStr()));
      memset(lpCompStr->GetCompReadAttr(), 1,
             lstrlen(lpCompStr->GetCompReadStr()));

      LPARAM lParam =
        GCS_COMPSTR | GCS_CURSORPOS | GCS_COMPATTR | GCS_COMPREADATTR;
      TheApp.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);
    }

    lpIMC->UnlockCandInfo();
    lpIMC->UnlockCompStr();
    TheApp.UnlockIMC();
    return bRc;
  }

  LPTSTR lpstr = szBuf;
  if (!*lpb) {
    // String is not converted yet.
    while (*lpstr) {
      if (0 != lstrcmp(lpstr, lpCompStr->GetCompStr())) {
      set_compstr:
        // Set the composition string to the structure.
        lstrcpy(lpCompStr->GetCompStr(), lpstr);

        lpstr = lpCompStr->GetCompStr();

        // Set the length and cursor position to the structure.
        lpCompStr->dwCompStrLen = lstrlen(lpstr);
        lpCompStr->dwCursorPos = 0;
        // Because MZ-IME does not support clause, DeltaStart is 0 anytime.
        lpCompStr->dwDeltaStart = 0;

        // make attribute
        memset(lpCompStr->GetCompAttr(), 1, lstrlen(lpstr));
        memset(lpCompStr->GetCompReadAttr(), 1,
               lstrlen(lpCompStr->GetCompReadStr()));

        // make clause info
        SetClause(lpCompStr->GetCompClause(), lstrlen(lpstr));
        SetClause(lpCompStr->GetCompReadClause(),
                  lstrlen(lpCompStr->GetCompReadStr()));
        lpCompStr->dwCompClauseLen = 8;
        lpCompStr->dwCompReadClauseLen = 8;

        // Generate messages.
        LPARAM lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
        TheApp.GenerateMessage(WM_IME_COMPOSITION, 0, lParam);

        bRc = TRUE;
        lpIMC->UnlockCandInfo();
        lpIMC->UnlockCompStr();
        TheApp.UnlockIMC();
        return bRc;
      }
      lpstr += (lstrlen(lpstr) + 1);
    }
  } else {
    // String is converted, so that open candidate.
    // generate WM_IME_NOTFIY IMN_OPENCANDIDATE message.
    if (!lpIMC->HasCandInfo()) {
      TheApp.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);
    }

    // Make candidate structures.
    DWORD i = 0;
    lpCandInfo->dwSize = sizeof(MZCAND);
    lpCandInfo->dwCount = 1;
    lpCandInfo->dwOffset[0] = sizeof(CANDIDATEINFO);
    CandList *lpCandList = lpCandInfo->GetList();
    while (*lpstr) {
      lpCandList->dwOffset[i] = lpCandInfo->GetCandOffset(i, lpCandList);
      lstrcpy(lpCandList->GetCandString(i), lpstr);
      lpstr += (lstrlen(lpstr) + 1);
      i++;
    }

    lpCandList->dwSize = sizeof(CANDIDATELIST) +
                         (MAXCANDSTRNUM * (sizeof(DWORD) + MAXCANDSTRSIZE));
    lpCandList->dwStyle = IME_CAND_READ;
    lpCandList->dwCount = i;
    if (i < MAXCANDPAGESIZE)
      lpCandList->dwPageSize = i;
    else
      lpCandList->dwPageSize = MAXCANDPAGESIZE;

    lpCandList->dwSelection++;
    if (lpCandList->dwSelection == i) {
      lpCandList->dwPageStart = 0;
      lpCandList->dwSelection = 0;
    } else if (lpCandList->dwSelection >= MAXCANDPAGESIZE) {
      if (lpCandList->dwPageStart + MAXCANDPAGESIZE < lpCandList->dwCount)
        lpCandList->dwPageStart++;
    }

    // Generate messages.
    TheApp.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

    // If the selected candidate string is changed, the composition string
    // should be updated.
    lpstr = lpCandList->GetCurString();
    goto set_compstr;
  }

  lpIMC->UnlockCandInfo();
  lpIMC->UnlockCompStr();
  TheApp.UnlockIMC();
  return bRc;
}

BOOL MakeResultString(HIMC hIMC, BOOL fFlag) {
  InputContext *lpIMC = TheApp.LockIMC(hIMC);
  CompStr *lpCompStr = lpIMC->LockCompStr();

  if (lpIMC->HasCandInfo()) {
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      lpIMC->UnlockCandInfo();
    }
    TheApp.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
  }

  lstrcpy(lpCompStr->GetResultStr(), lpCompStr->GetCompStr());
  lstrcpy(lpCompStr->GetResultReadStr(), lpCompStr->GetCompReadStr());

  lpCompStr->dwResultStrLen = lpCompStr->dwCompStrLen;
  lpCompStr->dwResultReadStrLen = lpCompStr->dwCompReadStrLen;

  lpCompStr->dwCompStrLen = 0;
  lpCompStr->dwCompReadStrLen = 0;

  //
  // make clause info
  //
  SetClause(lpCompStr->GetResultClause(), lstrlen(lpCompStr->GetResultStr()));
  SetClause(lpCompStr->GetResultReadClause(),
            lstrlen(lpCompStr->GetResultReadStr()));
  lpCompStr->dwResultClauseLen = 8;
  lpCompStr->dwResultReadClauseLen = 8;

  lpIMC->UnlockCompStr();

  if (fFlag) {
    TheApp.GenerateMessage(WM_IME_COMPOSITION, 0, GCS_RESULTALL);
    TheApp.GenerateMessage(WM_IME_ENDCOMPOSITION);
  }

  TheApp.UnlockIMC();

  return TRUE;
}

// Update the transrate key buffer
BOOL PASCAL MakeGuideLine(HIMC hIMC, DWORD dwID) {
  DWORD dwSize =
      sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(TCHAR)) * 2 * sizeof(TCHAR);
  LPTSTR lpStr;

  InputContext *lpIMC = TheApp.LockIMC(hIMC);
  lpIMC->hGuideLine = ImmReSizeIMCC(lpIMC->hGuideLine, dwSize);
  LPGUIDELINE lpGuideLine = lpIMC->LockGuideLine();

  lpGuideLine->dwSize = dwSize;
  lpGuideLine->dwLevel = glTable[dwID].dwLevel;
  lpGuideLine->dwIndex = glTable[dwID].dwIndex;
  lpGuideLine->dwStrOffset = sizeof(GUIDELINE);
  lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset);
  LoadString(TheApp.m_hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
  lpGuideLine->dwStrLen = lstrlen(lpStr);

  if (glTable[dwID].dwPrivateID) {
    lpGuideLine->dwPrivateOffset =
        sizeof(GUIDELINE) + (MAXGLCHAR + 1) * sizeof(TCHAR);
    lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwPrivateOffset);
    LoadString(TheApp.m_hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
    lpGuideLine->dwPrivateSize = lstrlen(lpStr) * sizeof(TCHAR);
  } else {
    lpGuideLine->dwPrivateOffset = 0L;
    lpGuideLine->dwPrivateSize = 0L;
  }

  TheApp.GenerateMessage(WM_IME_NOTIFY, IMN_GUIDELINE, 0);

  lpIMC->UnlockGuideLine();
  TheApp.UnlockIMC();

  return TRUE;
}

int CopyCandidateStringsFromDictionary(LPTSTR lpDic, LPTSTR lpRead,
                                       LPTSTR lpBuf, DWORD dwBufLen) {
  DWORD dwWritten = 0;
  LPTSTR lpSection, lpTemp;
  const LPTSTR szSep = TEXT(" \r\n\t");

  LPTSTR lpToken = wcstok(lpDic, szSep);
  while (NULL != lpToken) {
    if (TEXT('[') == *lpToken) {
      lpSection = lpToken + 1;
      if (NULL != (lpTemp = wcschr(lpSection, TEXT(']'))))
        *lpTemp = 0;
      if (0 == lstrcmp(lpSection, lpRead)) {
        lpToken = wcstok(NULL, szSep);
        break;  // found it.
      }
    }
    lpToken = wcstok(NULL, szSep);
  }
  if (NULL != lpToken) {
    LPTSTR lpWrite = lpBuf;
    DWORD dwW;
    while ((NULL != lpToken) && ((dwBufLen - dwWritten) > 1) &&
           (TEXT('[') != *lpToken)) {
      if (NULL != (lpTemp = wcschr(lpToken, TEXT('='))))
        *lpTemp = 0;
      lstrcpyn(lpWrite, lpToken, dwBufLen - dwWritten - 1);
      dwW = lstrlen(lpToken) + 1;
      lpWrite += dwW;
      dwWritten += dwW;
      lpToken = wcstok(NULL, szSep);
    }
    *lpWrite = 0;
    dwWritten++;
    return dwWritten;
  }
  return 0;
}

int GetCandidateStringsFromDictionary(LPTSTR lpRead, LPTSTR lpBuf,
                                      DWORD dwBufLen, LPTSTR lpFilename) {
  HANDLE hTblFile;
  PSECURITY_ATTRIBUTES psa;
  int nSize = 0;
  DWORD dwFileSize, dwRead;
  LPTSTR lpDic;

  psa = CreateSecurityAttributes();

  hTblFile = CreateFile(lpFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hTblFile == INVALID_HANDLE_VALUE) {
    goto Err0;
  }

  if (dwBufLen > 2) {
    if ((dwFileSize = GetFileSize(hTblFile, NULL)) != 0xffffffff) {
      if ((lpDic = (LPTSTR)GlobalAlloc(GPTR, dwFileSize + 2))) {
        if (ReadFile(hTblFile, lpDic, dwFileSize, &dwRead, NULL)) {
          if (*lpDic == 0xfeff) {
            *(LPWSTR)(((LPBYTE)lpDic) + dwFileSize) = 0;
            nSize = CopyCandidateStringsFromDictionary(lpDic + 1, lpRead, lpBuf,
                                                       dwBufLen);
          }
        }
        GlobalFree(lpDic);
      }
    }
  }

  CloseHandle(hTblFile);

Err0:
  FreeSecurityAttributes(psa);
  return nSize;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
