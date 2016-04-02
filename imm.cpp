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
//  ImeInquire �֐��� IME �̏���������舵���B�܂��AImeInquire �֐���
//  IMEINFO �\���̋y�� IME �� UI class name �����Ԃ��B
//  BOOL
//    ImeInquire(
//    LPIMEINFO lpIMEInfo,
//    LPTSTR lpszWndClass,
//    LPCTSTR lpszData
//  )
//  Parameters
//  lpIMEInfo
//    IME info �\���̂��w���|�C���^�B
//  lpszWndClass
//    IME �ɂ���� window class �����l�߂���B���̖��O�� IME �� UI
//    class ���ł���B
//  lpszData
//    IME �I�v�V�����u���b�N�B���̃o�[�W�����ł� NULL�B
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
//    IME info �\���̂��w���|�C���^�B
//  lpszWndClass
//    IME �ɂ���� window class �����l�߂���B���̖��O�� IME �� UI
//    class ���ł���B
//  dwSystemInfoFlags
//    �V�X�e���ɂ���Ē񋟂����V�X�e������ύX����B���̂悤�ȃt���O
//    ���^������B
//    IME_SYSINFO_WINLOGON
//      IME �ɃN���C�A���g�v���Z�X�� Winlogon �v���Z�X�ł��邱�Ƃ� IME
//      �ɒm�点��BIME �͂��̃t���O���w�肳�ꂽ�� IME �̐ݒ�����[�U��
//      �����ׂ��ł͂Ȃ��B
//    IME_SYSINFO_WOW16
//      IME �ɃN���C�A���g�v���Z�X�� 16-bit �A�v���P�[�V�����ł��邱��
//      ��m�点��B
//  Return Values
//    ��������΁ATRUE�B�����łȂ���΁AFALSE�B
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
  //    fdwSCSCaps �r�b�g�� SetCompositionString �̔\�͂��w�肷��B
  //    ----------------------------------------------------------
  //    SCS_CAP_COMPSTR
  //      IME �� SCS_SETSTR �� composition string �𐶐��ł���B
  //    SCS_CAP_MAKEREAD
  //      ImmSetCompositionString �� SCS_SETSTR �ŌĂяo��������
  //      IME �� compositoin string �� reading string �� lpRead 
  //      �Ȃ��ɐ������邱�Ƃ��ł���B���̔\�͂̂��� IME �̉���
  //      �̓A�v���P�[�V������ SCS_SETSTR �� lpRead ��ݒ肷��
  //      �K�v���Ȃ��B
  //    SCS_CAP_CONVERTSTRING
  //      IME �� reconvert �@�\�������Ă���B
  lpIMEInfo->fdwSCSCaps =
      SCS_CAP_COMPSTR | SCS_CAP_MAKEREAD | SCS_CAP_SETRECONVERTSTRING;

  lpIMEInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;

  lstrcpy(lpszClassName, szUIClassName);

  return TRUE;
}

//  ImeConversionList
//  ImeConversionList �֐��ɂ���ĕʂ̕����������͕�����̕ϊ����ʂ̃��X
//  �g�𓾂邱�Ƃ��ł���B
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
//    ���̓R���e�L�X�g�̃n���h���B
//  lpSrc
//    �ϊ�����镶����
//  lpDst
//    �ϊ����ʂ��i�[�����o�b�t�@���w���|�C���^�B
//  dwBufLen
//    �ϊ����ʂ��i�[����o�b�t�@�̑傫���B
//  uFlag
//    ���݂͎���3�̃t���O��1��ݒ肷�邱�Ƃ��ł���B
//    �EGCL_CONVERSION
//      �ǂݕ������ lpSrc �p�����[�^�ɐݒ肷��BIME �� lpDst �p����
//      �[�^�ɂ��̕ϊ����ʂ�Ԃ��B
//    �EGCL_REVERSECONVERSION
//      �ϊ����ʕ������ lpSrc �p�����[�^�ɐݒ肷��BIME �͂��̓ǂ݂�
//      lpDst�p�����[�^�ɕԂ��B
//    �EGCL_REVERSE_LENGTH
//      lpSrc �p�����[�^�ɕϊ����ʕ������ݒ肷��BIME ��
//      GCL_REVERSECONVERSION �̈����钷����Ԃ��B�Ⴆ�΁AIME ��
//      sentence period ���������ϊ����ʂ�ǂ݂ɕϊ����邱�Ƃ��ł���
//      ���B���ʂƂ��āAsentence period ���������o�C�g�P�ʂŕ�����
//      ��Ԃ��B
//  Return Values
//    �ϊ����ʕ����񃊃X�g�̃o�C�g����Ԃ��B
//  Comments
//    ���̊֐��̓A�v���P�[�V������������ IME-related ���b�Z�[�W�����
//    �Ȃ� IME �ɂ���ČĂяo����邱�Ƃ��Ӑ}���Ă���B����䂦�ɁAIME
//    �͂��̊֐� IME-related ���b�Z�[�W�𐶐����ׂ��łȂ��B
DWORD WINAPI ImeConversionList(HIMC hIMC, LPCTSTR lpSource,
                               LPCANDIDATELIST lpCandList, DWORD dwBufLen,
                               UINT uFlags) {
  ImeLog(LOGF_API, TEXT("ImeConversionList"));

  return 0;
}

