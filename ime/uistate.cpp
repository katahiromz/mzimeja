// uistate.cpp --- mzimeja status window UI
// IME��ԃE�B���h�E�B
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

// ���@�i�s�N�Z���P�ʁj�B
#define CX_MINICAPTION 10
#define CX_BUTTON 24
#define CY_BUTTON 24
#define CX_BTNEDGE 2
#define CY_BTNEDGE 2

// �����蔻��̌��ʁB
enum STATUS_WND_HITTEST {
    SWHT_NONE,      // �Ȃɂ��������Ă��Ȃ��B
    SWHT_CAPTION,   // �L���v�V�����B
    SWHT_BUTTON_1,  // �{�^��1�B
    SWHT_BUTTON_2,  // �{�^��2�B
    SWHT_BUTTON_3   // �{�^��3�B
};

//////////////////////////////////////////////////////////////////////////////

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// Create status window.
// IME��ԃE�B���h�E���쐬����B
HWND StatusWnd_Create(HWND hWnd, UIEXTRA *lpUIExtra) {
    const DWORD style = WS_DISABLED | WS_POPUP; // �E�B���h�E�X�^�C���B
    const DWORD exstyle = WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME; // �g���X�^�C���B
    HWND hwndStatus = lpUIExtra->hwndStatus;
    if (!::IsWindow(hwndStatus)) { // ��ԃE�B���h�E���Ȃ����H
        // ��ԃE�B���h�E�̃T�C�Y���v�Z����B
        INT cx, cy;
        cx = CX_MINICAPTION + CX_BUTTON * 3;
        cx += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
        cx += 3 * CX_BTNEDGE * 2;
        cy = CY_BUTTON;
        cy += ::GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
        cy += 2 * CY_BTNEDGE;
        POINT pt;
        if (!TheIME.GetUserData(L"ptStatusWindow", &pt, sizeof(pt))) { // �ʒu��񂪂��邩�H
            // ���[�N�G���A���g���Ĉʒu������������B
            RECT rcWorkArea;
            ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
            pt.x = rcWorkArea.right - cx;
            pt.y = rcWorkArea.bottom - cy;
        }
        // ���ۂɏ�ԃE�B���h�E���쐬����B
        lpUIExtra->hwndStatus = hwndStatus = ::CreateWindowEx(
                exstyle, szStatusClassName, NULL, style,
                pt.x, pt.y, cx, cy,
                hWnd, NULL, TheIME.m_hInst, NULL);
    } else {
        StatusWnd_Update(lpUIExtra); // ��ԃE�B���h�E���X�V����B
    }
    RepositionWindow(hwndStatus); // �ʒu��␳����B
    ::ShowWindow(hwndStatus, SW_SHOWNOACTIVATE); // �A�N�e�B�u�����邱�ƂȂ��\������B
    ::SetWindowLongPtr(hwndStatus, FIGWLP_SERVERWND, (LONG_PTR)hWnd); // UI�T�[�o�[���Z�b�g����B
    return hwndStatus;
} // StatusWnd_Create

