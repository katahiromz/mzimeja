// comp_str.cpp --- composition string of mzimeja
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"

//////////////////////////////////////////////////////////////////////////////
// 未確定文字列の余剰情報の論理データ。

// 余剰情報の物理データの合計サイズ。
DWORD LogCompStrExtra::GetTotalSize() const
{
    DWORD total = sizeof(COMPSTREXTRA);
    for (size_t i = 0; i < hiragana_clauses.size(); ++i) {
        total += DWORD((hiragana_clauses[i].size() + 1) * sizeof(WCHAR));
    }
    for (size_t i = 0; i < typing_clauses.size(); ++i) {
        total += DWORD((typing_clauses[i].size() + 1) * sizeof(WCHAR));
    }
    return total;
}

// 余剰情報の論理データをクリア。
void LogCompStrExtra::clear()
{
    iClause = 0;
    hiragana_clauses.clear();
    typing_clauses.clear();
}

//////////////////////////////////////////////////////////////////////////////
// 未確定文字列の余剰情報の物理データ。

// ひらがな文節を取得する。
WCHAR *COMPSTREXTRA::GetHiraganaClauses(DWORD& dwCount)
{
    dwCount = dwHiraganaClauseCount;
    if (dwCount) {
        return (WCHAR *)(GetBytes() + dwHiraganaClauseOffset);
    }
    return NULL;
}

// 入力文節を取得する。
WCHAR *COMPSTREXTRA::GetTypingClauses(DWORD& dwCount)
{
    dwCount = dwTypingClauseCount;
    if (dwCount) {
        return (WCHAR *)(GetBytes() + dwTypingClauseOffset);
    }
    return NULL;
}

// 未確定文字列の余剰情報の物理データから論理データへ。
void COMPSTREXTRA::GetLog(LogCompStrExtra& log)
{
    log.clear(); // 論理データをクリア。
    log.iClause = iClause; // 文節を指定。

    DWORD dwCount;
    WCHAR *pch = GetHiraganaClauses(dwCount);
    if (pch && dwCount) {
        while (dwCount--) {
            log.hiragana_clauses.push_back(pch);
            pch += lstrlenW(pch) + 1;
        }
    }

    pch = GetTypingClauses(dwCount);
    if (pch && dwCount) {
        while (dwCount--) {
            log.typing_clauses.push_back(pch);
            pch += lstrlenW(pch) + 1;
        }
    }
} // COMPSTREXTRA::GetLog

// 未確定文字列の余剰情報の論理データから物理データへ。
DWORD COMPSTREXTRA::Store(const LogCompStrExtra *log)
{
    ASSERT(this);
    ASSERT(log);

    BYTE *pb = GetBytes();
    dwSignature = 0xDEADFACE;
    iClause = log->iClause;
    pb += sizeof(COMPSTREXTRA);

    DWORD size;

    dwHiraganaClauseOffset = (DWORD)(pb - GetBytes());
    dwHiraganaClauseCount = 0;
    for (size_t i = 0; i < log->hiragana_clauses.size(); ++i) {
        size = DWORD((log->hiragana_clauses[i].size() + 1) * sizeof(WCHAR));
        memcpy(pb, &log->hiragana_clauses[i][0], size);
        ++dwHiraganaClauseCount;
        pb += size;
    }

    dwTypingClauseOffset = (DWORD)(pb - GetBytes());
    dwTypingClauseCount = 0;
    for (size_t i = 0; i < log->typing_clauses.size(); ++i) {
        size = DWORD((log->typing_clauses[i].size() + 1) * sizeof(WCHAR));
        memcpy(pb, &log->typing_clauses[i][0], size);
        ++dwTypingClauseCount;
        pb += size;
    }

    ASSERT(log->GetTotalSize() == (DWORD)(pb - GetBytes()));
    return (DWORD)(pb - GetBytes());
} // COMPSTREXTRA::Store

//////////////////////////////////////////////////////////////////////////////
// 未確定文字列の論理データ。

// 未確定文字列のクリア。
void LogCompStr::clear()
{
    clear_read();
    clear_comp();
    clear_result();
    clear_extra();
    fix();
}

// 未確定文字列の補正。
void LogCompStr::fix()
{
    extra.comp_str_clauses.clear();
    size_t count = comp_clause.size();
    if (count >= 1) {
        std::wstring str;
        for (size_t i = 0; i < count - 1; ++i) {
            str = GetClauseCompString(DWORD(i));
            extra.comp_str_clauses.push_back(str);
        }
    } else {
        extra.comp_str_clauses.push_back(L"");
    }
    if (extra.hiragana_clauses.empty()) {
        extra.hiragana_clauses.push_back(L"");
    }
    if (extra.typing_clauses.empty()) {
        extra.typing_clauses.push_back(L"");
    }
} // LogCompStr::fix

// 読みがなをクリア。
void LogCompStr::clear_read()
{
    comp_read_attr.clear();
    comp_read_clause.clear();
    comp_read_str.clear();
}

// 未確定文字列をクリア。
void LogCompStr::clear_comp()
{
    dwCursorPos = 0;
    dwDeltaStart = 0;
    comp_attr.clear();
    comp_clause.clear();
    comp_str.clear();
}

// 変換結果をクリア。
void LogCompStr::clear_result()
{
    result_read_clause.clear();
    result_read_str.clear();
    result_clause.clear();
    result_str.clear();
}

// 文節の個数を取得。
DWORD LogCompStr::GetClauseCount() const
{
    if (comp_clause.size() < 2) return 0;
    return (DWORD)(comp_clause.size() - 1);
}

// 未確定文字列の物理データの合計サイズを取得。
DWORD LogCompStr::GetTotalSize() const
{
    size_t total = sizeof(COMPOSITIONSTRING);
    total += comp_read_attr.size() * sizeof(BYTE);
    total += comp_read_clause.size() * sizeof(DWORD);
    total += comp_read_str.size() * sizeof(WCHAR);
    total += comp_attr.size() * sizeof(BYTE);
    total += comp_clause.size() * sizeof(DWORD);
    total += comp_str.size() * sizeof(WCHAR);
    total += result_read_clause.size() * sizeof(DWORD);
    total += result_read_str.size() * sizeof(WCHAR);
    total += result_clause.size() * sizeof(DWORD);
    total += result_str.size() * sizeof(WCHAR);
    total += extra.GetTotalSize();
    return (DWORD)total;
}

