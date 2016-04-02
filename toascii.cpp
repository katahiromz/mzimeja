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
