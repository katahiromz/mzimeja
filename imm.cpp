// imm.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

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
  FOOTMARK();

  lpIMEInfo->dwPrivateDataSize = sizeof(UIEXTRA);
  lpIMEInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST |
                           IME_PROP_UNICODE | IME_PROP_AT_CARET;
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

  if (dwSystemInfoFlags & IME_SYSINFO_WINLOGON) {
    TheIME.m_bWinLogOn = TRUE;
  }

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
  FOOTMARK();

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
  FOOTMARK();

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
  FOOTMARK();

  switch (uSubFunc) {
    case IME_ESC_QUERY_SUPPORT:
      switch (*(LPUINT)lpData) {
        case IME_ESC_QUERY_SUPPORT:
        case IME_ESC_GETHELPFILENAME:
          lRet = TRUE;
          break;

        default:
          lRet = FALSE;
          break;
      }
      break;

    case IME_ESC_GETHELPFILENAME:
      lstrcpy((LPTSTR)lpData, TEXT("mzimeja.hlp"));
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
  FOOTMARK();

  TheIME.UpdateIndicIcon(hIMC);

  return TRUE;
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
  InputContext *lpIMC;
  BOOL bRet = FALSE;
  CompStr *lpCompStr;
  CandInfo *lpCandInfo;
  CandList *lpCandList;
  TCHAR szBuf[256];
  LPTSTR lpstr;
  DWORD i = 0;
  //LPDWORD lpdw;

  FOOTMARK();

  switch (dwAction) {
    case NI_CONTEXTUPDATED:
      DebugPrint(TEXT("NI_CONTEXTUPDATED\n"));
      switch (dwValue) {
        case IMC_SETOPENSTATUS:
          if (!dwIndex) {
            lpIMC = TheIME.LockIMC(hIMC);
            if (lpIMC) {
              lpIMC->CancelText();
              TheIME.UnlockIMC(hIMC);
            }
          }
          TheIME.UpdateIndicIcon(hIMC);
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
      DebugPrint(TEXT("NI_COMPOSITIONSTR\n"));
      switch (dwIndex) {
        case CPS_COMPLETE:
          lpIMC = TheIME.LockIMC(hIMC);
          if (lpIMC) {
            lpIMC->MakeResult();
            TheIME.UnlockIMC(hIMC);
          }
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
          lpIMC = TheIME.LockIMC(hIMC);
          if (lpIMC) {
            lpIMC->CancelText();
            TheIME.UnlockIMC(hIMC);
          }
          bRet = TRUE;
          break;

        default:
          break;
      }
      break;

    case NI_OPENCANDIDATE:
      DebugPrint(TEXT("NI_OPENCANDIDATE\n"));
      lpIMC = TheIME.LockIMC(hIMC);
      if (!lpIMC) return FALSE;
      if (!(lpCompStr = lpIMC->LockCompStr())) {
        TheIME.UnlockIMC(hIMC);
        return FALSE;
      }
      if (!lpIMC->HasConvertedCompStr()) {
        TheIME.UnlockIMC(hIMC);
        return FALSE;
      }

      lpCandInfo = lpIMC->LockCandInfo();
      if (lpCandInfo) {
        // Get the candidate strings from dic file.
        GetCandidateStringsFromDictionary(
            lpCompStr->GetCompReadStr(), szBuf, 256, TheIME.m_szDicFileName);

        // generate WM_IME_NOTFIY IMN_OPENCANDIDATE message.
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_OPENCANDIDATE, 1);

        // Make candidate structures.
        lpCandInfo->dwSize = sizeof(MZCAND);
        lpCandInfo->dwCount = 1;
        lpCandInfo->dwOffset[0] = sizeof(CANDIDATEINFO);
        lpCandList = lpCandInfo->GetList();

        lpstr = &szBuf[0];
        while (*lpstr && (i < MAXCANDSTRNUM)) {
          lpCandList->dwOffset[i] = lpCandInfo->GetCandOffset(i, lpCandList);
          lstrcpy(lpCandList->GetCandString(i), lpstr);
          lpstr += (lstrlen(lpstr) + 1);
          i++;
        }

        lpCandList->dwSize = sizeof(CANDIDATELIST);
        lpCandList->dwSize +=
          (MAXCANDSTRNUM * (sizeof(DWORD) + MAXCANDSTRSIZE));
        lpCandList->dwStyle = IME_CAND_READ;
        lpCandList->dwCount = i;
        lpCandList->dwPageStart = 0;
        if (i < MAXCANDPAGESIZE)
          lpCandList->dwPageSize = i;
        else
          lpCandList->dwPageSize = MAXCANDPAGESIZE;

        lpCandList->dwSelection++;
        if (lpCandList->dwSelection == i) lpCandList->dwSelection = 0;

        //
        // Generate messages.
        //
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);

        lpIMC->UnlockCandInfo();
        TheIME.UnlockIMC(hIMC);

        bRet = TRUE;
      }
      break;

    case NI_CLOSECANDIDATE:
      DebugPrint(TEXT("NI_CLOSECANDIDATE\n"));
      lpIMC = TheIME.LockIMC(hIMC);
      if (!lpIMC) return FALSE;
      if (lpIMC->HasCandInfo()) {
        TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CLOSECANDIDATE, 1);
        bRet = TRUE;
      }
      TheIME.UnlockIMC(hIMC);
      break;

    case NI_SELECTCANDIDATESTR:
      DebugPrint(TEXT("NI_SELECTCANDIDATESTR\n"));
      lpIMC = TheIME.LockIMC(hIMC);
      if (!lpIMC) return FALSE;

      if (dwIndex == 1 && lpIMC->HasCandInfo()) {
        lpCandInfo = lpIMC->LockCandInfo();
        if (lpCandInfo) {
          lpCandList = lpCandInfo->GetList();
          if (lpCandList->dwCount > dwValue) {
            lpCandList->dwSelection = dwValue;
            bRet = TRUE;

            // Generate messages.
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
          }
          lpIMC->UnlockCandInfo();
        }
      }
      TheIME.UnlockIMC(hIMC);
      break;

    case NI_CHANGECANDIDATELIST:
      DebugPrint(TEXT("NI_CHANGECANDIDATELIST\n"));
      lpIMC = TheIME.LockIMC(hIMC);
      if (!lpIMC) return FALSE;
      if (dwIndex == 1 && lpIMC->HasCandInfo()) bRet = TRUE;
      TheIME.UnlockIMC(hIMC);
      break;

    case NI_SETCANDIDATE_PAGESIZE:
      DebugPrint(TEXT("NI_SETCANDIDATE_PAGESIZE\n"));
      lpIMC = TheIME.LockIMC(hIMC);
      if (!lpIMC) return FALSE;
      if (dwIndex == 1 && lpIMC->HasCandInfo()) {
        if (dwValue > MAXCANDPAGESIZE) return FALSE;

        lpCandInfo = lpIMC->LockCandInfo();
        if (lpCandInfo) {
          lpCandList = lpCandInfo->GetList();
          if (lpCandList->dwCount > dwValue) {
            lpCandList->dwPageSize = dwValue;
            bRet = TRUE;

            //
            // Generate messages.
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
          }
          lpIMC->UnlockCandInfo();
        }
      }
      TheIME.UnlockIMC(hIMC);
      break;

    case NI_SETCANDIDATE_PAGESTART:
      DebugPrint(TEXT("NI_SETCANDIDATE_PAGESTART\n"));
      lpIMC = TheIME.LockIMC(hIMC);
      if (!lpIMC) return FALSE;
      if (dwIndex == 1 && lpIMC->HasCandInfo()) {
        if (dwValue > MAXCANDPAGESIZE) return FALSE;

        lpCandInfo = lpIMC->LockCandInfo();
        if (lpCandInfo) {
          lpCandList = lpCandInfo->GetList();
          if (lpCandList->dwCount > dwValue) {
            lpCandList->dwPageStart = dwValue;
            bRet = TRUE;

            //
            // Generate messages.
            TheIME.GenerateMessage(WM_IME_NOTIFY, IMN_CHANGECANDIDATE, 1);
          }
          lpIMC->UnlockCandInfo();
        }
      }
      TheIME.UnlockIMC(hIMC);
      break;

    case NI_IMEMENUSELECTED:
      DebugPrint(TEXT("NI_IMEMENUSELECTED\n"));
      DebugPrint(TEXT("\thIMC is 0x%x\n"), hIMC);
      DebugPrint(TEXT("\tdwIndex is 0x%x\n"), dwIndex);
      DebugPrint(TEXT("\tdwValue is 0x%x\n"), dwValue);
      TheIME.DoCommand(hIMC, dwIndex);
      break;

    default:
      DebugPrint(TEXT("NI_(unknown)\n"));
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
  FOOTMARK();

  if (fSelect) TheIME.UpdateIndicIcon(hIMC);
  if (NULL != hIMC) {
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (fSelect) {
        lpIMC->Initialize();
      }
      TheIME.UnlockIMC(hIMC);
    }
  }
  return TRUE;
}

