// dict_compile.cpp --- dictionary compiler of MZ-IME Japanese Input
/////////////////////////////////////////////////////////////////////////////
// (Japanese, UTF-8)

#define _CRT_SECURE_NO_WARNINGS
#include "../dict.hpp"
#include "../str.hpp"
#include <algorithm>
#include <cassert>

static const wchar_t s_hiragana_table[][5] = {
    // DAN_A, DAN_I, DAN_U, DAN_E, DAN_O
    {L'あ', L'い', L'う', L'え', L'お'},   // GYOU_A
    {L'か', L'き', L'く', L'け', L'こ'},   // GYOU_KA
    {L'が', L'ぎ', L'ぐ', L'げ', L'ご'},   // GYOU_GA
    {L'さ', L'し', L'す', L'せ', L'そ'},   // GYOU_SA
    {L'ざ', L'じ', L'ず', L'ぜ', L'ぞ'},   // GYOU_ZA
    {L'た', L'ち', L'つ', L'て', L'と'},   // GYOU_TA
    {L'だ', L'ぢ', L'づ', L'で', L'ど'},   // GYOU_DA
    {L'な', L'に', L'ぬ', L'ね', L'の'},   // GYOU_NA
    {L'は', L'ひ', L'ふ', L'へ', L'ほ'},   // GYOU_HA
    {L'ば', L'び', L'ぶ', L'べ', L'ぼ'},   // GYOU_BA
    {L'ぱ', L'ぴ', L'ぷ', L'ぺ', L'ぽ'},   // GYOU_PA
    {L'ま', L'み', L'む', L'め', L'も'},   // GYOU_MA
    {L'や', 0, L'ゆ', 0, L'よ'},           // GYOU_YA
    {L'ら', L'り', L'る', L'れ', L'ろ'},   // GYOU_RA
    {L'わ', 0, 0, 0, L'を'},               // GYOU_WA
    {L'ん', 0, 0, 0, 0},                   // GYOU_NN
};

std::unordered_map<WCHAR,Dan>   g_hiragana_to_dan;      // 母音写像。
std::unordered_map<WCHAR,Gyou>  g_hiragana_to_gyou;  // 子音写像。

// 写像を準備する。
void MakeLiteralMaps() {
    if (g_hiragana_to_gyou.size()) {
        return;
    }
    g_hiragana_to_gyou.clear();
    g_hiragana_to_dan.clear();
    const size_t count = _countof(s_hiragana_table);
    for (size_t i = 0; i < count; ++i) {
        for (size_t k = 0; k < 5; ++k) {
            g_hiragana_to_gyou[s_hiragana_table[i][k]] = (Gyou)i;
        }
        for (size_t k = 0; k < 5; ++k) {
            g_hiragana_to_dan[s_hiragana_table[i][k]] = (Dan)k;
        }
    }
} // MzIme::MakeLiteralMaps

// 全角カタカナか？
inline BOOL is_fullwidth_katakana(WCHAR ch) {
    if (0x30A0 <= ch && ch <= 0x30FF) return TRUE;
    switch (ch) {
    case 0x30FD: case 0x30FE: case 0x3099: case 0x309A: case 0x309B:
    case 0x309C: case 0x30FC:
        return TRUE;
    default:
        return FALSE;
    }
}

// 文字列の文字種を変換する。
std::wstring lcmap(const std::wstring& str, DWORD dwFlags) {
    WCHAR szBuf[1024];
    const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
    ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                   str.c_str(), -1, szBuf, 1024);
    return szBuf;
}

