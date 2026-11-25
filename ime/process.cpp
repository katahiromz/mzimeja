// process.cpp --- mzimeja input process
// 入力処理。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "vksub.h"

extern "C" {

// 仮想キー判定関数群
inline BOOL isAlphaKey(UINT vKey) {
    return ('A' <= vKey && vKey <= 'Z');
}
inline BOOL isNumericKey(UINT vKey) {
    return ('0' <= vKey && vKey <= '9');
}
inline BOOL isAlphaNumericKey(UINT vKey) {
    return isAlphaKey(vKey) || isNumericKey(vKey);
}
inline BOOL isNumPadKey(UINT vKey) {
    return VK_NUMPAD0 <= vKey && vKey <= VK_DIVIDE;
}
inline BOOL isDbeKey(UINT vKey) {
    return vKey == VK_KANJI || vKey == VK_CONVERT ||
           (VK_DBE_ALPHANUMERIC <= vKey && vKey <= VK_DBE_ENTERDLGCONVERSIONMODE);
}
inline BOOL isOpenCloseKey(UINT vk) {
    return vk == VK_KANJI || vk == VK_DBE_DBCSCHAR || vk == VK_DBE_SBCSCHAR;
}

//////////////////////////////////////////////////////////////////////////////

BOOL
DoProcessKey(
    HIMC hIMC,
    InputContext *lpIMC,
    WPARAM wParam,
    CONST LPBYTE lpbKeyState,
    BOOL bKeyUp,
    BOOL bDoAction)
{
    DPRINTA("(%p, %p, %p)\n", lpIMC, wParam, lpbKeyState);
    BYTE vk = (BYTE)wParam; // 仮想キー

    if (vk == VK_SHIFT || vk == VK_CONTROL)
        return FALSE; // 処理しない

    BOOL bOpen = lpIMC->IsOpen();
    BOOL bCompStr = lpIMC->HasCompStr();
    BOOL bAlt = !!(lpbKeyState[VK_MENU] & 0x80);
    BOOL bShift = !!(lpbKeyState[VK_SHIFT] & 0x80);
    BOOL bCtrl = !!(lpbKeyState[VK_CONTROL] & 0x80);
    BOOL bCapsLock = !!(lpbKeyState[VK_CAPITAL] & 0x80);
    BOOL bRoman = IsRomanMode(hIMC);

    if (bKeyUp) {
        return FALSE;
    }

    switch (vk) {
    case VK_KANJI: case VK_DBE_DBCSCHAR: case VK_DBE_SBCSCHAR: // [半角／全角]キー
        if (bDoAction) {
            ImmSetOpenStatus(hIMC, !bOpen);
        }
        break;
    case VK_SPACE: // スペース キー
        if (!bOpen)
            return FALSE; // IMEが閉じている場合は処理しない
        if (bCtrl || bAlt)
            return FALSE; // CtrlやAltキーが押されている場合も処理しない
        if (bShift) { // Shift+Space？
            if (bDoAction) {
                if (Config_GetDWORD(TEXT("bNoFullwidthSpace"), FALSE)) { // 全角スペース禁止？
                    // 全角スペース全面禁止だと、全角スペースがどうやっても入力できないから、
                    // Shift+Spaceだけでは特別に認める。
                    if (bCompStr) {
                        lpIMC->AddChar(L'　', UNICODE_NULL); // U+3000
                    } else {
                        TheIME.GenerateMessage(WM_IME_CHAR, L'　', 1); // U+3000
                    }
                } else { // 半角スペース
                    if (bCompStr) {
                        lpIMC->AddChar(L' ', UNICODE_NULL);
                    } else {
                        TheIME.GenerateMessage(WM_IME_CHAR, L' ', 1);
                    }
                }
            }
        } else { // Shiftが押されていない？
            if (bDoAction) {
                if (bCompStr) {
                    lpIMC->Convert(bShift); // スペースは変換キーの代わりになる
                } else {
                    if (Config_GetDWORD(TEXT("bNoFullwidthSpace"), FALSE)) { // 全角スペース禁止？
                        TheIME.GenerateMessage(WM_IME_CHAR, L' ', 1); // 半角スペース
                    } else {
                        TheIME.GenerateMessage(WM_IME_CHAR, L'　', 1); // U+3000
                    }
                }
            }
        }
        break;
    case VK_DBE_ALPHANUMERIC: // 英数キー
        if (!bShift && !bCtrl && bOpen) {
            if (bDoAction) {
                INPUT_MODE imode = GetInputMode(hIMC);
                if (bOpen && imode == IMODE_FULL_HIRAGANA)
                    SetInputMode(hIMC, IMODE_FULL_ASCII);
                else
                    SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_OEM_3: // '~'
        // 英語キーボードなどの Alt+~ を処理する
        if (bAlt && !bShift && !bCtrl) {
            if (bDoAction)
                ImmSetOpenStatus(hIMC, !bOpen);
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_DBE_ROMAN: // ローマ字
        if (bDoAction) {
            if (bOpen)
                SetRomanMode(hIMC, !bRoman);
        }
        break;
    case VK_DBE_HIRAGANA: case VK_DBE_KATAKANA: // [カナ/かな]キー
        if (bDoAction) {
            // 入力モードの状態に応じて入力モードを切り替える
            INPUT_MODE imode = GetInputMode(hIMC);
            switch (imode) {
            case IMODE_FULL_HIRAGANA:
                if (bShift) SetInputMode(hIMC, IMODE_FULL_KATAKANA);
                break;
            case IMODE_FULL_KATAKANA:
                if (!bShift) SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
                break;
            case IMODE_FULL_ASCII: case IMODE_HALF_ASCII:
                if (bShift)
                    SetInputMode(hIMC, IMODE_FULL_KATAKANA);
                else
                    SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
                break;
            case IMODE_HALF_KANA:
                if (!bShift)
                    SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
                break;
            default:
                break;
            }
        }
        break;
    case VK_ESCAPE: // Escキー
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->Escape();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_DELETE: case VK_BACK: // DelキーまたはBkSpキー
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->DeleteChar(vk == VK_BACK);
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_CONVERT: // [変換]キー
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->Convert(bShift);
            }
        } else {
            // TODO: 再変換
            return FALSE; // 処理しない
        }
        break;
    case VK_NONCONVERT: // [無変換]キー
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MakeHiragana();
            }
        }
        break;
    case VK_F5: // F5はコード変換
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->ConvertCode();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_F6: // F6はひらがな変換
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MakeHiragana();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_F7: // F7はカタカナ変換
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MakeKatakana();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_F8: // F8は半角変換
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MakeHankaku();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_F9: // F9は全角英数変換
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MakeZenEisuu();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_F10: // F10は半角英数変換
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MakeHanEisuu();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_RETURN: // Enterキー
        if (bOpen && bCompStr) {
            if (bDoAction) {
                if (lpIMC->Conversion() & IME_CMODE_CHARCODE) {
                    lpIMC->CancelText();
                } else {
                    lpIMC->MakeResult();
                }
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_LEFT: // [←]
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MoveLeft(bShift);
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_RIGHT: // [→]
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MoveRight(bShift);
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_UP: // [↑]
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MoveUp();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_DOWN: // [↓]
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MoveDown();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_PRIOR: // Page Up
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->PageUp();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_NEXT: // Page Down
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->PageUp();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_HOME: // [Home]キー
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MoveHome();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    case VK_END: // [End]キー
        if (bOpen && bCompStr) {
            if (bDoAction) {
                lpIMC->MoveEnd();
            }
        } else {
            return FALSE; // 処理しない
        }
        break;
    default:
        {
            if (!bOpen || bCtrl || bAlt)
                return FALSE; // 処理しない

            // 可能ならキーをひらがなにする。
            WCHAR chTranslated = 0;
            if (!bRoman) {
                chTranslated = mz_vkey_to_hiragana(vk, bShift);
            }

            // 可能ならキーを文字にする。
            WCHAR chTyped;
            if (vk == VK_PACKET) {
                chTyped = chTranslated = HIWORD(wParam);
            } else {
                chTyped = mz_typing_key_to_char(vk, bShift, bCapsLock);
            }

            if (chTranslated || chTyped) { // キーを変換できたら
                if (lpIMC->HasCandInfo() && L'1' <= chTyped && chTyped <= L'9') {
                    // 候補情報があり、候補の選択であれば、候補を選択。
                    if (bDoAction) {
                        lpIMC->SelectCand(chTyped - L'1');
                    }
                } else {
                    // さもなければ文字を追加。
                    if (bDoAction) {
                        lpIMC->AddChar(chTyped, chTranslated);
                    }
                }
                return TRUE;
            }
        }
        return FALSE; // 処理しない
    }

    return TRUE; // 処理すべき／処理した
}

BOOL WINAPI
ImeProcessKey(
    HIMC hIMC,
    UINT vKey,
    LPARAM lKeyData,
    CONST LPBYTE lpbKeyState)
{
    // lKeyData の bit30 = 前回も押下中
    BOOL bRepeat = !!(lKeyData & 0x40000000);
    if (bRepeat && isOpenCloseKey(vKey))
        return FALSE;

    // キー判定
    BOOL ret = FALSE;
    InputContext *lpIMC = NULL;
    if (hIMC && (lpIMC = TheIME.LockIMC(hIMC))) {
        BOOL bKeyUp = !!(lKeyData & 0x80000000);
        ret = DoProcessKey(hIMC, lpIMC, vKey, lpbKeyState, bKeyUp, FALSE);
        TheIME.UnlockIMC(hIMC);
    }

    return ret;
}

UINT WINAPI
ImeToAsciiEx(
    UINT uVKey,
    UINT uScanCode,
    CONST LPBYTE lpbKeyState,
    LPTRANSMSGLIST lpTransBuf,
    UINT fuState,
    HIMC hIMC)
{
    FOOTMARK_FORMAT("(%u (0x%X), 0x%X, %p, %p, 0x%X, %p)\n",
                    uVKey, uVKey, uScanCode, lpbKeyState, lpTransBuf, fuState, hIMC);

    // ImeToAsciiEx: uScanCode の bit14 = 前回も押下中
    BOOL bRepeat = !!(uScanCode & 0x4000);
    if (bRepeat && isOpenCloseKey(uVKey))
        return 0; // 何もしない(下の DoProcessKey も呼ばない)

    TheIME.m_lpCurTransKey = lpTransBuf;
    TheIME.m_uNumTransKey = 0;

    InputContext *lpIMC;
    if (hIMC && (lpIMC = TheIME.LockIMC(hIMC))) {
        BOOL bKeyUp = !!(uScanCode & 0x8000);
        DoProcessKey(hIMC, lpIMC, uVKey, lpbKeyState, bKeyUp, TRUE);

        TheIME.m_lpCurTransKey = NULL;
        TheIME.UnlockIMC(hIMC);
    }

    if (TheIME.m_fOverflowKey) {
        DPRINTA("***************************************\n");
        DPRINTA("*   TransKey OVER FLOW Messages!!!    *\n");
        DPRINTA("*                by MZIMEJA.IME       *\n");
        DPRINTA("***************************************\n");
    }

    FOOTMARK_RETURN_INT(TheIME.m_uNumTransKey);
} // ImeToAsciiEx

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
