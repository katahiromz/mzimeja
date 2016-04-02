// regword.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define FAKEWORD_NOUN IME_REGWORD_STYLE_USER_FIRST
#define FAKEWORD_VERB (IME_REGWORD_STYLE_USER_FIRST + 1)

extern "C" {

//////////////////////////////////////////////////////////////////////////////

//  ImeRegisterWord
//  ImeRegisterWord 関数は現在のIMEの辞書に文字列を登録します。
//  BOOL WINAPI
//    ImeRegisterWord(
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString
//  )
//  (パラメータ)
//    lpszReading
//      登録される文字列の読み。
//    dwStyle
//      登録される文字列のスタイル。次のような値が用意されています。
//  
//      IME_REGWORD_STYLE_EUDC
//        文字列は EUDC の範囲にある。
//      IME_REGWORD_STYLE_USER_FIRST から IME_REGWORD_STYLE_USER_LAST 
//        IME_REGWORD_STYLE_USER_FIRST から 
//        IME_REGWORD_STYLE_USER_LAST の範囲は IME ISV の private
//        style に使われる。IME ISV は自由に独自のスタイルを定義して
//        よい。
//        例えば、
//          #define MSIME_NOUN  (IME_REGWORD_STYLE_USER_FIRST)
//          #define MSIME_VERB  (IME_REGWORD_STYLE_USER_FIRST+1)
//    lpszString
//      登録される文字列。
//  (返り値)
//    成功したならば、TRUE を。さもなくば FALSE を返す。
BOOL WINAPI ImeRegisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr) {
  if ((dw == FAKEWORD_NOUN) || (dw == FAKEWORD_VERB))
    return WritePrivateProfileString(lpRead, lpStr, lpStr, szDicFileName);

  return FALSE;
}

//  ImeUnregisterWord
//  ImeUnregisterWord は現在のIMEの辞書から登録されている文字列を削除しま
//  す。
//  BOOL WINAPI
//    ImeUnregisterWord(
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString
//  )
//  (パラメータ)
//    lpszReading
//      登録されている文字列の読み。
//    dwStyle
//      登録されている文字列のスタイル。dwStyle の中身については
//      ImeRegisterWord の説明を参照のこと。
//    lpszString
//      削除される文字列。
//  (返り値)
//    成功したならば、TRUE を。さもなくば FALSE を返す。
BOOL WINAPI ImeUnregisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr) {
  if ((dw == FAKEWORD_NOUN) || (dw == FAKEWORD_VERB))

    return WritePrivateProfileString(lpRead, lpStr, NULL, szDicFileName);

  return FALSE;
}

//  ImeGetRegisterWordStyle
//  ImeGetRegisterWordStyle は現在のIMEで利用可能なスタイルを得ます。
//  UINT WINAPI
//    ImeGetRegisterWordStyle(
//    UINT nItem,
//    LPSTYLEBUF lpStyleBuf
//  )
//  (パラメータ)
//    nItem
//      バッファに保存可能なスタイルの最大数。
//    lpStyleBuf
//      満たされるべきバッファ。
//  (返り値)
//    バッファにコピーされたスタイルの数が返り値です。もし、nItems が零
//    ならば、このIMEですべての可能なスタイルを受け取るのに必要な配列の
//    要素数になります。
UINT WINAPI ImeGetRegisterWordStyle(UINT u, LPSTYLEBUF lp) {
  UINT uRet = 0;

  if (u > 0 && lp) {
    lp->dwStyle = FAKEWORD_NOUN;
    lstrcpy(lp->szDescription, TEXT("NOUN"));

    if (u > 1) {
      lp++;
      lp->dwStyle = FAKEWORD_VERB;
      lstrcpy(lp->szDescription, TEXT("VERB"));
    }
  } else
    uRet = 2;

  return uRet;
}

//  ImeEnumRegisterWord
//  ImeEnumRegisterWord は特定された読み、スタイル、登録された文字列データ
//  でもって登録されている文字列の情報を列挙します。
//  UINT WINAPI
//    ImeEnumRegisterWord(
//    hKL,
//    REGISTERWORDENUMPROC lpfnEnumProc,
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString,
//    LPVOID lpData
//  )
//  (パラメータ)
//    hKL
//      Input language handle.
//    lpfnEnumProc
//      callback 関数のアドレス。
//    lpszReading
//      列挙される読みを特定します。NULLならば、dwStyle 及び lpszString
//      パラメータに一致する全ての可能な読みを列挙します。
//    dwStyle
//      列挙されるスタイルを特定します。NULLならば、lpszReading 及び
//      lpszString パラメータに一致する全ての可能なスタイルを列挙しま
//      す。
//    lpszString
//      列挙される文字列を特定します。NULLならば、ImeEnumRegisterWord
//      は lpszReading 及び dwStyle パラメータが一致する全ての文字列を
//      列挙します。
//    lpData
//      アプリケーション側が用意したデータのアドレス
//  (返り値)
//    成功ならば、callback function に最後に返された値が返り値になります。
//    アプリケーションによってその意味は決められます。
//  (コメント)
//    全てのパラメータが NULL だったら、IME 辞書の全ての登録されている
//    文字列を列挙します。（うげげ）
//    もじ入力パラメータの２つがNULLだった場合には、残った３つ目のパラメー
//    タに適合する登録されている文字列を全て列挙します。
//  (個人的なコメント)
//    辞書の構造をかなりうまく決めていないと、こんなことをされたら重く
//    てやってられなくなります。正引きは勿論逆引きも可能である、という
//    のは最低条件みたいです。
//    SKK辞書との相性は最悪では？
UINT WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROC lpfn, LPCTSTR lpRead,
                                DWORD dw, LPCTSTR lpStr, LPVOID lpData) {
  UINT uRet = 0;
  TCHAR szBuf[256];
  int nBufLen;
  LPTSTR lpBuf;

  if (!lpfn) return 0;

  lpBuf = (LPTSTR)szBuf;

  if (!dw || (dw == FAKEWORD_NOUN)) {
    if (lpRead) {
      nBufLen = GetPrivateProfileString(
          lpRead, NULL, (LPTSTR)TEXT(""), (LPTSTR)szBuf,
          sizeof(szBuf) / sizeof(szBuf[0]), (LPTSTR)szDicFileName);

      if (nBufLen) {
        while (*lpBuf) {
          if (lpStr && lstrcmp(lpStr, lpBuf)) continue;

          uRet = (*lpfn)(lpRead, dw, lpBuf, lpData);
          lpBuf += (lstrlen(lpBuf) + 1);

          if (!uRet) break;
        }
      }
    } else {
    }
  }

  return uRet;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
