// ui.cpp --- mzimeja UI server
// UI�T�[�o�[�E�B���h�E�B
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "resource.h"

extern "C" {

//////////////////////////////////////////////////////////////////////////////

// �w�肳��Ă���UI�E�B���h�E��\������^��\���ɂ���B
void PASCAL ShowUIWindows(HWND hwndServer, BOOL fFlag) {
    int nsw = (fFlag ? SW_SHOWNOACTIVATE : SW_HIDE);

    UIEXTRA *lpUIExtra = LockUIExtra(hwndServer);
    if (lpUIExtra) {
        if (IsWindow(lpUIExtra->hwndStatus)) {
            ::ShowWindow(lpUIExtra->hwndStatus, nsw);
        }
        if (IsWindow(lpUIExtra->hwndCand)) {
            ::ShowWindow(lpUIExtra->hwndCand, nsw);
        }
        if (IsWindow(lpUIExtra->hwndDefComp)) {
            ::ShowWindow(lpUIExtra->hwndDefComp, nsw);
        }
        if (IsWindow(lpUIExtra->hwndGuide)) {
            ::ShowWindow(lpUIExtra->hwndGuide, nsw);
        }
        UnlockUIExtra(hwndServer);
    }
}

void OnImeSetContext(HWND hWnd, HIMC hIMC, LPARAM lParam) {
    UIEXTRA *lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
        // input context was changed.
        lpUIExtra->hIMC = hIMC;

        if (hIMC) {
            // the display have to be updated.
            InputContext *lpIMC = TheIME.LockIMC(hIMC);
            if (lpIMC) {
                CompStr *lpCompStr = lpIMC->LockCompStr();
                CandInfo *lpCandInfo = lpIMC->LockCandInfo();
                if (::IsWindow(lpUIExtra->hwndCand)) {
                    CandWnd_Hide(lpUIExtra);
                }
                if (lParam & ISC_SHOWUICANDIDATEWINDOW) {
                    if (lpCandInfo->dwCount) {
                        CandWnd_Create(hWnd, lpUIExtra, lpIMC);
                        CandWnd_Resize(lpUIExtra, lpIMC);
                        CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE);
                    }
                }

                if (::IsWindow(lpUIExtra->hwndDefComp)) {
                    CompWnd_Hide(lpUIExtra);
                }
                if (lParam & ISC_SHOWUICOMPOSITIONWINDOW) {
                    if (lpCompStr->dwCompStrLen) {
                        CompWnd_Create(hWnd, lpUIExtra, lpIMC);
                        CompWnd_Move(lpUIExtra, lpIMC);
                    }
                }

                DWORD bIsNonRoman = FALSE;
                if (TheIME.GetUserDword(L"IsNonRoman", &bIsNonRoman)) {
                    if (bIsNonRoman) {
                        lpIMC->Conversion() &= ~IME_CMODE_ROMAN;
                    } else {
                        lpIMC->Conversion() |= IME_CMODE_ROMAN;
                    }
                }

                lpIMC->UnlockCompStr();
                lpIMC->UnlockCandInfo();
                StatusWnd_Update(lpUIExtra);
                TheIME.UnlockIMC(hIMC);
            } else {
                ShowUIWindows(hWnd, FALSE);
            }
        } else { // it is NULL input context.
            ShowUIWindows(hWnd, FALSE);
        }
        UnlockUIExtra(hWnd);
    }
} // OnImeSetContext

