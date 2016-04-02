/*++

Copyright (c) 1990-1998 Microsoft Corporation, All Rights Reserved

Module Name:

    IMM.C

++*/
#include "mzimeja.h"

extern "C" {

#if defined(UNICODE)
int GetCandidateStringsFromDictionary(LPWSTR lpString, LPWSTR lpBuf,
                                      DWORD dwBufLen, LPTSTR szDicFileName);
#endif

//  ImeInquire ()
//    For Windows 95, Windows 98, and Windows NT 3.51
//  ImeInquire 関数は IME の初期化を取り扱う。また、ImeInquire 関数は
//  IMEINFO 構造体及び IME の UI class name も又返す。
//  BOOL
//    ImeInquire(
//    LPIMEINFO lpIMEInfo,
//    LPTSTR lpszWndClass,
//    LPCTSTR lpszData
//  )
//  Parameters
//  lpIMEInfo
//    IME info 構造体を指すポインタ。
//  lpszWndClass
//    IME によって window class 名が詰められる。この名前が IME の UI
//    class 名である。
//  lpszData
//    IME オプションブロック。このバージョンでは NULL。
//
//    For Windows NT 4.0 and Windows 2000
//  BOOL
//    ImeInquire(
//    LPIMEINFO lpIMEInfo,
//    LPTSTRlpszWndClass,
//    DWORD dwSystemInfoFlags
//  )
//  Parameters
//  lpIMEInfo
//    IME info 構造体を指すポインタ。
//  lpszWndClass
//    IME によって window class 名が詰められる。この名前が IME の UI
//    class 名である。
//  dwSystemInfoFlags
//    システムによって提供されるシステム情報を変更する。次のようなフラグ
//    が与えられる。
//    IME_SYSINFO_WINLOGON
//      IME にクライアントプロセスが Winlogon プロセスであることを IME
//      に知らせる。IME はこのフラグが指定された時 IME の設定をユーザに
//      許すべきではない。
//    IME_SYSINFO_WOW16
//      IME にクライアントプロセスは 16-bit アプリケーションであること
//      を知らせる。
//  Return Values
//    成功すれば、TRUE。そうでなければ、FALSE。
BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo, LPTSTR lpszClassName,
                       DWORD dwSystemInfoFlags) {
  ImeLog(LOGF_ENTRY | LOGF_API, TEXT("ImeInquire"));

  // Init IMEINFO Structure.
  lpIMEInfo->dwPrivateDataSize = sizeof(UIEXTRA);
  lpIMEInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST |
#if defined(UNICODE)
                           IME_PROP_UNICODE |
#endif
                           IME_PROP_AT_CARET;
  lpIMEInfo->fdwConversionCaps = IME_CMODE_LANGUAGE | IME_CMODE_FULLSHAPE |
                                 IME_CMODE_ROMAN | IME_CMODE_CHARCODE;
  lpIMEInfo->fdwSentenceCaps = 0L;
  lpIMEInfo->fdwUICaps = UI_CAP_2700;

  //  fdwSCSCaps
  //    fdwSCSCaps ビットは SetCompositionString の能力を指定する。
  //    ----------------------------------------------------------
  //    SCS_CAP_COMPSTR
  //      IME は SCS_SETSTR で composition string を生成できる。
  //    SCS_CAP_MAKEREAD
  //      ImmSetCompositionString を SCS_SETSTR で呼び出した時に
  //      IME は compositoin string の reading string を lpRead 
  //      なしに生成することができる。この能力のある IME の下で
  //      はアプリケーションは SCS_SETSTR に lpRead を設定する
  //      必要がない。
  //    SCS_CAP_CONVERTSTRING
  //      IME は reconvert 機能を持っている。
  lpIMEInfo->fdwSCSCaps =
      SCS_CAP_COMPSTR | SCS_CAP_MAKEREAD | SCS_CAP_SETRECONVERTSTRING;

  lpIMEInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;

  lstrcpy(lpszClassName, szUIClassName);

  return TRUE;
}

//  ImeConversionList
//  ImeConversionList 関数によって別の文字もしくは文字列の変換結果のリス
//  トを得ることができる。
//    DWORD
//      IMEConversionList(
//      HIMC      hIMC,
//      LPCTSTR     lpSrc,
//      LPCANDIDATELIST lpDst,
//      DWORD     dwBufLen,
//      UINT      uFlag
//      )
//  Parameters
//  hIMC
//    入力コンテキストのハンドル。
//  lpSrc
//    変換される文字列
//  lpDst
//    変換結果が格納されるバッファを指すポインタ。
//  dwBufLen
//    変換結果を格納するバッファの大きさ。
//  uFlag
//    現在は次の3つのフラグの1つを設定することができる。
//    ・GCL_CONVERSION
//      読み文字列を lpSrc パラメータに設定する。IME は lpDst パラメ
//      ータにその変換結果を返す。
//    ・GCL_REVERSECONVERSION
//      変換結果文字列を lpSrc パラメータに設定する。IME はその読みを
//      lpDstパラメータに返す。
//    ・GCL_REVERSE_LENGTH
//      lpSrc パラメータに変換結果文字列を設定する。IME は
//      GCL_REVERSECONVERSION の扱える長さを返す。例えば、IME は
//      sentence period を持った変換結果を読みに変換することができな
//      い。結果として、sentence period を除いたバイト単位で文字列長
//      を返す。
//  Return Values
//    変換結果文字列リストのバイト数を返す。
//  Comments
//    この関数はアプリケーションもしくは IME-related メッセージを作ら
//    ない IME によって呼び出されることを意図している。それゆえに、IME
//    はこの関数 IME-related メッセージを生成すべきでない。
DWORD WINAPI ImeConversionList(HIMC hIMC, LPCTSTR lpSource,
                               LPCANDIDATELIST lpCandList, DWORD dwBufLen,
                               UINT uFlags) {
  ImeLog(LOGF_API, TEXT("ImeConversionList"));

  return 0;
}

