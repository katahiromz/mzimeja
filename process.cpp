// process.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "vksub.h"

#define IsCtrlPressed(x)  ((x)[VK_CONTROL] & 0x80)
#define IsShiftPressed(x) ((x)[VK_SHIFT] & 0x80)
#define IsAltPressed(x)   ((x)[VK_ALT] & 0x80)

extern "C" {

//////////////////////////////////////////////////////////////////////////////

WCHAR MapOemVirtualKey(BYTE vk, BOOL bShift) {
  switch (vk) {
  case VK_OEM_PLUS:     return (bShift ? L'+' : L';');
  case VK_OEM_MINUS:    return (bShift ? L'=' : L'-');
  case VK_OEM_PERIOD:   return (bShift ? L'>' : L'.');
  case VK_OEM_COMMA:    return (bShift ? L'<' : L',');
  case VK_OEM_1:        return (bShift ? L'*' : L':');
  case VK_OEM_2:        return (bShift ? L'?' : L'/');
  case VK_OEM_3:        return (bShift ? L'`' : L'@');
  case VK_OEM_4:        return (bShift ? L'{' : L'[');
  case VK_OEM_5:        return (bShift ? L'|' : L'\\');
  case VK_OEM_6:        return (bShift ? L'}' : L']');
  case VK_OEM_7:        return (bShift ? L'~' : L'^');
  case VK_OEM_8:        return 0;
  case VK_OEM_9:        return 0;
  case VK_OEM_102:      return (bShift ? L'_' : L'\\');
  default:              return 0;
  }
} // MapOemVirtualKey

WCHAR MapNumPadVirtualKey(BYTE vk) {
  switch (vk) {
  case VK_ADD:        return L'+';
  case VK_SUBTRACT:   return L'-';
  case VK_MULTIPLY:   return L'*';
  case VK_DIVIDE:     return L'/';
  case VK_SEPARATOR:  return L',';
  case VK_DECIMAL:    return L'.';
  case VK_NUMPAD0:    return L'0';
  case VK_NUMPAD1:    return L'1';
  case VK_NUMPAD2:    return L'2';
  case VK_NUMPAD3:    return L'3';
  case VK_NUMPAD4:    return L'4';
  case VK_NUMPAD5:    return L'5';
  case VK_NUMPAD6:    return L'6';
  case VK_NUMPAD7:    return L'7';
  case VK_NUMPAD8:    return L'8';
  case VK_NUMPAD9:    return L'9';
  default:            return 0;
  }
} // MapNumPadVirtualKey


WCHAR MapDigitVirtualKey(BYTE vk, BOOL bShift) {
  if (bShift) {
    static const WCHAR s_table[] = {
      0, L'!', L'"', L'#', L'$', L'%', L'&', L'\'', L'(', L')'
    };
    if (VK_0 <= vk && vk <= VK_9) {
      return s_table[vk - VK_0];
    }
  } else {
    return (WCHAR)vk;
  }
  return 0;
}

WCHAR MapAlphaVirtualKey(HIMC hIMC, BYTE vk, LPBYTE lpbKeyState) {
  if (IsRomanMode(hIMC)) {
    if (lpbKeyState[VK_SHIFT] & 0x80) {
      return L'A' + (vk - VK_A);
    }
  }
  return L'a' + (vk - VK_A);
}

// A function which handles WM_IME_KEYDOWN
BOOL IMEKeyDownHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                       LPBYTE lpbKeyState) {
  InputContext *lpIMC;
  WORD vk = (wParam & 0x00FF);
  BOOL bOpen;
  WCHAR ch;

  if (lpbKeyState[VK_CONTROL] & 0x80) return FALSE;

  switch (vk) {
  case VK_SHIFT:
  case VK_CONTROL:
    break;

  case VK_KANJI:
  case VK_OEM_AUTO:
  case VK_OEM_ENLW:
    if (hIMC) {
      bOpen = FALSE;
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        bOpen = lpIMC->IsOpen();
        TheIME.UnlockIMC(hIMC);
      }
      if (bOpen) {
        ImmSetOpenStatus(hIMC, FALSE);
      } else {
        ImmSetOpenStatus(hIMC, TRUE);
      }
    }
    break;

  case VK_OEM_COPY:
    bOpen = FALSE;
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      bOpen = lpIMC->IsOpen();
      TheIME.UnlockIMC(hIMC);
    }
    if (!bOpen) {
      ImmSetOpenStatus(hIMC, TRUE);
    }
    if (lpbKeyState[VK_MENU] & 0x80) {
      SetRomanMode(hIMC, !IsRomanMode(hIMC));
    } else if (lpbKeyState[VK_SHIFT] & 0x80) {
      SetInputMode(hIMC, IMODE_ZEN_KATAKANA);
    } else {
      SetInputMode(hIMC, IMODE_ZEN_HIRAGANA);
    }
    break;

  case VK_ESCAPE:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->CancelText();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_DELETE:
  case VK_BACK:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->DeleteChar(vk == VK_BACK);
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_SPACE:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (lpIMC->HasCompStr()) {
        lpIMC->DoConvert();
      } else {
        // add ideographic space
        TheIME.GenerateMessage(WM_IME_CHAR, L'　', 1);
      }
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_CONVERT:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->DoConvert();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_F6:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MakeHiragana();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_F7:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MakeKatakana();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_F8:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MakeHankaku();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_F9:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MakeZenEisuu();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_F10:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MakeHanEisuu();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_RETURN:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (lpIMC->Conversion() & IME_CMODE_CHARCODE) {
        // code input
        lpIMC->CancelText();
      } else {
        lpIMC->MakeResult();
      }
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_LEFT:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MoveLeft();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_RIGHT:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MoveRight();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_HOME: case VK_UP:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MoveToBeginning();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_END: case VK_DOWN:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MoveToEnd();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_OEM_PLUS: case VK_OEM_MINUS: case VK_OEM_PERIOD:
  case VK_OEM_COMMA:
  case VK_OEM_1: case VK_OEM_2: case VK_OEM_3: case VK_OEM_4:
  case VK_OEM_5: case VK_OEM_6: case VK_OEM_7: case VK_OEM_8:
  case VK_OEM_9: case VK_OEM_102:
    // OEM keys
    ch = MapOemVirtualKey((BYTE)vk, (lpbKeyState[VK_SHIFT] & 0x80));
    if (ch != 0) {
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->AddChar(ch);
        TheIME.UnlockIMC(hIMC);
      }
    }
    break;
  case VK_ADD: case VK_SUBTRACT: case VK_MULTIPLY: case VK_DIVIDE:
  case VK_SEPARATOR: case VK_DECIMAL:
  case VK_NUMPAD0: case VK_NUMPAD1: case VK_NUMPAD2: case VK_NUMPAD3:
  case VK_NUMPAD4: case VK_NUMPAD5: case VK_NUMPAD6: case VK_NUMPAD7:
  case VK_NUMPAD8: case VK_NUMPAD9:
    // num pad keys
    // OEM keys
    ch = MapNumPadVirtualKey((BYTE)vk);
    if (ch != 0) {
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->AddChar(ch);
        TheIME.UnlockIMC(hIMC);
      }
    }
    break;

  default:
    if (VK_0 <= vk && vk <= VK_9) {
      ch = MapDigitVirtualKey((BYTE)vk, (lpbKeyState[VK_SHIFT] & 0x80));
    } else if (VK_A <= vk && vk <= VK_Z) {
      ch = MapAlphaVirtualKey(hIMC, (BYTE)vk, lpbKeyState);
    } else if (vk == VK_PACKET) {
      ch = HIWORD(wParam);
    } else {
      return FALSE;
    }
    if (ch) {
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->AddChar(ch);
        TheIME.UnlockIMC(hIMC);
      }
    }
    break;
  }
  return TRUE;
} // IMEKeyDownHandler