// Draw status window.
// IME��ԃE�B���h�E��`�悷��B
void StatusWnd_Paint(HWND hWnd, HDC hDC, INT nPushed) {
    // UI�T�[�o�[��IMC���擾����B
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC);
    InputContext *lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N�B

    // �N���C�A���g�̈��h��Ԃ��B
    HBRUSH hbr3DFace = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    RECT rc;
    ::GetClientRect(hWnd, &rc);
    ::FillRect(hDC, &rc, hbr3DFace);
    DeleteObject(hbr3DFace);

    // �L���v�V�����i�^�C�g���o�[�j��h�蕨�|�B
    HBRUSH hbrCaption = ::CreateSolidBrush(RGB(0, 32, 255));
    rc.right = rc.left + CX_MINICAPTION;
    ::FillRect(hDC, &rc, hbrCaption);
    ::DeleteObject(hbrCaption);

    // �N���C�A���g�̈���Ď擾����B�L���v�V�����̈�����O����B
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION;

    // �r�b�g�}�b�v���擾����B
    HBITMAP hbmStatus = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);

    // �������[DC���쐬����B
    HDC hMemDC = ::CreateCompatibleDC(hDC);
    ASSERT(hMemDC != NULL);

    HGDIOBJ hbmOld = ::SelectObject(hMemDC, hbmStatus); // �r�b�g�}�b�v��I������B

    // �ŏ��̃{�^��1�̔w�i�B
    RECT rcButton;
    rcButton.left = rc.left;
    rcButton.top = rc.top;
    rcButton.right = rc.left + CX_BUTTON + 4;
    rcButton.bottom = rc.bottom;
    if (nPushed == 1) {
        ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                           DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
        ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON, DFCS_BUTTONPUSH);
    }

    // �{�^��2�̔w�i�B
    rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
    rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
    if (nPushed == 2) {
        ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                           DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
        ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                           DFCS_BUTTONPUSH);
    }

    // �{�^��3�̔w�i�B
    rcButton.left += CX_BUTTON + 2 * CX_BTNEDGE;
    rcButton.right += CX_BUTTON + 2 * CY_BTNEDGE;
    if (nPushed == 3) {
        ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                           DFCS_BUTTONPUSH | DFCS_PUSHED);
    } else {
        ::DrawFrameControl(hDC, &rcButton, DFC_BUTTON,
                           DFCS_BUTTONPUSH);
    }

    // IME��On/Off���r�b�g�}�b�v�ŕ`�悷��B
    if (lpIMC) {
        if (lpIMC->IsOpen()) {
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 7 * CY_BUTTON, SRCCOPY);
        } else {
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 8 * CY_BUTTON, SRCCOPY);
        }
    } else {
        // disabled
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    // ���̓��[�h���r�b�g�}�b�v�ŕ`�悷��B
    rc.left += CX_BUTTON + CX_BTNEDGE * 2;
    if (lpIMC) { // ���̓R���e�L�X�g���L�����H
        if (lpIMC->IsOpen()) { // �J����Ă��邩�H
            if (lpIMC->Conversion() & IME_CMODE_FULLSHAPE) { // �S�p���H
                if (lpIMC->Conversion() & IME_CMODE_JAPANESE) { // ���{����͂��H
                    if (lpIMC->Conversion() & IME_CMODE_KATAKANA) { // �J�^�J�i���H
                        // fullwidth katakana
                        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                 CX_BUTTON, CY_BUTTON,
                                 hMemDC, 0, 1 * CY_BUTTON, SRCCOPY);
                    } else { // �Ђ炪�Ȃ��H
                        // fullwidth hiragana
                        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                                 CX_BUTTON, CY_BUTTON,
                                 hMemDC, 0, 0 * CY_BUTTON, SRCCOPY);
                    }
                } else { // �S�p�p�����͂��H
                    // fullwidth alphanumeric
                    ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                             CX_BUTTON, CY_BUTTON,
                             hMemDC, 0, 2 * CY_BUTTON, SRCCOPY);
                }
            } else { // ���p���͂��H
                if (lpIMC->Conversion() & IME_CMODE_JAPANESE) { // ���p�J�i���H
                    // halfwidth kana
                    ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                             CX_BUTTON, CY_BUTTON,
                             hMemDC, 0, 3 * CY_BUTTON, SRCCOPY);
                } else { // ���p�p�����H
                    // halfwidth alphanumeric
                    ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                             CX_BUTTON, CY_BUTTON,
                             hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
                }
            }
        } else { // �����Ă��邩�H
            // halfwidth alphanumeric
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 4 * CY_BUTTON, SRCCOPY);
        }
    } else { // ���̓R���e�L�X�g���Ȃ��B
        // disabled
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    // ���[�}�����͂��r�b�g�}�b�v�ŕ`�悷��B
    rc.left += CX_BUTTON + CX_BTNEDGE * 2;
    if (lpIMC) { // ���̓R���e�L�X�g���L�����H
        if (lpIMC->Conversion() & IME_CMODE_ROMAN) { // ���[�}�����͂��H
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 5 * CY_BUTTON, SRCCOPY);
        } else {
            ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                     CX_BUTTON, CY_BUTTON,
                     hMemDC, 0, 6 * CY_BUTTON, SRCCOPY);
        }
    } else { // ���̓R���e�L�X�g���������H
        // disabled
        ::BitBlt(hDC, rc.left + CX_BTNEDGE, rc.top + CY_BTNEDGE,
                 CX_BUTTON, CY_BUTTON,
                 hMemDC, 0, 9 * CY_BUTTON, SRCCOPY);
    }

    ::SelectObject(hMemDC, hbmOld); // �r�b�g�}�b�v�̑I������������B
    ::DeleteDC(hMemDC); // �������[DC��j������B

    if (lpIMC) TheIME.UnlockIMC(hIMC);
} // StatusWnd_Paint