// エントリーを辞書形式にする。
bool MakeDictFormat(DictEntry& entry, const std::wstring& strBunrui)
{
    if (strBunrui.size() == 2) {
        if (strBunrui == L"名詞")            entry.bunrui = HB_MEISHI;
        else if (strBunrui == L"副詞")       entry.bunrui = HB_FUKUSHI;
        else if (strBunrui == L"漢語")       entry.bunrui = HB_KANGO;
        else return false;
    } else if (strBunrui.size() == 3) {
        if (strBunrui == L"連体詞")          entry.bunrui = HB_RENTAISHI;
        else if (strBunrui == L"接続詞")     entry.bunrui = HB_SETSUZOKUSHI;
        else if (strBunrui == L"感動詞")     entry.bunrui = HB_KANDOUSHI;
        else if (strBunrui == L"接頭辞")     entry.bunrui = HB_SETTOUJI;
        else if (strBunrui == L"接尾辞")     entry.bunrui = HB_SETSUBIJI;
        else if (strBunrui == L"格助詞")     entry.bunrui = HB_KAKU_JOSHI;
        else if (strBunrui == L"副助詞")     entry.bunrui = HB_FUKU_JOSHI;
        else if (strBunrui == L"終助詞")     entry.bunrui = HB_SHUU_JOSHI;
        else if (strBunrui == L"カンマ")     entry.bunrui = HB_COMMA;
        else if (strBunrui == L"記号類")     entry.bunrui = HB_SYMBOL;
        else return false;
    } else if (strBunrui.size() == 4) {
        if (strBunrui == L"い形容詞")        entry.bunrui = HB_IKEIYOUSHI;
        else if (strBunrui == L"な形容詞")   entry.bunrui = HB_NAKEIYOUSHI;
        else if (strBunrui == L"五段動詞")   entry.bunrui = HB_GODAN_DOUSHI;
        else if (strBunrui == L"一段動詞")   entry.bunrui = HB_ICHIDAN_DOUSHI;
        else if (strBunrui == L"カ変動詞")   entry.bunrui = HB_KAHEN_DOUSHI;
        else if (strBunrui == L"サ変動詞")   entry.bunrui = HB_SAHEN_DOUSHI;
        else if (strBunrui == L"接続助詞")   entry.bunrui = HB_SETSUZOKU_JOSHI;
        else if (strBunrui == L"ピリオド")   entry.bunrui = HB_PERIOD;
        else return false;
    } else if (strBunrui.size() == 5) {
        if (strBunrui == L"未然助動詞")      entry.bunrui = HB_MIZEN_JODOUSHI;
        else if (strBunrui == L"連用助動詞") entry.bunrui = HB_RENYOU_JODOUSHI;
        else if (strBunrui == L"終止助動詞") entry.bunrui = HB_SHUUSHI_JODOUSHI;
        else if (strBunrui == L"連体助動詞") entry.bunrui = HB_RENTAI_JODOUSHI;
        else if (strBunrui == L"仮定助動詞") entry.bunrui = HB_KATEI_JODOUSHI;
        else if (strBunrui == L"命令助動詞") entry.bunrui = HB_MEIREI_JODOUSHI;
        else return false;
    } else {
        return false;
    }

    size_t i;
    switch (entry.bunrui) {
    case HB_NAKEIYOUSHI: // 「な形容詞」
        // 終端の「な」を削る。
        i = entry.pre.size() - 1;
        if (entry.pre[i] == L'な')
            entry.pre.resize(i);
        i = entry.post.size() - 1;
        if (entry.post[i] == L'な')
            entry.post.resize(i);
        break;
    case HB_IKEIYOUSHI: // 「い形容詞」
        // 終端の「い」を削る。
        i = entry.pre.size() - 1;
        if (entry.pre[i] == L'い')
            entry.pre.resize(i);
        i = entry.post.size() - 1;
        if (entry.post[i] == L'い')
            entry.post.resize(i);
        break;
    case HB_ICHIDAN_DOUSHI: // 「一段動詞」
        // 終端の「る」を削る。
        if (entry.pre[entry.pre.size() - 1] == L'る')
            entry.pre.resize(entry.pre.size() - 1);
        if (entry.post[entry.post.size() - 1] == L'る')
            entry.post.resize(entry.post.size() - 1);
        break;
    case HB_KAHEN_DOUSHI: // 「カ変動詞」
        // 読みが３文字以上で「来る」「くる」で終わるとき、「来る」を削る。
        if (entry.pre.size() >= 3) {
            if (entry.pre.substr(entry.pre.size() - 2, 2) == L"くる" &&
                entry.post.substr(entry.post.size() - 2, 2) == L"来る")
            {
                entry.pre = entry.pre.substr(0, entry.pre.size() - 2);
                entry.post = entry.post.substr(0, entry.post.size() - 2);
            }
        }
        break;
    case HB_SAHEN_DOUSHI: // 「サ変動詞」
        entry.gyou = GYOU_SA;
        if (entry.pre.size() >= 2 && entry.post.size() >= 2) { // 三文字以上のとき。
            // 「する」「ずる」ならば「する」「ずる」を削る。
            if (entry.pre.substr(entry.pre.size() - 2) == L"する" && 
                entry.post.substr(entry.post.size() - 2) == L"する")
            {
                entry.gyou = GYOU_SA;
                entry.pre.resize(entry.pre.size() - 2);
                entry.post.resize(entry.post.size() - 2);
            }
            else if (entry.pre.substr(entry.pre.size() - 2) == L"ずる" &&
                     entry.post.substr(entry.post.size() - 2) == L"ずる")
            {
                entry.gyou = GYOU_ZA;
                entry.pre.resize(entry.pre.size() - 2);
                entry.post.resize(entry.post.size() - 2);
            }
        }
        break;
    case HB_GODAN_DOUSHI: // 「五段動詞」
        {
            // 終端の文字を取得する。
            WCHAR ch = entry.pre[entry.pre.size() - 1];
            // 終端の文字がウ段でなければ失敗。
            if (g_hiragana_to_dan[ch] != DAN_U)
                return false;
            // 終端の文字を削る。
            entry.pre.resize(entry.pre.size() - 1);
            entry.post.resize(entry.post.size() - 1);
            // 終端文字だったものの行を取得し、セットする。
            entry.gyou = g_hiragana_to_gyou[ch];
        }
        break;
    default:
        break;
    }

    return true;
}

