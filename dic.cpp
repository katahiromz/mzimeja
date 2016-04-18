// dic.cpp
//////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "mzimeja.h"
#include "immsec.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

void PASCAL FlushText(HIMC hIMC) {
  if (!IsCompStr(hIMC)) return;

  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
  if (NULL == lpIMC) return;

  TRANSMSG GnMsg;
  if (lpIMC->IsCandidate()) {
    // Flush candidate lists.
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      lpIMC->UnlockCandInfo();
    }
    GnMsg.message = WM_IME_NOTIFY;
    GnMsg.wParam = IMN_CLOSECANDIDATE;
    GnMsg.lParam = 1;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
  }

  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (lpCompStr) {
    // Flush composition strings.
    lpCompStr->Clear(CLR_RESULT_AND_UNDET);
    lpIMC->UnlockCompStr();

    GnMsg.message = WM_IME_COMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = 0;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

    GnMsg.message = WM_IME_ENDCOMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = 0;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
  }
  ImmUnlockIMC(hIMC);
}

void PASCAL RevertText(HIMC hIMC) {
  CompStr *lpCompStr;
  TRANSMSG GnMsg;
  LPTSTR lpread, lpstr;

  if (!IsCompStr(hIMC)) return;

  InputContext *lpIMC;
  if (!(lpIMC = (InputContext *)ImmLockIMC(hIMC))) return;

  if (lpIMC->IsCandidate()) {
    // Flush candidate lists.
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      lpIMC->UnlockCandInfo();
    }
    GnMsg.message = WM_IME_NOTIFY;
    GnMsg.wParam = IMN_CLOSECANDIDATE;
    GnMsg.lParam = 1;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
  }

  lpCompStr = lpIMC->LockCompStr();
  if (lpCompStr) {
    lpstr = lpCompStr->GetCompStr();
    lpread = lpCompStr->GetCompReadStr();
    lHanToZen(lpstr, lpread, lpIMC->fdwConversion);

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
    GnMsg.message = WM_IME_COMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

    lpIMC->UnlockCompStr();
  }
  ImmUnlockIMC(hIMC);
}

BOOL PASCAL ConvKanji(HIMC hIMC) {
  TRANSMSG GnMsg;
  BOOL bRc = FALSE;

  if ((GetFileAttributes(szDicFileName) == 0xFFFFFFFF) ||
      (GetFileAttributes(szDicFileName) & FILE_ATTRIBUTE_DIRECTORY)) {
    MakeGuideLine(hIMC, MYGL_NODICTIONARY);
  }

  if (!IsCompStr(hIMC)) return FALSE;

  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
  if (NULL == lpIMC) return FALSE;

  CompStr *lpCompStr = lpIMC->LockCompStr();
  if (NULL ==lpCompStr) {
    ImmUnlockIMC(hIMC);
    return bRc;
  }

  CandInfo *lpCandInfo = lpIMC->LockCandInfo();
  if (NULL == lpCandInfo) {
    lpIMC->UnlockCompStr();
    ImmUnlockIMC(hIMC);
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
    GetCandidateStringsFromDictionary(lpT2, szBuf, 256, szDicFileName);

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
      ImmUnlockIMC(hIMC);
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

      GnMsg.message = WM_IME_COMPOSITION;
      GnMsg.wParam = 0;
      GnMsg.lParam =
        GCS_COMPSTR | GCS_CURSORPOS | GCS_COMPATTR | GCS_COMPREADATTR;
      GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
    }

    lpIMC->UnlockCandInfo();
    lpIMC->UnlockCompStr();
    ImmUnlockIMC(hIMC);
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
        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

        bRc = TRUE;
        lpIMC->UnlockCandInfo();
        lpIMC->UnlockCompStr();
        ImmUnlockIMC(hIMC);
        return bRc;
      }
      lpstr += (lstrlen(lpstr) + 1);
    }
  } else {
    // String is converted, so that open candidate.
    // generate WM_IME_NOTFIY IMN_OPENCANDIDATE message.
    if (!lpIMC->IsCandidate()) {
      GnMsg.message = WM_IME_NOTIFY;
      GnMsg.wParam = IMN_OPENCANDIDATE;
      GnMsg.lParam = 1L;
      GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
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
    GnMsg.message = WM_IME_NOTIFY;
    GnMsg.wParam = IMN_CHANGECANDIDATE;
    GnMsg.lParam = 1L;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

    // If the selected candidate string is changed, the composition string
    // should be updated.
    lpstr = lpCandList->GetCurString();
    goto set_compstr;
  }

  lpIMC->UnlockCandInfo();
  lpIMC->UnlockCompStr();
  ImmUnlockIMC(hIMC);
  return bRc;
}