//  ImeDestroy()
//  ImeDestroy �֐��� IME ���g���I��������B
//  BOOL
//    ImeDestroy(
//      UINT uReserved
//    )
//  Parameters
//    uReserved
//      �\�񂳂�Ă��邯�ǁA���݂� 0 �ł��ꂼ�����B���̃o�[�W������
//      �́A0�łȂ������� FALSE ��Ԃ����ƁB
//  Return Values
//    �֐�������������ATRUE�B��������Ȃ�������AFALSE�B
BOOL WINAPI ImeDestroy(UINT uForce) {
  ImeLog(LOGF_ENTRY | LOGF_API, TEXT("ImeDestroy"));

  return TRUE;
}

//  ImeEscape
//  ImeEscape �֐��̓A�v���P�[�V�����ɁA���� IMM �֐��ł͒��ڂɗ��p�ł�
//  �Ȃ��悤�� IME �̓����ɃA�N�Z�X���邱�Ƃ������B
//  ����� IME �� country-specific �֐��������� private �֐��ɑ΂��ĂȂ�
//  �Ă͂Ȃ�Ȃ����̂ł���c�炵���B
//  LRESULT
//    ImeEscape(
//      HIMC hIMC,
//      UINT uEscape,
//      LPVOID lpData
//    )
//  Parameters
//    hIMC
//      ���̓R���e�L�X�g�̃n���h���B
//    uEscape
//      ���s�����G�X�P�[�v�֐����w�肷��B
//    lpData
//      �w�肳�ꂽ�G�X�P�[�v�ɕK�v�ȃf�[�^���w���|�C���^�B
//  ImeEscape �֐��͎��̂悤�ȃG�X�P�[�v�֐����T�|�[�g���Ă���B
//  IME_ESC_QUERY _SUPPORT
//    ��������Ă��邩�ǂ����`�F�b�N����B�����A���̃G�X�P�[�v��������
//    ��Ă��Ȃ���������Ԃ��B
//  IME_ESC_RESERVED_FIRST
//    IME_ESC_RESERVED_FIRST �� IME_ESC_RESERVED_LAST �̊Ԃ̃G�X�P�[�v
//    ���V�X�e���ɗ\�񂳂��B
//  IME_ESC_RESERVED_LAST
//    IME_ESC_RESERVED_FIRST �� IME_ESC_RESERVED_LAST �̊Ԃ̃G�X�P�[�v
//    ���V�X�e���ɗ\�񂳂��B
//  IME_ESC_PRIVATE_FIRST
//    IME_ESC_PRIVATE_FIRST �� IME_ESC_PRIVATE_LAST �̊Ԃ̃G�X�P�[�v��
//    IME �ɗ\�񂳂��BIME �͎��R�Ɏ����̖ړI�ɉ����Ă����̃G�X�P�[
//    �v�֐����g�����Ƃ��ł���B
//  IME_ESC_PRIVATE_LAST
//    IME_ESC_PRIVATE_FIRST �� IME_ESC_PRIVATE_LAST �̊Ԃ̃G�X�P�[�v��
//    IME �ɗ\�񂳂��BIME �͎��R�Ɏ����̖ړI�ɉ����Ă����̃G�X�P�[
//    �v�֐����g�����Ƃ��ł���B
//  IME_ESC_SEQUENCE_TO_INTERNAL
//    ������ŗL�̃G�X�P�[�v�BFar East �v���b�g�t�H�[���̉��œ�������
//    ��v������A�v���P�[�V�����͂�����g���ׂ��ł͂Ȃ��B������EUDC
//    �G�f�B�^�̂��߂̂��̂ł���B*(LPWORD)lpData �� sequence code 
//    �ł���A�Ԃ�l�͂��� sequence code �ɑ΂��镶���R�[�h�ł���B
//  IME_ESC_GETHELPFILENAME
//    IME �̃w���v�t�@�C���̖��O�𓾂�G�X�P�[�v�B�֐�����߂�������
//    (LPTSTR)lpData �� IME �w���v�t�@�C���̃t���p�X�ɂȂ��Ă���B
//    �p�X���� 80 * sizeof (TCHAR) ���Z�����B���̊֐��� Windows'98
//    Windows 2000 �ɒǉ�����Ă���B
//  IME_ESC_PRIVATE_HOTKEY
//    lpData �� hot key ID ���i�[���Ă��� DWORD �ւ̃|�C���^�ł���B
//    (IME_HOTKEY_PRIVATE_FIRST ����IME_HOTKEY_PRIVATE_LAST �͈̔�)
//    �V�X�e�������͈͓̔��� hot key ���󂯂Ƃ�����AIMM ��
//    ImeEscape �֐����g���� IME �Ƀf�B�X�p�b�`����BWindowsR95 ��
//    �̓T�|�[�g���Ă��Ȃ��B
//  Return Values
//    ���s������ 0 �ɂȂ�B�����Ȃ��΃G�X�P�[�v�֐��ɏ]���ĕԂ�l��
//    ���܂�B
//  Comments
//    �p�����[�^���������ǂ����͊e�X�̃G�X�P�[�v�֐��ɂ��B
//    uEscape �� IME_ESC_QUERY_SUPPORT �̎��AlpData �� IME �G�X�P�[
//    �v�l���܂񂾕ϐ��ւ̃|�C���^�ł���B���̃T���v���́AIME �� 
//    IME_ESC_GETHELPFILENAME ���T�|�[�g���Ă��邩�ǂ��������肷��
//    �̂ɗ��p�ł���B
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
//  ImeSetActiveContext �֐��� current IME �� Input Context �� active ��
//  �Ȃ������Ƃ�m�点��B
//  BOOL
//    ImeSetActiveContext(
//    HIMC hIMC,
//    BOOL fFlag
//  )
//  Parameters
//    hIMC
//      Input Context Handle
//    fFlag
//      TRUE �Ȃ�A�N�e�B�u�� FALSE �Ȃ��A�N�e�B�u�ɂȂ������Ƃ���
//      ���B
//    Return Values
//      ���������� TRUE ���A�����Ȃ��� FALSE ��Ԃ��B
//  Comments
//    IME �͐V�����I�����ꂽ Input Context �ɂ��Ă��̊֐��Œm�炳��
//    ��BIME �͏����������s���Ă悢���A�v������Ă���킯�ł͂Ȃ��B
//  See Also
//    ImeSetActiveContext
BOOL WINAPI ImeSetActiveContext(HIMC hIMC, BOOL fFlag) {
  ImeLog(LOGF_API, TEXT("ImeSetActiveContext"));

  UpdateIndicIcon(hIMC);

  return TRUE;
}