// 変換中か？
BOOL LogCompStr::IsBeingConverted()
{
    for (size_t i = 0; i < comp_attr.size(); ++i) {
        if (comp_attr[i] != ATTR_INPUT) return TRUE;
    }
    return FALSE;
}

// 未確定文字列があるか？
BOOL LogCompStr::HasCompStr() const
{
    return comp_str.size() > 0;
}

// 指定した文字位置がインデックスiClauseの文節に含まれるか？
BOOL LogCompStr::CompCharInClause(
        DWORD iCompChar, DWORD iClause, BOOL bExcludeEnd /* = FALSE */) const {
    if (bExcludeEnd) {
        return (iClause < GetClauseCount() &&
                comp_clause[iClause] <= iCompChar &&
                iCompChar < comp_clause[iClause + 1]);
    } else {
        return (iClause < GetClauseCount() &&
                comp_clause[iClause] <= iCompChar &&
                iCompChar <= comp_clause[iClause + 1]);
    }
}

// 文節属性を取得。
BYTE LogCompStr::GetClauseAttr(DWORD dwClauseIndex) const
{
    BYTE ret = ATTR_INPUT;
    if (dwClauseIndex < GetClauseCount()) {
        DWORD ich = ClauseToCompChar(dwClauseIndex);
        ret = GetCompCharAttr(ich);
    }
    return ret;
}

// 文節属性を設定。
void LogCompStr::SetClauseAttr(DWORD dwClauseIndex, BYTE attr)
{
    if (dwClauseIndex < GetClauseCount()) {
        DWORD ich0 = comp_clause[dwClauseIndex];
        DWORD ich1 = comp_clause[dwClauseIndex + 1];
        for (DWORD i = ich0; i < ich1; ++i) {
            if (i < comp_attr.size()) {
                comp_attr[i] = attr;
            }
        }
    }
}

// 現在の文節が変換されているか？
BOOL LogCompStr::IsClauseConverted() const
{
    return IsClauseConverted(extra.iClause);
}

// 文節が変換されているか？
BOOL LogCompStr::IsClauseConverted(DWORD dwClauseIndex) const
{
    return GetClauseAttr(dwClauseIndex) != ATTR_INPUT;
}

// 現在の文節が選択されているか？
BOOL LogCompStr::HasClauseSelected() const
{
    return IsClauseConverted(extra.iClause);
}

// 未確定文字列の文字属性を取得する。
BYTE LogCompStr::GetCompCharAttr(DWORD ich) const
{
    BYTE ret = ATTR_INPUT;
    if (ich < (DWORD)comp_attr.size()) {
        ret = comp_attr[ich];
    }
    return ret;
}

// 未確定文字列の文字数を取得する。
DWORD LogCompStr::GetCompCharCount() const
{
    return (DWORD)comp_str.size();
}

// 文節インデックスから文字位置を返す。
DWORD LogCompStr::ClauseToCompChar(DWORD dwClauseIndex) const
{
    if (dwClauseIndex >= GetClauseCount()) return GetCompCharCount();
    return comp_clause[dwClauseIndex];
}

// 文字位置から文節インデックスを返す。
DWORD LogCompStr::CompCharToClause(DWORD iCompChar) const
{
    DWORD dwClauseIndex = 0;
    const DWORD cClause = GetClauseCount();
    for (DWORD iClause = 0; iClause <= cClause; ++iClause) {
        if (comp_clause[iClause] <= iCompChar) {
            dwClauseIndex = iClause;
        } else {
            break;
        }
    }
    return dwClauseIndex;
}

// 文節インデックスから未確定文字列を返す。
std::wstring LogCompStr::GetClauseCompString(DWORD dwClauseIndex) const
{
    std::wstring ret;
    if (dwClauseIndex + 1 <= GetClauseCount()) {
        DWORD ich0 = comp_clause[dwClauseIndex];
        DWORD ich1 = comp_clause[dwClauseIndex + 1];
        ret = comp_str.substr(ich0, ich1 - ich0);
    }
    return ret;
}

// 文字列を合併する。
void LogCompStr::MergeAt(std::vector<std::wstring>& strs, DWORD istr)
{
    strs[istr - 1] += strs[istr];
    strs.erase(strs.begin() + istr);
}

// 一つ前の文字位置が現在の文節に含まれるか？
WCHAR LogCompStr::PrevCharInClause() const
{
    if (dwCursorPos > 0) {
        if (CompCharInClause(dwCursorPos - 1, extra.iClause)) {
            return comp_str[dwCursorPos - 1];
        }
    }
    return 0;
}

// 余剰情報を更新する。
void LogCompStr::UpdateExtraClause(DWORD iClause, DWORD dwConversion)
{
    BOOL bRoman = (dwConversion & IME_CMODE_ROMAN);
    std::wstring str = extra.comp_str_clauses[iClause];
    str = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
    if (bRoman) {
        extra.typing_clauses[iClause] =
                lcmap(hiragana_to_roman(str), LCMAP_HALFWIDTH);
        str = fullwidth_ascii_to_halfwidth(str);
        str = roman_to_hiragana(str);
        str = translateString(str);
        extra.hiragana_clauses[iClause] = str;
    } else {
        extra.hiragana_clauses[iClause] = str;
        extra.typing_clauses[iClause] =
                lcmap(hiragana_to_typing(str), LCMAP_HALFWIDTH);
    }
} // LogCompStr::UpdateExtraClause