//  ImeDestroy()
//  ImeDestroy 関数は IME 自身を終了させる。
//  BOOL
//    ImeDestroy(
//      UINT uReserved
//    )
//  Parameters
//    uReserved
//      予約されているけど、現在は 0 であれぞかし。このバージョンで
//      は、0でなかったら FALSE を返すこと。
//  Return Values
//    関数が成功したら、TRUE。そうじゃなかったら、FALSE。
BOOL WINAPI ImeDestroy(UINT uForce) {
  ImeLog(LOGF_ENTRY | LOGF_API, TEXT("ImeDestroy"));

  return TRUE;
}

//  ImeEscape
//  ImeEscape 関数はアプリケーションに、他の IMM 関数では直接に利用でき
//  ないような IME の特性にアクセスすることを許す。
//  これは IME の country-specific 関数もしくは private 関数に対してなく
//  てはならないものである…らしい。
//  LRESULT
//    ImeEscape(
//      HIMC hIMC,
//      UINT uEscape,
//      LPVOID lpData
//    )
//  Parameters
//    hIMC
//      入力コンテキストのハンドル。
//    uEscape
//      実行されるエスケープ関数を指定する。
//    lpData
//      指定されたエスケープに必要なデータを指すポインタ。
//  ImeEscape 関数は次のようなエスケープ関数をサポートしている。
//  IME_ESC_QUERY _SUPPORT
//    実装されているかどうかチェックする。もし、このエスケープが実装さ
//    れていなかったら零を返す。
//  IME_ESC_RESERVED_FIRST
//    IME_ESC_RESERVED_FIRST と IME_ESC_RESERVED_LAST の間のエスケープ
//    がシステムに予約される。
//  IME_ESC_RESERVED_LAST
//    IME_ESC_RESERVED_FIRST と IME_ESC_RESERVED_LAST の間のエスケープ
//    がシステムに予約される。
//  IME_ESC_PRIVATE_FIRST
//    IME_ESC_PRIVATE_FIRST と IME_ESC_PRIVATE_LAST の間のエスケープが
//    IME に予約される。IME は自由に自分の目的に応じてこれらのエスケー
//    プ関数を使うことができる。
//  IME_ESC_PRIVATE_LAST
//    IME_ESC_PRIVATE_FIRST と IME_ESC_PRIVATE_LAST の間のエスケープが
//    IME に予約される。IME は自由に自分の目的に応じてこれらのエスケー
//    プ関数を使うことができる。
//  IME_ESC_SEQUENCE_TO_INTERNAL
//    中国語固有のエスケープ。Far East プラットフォームの下で動くこと
//    を要求するアプリケーションはこれを使うべきではない。中国語EUDC
//    エディタのためのものである。*(LPWORD)lpData は sequence code 
//    であり、返り値はその sequence code に対する文字コードである。
//  IME_ESC_GETHELPFILENAME
//    IME のヘルプファイルの名前を得るエスケープ。関数から戻った時に
//    (LPTSTR)lpData は IME ヘルプファイルのフルパスになっている。
//    パス名は 80 * sizeof (TCHAR) より短かい。この関数は Windows'98
//    Windows 2000 に追加されている。
//  IME_ESC_PRIVATE_HOTKEY
//    lpData は hot key ID を格納している DWORD へのポインタである。
//    (IME_HOTKEY_PRIVATE_FIRST からIME_HOTKEY_PRIVATE_LAST の範囲)
//    システムがこの範囲内の hot key を受けとった後、IMM が
//    ImeEscape 関数を使って IME にディスパッチする。WindowsR95 で
//    はサポートしていない。
//  Return Values
//    失敗したら 0 になる。さもなくばエスケープ関数に従って返り値が
//    決まる。
//  Comments
//    パラメータが正当かどうかは各々のエスケープ関数による。
//    uEscape が IME_ESC_QUERY_SUPPORT の時、lpData は IME エスケー
//    プ値を含んだ変数へのポインタである。次のサンプルは、IME が 
//    IME_ESC_GETHELPFILENAME をサポートしているかどうかを決定する
//    のに利用できる。
//    DWORD dwEsc = IME_ESC_GETHELPFILENAME ;
//    LRESULT lRet  = ImmEscape(hKL, hIMC, IME_ESC_QUERYSUPPORT, (LPVOID)&dwEsc) ;
//  See Also
//    ImmEscape
LRESULT WINAPI ImeEscape(HIMC hIMC, UINT uSubFunc, LPVOID lpData) {
  LRESULT lRet = FALSE;

  ImeLog(LOGF_API, TEXT("ImeEscape"));

  switch (uSubFunc) {
    case IME_ESC_QUERY_SUPPORT:
      switch (*(LPUINT)lpData) {
        case IME_ESC_QUERY_SUPPORT:
        case IME_ESC_PRI_GETDWORDTEST:
        case IME_ESC_GETHELPFILENAME:
          lRet = TRUE;
          break;

        default:
          lRet = FALSE;
          break;
      }
      break;

    case IME_ESC_PRI_GETDWORDTEST:
      lRet = 0x12345678;
      break;

    case IME_ESC_GETHELPFILENAME:
      Mylstrcpy((LPMYSTR)lpData, MYTEXT("mzimeja.hlp"));
      lRet = TRUE;
      break;

    default:
      lRet = FALSE;
      break;
  }

  return lRet;
}

