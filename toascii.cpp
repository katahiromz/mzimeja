// toascii.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

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
  LPARAM lParam;
  LPINPUTCONTEXT lpIMC;
  BOOL fOpen;

  DebugPrint(TEXT("ImeToAsciiEx"));

  lpCurTransKey = lpTransBuf;
  lParam = ((DWORD)uScanCode << 16) + 1L;

  // Init uNumTransKey here.
  uNumTransKey = 0;

  // if hIMC is NULL, this means DISABLE IME.
  if (!hIMC) return 0;

  if (!(lpIMC = ImmLockIMC(hIMC))) return 0;

  fOpen = lpIMC->fOpen;

  ImmUnlockIMC(hIMC);

  // The current status of IME is "closed".
  if (!fOpen) goto itae_exit;

  if (uScanCode & 0x8000)
    IMEKeyupHandler(hIMC, uVKey, lParam, lpbKeyState);
  else
    IMEKeydownHandler(hIMC, uVKey, lParam, lpbKeyState);

  // Clear static value, no more generated message!
  lpCurTransKey = NULL;

itae_exit:

  // If trans key buffer that is allocated by USER.EXE full up,
  // the return value is the negative number.
  if (fOverTransKey) {
#ifdef _DEBUG
    OutputDebugString(
        TEXT("***************************************\r\n"));
    OutputDebugString(
        TEXT("*   TransKey OVER FLOW Messages!!!    *\r\n"));
    OutputDebugString(
        TEXT("*                by MZIMEJA.DLL       *\r\n"));
    OutputDebugString(
        TEXT("***************************************\r\n"));
#endif
    return (int)uNumTransKey;
  }

  return (int)uNumTransKey;
}

// Update the transrate key buffer
BOOL PASCAL GenerateMessageToTransKey(LPTRANSMSGLIST lpTransBuf,
                                      LPTRANSMSG lpGeneMsg) {
  LPTRANSMSG lpgmT0;

  uNumTransKey++;
  if (uNumTransKey >= lpTransBuf->uMsgCount) {
    fOverTransKey = TRUE;
    return FALSE;
  }

  lpgmT0 = lpTransBuf->TransMsg + (uNumTransKey - 1);
  *lpgmT0 = *lpGeneMsg;

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