// IME��ԃE�B���h�E�̓����蔻��B
STATUS_WND_HITTEST StatusWnd_HitTest(HWND hWnd, POINT pt) {
    ::ScreenToClient(hWnd, &pt);
    RECT rc;
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION;
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_1; // �{�^��1�B
    }
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION + CX_BUTTON + 2 * CX_BTNEDGE;
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_2; // �{�^��2�B
    }
    ::GetClientRect(hWnd, &rc);
    rc.left += CX_MINICAPTION + 2 * (CX_BUTTON + 2 * CX_BTNEDGE);
    rc.right = rc.left + CX_BUTTON + 2 * CX_BTNEDGE;
    if (::PtInRect(&rc, pt)) {
        return SWHT_BUTTON_3; // �{�^��3�B
    }
    ::GetWindowRect(hWnd, &rc);
    ::ClientToScreen(hWnd, &pt);
    if (::PtInRect(&rc, pt)) {
        return SWHT_CAPTION; // �L���v�V�����B
    }
    return SWHT_NONE;
} // StatusWnd_HitTest

// IME��ԃE�B���h�E�̈ʒu���X�V�B
void StatusWnd_Update(UIEXTRA *lpUIExtra) {
    HWND hwndStatus = lpUIExtra->hwndStatus;
    if (::IsWindow(hwndStatus)) {
        // �ݒ�f�[�^ "ptStatusWindow" ���g���āA�E�B���h�E�̈ʒu�𕜌�����B
        POINT pt;
        if (TheIME.GetUserData(L"ptStatusWindow", &pt, sizeof(pt))) {
            RECT rc;
            ::GetWindowRect(hwndStatus, &rc);
            ::MoveWindow(hwndStatus, pt.x, pt.y,
                         rc.right - rc.left, rc.bottom - rc.top, TRUE);
        }
        ::SendMessage(hwndStatus, WM_UI_UPDATE, 0, 0);
    }
} // StatusWnd_Update

// IME��ԃE�B���h�E�̃{�^�����������Ƃ��̓���B
void StatusWnd_OnButton(HWND hWnd, STATUS_WND_HITTEST hittest) {
    // UI�T�[�o�[�E�B���h�E��IMC���擾����B
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND); // �T�[�o�[�E�B���h�E�B
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC); // IMC�B
    if (hIMC == NULL) {
        ASSERT(0);
        return;
    }

    // IME�̏�Ԃ��擾����B
    DWORD dwConversion, dwSentence;
    BOOL bOpen = ImmGetOpenStatus(hIMC);
    if (!::ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence)) {
        ASSERT(0);
        return;
    }

    INPUT_MODE imode;
    switch (hittest) {
    case SWHT_BUTTON_1:
        // �ϊ����[�h��؂�ւ���B
        if (bOpen) {
            SetInputMode(hIMC, IMODE_HALF_ASCII);
        } else {
            SetInputMode(hIMC, IMODE_FULL_HIRAGANA);
        }
        break;
    case SWHT_BUTTON_2:
        // ���̓��[�h��؂�ւ���B
        imode = InputModeFromConversionMode(bOpen, dwConversion);
        imode = NextInputMode(imode);
        SetInputMode(hIMC, imode);
        break;
    case SWHT_BUTTON_3:
        // ���[�}�����̓��[�h��؂�ւ���B
        if (dwConversion & IME_CMODE_ROMAN) {
            dwConversion &= ~IME_CMODE_ROMAN;
        } else {
            dwConversion |= IME_CMODE_ROMAN;
        }
        ::ImmSetConversionStatus(hIMC, dwConversion, dwSentence);
        break;
    default:
        break;
    }
}

// IME��ԃE�B���h�E��Ń}�E�X���ړ����Ă���B
void StatusWnd_OnMouseMove(HWND hWnd, POINT pt, BOOL bDown) {
    static POINT prev = {-1, -1}; // ��O�̈ʒu�B
    if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) { // �L���v�V�������h���b�O���Ă���B
        if (bDown && ::GetCapture() == hWnd) { // �h���b�O�����H
            if (prev.x != -1 && prev.y != -1) { // ��O�̈ʒu�����邩�H
                // �E�B���h�E�̈ʒu�����炷�B
                RECT rc;
                ::GetWindowRect(hWnd, &rc);
                ::MoveWindow(hWnd,
                             rc.left + (pt.x - prev.x), rc.top + (pt.y - prev.y),
                             rc.right - rc.left, rc.bottom - rc.top,
                             TRUE);
            }
            prev = pt;
        } else { // ����ȊO�B�h���b�O���L�����Z������B
            prev.x = -1;
            prev.y = -1;
            ::ReleaseCapture();
            ::SetWindowLong(hWnd, FIGWL_MOUSE, SWHT_NONE);
        }
    }
}