void OnDestroy(HWND hWnd) {
    UIEXTRA *lpUIExtra = LockUIExtra(hWnd);
    if (lpUIExtra) {
        if (::IsWindow(lpUIExtra->hwndStatus))
            ::DestroyWindow(lpUIExtra->hwndStatus);

        if (::IsWindow(lpUIExtra->hwndCand))
            ::DestroyWindow(lpUIExtra->hwndCand);

        if (::IsWindow(lpUIExtra->hwndDefComp))
            ::DestroyWindow(lpUIExtra->hwndDefComp);

        for (INT i = 0; i < MAXCOMPWND; i++) {
            if (::IsWindow(lpUIExtra->hwndComp[i]))
                ::DestroyWindow(lpUIExtra->hwndComp[i]);
        }

        if (::IsWindow(lpUIExtra->hwndGuide))
            ::DestroyWindow(lpUIExtra->hwndGuide);

        if (lpUIExtra->hFont) {
            ::DeleteObject(lpUIExtra->hFont);
        }

        UnlockUIExtra(hWnd);
        FreeUIExtra(hWnd);
    }
}

// IME UI server window procedure
// IME UI�T�[�o�[�E�B���h�E�̃E�B���h�E�v���V�[�W���B
LRESULT CALLBACK MZIMEWndProc(HWND hWnd, UINT message, WPARAM wParam,
                              LPARAM lParam) {
    InputContext *lpIMC;
    UIEXTRA *lpUIExtra;
    HGLOBAL hUIExtra;
    LONG lRet = 0;

    HIMC hIMC = (HIMC)GetWindowLongPtr(hWnd, IMMGWLP_IMC);

    // Even if there is no current UI. these messages should not be pass to
    // DefWindowProc().
    if (hIMC == NULL) {
        if (IsImeMessage2(message)) {
            DPRINT("Why hIMC is NULL?\n");
            DPRINT("hWnd: %p, message: 0x%x, wParam: %x, lParam: %x\n",
                   hWnd, message, wParam, lParam);
            return 0;
        }
    }

    switch (message) {
    case WM_CREATE: // �E�B���h�E�쐬���B
        DPRINT("WM_CREATE\n");
        // Allocate UI's extra memory block.
        hUIExtra = GlobalAlloc(GHND, sizeof(UIEXTRA));
        lpUIExtra = (UIEXTRA *)GlobalLock(hUIExtra);
        if (lpUIExtra) {
            lpUIExtra->ptCand.x = -1;
            lpUIExtra->ptCand.y = -1;
            lpUIExtra->hFont = NULL;
            GlobalUnlock(hUIExtra);
        }
        SetUIExtraToServerWnd(hWnd, hUIExtra);
        break;

    case WM_IME_SETCONTEXT: // IME�R���e�L�X�g�ݒ莞�B
        DPRINT("WM_IME_SETCONTEXT\n");
        if (wParam) {
            OnImeSetContext(hWnd, hIMC, lParam);
        }
        // else
        //    ShowUIWindows(hWnd, FALSE);
        break;

    case WM_IME_STARTCOMPOSITION: // IME�ϊ��J�n���B
        DPRINT("WM_IME_STARTCOMPOSITION\n");
        // Start composition! Ready to display the composition string.
        lpUIExtra = LockUIExtra(hWnd); // �]��������b�N�B
        if (lpUIExtra) {
            lpIMC = TheIME.LockIMC(hIMC);
            CompWnd_Create(hWnd, lpUIExtra, lpIMC); // ���m�蕶����E�B���h�E���쐬�B
            TheIME.UnlockIMC(hIMC);
            UnlockUIExtra(hWnd); // �]����̃��b�N�������B
        }
        break;

    case WM_IME_COMPOSITION: // IME�ϊ����B
        DPRINT("WM_IME_COMPOSITION\n");
        // Update to display the composition string.
        lpIMC = TheIME.LockIMC(hIMC);
        if (lpIMC) {
            lpUIExtra = LockUIExtra(hWnd); // �]��������b�N�B
            if (lpUIExtra) {
                CompWnd_Move(lpUIExtra, lpIMC); // ���m�蕶������ړ��B
                CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE); // ���E�B���h�E���ړ��B
                UnlockUIExtra(hWnd); // �]����̃��b�N�������B
            }
            TheIME.UnlockIMC(hIMC);
        }
        break;

    case WM_IME_ENDCOMPOSITION: // IME�ϊ��I�����B
        DPRINT("WM_IME_ENDCOMPOSITION\n");
        // Finish to display the composition string.
        lpUIExtra = LockUIExtra(hWnd); // �]��������b�N�B
        if (lpUIExtra) {
            CompWnd_Hide(lpUIExtra); // ���m�蕶������B���B
            UnlockUIExtra(hWnd); // �]����̃��b�N�������B
        }
        break;

    case WM_IME_COMPOSITIONFULL: // �ϊ������񂪂����ς��B
        DPRINT("WM_IME_COMPOSITIONFULL\n");
        break;

    case WM_IME_SELECT: // IME�I�����B
        DPRINT("WM_IME_SELECT\n");
        if (wParam) {
            lpUIExtra = LockUIExtra(hWnd); // �]��������b�N�B
            if (lpUIExtra) {
                lpUIExtra->hIMC = hIMC; // hIMC���Z�b�g�B
                UnlockUIExtra(hWnd); // �]����̃��b�N�������B
            }
        }
        break;

    case WM_IME_CONTROL: // IME���䎞�B
        DPRINT("WM_IME_CONTROL\n");
        lRet = ControlCommand(hIMC, hWnd, wParam, lParam);
        break;

    case WM_IME_NOTIFY: // IME�ʒm���B
        DPRINT("WM_IME_NOTIFY\n");
        lRet = NotifyCommand(hIMC, hWnd, wParam, lParam);
        break;

    case WM_DESTROY: // �E�B���h�E�j�����B
        DPRINT("WM_DESTROY\n");
        OnDestroy(hWnd);
        break;

    case WM_UI_STATEMOVE: // IME��ԃE�B���h�E���ړ��B
        // Set the position of the status window to UIExtra.
        // This message is sent by the status window.
        lpUIExtra = LockUIExtra(hWnd); // �]��������b�N�B
        if (lpUIExtra) {
            // �ʒu���擾���A
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            // �o���Ă����B
            POINT pt = { rc.left, rc.top };
            TheIME.SetUserData(L"ptStatusWindow", &pt, sizeof(pt));
            UnlockUIExtra(hWnd); // �]����̃��b�N�������B
        }
        break;

    case WM_UI_DEFCOMPMOVE:
        // Set the position of the composition window to UIExtra.
        // This message is sent by the composition window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            if (!lpUIExtra->dwCompStyle) {
                RECT rc;
                ::GetWindowRect(lpUIExtra->hwndDefComp, &rc);
                POINT pt = { rc.left, rc.top };
                TheIME.SetUserData(L"ptDefComp", &pt, sizeof(pt));
            }
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_UI_CANDMOVE:
        // Set the position of the candidate window to UIExtra.
        // This message is sent by the candidate window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            // �ʒu���擾�B
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndCand, &rc);
            // �ʒu���o���Ă����B
            POINT pt = { rc.left, rc.top };
            lpUIExtra->ptCand = pt;
            UnlockUIExtra(hWnd);
        }
        break;

    case WM_UI_GUIDEMOVE: // �K�C�h���C���E�B���h�E���ړ������B
        // Set the position of the status window to UIExtra.
        // This message is sent by the status window.
        lpUIExtra = LockUIExtra(hWnd);
        if (lpUIExtra) {
            // �ʒu���擾�B
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndGuide, &rc);
            // �ʒu���o���Ă����B
            POINT pt = { rc.left, rc.top };
            TheIME.SetUserData(L"ptGuide", &pt, sizeof(pt));
            UnlockUIExtra(hWnd);
        }
        break;

    default: // ���̑��̃��b�Z�[�W�B
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return lRet;
}