void PASCAL DeleteChar(HIMC hIMC, UINT uVKey) {
  LPTSTR lpstr, lpread, lpptr;
  int nChar;
  BOOL fDone = FALSE;
  DWORD dwCurPos;
  TRANSMSG GnMsg;

  if (!IsCompStr(hIMC)) return;

  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
  if (NULL == lpIMC) return;
  CompStr *lpCompStr = lpIMC->LockCompStr();

  dwCurPos = lpCompStr->dwCursorPos;
  lpstr = lpCompStr->GetCompStr();

  if (uVKey == VK_BACK) {
    if (dwCurPos == 0) goto dc_exit;

    lpptr = CharPrev(lpstr, lpstr + dwCurPos);
    nChar = 1;
    if (lpstr == lpptr && lstrlen(lpstr) == nChar) {
      dwCurPos = 0;
      *lpstr = 0;
    } else {
      lstrcpy(lpptr, lpstr + dwCurPos);
      dwCurPos -= nChar;
    }

    fDone = TRUE;
  } else if (uVKey == VK_DELETE) {
    if (dwCurPos == (DWORD)lstrlen(lpstr)) goto dc_exit;

    nChar = 1;
    lstrcpy(lpstr + dwCurPos, lpstr + dwCurPos + nChar);

    fDone = TRUE;
  }

  if (fDone) {
    lpstr = lpCompStr->GetCompStr();
    lpread = lpCompStr->GetCompReadStr();
    lZenToHan(lpread, lpstr);

    memset(lpCompStr->GetCompAttr(), 0, lstrlen(lpstr));
    memset(lpCompStr->GetCompReadAttr(), 0, lstrlen(lpread));

    // make length
    lpCompStr->dwCompStrLen = lstrlen(lpstr);
    lpCompStr->dwCompReadStrLen = lstrlen(lpread);
    lpCompStr->dwCompAttrLen = lstrlen(lpstr);
    lpCompStr->dwCompReadAttrLen = lstrlen(lpread);

    lpCompStr->dwCursorPos = dwCurPos;
    // DeltaStart is same of Cursor Pos at DeleteChar time.
    lpCompStr->dwDeltaStart = dwCurPos;

    // make clause info
    SetClause(lpCompStr->GetCompClause(), lstrlen(lpstr));
    SetClause(lpCompStr->GetCompReadClause(), lstrlen(lpread));
    lpCompStr->dwCompClauseLen = 8;
    lpCompStr->dwCompReadClauseLen = 8;

    if (lpCompStr->dwCompStrLen) {
      GnMsg.message = WM_IME_COMPOSITION;
      GnMsg.wParam = 0;
      GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
      GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
    } else {
      if (lpIMC->IsCandidate()) {
        CandInfo *lpCandInfo = lpIMC->LockCandInfo();
        lpCandInfo->Clear();
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
        lpIMC->UnlockCandInfo();
      }

      lpCompStr->Clear(CLR_RESULT_AND_UNDET);

      GnMsg.message = WM_IME_COMPOSITION;
      GnMsg.wParam = 0;
      GnMsg.lParam = 0;
      GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

      GnMsg.message = WM_IME_ENDCOMPOSITION;
      GnMsg.wParam = 0;
      GnMsg.lParam = 0;
      GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
    }
  }

dc_exit:
  lpIMC->UnlockCompStr();
  ImmUnlockIMC(hIMC);
}

