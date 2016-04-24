// imm.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

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
  FOOTMARK();

  lpIMEInfo->dwPrivateDataSize = sizeof(UIEXTRA);
  lpIMEInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST |
                           IME_PROP_UNICODE | IME_PROP_AT_CARET;
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

  if (dwSystemInfoFlags & IME_SYSINFO_WINLOGON) {
    TheIME.m_bWinLogOn = TRUE;
  }

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
  FOOTMARK();

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
  FOOTMARK();

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
  FOOTMARK();

  TheIME.UpdateIndicIcon(hIMC);

  return TRUE;
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
  FOOTMARK();

  // �T�C�Y�����܂����̏ꍇ�ɂ́A��芸������ dwStrLen ��
  // ���킹��Bkakasi �Ƃ��̈ʒu�ŘA�g�������������̂�����
  // ��Ȃ��B
  // �T�C�Y���傫�߂���Ȃ璲�����Ȃ���΁B

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
  // dwType �� MSIME �̓`�F�b�N���Ă��Ȃ��悤���B����ɍ��킹��B
  // �����ATSF �������Ă���ƁA���� method �͏�ɓ��삵�ĂȂ��悤�Ɏv���邪�B
  //
  // if (IsTSFEnabled ())
  //   return  0 ;
  //
  // ���̃R�[�h��L���ɂ��邩�ǂ����͔������B

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