// �t�H���g�̍������擾����B
int GetCompFontHeight(UIEXTRA *lpUIExtra) {
    HDC hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL); // DC���쐬����i���̂݁j�B
    HFONT hOldFont = NULL;

    // �t�H���g������΃t�H���g��I���B
    if (lpUIExtra->hFont) hOldFont = (HFONT)SelectObject(hIC, lpUIExtra->hFont);

    // �T�C�Y���擾�B
    SIZE siz;
    GetTextExtentPoint(hIC, TEXT("A"), 1, &siz);

    if (hOldFont) SelectObject(hIC, hOldFont); // �t�H���g�̑I������������B

    DeleteDC(hIC); // DC��j������B
    return siz.cy; // ���ꂪ�����B
}

// WM_IME_NOTIFY ���b�Z�[�W����������B
LONG NotifyCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam) {
    LONG ret = 0;
    RECT rc;
    LOGFONT lf;
    InputContext *lpIMC;

    UIEXTRA *lpUIExtra = LockUIExtra(hWnd); // �]����B

    switch (wParam) {
    case IMN_CLOSESTATUSWINDOW: // ��ԃE�B���h�E�����B
        DPRINT("IMN_CLOSESTATUSWINDOW\n");
        if (::IsWindow(lpUIExtra->hwndStatus)) { // ��ԃE�B���h�E�������Ă���H
            // �ʒu��ۑ��B
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            POINT pt;
            pt.x = rc.left;
            pt.y = rc.top;
            TheIME.SetUserData(L"ptStatusWindow", &pt, sizeof(pt));

            ::ShowWindow(lpUIExtra->hwndStatus, SW_HIDE); // ���ۂɂ͔j�������̂ł͂Ȃ��B�����B
        }
        break;

    case IMN_OPENSTATUSWINDOW: // ��ԃE�B���h�E���J�����B
        DPRINT("IMN_OPENSTATUSWINDOW\n");
        StatusWnd_Create(hWnd, lpUIExtra); // ��ԃE�B���h�E���쐬����B
        break;

    case IMN_SETCONVERSIONMODE: // �ϊ����[�h���Z�b�g�����B
        DPRINT("IMN_SETCONVERSIONMODE\n");
        lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
        if (lpIMC) {
            // ���[�}�����[�h��ۑ��B
            if (lpIMC->Conversion() & IME_CMODE_ROMAN) {
                TheIME.SetUserDword(L"IsNonRoman", FALSE);
            } else {
                TheIME.SetUserDword(L"IsNonRoman", TRUE);
            }
            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B
        }
        StatusWnd_Update(lpUIExtra); // �]������X�V�B
        break;

    case IMN_SETSENTENCEMODE:
        DPRINT("IMN_SETSENTENCEMODE\n");
        break;

    case IMN_SETCOMPOSITIONFONT: // ���m�蕶����̃t�H���g���Z�b�g�����B
        DPRINT("IMN_SETCOMPOSITIONFONT\n");
        lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
        if (lpIMC) {
            lf = lpIMC->lfFont.W; // �_���t�H���g�B
            if (lpUIExtra->hFont) DeleteObject(lpUIExtra->hFont); // ���łɂ���Δj���B

            // �t�H���g�̌����B
            if (lf.lfEscapement == 2700) // 270�x�B
                lpUIExtra->bVertical = TRUE; // �c�����B
            else {
                lf.lfEscapement = 0;
                lpUIExtra->bVertical = FALSE; // �������B
            }

            // ���݂̃t�H���g�����{��łȂ���Εʂ̓��{��t�H���g��T���B
            if (lf.lfCharSet != SHIFTJIS_CHARSET) {
                lf.lfCharSet = SHIFTJIS_CHARSET;
                lf.lfFaceName[0] = 0;
            }

            lpUIExtra->hFont = CreateFontIndirect(&lf); // �_���t�H���g����t�H���g���쐬�B
            CompWnd_SetFont(lpUIExtra); // �t�H���g��]����ɃZ�b�g�B
            CompWnd_Move(lpUIExtra, lpIMC); // ��ԃE�B���h�E���ړ��B

            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N������
        }
        break;

    case IMN_SETOPENSTATUS: // IME��ON/OFF��؂�ւ��B
        DPRINT("IMN_SETOPENSTATUS\n");
        StatusWnd_Update(lpUIExtra); // ��ԃE�B���h�E���X�V����B
        break;

    case IMN_OPENCANDIDATE: // ��₪�J�����B
        DPRINT("IMN_OPENCANDIDATE\n");
        lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
        if (lpIMC) {
            CandWnd_Create(hWnd, lpUIExtra, lpIMC); // ���E�B���h�E���쐬�B
            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B
        }
        break;

    case IMN_CHANGECANDIDATE: // ��₪�ύX�����B
        DPRINT("IMN_CHANGECANDIDATE\n");
        lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
        if (lpIMC) {
            CandWnd_Resize(lpUIExtra, lpIMC); // ���E�B���h�E�̃T�C�Y��ύX�B
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE); // ���E�B���h�E���ړ��B
            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B
        }
        break;

    case IMN_CLOSECANDIDATE: // ��₪������B
        DPRINT("IMN_CLOSECANDIDATE\n");
        CandWnd_Hide(lpUIExtra); // ���E�B���h�E���B���B
        break;

    case IMN_GUIDELINE:
        DPRINT("IMN_GUIDELINE\n");
        if (::ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0)) {
            if (!::IsWindow(lpUIExtra->hwndGuide)) {
                HDC hdcIC;
                TEXTMETRIC tm;
                int dx, dy;
                POINT pt;

                lpIMC = TheIME.LockIMC(hIMC);
                if (lpIMC) {
                    if (!TheIME.GetUserData(L"ptGuide", &pt, sizeof(pt))) {
                        ::GetWindowRect(lpIMC->hWnd, &rc);
                        pt.x = rc.left;
                        pt.y = rc.bottom;
                    }
                    TheIME.UnlockIMC(hIMC);
                }

                hdcIC = ::CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
                ::GetTextMetrics(hdcIC, &tm);
                dx = tm.tmAveCharWidth * MAXGLCHAR;
                dy = tm.tmHeight + tm.tmExternalLeading;
                ::DeleteDC(hdcIC);

                lpUIExtra->hwndGuide = ::CreateWindowEx(
                        WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME, szGuideClassName,
                        NULL, WS_DISABLED | WS_POPUP | WS_BORDER, pt.x, pt.y,
                        dx + 2 * GetSystemMetrics(SM_CXBORDER) + 2 * GetSystemMetrics(SM_CXEDGE),
                        dy + GetSystemMetrics(SM_CYSMCAPTION) + 2 * GetSystemMetrics(SM_CYBORDER) +
                        2 * GetSystemMetrics(SM_CYEDGE),
                        hWnd, NULL, TheIME.m_hInst, NULL);
            }
            ::ShowWindow(lpUIExtra->hwndGuide, SW_SHOWNOACTIVATE);
            ::SetWindowLongPtr(lpUIExtra->hwndGuide, FIGWLP_SERVERWND, (LONG_PTR)hWnd);
            GuideWnd_Update(lpUIExtra);
        }
        break;

    case IMN_SETCANDIDATEPOS: // ���̈ʒu���Z�b�g�����B
        DPRINT("IMN_SETCANDIDATEPOS\n");
        lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
        if (lpIMC) {
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, FALSE); // ���E�B���h�E���ړ��B
            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B
        }
        break;

    case IMN_SETCOMPOSITIONWINDOW: // ���m�蕶����E�B���h�E���Z�b�g�����B
        DPRINT("IMN_SETCOMPOSITIONWINDOW\n");
        lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
        if (lpIMC) {
            CompWnd_Move(lpUIExtra, lpIMC); // ���m�蕶����E�B���h�E���ړ��B
            CandWnd_Move(hWnd, lpIMC, lpUIExtra, TRUE); // ���E�B���h�E���ړ��B
            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B
        }
        break;

    case IMN_SETSTATUSWINDOWPOS: // ��ԃE�B���h�E�̈ʒu���Z�b�g�����B
        DPRINT("IMN_SETSTATUSWINDOWPOS\n");
        lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
        if (lpIMC) {
            // �ʒu���Z�b�g����B���ƍ����͂��̂܂܁B
            RECT rc;
            ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
            POINT pt = lpIMC->ptStatusWndPos;
            ::MoveWindow(lpUIExtra->hwndStatus, pt.x, pt.y,
                         rc.right - rc.left, rc.bottom - rc.top, TRUE);

            TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B
        }
        break;

    case IMN_PRIVATE: // �v���C�x�[�g�Ȓʒm�B
        DPRINT("IMN_PRIVATE\n");
        if (HIWORD(lParam) == 0xFACE) {
            std::wstring imepad_file;
            if (TheIME.GetComputerString(L"ImePadFile", imepad_file)) {
                ::ShellExecuteW(NULL, NULL, imepad_file.c_str(),
                                NULL, NULL, SW_SHOWNOACTIVATE);
            }
            break;
        }
        if (HIWORD(lParam) == 0xDEAD) {
            std::wstring verinfo_file;
            if (TheIME.GetComputerString(L"VerInfoFile", verinfo_file)) {
                ::ShellExecuteW(NULL, NULL, verinfo_file.c_str(),
                                NULL, NULL, SW_SHOWNOACTIVATE);
            }
            break;
        }
        break;

    default:
        break;
    }

    UnlockUIExtra(hWnd);

    return ret;
}