// 辞書データファイルを読み込む。
BOOL LoadDictDataFile(const wchar_t *fname, std::vector<DictEntry>& entries)
{
    // ファイルを開く。
    FILE *fp = _wfopen(fname, L"rb");
    if (fp == NULL) {
        return FALSE;
    }

    // 一行ずつ処理する。
    int lineno = 0;
    char buf[256];
    wchar_t wbuf[256];
    while (fgets(buf, 256, fp) != NULL) {
        ++lineno;
        if (buf[0] == ';') continue;  // comment

        // convert to UTF-16
        ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);
        std::wstring str = wbuf;

        // split to fields
        str_trim_right(str, L"\r\n");
        WStrings fields;
        str_split(fields, str, L"\t");

        // is it an invalid line?
        if (fields.empty() || fields[I_FIELD_PRE].empty())
            continue;

        fields.resize(NUM_FIELDS);

        // fields[I_FIELD_HINSHI]が空だったら「名詞」にする。
        if (fields[I_FIELD_HINSHI].empty())
            fields[I_FIELD_HINSHI] = L"名詞";

        // fields[I_FIELD_POST]が空だったらfields[I_FIELD_PRE]をコピーする。
        if (fields[I_FIELD_POST].empty())
            fields[I_FIELD_POST] = fields[I_FIELD_PRE];

        // fields[I_FIELD_PRE]を全角ひらがなにする。
        fields[I_FIELD_PRE] = lcmap(fields[I_FIELD_PRE], LCMAP_FULLWIDTH | LCMAP_HIRAGANA);

        // 辞書にエントリーを追加する準備をする。
        DictEntry entry;
        std::wstring pre = entry.pre = fields[I_FIELD_PRE];
        entry.gyou = GYOU_A;
        std::wstring post = entry.post = fields[I_FIELD_POST];
        entry.tags = fields[I_FIELD_TAGS];

        // 辞書形式にする。
        if (!MakeDictFormat(entry, fields[I_FIELD_HINSHI]))
            continue;

        if (fields[I_FIELD_HINSHI] == L"サ変動詞") {
            pre = entry.pre;
            post = entry.post;
            if (entry.gyou == GYOU_ZA) {
                entry.pre = pre + L"ざ";
                entry.post = post + L"ざ";
                entries.push_back(entry);
                entry.pre = pre + L"じ";
                entry.post = post + L"じ";
                entries.push_back(entry);
                entry.pre = pre + L"ぜ";
                entry.post = post + L"ぜ";
                entries.push_back(entry);
                entry.pre = pre + L"ずる";
                entry.post = post + L"ずる";
                entries.push_back(entry);
                entry.pre = pre + L"ずれ";
                entry.post = post + L"ずれ";
                entries.push_back(entry);
                entry.pre = pre + L"じろ";
                entry.post = post + L"じろ";
                entries.push_back(entry);
                entry.pre = pre + L"ぜよ";
                entry.post = post + L"ぜよ";
                entries.push_back(entry);
                entry.pre = pre + L"じよう";
                entry.post = post + L"じよう";
                entries.push_back(entry);
            } else {
                entry.pre = pre + L"さ";
                entry.post = post + L"さ";
                entries.push_back(entry);
                entry.pre = pre + L"し";
                entry.post = post + L"し";
                entries.push_back(entry);
                entry.pre = pre + L"せ";
                entry.post = post + L"せ";
                entries.push_back(entry);
                entry.pre = pre + L"する";
                entry.post = post + L"する";
                entries.push_back(entry);
                entry.pre = pre + L"すれ";
                entry.post = post + L"すれ";
                entries.push_back(entry);
                entry.pre = pre + L"しろ";
                entry.post = post + L"しろ";
                entries.push_back(entry);
                entry.pre = pre + L"せよ";
                entry.post = post + L"せよ";
                entries.push_back(entry);
                entry.pre = pre + L"しよう";
                entry.post = post + L"しよう";
                entries.push_back(entry);
            }
        } else {
            // エントリーを追加する。
            entries.push_back(entry);
        }
    }

    // close the file
    fclose(fp);

    // sort by preconversion string
    std::sort(entries.begin(), entries.end(), [](const DictEntry& e1, const DictEntry& e2){
        return (e1.pre < e2.pre);
    });
    return TRUE;  // success
} // LoadDictDataFile

