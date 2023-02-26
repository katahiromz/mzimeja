// uiguide.cpp --- mzimeja guideline window UI
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// �K�C�h���C���E�B���h�E�̃E�B���h�E�v���V�[�W���B
LRESULT CALLBACK GuideWnd_WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                                     LPARAM lParam) {
    PAINTSTRUCT ps;
    HWND hUIWnd;
    HDC hDC;
    HBITMAP hbmpGuide;
    RECT rc;

    switch (message) {
    case WM_UI_HIDE: // UI���B���������B
        ShowWindow(hWnd, SW_HIDE);
        break;

    case WM_UI_UPDATE: // UI���X�V���鎞�B
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT: // �`�掞�B
        hDC = BeginPaint(hWnd, &ps);
        GuideWnd_Paint(hWnd, hDC, NULL, 0);
        EndPaint(hWnd, &ps);
        break;

    case WM_MOUSEMOVE: // �}�E�X�ړ����B
    case WM_SETCURSOR: // �J�[�\���ݒ莞�B
    case WM_LBUTTONUP: // ���{�^��������B
    case WM_RBUTTONUP: // �E�{�^��������B
        GuideWnd_Button(hWnd, message, wParam, lParam); // �}�E�X�A�N�V�����B
        if ((message == WM_SETCURSOR) && (HIWORD(lParam) != WM_LBUTTONDOWN) &&
            (HIWORD(lParam) != WM_RBUTTONDOWN))
            return DefWindowProc(hWnd, message, wParam, lParam);
        if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP)) {
            // ��Ԃ�߂��B
            SetWindowLong(hWnd, FIGWL_MOUSE, 0);
            SetWindowLong(hWnd, FIGWL_PUSHSTATUS, 0);
        }
        break;

    case WM_MOVE: // �E�B���h�E�ړ����B
        hUIWnd = (HWND)GetWindowLongPtr(hWnd, FIGWLP_SERVERWND);
        if (IsWindow(hUIWnd))
            SendMessage(hUIWnd, WM_UI_GUIDEMOVE, 0, 0); // UI�T�[�o�[�ɑ���B
        break;

    case WM_CREATE: // �E�B���h�E�쐬���B
        hbmpGuide = TheIME.LoadBMP(TEXT("CLOSEBMP")); // �r�b�g�}�b�v��ǂݍ��݁A�ۑ�����B
        SetWindowLongPtr(hWnd, FIGWLP_CLOSEBMP, (LONG_PTR)hbmpGuide);
        GetClientRect(hWnd, &rc);
        break;

    case WM_DESTROY: // �E�B���h�E�j�����B
        hbmpGuide = (HBITMAP)GetWindowLongPtr(hWnd, FIGWLP_CLOSEBMP);
        DeleteObject(hbmpGuide); // �r�b�g�}�b�v��j������B
        break;

    default: // ����ȊO�̃��b�Z�[�W�B
        if (!IsImeMessage(message))
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

DWORD PASCAL CheckPushedGuide(HWND hGuideWnd, LPPOINT lppt) {
    POINT pt;
    RECT rc;

    if (lppt) {
        pt = *lppt;
        ScreenToClient(hGuideWnd, &pt);
        GetClientRect(hGuideWnd, &rc);
        if (!PtInRect(&rc, pt)) return 0;

        rc.left = rc.right - STCLBT_DX - 2;
        rc.top = STCLBT_Y;
        rc.right = rc.left + STCLBT_DX;
        rc.bottom = rc.top + STCLBT_DY;
        if (PtInRect(&rc, pt)) return PUSHED_STATUS_CLOSE;
    }
    return 0;
}