// WM_IME_CONTROL ���b�Z�[�W����������B
LONG ControlCommand(HIMC hIMC, HWND hWnd, WPARAM wParam, LPARAM lParam) {
    LONG ret = 1;

    InputContext *lpIMC = TheIME.LockIMC(hIMC); // ���̓R���e�L�X�g�����b�N����B
    if (NULL == lpIMC) return ret;

    UIEXTRA *lpUIExtra = LockUIExtra(hWnd); // �]��������b�N����B
    if (lpUIExtra) {
        switch (wParam) {
        case IMC_GETCANDIDATEPOS: // ���̈ʒu���擾�����B
            DPRINT("IMC_GETCANDIDATEPOS\n");
            if (IsWindow(lpUIExtra->hwndCand)) { // ���E�B���h�E�������Ă����
                *(LPCANDIDATEFORM)lParam = lpIMC->cfCandForm[0]; // ���̓R���e�L�X�g����擾�B
                ret = 0;
            }
            break;

        case IMC_GETCOMPOSITIONWINDOW: // ���m�蕶����̍\���̂��擾�B
            DPRINT("IMC_GETCOMPOSITIONWINDOW\n");
            *(LPCOMPOSITIONFORM)lParam = lpIMC->cfCompForm; // ���̓R���e�L�X�g����擾�B
            ret = 0;
            break;

        case IMC_GETSTATUSWINDOWPOS: // ��ԃE�B���h�E�̈ʒu���擾�����B
            DPRINT("IMC_GETSTATUSWINDOWPOS\n");
            {
                RECT rc;
                ::GetWindowRect(lpUIExtra->hwndStatus, &rc);
                ret = MAKELONG(rc.left, rc.top); // �ʒu��Ԃ��B
            }
            break;

        default:
            break;
        }

        UnlockUIExtra(hWnd); // �]����̃��b�N�������B
    }

    TheIME.UnlockIMC(hIMC); // ���̓R���e�L�X�g�̃��b�N�������B

    return ret;
}

