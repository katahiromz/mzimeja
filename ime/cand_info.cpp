// cand_info.cpp --- candidate info of mzimeja
// 候補情報。
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

#define MAX_CANDLISTS   32
#define CANDPAGE_SIZE   8

//////////////////////////////////////////////////////////////////////////////
// LogCandList - 候補リストの論理データ。

// 候補情報のクリア。
void LogCandList::clear() {
    dwStyle = IME_CAND_READ;
    dwSelection = 0;
    dwPageStart = 0;
    dwPageSize = CANDPAGE_SIZE;
    cand_strs.clear();
}

// 候補リストの物理データの合計サイズを計算。
DWORD LogCandList::GetTotalSize() const {
    DWORD total = sizeof(CANDIDATELIST);
    total += DWORD(cand_strs.size() * sizeof(DWORD));
    for (size_t iCand = 0; iCand < cand_strs.size(); ++iCand) {
        total += DWORD((cand_strs[iCand].size() + 1) * sizeof(WCHAR));
    }
    return total;
}

// 候補の個数。
DWORD LogCandList::GetCandCount() const {
    return (DWORD)cand_strs.size();
}

// 次の候補リストへ。
void LogCandList::MoveNext() {
    ++dwSelection;
    if (dwSelection >= GetCandCount()) {
        dwSelection = 0;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// 前の候補リストへ。
void LogCandList::MovePrev() {
    if (dwSelection > 0) {
        --dwSelection;
    } else {
        dwSelection = GetCandCount() - 1;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// キーボードのPageUpキーの処理。
void LogCandList::PageUp() {
    if (dwPageStart >= dwPageSize) {
        dwSelection -= dwPageSize;
    } else {
        dwSelection = 0;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// キーボードのPageDownキーの処理。
void LogCandList::PageDown() {
    if (dwPageStart + dwPageSize < GetCandCount()) {
        dwSelection += dwPageSize;
    } else {
        dwSelection = GetCandCount() - 1;
    }
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// キーボードのHomeキーの処理。
void LogCandList::MoveHome() {
    dwSelection = 0;
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// キーボードのEndキーの処理。
void LogCandList::MoveEnd() {
    dwSelection = GetCandCount() - 1;
    dwPageStart = dwSelection / CANDPAGE_SIZE * CANDPAGE_SIZE;
}

// 候補の文字列を取得する。
std::wstring LogCandList::GetString(DWORD iCand) const {
    return cand_strs[iCand];
}

// 候補の文字列を取得する。
std::wstring LogCandList::GetString() const {
    return GetString(dwSelection);
}

//////////////////////////////////////////////////////////////////////////////
// LogCandInfo - 候補情報の論理データ。

// クリア。
void LogCandInfo::clear() {
    cand_lists.clear();
    iClause = 0;
}

// 候補情報があるか？
BOOL LogCandInfo::HasCandInfo() const {
    return cand_lists.size() > 0;
}

// 節の個数。
DWORD LogCandInfo::GetClauseCount() const {
    return DWORD(cand_lists.size());
}

// 候補を選択する。
BOOL LogCandInfo::SelectCand(UINT uCandIndex) {
    DWORD dwPageStart = cand_lists[iClause].dwPageStart;
    if (dwPageStart + uCandIndex < cand_lists[iClause].dwPageSize) {
        cand_lists[iClause].dwSelection = dwPageStart + uCandIndex;
        return TRUE;
    }
    return FALSE;
}

// 次の候補へ移動。
void LogCandInfo::MoveNext() {
    cand_lists[iClause].MoveNext();
}

// 前の候補へ移動。
void LogCandInfo::MovePrev() {
    cand_lists[iClause].MovePrev();
}

// キーボードのHomeキーの処理。
void LogCandInfo::MoveHome() {
    cand_lists[iClause].MoveHome();
}

// キーボードのEndキーの処理。
void LogCandInfo::MoveEnd() {
    cand_lists[iClause].MoveEnd();
}

// キーボードのPageUpキーの処理。
void LogCandInfo::PageUp() {
    cand_lists[iClause].PageUp();
}

// キーボードのPageDownキーの処理。
void LogCandInfo::PageDown() {
    cand_lists[iClause].PageDown();
}

// 候補項目の文字列。
std::wstring LogCandInfo::GetString() const {
    return cand_lists[iClause].GetString(cand_lists[iClause].dwSelection);
}

// 候補項目の文字列。
std::wstring LogCandInfo::GetString(DWORD iCand) const {
    return cand_lists[iClause].GetString(iCand);
}

// 候補情報の物理データの合計サイズを計算する。
DWORD LogCandInfo::GetTotalSize() const {
    DWORD total = sizeof(CANDIDATEINFO);
    for (size_t i = 0; i < cand_lists.size(); ++i) {
        total += cand_lists[i].GetTotalSize();
    }
    total += sizeof(CANDINFOEXTRA);
    return total;
}

// 候補情報の論理データをダンプ。
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
// CandList - 候補リストの物理データ。

// 最後のページを取得する。
DWORD CandList::GetPageEnd() const {
    DWORD dw = dwPageStart + dwPageSize;
    if (dw > dwCount) dw = dwCount;
    return dw;
}

// 物理データから論理データへ。
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

// 論理データから物理データへ。
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
// CandInfo - 候補情報。

// 候補リストの物理データを参照する。
CandList *CandInfo::GetList(DWORD i) {
    ASSERT(i < dwCount);
    return (CandList *)(GetBytes() + dwOffset[i]);
}

// 候補情報の物理データから論理データへ。
void CandInfo::GetLog(LogCandInfo& log) {
    log.clear(); // 論理データをクリア。

    LogCandList cand; // 候補リストの論理データ。
    for (DWORD iList = 0; iList < dwCount; ++iList) {
        CandList *pList = GetList(iList);
        pList->GetLog(cand); // 候補リストの論理データを取得。
        log.cand_lists.push_back(cand); // 論理データに候補リストを追加。
    }

    CANDINFOEXTRA *extra = GetExtra(); // 余剰情報を取得。
    if (extra && extra->dwSignature == 0xDEADFACE) {
        log.iClause = extra->iClause; // 現在の文節のインデックス。
    } else {
        log.iClause = 0;
    }
}

// 候補情報の論理データから物理データへ。
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

// 候補情報の余剰情報を取得する。
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

// 候補情報を再作成する。
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

// 候補情報をダンプする。
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
