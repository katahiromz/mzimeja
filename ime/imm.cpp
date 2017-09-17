// imm.cpp --- IME/IMM related
//////////////////////////////////////////////////////////////////////////////

#include "../mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo, TCHAR *lpszClassName,
                       DWORD dwSystemInfoFlags) {
  FOOTMARK_FORMAT("((0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX), %S, 0x%08lX)\n",
    lpIMEInfo->dwPrivateDataSize,
    lpIMEInfo->fdwProperty,
    lpIMEInfo->fdwConversionCaps,
    lpIMEInfo->fdwSentenceCaps,
    lpIMEInfo->fdwUICaps,
    lpIMEInfo->fdwSCSCaps,
    lpIMEInfo->fdwSelectCaps,
    lpszClassName, dwSystemInfoFlags);

  lpIMEInfo->dwPrivateDataSize = sizeof(UIEXTRA);
  lpIMEInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST |
                           IME_PROP_UNICODE | IME_PROP_AT_CARET;
  lpIMEInfo->fdwConversionCaps = IME_CMODE_LANGUAGE | IME_CMODE_FULLSHAPE |
                                 IME_CMODE_ROMAN | IME_CMODE_CHARCODE;
  lpIMEInfo->fdwSentenceCaps = 0;
  lpIMEInfo->fdwUICaps = UI_CAP_2700;

  lpIMEInfo->fdwSCSCaps = 0;
  //lpIMEInfo->fdwSCSCaps = SCS_CAP_COMPSTR | SCS_CAP_MAKEREAD |
  //                        SCS_CAP_SETRECONVERTSTRING;

  lpIMEInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;

  lstrcpy(lpszClassName, szUIServerClassName);

  if (dwSystemInfoFlags & IME_SYSINFO_WINLOGON) {
    TheIME.m_bWinLogOn = TRUE;
  }

  FOOTMARK_RETURN_INT(TRUE);
}

DWORD WINAPI ImeConversionList(HIMC hIMC, LPCTSTR lpSource,
                               LPCANDIDATELIST lpCandList, DWORD dwBufLen,
                               UINT uFlags) {
  FOOTMARK_FORMAT("(%p, %S, %p, 0x%08lX, 0x%08X)\n",
    hIMC, lpSource, lpCandList, dwBufLen, uFlags);

  FOOTMARK_RETURN_LONG(0);
}

BOOL WINAPI ImeDestroy(UINT uForce) {
  FOOTMARK_FORMAT("(0x%08X)\n", uForce);

  FOOTMARK_RETURN_INT(TRUE);
}

LRESULT WINAPI ImeEscape(HIMC hIMC, UINT uSubFunc, LPVOID lpData) {
  LRESULT ret = FALSE;
  FOOTMARK_FORMAT("(%p, %u, %p)\n", hIMC, uSubFunc, lpData);

  switch (uSubFunc) {
  case IME_ESC_QUERY_SUPPORT:
    switch (*(LPUINT)lpData) {
      case IME_ESC_QUERY_SUPPORT:
      case IME_ESC_GETHELPFILENAME:
        ret = TRUE;
        break;

      default:
        ret = FALSE;
        break;
    }
    break;

  case IME_ESC_GETHELPFILENAME:
    {
      std::wstring pathname;
      if (TheIME.GetComputerString(L"ReadMeFile", pathname)) {
        lstrcpyW((WCHAR *)lpData, pathname.c_str());
        ret = TRUE;
      }
    }
    break;

  default:
    ret = FALSE;
    break;
  }

  FOOTMARK_RETURN_LPARAM(ret);
}

BOOL WINAPI ImeSetActiveContext(HIMC hIMC, BOOL fFlag) {
  FOOTMARK_FORMAT("(%p, %u)\n", hIMC, fFlag);

  TheIME.UpdateIndicIcon(hIMC);

  FOOTMARK_RETURN_INT(TRUE);
}