// �K�C�h���C���̕`�掞�B
void GuideWnd_Paint(HWND hGuideWnd, HDC hDC, LPPOINT lppt,
                    DWORD dwPushedGuide) {
    HBITMAP hbmpOld;
    HANDLE hGLStr;
    WCHAR *lpGLStr;
    DWORD dwLevel;
    DWORD dwSize;

    HWND hSvrWnd = (HWND)GetWindowLongPtr(hGuideWnd, FIGWLP_SERVERWND); // UI�T�[�o�[�B

    HIMC hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC); // IMC�B
    if (hIMC == NULL) {
        ASSERT(0);
        return;
    }

    HBITMAP hbmpGuide;
    HBRUSH hOldBrush, hBrush;
    int nCyCap = GetSystemMetrics(SM_CYSMCAPTION); // �������L���v�V�����̃^�e���@�B

    HDC hMemDC = CreateCompatibleDC(hDC); // �������[DC���쐬�B

    // �L���v�V������`�悷��B
    hBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION)); // �u���V���쐬�B
    hOldBrush = (HBRUSH)SelectObject(hDC, hBrush); // �u���V�I���B
    RECT rc;
    GetClientRect(hGuideWnd, &rc); // �N���C�A���g�̈���擾�B
    // rc.top = rc.left = 0;
    // rc.right = BTX*3;
    rc.bottom = nCyCap; // �L���v�V�����̗̈�ɂ���B
    FillRect(hDC, &rc, hBrush); // �h��Ԃ��B
    SelectObject(hDC, hOldBrush); // �u���V�I���������B
    DeleteObject(hBrush); // �u���V��j���B

    // ����{�^���̃r�b�g�}�b�v���擾�B
    hbmpGuide = (HBITMAP)GetWindowLongPtr(hGuideWnd, FIGWLP_CLOSEBMP);
    hbmpOld = (HBITMAP)SelectObject(hMemDC, hbmpGuide); // �r�b�g�}�b�v��I���B

    // ��Ԃɉ����ăr�b�g�}�b�v�ŕ`��B
    if (!(dwPushedGuide & PUSHED_STATUS_CLOSE))
        BitBlt(hDC, rc.right - STCLBT_DX - 2, STCLBT_Y, STCLBT_DX, STCLBT_DY,
               hMemDC, 0, 0, SRCCOPY);
    else
        BitBlt(hDC, rc.right - STCLBT_DX - 2, STCLBT_Y, STCLBT_DX, STCLBT_DY,
               hMemDC, STCLBT_DX, 0, SRCCOPY);

    // �K�C�h���C���̃��x�����擾�B
    dwLevel = ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0);
    if (dwLevel) {
        // �K�C�h���C���̕�����̃T�C�Y���擾�B
        dwSize = ImmGetGuideLine(hIMC, GGL_STRING, NULL, 0) + 1;
        // ������ɑ΂��郁�����[���蓖�āB
        if ((dwSize > 1) && (hGLStr = GlobalAlloc(GHND, dwSize))) {
            lpGLStr = (LPTSTR)GlobalLock(hGLStr); // �������[�����b�N�B
            if (lpGLStr) {
                COLORREF rgb = 0;
                HBRUSH hbrLGR = (HBRUSH)GetStockObject(LTGRAY_BRUSH); // �u���V���擾�B
                HBRUSH hbr;

                hbr = (HBRUSH)SelectObject(hDC, hbrLGR); // �u���V�I���B
                GetClientRect(hGuideWnd, &rc); // �N���C�A���g�̈���擾�B
                // �u���V�œh��Ԃ��B
                PatBlt(hDC, 0, nCyCap, rc.right, rc.bottom - nCyCap, PATCOPY);
                SelectObject(hDC, hbr); // �u���V�I���������B

                // ���x���ɉ����ĐF��ς���B
                switch (dwLevel) {
                case GL_LEVEL_FATAL:
                case GL_LEVEL_ERROR:
                    rgb = RGB(255, 0, 0);
                    break;
                case GL_LEVEL_WARNING:
                    rgb = RGB(0, 0, 255);
                    break;
                case GL_LEVEL_INFORMATION:
                default:
                    rgb = RGB(0, 0, 0);
                    break;
                }

                // �m�ۂ����������[���g���ăK�C�h���C����������擾�B
                dwSize = ImmGetGuideLine(hIMC, GGL_STRING, lpGLStr, dwSize);
                if (dwSize) {
                    SetTextColor(hDC, rgb); // �e�L�X�g�F���w��B
                    SetBkMode(hDC, TRANSPARENT); // �w�i�����B
                    TextOut(hDC, 0, nCyCap, lpGLStr, dwSize); // ������`��B
                }
                GlobalUnlock(hGLStr); // �����񃁃����[�̃��b�N�������B
            }
            GlobalFree(hGLStr); // ����B
        }
    }

    SelectObject(hMemDC, hbmpOld); // �r�b�g�}�b�v�I���������B
    DeleteDC(hMemDC); // �������[DC��j���B
}