//  ImeProcessKey ()
//  ImeProcessKey �֐��� IMM ��ʂ��ė^����ꂽ�S�ẴL�[�X�g���[�N��O��
//  �����āA�������̃L�[���^����ꂽ Input Context �� IME �ɕK�v�Ȃ��̂�
//  ����� TRUE ��Ԃ��B
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
//      ���������ׂ����z�L�[�B
//    lParam
//      �L�[���b�Z�[�W�� lParam�B(WM_KEYDOWN,WM_KEYUP �� LPARAM)
//    lpbKeyState
//      ���݂̃L�[�{�[�h�̏�Ԃ��܂�256�o�C�g�̔z����w���|�C���^�B
//      IME �͂��̓��e��ύX���ׂ��ł͂Ȃ��B
//  Return Values
//    �����Ȃ� TRUE�B�����łȂ���� FALSE�B
//  Comments
//    �V�X�e���̓L�[�� IME �ɂ���Ď�舵����ׂ����ۂ������̊֐���
//    �Ăяo�����Ƃɂ���Č��肵�Ă���B�A�v���P�[�V�������L�[���b�Z�[
//    �W���󂯎��O�ɂ��̊֐��� TRUE ��Ԃ��΁AIME �͂��̃L�[��������
//    ��B�V�X�e���� ImeToAsciiEx �֐����Ăяo���B
//    FALSE ��Ԃ����Ȃ�΁A�V�X�e���͂��̃L�[�� IME �ɂ���ď�������
//    �Ȃ����Ƃ�������̂ŃL�[���b�Z�[�W�̓A�v���P�[�V�����ɑ�����B
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
//  NotifyIME �֐��͗^����ꂽ�p�����[�^�ɏ]���� IME �̏�Ԃ�ύX����B
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
//      dwAction �p�����[�^�ɃA�v���P�[�V�������w��ł��� context
//      item �͎��̒ʂ�ł���B
//  --------------------------------------------------------------------
//  NI_OPENCANDIDATE
//    �A�v���P�[�V������ IME �Ɍ�⃊�X�g���J������B���� IME ����⃊
//    �X�g���J���΁AIME �� WM_IME_NOTIFY (�T�u�֐��� IMN_OPENCANDIDATE)
//    ���b�Z�[�W�𑗐M����B
//    dwIndex   dwIndex �͊J���ꂽ��⃊�X�g�� index�B
//    dwValue   �g���Ȃ��B
//  NI_CLOSECANDIDATE
//    �A�v���P�[�V������ IME �Ɍ�⃊�X�g���������B���� IME �����
//    ���X�g���������AIME �� WM_IME_NOTIFY (�T�u�֐��� 
//    IMN_CLOSE_CANDIDATE)���b�Z�[�W�𑗐M����B
//    dwIndex   �������⃊�X�g�� index
//    dwValue   �g���Ȃ��B
//  NI_SELECTCANDIDATESTR
//    �A�v���P�[�V�����͌���1��I������B
//    dwIndex   �I������Ă����⃊�X�g�� index�B
//    dwValue   �I������Ă����⃊�X�g�̌�╶����� index�B
//  NI_CHANGECANDIDATELIST
//    �A�v���P�[�V�����͌��ݑI������Ă������ύX����B
//    dwIndex   �I������Ă����⃊�X�g�� index�B
//    dwValue   �g���Ȃ��B
//  NI_SETCANDIDATE_PAGESTART
//    �A�v���P�[�V�����͌�⃊�X�g�� index ���͂��܂��Ă���y�[�W��
//    �ύX����B
//    dwIndex   �ύX������⃊�X�g�� index�B
//    dwValue   �V�����y�[�W�J�n index�B
//  NI_SETCANDIDATE_PAGESIZE
//    �A�v���P�[�V�����͌�⃊�X�g�̃y�[�W�T�C�Y��ύX����B
//    dwIndex   �ύX������⃊�X�g�� index�B
//    dwValue   �V�����y�[�W�T�C�Y�B
//  NI_CONTEXTUPDATED
//    �A�v���P�[�V�����������̓V�X�e���� Input Context ���X�V����B
//    dwIndex   dwValue �̒l�� IMC_SETCONVERSIONMODE �Ȃ� dwIndex ��
//          �O��� conversion mode �ł���B
//          dwValue �̒l�� IMC_SETSENTENCEMODE �Ȃ� dwIndex ��
//          �O��� sentence mode �ł���B
//          ���̑��� dwValue �ɂ��Ă� dwIndex �͎g���Ȃ��B
//    dwValue   WM_IME_CONTROL ���b�Z�[�W�ɂ���Ďg���鎟�̒l��1��:
//          IMC_SETCANDIDATEPOS
//          IMC_SETCOMPOSITIONFONT
//          IMC_SETCOMPOSITIONWINDOW
//          IMC_SETCONVERSIONMODE
//          IMC_SETSENTENCEMODE
//          IMC_SETOPENSTATUS
//  NI_COMPOSITIONSTR
//    �A�v���P�[�V������ composition string ��ύX����B���̓����
//    composition string �� input context �ɑ��݂��鎞�̂݉e������B
//    dwIndex   dwIndex �ɂ͎��̂悤�Ȓl���^������B
//          CPS_COMPLETE
//            composition string �� result string �ł����
//            ���肷��B
//          CPS_CONVERT
//            composition string ��ϊ�����B
//          CPS_REVERT
//            composition string �� revert ����B���݂�
//            composition string ���L�����Z������A���ϊ���
//            ������ composition string �Ƃ��Đݒ肳���B
//          CPS_CANCEL 
//            composition string ���N���A���āAcomposition
//            string ��������Ԃɂ���B
//    dwValue   �g���Ȃ��B
//  --------------------------------------------------------------------
//    dwIndex
//      uAction �Ɉˑ��B
//    dwValue
//      uAction �Ɉˑ��B
//  Return Values
//    ��������� TRUE�B�����łȂ���� FALSE�B
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
//  ImeSelect �֐��� IME private context �̏������y�є񏉊����������s��
//  �̂ɗp������B
//  BOOL
//    ImeSelect(
//      HIMC hIMC,
//      BOOL fSelect
//    )
//  Parameters
//    hIMC
//      Input context handle
//    fSelect
//      TRUE �Ȃ�Ώ��������AFALSE �Ȃ�񏉊���(���\�[�X�̉��)����
//      ������B
//  Return Values
//    ��������� TRUE�B�����Ȃ��� FALSE�B
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
//  ImeSetCompositionString �֐��̓A�v���P�[�V������ lpComp ��lpRead �p
//  �����[�^�ɓ����ꂽ�f�[�^��IME��composition string �\���̂ɐݒ肷��
//  �̂ɗp������BIME �� WM_IME_COMPOSITION ���b�Z�[�W�𐶐�����B
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
//      dwIndex �ɗ^������l�͎��̒ʂ�ł���B
//      ------------------------------------------------------------
//  SCS_SETSTR
//    �A�v���P�[�V������ composition string ���� reading string �܂�
//    �͂��̗������Z�b�g����B���Ȃ��Ƃ� lpComp �� lpRead �p�����[�^
//    ��1�͐����ȕ�������w���|�C���^�łȂ���΂Ȃ�Ȃ��B
//    ���������񂪒�������̂Ȃ�AIME �͂����؂�l�߁B
//  SCS_CHANGEATTR
//    �A�v���P�[�V������ composition string ���� reading string ����
//    ���̗��҂̐ߏ���ݒ肷��B���Ȃ��Ƃ� lpComp �� lpRead ��1��
//    �� valid �Ȑߏ��̔z����w���|�C���^�łȂ���΂Ȃ�Ȃ��B
//  SCS_QUERYRECONVERTSTRING
//    �A�v���P�[�V������ IME �ɂ��� RECONVERTSTRINGSTRUCTURE �𒲐�
//    ���Ă����悤�˗�����B�����A�v���P�[�V���������̒l��ݒ肵��
//    ImeSetCompositionString ���Ăяo�����Ȃ�AIME �� RECONVERTSTRING
//    �\���̂𒲐����Ȃ���΂Ȃ�Ȃ��B�A�v���P�[�V�����͂��̎�������
//    �ꂽ RECONVERTSTRING �\���̂� SCS_RECONVERTSTRING �ł����āA
//    ���̊֐��ɓn�����Ƃ��ł���BIME �� WM_IMECOMPOSITION ���b�Z�[�W
//    �𐶐����Ă͂Ȃ�Ȃ��B
//  SCS_SETRECONVERTSTRING
//    �A�v���P�[�V������ IME �� RECONVERTSTRING �\���̂Ɋ܂܂��
//    ������� reconvert ����悤�Ɉ˗�����B
//      ------------------------------------------------------------
//    lpComp
//      �X�V���ꂽ��������܂ރo�b�t�@���w���|�C���^�B������̌^��
//      dwIndex �̒l�ɂ���Č��肳���B
//    dwCompLen
//      �o�b�t�@�̃o�C�g���B
//    lpRead
//      �X�V���ꂽ��������܂ރo�b�t�@���w���|�C���^�B������̌^��
//      dwIndex �̒l�ɂ���Č��肳���BdwIndex �̒l��
//      SCS_SETRECONVERTSTRING �������� SCS_QUERYRECONVERTSTRING ��
//      ��΁AlpRead �͍X�V���ꂽ reading string ���܂�
//      RECONVERTSTRING �\���̂��w���|�C���^�ł���B�����I�����ꂽ
//      IME �� SCS_CAP_MAKEREAD �������Ă���΁ANULL �ɂȂ�B
//    dwReadLen
//      �o�b�t�@�̃o�C�g���B
//  Comments
//    Unicode �ɂ��ẮA���Ƃ� SCS_SETSTR �����肳��Ă��� Unicode 
//    ��������܂�ł��Ă��AdwCompLen �� dwReadLen �̓o�b�t�@�̃o�C�g
//    ���ł���B
//    SCS_SETRECONVERTSTRING �܂��� SCS_QUERYRECONVERTSTRING ��
//    SCS_CAP_CONVERTSTRING �v���p�e�B�������� IME �ɂ̂ݗ��p�ł���B
//    ���̃v���p�e�B�� ImmGetProperty �֐����g�����Ƃœ��邱�Ƃ��ł���B
BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPVOID lpComp,
                                    DWORD dwComp, LPVOID lpRead, DWORD dwRead) {
  ImeLog(LOGF_API, TEXT("ImeSetCompositionString"));

  // �T�C�Y�����܂����̏ꍇ�ɂ́A��芸������ dwStrLen ��
  // ���킹��Bkakasi �Ƃ��̈ʒu�ŘA�g�������������̂�����
  // ��Ȃ��B
  // �T�C�Y���傫�߂���Ȃ璲�����Ȃ���΁B

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
//  ImeGetImeMenuItems �֐��� IME menu �ɓo�^����Ă��� menu item �𓾂�B
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
//      lpMenuItem �͂��� input context �Ɋ֘A�t�����Ă��� menu
//      item ���܂ށB
//    dwFlags
//      �����r�b�g�̑g�ݍ��킹����Ȃ�B
//      ------------------------------------------------------------
//      IGIMIF_RIGHTMENU  
//        ���� 1 �Ȃ�A���̊֐��͉E�N���b�N Context menu �ɑ΂���
//        menu item ��Ԃ��B
//      ------------------------------------------------------------
//    dwType
//      �����r�b�g�̑g�ݍ��킹����Ȃ�B
//      ------------------------------------------------------------
//      IGIMII_CMODE
//        conversion mode �Ɋ֘A�t�����Ă��� menu item ��Ԃ��B
//      IGIMII_SMODE
//        sentence mode �Ɋ֘A�t�����Ă��� menu item ��Ԃ��B
//      IGIMII_CONFIGURE
//        IME �̐ݒ�Ɋ֘A�t�����Ă��� menu item ��Ԃ��B
//      IGIMII_TOOLS
//        IME �c�[���Ɋ֘A�t�����Ă��� menu item ��Ԃ��B
//      IGIMII_HELP
//        IME �w���v�Ɋ֘A�t�����Ă��� menu item ��Ԃ��B
//      IGIMII_OTHER
//        ���̑��� menu item ��Ԃ��B
//      IGIMII_INPUTTOOLS
//        �g�����ꂽ�����̓��͕��@��񋟂��� IME ���̓c�[���Ɋ֘A
//        �t����ꂽ menu item ��Ԃ��B
//      ------------------------------------------------------------
//    lpImeParentMenu
//      fType �� MFT_SUBMENU �������� IMEMENUINFO �\���̂��w���|�C��
//      �^�BImeGetMenuItems �͂��� menu item �� submenu item ��Ԃ��B
//      ���� NULL �Ȃ� lpImeMenu �� top-level �� IME menu item ����
//      ��ł���B
//    lpImeMenu
//      menu item �̓��e���󂯎��o�b�t�@���w�����|�C���^�B���̃o�b
//      �t�@�� IMEMENUINFO �\���̂̔z��ł���B���� NULL �Ȃ�
//      ImeGetImeMenuItems �͓o�^����� menu item �̐���Ԃ��B
//    dwSize
//      IMEMENUITEMINFO �\���̂��󂯂�o�b�t�@�̃T�C�Y�B
//  Return Values
//    lpIM �ɐݒ肳�ꂽ menu item �̐����A��B���� lpImeMenu �� NULL 
//    �Ȃ�� ImeMenuItems �͎w�肳�ꂽ hKL �ɓo�^����Ă��� menu item 
//    �̐���Ԃ��B
//  ImeGetImeMenuItems �� Windows'98 �� Windows 2000 �p�̐V�����֐��ł���B
DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType,
                                LPMYIMEMENUITEMINFO lpImeParentMenu,
                                LPMYIMEMENUITEMINFO lpImeMenu, DWORD dwSize) {
  // dwType �� MSIME �̓`�F�b�N���Ă��Ȃ��悤���B����ɍ��킹��B
  // �����ATSF �������Ă���ƁA���� method �͏�ɓ��삵�ĂȂ��悤�Ɏv���邪�B
  //
  // if (IsTSFEnabled ())
  //   return  0 ;
  //
  // ���̃R�[�h��L���ɂ��邩�ǂ����͔������B
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