#ifdef _DEBUG
void DumpRS(LPRECONVERTSTRING lpRS) {
  LPTSTR lpDump = ((LPTSTR)lpRS) + lpRS->dwStrOffset;
  *(LPTSTR)(lpDump + lpRS->dwStrLen) = 0;

  DebugPrint(TEXT("DumpRS\n"));
  DebugPrint(TEXT("dwSize            %x\n"), lpRS->dwSize);
  DebugPrint(TEXT("dwStrLen          %x\n"), lpRS->dwStrLen);
  DebugPrint(TEXT("dwStrOffset       %x\n"), lpRS->dwStrOffset);
  DebugPrint(TEXT("dwCompStrLen      %x\n"), lpRS->dwCompStrLen);
  DebugPrint(TEXT("dwCompStrOffset   %x\n"), lpRS->dwCompStrOffset);
  DebugPrint(TEXT("dwTargetStrLen    %x\n"), lpRS->dwTargetStrLen);
  DebugPrint(TEXT("dwTargetStrOffset %x\n"), lpRS->dwTargetStrOffset);
  DebugPrint(TEXT("%s\n"), lpDump);
}
#endif  // ndef _DEBUG

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
  FOOTMARK();

  // サイズがおまかせの場合には、取り敢えずは dwStrLen に
  // あわせる。kakasi とこの位置で連携した方がいいのかもし
  // れない。
  // サイズが大き過ぎるなら調整しなければ。

  switch (dwIndex) {
    case SCS_QUERYRECONVERTSTRING:
      DebugPrint(TEXT("SCS_QUERYRECONVERTSTRING\n"));
#ifdef _DEBUG
      if (lpComp) DumpRS((LPRECONVERTSTRING)lpComp);
      if (lpRead) DumpRS((LPRECONVERTSTRING)lpRead);
#endif
      break;

    case SCS_SETRECONVERTSTRING:
      DebugPrint(TEXT("SCS_SETRECONVERTSTRING\n"));
#ifdef _DEBUG
      if (lpComp) DumpRS((LPRECONVERTSTRING)lpComp);
      if (lpRead) DumpRS((LPRECONVERTSTRING)lpRead);
#endif
      break;

    default:
      DebugPrint(TEXT("SCS_(unknown)\n"));
      break;
  }

  return FALSE;
}