// �K�C�h���C���E�B���h�E�̃}�E�X�A�N�V�����B
void GuideWnd_Button(HWND hGuideWnd, UINT message, WPARAM wParam,
                     LPARAM lParam) {
    POINT pt;
    HDC hDC;
    DWORD dwMouse;
    DWORD dwPushedGuide;
    DWORD dwTemp;
    HIMC hIMC;
    HWND hSvrWnd;
    static POINT ptdif;
    static RECT drc;
    static RECT rc;
    static DWORD dwCurrentPushedGuide;

    hDC = GetDC(hGuideWnd); // DC���擾�B
    switch (message) {
    case WM_SETCURSOR: // �J�[�\���`��̐ݒ莞�B
        if (HIWORD(lParam) == WM_LBUTTONDOWN ||
            HIWORD(lParam) == WM_RBUTTONDOWN) { // ���{�^�����E�{�^���������ꂽ���B
            GetCursorPos(&pt); // �J�[�\���ʒu���擾�B
            SetCapture(hGuideWnd); // �L���v�`���[���Z�b�g���ăh���b�O���J�n�B
            GetWindowRect(hGuideWnd, &drc); // �E�B���h�E�̈ʒu�ƃT�C�Y���擾�B
            // �E�B���h�E�ʒu�ƃN���b�N�ʒu�̍����擾�B
            ptdif.x = pt.x - drc.left;
            ptdif.y = pt.y - drc.top;
            rc = drc;
            rc.right -= rc.left;
            rc.bottom -= rc.top;
            SetWindowLong(hGuideWnd, FIGWL_MOUSE, FIM_CAPUTURED); // �L���v�`����Ԃ�ۑ��B
            dwPushedGuide = CheckPushedGuide(hGuideWnd, &pt);
            SetWindowLong(hGuideWnd, FIGWL_PUSHSTATUS, dwPushedGuide); // �����ꂽ��Ԃ�ۑ��B
            GuideWnd_Paint(hGuideWnd, hDC, &pt, dwPushedGuide); // �ĕ`��B
            dwCurrentPushedGuide = dwPushedGuide; // �����ꂽ��Ԃ��o����B
        }
        break;

    case WM_MOUSEMOVE:
        dwMouse = GetWindowLong(hGuideWnd, FIGWL_MOUSE); // ��Ԃ��擾�B
        if (!(dwPushedGuide = GetWindowLong(hGuideWnd, FIGWL_PUSHSTATUS))) { // �����ꂽ��Ԃ��Ȃ����
            if (dwMouse & FIM_MOVED) { // �ړ������H
                DrawUIBorder(&drc);
                GetCursorPos(&pt);
                drc.left = pt.x - ptdif.x;
                drc.top = pt.y - ptdif.y;
                drc.right = drc.left + rc.right;
                drc.bottom = drc.top + rc.bottom;
                DrawUIBorder(&drc);
            } else if (dwMouse & FIM_CAPUTURED) {
                DrawUIBorder(&drc);
                SetWindowLong(hGuideWnd, FIGWL_MOUSE, dwMouse | FIM_MOVED);
            }
        } else {
            GetCursorPos(&pt); // �}�E�X�J�[�\���ʒu���擾�B
            dwTemp = CheckPushedGuide(hGuideWnd, &pt);
            if ((dwTemp ^ dwCurrentPushedGuide) & dwPushedGuide)
                GuideWnd_Paint(hGuideWnd, hDC, &pt, dwPushedGuide & dwTemp);
            dwCurrentPushedGuide = dwTemp;
        }
        break;

    case WM_LBUTTONUP: // ���{�^��������B
    case WM_RBUTTONUP: // �E�{�^��������B
        dwMouse = GetWindowLong(hGuideWnd, FIGWL_MOUSE); // �}�E�X��Ԃ��擾�B
        if (dwMouse & FIM_CAPUTURED) { // �L���v�`�����Ă���H
            ReleaseCapture(); // �L���v�`��������B
            if (dwMouse & FIM_MOVED) { // �ړ������H
                DrawUIBorder(&drc); // �{�[�_�[���ĕ`��B
                GetCursorPos(&pt); // �}�E�X�J�[�\���ʒu���擾�B
                // �}�E�X�ʒu�Ɉړ��B
                MoveWindow(hGuideWnd, pt.x - ptdif.x, pt.y - ptdif.y, rc.right,
                           rc.bottom, TRUE);
            }
        }

        // UI�T�[�o�[���擾�B
        hSvrWnd = (HWND)GetWindowLongPtr(hGuideWnd, FIGWLP_SERVERWND);

        hIMC = (HIMC)GetWindowLongPtr(hSvrWnd, IMMGWLP_IMC); // IMC�B
        if (hIMC) {
            GetCursorPos(&pt); // �}�E�X�ʒu���擾�B
            dwPushedGuide = GetWindowLong(hGuideWnd, FIGWL_PUSHSTATUS); // �����ꂽ��Ԃ��擾�B
            dwPushedGuide &= CheckPushedGuide(hGuideWnd, &pt);
            if (!dwPushedGuide) {
            } else if (dwPushedGuide == PUSHED_STATUS_CLOSE) {
                PostMessage(hGuideWnd, WM_UI_HIDE, 0, 0);
            }
        }
        GuideWnd_Paint(hGuideWnd, hDC, NULL, 0); // �ĕ`��B
        break;
    }
    ReleaseDC(hGuideWnd, hDC); // DC������B
}

// �K�C�h���C���̍X�V�B
void GuideWnd_Update(UIEXTRA *lpUIExtra) {
    // �K�C�h���C���E�B���h�E�ɍX�V���b�Z�[�W�𑗂�B
    if (::IsWindow(lpUIExtra->hwndGuide)) {
        ::SendMessage(lpUIExtra->hwndGuide, WM_UI_UPDATE, 0, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