void PASCAL AddChar(HIMC hIMC, WORD code) {
  LPTSTR lpchText;
  LPTSTR lpread, lpstr, lpprev;
  WORD code2 = 0, code3;
  DWORD fdwConversion;
  CompStr *lpCompStr;
  DWORD dwStrLen;
  DWORD dwSize;
  TRANSMSG GnMsg;
  DWORD dwGCR = 0L;
  WCHAR Katakana, Sound;

  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);

  if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof(MZCOMPSTR)) {
    // Init time.
    dwSize = sizeof(MZCOMPSTR);
    lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr, dwSize);
    lpCompStr = lpIMC->LockCompStr();
    lpCompStr->dwSize = dwSize;
  } else {
    lpCompStr = lpIMC->LockCompStr();
  }

  dwStrLen = lpCompStr->dwCompStrLen;

  if (!dwStrLen) {
    lpCompStr->Init(CLR_RESULT_AND_UNDET);

    GnMsg.message = WM_IME_STARTCOMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = 0;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

  } else if (IsConvertedCompStr(hIMC)) {
    MakeResultString(hIMC, FALSE);
    lpCompStr->Init(CLR_UNDET);
    dwGCR = GCS_RESULTALL;
  }

  // Get ConvMode from IMC.
  fdwConversion = lpIMC->fdwConversion;

  lpchText = lpCompStr->GetCompStr();
  lpstr = lpchText;
  if (lpCompStr->dwCursorPos) lpstr += lpCompStr->dwCursorPos;
  lpstr = lpchText + lstrlen(lpchText);
  lpprev = CharPrev(lpchText, lpstr);

  if (fdwConversion & IME_CMODE_CHARCODE) {
    code = (WORD)(LONG_PTR)AnsiUpper((LPSTR)(LONG_PTR)code);
    if (!((code >= TEXT('0') && code <= TEXT('9')) ||
          (code >= TEXT('A') && code <= TEXT('F'))) ||
        lpCompStr->dwCursorPos >= 4) {
      MessageBeep(0);
      goto ac_exit;
    }
    *lpstr++ = (BYTE)code;
    lpCompStr->dwCursorPos++;
  } else if (fdwConversion & IME_CMODE_FULLSHAPE) {
    if (fdwConversion & IME_CMODE_ROMAN && fdwConversion & IME_CMODE_NATIVE) {
      if (*lpprev) {
        code2 = *lpprev;
      } else {
        if (IsSecond(code)) {
          code = ConvChar(hIMC, 0, code);
          if (!(fdwConversion & IME_CMODE_KATAKANA)) {
            code = KataToHira(code);
          }
        }
        goto DBCS_BETA;
      }

      if (!(code2 = ZenToHan(code2))) {
        if (IsSecond(code)) {
          code = ConvChar(hIMC, 0, code);
          if (!(fdwConversion & IME_CMODE_KATAKANA)) {
            code = KataToHira(code);
          }
        }
        goto DBCS_BETA;
      }

      if (IsSecond(code)) {
        if (IsFirst(code2) && (code3 = ConvChar(hIMC, code2, code))) {
          if (fdwConversion & IME_CMODE_KATAKANA) {
            *lpprev = code3;
          } else {
            *lpprev = KataToHira(code3);
          }
        } else {
          code = ConvChar(hIMC, 0, code);

          if (!(fdwConversion & IME_CMODE_KATAKANA)) {
            code = KataToHira(code);
          }
          goto DBCS_BETA;
        }
      } else if ((WORD)(LONG_PTR)CharUpper((LPTSTR)(LONG_PTR)code) == 'N' &&
                 (WORD)(LONG_PTR)CharUpper((LPTSTR)(LONG_PTR)code2) == 'N') {
        code3 = 0xFF9D;
        code2 = HanToZen(code3, 0, fdwConversion);
        *lpprev = code2;
      } else
        goto DBCS_BETA;
    } else {
    DBCS_BETA:
      if (code == TEXT('^')) {
        code2 = *lpprev;
        if (IsTenten(code2) == FALSE) goto DBCS_BETA2;
        code2 = ConvTenten(code2);
        *lpprev++ = code2;
      } else if (code == TEXT('_')) {
        code2 = *lpprev;
        if (IsMaru(code2) == FALSE) goto DBCS_BETA2;
        code2 = ConvMaru(code2);
        *lpprev = code2;
      } else {
        code = HanToZen(code, 0, fdwConversion);
      DBCS_BETA2:
        *lpstr++ = code;
        lpCompStr->dwCursorPos += 1;
      }
    }
  } else {
    if (fdwConversion & IME_CMODE_ROMAN && fdwConversion & IME_CMODE_NATIVE) {
      if (IsSecond(code)) {
        if (IsFirst(*lpprev) && (code2 = ConvChar(hIMC, *lpprev, code))) {
          if (OneCharZenToHan(code2, &Katakana, &Sound)) {
            *lpprev = Katakana;
            if (Sound) {
              *lpstr++ = Sound;
              lpCompStr->dwCursorPos++;
            }
          } else {
            code = ConvChar(hIMC, 0, code);
            goto SBCS_BETA;
          }
        } else {
          code = ConvChar(hIMC, 0, code);
          // MakeGuideLine(hIMC,MYGL_TYPINGERROR);
          goto SBCS_BETA;
        }
      } else {
        if ((WORD)(LONG_PTR)CharUpper((LPTSTR)(LONG_PTR)code) == 'N' &&
            (WORD)(LONG_PTR)CharUpper(
                (LPTSTR)(LONG_PTR)(code2 = *lpprev)) == 'N') {
          *lpprev = (TCHAR)0xFF9D;
        } else {
          // MakeGuideLine(hIMC,MYGL_TYPINGERROR);
          goto SBCS_BETA;
        }
      }
    } else {
    SBCS_BETA:
      if (OneCharZenToHan(code, &Katakana, &Sound)) {
        *lpstr++ = Katakana;
        if (Sound) {
          *lpstr++ = Sound;
          lpCompStr->dwCursorPos++;
        }
      } else {
        *lpstr++ = code;
      }
      lpCompStr->dwCursorPos++;
    }
  }
  *lpstr = 0;

  // make reading string.
  lpstr = lpCompStr->GetCompStr();
  lpread = lpCompStr->GetCompReadStr();
  if (fdwConversion & IME_CMODE_KATAKANA) {
    if (fdwConversion & IME_CMODE_FULLSHAPE) {
      lstrcpy(lpread, lpstr);
    } else {
      lHanToZen(lpread, lpstr, fdwConversion);
    }
  } else {
    LPTSTR pSrc = lpstr;
    LPTSTR pDst = lpread;

    for (; *pSrc;) {
      *pDst++ = HiraToKata(*pSrc);
      pSrc++;
    }
    *pDst = (TCHAR)0;
  }

  // make attribute
  lpCompStr->dwCursorPos = lstrlen(lpstr);
  lpCompStr->dwDeltaStart =
      (DWORD)(CharPrev(lpstr, lpstr + lstrlen(lpstr)) - lpstr);

  // MakeAttrClause(lpCompStr);
  memset(lpCompStr->GetCompAttr(), 0, lstrlen(lpstr));
  memset(lpCompStr->GetCompReadAttr(), 0, lstrlen(lpread));

  // make length
  lpCompStr->dwCompStrLen = lstrlen(lpstr);
  lpCompStr->dwCompReadStrLen = lstrlen(lpread);
  lpCompStr->dwCompAttrLen = lstrlen(lpstr);
  lpCompStr->dwCompReadAttrLen = lstrlen(lpread);

  // make clause info
  SetClause(lpCompStr->GetCompClause(), lstrlen(lpstr));
  SetClause(lpCompStr->GetCompReadClause(), lstrlen(lpread));
  lpCompStr->dwCompClauseLen = 8;
  lpCompStr->dwCompReadClauseLen = 8;

  GnMsg.message = WM_IME_COMPOSITION;
  GnMsg.wParam = 0;
  GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART | dwGCR;
  GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