//  ImeProcessKey ()
//  ImeProcessKey 関数は IMM を通して与えられた全てのキーストロークを前処
//  理して、もしそのキーが与えられた Input Context で IME に必要なもので
//  あれば TRUE を返す。
//  BOOL
//    ImeProcessKey(
//    HIMC hIMC,
//    UINT uVirKey,
//    DWORD lParam,
//    CONST LPBYTE lpbKeyState
//  )
//  Parameters
//    hIMC
//      Input context handle
//    uVirKey
//      処理されるべき仮想キー。
//    lParam
//      キーメッセージの lParam。(WM_KEYDOWN,WM_KEYUP の LPARAM)
//    lpbKeyState
//      現在のキーボードの状態を含んだ256バイトの配列を指すポインタ。
//      IME はこの内容を変更すべきではない。
//  Return Values
//    成功なら TRUE。そうでなければ FALSE。
//  Comments
//    システムはキーが IME によって取り扱われるべきか否かをこの関数を
//    呼び出すことによって決定している。アプリケーションがキーメッセー
//    ジを受け取る前にこの関数が TRUE を返せば、IME はそのキーを処理す
//    る。システムは ImeToAsciiEx 関数を呼び出す。
//    FALSE を返したならば、システムはそのキーが IME によって処理され
//    ないことが分かるのでキーメッセージはアプリケーションに送られる。
BOOL WINAPI ImeProcessKey(HIMC hIMC, UINT vKey, LPARAM lKeyData,
                          CONST LPBYTE lpbKeyState) {
  BOOL ret = FALSE;
  FOOTMARK();

  if (lKeyData & 0x80000000) {
    return FALSE;
  }

  InputContext *lpIMC = TheIME.LockIMC(hIMC);
  if (lpIMC == NULL) {
    return FALSE;
  }

  BOOL fOpen = lpIMC->IsOpen();
  BOOL fAlt = (lpbKeyState[VK_MENU] & 0x80);
  BOOL fCtrl = (lpbKeyState[VK_CONTROL] & 0x80);
  BOOL fShift = (lpbKeyState[VK_SHIFT] & 0x80);

  switch (vKey) {
  case VK_KANJI:
  case VK_OEM_AUTO:
  case VK_OEM_ENLW:
    if (!fShift && !fCtrl) {
      // switch zenkaku/hankaku input mode
      ret = TRUE;
    }
    break;
  }

  if (fOpen) {
    BOOL fCompStr = lpIMC->HasCompStr();
    BOOL fCandInfo = lpIMC->HasCandInfo();
    if (fAlt) {
      // Alt key is down
    } else if (fCtrl) {
      // Ctrl key is down
      if (fCompStr) {
        switch (vKey) {
        case VK_UP: case VK_DOWN:
        case VK_LEFT: case VK_RIGHT:
          // widely move
          ret = TRUE;
          break;
        }
      }
    } else if (fShift) {
      // Shift key is down
      switch (vKey) {
      case VK_LEFT: case VK_RIGHT:
        // fix clauses
        ret = TRUE;
        break;
      }
    } else {
      // Neither Ctrl nor Shift key is down
      ret = FALSE;
      if (fCompStr) {
        switch (vKey) {
        case VK_F6:     // make composition zenkaku Hiragana
        case VK_F7:     // make composition zenkaku Katakana
        case VK_F8:     // make composition hankaku Katakana
        case VK_F9:     // make composition zenkaku alphanumeric
        case VK_F10:    // make composition hankaku alphanumeric
        case VK_ESCAPE: // close composition
          ret = TRUE;
          break;
        }
      }
      switch (vKey) {
      case VK_OEM_PLUS: case VK_OEM_MINUS: case VK_OEM_PERIOD:
      case VK_OEM_COMMA:
      case VK_OEM_1: case VK_OEM_2: case VK_OEM_3: case VK_OEM_4:
      case VK_OEM_5: case VK_OEM_6: case VK_OEM_7: case VK_OEM_8:
      case VK_OEM_9: case VK_OEM_102: case VK_OEM_COPY:
        // OEM keys
        ret = TRUE;
        break;
      case VK_ADD: case VK_SUBTRACT:
      case VK_MULTIPLY: case VK_DIVIDE:
      case VK_SEPARATOR: case VK_DECIMAL:
        // numpad keys
        ret = TRUE;
        break;
      case VK_HOME: case VK_END:
      case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
        // arrow and moving key
        ret = TRUE;
        break;
      case VK_SPACE: case VK_BACK: case VK_DELETE: case VK_RETURN:
      case VK_CAPITAL: case VK_CONVERT: case VK_NONCONVERT:
        // special keys
        ret = TRUE;
        break;
      default:
        if ('0' <= vKey && vKey <= '9') {
          // numbers
          ret = TRUE;
        } else if ('A' <= vKey && vKey <= 'Z') {
          // alphabets
          ret = TRUE;
        } else if (VK_NUMPAD0 <= vKey && vKey <= VK_NUMPAD9) {
          // numpad numbers
          ret = TRUE;
        } else {
          if (fCandInfo) {
            switch (vKey) {
            case VK_PRIOR: case VK_NEXT:
              // next or previous page of candidates
              ret = TRUE;
              break;
            }
          }
        }
      }
    }
  }

  TheIME.UnlockIMC(hIMC);
  return ret;
} // ImeProcessKey