// 未確定文字列を更新する。
void LogCompStr::UpdateCompStr()
{
    std::wstring str;
    size_t ich = 0;
    size_t count = extra.comp_str_clauses.size();
    comp_clause.resize(count + 1);
    for (size_t i = 0; i < count; ++i) {
        comp_clause[i] = (DWORD)ich;
        str += extra.comp_str_clauses[i];
        ich += extra.comp_str_clauses[i].size();
    }
    comp_str = str;
    comp_clause[count] = (DWORD)ich;
}

// 余剰情報から未確定文字列を更新する。
void LogCompStr::UpdateFromExtra(BOOL bRoman)
{
    UpdateCompStr();
    extra.typing_clauses.resize(extra.hiragana_clauses.size());
    if (bRoman) {
        for (DWORD i = 0; i < GetClauseCount(); ++i) {
            std::wstring& hira = extra.hiragana_clauses[i];
            extra.typing_clauses[i] = lcmap(hiragana_to_typing(hira), LCMAP_HALFWIDTH);
        }
    } else {
        for (DWORD i = 0; i < GetClauseCount(); ++i) {
            std::wstring& hira = extra.hiragana_clauses[i];
            extra.typing_clauses[i] = lcmap(hiragana_to_roman(hira), LCMAP_HALFWIDTH);
        }
    }
    comp_attr.assign(comp_str.size(), ATTR_CONVERTED);
}