ac_exit:
  lpIMC->UnlockCompStr();
  ImmUnlockIMC(hIMC);
}

BOOL WINAPI MakeResultString(HIMC hIMC, BOOL fFlag) {
  TRANSMSG GnMsg;

  if (!IsCompStr(hIMC)) return FALSE;

  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
  CompStr *lpCompStr = lpIMC->LockCompStr();

  if (lpIMC->IsCandidate()) {
    CandInfo *lpCandInfo = lpIMC->LockCandInfo();
    if (lpCandInfo) {
      lpCandInfo->Clear();
      lpIMC->UnlockCandInfo();
    }
    GnMsg.message = WM_IME_NOTIFY;
    GnMsg.wParam = IMN_CLOSECANDIDATE;
    GnMsg.lParam = 1L;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
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
    GnMsg.message = WM_IME_COMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = GCS_RESULTALL;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

    GnMsg.message = WM_IME_ENDCOMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = 0;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);
  }

  ImmUnlockIMC(hIMC);

  return TRUE;
}

// Update the transrate key buffer
BOOL PASCAL MakeGuideLine(HIMC hIMC, DWORD dwID) {
  TRANSMSG GnMsg;
  DWORD dwSize =
      sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(TCHAR)) * 2 * sizeof(TCHAR);
  LPTSTR lpStr;

  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
  lpIMC->hGuideLine = ImmReSizeIMCC(lpIMC->hGuideLine, dwSize);
  LPGUIDELINE lpGuideLine = lpIMC->LockGuideLine();

  lpGuideLine->dwSize = dwSize;
  lpGuideLine->dwLevel = glTable[dwID].dwLevel;
  lpGuideLine->dwIndex = glTable[dwID].dwIndex;
  lpGuideLine->dwStrOffset = sizeof(GUIDELINE);
  lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwStrOffset);
  LoadString(hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
  lpGuideLine->dwStrLen = lstrlen(lpStr);

  if (glTable[dwID].dwPrivateID) {
    lpGuideLine->dwPrivateOffset =
        sizeof(GUIDELINE) + (MAXGLCHAR + 1) * sizeof(TCHAR);
    lpStr = (LPTSTR)((LPBYTE)lpGuideLine + lpGuideLine->dwPrivateOffset);
    LoadString(hInst, glTable[dwID].dwStrID, lpStr, MAXGLCHAR);
    lpGuideLine->dwPrivateSize = lstrlen(lpStr) * sizeof(TCHAR);
  } else {
    lpGuideLine->dwPrivateOffset = 0L;
    lpGuideLine->dwPrivateSize = 0L;
  }

  GnMsg.message = WM_IME_NOTIFY;
  GnMsg.wParam = IMN_GUIDELINE;
  GnMsg.lParam = 0;
  GenerateMessage(hIMC, lpIMC, lpCurTransKey, &GnMsg);

  lpIMC->UnlockGuideLine();
  ImmUnlockIMC(hIMC);

  return TRUE;
}

