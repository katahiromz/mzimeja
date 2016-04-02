// regword.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define FAKEWORD_NOUN IME_REGWORD_STYLE_USER_FIRST
#define FAKEWORD_VERB (IME_REGWORD_STYLE_USER_FIRST + 1)

extern "C" {

//////////////////////////////////////////////////////////////////////////////

//  ImeRegisterWord
//  ImeRegisterWord �֐��͌��݂�IME�̎����ɕ������o�^���܂��B
//  BOOL WINAPI
//    ImeRegisterWord(
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString
//  )
//  (�p�����[�^)
//    lpszReading
//      �o�^����镶����̓ǂ݁B
//    dwStyle
//      �o�^����镶����̃X�^�C���B���̂悤�Ȓl���p�ӂ���Ă��܂��B
//  
//      IME_REGWORD_STYLE_EUDC
//        ������� EUDC �͈̔͂ɂ���B
//      IME_REGWORD_STYLE_USER_FIRST ���� IME_REGWORD_STYLE_USER_LAST 
//        IME_REGWORD_STYLE_USER_FIRST ���� 
//        IME_REGWORD_STYLE_USER_LAST �͈̔͂� IME ISV �� private
//        style �Ɏg����BIME ISV �͎��R�ɓƎ��̃X�^�C�����`����
//        �悢�B
//        �Ⴆ�΁A
//          #define MSIME_NOUN  (IME_REGWORD_STYLE_USER_FIRST)
//          #define MSIME_VERB  (IME_REGWORD_STYLE_USER_FIRST+1)
//    lpszString
//      �o�^����镶����B
//  (�Ԃ�l)
//    ���������Ȃ�΁ATRUE ���B�����Ȃ��� FALSE ��Ԃ��B
BOOL WINAPI ImeRegisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr) {
  if ((dw == FAKEWORD_NOUN) || (dw == FAKEWORD_VERB))
    return WritePrivateProfileString(lpRead, lpStr, lpStr, szDicFileName);

  return FALSE;
}

//  ImeUnregisterWord
//  ImeUnregisterWord �͌��݂�IME�̎�������o�^����Ă��镶������폜����
//  ���B
//  BOOL WINAPI
//    ImeUnregisterWord(
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString
//  )
//  (�p�����[�^)
//    lpszReading
//      �o�^����Ă��镶����̓ǂ݁B
//    dwStyle
//      �o�^����Ă��镶����̃X�^�C���BdwStyle �̒��g�ɂ��Ă�
//      ImeRegisterWord �̐������Q�Ƃ̂��ƁB
//    lpszString
//      �폜����镶����B
//  (�Ԃ�l)
//    ���������Ȃ�΁ATRUE ���B�����Ȃ��� FALSE ��Ԃ��B
BOOL WINAPI ImeUnregisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr) {
  if ((dw == FAKEWORD_NOUN) || (dw == FAKEWORD_VERB))

    return WritePrivateProfileString(lpRead, lpStr, NULL, szDicFileName);

  return FALSE;
}

//  ImeGetRegisterWordStyle
//  ImeGetRegisterWordStyle �͌��݂�IME�ŗ��p�\�ȃX�^�C���𓾂܂��B
//  UINT WINAPI
//    ImeGetRegisterWordStyle(
//    UINT nItem,
//    LPSTYLEBUF lpStyleBuf
//  )
//  (�p�����[�^)
//    nItem
//      �o�b�t�@�ɕۑ��\�ȃX�^�C���̍ő吔�B
//    lpStyleBuf
//      ���������ׂ��o�b�t�@�B
//  (�Ԃ�l)
//    �o�b�t�@�ɃR�s�[���ꂽ�X�^�C���̐����Ԃ�l�ł��B�����AnItems ����
//    �Ȃ�΁A����IME�ł��ׂẲ\�ȃX�^�C�����󂯎��̂ɕK�v�Ȕz���
//    �v�f���ɂȂ�܂��B
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
//  ImeEnumRegisterWord �͓��肳�ꂽ�ǂ݁A�X�^�C���A�o�^���ꂽ������f�[�^
//  �ł����ēo�^����Ă��镶����̏���񋓂��܂��B
//  UINT WINAPI
//    ImeEnumRegisterWord(
//    hKL,
//    REGISTERWORDENUMPROC lpfnEnumProc,
//    LPCTSTR lpszReading,
//    DWORD dwStyle,
//    LPCTSTR lpszString,
//    LPVOID lpData
//  )
//  (�p�����[�^)
//    hKL
//      Input language handle.
//    lpfnEnumProc
//      callback �֐��̃A�h���X�B
//    lpszReading
//      �񋓂����ǂ݂���肵�܂��BNULL�Ȃ�΁AdwStyle �y�� lpszString
//      �p�����[�^�Ɉ�v����S�Ẳ\�ȓǂ݂�񋓂��܂��B
//    dwStyle
//      �񋓂����X�^�C������肵�܂��BNULL�Ȃ�΁AlpszReading �y��
//      lpszString �p�����[�^�Ɉ�v����S�Ẳ\�ȃX�^�C����񋓂���
//      ���B
//    lpszString
//      �񋓂���镶�������肵�܂��BNULL�Ȃ�΁AImeEnumRegisterWord
//      �� lpszReading �y�� dwStyle �p�����[�^����v����S�Ă̕������
//      �񋓂��܂��B
//    lpData
//      �A�v���P�[�V���������p�ӂ����f�[�^�̃A�h���X
//  (�Ԃ�l)
//    �����Ȃ�΁Acallback function �ɍŌ�ɕԂ��ꂽ�l���Ԃ�l�ɂȂ�܂��B
//    �A�v���P�[�V�����ɂ���Ă��̈Ӗ��͌��߂��܂��B
//  (�R�����g)
//    �S�Ẵp�����[�^�� NULL ��������AIME �����̑S�Ă̓o�^����Ă���
//    �������񋓂��܂��B�i�������j
//    �������̓p�����[�^�̂Q��NULL�������ꍇ�ɂ́A�c�����R�ڂ̃p�����[
//    �^�ɓK������o�^����Ă��镶�����S�ė񋓂��܂��B
//  (�l�I�ȃR�����g)
//    �����̍\�������Ȃ肤�܂����߂Ă��Ȃ��ƁA����Ȃ��Ƃ����ꂽ��d��
//    �Ă���Ă��Ȃ��Ȃ�܂��B�������͖ܘ_�t�������\�ł���A�Ƃ���
//    �͍̂Œ�����݂����ł��B
//    SKK�����Ƃ̑����͍ň��ł́H
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
