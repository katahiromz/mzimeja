// cand_info.cpp --- candidate info of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define MAX_CANDLISTS   32
#define CANDPAGE_SIZE       8

//////////////////////////////////////////////////////////////////////////////
// LogCandList - ��⃊�X�g�̘_���f�[�^�B

// �����̃N���A�B
void LogCandList::clear() {
    dwStyle = IME_CAND_READ;
    dwSelection = 0;
    dwPageStart = 0;
    dwPageSize = CANDPAGE_SIZE;
    cand_strs.clear();
}

// ��⃊�X�g�̕����f�[�^�̍��v�T�C�Y���v�Z�B
DWORD LogCandList::GetTotalSize() const {
    DWORD total = sizeof(CANDIDATELIST);
    total += DWORD(cand_strs.size() * sizeof(DWORD));
    for (size_t iCand = 0; iCand < cand_strs.size(); ++iCand) {
        total += DWORD((cand_strs[iCand].size() + 1) * sizeof(WCHAR));
    }
    return total;
}

// ���̌��B
DWORD LogCandList::GetCandCount() const {
    return (DWORD)cand_strs.size();
}

// ���̌�⃊�X�g�ցB
void LogCandList::MoveNext() {
    ++dwSelection;
    if (dwSelection >= GetCandCount()) {
        dwSelection = 0;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// �O�̌�⃊�X�g�ցB
void LogCandList::MovePrev() {
    if (dwSelection > 0) {
        --dwSelection;
    } else {
        dwSelection = GetCandCount() - 1;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// �L�[�{�[�h��PageUp�L�[�̏����B
void LogCandList::PageUp() {
    if (dwPageStart >= dwPageSize) {
        dwSelection -= dwPageSize;
    } else {
        dwSelection = 0;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// �L�[�{�[�h��PageDown�L�[�̏����B
void LogCandList::PageDown() {
    if (dwPageStart + dwPageSize < GetCandCount()) {
        dwSelection += dwPageSize;
    } else {
        dwSelection = GetCandCount() - 1;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// �L�[�{�[�h��Home�L�[�̏����B
void LogCandList::MoveHome() {
    dwSelection = 0;
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// �L�[�{�[�h��End�L�[�̏����B
void LogCandList::MoveEnd() {
    dwSelection = GetCandCount() - 1;
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// ���̕�������擾����B
std::wstring LogCandList::GetString(DWORD iCand) const {
    return cand_strs[iCand];
}

// ���̕�������擾����B
std::wstring LogCandList::GetString() const {
    return GetString(dwSelection);
}

//////////////////////////////////////////////////////////////////////////////
// LogCandInfo - �����̘_���f�[�^�B

// �N���A�B
void LogCandInfo::clear() {
    cand_lists.clear();
    iClause = 0;
}

// ����񂪂��邩�H
BOOL LogCandInfo::HasCandInfo() const {
    return cand_lists.size() > 0;
}

// �߂̌��B
DWORD LogCandInfo::GetClauseCount() const {
    return DWORD(cand_lists.size());
}

// ����I������B
BOOL LogCandInfo::SelectCand(UINT uCandIndex) {
    DWORD dwPageStart = cand_lists[iClause].dwPageStart;
    if (dwPageStart + uCandIndex < cand_lists[iClause].dwPageSize) {
        cand_lists[iClause].dwSelection = dwPageStart + uCandIndex;
        return TRUE;
    }
    return FALSE;
}

// ���̌��ֈړ��B
void LogCandInfo::MoveNext() {
    cand_lists[iClause].MoveNext();
}

// �O�̌��ֈړ��B
void LogCandInfo::MovePrev() {
    cand_lists[iClause].MovePrev();
}

// �L�[�{�[�h��Home�L�[�̏����B
void LogCandInfo::MoveHome() {
    cand_lists[iClause].MoveHome();
}

// �L�[�{�[�h��End�L�[�̏����B
void LogCandInfo::MoveEnd() {
    cand_lists[iClause].MoveEnd();
}

// �L�[�{�[�h��PageUp�L�[�̏����B
void LogCandInfo::PageUp() {
    cand_lists[iClause].PageUp();
}

// �L�[�{�[�h��PageDown�L�[�̏����B
void LogCandInfo::PageDown() {
    cand_lists[iClause].PageDown();
}

// ��⍀�ڂ̕�����B
std::wstring LogCandInfo::GetString() const {
    return cand_lists[iClause].GetString(cand_lists[iClause].dwSelection);
}

// ��⍀�ڂ̕�����B
std::wstring LogCandInfo::GetString(DWORD iCand) const {
    return cand_lists[iClause].GetString(iCand);
}

// �����̕����f�[�^�̍��v�T�C�Y���v�Z����B
DWORD LogCandInfo::GetTotalSize() const {
    DWORD total = sizeof(CANDIDATEINFO);
    for (size_t i = 0; i < cand_lists.size(); ++i) {
        total += cand_lists[i].GetTotalSize();
    }
    total += sizeof(CANDINFOEXTRA);
    return total;
}

// �����̘_���f�[�^���_���v�B
void LogCandInfo::Dump() {
    DebugPrintA("LogCandInfo::Dump\n");
    for (size_t i = 0; i < cand_lists.size(); ++i) {
        DebugPrintA("### CandList %u ###\n", i);
        DebugPrintA("+ dwStyle: %08X", cand_lists[i].dwStyle);
        DebugPrintA("+ dwSelection: %08X", cand_lists[i].dwSelection);
        DebugPrintA("+ dwPageStart: %08X", cand_lists[i].dwPageStart);
        DebugPrintA("+ dwPageSize: %08X", cand_lists[i].dwPageSize);
        DebugPrintA("+ cand_strs: ");
        for (size_t k = 0; k < cand_lists[i].cand_strs.size(); ++k) {
            DebugPrintA("%ls ", cand_lists[i].cand_strs[k].c_str());
        }
        DebugPrintA("+ iClause: %u\n", iClause);
    }
} // LogCandInfo::Dump

//////////////////////////////////////////////////////////////////////////////
// CandList - ��⃊�X�g�̕����f�[�^�B

// �Ō�̃y�[�W���擾����B
DWORD CandList::GetPageEnd() const {
    DWORD dw = dwPageStart + dwPageSize;
    if (dw > dwCount) dw = dwCount;
    return dw;
}

// �����f�[�^����_���f�[�^�ցB
void CandList::GetLog(LogCandList& log) {
    log.dwStyle = dwStyle;
    log.dwSelection = dwSelection;
    log.dwPageStart = dwPageStart;
    log.dwPageSize = dwPageSize;
    log.cand_strs.clear();
    for (DWORD iCand = 0; iCand < dwCount; ++iCand) {
        log.cand_strs.push_back(GetCandString(iCand));
    }
}

// �_���f�[�^���畨���f�[�^�ցB
DWORD CandList::Store(const LogCandList *log) {
    dwSize = log->GetTotalSize();
    dwStyle = log->dwStyle;
    dwCount = DWORD(log->cand_strs.size());
    dwSelection = log->dwSelection;
    dwPageStart = log->dwPageStart;
    dwPageSize = log->dwPageSize;
    if (dwCount < dwPageSize) dwPageSize = dwCount;

    BYTE *pb = GetBytes();
    pb += sizeof(CANDIDATELIST);
    pb += dwCount * sizeof(DWORD);

    for (DWORD iCand = 0; iCand < dwCount; ++iCand) {
        dwOffset[iCand] = DWORD(pb - GetBytes());
        const std::wstring& str = log->cand_strs[iCand];
        DWORD cb = DWORD((str.size() + 1) * sizeof(WCHAR));
        memcpy(pb, &str[0], cb);
        pb += cb;
    }

    ASSERT(dwSize == DWORD(pb - GetBytes()));
    return DWORD(pb - GetBytes());
}

//////////////////////////////////////////////////////////////////////////////
// CandInfo - �����B

// ��⃊�X�g�̕����f�[�^���Q�Ƃ���B
CandList *CandInfo::GetList(DWORD i) {
    ASSERT(i < dwCount);
    return (CandList *)(GetBytes() + dwOffset[i]);
}

// �����̕����f�[�^����_���f�[�^�ցB
void CandInfo::GetLog(LogCandInfo& log) {
    log.clear(); // �_���f�[�^���N���A�B

    LogCandList cand; // ��⃊�X�g�̘_���f�[�^�B
    for (DWORD iList = 0; iList < dwCount; ++iList) {
        CandList *pList = GetList(iList);
        pList->GetLog(cand); // ��⃊�X�g�̘_���f�[�^���擾�B
        log.cand_lists.push_back(cand); // �_���f�[�^�Ɍ�⃊�X�g��ǉ��B
    }

    CANDINFOEXTRA *extra = GetExtra(); // �]������擾�B
    if (extra && extra->dwSignature == 0xDEADFACE) {
        log.iClause = extra->iClause; // ���݂̕��߂̃C���f�b�N�X�B
    } else {
        log.iClause = 0;
    }
}

// �����̘_���f�[�^���畨���f�[�^�ցB
DWORD CandInfo::Store(const LogCandInfo *log) {
    dwSize = log->GetTotalSize();
    dwCount = (DWORD)log->cand_lists.size();
    if (MAX_CANDLISTS < dwCount) {
        dwCount = MAX_CANDLISTS;
    }

    BYTE *pb = GetBytes();
    pb += sizeof(CANDIDATEINFO);

    for (DWORD iList = 0; iList < dwCount; ++iList) {
        dwOffset[iList] = DWORD(pb - GetBytes());
        CandList *pList = GetList(iList);
        pb += pList->Store(&log->cand_lists[iList]);
    }

    dwPrivateSize = sizeof(CANDINFOEXTRA);
    dwPrivateOffset = DWORD(pb - GetBytes());

    CANDINFOEXTRA *extra = (CANDINFOEXTRA *)pb;
    extra->dwSignature = 0xDEADFACE;
    extra->iClause = log->iClause;
    pb += sizeof(CANDINFOEXTRA);

    ASSERT(dwSize == DWORD(pb - GetBytes()));
    return DWORD(pb - GetBytes());
}

// �����̗]������擾����B
CANDINFOEXTRA *CandInfo::GetExtra() {
    if (dwPrivateSize >= sizeof(CANDINFOEXTRA)) {
        BYTE *pb = GetBytes();
        pb += dwPrivateOffset;
        CANDINFOEXTRA *extra = (CANDINFOEXTRA *)pb;
        if (extra->dwSignature == 0xDEADFACE) {
            return extra;
        } else {
            ASSERT(0);
        }
    }
    return NULL;
}

// �������č쐬����B
/*static*/ HIMCC CandInfo::ReCreate(HIMCC hCandInfo, const LogCandInfo *log) {
    LogCandInfo log_cand_info;
    if (log == NULL) {
        log = &log_cand_info;
    }

    const DWORD total = log->GetTotalSize();
    HIMCC hNewCandInfo = ::ImmReSizeIMCC(hCandInfo, total);
    if (hNewCandInfo) {
        CandInfo *cand_info = (CandInfo *)::ImmLockIMCC(hNewCandInfo);
        if (cand_info) {
            DWORD cb = cand_info->Store(log);
            ASSERT(cb == total);

            ImmUnlockIMCC(hNewCandInfo);
            hCandInfo = hNewCandInfo;
        } else {
            ASSERT(0);
        }
    } else {
        ASSERT(0);
    }
    return hCandInfo;
} // CandInfo::ReCreate

// �������_���v����B
void CandInfo::Dump() {
    DebugPrintA("### CandInfo ###\n");
    DebugPrintA("+ dwSize: %u\n", dwSize);
    DebugPrintA("+ dwCount: %u\n", dwCount);
    DebugPrintA("+ dwPrivateSize: %u\n", dwPrivateSize);
    DebugPrintA("+ dwPrivateOffset: %u\n", dwPrivateOffset);
    for (DWORD i = 0; i < dwCount; ++i) {
        DebugPrintA("+ List #%u\n", i);
        DebugPrintA("++ dwOffset: %08X\n", dwOffset[i]);
        CandList *list = GetList(i);
        DebugPrintA("++ dwSize: %u\n", list->dwSize);
        DebugPrintA("++ dwStyle: %08X\n", list->dwStyle);
        DebugPrintA("++ dwCount: %08X\n", list->dwCount);
        DebugPrintA("++ dwSelection: %08X\n", list->dwSelection);
        DebugPrintA("++ dwPageStart: %u\n", list->dwPageStart);
        DebugPrintA("++ dwPageSize: %u\n", list->dwPageSize);
        DebugPrintA("++ Cand Strings: ");
        for (DWORD k = 0; k < list->dwCount; ++k) {
            DebugPrintA("%ls ", list->GetCandString(k));
        }
        DebugPrintA("\n");
    }
} // CandInfo::Dump

//////////////////////////////////////////////////////////////////////////////
