// process.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "vksub.h"

#define IsCtrlPressed(x)  ((x)[VK_CONTROL] & 0x80)
#define IsShiftPressed(x) ((x)[VK_SHIFT] & 0x80)
#define IsAltPressed(x)   ((x)[VK_ALT] & 0x80)

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// A function which handles WM_IME_KEYDOWN
BOOL IMEKeyDownHandler(HIMC hIMC, WPARAM wParam, LPARAM lParam,
                       LPBYTE lpbKeyState) {
  InputContext *lpIMC;
  WORD vk = (wParam & 0x00FF);
  switch (vk) {
  case VK_SHIFT:
  case VK_CONTROL:
    break;

  case VK_ESCAPE:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->CancelText();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_DELETE:
  case VK_BACK:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->DeleteChar(vk == VK_BACK);
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_SPACE:
  case VK_CONVERT:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->DoConvert();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_F6:
    break;

  case VK_F7:
    break;

  case VK_F8:
    break;

  case VK_F9:
    break;

  case VK_RETURN:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      if (lpIMC->Conversion() & IME_CMODE_CHARCODE) {
        // code input
        lpIMC->CancelText();
      } else {
        lpIMC->MakeResult();
      }
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_LEFT:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MoveLeft();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  case VK_RIGHT:
    lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      lpIMC->MoveRight();
      TheIME.UnlockIMC(hIMC);
    }
    break;

  default:
    if ((VK_0 <= vk && vk <= VK_9) ||
        (VK_A <= vk && vk <= VK_Z) ||
        (VK_NUMPAD0 <= vk && vk <= VK_NUMPAD9) ||
        (VK_OEM_1 <= vk && vk <= VK_OEM_9) ||
        (VK_MULTIPLY <= vk && vk <= VK_DIVIDE) ||
        (vk == VK_PACKET))
    {
      lpIMC = TheIME.LockIMC(hIMC);
      if (lpIMC) {
        lpIMC->AddChar(HIWORD(wParam));
        TheIME.UnlockIMC(hIMC);
      }
    }
    break;
  }
  return TRUE;
} // IMEKeyDownHandler

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
  BOOL ret = FALSE;
  FOOTMARK();

  if (lKeyData & 0x80000000) {
    return FALSE;
  }

  InputContext *lpIMC = TheIME.LockIMC(hIMC);
  if (lpIMC == NULL) {
    return FALSE;
  }

  BOOL fOpen = lpIMC->IsOpen();
  BOOL fAlt = (lpbKeyState[VK_MENU] & 0x80);
  BOOL fCtrl = (lpbKeyState[VK_CONTROL] & 0x80);
  BOOL fShift = (lpbKeyState[VK_SHIFT] & 0x80);

  switch (vKey) {
  case VK_KANJI:
  case VK_OEM_AUTO:
  case VK_OEM_ENLW:
    if (!fShift && !fCtrl) {
      // switch zenkaku/hankaku input mode
      ret = TRUE;
    }
    break;
  }

  if (fOpen) {
    BOOL fCompStr = lpIMC->HasCompStr();
    BOOL fCandInfo = lpIMC->HasCandInfo();
    if (fAlt) {
      // Alt key is down
    } else if (fCtrl) {
      // Ctrl key is down
      if (fCompStr) {
        switch (vKey) {
        case VK_UP: case VK_DOWN:
        case VK_LEFT: case VK_RIGHT:
          // widely move
          ret = TRUE;
          break;
        }
      }
    } else if (fShift) {
      // Shift key is down
      switch (vKey) {
      case VK_LEFT: case VK_RIGHT:
        // fix clauses
        ret = TRUE;
        break;
      }
    } else {
      // Neither Ctrl nor Shift key is down
      ret = FALSE;
      if (fCompStr) {
        switch (vKey) {
        case VK_F6:     // make composition zenkaku Hiragana
        case VK_F7:     // make composition zenkaku Katakana
        case VK_F8:     // make composition hankaku Katakana
        case VK_F9:     // make composition zenkaku alphanumeric
        case VK_F10:    // make composition hankaku alphanumeric
        case VK_ESCAPE: // close composition
          ret = TRUE;
          break;
        }
      }
      switch (vKey) {
      case VK_OEM_PLUS: case VK_OEM_MINUS: case VK_OEM_PERIOD:
      case VK_OEM_COMMA:
      case VK_OEM_1: case VK_OEM_2: case VK_OEM_3: case VK_OEM_4:
      case VK_OEM_5: case VK_OEM_6: case VK_OEM_7: case VK_OEM_8:
      case VK_OEM_9: case VK_OEM_102: case VK_OEM_COPY:
        // OEM keys
        ret = TRUE;
        break;
      case VK_ADD: case VK_SUBTRACT:
      case VK_MULTIPLY: case VK_DIVIDE:
      case VK_SEPARATOR: case VK_DECIMAL:
        // numpad keys
        ret = TRUE;
        break;
      case VK_HOME: case VK_END:
      case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
        // arrow and moving key
        ret = TRUE;
        break;
      case VK_SPACE: case VK_BACK: case VK_DELETE: case VK_RETURN:
      case VK_CAPITAL: case VK_CONVERT: case VK_NONCONVERT:
        // special keys
        ret = TRUE;
        break;
      default:
        if ('0' <= vKey && vKey <= '9') {
          // numbers
          ret = TRUE;
        } else if ('A' <= vKey && vKey <= 'Z') {
          // alphabets
          ret = TRUE;
        } else if (VK_NUMPAD0 <= vKey && vKey <= VK_NUMPAD9) {
          // numpad numbers
          ret = TRUE;
        } else {
          if (fCandInfo) {
            switch (vKey) {
            case VK_PRIOR: case VK_NEXT:
              // next or previous page of candidates
              ret = TRUE;
              break;
            }
          }
        }
      }
    }
  }

  TheIME.UnlockIMC(hIMC);
  return ret;
} // ImeProcessKey

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
  UINT ret = 0;
  FOOTMARK();

  TheIME.m_lpCurTransKey = lpTransBuf;
  TheIME.m_uNumTransKey = 0;

  // if hIMC is NULL, this means DISABLE IME.
  if (hIMC) {
    InputContext *lpIMC = TheIME.LockIMC(hIMC);
    if (lpIMC) {
      BOOL fOpen = lpIMC->IsOpen();
      TheIME.UnlockIMC(hIMC);

      if (fOpen) {
        if ((uScanCode & 0x8000) == 0) {
          LPARAM lParam = ((DWORD)uScanCode << 16) + 1L;
          IMEKeyDownHandler(hIMC, uVKey, lParam, lpbKeyState);
        }

        // Clear static value, no more generated message!
        TheIME.m_lpCurTransKey = NULL;
      }

      // If trans key buffer that is allocated by USER.EXE full up,
      // the return value is the negative number.
      if (TheIME.m_fOverTransKey) {
        DebugPrint(TEXT("***************************************\n"));
        DebugPrint(TEXT("*   TransKey OVER FLOW Messages!!!    *\n"));
        DebugPrint(TEXT("*                by MZIMEJA.IME       *\n"));
        DebugPrint(TEXT("***************************************\n"));
      }
      ret = TheIME.m_uNumTransKey;
    }
  }
  return ret;
} // ImeToAsciiEx

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