BOOL WINAPI NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue) {
  InputContext *lpIMC;
  BOOL ret = FALSE;
  CandInfo *lpCandInfo;
  CandList *lpCandList;

  FOOTMARK_FORMAT("(%p, 0x%08lX, 0x%08lX, 0x%08lX)\n",
    hIMC, dwAction, dwIndex, dwValue);

  switch (dwAction) {
  case NI_CONTEXTUPDATED:
    DebugPrintA("NI_CONTEXTUPDATED\n");
    switch (dwValue) {
    case IMC_SETOPENSTATUS:
      DebugPrintA("IMC_SETOPENSTATUS\n");
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        if (dwIndex == 0) { // close
          lpIMC->CancelText();
        } else {  // open
          ;
        }
        TheIME.UnlockIMC(hIMC);
      }
      TheIME.UpdateIndicIcon(hIMC);
      ret = TRUE;
      break;

    case IMC_SETCONVERSIONMODE:
      DebugPrintA("IMC_SETCONVERSIONMODE\n");
      break;

    case IMC_SETCOMPOSITIONWINDOW:
      DebugPrintA("IMC_SETCOMPOSITIONWINDOW\n");
      break;

    default:
      break;
    }
    break;

  case NI_COMPOSITIONSTR:
    DebugPrintA("NI_COMPOSITIONSTR\n");
    switch (dwIndex) {
    case CPS_COMPLETE:  // make result
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->MakeResult();
        TheIME.UnlockIMC(hIMC);
      }
      ret = TRUE;
      break;

    case CPS_CONVERT:   // do convert
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->Convert(FALSE);
        ret = TRUE;
        TheIME.UnlockIMC(hIMC);
      }
      break;

    case CPS_REVERT:    // do revert
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->RevertText();
        TheIME.UnlockIMC(hIMC);
      }
      ret = TRUE;
      break;

    case CPS_CANCEL:    // do cancel
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->CancelText();
        TheIME.UnlockIMC(hIMC);
      }
      ret = TRUE;
      break;

    default:
      break;
    }
    break;

  case NI_OPENCANDIDATE:  // open candidate
    DebugPrintA("NI_OPENCANDIDATE\n");
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->OpenCandidate();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case NI_CLOSECANDIDATE: // close candidate
    DebugPrintA("NI_CLOSECANDIDATE\n");
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      ret = lpIMC->CloseCandidate();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case NI_SELECTCANDIDATESTR:
    DebugPrintA("NI_SELECTCANDIDATESTR\n");
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (dwIndex == 1) {
        ret = lpIMC->SelectCand(dwValue);
      }
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case NI_CHANGECANDIDATELIST:
    DebugPrintA("NI_CHANGECANDIDATELIST\n");
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (dwIndex == 1 && lpIMC->HasCandInfo()) ret = TRUE;
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case NI_SETCANDIDATE_PAGESIZE:
    DebugPrintA("NI_SETCANDIDATE_PAGESIZE\n");
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (dwIndex == 1 && lpIMC->HasCandInfo()) {
        lpCandInfo = lpIMC->LockCandInfo();
        if (lpCandInfo) {
          if (lpCandInfo->dwCount > 0) {
            CandList *lpCandList = lpCandInfo->GetList(0);
            lpCandList->dwPageSize = dwValue;
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
            ret = TRUE;
          }
          lpIMC->UnlockCandInfo();
        }
      }
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case NI_SETCANDIDATE_PAGESTART:
    DebugPrintA("NI_SETCANDIDATE_PAGESTART\n");
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (dwIndex == 1 && lpIMC->HasCandInfo()) {
        lpCandInfo = lpIMC->LockCandInfo();
        if (lpCandInfo) {
          lpCandList = lpCandInfo->GetList(0);
          if (dwValue < lpCandList->dwCount) {
            lpCandList->dwPageStart = dwValue;
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
            ret = TRUE;
          }
          lpIMC->UnlockCandInfo();
        }
      }
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case NI_IMEMENUSELECTED:
    DebugPrintA("NI_IMEMENUSELECTED\n");
    TheIME.DoCommand(hIMC, dwIndex);
    break;

  default:
    DebugPrintA("NI_(unknown)\n");
    break;
  }

  FOOTMARK_RETURN_INT(ret);
}

BOOL WINAPI ImeSelect(HIMC hIMC, BOOL fSelect) {
  FOOTMARK_FORMAT("(%p, %u)\n", hIMC, fSelect);

  if (fSelect) TheIME.UpdateIndicIcon(hIMC);
  if (hIMC != NULL) {
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (fSelect) {
        lpIMC->Initialize();
      }
      TheIME.UnlockIMC(hIMC);
    }
  }
  FOOTMARK_RETURN_INT(TRUE);
}

BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPVOID lpComp,
                                    DWORD dwComp, LPVOID lpRead, DWORD dwRead) {
  FOOTMARK_FORMAT("(%p, 0x%08lX, %p, 0x%08lX, %p, 0x%08lX)\n",
    hIMC, dwIndex, lpComp, dwComp, lpRead, dwRead);

  switch (dwIndex) {
    case SCS_QUERYRECONVERTSTRING:
      DebugPrintA("SCS_QUERYRECONVERTSTRING\n");
      break;

    case SCS_SETRECONVERTSTRING:
      DebugPrintA("SCS_SETRECONVERTSTRING\n");
      break;

    default:
      DebugPrintA("SCS_(unknown)\n");
      break;
  }

  FOOTMARK_RETURN_INT(FALSE);
}