// IME��ԃE�B���h�E�ō��{�^���������ꂽ�^�����ꂽ�B
void StatusWnd_OnLButton(HWND hWnd, POINT pt, BOOL bDown) {
    STATUS_WND_HITTEST hittest = StatusWnd_HitTest(hWnd, pt); // �����蔻����s���B
    switch (hittest) {
    case SWHT_CAPTION: // �L���v�V������B
        break;
    case SWHT_BUTTON_1: // �{�^��1�B
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_1) {
            // �ĕ`��B
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Paint(hWnd, hDC, (bDown ? 1 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_BUTTON_2: // �{�^��2�B
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_2) {
            // �ĕ`��B
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Paint(hWnd, hDC, (bDown ? 2 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_BUTTON_3: // �{�^��3�B
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_BUTTON_3) {
            // �ĕ`��B
            HDC hDC = ::GetDC(hWnd);
            StatusWnd_Paint(hWnd, hDC, (bDown ? 3 : 0));
            ::ReleaseDC(hWnd, hDC);
        }
        break;
    case SWHT_NONE: // ����ȊO�B
    {
        // �ĕ`��B
        HDC hDC = ::GetDC(hWnd);
        StatusWnd_Paint(hWnd, hDC, 0);
        ::ReleaseDC(hWnd, hDC);
    }
    break;
    }
    if (bDown) { // �����ꂽ�B
        ::SetCapture(hWnd); // �h���b�O���J�n���邽�߁A�L���v�`���[���Z�b�g����B
        ::SetWindowLong(hWnd, FIGWL_MOUSE, hittest); // �����ꂽ�ʒu���o���Ă����B
    } else { // �����ꂽ�B
        ::ReleaseCapture(); // �L���v�`���[��������A�h���b�O���I������B
        if (hittest == SWHT_CAPTION) { // �L���v�V������ł����
            RepositionWindow(hWnd); // �ʒu��␳����B
        } else { // �����Ȃ����
            StatusWnd_OnButton(hWnd, hittest); // �{�^���̃A�N�V�����𔭓�����B
            ::SetWindowLong(hWnd, FIGWL_MOUSE, SWHT_NONE); // �����ꂽ�ʒu���N���A����B
        }
    }
} // StatusWnd_OnLButton

// IME��ԃE�B���h�E���E�N���b�N���ꂽ�B
static BOOL StatusWnd_OnRClick(HWND hWnd, POINT pt) {
    HWND hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND); // IME UI�T�[�o�[�E�B���h�E�B
    HIMC hIMC = (HIMC)GetWindowLongPtr(hwndServer, IMMGWLP_IMC); // IMC�B
    if (hIMC == NULL) {
        ASSERT(0);
        return FALSE;
    }

    HMENU hMenu = ::LoadMenu(TheIME.m_hInst, TEXT("STATUSRMENU")); // ���j���[�����\�[�X����ǂݍ��ށB
    if (hMenu == NULL) {
        ASSERT(0);
        return FALSE;
    }

    HMENU hSubMenu = ::GetSubMenu(hMenu, 0); // �C���f�b�N�X0�̎q���j���[���擾�B

    TPMPARAMS params = { sizeof(params) };
    ::GetWindowRect(hWnd, &params.rcExclude);

    HWND hwndFore = ::GetForegroundWindow(); // �őO�ʃE�B���h�E���o���Ă����B
    ::SetForegroundWindow(hWnd); // TrackPopupMenuEx�̃o�O������B

    // ���j���[�̓��̓��[�h�Ƀ��W�I�}�[�N��t����B
    UINT uCheck = CommandFromInputMode(GetInputMode(hIMC));
    ::CheckMenuRadioItem(hSubMenu, IDM_HIRAGANA, IDM_HALF_ASCII, uCheck, MF_BYCOMMAND);
    if (IsRomanMode(hIMC)) {
        ::CheckMenuRadioItem(hSubMenu, IDM_ROMAN_INPUT, IDM_KANA_INPUT,
                             IDM_ROMAN_INPUT, MF_BYCOMMAND);
    } else {
        ::CheckMenuRadioItem(hSubMenu, IDM_ROMAN_INPUT, IDM_KANA_INPUT,
                             IDM_KANA_INPUT, MF_BYCOMMAND);
    }

    // ���j���[��\�����đI�������̂�҂B�I�����ꂽ��R�}���hID��Ԃ��B
    UINT nCommand = ::TrackPopupMenuEx(hSubMenu, TPM_RETURNCMD | TPM_NONOTIFY,
                                       pt.x, pt.y, hWnd, &params);
    TheIME.DoCommand(hIMC, nCommand); // �R�}���h�����B
    ::PostMessage(hWnd, WM_NULL, 0, 0); // TrackPopupMenuEx�̃o�O����B
    ::DestroyMenu(hMenu); // ���j���[��j������B
    ::SetForegroundWindow(hwndFore); // �őO�ʃE�B���h�E��߂��B
    return TRUE;
} // StatusWnd_OnRClick

// IME��ԃE�B���h�E�̃E�B���h�E�v���V�[�W���B
LRESULT CALLBACK
StatusWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HWND hwndServer;
    HDC hDC;
    HBITMAP hbm;
    POINT pt;

    switch (message) {
    case WM_CREATE: // �E�B���h�E�쐬���B
        // �r�b�g�}�b�v��ǂݍ��݁A�Z�b�g����B
        hbm = TheIME.LoadBMP(TEXT("MODESBMP"));
        SetWindowLongPtr(hWnd, FIGWLP_STATUSBMP, (LONG_PTR)hbm);
        break;

    case WM_PAINT: // �`�掞�B
        hDC = ::BeginPaint(hWnd, &ps);
        StatusWnd_Paint(hWnd, hDC, 0);
        ::EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY: // �E�B���h�E�j�����B
        hbm = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_STATUSBMP);
        ::DeleteObject(hbm);
        break;

    case WM_UI_UPDATE: // UI�X�V���B
        ::InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_LBUTTONUP: // ���{�^��������B
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        StatusWnd_OnLButton(hWnd, pt, FALSE);
        break;

    case WM_LBUTTONDOWN: // ���{�^���������ꂽ���B
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        StatusWnd_OnLButton(hWnd, pt, TRUE);
        break;

    case WM_MOUSEMOVE: // �}�E�X�ړ����B���Ƀh���b�O���B
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
            StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
        }
        break;

    case WM_RBUTTONUP: // �E�{�^��������B
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        break;

    case WM_RBUTTONDOWN: // �E�{�^���������ꂽ���B
        // This message comes from the captured window.
        ::GetCursorPos(&pt);
        break;

    case WM_SETCURSOR: // �}�E�X�J�[�\���ݒ莞�B
        // This message comes even from the disabled window.
        // ���̃��b�Z�[�W�͖����ȃE�B���h�E�ł�����B
        ::GetCursorPos(&pt);
        switch (HIWORD(lParam)) {
        case WM_MOUSEMOVE: // �}�E�X�ړ����B
            if (::GetWindowLong(hWnd, FIGWL_MOUSE) == SWHT_CAPTION) {
                StatusWnd_OnMouseMove(hWnd, pt, ::GetAsyncKeyState(VK_LBUTTON) < 0);
            }
            break;
        case WM_LBUTTONDOWN: // ���{�^���������ꂽ���B
            StatusWnd_OnLButton(hWnd, pt, TRUE);
            break;
        case WM_LBUTTONUP: // ���{�^��������B
            StatusWnd_OnLButton(hWnd, pt, FALSE);
            break;
        case WM_RBUTTONDOWN: // �E�{�^���������ꂽ���B
            break;
        case WM_RBUTTONUP: // �E�{�^��������B
            StatusWnd_OnRClick(hWnd, pt);
            break;
        }
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW)); // ���J�[�\�����w��B
        break;

    case WM_MOVE: // �E�B���h�E�ړ����B
        hwndServer = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (::IsWindow(hwndServer))
            SendMessage(hwndServer, WM_UI_STATEMOVE, wParam, lParam); // UI�T�[�o�[�ɑ���B
        break;

    default: // ���̑��̃��b�Z�[�W�B
        if (!IsImeMessage(message))
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
} // StatusWnd_WindowProc

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
