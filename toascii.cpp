// toascii.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

//  ImeToAsciiEx 
//  ImeToAsciiEx �֐��� hIMC �p�����[�^�ɏ]���� IME �ϊ��G���W����ʂ��ĕ�
//  �����ʂ𐶐����܂��B
//  UINT
//    ImeToAsciiEx(
//    UINT uVirKey,
//    UINT uScanCode,
//    CONST LPBYTE lpbKeyState,
//    LPDWORD lpdwTransBuf,
//    UINT fuState,
//    HIMC hIMC
//  )
//  (�p�����[�^)
//  uVirKey
//    �ϊ�����鉼�z�L�[�R�[�h���w�肵�܂��B�v���p�e�B�� 
//    IME_PROP_KBD_CHAR_FIRST �r�b�g���I����������A���z�L�[�̏�ʃo�C�g
//    �͕⏕�L�����N�^�R�[�h(?)�ɂȂ�܂��B
//    Unicode �ɂ��ẮAIME_PROP_KBD_CHAR_FIRST �r�b�g���I���������� 
//    uVirKey �̏�ʃ��[�h�� Unicode �������܂݂܂��B
//  uScanCode
//    �ϊ������L�[�̃n�[�h�E�F�A�X�L�����R�[�h���w�肵�܂��B
//  lpbKeyState
//    ���݂̃L�[�{�[�h�̏�Ԃ��܂�256�o�C�g�̔z��ւ̃|�C���^�ł��B
//    IME �͂��̓��e��ύX���Ă͂����܂���B
//  lpdwTransBuf
//    �ϊ����ʂ��󂯎�� DWORD �̃o�b�t�@���w���|�C���^�ł��B���̏����́A
//    [���b�Z�[�W�o�b�t�@�̒���][���b�Z�[�W1][wParam1][lParam1]
//    [���b�Z�[�W2][wParam2][lParam2][...[...[...]]] �̂悤�ɂȂ�܂��B
//  fuState
//    Active menu flag. �H
//  hIMC
//    Input context handle.
//  (�Ԃ�l)
//    �Ԃ�l�̓��b�Z�[�W�̐���\���܂��B�������̐����o�b�t�@�T�C�Y���
//    �傫����΃��b�Z�[�W�o�b�t�@�͏\���ł͂Ȃ��ł��c���ē��R�ł́c(^^;;
//    �V�X�e���� hMsgBuf ���`�F�b�N���ă��b�Z�[�W�𓾂܂��B
//  (See Also)
//    ImmToAsciiEx
UINT WINAPI ImeToAsciiEx(UINT uVKey, UINT uScanCode, CONST LPBYTE lpbKeyState,
                         LPTRANSMSGLIST lpTransBuf, UINT fuState, HIMC hIMC) {
  DebugPrint(TEXT("ImeToAsciiEx"));

  TheApp.m_lpCurTransKey = lpTransBuf;
  LPARAM lParam = ((DWORD)uScanCode << 16) + 1L;

  // Init TheApp.m_uNumTransKey here.
  TheApp.m_uNumTransKey = 0;

  // if hIMC is NULL, this means DISABLE IME.
  if (!hIMC) return 0;

  InputContext *lpIMC = (InputContext *)ImmLockIMC(hIMC);
  if (NULL == lpIMC) return 0;
  BOOL fOpen = lpIMC->fOpen;
  ImmUnlockIMC(hIMC);

  if (fOpen) {
    if (uScanCode & 0x8000)
      IMEKeyupHandler(hIMC, uVKey, lParam, lpbKeyState);
    else
      IMEKeydownHandler(hIMC, uVKey, lParam, lpbKeyState);

    // Clear static value, no more generated message!
    TheApp.m_lpCurTransKey = NULL;
  }

  // If trans key buffer that is allocated by USER.EXE full up,
  // the return value is the negative number.
  if (TheApp.m_fOverTransKey) {
    DebugPrint(TEXT("***************************************"));
    DebugPrint(TEXT("*   TransKey OVER FLOW Messages!!!    *"));
    DebugPrint(TEXT("*                by MZIMEJA.DLL       *"));
    DebugPrint(TEXT("***************************************"));
    return (int)TheApp.m_uNumTransKey;
  }

  return (int)TheApp.m_uNumTransKey;
}

// Update the transrate key buffer
BOOL PASCAL GenerateMessageToTransKey(LPTRANSMSGLIST lpTransBuf,
                                      LPTRANSMSG lpGeneMsg) {
  LPTRANSMSG lpgmT0;

  ++TheApp.m_uNumTransKey;
  if (TheApp.m_uNumTransKey >= lpTransBuf->uMsgCount) {
    TheApp.m_fOverTransKey = TRUE;
    return FALSE;
  }

  lpgmT0 = lpTransBuf->TransMsg + (TheApp.m_uNumTransKey - 1);
  *lpgmT0 = *lpGeneMsg;

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