struct MYMENUITEM {
  INT nCommandID;
  INT nStringID;
  BOOL bDisabled;
};
static const MYMENUITEM top_menu_items[] = {
  {IDM_HIRAGANA, IDM_HIRAGANA, FALSE},
  {IDM_FULL_KATAKANA, IDM_FULL_KATAKANA, FALSE},
  {IDM_FULL_ASCII, IDM_FULL_ASCII, FALSE},
  {IDM_HALF_KATAKANA, IDM_HALF_KATAKANA, FALSE},
  {IDM_HALF_ASCII, IDM_HALF_ASCII, FALSE},
  {-1, -1},
  {IDM_ROMAN_INPUT, IDM_ROMAN_INPUT, FALSE},
  {IDM_KANA_INPUT, IDM_KANA_INPUT, FALSE},
  {-1, -1},
  {IDM_ADD_WORD, IDM_ADD_WORD, TRUE},
  {IDM_RECONVERT, IDM_RECONVERT, TRUE},
  {IDM_IME_PAD, IDM_IME_PAD, FALSE},
  {-1, -1},
  {IDM_PROPERTY, IDM_PROPERTY, TRUE},
  {IDM_ABOUT, IDM_ABOUT, FALSE},
};

DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType,
                                LPIMEMENUITEMINFO lpImeParentMenu,
                                LPIMEMENUITEMINFO lpImeMenu, DWORD dwSize) {
  FOOTMARK_FORMAT("(%p, 0x%08lX, 0x%08lX, %p, %p, 0x%08lX)\n",
    hIMC, dwFlags, dwType, lpImeParentMenu, lpImeMenu, dwSize);
  DWORD ret = 0;

  if (lpImeMenu == NULL) {
    if (lpImeParentMenu == NULL) {
      if (dwFlags & IGIMIF_RIGHTMENU)
        ret = _countof(top_menu_items);
    }
    FOOTMARK_RETURN_LONG(ret);
  }

  if (lpImeParentMenu == NULL) {
    if (dwFlags & IGIMIF_RIGHTMENU) {
      BOOL bOpen;
      bOpen = ImmGetOpenStatus(hIMC);
      DWORD dwConversion, dwSentence;
      ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
      INPUT_MODE imode;
      imode = InputModeFromConversionMode(bOpen, dwConversion);

      for (size_t i = 0; i < _countof(top_menu_items); ++i) {
        const MYMENUITEM& item = top_menu_items[i];
        lpImeMenu[i].cbSize = sizeof(IMEMENUITEMINFO);
        lpImeMenu[i].fState = 0;
        switch (item.nCommandID) {
        case -1:
          lpImeMenu[i].fType = IMFT_SEPARATOR;
          break;
        case IDM_HIRAGANA:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_FULL_HIRAGANA) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_FULL_KATAKANA:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_FULL_KATAKANA) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_FULL_ASCII:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_FULL_ASCII) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_HALF_KATAKANA:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_HALF_KANA) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_HALF_ASCII:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_HALF_ASCII) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_ROMAN_INPUT:
        case IDM_KANA_INPUT:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (dwConversion & IME_CMODE_ROMAN) {
            if (item.nCommandID == IDM_ROMAN_INPUT) {
              lpImeMenu[i].fState = IMFS_CHECKED;
            }
          } else {
            if (item.nCommandID == IDM_KANA_INPUT) {
              lpImeMenu[i].fState = IMFS_CHECKED;
            }
          }
          break;
        default:
          lpImeMenu[i].fType = 0;
          break;
        }
        if (item.bDisabled) {
          lpImeMenu[i].fState |= MFS_GRAYED;
        }
        lpImeMenu[i].wID = item.nCommandID;
        lpImeMenu[i].hbmpChecked = 0;
        lpImeMenu[i].hbmpUnchecked = 0;
        if (item.nStringID != -1) {
          lstrcpy(lpImeMenu[i].szString, TheIME.LoadSTR(item.nStringID));
        } else {
          lpImeMenu[i].szString[0] = TEXT('\0');
        }
        lpImeMenu[i].hbmpItem = 0;
      }
      ret = _countof(top_menu_items);
    }
  }

  FOOTMARK_RETURN_LONG(ret);
} // ImeGetImeMenuItems

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