//  ImeSetActiveContext ()
//  ImeSetActiveContext 関数は current IME に Input Context が active に
//  なったことを知らせる。
//  BOOL
//    ImeSetActiveContext(
//    HIMC hIMC,
//    BOOL fFlag
//  )
//  Parameters
//    hIMC
//      Input Context Handle
//    fFlag
//      TRUE ならアクティブに FALSE なら非アクティブになったことを示
//      す。
//    Return Values
//      成功したら TRUE を、さもなくば FALSE を返す。
//  Comments
//    IME は新しく選択された Input Context についてこの関数で知らされ
//    る。IME は初期化を実行してよいが、要求されているわけではない。
//  See Also
//    ImeSetActiveContext
BOOL WINAPI ImeSetActiveContext(HIMC hIMC, BOOL fFlag) {
  ImeLog(LOGF_API, TEXT("ImeSetActiveContext"));

  UpdateIndicIcon(hIMC);

  return TRUE;
}

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
  BOOL fRet = FALSE;
  BOOL fOpen;
  BOOL fCompStr = FALSE;
  LPINPUTCONTEXT lpIMC;
  LPCOMPOSITIONSTRING lpCompStr;

  ImeLog(LOGF_KEY | LOGF_API, TEXT("ImeProcessKey"));

  if (lKeyData & 0x80000000) return FALSE;

  if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

  fOpen = lpIMC->fOpen;

  if (fOpen) {
    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    if (lpCompStr) {
      if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING)) &&
          (lpCompStr->dwCompStrLen))
        fCompStr = TRUE;
    }

    if (lpbKeyState[VK_MENU] & 0x80) {
      fRet = FALSE;
    } else if (lpbKeyState[VK_CONTROL] & 0x80) {
      if (fCompStr)
        fRet = (BOOL)bCompCtl[vKey];
      else
        fRet = (BOOL)bNoCompCtl[vKey];
    } else if (lpbKeyState[VK_SHIFT] & 0x80) {
      if (fCompStr)
        fRet = (BOOL)bCompSht[vKey];
      else
        fRet = (BOOL)bNoCompSht[vKey];
    } else {
      if (fCompStr)
        fRet = (BOOL)bComp[vKey];
      else
        fRet = (BOOL)bNoComp[vKey];
    }

    if (lpCompStr) ImmUnlockIMCC(lpIMC->hCompStr);
  }

  ImmUnlockIMC(hIMC);
  return fRet;
}