// �q�E�B���h�E���h���b�O����Ă���΁A���̊֐��̓{�[�_�[��`�悷��B
void DrawUIBorder(LPRECT lprc) {
    HDC hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL); // DC���쐬�B
    SelectObject(hDC, GetStockObject(GRAY_BRUSH)); // �u���V��I���B�����PatBlt�œh��Ԃ��B

    // �{�[�_�[���ƍ����B
    INT sbx = GetSystemMetrics(SM_CXBORDER);
    INT sby = GetSystemMetrics(SM_CYBORDER);

    // �h��Ԃ��B
    PatBlt(hDC, lprc->left, lprc->top, lprc->right - lprc->left - sbx, sby, PATINVERT);
    PatBlt(hDC, lprc->right - sbx, lprc->top, sbx, lprc->bottom - lprc->top - sby, PATINVERT);
    PatBlt(hDC, lprc->right, lprc->bottom - sby, -(lprc->right - lprc->left - sbx), sby, PATINVERT);
    PatBlt(hDC, lprc->left, lprc->bottom, sbx, -(lprc->bottom - lprc->top - sby), PATINVERT);

    DeleteDC(hDC); // DC��j���B
}

// Handling mouse messages for the child windows
void DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    POINT pt;
    static POINT ptdif;
    static RECT drc;
    static RECT rc;
    DWORD dwT;

    switch (message) {
    case WM_SETCURSOR:
        if (HIWORD(lParam) == WM_LBUTTONDOWN ||
            HIWORD(lParam) == WM_RBUTTONDOWN) {
            GetCursorPos(&pt);
            SetCapture(hWnd);
            GetWindowRect(hWnd, &drc);
            ptdif.x = pt.x - drc.left;
            ptdif.y = pt.y - drc.top;
            rc = drc;
            rc.right -= rc.left;
            rc.bottom -= rc.top;
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, FIM_CAPUTURED);
        }
        break;

    case WM_MOUSEMOVE:
        dwT = (DWORD)::GetWindowLongPtr(hWnd, FIGWL_MOUSE);
        if (dwT & FIM_MOVED) {
            DrawUIBorder(&drc);
            GetCursorPos(&pt);
            drc.left = pt.x - ptdif.x;
            drc.top = pt.y - ptdif.y;
            drc.right = drc.left + rc.right;
            drc.bottom = drc.top + rc.bottom;
            DrawUIBorder(&drc);
        } else if (dwT & FIM_CAPUTURED) {
            DrawUIBorder(&drc);
            ::SetWindowLongPtr(hWnd, FIGWL_MOUSE, dwT | FIM_MOVED);
        }
        break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        dwT = (DWORD)::GetWindowLongPtr(hWnd, FIGWL_MOUSE);

        if (dwT & FIM_CAPUTURED) {
            ReleaseCapture();
            if (dwT & FIM_MOVED) {
                DrawUIBorder(&drc);
                GetCursorPos(&pt);
                MoveWindow(hWnd, pt.x - ptdif.x, pt.y - ptdif.y, rc.right, rc.bottom,
                           TRUE);
            }
        }
        break;
    }
}

// IME���b�Z�[�W���i���̂P�j�H
// Any UI window should not pass the IME messages to DefWindowProc.
BOOL IsImeMessage(UINT message) {
    switch (message) {
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_COMPOSITION:
    case WM_IME_NOTIFY:
    case WM_IME_SETCONTEXT:
    case WM_IME_CONTROL:
    case WM_IME_COMPOSITIONFULL:
    case WM_IME_SELECT:
    case WM_IME_CHAR:
        return TRUE;
    }
    return FALSE;
}

// IME���b�Z�[�W���i���̂Q�j�H
BOOL IsImeMessage2(UINT message) {
    switch (message) {
    case WM_IME_STARTCOMPOSITION:
    case WM_IME_ENDCOMPOSITION:
    case WM_IME_COMPOSITION:
    //case WM_IME_NOTIFY:
    //case WM_IME_SETCONTEXT:
    case WM_IME_CONTROL:
    case WM_IME_COMPOSITIONFULL:
    case WM_IME_SELECT:
    case WM_IME_CHAR:
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

}  // extern "C"

//////////////////////////////////////////////////////////////////////////////