// コンパイル済みの辞書ファイルを作成する。
BOOL CreateDictFile(const wchar_t *fname, const std::vector<DictEntry>& entries)
{
    // calculate the total size
    size_t size = 0;
    size += 1;  // UTF-16 BOM
    size += 1;  // \n
    for (size_t i = 0; i < entries.size(); ++i) {
        const DictEntry& entry = entries[i];
        size += entry.pre.size();
        //size += 3;  // \t hb \t
        size += entry.post.size();
        //size += 1;  // \t
        size += entry.tags.size();
        //size += 1;  // \n
        size += 3 + 1 + 1;
    }
    size += 1;  // \0
    size *= sizeof(WCHAR);
    printf("size: %d\n", (INT)size);

    // メモリーブロックを確保する。
    void *pv = calloc(1, size);
    if (pv == NULL) {
        return FALSE;
    }

    WCHAR *pch = (WCHAR *)pv;
    *pch++ = 0xFEFF; // UTF-16 BOM
    *pch++ = RECORD_SEP;
    for (size_t i = 0; i < entries.size(); ++i) {
        // line format:
        // pre FIELD_SEP MAKEWORD(bunrui, gyou) FIELD_SEP post FIELD_SEP tags RECORD_SEP
        const DictEntry& entry = entries[i];
        // pre \t
        size_t cch = entry.pre.size();
        memcpy(pch, entry.pre.c_str(), cch * sizeof(WCHAR));
        pch += cch;
        *pch++ = FIELD_SEP;
        // MAKEWORD(bunrui, gyou) \t
        *pch++ = MAKEWORD(entry.bunrui, entry.gyou);
        *pch++ = FIELD_SEP;
        // post \t
        cch = entry.post.size();
        memcpy(pch, entry.post.c_str(), cch * sizeof(WCHAR));
        pch += cch;
        *pch++ = FIELD_SEP;
        // tags
        cch = entry.tags.size();
        memcpy(pch, entry.tags.c_str(), cch * sizeof(WCHAR));
        pch += cch;
        // new line
        *pch++ = RECORD_SEP;
    }
    *pch++ = L'\0'; // NUL
    assert(size / 2 == size_t(pch - reinterpret_cast<WCHAR *>(pv)));

    BOOL ret = FALSE;

    // コンパイル済みの辞書ファイルを作成する。
    HANDLE hFile = ::CreateFileW(fname, GENERIC_WRITE, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
        NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwWritten;
        ret = WriteFile(hFile, pv, DWORD(size), &dwWritten, NULL); // 書き込む。
        CloseHandle(hFile); // ファイルを閉じる。
    }

    free(pv);
    return ret;
} // CreateDictFile

extern "C"
int wmain(int argc, wchar_t **wargv) {
    // 引数の数を確認する。
    if (argc != 3) {
        printf("ERROR: missing parameters\n");
        return 1;
    }

    // 写像を準備する。
    MakeLiteralMaps();

    // 辞書を読み込んで、辞書形式のエントリ群を構築する。
    std::vector<DictEntry> entries;
    if (!LoadDictDataFile(wargv[1], entries)) {
        printf("ERROR: cannot load\n");
        return 2;
    }

    // バイナリ辞書を書き込む。
    if (!CreateDictFile(wargv[2], entries)) {
        printf("ERROR: cannot create\n");
        return 3;
    }

    printf("success.\n");

    return 0;
} // wmain