//  NotifyIME
//  NotifyIME 関数は与えられたパラメータに従って IME の状態を変更する。
//  BOOL
//    NotifyIME(
//    HIMC hIMC,
//    DWORD dwAction,
//    DWORD dwIndex,
//    DWORD dwValue
//  )
//  Parameters
//    hIMC
//      Input context handle.
//    dwAction
//      dwAction パラメータにアプリケーションが指定できる context
//      item は次の通りである。
//  --------------------------------------------------------------------
//  NI_OPENCANDIDATE
//    アプリケーションは IME に候補リストを開かせる。もし IME が候補リ
//    ストを開けば、IME は WM_IME_NOTIFY (サブ関数は IMN_OPENCANDIDATE)
//    メッセージを送信する。
//    dwIndex   dwIndex は開かれた候補リストの index。
//    dwValue   使われない。
//  NI_CLOSECANDIDATE
//    アプリケーションは IME に候補リストを閉じさせる。もし IME が候補
//    リストを閉ざしたら、IME は WM_IME_NOTIFY (サブ関数は 
//    IMN_CLOSE_CANDIDATE)メッセージを送信する。
//    dwIndex   閉じられる候補リストの index
//    dwValue   使われない。
//  NI_SELECTCANDIDATESTR
//    アプリケーションは候補の1つを選択する。
//    dwIndex   選択されている候補リストの index。
//    dwValue   選択されている候補リストの候補文字列の index。
//  NI_CHANGECANDIDATELIST
//    アプリケーションは現在選択されている候補を変更する。
//    dwIndex   選択されている候補リストの index。
//    dwValue   使われない。
//  NI_SETCANDIDATE_PAGESTART
//    アプリケーションは候補リストの index がはじまっているページを
//    変更する。
//    dwIndex   変更される候補リストの index。
//    dwValue   新しいページ開始 index。
//  NI_SETCANDIDATE_PAGESIZE
//    アプリケーションは候補リストのページサイズを変更する。
//    dwIndex   変更される候補リストの index。
//    dwValue   新しいページサイズ。
//  NI_CONTEXTUPDATED
//    アプリケーションもしくはシステムは Input Context を更新する。
//    dwIndex   dwValue の値が IMC_SETCONVERSIONMODE なら dwIndex は
//          前回の conversion mode である。
//          dwValue の値が IMC_SETSENTENCEMODE なら dwIndex は
//          前回の sentence mode である。
//          その他の dwValue については dwIndex は使われない。
//    dwValue   WM_IME_CONTROL メッセージによって使われる次の値の1つ:
//          IMC_SETCANDIDATEPOS
//          IMC_SETCOMPOSITIONFONT
//          IMC_SETCOMPOSITIONWINDOW
//          IMC_SETCONVERSIONMODE
//          IMC_SETSENTENCEMODE
//          IMC_SETOPENSTATUS
//  NI_COMPOSITIONSTR
//    アプリケーションは composition string を変更する。この動作は
//    composition string が input context に存在する時のみ影響する。
//    dwIndex   dwIndex には次のような値が与えられる。
//          CPS_COMPLETE
//            composition string が result string であると
//            決定する。
//          CPS_CONVERT
//            composition string を変換する。
//          CPS_REVERT
//            composition string を revert する。現在の
//            composition string がキャンセルされ、未変換の
//            文字列が composition string として設定される。
//          CPS_CANCEL 
//            composition string をクリアして、composition
//            string が無い状態にする。
//    dwValue   使われない。
//  --------------------------------------------------------------------
//    dwIndex
//      uAction に依存。
//    dwValue
//      uAction に依存。
//  Return Values
//    成功すれば TRUE。そうでなければ FALSE。
//  See Also
//    ImmNotifyIME
BOOL WINAPI NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue) {
  LPINPUTCONTEXT lpIMC;
  BOOL bRet = FALSE;
  LPCOMPOSITIONSTRING lpCompStr;
  LPCANDIDATEINFO lpCandInfo;
  LPCANDIDATELIST lpCandList;
  MYCHAR szBuf[256];
  int nBufLen;
  LPMYSTR lpstr;
  TRANSMSG GnMsg;
  int i = 0;
  //LPDWORD lpdw;

  ImeLog(LOGF_API, TEXT("NotifyIME"));

  switch (dwAction) {
    case NI_CONTEXTUPDATED:
      switch (dwValue) {
        case IMC_SETOPENSTATUS:
          lpIMC = ImmLockIMC(hIMC);
          if (lpIMC) {
            if (!lpIMC->fOpen && IsCompStr(hIMC)) FlushText(hIMC);
            ImmUnlockIMC(hIMC);
          }
          UpdateIndicIcon(hIMC);
          bRet = TRUE;
          break;

        case IMC_SETCONVERSIONMODE:
          break;

        case IMC_SETCOMPOSITIONWINDOW:
          break;

        default:
          break;
      }
      break;

    case NI_COMPOSITIONSTR:
      switch (dwIndex) {
        case CPS_COMPLETE:
          MakeResultString(hIMC, TRUE);
          bRet = TRUE;
          break;

        case CPS_CONVERT:
          ConvKanji(hIMC);
          bRet = TRUE;
          break;

        case CPS_REVERT:
          RevertText(hIMC);
          bRet = TRUE;
          break;

        case CPS_CANCEL:
          FlushText(hIMC);
          bRet = TRUE;
          break;

        default:
          break;
      }
      break;

    case NI_OPENCANDIDATE:
      if (IsConvertedCompStr(hIMC)) {
        if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

        if (!(lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))) {
          ImmUnlockIMC(hIMC);
          return FALSE;
        }

        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        if (lpCandInfo) {
//
// Get the candidate strings from dic file.
//
#if defined(UNICODE)
          nBufLen = GetCandidateStringsFromDictionary(
              GETLPCOMPREADSTR(lpCompStr), (LPMYSTR)szBuf, 256,
              (LPTSTR)szDicFileName);
#else
          nBufLen = GetPrivateProfileString(GETLPCOMPREADSTR(lpCompStr), NULL,
                                            (LPSTR) "", (LPSTR)szBuf, 256,
                                            (LPSTR)szDicFileName);
#endif

          //
          // generate WM_IME_NOTFIY IMN_OPENCANDIDATE message.
          //
          GnMsg.message = WM_IME_NOTIFY;
          GnMsg.wParam = IMN_OPENCANDIDATE;
          GnMsg.lParam = 1L;
          GenerateMessage(hIMC, lpIMC, lpCurTransKey, (LPTRANSMSG)&GnMsg);

          //
          // Make candidate structures.
          //
          lpCandInfo->dwSize = sizeof(MYCAND);
          lpCandInfo->dwCount = 1;
          lpCandInfo->dwOffset[0] =
              (DWORD)((LPSTR) & ((LPMYCAND)lpCandInfo)->cl - (LPSTR)lpCandInfo);
          lpCandList =
              (LPCANDIDATELIST)((LPSTR)lpCandInfo + lpCandInfo->dwOffset[0]);
          //lpdw = (LPDWORD) & (lpCandList->dwOffset);

          lpstr = &szBuf[0];
          while (*lpstr && (i < MAXCANDSTRNUM)) {
            lpCandList->dwOffset[i] = (DWORD)(
                (LPSTR)((LPMYCAND)lpCandInfo)->szCand[i] - (LPSTR)lpCandList);
            Mylstrcpy((LPMYSTR)((LPMYSTR)lpCandList + lpCandList->dwOffset[i]),
                      lpstr);
            lpstr += (Mylstrlen(lpstr) + 1);
            i++;
          }

          lpCandList->dwSize =
              sizeof(CANDIDATELIST) +
              (MAXCANDSTRNUM * (sizeof(DWORD) + MAXCANDSTRSIZE));
          lpCandList->dwStyle = IME_CAND_READ;
          lpCandList->dwCount = i;
          lpCandList->dwPageStart = 0;
          if (i < MAXCANDPAGESIZE)
            lpCandList->dwPageSize = i;
          else
            lpCandList->dwPageSize = MAXCANDPAGESIZE;

          lpCandList->dwSelection++;
          if (lpCandList->dwSelection == (DWORD)i) lpCandList->dwSelection = 0;

          //
          // Generate messages.
          //
          GnMsg.message = WM_IME_NOTIFY;
          GnMsg.wParam = IMN_CHANGECANDIDATE;
          GnMsg.lParam = 1L;
          GenerateMessage(hIMC, lpIMC, lpCurTransKey, (LPTRANSMSG)&GnMsg);

          ImmUnlockIMCC(lpIMC->hCandInfo);
          ImmUnlockIMC(hIMC);

          bRet = TRUE;
        }
      }
      break;

    case NI_CLOSECANDIDATE:
      if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;
      if (IsCandidate(lpIMC)) {
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1L;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey, (LPTRANSMSG)&GnMsg);
        bRet = TRUE;
      }
      ImmUnlockIMC(hIMC);
      break;

    case NI_SELECTCANDIDATESTR:
      if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

      if (dwIndex == 1 && IsCandidate(lpIMC)) {
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        if (lpCandInfo) {
          lpCandList =
              (LPCANDIDATELIST)((LPSTR)lpCandInfo + lpCandInfo->dwOffset[0]);
          if (lpCandList->dwCount > dwValue) {
            lpCandList->dwSelection = dwValue;
            bRet = TRUE;

            //
            // Generate messages.
            //
            GnMsg.message = WM_IME_NOTIFY;
            GnMsg.wParam = IMN_CHANGECANDIDATE;
            GnMsg.lParam = 1L;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey, (LPTRANSMSG)&GnMsg);
          }
          ImmUnlockIMCC(lpIMC->hCandInfo);
        }
      }
      ImmUnlockIMC(hIMC);
      break;

    case NI_CHANGECANDIDATELIST:
      if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

      if (dwIndex == 1 && IsCandidate(lpIMC)) bRet = TRUE;

      ImmUnlockIMC(hIMC);
      break;

    case NI_SETCANDIDATE_PAGESIZE:
      if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

      if (dwIndex == 1 && IsCandidate(lpIMC)) {
        if (dwValue > MAXCANDPAGESIZE) return FALSE;

        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        if (lpCandInfo) {
          lpCandList =
              (LPCANDIDATELIST)((LPSTR)lpCandInfo + lpCandInfo->dwOffset[0]);
          if (lpCandList->dwCount > dwValue) {
            lpCandList->dwPageSize = dwValue;
            bRet = TRUE;

            //
            // Generate messages.
            //
            GnMsg.message = WM_IME_NOTIFY;
            GnMsg.wParam = IMN_CHANGECANDIDATE;
            GnMsg.lParam = 1L;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey, (LPTRANSMSG)&GnMsg);
          }
          ImmUnlockIMCC(lpIMC->hCandInfo);
        }
      }
      ImmUnlockIMC(hIMC);
      break;

    case NI_SETCANDIDATE_PAGESTART:
      if (!(lpIMC = ImmLockIMC(hIMC))) return FALSE;

      if (dwIndex == 1 && IsCandidate(lpIMC)) {
        if (dwValue > MAXCANDPAGESIZE) return FALSE;

        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        if (lpCandInfo) {
          lpCandList =
              (LPCANDIDATELIST)((LPSTR)lpCandInfo + lpCandInfo->dwOffset[0]);
          if (lpCandList->dwCount > dwValue) {
            lpCandList->dwPageStart = dwValue;
            bRet = TRUE;

            //
            // Generate messages.
            //
            GnMsg.message = WM_IME_NOTIFY;
            GnMsg.wParam = IMN_CHANGECANDIDATE;
            GnMsg.lParam = 1L;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey, (LPTRANSMSG)&GnMsg);
          }
          ImmUnlockIMCC(lpIMC->hCandInfo);
        }
      }
      ImmUnlockIMC(hIMC);
      break;

    case NI_IMEMENUSELECTED:
#ifdef _DEBUG
    {
      TCHAR szDev[80];
      OutputDebugString((LPTSTR)TEXT("NotifyIME IMEMENUSELECTED\r\n"));
      wsprintf((LPTSTR)szDev, TEXT("\thIMC is 0x%x\r\n"), hIMC);
      OutputDebugString((LPTSTR)szDev);
      wsprintf((LPTSTR)szDev, TEXT("\tdwIndex is 0x%x\r\n"), dwIndex);
      OutputDebugString((LPTSTR)szDev);
      wsprintf((LPTSTR)szDev, TEXT("\tdwValue is 0x%x\r\n"), dwValue);
      OutputDebugString((LPTSTR)szDev);
    }
#endif
    break;

    default:
      break;
  }
  return bRet;
}

//  ImeSelect
//  ImeSelect 関数は IME private context の初期化及び非初期化処理を行う
//  のに用いられる。
//  BOOL
//    ImeSelect(
//      HIMC hIMC,
//      BOOL fSelect
//    )
//  Parameters
//    hIMC
//      Input context handle
//    fSelect
//      TRUE ならば初期化を、FALSE なら非初期化(リソースの解放)を意
//      味する。
//  Return Values
//    成功すれば TRUE。さもなくば FALSE。
BOOL WINAPI ImeSelect(HIMC hIMC, BOOL fSelect) {
  LPINPUTCONTEXT lpIMC;

  ImeLog(LOGF_ENTRY | LOGF_API, TEXT("ImeSelect"));

  if (fSelect) UpdateIndicIcon(hIMC);

  // it's NULL context.
  if (!hIMC) return TRUE;

  lpIMC = ImmLockIMC(hIMC);
  if (lpIMC) {
    if (fSelect) {
      LPCOMPOSITIONSTRING lpCompStr;
      LPCANDIDATEINFO lpCandInfo;

      // Init the general member of IMC.
      if (!(lpIMC->fdwInit & INIT_LOGFONT)) {
        lpIMC->lfFont.A.lfCharSet = SHIFTJIS_CHARSET;
        lpIMC->fdwInit |= INIT_LOGFONT;
      }

      if (!(lpIMC->fdwInit & INIT_CONVERSION)) {
        lpIMC->fdwConversion =
            IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
        lpIMC->fdwInit |= INIT_CONVERSION;
      }

      lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr, sizeof(MYCOMPSTR));
      lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
      if (lpCompStr) {
        lpCompStr->dwSize = sizeof(MYCOMPSTR);
        ImmUnlockIMCC(lpIMC->hCompStr);
      }
      lpIMC->hCandInfo = ImmReSizeIMCC(lpIMC->hCandInfo, sizeof(MYCAND));
      lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
      if (lpCandInfo) {
        lpCandInfo->dwSize = sizeof(MYCAND);
        ImmUnlockIMCC(lpIMC->hCandInfo);
      }
    }
  }

  ImmUnlockIMC(hIMC);
  return TRUE;
}