//  ImeToAsciiEx 
//  ImeToAsciiEx 関数は hIMC パラメータに従って IME 変換エンジンを通して変
//  換結果を生成します。
//  UINT
//    ImeToAsciiEx(
//    UINT uVirKey,
//    UINT uScanCode,
//    CONST LPBYTE lpbKeyState,
//    LPDWORD lpdwTransBuf,
//    UINT fuState,
//    HIMC hIMC
//  )
//  (パラメータ)
//  uVirKey
//    変換される仮想キーコードを指定します。プロパティの 
//    IME_PROP_KBD_CHAR_FIRST ビットがオンだったら、仮想キーの上位バイト
//    は補助キャラクタコード(?)になります。
//    Unicode については、IME_PROP_KBD_CHAR_FIRST ビットがオンだったら 
//    uVirKey の上位ワードが Unicode 文字を含みます。
//  uScanCode
//    変換されるキーのハードウェアスキャンコードを指定します。
//  lpbKeyState
//    現在のキーボードの状態を含んだ256バイトの配列へのポインタです。
//    IME はこの内容を変更してはいけません。
//  lpdwTransBuf
//    変換結果を受け取る DWORD のバッファを指すポインタです。その書式は、
//    [メッセージバッファの長さ][メッセージ1][wParam1][lParam1]
//    [メッセージ2][wParam2][lParam2][...[...[...]]] のようになります。
//  fuState
//    Active menu flag. ？
//  hIMC
//    Input context handle.
//  (返り値)
//    返り値はメッセージの数を表します。もしその数がバッファサイズより
//    大きければメッセージバッファは十分ではないです…って当然では…(^^;;
//    システムは hMsgBuf をチェックしてメッセージを得ます。
//  (See Also)
//    ImmToAsciiEx
UINT WINAPI ImeToAsciiEx(UINT uVKey, UINT uScanCode, CONST LPBYTE lpbKeyState,
                         LPTRANSMSGLIST lpTransBuf, UINT fuState, HIMC hIMC) {
  UINT ret = 0;
  FOOTMARK();

  TheIME.m_lpCurTransKey = lpTransBuf;
  TheIME.m_uNumTransKey = 0;

  // if hIMC is NULL, this means DISABLE IME.
  if (hIMC) {
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      BOOL fOpen = lpIMC->IsOpen();
      TheIME.UnlockIMC(hIMC);

      if (fOpen) {
        if ((uScanCode & 0x8000) == 0) {
          LPARAM lParam = ((DWORD)uScanCode << 16) + 1L;
          IMEKeyDownHandler(hIMC, uVKey, lParam, lpbKeyState);
        }

        // Clear static value, no more generated message!
        TheIME.m_lpCurTransKey = NULL;
      }

      // If trans key buffer that is allocated by USER.EXE full up,
      // the return value is the negative number.
      if (TheIME.m_fOverTransKey) {
        DebugPrint(TEXT("***************************************\n"));
        DebugPrint(TEXT("*   TransKey OVER FLOW Messages!!!    *\n"));
        DebugPrint(TEXT("*                by MZIMEJA.IME       *\n"));
        DebugPrint(TEXT("***************************************\n"));
      }
      ret = TheIME.m_uNumTransKey;
    }
  }
  return ret;
} // ImeToAsciiEx

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