// Update the transrate key buffer
BOOL PASCAL GenerateMessage(HIMC hIMC, InputContext *lpIMC,
                            LPTRANSMSGLIST lpTransBuf, LPTRANSMSG lpGeneMsg) {
  if (lpTransBuf) return GenerateMessageToTransKey(lpTransBuf, lpGeneMsg);

  if (IsWindow(lpIMC->hWnd)) {
    DWORD dwNewSize = sizeof(TRANSMSG) * (lpIMC->NumMsgBuf() + 1);
    if (!(lpIMC->hMsgBuf = ImmReSizeIMCC(lpIMC->hMsgBuf, dwNewSize)))
      return FALSE;

    LPTRANSMSG lpTransMsg = lpIMC->LockMsgBuf();
    if (NULL == lpTransMsg) return FALSE;
    lpTransMsg[lpIMC->NumMsgBuf()] = *lpGeneMsg;
    lpIMC->NumMsgBuf()++;
    lpIMC->UnlockMsgBuf();

    ImmGenerateMessage(hIMC);
  }
  return TRUE;
}

void PASCAL HandleShiftArrow(HIMC hIMC, BOOL fArrow) {
  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
  if (lpIMC) {
    CompStr *lpCompStr = lpIMC->LockCompStr();
    if (lpCompStr) {
      if (!lpCompStr->CheckAttr()) {
        LPTSTR lpstart = lpCompStr->GetCompStr();
        LPTSTR lpstr = lpstart + lpCompStr->dwCursorPos;
        LPTSTR lpend = lpstart + lstrlen(lpstart);

        if (fArrow == ARR_RIGHT) {
          if (lpstr < lpend) lpstr = CharNext(lpstr);
        } else {
          if (lpstr > lpstart) lpstr = CharPrev(lpstart, lpstr);
        }

        lpCompStr->dwCursorPos = (DWORD)(lpstr - lpstart);
        lpCompStr->MakeAttrClause();
      }
      lpIMC->UnlockCompStr();
    }
    ImmUnlockIMC(hIMC);
  }
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