#ifdef _DEBUG
void DumpRS(LPRECONVERTSTRING lpRS) {
  TCHAR szDev[80];
  LPMYSTR lpDump = ((LPMYSTR)lpRS) + lpRS->dwStrOffset;
  *(LPMYSTR)(lpDump + lpRS->dwStrLen) = MYTEXT('\0');

  OutputDebugString(TEXT("DumpRS\r\n"));
  wsprintf(szDev, TEXT("dwSize            %x\r\n"), lpRS->dwSize);
  OutputDebugString(szDev);
  wsprintf(szDev, TEXT("dwStrLen          %x\r\n"), lpRS->dwStrLen);
  OutputDebugString(szDev);
  wsprintf(szDev, TEXT("dwStrOffset       %x\r\n"), lpRS->dwStrOffset);
  OutputDebugString(szDev);
  wsprintf(szDev, TEXT("dwCompStrLen      %x\r\n"), lpRS->dwCompStrLen);
  OutputDebugString(szDev);
  wsprintf(szDev, TEXT("dwCompStrOffset   %x\r\n"), lpRS->dwCompStrOffset);
  OutputDebugString(szDev);
  wsprintf(szDev, TEXT("dwTargetStrLen    %x\r\n"), lpRS->dwTargetStrLen);
  OutputDebugString(szDev);
  wsprintf(szDev, TEXT("dwTargetStrOffset %x\r\n"), lpRS->dwTargetStrOffset);
  OutputDebugString(szDev);
  MyOutputDebugString(lpDump);
  OutputDebugString(TEXT("\r\n"));
}
#endif