struct MYMENUITEM {
  INT nCommandID;
  INT nStringID;
};
static const MYMENUITEM top_menu_items[] = {
  {IDM_HIRAGANA, IDM_HIRAGANA},
  {IDM_ZEN_KATAKANA, IDM_ZEN_KATAKANA},
  {IDM_ZEN_ALNUM, IDM_ZEN_ALNUM},
  {IDM_HAN_KATAKANA, IDM_HAN_KATAKANA},
  {IDM_ALNUM, IDM_ALNUM},
  {-1, -1},
  {IDM_ROMAJI_INPUT, IDM_ROMAJI_INPUT},
  {IDM_KANA_INPUT, IDM_KANA_INPUT},
  {-1, -1},
  {IDM_ADD_WORD, IDM_ADD_WORD},
  {IDM_RECONVERT, IDM_RECONVERT},
  {-1, -1},
  {IDM_PROPERTY, IDM_PROPERTY},
  {IDM_ABOUT, IDM_ABOUT},
};

DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType,
                                LPIMEMENUITEMINFO lpImeParentMenu,
                                LPIMEMENUITEMINFO lpImeMenu, DWORD dwSize) {
  INT ret = 0;
  FOOTMARK();
  // dwType を MSIME はチェックしていないようだ。それに合わせる。
  // ただ、TSF が生きていると、この method は常に動作してないように思えるが。
  //
  // if (IsTSFEnabled ())
  //   return  0 ;
  //
  // このコードを有効にするかどうかは微妙だ。

  if (lpImeMenu == NULL) {
    if (lpImeParentMenu == NULL) {
      if (dwFlags & IGIMIF_RIGHTMENU)
        ret = _countof(top_menu_items);
    }
    return ret;
  }

  if (lpImeParentMenu == NULL) {
    if (dwFlags & IGIMIF_RIGHTMENU) {
      BOOL bOpen;
      bOpen = ImmGetOpenStatus(hIMC);
      DWORD dwConversion, dwSentence;
      ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence);
      InputMode imode;
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
          if (imode == IMODE_ZEN_HIRAGANA) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_ZEN_KATAKANA:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_ZEN_KATAKANA) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_ZEN_ALNUM:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_ZEN_EISUU) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_HAN_KATAKANA:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_HAN_KANA) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_ALNUM:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (imode == IMODE_HAN_EISUU) {
            lpImeMenu[i].fState = IMFS_CHECKED;
          }
          break;
        case IDM_ROMAJI_INPUT:
        case IDM_KANA_INPUT:
          lpImeMenu[i].fType = IMFT_RADIOCHECK;
          if (dwConversion & IME_CMODE_ROMAN) {
            if (item.nCommandID == IDM_ROMAJI_INPUT) {
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

  return ret;
} // ImeGetImeMenuItems

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