// 現在の文節をひらがなにする。
void LogCompStr::MakeHiragana()
{
    std::wstring str =
            lcmap(extra.hiragana_clauses[extra.iClause], LCMAP_HIRAGANA);
    SetClauseCompString(extra.iClause, str);
    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

// 現在の文節をカタカナにする。
void LogCompStr::MakeKatakana()
{
    std::wstring str =
            lcmap(extra.hiragana_clauses[extra.iClause], LCMAP_KATAKANA);
    SetClauseCompString(extra.iClause, str);
    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

// 現在の文節を半角にする。
void LogCompStr::MakeHankaku()
{
    std::wstring str = lcmap(
            extra.hiragana_clauses[extra.iClause],
            LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    SetClauseCompString(extra.iClause, str);
    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

// 現在の文節を全角英数にする。
void LogCompStr::MakeZenEisuu()
{
    std::wstring str =
            lcmap(extra.typing_clauses[extra.iClause], LCMAP_FULLWIDTH);
    SetClauseCompString(extra.iClause, str);
    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

// 現在の文節を半角英数にする。
void LogCompStr::MakeHanEisuu()
{
    std::wstring str =
            lcmap(extra.typing_clauses[extra.iClause], LCMAP_HALFWIDTH);
    SetClauseCompString(extra.iClause, str);
    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
}

// 末尾に文字を追加する。
void LogCompStr::AddCharToEnd(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv)
{
    BOOL bRoman = (dwConv & IME_CMODE_ROMAN);
    std::wstring str, typed, translated;
    typed += chTyped;
    translated += chTranslated;
    // カタカナはひらがなに直す。
    if (is_fullwidth_katakana(chTranslated)) {
        translated = lcmap(translated, LCMAP_HIRAGANA);
        chTranslated = translated[0];
    }
    int len = 0;
    INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConv);
    switch (imode) {
    case IMODE_FULL_HIRAGANA:
        if (is_hiragana(chTranslated)) {
            // set comp str and get delta length
            len = 1;
            chTranslated = translateChar(chTranslated);
            extra.comp_str_clauses[extra.iClause] += chTranslated;
            // set hiragana
            chTranslated = translateChar(chTranslated);
            extra.hiragana_clauses[extra.iClause] += chTranslated;
            // set typing
            if (chTyped == chTranslated) {
                if (bRoman) {
                    translated = fullwidth_ascii_to_halfwidth(translated);
                    translated = translateString(translated);
                    translated = hiragana_to_roman(translated);
                    extra.typing_clauses[extra.iClause] += translated;
                } else {
                    translated = hiragana_to_typing(translated);
                    extra.typing_clauses[extra.iClause] += translated;
                }
            } else {
                extra.typing_clauses[extra.iClause] += chTyped;
            }
        } else {
            // set comp str and get delta length
            str = extra.comp_str_clauses[extra.iClause];
            len = (int)str.size();
            str += chTyped;
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_hiragana(str, str.size());
            str = translateString(str);
            extra.comp_str_clauses[extra.iClause] = str;
            len = (int)str.size() - len;
            // set hiragana
            str = extra.hiragana_clauses[extra.iClause];
            str += chTyped;
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_hiragana(str, str.size());
            str = translateString(str);
            extra.hiragana_clauses[extra.iClause] = str;
            // set typing
            chTyped = translateChar(chTyped);
            extra.typing_clauses[extra.iClause] += chTyped;
        }
        break;
    case IMODE_FULL_KATAKANA:
        if (is_hiragana(chTranslated)) {
            // set comp str and get delta length
            len = 1;
            extra.comp_str_clauses[extra.iClause] += lcmap(translated, LCMAP_KATAKANA);
            // set hiragana
            extra.hiragana_clauses[extra.iClause] += chTranslated;
            // set typing
            if (chTyped == chTranslated) {
                if (bRoman) {
                    translated = fullwidth_ascii_to_halfwidth(translated);
                    translated = hiragana_to_roman(translated);
                    translated = translateString(translated);
                    extra.typing_clauses[extra.iClause] += translated;
                } else {
                    translated = hiragana_to_typing(translated);
                    translated = translateString(translated);
                    extra.typing_clauses[extra.iClause] += translated;
                }
            } else {
                extra.typing_clauses[extra.iClause] += typed;
            }
        } else {
            // set comp str and get delta length
            str = extra.comp_str_clauses[extra.iClause];
            len = (int)str.size();
            str += chTyped;
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_katakana(str, str.size());
            str = translateString(str);
            extra.comp_str_clauses[extra.iClause] = str;
            len = (int)str.size() - len;
            // set hiragana
            str = extra.hiragana_clauses[extra.iClause];
            str += chTyped;
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_hiragana(str, str.size());
            str = translateString(str);
            extra.hiragana_clauses[extra.iClause] = str;
            // set typing
            chTyped = translateChar(chTyped);
            extra.typing_clauses[extra.iClause] += chTyped;
        }
        break;
    case IMODE_FULL_ASCII:
        // set comp str and get delta length
        str = extra.comp_str_clauses[extra.iClause];
        len = (int)str.size();
        str += chTyped;
        str = lcmap(str, LCMAP_FULLWIDTH);
        extra.comp_str_clauses[extra.iClause] = str;
        len = (int)str.size() - len;
        // set hiragana
        str = extra.hiragana_clauses[extra.iClause];
        str += chTyped;
        str = fullwidth_ascii_to_halfwidth(str);
        str = roman_to_hiragana(str);
        extra.hiragana_clauses[extra.iClause] = str;
        // set typing
        extra.typing_clauses[extra.iClause] += chTyped;
        break;
    case IMODE_HALF_KANA:
        if (is_hiragana(chTranslated)) {
            // set comp str and get delta length
            len = (int)translated.size();
            extra.comp_str_clauses[extra.iClause] +=
                    lcmap(translated, LCMAP_KATAKANA | LCMAP_HALFWIDTH);
            // set hiragana
            extra.hiragana_clauses[extra.iClause] += chTranslated;
            // set typing
            if (chTyped == chTranslated) {
                if (bRoman) {
                    extra.typing_clauses[extra.iClause] +=
                            hiragana_to_roman(fullwidth_ascii_to_halfwidth(translated));
                } else {
                    extra.typing_clauses[extra.iClause] +=
                            hiragana_to_typing(translated);
                }
            } else {
                extra.typing_clauses[extra.iClause] +=
                        hiragana_to_typing(translated);
            }
        } else {
            // set comp str and get delta length
            str = extra.comp_str_clauses[extra.iClause];
            len = (int)str.size();
            str += chTyped;
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_halfwidth_katakana(str, str.size());
            extra.comp_str_clauses[extra.iClause] = str;
            len = (int)str.size() - len;
            // set hiragana
            str = extra.hiragana_clauses[extra.iClause];
            str += chTyped;
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_hiragana(str, str.size());
            extra.hiragana_clauses[extra.iClause] = str;
            // set typing
            extra.typing_clauses[extra.iClause] += chTyped;
        }
        break;
    case IMODE_HALF_ASCII:
    case IMODE_DISABLED:
        break;
    }
    dwCursorPos += len;
    UpdateCompStr();
} // LogCompStr::AddCharToEnd

// 文字を挿入する。
void LogCompStr::InsertChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv)
{
    std::wstring typed, translated;
    typed += chTyped;
    translated += chTranslated;
    DWORD dwIndexInClause = dwCursorPos - ClauseToCompChar(extra.iClause);
    // カタカナはひらがなに直す。
    if (is_fullwidth_katakana(chTranslated)) {
        translated = lcmap(translated, LCMAP_HIRAGANA);
        chTranslated = translated[0];
    }
    int len = 0;
    std::wstring str = extra.comp_str_clauses[extra.iClause];
    INPUT_MODE imode = InputModeFromConversionMode(TRUE, dwConv);
    switch (imode) {
    case IMODE_FULL_HIRAGANA:
        if (is_hiragana(chTranslated)) {
            str.insert(dwIndexInClause, translated);
            len = 1;
        } else {
            len = (int)str.size();
            str.insert(dwIndexInClause, typed);
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_hiragana(str);
            str = translateString(str);
            len = (int)str.size() - len;
        }
        break;
    case IMODE_FULL_KATAKANA:
        translated = lcmap(translated, LCMAP_KATAKANA);
        if (is_hiragana(chTranslated)) {
            str.insert(dwIndexInClause, translated);
            len = 1;
        } else {
            len = (int)str.size();
            str.insert(dwIndexInClause, typed);
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_katakana(str);
            str = translateString(str);
            len = (int)str.size() - len;
        }
        break;
    case IMODE_FULL_ASCII:
        len = (int)str.size();
        str.insert(dwIndexInClause, typed);
        str = lcmap(str, LCMAP_FULLWIDTH);
        len = (int)str.size() - len;
        break;
    case IMODE_HALF_KANA:
        translated = lcmap(translated, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
        if (is_hiragana(chTranslated)) {
            str.insert(dwIndexInClause, translated);
            len = 1;
        } else {
            len = (int)str.size();
            str.insert(dwIndexInClause, typed);
            str = fullwidth_ascii_to_halfwidth(str);
            str = roman_to_halfwidth_katakana(str);
            len = (int)str.size() - len;
        }
        break;
    case IMODE_HALF_ASCII:
    case IMODE_DISABLED:
        break;
    }
    extra.comp_str_clauses[extra.iClause] = str;
    dwCursorPos += len;
    UpdateCompStr();
    UpdateExtraClause(extra.iClause, dwConv);
} // LogCompStr::InsertChar

void
LogCompStr::AddDakuonChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv)
{
    DWORD dwIndexInClause = dwCursorPos - ClauseToCompChar(extra.iClause);
    std::wstring str = extra.comp_str_clauses[extra.iClause];
    if (dwIndexInClause - 1 < str.size()) {
        str[dwIndexInClause - 1] = chTranslated;
    }
    extra.comp_str_clauses[extra.iClause] = str;
    UpdateCompStr();
    UpdateExtraClause(extra.iClause, dwConv);
}

// 文字を追加する。
void LogCompStr::AddChar(WCHAR chTyped, WCHAR chTranslated, DWORD dwConv)
{
    size_t size0 = comp_str.size();
    WCHAR ch = PrevCharInClause();
    if (ch) ch = dakuon_shori(ch, chTranslated);
    if (ch) {
        chTyped = L'@';
        chTranslated = ch;
        AddDakuonChar(chTyped, chTranslated, dwConv);
    } else if (!CompCharInClause(dwCursorPos, extra.iClause, TRUE)) {
        AddCharToEnd(chTyped, chTranslated, dwConv);
    } else {
        InsertChar(chTyped, chTranslated, dwConv);
    }
    size_t size1 = comp_str.size();
    DWORD ich = ClauseToCompChar(extra.iClause);
    if (size0 < size1) {
        std::vector<BYTE> addition(size1 - size0);
        comp_attr.insert(comp_attr.begin() + ich, addition.begin(), addition.end());
    } else if (size1 < size0) {
        comp_attr.erase(comp_attr.begin() + ich,
                        comp_attr.begin() + ich + DWORD(size0 - size1));
    }
} // LogCompStr::AddChar

void LogCompStr::DeleteChar(BOOL bBackSpace /* = FALSE*/, DWORD dwConv)
{
    // is the current clause being converted?
    if (IsClauseConverted()) { // being converted
        // set hiragana string to current clause
        extra.comp_str_clauses[extra.iClause] =
                extra.hiragana_clauses[extra.iClause];
        UpdateCompStr();
        SetClauseAttr(extra.iClause, ATTR_INPUT);
        dwCursorPos = ClauseToCompChar(extra.iClause + 1);
    } else { // not being converted
        BOOL flag = FALSE;
        // is it back space?
        if (bBackSpace) { // back space
            if (CompCharInClause(dwCursorPos - 1, extra.iClause)) {
                --dwCursorPos; // move left
                flag = TRUE;
            }
        } else { // not back space
            if (CompCharInClause(dwCursorPos, extra.iClause, TRUE)) {
                flag = TRUE;
            }
        }
        if (flag) {
            // erase the character
            DWORD ich = ClauseToCompChar(extra.iClause);
            DWORD delta = dwCursorPos - ich;
            extra.comp_str_clauses[extra.iClause].erase(delta, 1);
            // update extra clause
            UpdateExtraClause(extra.iClause, dwConv);
            // update composition string
            UpdateCompStr();
            // update comp_attr
            comp_attr.erase(comp_attr.begin() + ich);
        }
    }
} // LogCompStr::DeleteChar

void LogCompStr::RevertText()
{
    // reset composition
    if (extra.iClause < GetClauseCount()) {
        // merge adjacent not converted clauses
        if (extra.iClause > 0) {
            if (!IsClauseConverted(extra.iClause - 1)) {
                MergeAt(extra.hiragana_clauses, extra.iClause);
                MergeAt(extra.typing_clauses, extra.iClause);
                MergeAt(extra.comp_str_clauses, extra.iClause);
                comp_clause.erase(comp_clause.begin() + extra.iClause);
                --extra.iClause;
            }
        }
        if (extra.iClause + 1 < GetClauseCount()) {
            if (!IsClauseConverted(extra.iClause + 1)) {
                MergeAt(extra.hiragana_clauses, extra.iClause + 1);
                MergeAt(extra.typing_clauses, extra.iClause + 1);
                MergeAt(extra.comp_str_clauses, extra.iClause + 1);
                comp_clause.erase(comp_clause.begin() + extra.iClause + 1);
            }
        }
        // compare old and new string
        std::wstring old_str = extra.comp_str_clauses[extra.iClause];
        std::wstring str = lcmap(extra.hiragana_clauses[extra.iClause],
                                 LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
        DWORD ich = ClauseToCompChar(extra.iClause);
        if (old_str.size() < str.size()) {
            size_t diff = str.size() - old_str.size();
            std::vector<BYTE> addition(diff, ATTR_CONVERTED);
            comp_attr.insert(
                    comp_attr.begin() + ich, addition.begin(), addition.end());
        } else if (old_str.size() > str.size()) {
            size_t diff = old_str.size() - str.size();
            comp_attr.erase(
                    comp_attr.begin() + ich, comp_attr.begin() + ich + diff);
        }
        // update composition string
        extra.comp_str_clauses[extra.iClause] = str;
        UpdateCompStr();
        // set cursor position
        dwCursorPos = ClauseToCompChar(extra.iClause + 1);
        // set delta start
        dwDeltaStart = ClauseToCompChar(extra.iClause);
        // untarget
        SetClauseAttr(extra.iClause, ATTR_INPUT);
    }
} // LogCompStr::RevertText

void LogCompStr::MakeResult()
{
    // setting result_read_clause and result_read_str
    result_read_str.clear();
    const size_t count = extra.hiragana_clauses.size();
    result_read_clause.resize(count + 1);
    for (size_t i = 0; i < count; ++i) {
        result_read_clause[i] = DWORD(result_read_str.size());
        result_read_str +=
                lcmap(extra.hiragana_clauses[i], LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    }
    result_read_clause[count] = DWORD(result_read_str.size());

    result_clause = comp_clause;
    result_str = comp_str;
    clear_read();
    clear_comp();
    clear_extra();
    fix();
}

// 左に移動。
BOOL LogCompStr::MoveLeft()
{
    if (IsClauseConverted()) { // 現在の文節が変換中か？
        // untarget
        SetClauseAttr(extra.iClause, ATTR_CONVERTED);
        // set the current clause
        if (extra.iClause > 0) {
            --extra.iClause;
        } else {
            extra.iClause = GetClauseCount() - 1;
        }
        // retarget
        SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
        // move cursor
        dwCursorPos = GetCompCharCount();
        return TRUE;
    } else {
        // move cursor
        if (dwCursorPos > 0) {
            --dwCursorPos;
            // went out of clause?
            if (!CompCharInClause(dwCursorPos, extra.iClause)) {
                // set the current clause
                extra.iClause = CompCharToClause(dwCursorPos);
                // is the clause converted?
                if (IsClauseConverted()) {
                    // retarget
                    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
                    // move cursor
                    dwCursorPos = GetCompCharCount();
                    return TRUE;
                } else {
                    // move cursor
                    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
                }
            }
        }
    }
    return FALSE;
} // LogCompStr::MoveLeft

// 右に移動。
BOOL LogCompStr::MoveRight()
{
    if (IsClauseConverted()) { // 現在の文節が変換中か？
        // untarget
        SetClauseAttr(extra.iClause, ATTR_CONVERTED);
        // set current clause
        ++extra.iClause;
        if (extra.iClause >= GetClauseCount()) { // exceeded
            extra.iClause = 0;
        }
        // retarget
        SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
        // move cursor
        dwCursorPos = GetCompCharCount();
        return TRUE;
    } else { // not being converted
        // move cursor
        if (dwCursorPos + 1 <= GetCompCharCount()) {
            ++dwCursorPos;
            // went out of clause?
            if (!CompCharInClause(dwCursorPos, extra.iClause)) {
                // set current clause
                extra.iClause = CompCharToClause(dwCursorPos);
                // is the clause converted?
                if (IsClauseConverted()) {
                    // retarget
                    SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
                    // move cursor
                    dwCursorPos = GetCompCharCount();
                    return TRUE;
                } else {
                    // move cursor
                    dwCursorPos = ClauseToCompChar(extra.iClause + 1);
                }
            }
        }
    }
    return FALSE;
} // LogCompStr::MoveRight

// 未確定文字列の先頭に移動。
BOOL LogCompStr::MoveHome()
{
    if (IsClauseConverted()) { // 現在の文節が変換中か？
        // untarget
        SetClauseAttr(extra.iClause, ATTR_CONVERTED);
        // set the current clause to first
        extra.iClause = 0;
        // retarget
        SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
        // move cursor
        dwCursorPos = GetCompCharCount();
        return TRUE;
    } else {
        dwCursorPos = 0; // キャレットを先頭に移動。
        return FALSE;
    }
} // LogCompStr::MoveHome

// 未確定文字列の末尾に移動。
BOOL LogCompStr::MoveEnd()
{
    if (IsClauseConverted()) { // 現在の文節が変換中か？
        // untarget
        SetClauseAttr(extra.iClause, ATTR_CONVERTED);
        // set the current clause to last
        extra.iClause = GetClauseCount() - 1;
        // retarget
        SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
        // move cursor
        dwCursorPos = GetCompCharCount();
        return TRUE;
    } else {
        dwCursorPos = GetCompCharCount(); // キャレットを末尾に移動。
        return FALSE;
    }
} // LogCompStr::MoveEnd

DWORD LogCompStr::GetClauseCompStrLen(DWORD dwClauseIndex) const
{
    return (DWORD)extra.comp_str_clauses[dwClauseIndex].size();
}

void LogCompStr::SetClauseCompString(DWORD iClause, std::wstring& str)
{
    if (iClause < GetClauseCount()) {
        // fix comp_attr
        std::wstring old_str = extra.comp_str_clauses[iClause];
        DWORD ich = ClauseToCompChar(iClause);
        if (old_str.size() < str.size()) {
            size_t diff = str.size() - old_str.size();
            std::vector<BYTE> addition(diff, ATTR_CONVERTED);
            comp_attr.insert(
                    comp_attr.begin() + ich, addition.begin(), addition.end());
        } else if (old_str.size() > str.size()) {
            size_t diff = old_str.size() - str.size();
            comp_attr.erase(
                    comp_attr.begin() + ich, comp_attr.begin() + ich + diff);
        }
        // update comp str
        extra.comp_str_clauses[iClause] = str;
        UpdateCompStr();
        // untarget
        SetClauseAttr(extra.iClause, ATTR_CONVERTED);
        extra.iClause = iClause;
        // set target
        SetClauseAttr(extra.iClause, ATTR_TARGET_CONVERTED);
        // update cursor pos
        dwCursorPos = (DWORD)comp_str.size();
    }
} // LogCompStr::SetClauseCompString

// 指定した文節にひらがなをセットする。
void LogCompStr::SetClauseCompHiragana(DWORD iClause, std::wstring& str)
{
    if (iClause < GetClauseCount()) {
        extra.hiragana_clauses[iClause] = str;
    }
}

// 指定した文節に入力文字列を指定する。
void LogCompStr::SetClauseCompHiragana(
        DWORD iClause, std::wstring& str, BOOL bRoman)
{
    if (iClause < GetClauseCount()) {
        if (bRoman) {
            extra.typing_clauses[iClause] =
                    hiragana_to_roman(fullwidth_ascii_to_halfwidth(str));
        } else {
            extra.typing_clauses[iClause] = hiragana_to_typing(str);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

// 未確定文字列の論理データから物理データを格納する。
DWORD CompStr::Store(const LogCompStr *log)
{
    const DWORD total = log->GetTotalSize();

    BYTE *pb = GetBytes();
    dwSize = total;
    dwCursorPos = log->dwCursorPos;
    dwDeltaStart = log->dwDeltaStart;
    pb += sizeof(COMPOSITIONSTRING);

#define ADD_BYTES(member) \
    if (log->member.size()) { \
        memcpy(pb, &log->member[0], log->member.size() * sizeof(BYTE)); \
        pb += log->member.size() * sizeof(BYTE); \
    }

#define ADD_DWORDS(member) \
    if (log->member.size()) { \
        memcpy(pb, &log->member[0], log->member.size() * sizeof(DWORD)); \
        pb += log->member.size() * sizeof(DWORD); \
    }

#define ADD_STRING(member) \
    if (log->member.size()) { \
        memcpy(pb, &log->member[0], log->member.size() * sizeof(WCHAR)); \
        pb += log->member.size() * sizeof(WCHAR); \
    }

    dwCompReadAttrOffset = DWORD(pb - GetBytes());
    dwCompReadAttrLen = DWORD(log->comp_read_attr.size() * sizeof(BYTE));
    ADD_BYTES(comp_read_attr);

    dwCompReadClauseOffset = DWORD(pb - GetBytes());
    dwCompReadClauseLen = DWORD(log->comp_read_clause.size() * sizeof(DWORD));
    ADD_DWORDS(comp_read_clause);

    dwCompReadStrOffset = DWORD(pb - GetBytes());
    dwCompReadStrLen = DWORD(log->comp_read_str.size());
    ADD_STRING(comp_read_str);

    dwCompAttrOffset = DWORD(pb - GetBytes());
    dwCompAttrLen = DWORD(log->comp_attr.size() * sizeof(BYTE));
    ADD_BYTES(comp_attr);

    dwCompClauseOffset = DWORD(pb - GetBytes());
    dwCompClauseLen = DWORD(log->comp_clause.size() * sizeof(DWORD));
    ADD_DWORDS(comp_clause);

    dwCompStrOffset = DWORD(pb - GetBytes());
    dwCompStrLen = DWORD(log->comp_str.size());
    ADD_STRING(comp_str);

    dwResultReadClauseOffset = DWORD(pb - GetBytes());
    dwResultReadClauseLen = DWORD(log->result_read_clause.size() * sizeof(DWORD));
    ADD_DWORDS(result_read_clause);

    dwResultReadStrOffset = DWORD(pb - GetBytes());
    dwResultReadStrLen = DWORD(log->result_read_str.size());
    ADD_STRING(result_read_str);

    dwResultClauseOffset = DWORD(pb - GetBytes());
    dwResultClauseLen = DWORD(log->result_clause.size() * sizeof(DWORD));
    ADD_DWORDS(result_clause);

    dwResultStrOffset = DWORD(pb - GetBytes());
    dwResultStrLen = DWORD(log->result_str.size());
    ADD_STRING(result_str);

    COMPSTREXTRA *pExtra = (COMPSTREXTRA *)pb;
    dwPrivateSize = log->extra.GetTotalSize();
    dwPrivateOffset = DWORD(pb - GetBytes());
    pb += pExtra->Store(&log->extra);

#undef ADD_BYTES
#undef ADD_DWORDS
#undef ADD_STRING
    ASSERT(DWORD(pb - GetBytes()) == total);

    return DWORD(pb - GetBytes());
} // CompStr::Store

// 未確定文字列の論理データを取得する。
void CompStr::GetLog(LogCompStr& log)
{
    log.dwCursorPos = dwCursorPos;
    log.dwDeltaStart = dwDeltaStart;
    log.comp_read_attr.assign(GetCompReadAttr(), GetCompReadAttr() + dwCompReadAttrLen);
    log.comp_read_clause.assign(GetCompReadClause(), GetCompReadClause() + dwCompReadClauseLen / sizeof(DWORD));
    log.comp_read_str.assign(GetCompReadStr(), dwCompReadStrLen);
    log.comp_attr.assign(GetCompAttr(), GetCompAttr() + dwCompAttrLen);
    log.comp_clause.assign(GetCompClause(), GetCompClause() + dwCompClauseLen / sizeof(DWORD));
    log.comp_str.assign(GetCompStr(), dwCompStrLen);
    log.result_read_clause.assign(GetResultReadClause(), GetResultReadClause() + dwResultReadClauseLen / sizeof(DWORD));
    log.result_read_str.assign(GetResultReadStr(), dwResultReadStrLen);
    log.result_clause.assign(GetResultClause(), GetResultClause() + dwResultClauseLen / sizeof(DWORD));
    log.result_str.assign(GetResultStr(), dwResultStrLen);
    COMPSTREXTRA *extra = GetExtra();
    if (extra && extra->dwSignature == 0xDEADFACE) {
        extra->GetLog(log.extra);
        log.fix();
    }
}

// 論理データから未確定文字列を再作成する。
/*static*/ HIMCC CompStr::ReCreate(HIMCC hCompStr, const LogCompStr *log) {
    LogCompStr log_comp_str;
    if (log == NULL) {
        log = &log_comp_str;
    }
    const DWORD total = log->GetTotalSize();
    HIMCC hNewCompStr = ::ImmReSizeIMCC(hCompStr, total);
    if (hNewCompStr) {
        CompStr *lpCompStr = (CompStr *)::ImmLockIMCC(hNewCompStr);
        if (lpCompStr) {
            DWORD size = lpCompStr->Store(log);
            ASSERT(size == total);

            ::ImmUnlockIMCC(hNewCompStr);
            hCompStr = hNewCompStr;
        } else {
            DPRINTA("CompStr::ReCreate: failed #2");
            ASSERT(0);
        }
    } else {
        DPRINTA("CompStr::ReCreate: failed");
        ASSERT(0);
    }
    return hCompStr;
} // CompStr::ReCreate

// 未確定文字列の余剰情報を取得する。
COMPSTREXTRA *CompStr::GetExtra()
{
    if (dwPrivateSize > sizeof(COMPSTREXTRA)) {
        BYTE *pb = GetBytes();
        pb += dwPrivateOffset;
        COMPSTREXTRA *pExtra = (COMPSTREXTRA *)pb;
        if (pExtra->dwSignature == 0xDEADFACE) {
            return pExtra;
        }
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// デバッグ用。

void LogCompStr::AssertValid()
{
    if (comp_attr.size()) {
        if (comp_attr.size() != comp_str.size()) {
            Dump();
            ASSERT(0);
        }
    }
    if (comp_clause.size()) {
        if (comp_clause[0] != 0) {
            Dump();
            ASSERT(0);
        }
        if (comp_clause[comp_clause.size() - 1] != GetCompCharCount()) {
            Dump();
            ASSERT(0);
        }
        if (extra.iClause > (DWORD)comp_clause.size()) {
            Dump();
            ASSERT(0);
        }
        for (size_t i = 1; i < comp_clause.size(); ++i) {
            if (comp_clause[i] > GetCompCharCount()) {
                Dump();
                ASSERT(0);
            }
            if (comp_clause[i - 1] > comp_clause[i]) {
                Dump();
                ASSERT(0);
            }
        }
    }
    if (result_read_clause.size()) {
        if (result_read_clause[0] != 0) {
            Dump();
            ASSERT(0);
        }
        for (size_t i = 1; i < result_read_clause.size(); ++i) {
            if (result_read_clause[i] > (DWORD)result_read_str.size()) {
                Dump();
                ASSERT(0);
            }
            if (result_read_clause[i - 1] > result_read_clause[i]) {
                Dump();
                ASSERT(0);
            }
        }
    }
    if (result_clause.size()) {
        if (result_clause[0] != 0) {
            Dump();
            ASSERT(0);
        }
        for (size_t i = 1; i < result_clause.size(); ++i) {
            if (result_clause[i] > (DWORD)result_str.size()) {
                Dump();
                ASSERT(0);
            }
            if (result_clause[i - 1] > result_clause[i]) {
                Dump();
                ASSERT(0);
            }
        }
    }
    if (extra.hiragana_clauses.size() != extra.typing_clauses.size()) {
        Dump();
        ASSERT(0);
    }
} // LogCompStr::AssertValid

// 未確定文字列のダンプ。
void CompStr::Dump()
{
    DPRINTA("dwSize: %08X\n", dwSize);
    DPRINTA("dwCursorPos: %08X\n", dwCursorPos);
    DPRINTA("dwDeltaStart: %08X\n", dwDeltaStart);
    DPRINTA("CompReadAttr: ");
    if (dwCompReadAttrLen) {
        BYTE *attrs = GetCompReadAttr();
        for (DWORD i = 0; i < dwCompReadAttrLen; ++i) {
            DPRINTA("%02X ", attrs[i]);
        }
    }
    DPRINTA("\n");
    DPRINTA("CompReadClause: ");
    if (dwCompReadClauseLen) {
        DWORD *clauses = GetCompReadClause();
        for (DWORD i = 0; i < dwCompReadClauseLen / 4; ++i) {
            DPRINTA("%08X ", clauses[i]);
        }
    }
    DPRINTA("\n");
    DPRINTA("CompReadStr: ");
    if (dwCompReadStrLen) {
        WCHAR *str = GetCompReadStr();
        DPRINTA("%ls", str);
    }
    DPRINTA("\n");
    DPRINTA("CompAttr: ");
    if (dwCompAttrLen) {
        BYTE *attrs = GetCompAttr();
        for (DWORD i = 0; i < dwCompAttrLen; ++i) {
            DPRINTA("%02X ", attrs[i]);
        }
    }
    DPRINTA("\n");
    DPRINTA("CompClause: ");
    if (dwCompClauseLen) {
        DWORD *clauses = GetCompClause();
        for (DWORD i = 0; i < dwCompClauseLen / 4; ++i) {
            DPRINTA("%08X ", clauses[i]);
        }
    }
    DPRINTA("\n");
    DPRINTA("CompStr: ");
    if (dwCompStrLen) {
        WCHAR *str = GetCompStr();
        DPRINTA("%ls", str);
    }
    DPRINTA("\n");
    DPRINTA("ResultReadClause: ");
    if (dwResultReadClauseLen) {
        DWORD *clauses = GetResultReadClause();
        for (DWORD i = 0; i < dwResultReadClauseLen / 4; ++i) {
            DPRINTA("%08X ", clauses[i]);
        }
    }
    DPRINTA("\n");
    DPRINTA("ResultReadStr: ");
    if (dwResultReadStrLen) {
        WCHAR *str = GetResultReadStr();
        DPRINTA("%ls", str);
    }
    DPRINTA("\n");
    DPRINTA("ResultClause: ");
    if (dwResultClauseLen) {
        DWORD *clauses = GetResultClause();
        for (DWORD i = 0; i < dwResultClauseLen / 4; ++i) {
            DPRINTA("%08X ", clauses[i]);
        }
    }
    DPRINTA("\n");
    DPRINTA("ResultStr: ");
    if (dwResultStrLen) {
        WCHAR *str = GetResultStr();
        DPRINTA("%ls", str);
    }
    DPRINTA("\n");
} // CompStr::Dump

// 未確定文字列の論理データをダンプ。
void LogCompStr::Dump()
{
    DPRINTA("### LogCompStr ###\n");
    DPRINTA("+ dwCursorPos: %08X\n", dwCursorPos);
    DPRINTA("+ dwDeltaStart: %08X\n", dwDeltaStart);

    DPRINTA("+ comp_read_attr: ");
    for (size_t i = 0; i < comp_read_attr.size(); ++i) {
        DPRINTA("%02X ", comp_read_attr[i]);
    }
    DPRINTA("\n");

    DPRINTA("+ comp_read_clause: ");
    for (size_t i = 0; i < comp_read_clause.size(); ++i) {
        DPRINTA("%08X ", comp_read_clause[i]);
    }
    DPRINTA("\n");

    DPRINTA("+ comp_read_str: %ls\n", comp_read_str.c_str());
    DPRINTA("+ comp_attr: ");
    for (size_t i = 0; i < comp_attr.size(); ++i) {
        DPRINTA("%02X ", comp_attr[i]);
    }
    DPRINTA("\n");

    DPRINTA("+ comp_clause: ");
    for (size_t i = 0; i < comp_clause.size(); ++i) {
        DPRINTA("%08X ", comp_clause[i]);
    }
    DPRINTA("\n");

    DPRINTA("+ comp_str: %ls\n", comp_str.c_str());

    DPRINTA("+ result_read_clause: ");
    for (size_t i = 0; i < result_read_clause.size(); ++i) {
        DPRINTA("%08X ", result_read_clause[i]);
    }
    DPRINTA("\n");

    DPRINTA("+ result_read_str: %ls\n", result_read_str.c_str());

    DPRINTA("+ result_clause: ");
    for (size_t i = 0; i < result_clause.size(); ++i) {
        DPRINTA("%08X ", result_clause[i]);
    }
    DPRINTA("\n");

    DPRINTA("+ result_str: %ls\n", result_str.c_str());

    DPRINTA("+ extra.iClause: %08X\n", extra.iClause);

    DPRINTA("+ extra.hiragana_clauses: ");
    for (size_t i = 0; i < extra.hiragana_clauses.size(); ++i) {
        DPRINTA("%ls ", extra.hiragana_clauses[i].c_str());
    }
    DPRINTA("\n");

    DPRINTA("+ extra.typing_clauses: ");
    for (size_t i = 0; i < extra.typing_clauses.size(); ++i) {
        DPRINTA("%ls ", extra.typing_clauses[i].c_str());
    }
    DPRINTA("\n");
} // LogCompStr::Dump

//////////////////////////////////////////////////////////////////////////////