//  ImeSetCompositionString ()
//  ImeSetCompositionString 関数はアプリケーションに lpComp やlpRead パ
//  ラメータに入れられたデータをIMEのcomposition string 構造体に設定する
//  のに用いられる。IME は WM_IME_COMPOSITION メッセージを生成する。
//  BOOL WINAPI
//    ImeSetCompositionString(
//    HIMC hIMC,
//    DWORD dwIndex,
//    LPCVOID lpComp,
//    DWORD dwCompLen,
//    LPCVOID lpRead,
//    DWORD dwReadLen
//  );
//  Parameters
//    hIMC
//      Input context handle. 
//    dwIndex
//      dwIndex に与えられる値は次の通りである。
//      ------------------------------------------------------------
//  SCS_SETSTR
//    アプリケーションは composition string 又は reading string また
//    はその両方をセットする。少なくとも lpComp か lpRead パラメータ
//    の1つは正当な文字列を指すポインタでなければならない。
//    もし文字列が長すぎるのなら、IME はそれを切り詰め。
//  SCS_CHANGEATTR
//    アプリケーションは composition string 又は reading string 又は
//    その両者の節情報を設定する。少なくとも lpComp か lpRead の1つ
//    は valid な節情報の配列を指すポインタでなければならない。
//  SCS_QUERYRECONVERTSTRING
//    アプリケーションは IME にその RECONVERTSTRINGSTRUCTURE を調整
//    してくれるよう依頼する。もしアプリケーションがこの値を設定して
//    ImeSetCompositionString を呼び出したなら、IME は RECONVERTSTRING
//    構造体を調整しなければならない。アプリケーションはその時調整さ
//    れた RECONVERTSTRING 構造体を SCS_RECONVERTSTRING でもって、
//    この関数に渡すことができる。IME は WM_IMECOMPOSITION メッセージ
//    を生成してはならない。
//  SCS_SETRECONVERTSTRING
//    アプリケーションは IME に RECONVERTSTRING 構造体に含まれる
//    文字列を reconvert するように依頼する。
//      ------------------------------------------------------------
//    lpComp
//      更新された文字列を含むバッファを指すポインタ。文字列の型は
//      dwIndex の値によって決定される。
//    dwCompLen
//      バッファのバイト長。
//    lpRead
//      更新された文字列を含むバッファを指すポインタ。文字列の型は
//      dwIndex の値によって決定される。dwIndex の値が
//      SCS_SETRECONVERTSTRING もしくは SCS_QUERYRECONVERTSTRING な
//      らば、lpRead は更新された reading string を含む
//      RECONVERTSTRING 構造体を指すポインタである。もし選択された
//      IME が SCS_CAP_MAKEREAD をもっていれば、NULL になる。
//    dwReadLen
//      バッファのバイト長。
//  Comments
//    Unicode については、たとえ SCS_SETSTR が特定されており Unicode 
//    文字列を含んでいても、dwCompLen や dwReadLen はバッファのバイト
//    長である。
//    SCS_SETRECONVERTSTRING または SCS_QUERYRECONVERTSTRING は
//    SCS_CAP_CONVERTSTRING プロパティを持った IME にのみ利用できる。
//    このプロパティは ImmGetProperty 関数を使うことで得ることができる。
BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPVOID lpComp,
                                    DWORD dwComp, LPVOID lpRead, DWORD dwRead) {
  ImeLog(LOGF_API, TEXT("ImeSetCompositionString"));

  // サイズがおまかせの場合には、取り敢えずは dwStrLen に
  // あわせる。kakasi とこの位置で連携した方がいいのかもし
  // れない。
  // サイズが大き過ぎるなら調整しなければ。

  switch (dwIndex) {
    case SCS_QUERYRECONVERTSTRING:
#ifdef _DEBUG
      OutputDebugString(TEXT("SCS_QUERYRECONVERTSTRING\r\n"));
      if (lpComp) DumpRS((LPRECONVERTSTRING)lpComp);
      if (lpRead) DumpRS((LPRECONVERTSTRING)lpRead);
#endif
      break;

    case SCS_SETRECONVERTSTRING:
#ifdef _DEBUG
      OutputDebugString(TEXT("SCS_SETRECONVERTSTRING\r\n"));
      if (lpComp) DumpRS((LPRECONVERTSTRING)lpComp);
      if (lpRead) DumpRS((LPRECONVERTSTRING)lpRead);
#endif
      break;
  }

  return FALSE;
}

//  ImeGetImeMenuItems
//  ImeGetImeMenuItems 関数は IME menu に登録されている menu item を得る。
//  DWORD WINAPI
//    ImeGetImeMenuItems(
//    HIMC hIMC,
//    DWORD dwFlags,
//    DWORD dwType,
//    LPIMEMENUITEMINFO lpImeParentMenu,
//    LPIMEMENUITEMINFO lpImeMenu,
//    DWORD dwSize
//  )
//  Parameters
//    hIMC
//      lpMenuItem はこの input context に関連付けられている menu
//      item を含む。
//    dwFlags
//      続くビットの組み合わせからなる。
//      ------------------------------------------------------------
//      IGIMIF_RIGHTMENU  
//        もし 1 なら、この関数は右クリック Context menu に対する
//        menu item を返す。
//      ------------------------------------------------------------
//    dwType
//      続くビットの組み合わせからなる。
//      ------------------------------------------------------------
//      IGIMII_CMODE
//        conversion mode に関連付けられている menu item を返す。
//      IGIMII_SMODE
//        sentence mode に関連付けられている menu item を返す。
//      IGIMII_CONFIGURE
//        IME の設定に関連付けられている menu item を返す。
//      IGIMII_TOOLS
//        IME ツールに関連付けられている menu item を返す。
//      IGIMII_HELP
//        IME ヘルプに関連付けられている menu item を返す。
//      IGIMII_OTHER
//        その他の menu item を返す。
//      IGIMII_INPUTTOOLS
//        拡張された文字の入力方法を提供する IME 入力ツールに関連
//        付けられた menu item を返す。
//      ------------------------------------------------------------
//    lpImeParentMenu
//      fType に MFT_SUBMENU を持った IMEMENUINFO 構造体を指すポイン
//      タ。ImeGetMenuItems はこの menu item の submenu item を返す。
//      もし NULL なら lpImeMenu は top-level の IME menu item を含
//      んでいる。
//    lpImeMenu
//      menu item の内容を受け取るバッファを指したポインタ。このバッ
//      ファは IMEMENUINFO 構造体の配列である。もし NULL なら
//      ImeGetImeMenuItems は登録される menu item の数を返す。
//    dwSize
//      IMEMENUITEMINFO 構造体を受けるバッファのサイズ。
//  Return Values
//    lpIM に設定された menu item の数が帰る。もし lpImeMenu が NULL 
//    ならば ImeMenuItems は指定された hKL に登録されている menu item 
//    の数を返す。
//  ImeGetImeMenuItems は Windows'98 や Windows 2000 用の新しい関数である。
DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType,
                                LPMYIMEMENUITEMINFO lpImeParentMenu,
                                LPMYIMEMENUITEMINFO lpImeMenu, DWORD dwSize) {
  // dwType を MSIME はチェックしていないようだ。それに合わせる。
  // ただ、TSF が生きていると、この method は常に動作してないように思えるが。
  //
  // if (IsTSFEnabled ())
  //   return  0 ;
  //
  // このコードを有効にするかどうかは微妙だ。
  ImeLog(LOGF_API, TEXT("ImeGetImeMenuItems"));

  if (!lpImeMenu) {
    if (!lpImeParentMenu) {
      if (dwFlags & IGIMIF_RIGHTMENU)
        return NUM_ROOT_MENU_R;
      else
        return NUM_ROOT_MENU_L;
    } else {
      if (dwFlags & IGIMIF_RIGHTMENU)
        return NUM_SUB_MENU_R;
      else
        return NUM_SUB_MENU_L;
    }

    return 0;
  }

  if (!lpImeParentMenu) {
    if (dwFlags & IGIMIF_RIGHTMENU) {
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_ROOT_MR_1;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("RootRightMenu1"));
      lpImeMenu->hbmpItem = 0;

      lpImeMenu++;
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = IMFT_SUBMENU;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_ROOT_MR_2;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("RootRightMenu2"));
      lpImeMenu->hbmpItem = 0;

      lpImeMenu++;
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_ROOT_MR_3;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("RootRightMenu3"));
      lpImeMenu->hbmpItem = 0;

      return NUM_ROOT_MENU_R;
    } else {
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_ROOT_ML_1;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("RootLeftMenu1"));
      lpImeMenu->hbmpItem = 0;

      lpImeMenu++;
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = IMFT_SUBMENU;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_ROOT_ML_2;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("RootLeftMenu2"));
      lpImeMenu->hbmpItem = 0;

      lpImeMenu++;
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_ROOT_ML_3;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("RootLeftMenu3"));
      lpImeMenu->hbmpItem = LoadBitmap(hInst, TEXT("FACEBMP"));

      return NUM_ROOT_MENU_L;
    }
  } else {
    if (dwFlags & IGIMIF_RIGHTMENU) {
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_SUB_MR_1;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("SubRightMenu1"));
      lpImeMenu->hbmpItem = 0;

      lpImeMenu++;
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = 0;
      lpImeMenu->wID = IDIM_SUB_MR_2;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("SubRightMenu2"));
      lpImeMenu->hbmpItem = 0;

      return NUM_SUB_MENU_R;
    } else {
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = IMFS_CHECKED;
      lpImeMenu->wID = IDIM_SUB_ML_1;
      lpImeMenu->hbmpChecked = 0;
      lpImeMenu->hbmpUnchecked = 0;
      Mylstrcpy(lpImeMenu->szString, MYTEXT("SubLeftMenu1"));
      lpImeMenu->hbmpItem = 0;

      lpImeMenu++;
      lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
      lpImeMenu->fType = 0;
      lpImeMenu->fState = IMFS_CHECKED;
      lpImeMenu->wID = IDIM_SUB_ML_2;
      lpImeMenu->hbmpChecked = LoadBitmap(hInst, TEXT("CHECKBMP"));
      lpImeMenu->hbmpUnchecked = LoadBitmap(hInst, TEXT("UNCHECKBMP"));
      Mylstrcpy(lpImeMenu->szString, MYTEXT("SubLeftMenu2"));
      lpImeMenu->hbmpItem = 0;

      return NUM_SUB_MENU_L;
    }
  }

  return 0;
}

}  // extern "C"
