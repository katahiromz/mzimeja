// convert.cpp --- mzimeja kana kanji conversion
// (Japanese, UTF-8)
// かな漢字変換。
//////////////////////////////////////////////////////////////////////////////
// 参考文献1：『自然言語処理の基礎』2010年、コロナ社。
// 参考文献2：『新編常用国語便覧』1995年、浜島書店。

#include "mzimeja.h"
#include "resource.h"
#include <algorithm>        // for std::sort

const DWORD c_dwMilliseconds = 8000;

// 辞書。
Dict g_basic_dict;
Dict g_name_dict;

// ひらがな表。品詞の活用で使用される。
static const WCHAR s_hiragana_table[][5] =
{
    // DAN_A, DAN_I, DAN_U, DAN_E, DAN_O
    {L'あ', L'い', L'う', L'え', L'お'}, // GYOU_A
    {L'か', L'き', L'く', L'け', L'こ'}, // GYOU_KA
    {L'が', L'ぎ', L'ぐ', L'げ', L'ご'}, // GYOU_GA
    {L'さ', L'し', L'す', L'せ', L'そ'}, // GYOU_SA
    {L'ざ', L'じ', L'ず', L'ぜ', L'ぞ'}, // GYOU_ZA
    {L'た', L'ち', L'つ', L'て', L'と'}, // GYOU_TA
    {L'だ', L'ぢ', L'づ', L'で', L'ど'}, // GYOU_DA
    {L'な', L'に', L'ぬ', L'ね', L'の'}, // GYOU_NA
    {L'は', L'ひ', L'ふ', L'へ', L'ほ'}, // GYOU_HA
    {L'ば', L'び', L'ぶ', L'べ', L'ぼ'}, // GYOU_BA
    {L'ぱ', L'ぴ', L'ぷ', L'ぺ', L'ぽ'}, // GYOU_PA
    {L'ま', L'み', L'む', L'め', L'も'}, // GYOU_MA
    {L'や',     0, L'ゆ',     0, L'よ'}, // GYOU_YA
    {L'ら', L'り', L'る', L'れ', L'ろ'}, // GYOU_RA
    {L'わ',     0,     0,     0, L'を'}, // GYOU_WA
    {L'ん',     0,     0,     0,     0}, // GYOU_NN
}; // ※ s_hiragana_table[GYOU_DA][DAN_U] のように使用する。

std::unordered_map<WCHAR,Dan>   g_hiragana_to_dan;  // 母音写像。
std::unordered_map<WCHAR,Gyou>  g_hiragana_to_gyou; // 子音写像。

// 子音の写像と母音の写像を作成する。
void MakeLiteralMaps()
{
    if (g_hiragana_to_gyou.size())
        return;
    g_hiragana_to_gyou.clear();
    g_hiragana_to_dan.clear();
    const size_t count = _countof(s_hiragana_table);
    for (size_t i = 0; i < count; ++i) {
        for (size_t k = 0; k < 5; ++k) {
            g_hiragana_to_gyou[ARRAY_AT_AT(s_hiragana_table, i, k)] = (Gyou)i;
        }
        for (size_t k = 0; k < 5; ++k) {
            g_hiragana_to_dan[ARRAY_AT_AT(s_hiragana_table, i, k)] = (Dan)k;
        }
    }
} // MakeLiteralMaps

// 品詞分類から文字列を取得する関数。
LPCTSTR HinshiToString(HinshiBunrui hinshi)
{
    if (HB_MEISHI <= hinshi && hinshi <= HB_MAX)
        return TheIME.LoadSTR(IDS_HINSHI_00 + (hinshi - HB_MEISHI));
    return TEXT("");
}

// 文字列から品詞分類を取得する関数。
HinshiBunrui StringToHinshi(LPCTSTR str)
{
    for (INT hinshi = HB_MEISHI; hinshi <= HB_MAX; ++hinshi) {
        LPCTSTR psz = HinshiToString((HinshiBunrui)hinshi);
        if (lstrcmpW(psz, str) == 0)
            return (HinshiBunrui)hinshi;
    }
    return HB_UNKNOWN;
}

// 品詞分類を文字列に変換する（デバッグ用）。
LPCWSTR BunruiToString(HinshiBunrui bunrui)
{
    int index = int(bunrui) - int(HB_HEAD);
    static const WCHAR *s_array[] = {
        L"HB_HEAD",
        L"HB_TAIL",
        L"HB_UNKNOWN",
        L"HB_MEISHI",
        L"HB_IKEIYOUSHI",
        L"HB_NAKEIYOUSHI",
        L"HB_RENTAISHI",
        L"HB_FUKUSHI",
        L"HB_SETSUZOKUSHI",
        L"HB_KANDOUSHI",
        L"HB_KAKU_JOSHI",
        L"HB_SETSUZOKU_JOSHI",
        L"HB_FUKU_JOSHI",
        L"HB_SHUU_JOSHI",
        L"HB_JODOUSHI",
        L"HB_MIZEN_JODOUSHI",
        L"HB_RENYOU_JODOUSHI",
        L"HB_SHUUSHI_JODOUSHI",
        L"HB_RENTAI_JODOUSHI",
        L"HB_KATEI_JODOUSHI",
        L"HB_MEIREI_JODOUSHI",
        L"HB_GODAN_DOUSHI",
        L"HB_ICHIDAN_DOUSHI",
        L"HB_KAHEN_DOUSHI",
        L"HB_SAHEN_DOUSHI",
        L"HB_KANGO",
        L"HB_SETTOUJI",
        L"HB_SETSUBIJI",
        L"HB_PERIOD",
        L"HB_COMMA",
        L"HB_SYMBOL"
    };
    ASSERT(index <= HB_MAX);
    return s_array[index];
} // BunruiToString

// 品詞の連結可能性。
static BOOL
IsNodeConnectable(const LatticeNode& node1, const LatticeNode& node2)
{
    if (node1.bunrui == HB_HEAD || node2.bunrui == HB_TAIL)
        return TRUE;
    if (node2.bunrui == HB_PERIOD || node2.bunrui == HB_COMMA)
        return TRUE;
    if (node1.bunrui == HB_SYMBOL || node2.bunrui == HB_SYMBOL)
        return TRUE;
    if (node1.bunrui == HB_UNKNOWN || node2.bunrui == HB_UNKNOWN)
        return TRUE;

    switch (node1.bunrui) {
    case HB_MEISHI: // 名詞
        switch (node2.bunrui) {
        case HB_SETTOUJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI: // い形容詞、な形容詞
        switch (node1.katsuyou) {
        case MIZEN_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[未然形に連結]")) {
                    if (node2.pre[0] == L'な' || node2.pre == L"う") {
                        return TRUE;
                    }
                }
            }
            return FALSE;
        case RENYOU_KEI:
            switch (node2.bunrui) {
            case HB_JODOUSHI:
                if (node2.HasTag(L"[連用形に連結]")) {
                    return TRUE;
                }
                return FALSE;
            case HB_MEISHI: case HB_SETTOUJI:
            case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
            case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
            case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
                return TRUE;
            default:
                return FALSE;
            }
            break;
        case SHUUSHI_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[終止形に連結]")) {
                    return TRUE;
                }
                if (node2.HasTag(L"[種々の語]")) {
                    return TRUE;
                }
            }
            if (node2.bunrui == HB_SHUU_JOSHI) {
                return TRUE;
            }
            return FALSE;
        case RENTAI_KEI:
            switch (node2.bunrui) {
            case HB_KANDOUSHI: case HB_JODOUSHI:
                return FALSE;
            default:
                return TRUE;
            }
        case KATEI_KEI:
            switch (node2.bunrui) {
            case HB_SETSUZOKU_JOSHI:
                if (node2.pre == L"ば" || node2.pre == L"ども" || node2.pre == L"ど") {
                    return TRUE;
                }
            default:
                break;
            }
            return FALSE;
        case MEIREI_KEI:
            switch (node2.bunrui) {
            case HB_SHUU_JOSHI: case HB_MEISHI: case HB_SETTOUJI:
                return TRUE;
            default:
                break;
            }
            return FALSE;
        }
        break;
    case HB_RENTAISHI: // 連体詞
        switch (node2.bunrui) {
        case HB_KANDOUSHI: case HB_JODOUSHI: case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_FUKUSHI: // 副詞
        switch (node2.bunrui) {
        case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
        case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_SETSUZOKUSHI: // 接続詞
        switch (node2.bunrui) {
        case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
        case HB_FUKU_JOSHI: case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_KANDOUSHI: // 感動詞
        switch (node2.bunrui) {
        case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
        case HB_FUKU_JOSHI: case HB_SETSUBIJI: case HB_JODOUSHI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
        // 終助詞以外の助詞
        switch (node2.bunrui) {
        case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_SHUU_JOSHI: // 終助詞
        switch (node2.bunrui) {
        case HB_MEISHI: case HB_SETTOUJI: case HB_SHUU_JOSHI:
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case HB_JODOUSHI: // 助動詞
        switch (node1.katsuyou) {
        case MIZEN_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[未然形に連結]")) {
                    return TRUE;
                }
            }
            return FALSE;
        case RENYOU_KEI:
            switch (node2.bunrui) {
            case HB_JODOUSHI:
                if (node2.HasTag(L"[連用形に連結]")) {
                    return TRUE;
                }
                return FALSE;
            case HB_MEISHI: case HB_SETTOUJI:
            case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
            case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
            case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
                return TRUE;
            default:
                return FALSE;
            }
            break;
        case SHUUSHI_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[終止形に連結]")) {
                    return TRUE;
                }
                if (node2.HasTag(L"[種々の語]")) {
                    return TRUE;
                }
            }
            if (node2.bunrui == HB_SHUU_JOSHI) {
                return TRUE;
            }
            return FALSE;
        case RENTAI_KEI:
            switch (node2.bunrui) {
            case HB_KANDOUSHI: case HB_JODOUSHI:
                return FALSE;
            default:
                return TRUE;
            }
        case KATEI_KEI:
            switch (node2.bunrui) {
            case HB_SETSUZOKU_JOSHI:
                if (node2.pre == L"ば" || node2.pre == L"ども" || node2.pre == L"ど") {
                    return TRUE;
                }
                break;
            default:
                break;
            }
            return FALSE;
        case MEIREI_KEI:
            switch (node2.bunrui) {
            case HB_SHUU_JOSHI: case HB_MEISHI: case HB_SETTOUJI:
                return TRUE;
            default:
                return FALSE;
            }
        }
        break;
    case HB_MIZEN_JODOUSHI: case HB_RENYOU_JODOUSHI:
    case HB_SHUUSHI_JODOUSHI: case HB_RENTAI_JODOUSHI:
    case HB_KATEI_JODOUSHI: case HB_MEIREI_JODOUSHI:
        ASSERT(0);
        break;
    case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
    case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
        // 動詞
        switch (node1.katsuyou) {
        case MIZEN_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[未然形に連結]")) {
                    return TRUE;
                }
            }
            return FALSE;
        case RENYOU_KEI:
            switch (node2.bunrui) {
            case HB_JODOUSHI:
                if (node2.HasTag(L"[連用形に連結]")) {
                    return TRUE;
                }
                return FALSE;
            case HB_MEISHI: case HB_SETTOUJI:
            case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
            case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
            case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
                return TRUE;
            default:
                return FALSE;
            }
            break;
        case SHUUSHI_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[終止形に連結]")) {
                    return TRUE;
                }
                if (node2.HasTag(L"[種々の語]")) {
                    return TRUE;
                }
            }
            if (node2.bunrui == HB_SHUU_JOSHI) {
                return TRUE;
            }
            return FALSE;
        case RENTAI_KEI:
            switch (node2.bunrui) {
            case HB_KANDOUSHI: case HB_JODOUSHI:
                return FALSE;
            default:
                return TRUE;
            }
            break;
        case KATEI_KEI:
            switch (node2.bunrui) {
            case HB_SETSUZOKU_JOSHI:
                if (node2.pre == L"ば" || node2.pre == L"ども" || node2.pre == L"ど") {
                    return TRUE;
                }
            default:
                break;
            }
            return FALSE;
        case MEIREI_KEI:
            switch (node2.bunrui) {
            case HB_SHUU_JOSHI: case HB_MEISHI: case HB_SETTOUJI:
                return TRUE;
            default:
                return FALSE;
            }
            break;
        }
        break;
    case HB_SETTOUJI: // 接頭辞
        switch (node2.bunrui) {
        case HB_MEISHI:
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case HB_SETSUBIJI: // 接尾辞
        switch (node2.bunrui) {
        case HB_SETTOUJI:
            return FALSE;
        default:
            break;
        }
        break;
    case HB_COMMA: case HB_PERIOD: // 、。
        switch (node2.bunrui) {
        case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
        case HB_SHUU_JOSHI: case HB_SETSUBIJI: case HB_JODOUSHI:
            return FALSE;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return TRUE;
} // IsNodeConnectable

// 単語コストの計算。
INT WordCost(const LatticeNode *ptr1)
{
    INT ret = 20;

    auto h = ptr1->bunrui;
    if (h == HB_MEISHI)
        ret += 30;
    else if (ptr1->IsJodoushi())
        ret += 10;
    else if (ptr1->IsDoushi())
        ret += 20;
    else if (ptr1->IsJoshi())
        ret += 10;
    else if (h == HB_SETSUZOKUSHI)
        ret += 10;
    else
        ret += 30;

    if (h == HB_KANGO)
        ret += 300;
    if (h == HB_SYMBOL)
        ret += 120;
    if (h == HB_GODAN_DOUSHI && ptr1->katsuyou == RENYOU_KEI)
        ret += 30;
    if (h == HB_SETTOUJI)
        ret += 200;

    if (ptr1->HasTag(L"[数単位]"))
        ret += 10;
    if (ptr1->HasTag(L"[非標準]"))
        ret += 100;
    if (ptr1->HasTag(L"[不謹慎]"))
        ret += 50;
    if (ptr1->HasTag(L"[人名]"))
        ret += 30;
    else if (ptr1->HasTag(L"[駅名]"))
        ret += 30;
    else if (ptr1->HasTag(L"[地名]"))
        ret += 30;
    if (ptr1->HasTag(L"[ユーザ辞書]"))
        ret -= 20;
    if (ptr1->HasTag(L"[優先1]"))
        ret -= 90;
    if (ptr1->HasTag(L"[優先2]"))
        ret -= 30;

    ret += ptr1->deltaCost;
    return ret;
}

// 連結コストの計算。
INT ConnectCost(const LatticeNode& n0, const LatticeNode& n1)
{
    auto h0 = n0.bunrui, h1 = n1.bunrui;
    if (h0 == HB_HEAD || h1 == HB_TAIL)
        return 0;
    if (h1 == HB_PERIOD || h1 == HB_COMMA)
        return 0;
    if (h0 == HB_SYMBOL || h1 == HB_SYMBOL)
        return 0;
    if (h0 == HB_UNKNOWN || h1 == HB_UNKNOWN)
        return 0;

    INT ret = 10;
    if (h0 == HB_MEISHI) {
        if (h1 == HB_MEISHI)
            ret += 10;
        if (h1 == HB_SETTOUJI)
            ret += 200;
        if (n1.IsDoushi())
            ret += 60;
        if (n1.IsKeiyoushi())
            ret += 20;
        if (n1.IsJodoushi())
            ret += 50;
    }
    if (n0.IsKeiyoushi()) {
        if (n1.IsKeiyoushi())
            ret += 10;
        if (n1.IsDoushi())
            ret += 50;
    }
    if (n0.IsDoushi()) {
        if (n1.IsJoshi())
            ret -= 5;
        if (n1.IsDoushi())
            ret += 20;
        if (n1.IsJodoushi())
            ret -= 10;
    }
    if (h0 == HB_SETSUZOKUSHI && n1.IsJoshi())
        ret += 5;
    if (h0 == HB_SETSUZOKUSHI && h1 == HB_MEISHI)
        ret += 5;
    if (h0 == HB_KANDOUSHI && h1 == HB_SHUU_JOSHI)
        ret += 300;
    return ret;
} // ConnectCost

// マーキングを最適化する。
BOOL Lattice::OptimizeMarking(LatticeNode *ptr0)
{
    ASSERT(ptr0);

    if (!ptr0->marked)
        return FALSE;

    BOOL reach = (ptr0->bunrui == HB_TAIL);
    INT min_cost = MAXLONG;
    LatticeNode *min_node = NULL;
    for (auto& ptr1 : ptr0->branches) {
        if (OptimizeMarking(ptr1.get())) {
            reach = TRUE;
            if (ptr1->subtotal_cost < min_cost) {
                min_cost = ptr1->subtotal_cost;
                min_node = ptr1.get();
            }
        }
    }

    for (auto& ptr1 : ptr0->branches) {
        if (ptr1.get() != min_node) {
            ptr1->marked = 0;
        }
    }

    if (!reach) {
        ptr0->marked = 0;
        return FALSE;
    }

    return TRUE;
} // Lattice::OptimizeMarking

// 基本辞書データをスキャンする。
static size_t ScanBasicDict(WStrings& records, const WCHAR *dict_data, WCHAR ch)
{
    DPRINTW(L"%c\n", ch);

    ASSERT(dict_data);

    if (ch == 0)
        return 0;

    // レコード区切りと文字chの組み合わせを検索する。
    // これで文字chで始まる単語を検索できる。
    // レコード群はソートされていると仮定。
    WCHAR sz[] = { RECORD_SEP, ch, 0 };
    const WCHAR *pch1 = wcsstr(dict_data, sz);
    if (pch1 == NULL)
        return FALSE;

    const WCHAR *pch2 = pch1; // 現在の位置。
    const WCHAR *pch3;
    for (;;) {
        // 現在の位置の次のレコード区切りと文字chの組み合わせを検索する。
        pch3 = wcsstr(pch2 + 1, sz);
        if (pch3 == NULL)
            break; // なければループ終わり。
        pch2 = pch3; // 現在の位置を更新。
    }
    pch3 = wcschr(pch2 + 1, RECORD_SEP); // 現在の位置の次のレコード区切りを検索する。
    if (pch3 == NULL)
        return FALSE;

    // 最初に発見したレコード区切りから最後のレコード区切りまでの文字列を取得する。
    std::wstring str;
    str.assign(pch1 + 1, pch3);

    // レコード区切りで分割してレコードを取得する。
    sz[0] = RECORD_SEP;
    sz[1] = 0;
    str_split(records, str, sz);
    ASSERT(records.size());
    return records.size();
} // ScanBasicDict

static WStrings s_UserDictRecords;

static INT CALLBACK UserDictProc(LPCTSTR lpRead, DWORD dwStyle, LPCTSTR lpStr, LPVOID lpData)
{
    ASSERT(lpStr && lpStr[0]);
    ASSERT(lpRead && lpRead[0]);
    Lattice *pThis = (Lattice *)lpData;
    ASSERT(pThis != NULL);

    // データの初期化。
    std::wstring pre = lpRead;
    std::wstring post = lpStr;
    Gyou gyou = GYOU_A;
    HinshiBunrui bunrui = StyleToHinshi(dwStyle);

    if (pre.size() <= 1)
        return 0;

    // データを辞書形式に変換する。
    WCHAR ch;
    size_t i;
    switch (bunrui) {
    case HB_NAKEIYOUSHI: // な形容詞
        // 終端の「な」を削る。
        i = pre.size() - 1;
        if (pre[i] == L'な')
            pre.resize(i);
        i = post.size() - 1;
        if (post[i] == L'な')
            post.resize(i);
        break;
    case HB_IKEIYOUSHI: // い形容詞
        // 終端の「い」を削る。
        i = pre.size() - 1;
        if (pre[i] == L'い')
            pre.resize(i);
        i = post.size() - 1;
        if (post[i] == L'い')
            post.resize(i);
        break;
    case HB_ICHIDAN_DOUSHI: // 一段動詞
        // 終端の「る」を削る。
        if (pre[pre.size() - 1] == L'る')
            pre.resize(pre.size() - 1);
        if (post[post.size() - 1] == L'る')
            post.resize(post.size() - 1);
        break;
    case HB_KAHEN_DOUSHI: // カ変動詞
        // 読みが３文字以上で「来る」「くる」で終わるとき、「来る」を削る。
        if (pre.size() >= 3) {
            if (pre.substr(pre.size() - 2, 2) == L"くる" &&
                post.substr(post.size() - 2, 2) == L"来る")
            {
                pre = pre.substr(0, pre.size() - 2);
                post = post.substr(0, post.size() - 2);
            }
        }
        break;
    case HB_SAHEN_DOUSHI: // サ変動詞
        {
            std::wstring substr;
            // 「する」「ずる」そのものは登録しない。
            if (pre == L"する" || pre == L"ずる")
                return TRUE;
            //  「する」または「ずる」で終わらなければ失敗。
            substr = pre.substr(pre.size() - 2, 2);
            if (substr == L"する" && post.substr(post.size() - 2, 2) == L"する")
                gyou = GYOU_SA;
            else if (substr == L"ずる" && post.substr(post.size() - 2, 2) == L"ずる")
                gyou = GYOU_ZA;
            else
                return TRUE;
            pre = pre.substr(0, pre.size() - 2);
            post = post.substr(0, post.size() - 2);
        }
        break;
    case HB_GODAN_DOUSHI: // 五段動詞
        // 写像を準備する。
        MakeLiteralMaps();
        // 終端がウ段の文字でなければ失敗。
        if (pre.empty())
            return TRUE;
        ch = pre[pre.size() - 1];
        if (g_hiragana_to_dan[ch] != DAN_U)
            return TRUE;
        if (ch != post[post.size() - 1])
            return TRUE;
        // 終端の文字を削る。
        pre.resize(pre.size() - 1);
        post.resize(post.size() - 1);
        // 終端の文字だったものの行を取得する。
        gyou = g_hiragana_to_gyou[ch];
        break;
    default:
        break;
    }

    WStrings fields(NUM_FIELDS);
    fields[I_FIELD_PRE] = pre;
    fields[I_FIELD_HINSHI] = { MAKEWORD(bunrui, gyou) };
    fields[I_FIELD_POST] = post;
    fields[I_FIELD_TAGS] = L"[ユーザ辞書]";

    std::wstring sep = { FIELD_SEP };
    std::wstring record = str_join(fields, sep);
    s_UserDictRecords.push_back(record);

    return TRUE;
}

// ユーザー辞書データをスキャンする。
static size_t ScanUserDict(WStrings& records, WCHAR ch, Lattice *pThis)
{
    DPRINTW(L"%c\n", ch);
    s_UserDictRecords.clear();
    ImeEnumRegisterWord(UserDictProc, NULL, 0, NULL, pThis);

    records.insert(records.end(), s_UserDictRecords.begin(), s_UserDictRecords.end());
    s_UserDictRecords.clear();
 
    return records.size();
}

//////////////////////////////////////////////////////////////////////////////
// Dict (dictionary) - 辞書データ。

// 辞書データのコンストラクタ。
Dict::Dict()
{
    m_hMutex = NULL;
    m_hFileMapping = NULL;
}

// 辞書データのデストラクタ。
Dict::~Dict()
{
    Unload();
}

// 辞書データファイルのサイズを取得する。
DWORD Dict::GetSize() const
{
    WIN32_FIND_DATAW find;
    HANDLE hFind = ::FindFirstFileW(m_strFileName.c_str(), &find);
    if (hFind != INVALID_HANDLE_VALUE) {
        ::FindClose(hFind);
        return find.nFileSizeLow;
    }
    return 0;
}

// 辞書を読み込む。
BOOL Dict::Load(const WCHAR *file_name, const WCHAR *object_name)
{
    if (IsLoaded())
        return TRUE; // すでに読み込み済み。

    m_strFileName = file_name;
    m_strObjectName = object_name;

    if (file_name == NULL)
        return FALSE;

    SECURITY_ATTRIBUTES *psa = CreateSecurityAttributes(); // セキュリティ属性を作成。
    ASSERT(psa);

    // ミューテックス (排他制御を行うオブジェクト) を作成。
    if (m_hMutex == NULL) {
        m_hMutex = ::CreateMutexW(psa, FALSE, m_strObjectName.c_str());
    }
    if (m_hMutex == NULL) {
        // free sa
        FreeSecurityAttributes(psa);
        return FALSE;
    }

    // ファイルサイズを取得。
    DWORD cbSize = GetSize();
    if (cbSize == 0)
        return FALSE;

    BOOL ret = FALSE;
    DWORD wait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds); // 排他制御を待つ。
    if (wait == WAIT_OBJECT_0) {
        // ファイルマッピングを作成する。
        m_hFileMapping = ::CreateFileMappingW(
                INVALID_HANDLE_VALUE, psa, PAGE_READWRITE,
                0, cbSize, (m_strObjectName + L"FileMapping").c_str());
        if (m_hFileMapping) {
            // ファイルマッピングが作成された。
            if (::GetLastError() == ERROR_ALREADY_EXISTS) {
                // ファイルマッピングがすでに存在する。
                ret = TRUE;
            } else {
                // 新しく作成された。ファイルを読み込む。
                FILE *fp = _wfopen(m_strFileName.c_str(), L"rb");
                if (fp) {
                    WCHAR *pch = Lock();
                    if (pch) {
                        ret = (BOOL)fread(pch, cbSize, 1, fp);
                        Unlock(pch);
                    }
                    fclose(fp);
                }
            }
        }
        // 排他制御を解放。
        ::ReleaseMutex(m_hMutex);
    }

    // free sa
    FreeSecurityAttributes(psa);

    return ret;
} // Dict::Load

// 辞書をアンロードする。
void Dict::Unload()
{
    if (m_hMutex) {
        if (m_hFileMapping) {
            DWORD wait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds); // 排他制御を待つ。
            if (wait == WAIT_OBJECT_0) {
                // ファイルマッピングを閉じる。
                if (m_hFileMapping) {
                    ::CloseHandle(m_hFileMapping);
                    m_hFileMapping = NULL;
                }
                // 排他制御を解放。
                ::ReleaseMutex(m_hMutex);
            }
        }
        // ミューテックスを閉じる。
        ::CloseHandle(m_hMutex);
        m_hMutex = NULL;
    }
}

// 辞書をロックして情報の取得を開始。
WCHAR *Dict::Lock()
{
    if (m_hFileMapping == NULL)
        return NULL;
    DWORD cbSize = GetSize();
    void *pv = ::MapViewOfFile(m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, cbSize);
    return reinterpret_cast<WCHAR *>(pv);
}

// 辞書のロックを解除して、情報の取得を終了。
void Dict::Unlock(WCHAR *data)
{
    ::UnmapViewOfFile(data);
}

// 辞書は読み込まれたか？
BOOL Dict::IsLoaded() const
{
    return (m_hMutex != NULL && m_hFileMapping != NULL);
}

//////////////////////////////////////////////////////////////////////////////
// MzConvResult, MzConvClause etc.

// 文節にノードを追加する。
void MzConvClause::add(const LatticeNode *node)
{
    for (auto& cand : candidates) {
        if (cand.post == node->post) {
            if (node->subtotal_cost < cand.cost) {
                cand.cost = node->subtotal_cost;
                cand.bunrui = node->bunrui;
                cand.katsuyou = node->katsuyou;
            }
            if (WordCost(node) < cand.word_cost) {
                cand.word_cost = WordCost(node);
            }
            cand.bunruis.insert(node->bunrui);
            cand.tags += node->tags;
            return;
        }
    }
    MzConvCandidate cand;
    cand.pre = node->pre;
    cand.post = node->post;
    cand.cost = node->subtotal_cost;
    cand.word_cost = WordCost(node);
    cand.bunruis.insert(node->bunrui);
    cand.bunrui = node->bunrui;
    cand.katsuyou = node->katsuyou;
    cand.tags = node->tags;
    candidates.push_back(cand);
}

// コストで候補をソートする。
void MzConvClause::sort()
{
    std::sort(candidates.begin(), candidates.end(), [](const MzConvCandidate& cand1, const MzConvCandidate& cand2){
        if (cand1.cost < cand2.cost)
            return true;
        if (cand1.cost > cand2.cost)
            return false;
        if (cand1.post < cand2.post)
            return true;
        if (cand1.post > cand2.post)
            return false;
        return false;
    });
    candidates.erase(std::unique(candidates.begin(), candidates.end(),
        [](const MzConvCandidate& cand1, const MzConvCandidate& cand2){
            return cand1.post == cand2.post;
        }),
        candidates.end()
    );
}

// コストで結果をソートする。
void MzConvResult::sort()
{
    for (auto& clause : clauses) {
        clause.sort();
    }
}

//////////////////////////////////////////////////////////////////////////////
// LatticeNode - ラティス（lattice）のノード。

// 動詞か？
bool LatticeNode::IsDoushi() const
{
    switch (bunrui) {
    case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
    case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
        return true;
    default:
        return false;
    }
}

// 助動詞か？
bool LatticeNode::IsJodoushi() const
{
    switch (bunrui) {
    case HB_JODOUSHI:
    case HB_MIZEN_JODOUSHI: case HB_RENYOU_JODOUSHI:
    case HB_SHUUSHI_JODOUSHI: case HB_RENTAI_JODOUSHI:
    case HB_KATEI_JODOUSHI: case HB_MEIREI_JODOUSHI:
        return true;
    default:
        return false;
    }
}

// 助詞か？
bool LatticeNode::IsJoshi() const
{
    switch (bunrui) {
    case HB_KAKU_JOSHI:
    case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI:
    case HB_SHUU_JOSHI:
        return true;
    default:
        return false;
    }
}

// 形容詞か？
bool LatticeNode::IsKeiyoushi() const
{
    return bunrui == HB_IKEIYOUSHI || bunrui == HB_NAKEIYOUSHI;
}

//////////////////////////////////////////////////////////////////////////////
// Lattice - ラティス

// 追加情報。
void Lattice::AddExtraNodes()
{
    FOOTMARK();
    static const LPCWSTR s_weekdays[] = {
        L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat"
    };
    static const LPCWSTR s_months[] = {
        L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun",
        L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"
    };

    WCHAR sz[128];

    // 現在の日時を取得する。
    SYSTEMTIME st;
    ::GetLocalTime(&st);

    // 今日（today）
    if (m_pre == L"きょう") {
        WStrings fields(NUM_FIELDS);
        fields[I_FIELD_PRE] = m_pre;
        fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };

        StringCchPrintfW(sz, _countof(sz), L"%u年%u月%u日", st.wYear, st.wMonth, st.wDay);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
        DoFields(0, fields, +10);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
        DoFields(0, fields, +10);

        StringCchPrintfW(sz, _countof(sz), L"%u年%02u月%02u日", st.wYear, st.wMonth, st.wDay);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
        DoFields(0, fields, +10);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
        DoFields(0, fields, +10);

        StringCchPrintfW(sz, _countof(sz), L"%04u-%02u-%02u", st.wYear, st.wMonth, st.wDay);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u/%02u/%02u", st.wYear, st.wMonth, st.wDay);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u/%u/%u", st.wYear, st.wMonth, st.wDay);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u.%02u.%02u", st.wYear, st.wMonth, st.wDay);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u.%u.%u", st.wYear, st.wMonth, st.wDay);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%02u/%02u/%04u", st.wMonth, st.wDay, st.wYear);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%u/%u/%04u", st.wMonth, st.wDay, st.wYear);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%s %s %02u %04u",
                         s_weekdays[st.wDayOfWeek], s_months[st.wMonth - 1],
                         st.wDay, st.wYear);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        return;
    }

    // 今年（this year）
    if (m_pre == L"ことし") {
        WStrings fields(NUM_FIELDS);
        fields[I_FIELD_PRE] = m_pre;
        fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };

        StringCchPrintfW(sz, _countof(sz), L"%u年", st.wYear);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
        DoFields(0, fields, +10);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
        DoFields(0, fields, +10);

        return;
    }

    // 今月（this month）
    if (m_pre == L"こんげつ") {
        WStrings fields(NUM_FIELDS);
        fields[I_FIELD_PRE] = m_pre;
        fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };

        StringCchPrintfW(sz, _countof(sz), L"%u年%u月", st.wYear, st.wMonth);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
        DoFields(0, fields, +10);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
        DoFields(0, fields, +10);

        StringCchPrintfW(sz, _countof(sz), L"%u年%02u月", st.wYear, st.wMonth);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
        DoFields(0, fields, +10);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
        DoFields(0, fields, +10);

        return;
    }

    // 現在の時刻（current time）
    if (m_pre == L"じこく" || m_pre == L"ただいま") {
        WStrings fields(NUM_FIELDS);
        fields[I_FIELD_PRE] = m_pre;
        fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };

        if (m_pre == L"ただいま") {
            fields[I_FIELD_POST] = L"ただ今";
            DoFields(0, fields);
            fields[I_FIELD_POST] = L"只今";
            DoFields(0, fields);
        }

        StringCchPrintfW(sz, _countof(sz), L"%u時%u分%u秒", st.wHour, st.wMinute, st.wSecond);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
        DoFields(0, fields, +10);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
        DoFields(0, fields, +10);

        StringCchPrintfW(sz, _countof(sz), L"%02u時%02u分%02u秒", st.wHour, st.wMinute, st.wSecond);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
        DoFields(0, fields, +10);

        fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
        DoFields(0, fields, +10);

        if (st.wHour >= 12) {
            StringCchPrintfW(sz, _countof(sz), L"午後%u時%u分%u秒", st.wHour - 12, st.wMinute, st.wSecond);
            fields[I_FIELD_POST] = sz;
            DoFields(0, fields);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
            DoFields(0, fields, +10);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
            DoFields(0, fields, +10);

            StringCchPrintfW(sz, _countof(sz), L"午後%02u時%02u分%02u秒", st.wHour - 12, st.wMinute, st.wSecond);
            fields[I_FIELD_POST] = sz;
            DoFields(0, fields);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
            DoFields(0, fields, +10);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
            DoFields(0, fields, +10);
        } else {
            StringCchPrintfW(sz, _countof(sz), L"午前%u時%u分%u秒", st.wHour, st.wMinute, st.wSecond);
            fields[I_FIELD_POST] = sz;
            DoFields(0, fields);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
            DoFields(0, fields, +10);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
            DoFields(0, fields, +10);

            StringCchPrintfW(sz, _countof(sz), L"午前%02u時%02u分%02u秒", st.wHour, st.wMinute, st.wSecond);
            fields[I_FIELD_POST] = sz;
            DoFields(0, fields);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief(sz);
            DoFields(0, fields, +10);

            fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(sz);
            DoFields(0, fields, +10);
        }

        StringCchPrintfW(sz, _countof(sz), L"%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);
        return;
    }

    if (m_pre == L"にちじ") { // date and time
        WStrings fields(NUM_FIELDS);
        fields[I_FIELD_PRE] = m_pre;
        fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };

        StringCchPrintfW(sz, _countof(sz), L"%u年%u月%u日%u時%u分%u秒",
                         st.wYear, st.wMonth, st.wDay,
                         st.wHour, st.wMinute, st.wSecond);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u年%02u月%02u日%02u時%02u分%02u秒",
                         st.wYear, st.wMonth, st.wDay,
                         st.wHour, st.wMinute, st.wSecond);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u-%02u-%02u %02u:%02u:%02u",
                         st.wYear, st.wMonth, st.wDay,
                         st.wHour, st.wMinute, st.wSecond);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%s %s %02u %04u %02u:%02u:%02u",
                         s_weekdays[st.wDayOfWeek], s_months[st.wMonth - 1],
                         st.wDay, st.wYear,
                         st.wHour, st.wMinute, st.wSecond);
        fields[I_FIELD_POST] = sz;
        DoFields(0, fields);
    }

    if (m_pre == L"じぶん") { // myself
        DWORD dwSize = _countof(sz);
        if (::GetUserNameW(sz, &dwSize)) {
            WStrings fields(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre;
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };
            fields[I_FIELD_POST] = sz;
            DoFields(0, fields);
        }
        return;
    }

    // カッコ (parens, brackets, braces, ...)
    if (m_pre == L"かっこ") {
        WStrings items;
        str_split(items, TheIME.LoadSTR(IDS_PAREN), std::wstring(L"\t"));

        WStrings fields(NUM_FIELDS);
        fields[I_FIELD_PRE] = m_pre;
        fields[I_FIELD_HINSHI] = { MAKEWORD(HB_SYMBOL, 0) };
        for (auto& item : items) {
            fields[I_FIELD_POST] = item;
            DoFields(0, fields);
        }
        return;
    }

    // 記号（symbols）
    static const WCHAR *s_words[] = {
        L"きごう",      // IDS_SYMBOLS
        L"けいせん",    // IDS_KEISEN
        L"けいさん",    // IDS_MATH
        L"さんかく",    // IDS_SANKAKU
        L"しかく",      // IDS_SHIKAKU
        L"ずけい",      // IDS_ZUKEI
        L"まる",        // IDS_MARU
        L"ほし",        // IDS_STARS
        L"ひし",        // IDS_HISHI
        L"てん",        // IDS_POINTS
        L"たんい",      // IDS_UNITS
        L"ふとうごう",  // IDS_FUTOUGOU
        L"たて",        // IDS_TATE
        L"たてひだり",  // IDS_TATE_HIDARI
        L"たてみぎ",    // IDS_TATE_MIGI
        L"ひだりうえ",  // IDS_HIDARI_UE
        L"ひだりした",  // IDS_HIDARI_SHITA
        L"ふとわく",    // IDS_FUTO_WAKU
        L"ほそわく",    // IDS_HOSO_WAKU
        L"まんなか",    // IDS_MANNAKA
        L"みぎうえ",    // IDS_MIGI_UE
        L"みぎした",    // IDS_MIGI_SHITA
        L"よこ",        // IDS_YOKO
        L"よこうえ",    // IDS_YOKO_UE
        L"よこした",    // IDS_YOKO_SHITA
        L"おなじ",      // IDS_SAME
        L"やじるし",    // IDS_ARROWS
        L"ぎりしゃ",    // IDS_GREEK
        L"うえ",        // IDS_UP
        L"した",        // IDS_DOWN
        L"ひだり",      // IDS_LEFT
        L"みぎ",        // IDS_RIGHT
    };
    for (size_t i = 0; i < _countof(s_words); ++i) {
        if (m_pre == s_words[i]) {
            WStrings items;
            WCHAR *pch = TheIME.LoadSTR(IDS_SYMBOLS + INT(i));
            WStrings fields(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre;
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_SYMBOL, 0) };
            int cost = 0;
            while (*pch) {
                fields[I_FIELD_POST].assign(1, *pch++);
                DoFields(0, fields, cost);
                ++cost;
            }
            return;
        }
    }
} // Lattice::AddExtraNodes

// 辞書からノード群を追加する。
BOOL Lattice::AddNodesFromDict(size_t index, const WCHAR *dict_data)
{
    FOOTMARK();
    const size_t length = m_pre.size();
    ASSERT(length);

    // フィールド区切り（separator）。
    std::wstring sep = { FIELD_SEP };

    WStrings fields, records;
    for (; index < length; ++index) {
        // periods (。。。)
        if (is_period(m_pre[index])) {
            size_t saved = index;
            do {
                ++index;
            } while (is_period(m_pre[index]));

            fields.resize(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre.substr(saved, index - saved);
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_PERIOD, 0) };
            switch (index - saved) {
            case 2:
                fields[I_FIELD_POST] += L'‥';
                break;
            case 3:
                fields[I_FIELD_POST] += L'…';
                break;
            default:
                fields[I_FIELD_POST] = fields[I_FIELD_PRE];
                break;
            }
            DoFields(saved, fields);
            --index;
            continue;
        }

        // center dots (・・・)
        if (m_pre[index] == L'・') {
            size_t saved = index;
            do {
                ++index;
            } while (m_pre[index] == L'・');

            fields.resize(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre.substr(saved, index - saved);
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_SYMBOL, 0) };
            switch (index - saved) {
            case 2:
                fields[I_FIELD_POST] += L'‥';
                break;
            case 3:
                fields[I_FIELD_POST] += L'…';
                break;
            default:
                fields[I_FIELD_POST] = fields[I_FIELD_PRE];
                break;
            }
            DoFields(saved, fields);
            --index;
            continue;
        }

        // commas (、、、)
        if (is_comma(m_pre[index])) {
            size_t saved = index;
            do {
                ++index;
            } while (is_comma(m_pre[index]));

            fields.resize(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre.substr(saved, index - saved);
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_COMMA, 0) };
            fields[I_FIELD_POST] = fields[I_FIELD_PRE];
            DoFields(saved, fields);
            --index;
            continue;
        }

        // arrow right (→)
        if (is_hyphen(m_pre[index]) && (m_pre[index + 1] == L'>' || m_pre[index + 1] == L'＞'))
        {
            fields.resize(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre.substr(index, 2);
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_SYMBOL, 0) };
            fields[I_FIELD_POST] = { L'→' };
            DoFields(index, fields);
            ++index;
            continue;
        }

        // arrow left (←)
        if ((m_pre[index] == L'<' || m_pre[index] == L'＜') && is_hyphen(m_pre[index + 1]))
        {
            fields.resize(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre.substr(index, 2);
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_SYMBOL, 0) };
            fields[I_FIELD_POST] = { L'←' };
            DoFields(index, fields);
            ++index;
            continue;
        }

        // arrows (zh, zj, zk, zl) and z. etc.
        WCHAR ch0 = translateChar(m_pre[index], FALSE, TRUE);
        if (ch0 == L'z' || ch0 == L'Z') {
            WCHAR ch1 = translateChar(m_pre[index + 1], FALSE, TRUE);
            WCHAR ch2 = 0;
            if (ch1 == L'h' || ch1 == L'H') ch2 = L'←'; // zh
            else if (ch1 == L'j' || ch1 == L'J') ch2 = L'↓'; // zj
            else if (ch1 == L'k' || ch1 == L'K') ch2 = L'↑'; // zk
            else if (ch1 == L'l' || ch1 == L'L') ch2 = L'→'; // zl
            else if (is_hyphen(ch1)) ch2 = L'～'; // z-
            else if (is_period(ch1)) ch2 = L'…'; // z.
            else if (is_comma(ch1)) ch2 = L'‥'; // z,
            else if (ch1 == L'[' || ch1 == L'［' || ch1 == L'「') ch2 = L'『'; // z[
            else if (ch1 == L']' || ch1 == L'］' || ch1 == L'」') ch2 = L'』'; // z]
            else if (ch1 == L'/' || ch1 == L'／') ch2 = L'・'; // z/
            if (ch2) {
                fields.resize(NUM_FIELDS);
                fields[I_FIELD_PRE] = m_pre.substr(index, 2);
                fields[I_FIELD_HINSHI] = { MAKEWORD(HB_SYMBOL, 0) };
                fields[I_FIELD_POST] = { ch2 };
                DoFields(index, fields, -100);
                ++index;
                continue;
            }
        }

        if (!is_hiragana(m_pre[index])) { // ひらがなではない？
            size_t saved = index;
            do {
                ++index;
            } while ((!is_hiragana(m_pre[index]) || is_hyphen(m_pre[index])) && m_pre[index]);

            fields.resize(NUM_FIELDS);
            fields[I_FIELD_PRE] = m_pre.substr(saved, index - saved);
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };
            fields[I_FIELD_POST] = fields[I_FIELD_PRE];
            DoMeishi(saved, fields);

            // 全部が数字なら特殊な変換を行う。
            if (are_all_chars_numeric(fields[I_FIELD_PRE])) {
                fields[I_FIELD_POST] = convert_to_kansuuji(fields[I_FIELD_PRE]);
                DoMeishi(saved, fields);
                fields[I_FIELD_POST] = convert_to_kansuuji_brief(fields[I_FIELD_PRE]);
                DoMeishi(saved, fields);
                fields[I_FIELD_POST] = convert_to_kansuuji_formal(fields[I_FIELD_PRE]);
                DoMeishi(saved, fields);
                fields[I_FIELD_POST] = convert_to_kansuuji_brief_formal(fields[I_FIELD_PRE]);
                DoMeishi(saved, fields);
            }

            // 郵便番号変換。
            std::wstring postal = normalize_postal_code(fields[I_FIELD_PRE]);
            if (postal.size()) {
                std::wstring addr = convert_postal_code(postal);
                if (addr.size()) {
                    fields[I_FIELD_POST] = addr;
                    DoMeishi(saved, fields, -10);
                }
            }

            --index;
            continue;
        }

        // 基本辞書をスキャンする。
        size_t count = ScanBasicDict(records, dict_data, m_pre[index]);
        DPRINTW(L"ScanBasicDict(%c) count: %d\n", m_pre[index], count);

        // ユーザー辞書をスキャンする。
        count = ScanUserDict(records, m_pre[index], this);
        DPRINTW(L"ScanUserDict(%c) count: %d\n", m_pre[index], count);

        // 各レコードをフィールドに分割し、処理する。
        for (auto& record : records) {
            str_split(fields, record, sep);
            DoFields(index, fields);
        }

        // special cases
        switch (m_pre[index]) {
        case L'さ': case L'し': case L'せ': case L'す': // SURU
            // サ変動詞。
            fields.resize(NUM_FIELDS);
            fields[I_FIELD_HINSHI] = { MAKEWORD(HB_SAHEN_DOUSHI, GYOU_SA) };
            DoSahenDoushi(index, fields);
            break;
        default:
            break;
        }
    }

    return TRUE;
} // Lattice::AddNodesFromDict

// 単一文節変換用のノード群を追加する。
BOOL Lattice::AddNodesFromDict(const WCHAR *dict_data)
{
    // 区切りを準備。
    std::wstring sep = { FIELD_SEP };

    // 基本辞書をスキャンする。
    WStrings fields, records;
    size_t count = ScanBasicDict(records, dict_data, m_pre[0]);
    DPRINTW(L"ScanBasicDict(%c) count: %d\n", m_pre[0], count);

    // ユーザー辞書をスキャンする。
    count = ScanUserDict(records, m_pre[0], this);
    DPRINTW(L"ScanUserDict(%c) count: %d\n", m_pre[0], count);

    // 各レコードをフィールドに分割して処理。
    for (auto& record : records) {
        str_split(fields, record, sep);
        DoFields(0, fields);
    }

    // 異なるサイズのノードを削除する。
    for (size_t i = 0; i < m_chunks[0].size(); ++i) {
        auto it = std::remove_if(m_chunks[0].begin(), m_chunks[0].end(), [this](const LatticeNodePtr& n){
            return n->pre.size() != m_pre.size();
        });
        m_chunks[0].erase(it, m_chunks[0].end());
    }

    return !m_chunks[0].empty();
}

// 部分最小コストを計算する。
INT Lattice::CalcSubTotalCosts(LatticeNode *ptr1)
{
    ASSERT(ptr1);

    if (ptr1->subtotal_cost != MAXLONG)
        return ptr1->subtotal_cost;

    INT min_cost = MAXLONG;
    LatticeNode *min_node = NULL;
    if (ptr1->reverse_branches.empty())
        min_cost = 0;

    for (auto& ptr0 : ptr1->reverse_branches) {
        INT word_cost = WordCost(ptr1);
        INT connect_cost = ConnectCost(*ptr0, *ptr1);
        INT cost = CalcSubTotalCosts(ptr0);
        cost += word_cost;
        cost += connect_cost;
        if (cost < min_cost) {
            min_cost = cost;
            min_node = ptr0;
        }
    }

    if (min_node) {
        min_node->marked = 1;
    }

    ptr1->subtotal_cost = min_cost;
    return min_cost;
} // Lattice::CalcSubTotalCosts

// リンクを更新する。
void Lattice::UpdateLinksAndBranches()
{
    ASSERT(m_pre.size());
    ASSERT(m_pre.size() + 1 == m_chunks.size());

    // リンク数とブランチ群をリセットする。
    ResetLatticeInfo();

    // ヘッド（頭）を追加する。リンク数は１。
    {
        LatticeNode node;
        node.bunrui = HB_HEAD;
        node.linked = 1;
        // 現在位置のノードを先頭ブランチに追加する。
        LatticeChunk& chunk1 = m_chunks[0];
        for (auto& ptr1 : chunk1) {
            ptr1->linked = 1;
            node.branches.push_back(ptr1);
        }
        m_head = std::make_shared<LatticeNode>(node);
    }

    // 尻尾（テイル）を追加する。
    {
        LatticeNode node;
        node.bunrui = HB_TAIL;
        m_tail = std::make_shared<LatticeNode>(node);
        m_chunks[m_pre.size()].clear();
        m_chunks[m_pre.size()].push_back(m_tail);
    }

    // 各インデックス位置について。
    for (size_t index = 0; index < m_pre.size(); ++index) {
        // インデックス位置のノード集合を取得。
        LatticeChunk& chunk1 = m_chunks[index];
        // 各ノードについて。
        for (auto& ptr1 : chunk1) {
            // リンク数がゼロならば無視。
            if (!ptr1->linked)
                continue;
            // 連結可能であれば、リンク先をブランチに追加し、リンク先のリンク数を増やす。
            auto& chunk2 = m_chunks[index + ptr1->pre.size()];
            for (auto& ptr2 : chunk2) {
                if (IsNodeConnectable(*ptr1.get(), *ptr2.get())) {
                    ptr1->branches.push_back(ptr2);
                    ptr2->linked++;
                }
            }
        }
    }
} // Lattice::UpdateLinksAndBranches

// リンク数とブランチ群をリセットする。
void Lattice::ResetLatticeInfo()
{
    for (size_t index = 0; index < m_pre.size(); ++index) {
        LatticeChunk& chunk1 = m_chunks[index];
        for (auto& ptr1 : chunk1) {
            ptr1->linked = 0;
            ptr1->branches.clear();
            ptr1->reverse_branches.clear();
        }
    }
} // Lattice::ResetLatticeInfo

// 変換失敗時に未定義の単語を追加する。
void Lattice::AddComplement()
{
    size_t lastIndex = GetLastLinkedIndex();
    if (lastIndex == m_pre.size())
        return;

    lastIndex += m_chunks[lastIndex][0]->pre.size();

    LatticeNode node;
    node.bunrui = HB_UNKNOWN;
    node.deltaCost = 0;
    node.pre = node.post = m_pre.substr(lastIndex);
    m_chunks[lastIndex].push_back(std::make_shared<LatticeNode>(node));
    UpdateLinksAndBranches();
} // Lattice::AddComplement

// 変換失敗時に未定義の単語を追加する。
void Lattice::AddComplement(size_t index, size_t min_size, size_t max_size)
{
    WStrings fields(NUM_FIELDS);
    fields[I_FIELD_HINSHI] = { MAKEWORD(HB_MEISHI, 0) };
    for (size_t count = min_size; count <= max_size; ++count) {
        if (m_pre.size() < index + count)
            continue;
        fields[I_FIELD_PRE] = m_pre.substr(index, count);
        fields[I_FIELD_POST] = fields[I_FIELD_PRE];
        DoFields(index, fields);
    }
} // Lattice::AddComplement

// リンクされていないノードを削除。
void Lattice::CutUnlinkedNodes()
{
    for (size_t index = 0; index < m_pre.size(); ++index) {
        auto& chunk1 = m_chunks[index];
        auto it = std::remove_if(chunk1.begin(), chunk1.end(), [](const LatticeNodePtr& node) {
            return node->linked == 0;
        });
        chunk1.erase(it, chunk1.end());
    }
} // Lattice::CutUnlinkedNodes

// 最後にリンクされたインデックスを取得する。
size_t Lattice::GetLastLinkedIndex() const
{
    // 最後にリンクされたノードがあるか？
    if (m_chunks[m_pre.size()][0]->linked) {
        return m_pre.size(); // 最後のインデックスを返す。
    }

    // チャンクを逆順でスキャンする。
    for (size_t index = m_pre.size(); index > 0; ) {
        --index;
        for (auto& ptr : m_chunks[index]) {
            if (ptr->linked) {
                return index; // リンクされたノードが見つかった。
            }
        }
    }
    return 0; // not found
} // Lattice::GetLastLinkedIndex

// イ形容詞を変換する。
void Lattice::DoIkeiyoushi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());
    size_t length = fields[I_FIELD_PRE].size();

    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_IKEIYOUSHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;

    // い形容詞の未然形。
    // 「痛い」→「痛かろ(う)」
    do {
        if (tail.empty() || tail.substr(0, 3) != L"かろう") break;
        node.katsuyou = MIZEN_KEI;
        node.pre = fields[I_FIELD_PRE] + L"かろう";
        node.post = fields[I_FIELD_POST] + L"かろう";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    MakeLiteralMaps();

    // い形容詞の連用形。
    // 「痛い」→「痛かっ(た)」
    do {
        if (tail.empty() || tail.substr(0, 2) != L"かっ") break;
        node.pre = fields[I_FIELD_PRE] + L"かっ";
        node.post = fields[I_FIELD_POST] + L"かっ";
        node.katsuyou = RENYOU_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 3 || tail[2] != L'た') break;
        node.pre += L'た';
        node.post += L'た';
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    // 「痛い」→「痛く(て)」、「広い」→「広く(て)」
    do {
        if (tail.empty() || tail[0] != L'く') break;
        node.pre = fields[I_FIELD_PRE] + L'く';
        node.post = fields[I_FIELD_POST] + L'く';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 2 || tail[1] != L'て') break;
        node.pre += tail[1];
        node.post += tail[1];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    // 「広い」→「広う(て)」
    do {
        if (tail.empty() || tail[0] != L'う') break;
        node.pre = fields[I_FIELD_PRE] + L'う';
        node.post = fields[I_FIELD_POST] + L'う';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    // 「美しい」→「美しゅう(て)」
    do {
        if (tail.empty() || tail[0] != L'ゅ' || tail[1] != L'う') break;
        node.pre = fields[I_FIELD_PRE] + L"ゅう";
        node.post = fields[I_FIELD_POST] + L"ゅう";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    // TODO: 「危ない」→「危のう(て)」
    // TODO: 「暖かい」→「暖こう(て)」

    // い形容詞の終止形。「かわいい」「かわいいよ」「かわいいね」「かわいいぞ」
    node.katsuyou = SHUUSHI_KEI;
    do {
        if (tail.empty() || tail[0] != L'い') break;
        node.pre = fields[I_FIELD_PRE] + L'い';
        node.post = fields[I_FIELD_POST] + L'い';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        if (tail.size() < 1 ||
            (tail[1] != L'よ' && tail[1] != L'ね' && tail[1] != L'な' && tail[1] != L'ぞ'))
                break;
        node.pre += tail[1];
        node.post += tail[1];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // い形容詞の連体形。
    // 「痛い」→「痛い(とき)」
    node.katsuyou = RENTAI_KEI;
    do {
        if (tail.empty() || tail[0] != L'い') break;
        node.pre = fields[I_FIELD_PRE] + L'い';
        node.post = fields[I_FIELD_POST] + L'い';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    // 「痛い」→「痛き(とき)」
    do {
        if (tail.empty() || tail[0] != L'き') break;
        node.pre = fields[I_FIELD_PRE] + L'き';
        node.post = fields[I_FIELD_POST] + L'き';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // い形容詞の仮定形。
    // 「痛い」→「痛けれ(ば)」
    do {
        if (tail.empty() || tail.substr(0, 2) != L"けれ") break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[I_FIELD_PRE] + L"けれ";
        node.post = fields[I_FIELD_POST] + L"けれ";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        if (tail.empty() || tail.substr(0, 3) != L"ければ") break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[I_FIELD_PRE] + L"ければ";
        node.post = fields[I_FIELD_POST] + L"ければ";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // い形容詞の名詞形。
    // 「痛い(い形容詞)」→「痛さ(名詞)」、
    // 「痛い(い形容詞)」→「痛み(名詞)」、
    // 「痛い(い形容詞)」→「痛め(名詞)」「痛目(名詞)」など。
    node.bunrui = HB_MEISHI;
    do {
        if (tail.empty() || tail[0] != L'さ') break;
        node.pre = fields[I_FIELD_PRE] + L'さ';
        node.post = fields[I_FIELD_POST] + L'さ';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (tail.empty() || tail[0] != L'み') break;
        node.pre = fields[I_FIELD_PRE] + L'み';
        node.post = fields[I_FIELD_POST] + L'み';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (tail.empty() || tail[0] != L'め') break;
        node.pre = fields[I_FIELD_PRE] + L'め';
        node.post = fields[I_FIELD_POST] + L'め';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.post = fields[I_FIELD_POST] + L'目';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 「痛い(い形容詞)」→「痛そうな(な形容詞)」など。
    if (tail.size() >= 2 && tail[0] == L'そ' && tail[1] == L'う') {
        WStrings new_fields = fields;
        new_fields[I_FIELD_PRE] = fields[I_FIELD_PRE] + L"そう";
        new_fields[I_FIELD_POST] = fields[I_FIELD_POST] + L"そう";
        DoNakeiyoushi(index, new_fields, deltaCost);
    }

    // 「痛い(い形容詞)」→「痛すぎる(一段動詞)」
    if (tail.size() >= 2 && tail[0] == L'す' && tail[1] == L'ぎ') {
        WStrings new_fields = fields;
        new_fields[I_FIELD_PRE] = fields[I_FIELD_PRE] + L"すぎ";
        new_fields[I_FIELD_POST] = fields[I_FIELD_POST] + L"すぎ";
        DoIchidanDoushi(index, new_fields, deltaCost);
        new_fields[I_FIELD_POST] = fields[I_FIELD_POST] + L"過ぎ";
        DoIchidanDoushi(index, new_fields, deltaCost);
    }

    // 「痛。」「寒。」など
    if (tail.empty()) {
        DoMeishi(index, fields, deltaCost);
    } else {
        switch (tail[0]) {
        case L'。': case L'、': case L'，': case L'．': case L'.': case L',':
            DoMeishi(index, fields, deltaCost);
            break;
        }
    }
} // Lattice::DoIkeiyoushi

// ナ形容詞を変換する。
void Lattice::DoNakeiyoushi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());
    size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_NAKEIYOUSHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;

    // な形容詞の未然形。
    // 「巨大な」→「巨大だろ(う)」
    do {
        if (tail.empty() || tail.substr(0, 3) != L"だろう") break;
        node.katsuyou = MIZEN_KEI;
        node.pre = fields[I_FIELD_PRE] + L"だろう";
        node.post = fields[I_FIELD_POST] + L"だろう";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        if (tail.empty() || tail.substr(0, 4) != L"だろうに") break;
        node.pre = fields[I_FIELD_PRE] + L"だろうに";
        node.post = fields[I_FIELD_POST] + L"だろうに";
        node.bunrui = HB_FUKUSHI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        node.bunrui = HB_NAKEIYOUSHI;
    } while (0);

    // な形容詞の連用形。
    // 「巨大な」→「巨大だっ(た)」
    do {
        if (tail.empty() || tail.substr(0, 2) != L"だっ") break;
        node.pre = fields[I_FIELD_PRE] + L"だっ";
        node.post = fields[I_FIELD_POST] + L"だっ";
        node.katsuyou = RENYOU_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 3 || tail[2] != L'た') break;
        node.pre += L'た';
        node.post += L'た';
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    node.katsuyou = RENYOU_KEI;
    do {
        if (tail.empty() || tail[0] != L'で') break;
        node.pre = fields[I_FIELD_PRE] + L'で';
        node.post = fields[I_FIELD_POST] + L'で';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (tail.empty() || tail[0] != L'に') break;
        node.pre = fields[I_FIELD_PRE] + L'に';
        node.post = fields[I_FIELD_POST] + L'に';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // な形容詞の終止形。
    // 「巨大な」→「巨大だ」「巨大だね」「巨大だぞ」
    do {
        if (tail.empty() || tail[0] != L'だ') break;
        node.katsuyou = SHUUSHI_KEI;
        node.pre = fields[I_FIELD_PRE] + L'だ';
        node.post = fields[I_FIELD_POST] + L'だ';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 1 ||
            (tail[1] != L'よ' && tail[1] != L'ね' && tail[1] != L'な' && tail[1] != L'ぞ'))
            break;
        node.pre += tail[1];
        node.post += tail[1];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // な形容詞の連体形。
    // 「巨大な」→「巨大な(とき)」
    do {
        if (tail.empty() || tail[0] != L'な') break;
        node.katsuyou = RENTAI_KEI;
        node.pre = fields[I_FIELD_PRE] + L'な';
        node.post = fields[I_FIELD_POST] + L'な';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // な形容詞の仮定形。
    // 「巨大な」→「巨大なら」
    do {
        if (tail.empty() || tail.substr(0, 2) != L"なら") break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[I_FIELD_PRE] + L"なら";
        node.post = fields[I_FIELD_POST] + L"なら";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        if (tail.empty() || tail.substr(0, 3) != L"ならば") break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[I_FIELD_PRE] + L"ならば";
        node.post = fields[I_FIELD_POST] + L"ならば";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // な形容詞の名詞形。
    // 「きれいな(な形容詞)」→「きれいさ(名詞)」、
    // 「巨大な」→「巨大さ」。
    node.bunrui = HB_MEISHI;
    do {
        if (tail.empty() || tail[0] != L'さ') break;
        node.pre = fields[I_FIELD_PRE] + L'さ';
        node.post = fields[I_FIELD_POST] + L'さ';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 「きれい。」「静か。」「巨大。」など
    if (tail.empty()) {
        DoMeishi(index, fields, deltaCost);
    } else {
        switch (tail[0]) {
        case L'。': case L'、': case L'，': case L'．': case L',': case L'.':
            DoMeishi(index, fields, deltaCost);
            break;
        }
    }
} // Lattice::DoNakeiyoushi

// 五段動詞を変換する。
void Lattice::DoGodanDoushi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());
    size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);
    DPRINTW(L"DoGodanDoushi: %s, %s\n", fields[I_FIELD_PRE].c_str(), tail.c_str());

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_GODAN_DOUSHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;
    node.gyou = (Gyou)HIBYTE(fields[I_FIELD_HINSHI][0]);

    // 五段動詞の未然形。
    // 「咲く(五段)」→「咲か(ない)」、「食う(五段)」→「食わ(ない)」
    do {
        node.katsuyou = MIZEN_KEI;
        WCHAR ch;
        if (node.gyou == GYOU_A) {
            ch = L'わ';
        } else {
            ch = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_A);
        }
        if (tail.empty() || tail[0] != ch)
            break;
        node.pre = fields[I_FIELD_PRE] + ch;
        node.post = fields[I_FIELD_POST] + ch;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        // 「咲かせ」「食わせ」～られる
        if (tail.size() >= 2 && tail[1] == L'せ') {
            node.pre = fields[I_FIELD_PRE] + ch + tail[1];
            node.post = fields[I_FIELD_POST] + ch + tail[1];
            m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        }
    } while (0);

    // 五段動詞の連用形。
    // 「咲く(五段)」→「咲き(ます)」、「食う(五段)」→「食い(ます)」
    node.katsuyou = RENYOU_KEI;
    WCHAR ch = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_I);
    if (tail.size() >= 1 && tail[0] == ch) {
        node.pre = fields[I_FIELD_PRE] + ch;
        node.post = fields[I_FIELD_POST] + ch;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    }

    // 五段動詞の連用形の音便処理。
    // 「咲き(て/た/たり/ても)」→「咲い(て/た/たり/ても)」
    // 「食い(て/た/たり/ても)」→「食っ(て/た/たり/ても)」
    // 「泣き(て/た/たり/ても)」→「泣い(て/た/たり/ても)」
    // 「持ち(て/た/たり/ても)」→「持っ(て/た/たり/ても)」
    // 「呼び(て/た/たり/ても)」→「呼ん(で/だ/だり/でも)」
    // 「書き(て/た/たり/ても)」→「書い(て/た/たり/ても)」
    // 「担い(て/た/たり/ても)」→「担っ(て/た/たり/ても)」
    WCHAR ch2 = 0;
    switch (node.gyou) {
    case GYOU_KA: case GYOU_GA:
        ch2 = L'い';
        break;

    case GYOU_NA: case GYOU_BA: case GYOU_MA:
        ch2 = L'ん';
        break;

    case GYOU_A: case GYOU_TA: case GYOU_RA: case GYOU_WA:
        ch2 = L'っ';
        break;

    case GYOU_SA: case GYOU_ZA: case GYOU_DA: case GYOU_HA: case GYOU_PA:
    case GYOU_YA: case GYOU_NN:
        ch2 = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_I);
        break;
    }
    if (ch2 != 0 && tail.size() >= 1 && tail[0] == ch2) {
        node.pre = fields[I_FIELD_PRE] + ch2;
        node.post = fields[I_FIELD_POST] + ch2;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() >= 3 && (tail[1] == L'て' || tail[1] == L'で') && tail[2] == L'も') {
            // 連用形「ても」「でも」
            node.katsuyou = RENYOU_KEI;
            node.pre = fields[I_FIELD_PRE] + ch2 + tail[1] + tail[2];
            node.post = fields[I_FIELD_POST] + ch2 + tail[1] + tail[2];
            m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        } else if (tail.size() >= 2 && (tail[1] == L'て' || tail[1] == L'で')) {
            // 連用形「て」「で」
            node.katsuyou = RENYOU_KEI;
            node.pre = fields[I_FIELD_PRE] + ch2 + tail[1];
            node.post = fields[I_FIELD_POST] + ch2 + tail[1];
            m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        } else if (tail.size() >= 3 && (tail[1] == L'た' || tail[1] == L'だ') && tail[2] == L'り') {
            // 連用形「たり」「だり」
            node.katsuyou = RENYOU_KEI;
            node.pre = fields[I_FIELD_PRE] + ch2 + tail[1] + tail[2];
            node.post = fields[I_FIELD_POST] + ch2 + tail[1] + tail[2];
            m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        } else if (tail.size() >= 2 && tail[1] == L'た') {
            // 終止形「た」
            node.katsuyou = SHUUSHI_KEI;
            node.pre = fields[I_FIELD_PRE] + ch2 + tail[1];
            node.post = fields[I_FIELD_POST] + ch2 + tail[1];
            m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        }
    }

    // 五段動詞の終止形。「動く」「聞き取る」
    // 五段動詞の連体形。「動く(とき)」「聞き取る(とき)」
    do {
        WCHAR ch = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_U);
        if (tail.size() <= 0 || tail[0] != ch)
            break;

        node.katsuyou = SHUUSHI_KEI;
        node.pre = fields[I_FIELD_PRE] + ch;
        node.post = fields[I_FIELD_POST] + ch;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.katsuyou = RENTAI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        // 「動くよ」「動くね」「動くな」「動くぞ」
        if (tail.size() < 2 ||
            (tail[1] != L'よ' && tail[1] != L'ね' && tail[1] != L'な' && tail[1] != L'ぞ'))
                break;
        node.pre += tail[1];
        node.post += tail[1];
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 五段動詞の仮定形。「動く」→「動け(ば)」、「聞き取る」→「聞き取れ(ば)」
    // 五段動詞の命令形。
    // 「動く」→「動け」「動けよ」、「聞き取る」→「聞き取れ」「聞き取れよ」
    // 「くださる」→「ください」
    do {
        WCHAR ch = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_E);
        if (tail.empty() || tail[0] != ch) {
            ch = L'い';
            if (tail.empty() || tail[0] != ch) {
                break;
            }
        }
        node.katsuyou = KATEI_KEI;
        node.pre = fields[I_FIELD_PRE] + ch;
        node.post = fields[I_FIELD_POST] + ch;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.katsuyou = MEIREI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 2 || (tail[1] != L'よ' && tail[1] != L'や'))
            break;
        node.pre += tail[1];
        node.post += tail[1];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 五段動詞の命令形。「動く」→「動こう」「動こうよ」、「聞き取る」→「聞き取ろう」「聞き取ろうよ」
    do {
        WCHAR ch = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_O);
        if (tail.size() < 2 || tail[0] != ch || tail[1] != L'う')
            break;
        node.katsuyou = MEIREI_KEI;
        node.pre = fields[I_FIELD_PRE] + ch + L'う';
        node.post = fields[I_FIELD_POST] + ch + L'う';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 3 ||
            (tail[2] != L'よ' && tail[2] != L'や' && tail[2] != L'な' && tail[2] != L'ね'))
            break;
        node.pre += tail[2];
        node.post += tail[2];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 五段動詞の名詞形。
    // 「動く(五段)」→「動き(名詞)」「動き方(名詞)」、
    // 「聞き取る(五段)」→「聞き取り(名詞)」「聞き取り方(名詞)」など。
    node.bunrui = HB_MEISHI;
    do {
        WCHAR ch = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_I);
        if (tail.empty() || tail[0] != ch)
            break;
        node.pre = fields[I_FIELD_PRE] + ch;
        node.post = fields[I_FIELD_POST] + ch;
        node.deltaCost = deltaCost + 40;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail[1] != L'か' || tail[2] != L'た')
            break;
        node.pre += L"かた";
        node.post += L"方";
        node.deltaCost = deltaCost;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 「動く(五段)」→「動ける(一段)」、
    // 「聞く(五段)」→「聞ける(一段)」など。
    do {
        WCHAR ch = ARRAY_AT_AT(s_hiragana_table, node.gyou, DAN_E);
        if (tail.empty() || tail[0] != ch)
            break;
        WStrings new_fields = fields;
        new_fields[I_FIELD_PRE] += ch;
        new_fields[I_FIELD_POST] += ch;
        DoIchidanDoushi(index, new_fields, deltaCost + 30);
    } while (0);
} // Lattice::DoGodanDoushi

// 一段動詞を変換する。
void Lattice::DoIchidanDoushi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());
    size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_ICHIDAN_DOUSHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;

    // 一段動詞の未然形。「寄せる」→「寄せ(ない/よう)」、「見る」→「見(ない/よう)」
    // 一段動詞の連用形。「寄せる」→「寄せ(ます/た/て)」、「見る」→「見(ます/た/て)」
    do {
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        node.katsuyou = MIZEN_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.katsuyou = RENYOU_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        // 「見て」
        if (tail.size() >= 1 && tail[0] == L'て') {
            node.pre += tail[0];
            node.post += tail[0];
            m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
            // 「見ていた」
            if (tail.size() >= 3 && tail[1] == L'い' && tail[2] == L'た') {
                node.pre += L"いた";
                node.post += L"いた";
                node.katsuyou = SHUUSHI_KEI;
                m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
                // 「見ていたよ」「見ていたな」「見ていたね」
                if (tail.size() >= 4 && (tail[3] == L'よ' || tail[3] == L'な' || tail[3] == L'ね')) {
                    node.pre += tail[3];
                    node.post += tail[3];
                    node.katsuyou = SHUUSHI_KEI;
                    m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
                    // 「見ていたよぉ」「見ていたなー」「見ていたねえ」
                    if (tail.size() >= 5 &&
                        (tail[4] == L'ー' ||
                         tail[4] == L'あ' || tail[4] == L'ぁ' ||
                         tail[4] == L'え' || tail[4] == L'ぇ' ||
                         tail[4] == L'お' || tail[4] == L'ぉ'))
                    {
                        node.pre += tail[4];
                        node.post += tail[4];
                        node.katsuyou = SHUUSHI_KEI;
                        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
                    }
                }
            }
            // 「見てた」
            if (tail.size() >= 2 && tail[1] == L'た') {
                node.pre = fields[I_FIELD_PRE] + L"てた";
                node.post = fields[I_FIELD_POST] + L"てた";
                node.katsuyou = SHUUSHI_KEI;
                m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
                // 「見てたよ」「見てたな」「見てたね」
                if (tail.size() >= 3 && (tail[2] == L'よ' || tail[2] == L'な' || tail[2] == L'ね')) {
                    node.pre += tail[2];
                    node.post += tail[2];
                    node.katsuyou = SHUUSHI_KEI;
                    m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
                    // 「見てたよぉ」「見てたなー」「見てたねえ」
                    if (tail.size() >= 4 &&
                        (tail[3] == L'ー' ||
                         tail[3] == L'あ' || tail[3] == L'ぁ' ||
                         tail[3] == L'え' || tail[3] == L'ぇ' ||
                         tail[3] == L'お' || tail[3] == L'ぉ'))
                    {
                        node.pre += tail[3];
                        node.post += tail[3];
                        node.katsuyou = SHUUSHI_KEI;
                        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
                    }
                }
            }
        }
        // 終止形「見よう」「見ようね」「見ようや」「見ような」「見ようぞ」
        if (tail.size() >= 2 && tail[0] == L'よ' && tail[1] == L'う') {
            node.katsuyou = SHUUSHI_KEI;
            node.pre = fields[I_FIELD_PRE] + L"よう";
            node.post = fields[I_FIELD_POST] + L"よう";
            m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

            if (tail.size() >= 3 &&
                (tail[2] == L'ね' || tail[2] == L'や' || tail[2] == L'な' || tail[2] == L'ぞ'))
            {
                node.pre += tail[2];
                node.post += tail[2];
                m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
            }
        }
    } while (0);

    // 一段動詞の終止形。「寄せる」「見る」
    // 一段動詞の連体形。「寄せる(とき)」「見る(とき)」
    do {
        if (tail.empty() || tail[0] != L'る') break;
        node.pre = fields[I_FIELD_PRE] + L'る';
        node.post = fields[I_FIELD_POST] + L'る';
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.katsuyou = RENTAI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        // 「見るよ」「見るね」「見るな」「見るぞ」
        if (tail.size() < 2 ||
            (tail[1] != L'よ' && tail[1] != L'ね' && tail[1] != L'な' && tail[1] != L'ぞ'))
                break;
        node.pre += tail[1];
        node.post += tail[1];
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 一段動詞の仮定形。「寄せる」→「寄せれ(ば)」、「見る」→「見れ(ば)」
    do {
        if (tail.empty() || tail[0] != L'れ') break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[I_FIELD_PRE] + L'れ';
        node.post = fields[I_FIELD_POST] + L'れ';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 一段動詞の命令形。
    // 「寄せる」→「寄せろ」「寄せろよ」、「見る」→「見ろ」「見ろよ」
    node.katsuyou = MEIREI_KEI;
    do {
        if (tail.empty() || tail[0] != L'ろ') break;
        node.pre = fields[I_FIELD_PRE] + L'ろ';
        node.post = fields[I_FIELD_POST] + L'ろ';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 2 || (tail[1] != L'よ' && tail[1] != L'や')) break;
        node.pre += tail[1];
        node.post += tail[1];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    // 「寄せる」→「寄せよ」、「見る」→「見よ」
    do {
        if (tail.empty() || tail[0] != L'よ') break;
        node.pre = fields[I_FIELD_PRE] + L'よ';
        node.post = fields[I_FIELD_POST] + L'よ';
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 一段動詞の名詞形。
    // 「寄せる」→「寄せ」「寄せ方」、「見る」→「見」「見方」
    node.bunrui = HB_MEISHI;
    do {
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.empty() || tail[0] != L'か' || tail[1] != L'た') break;
        node.pre += L"かた";
        node.post += L"方";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
} // Lattice::DoIchidanDoushi

// カ変動詞を変換する。
void Lattice::DoKahenDoushi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());
    size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_KAHEN_DOUSHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;

    // 「くる」「こ(ない)」「き(ます)」などと、語幹が一致しないので、
    // 実際の辞書では「来い」を登録するなど回避策を施している。

    // 終止形と連用形「～来る」
    do {
        if (tail.size() < 2 || tail[0] != L'く' || tail[1] != L'る') break;
        node.pre = fields[I_FIELD_PRE] + L"くる";
        node.post = fields[I_FIELD_POST] + L"来る";
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.katsuyou = RENTAI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        // 「来るよ」「来るね」「来るな」「来るぞ」
        if (tail.size() < 3 ||
            (tail[2] != L'よ' && tail[2] != L'ね' && tail[2] != L'な' && tail[2] != L'ぞ'))
                break;
        node.pre += tail[2];
        node.post += tail[2];
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (fields[I_FIELD_PRE] != L"くる") break;
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.katsuyou = RENTAI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        // 「来るよ」「来るね」「来るな」「来るぞ」
        if (tail.size() < 1 ||
            (tail[0] != L'よ' && tail[0] != L'ね' && tail[0] != L'な' && tail[0] != L'ぞ'))
                break;
        node.pre += tail[0];
        node.post += tail[0];
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 命令形「～こい」「～こいよ」「～こいや」
    do {
        if (tail.size() < 2 || tail[0] != L'こ' || tail[1] != L'い') break;
        node.pre = fields[I_FIELD_PRE] + L"こい";
        node.post = fields[I_FIELD_POST] + L"来い";
        node.katsuyou = MEIREI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 3 || (tail[2] != L'よ' && tail[2] != L'や')) break;
        node.pre += tail[2];
        node.post += tail[2];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (fields[I_FIELD_PRE] != L"こい") break;
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        node.katsuyou = MEIREI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        if (tail.size() < 1 || (tail[0] != L'よ' && tail[0] != L'や')) break;
        node.pre += tail[0];
        node.post += tail[0];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 仮定形「～来れ」
    do {
        if (tail.size() < 2 || tail[0] != L'く' || tail[1] != L'れ') break;
        node.pre = fields[I_FIELD_PRE] + L"くれ";
        node.post = fields[I_FIELD_POST] + L"来れ";
        node.katsuyou = KATEI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (fields[I_FIELD_PRE] != L"くれ") break;
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        node.katsuyou = KATEI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 未然形「～来」（こ）
    do {
        if (tail.size() < 1 || tail[0] != L'こ') break;
        node.pre = fields[I_FIELD_PRE] + L"こ";
        node.post = fields[I_FIELD_POST] + L"来";
        node.katsuyou = MIZEN_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        if (tail.substr(0, 3) != L"こさせ") break;
        node.pre = fields[I_FIELD_PRE] + L"こさせ";
        node.post = fields[I_FIELD_POST] + L"来させ";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (fields[I_FIELD_PRE] != L"こ") break;
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        node.katsuyou = MIZEN_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        if (tail.substr(0, 3) != L"させ") break;
        node.pre = fields[I_FIELD_PRE] + L"させ";
        node.post = fields[I_FIELD_POST] + L"させ";
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 連用形「～来」（き）
    do {
        if (tail.size() < 1 || tail[0] != L'き') break;
        node.pre = fields[I_FIELD_PRE] + L"き";
        node.post = fields[I_FIELD_POST] + L"来";
        node.katsuyou = RENYOU_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (fields[I_FIELD_PRE] != L"き") break;
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        node.katsuyou = RENYOU_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
} // Lattice::DoKahenDoushi

// サ変動詞を変換する。
void Lattice::DoSahenDoushi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());
    size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_SAHEN_DOUSHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;
    node.gyou = (Gyou)HIBYTE(fields[I_FIELD_HINSHI][0]);

    // 未然形
    node.katsuyou = MIZEN_KEI;
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail[0] != L'ざ') break;
            node.pre = fields[I_FIELD_PRE] + L'ざ';
            node.post = fields[I_FIELD_POST] + L'ざ';
        } else {
            if (tail.empty() || tail[0] != L'さ') break;
            node.pre = fields[I_FIELD_PRE] + L'さ';
            node.post = fields[I_FIELD_POST] + L'さ';
        }
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail[0] != L'じ') break;
            node.pre = fields[I_FIELD_PRE] + L'じ';
            node.post = fields[I_FIELD_POST] + L'じ';
        } else {
            if (tail.empty() || tail[0] != L'し') break;
            node.pre = fields[I_FIELD_PRE] + L'し';
            node.post = fields[I_FIELD_POST] + L'し';
        }
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail[0] != L'ぜ') break;
            node.pre = fields[I_FIELD_PRE] + L'ぜ';
            node.post = fields[I_FIELD_POST] + L'ぜ';
        } else {
            if (tail.empty() || tail[0] != L'せ') break;
            node.pre = fields[I_FIELD_PRE] + L'せ';
            node.post = fields[I_FIELD_POST] + L'せ';
        }
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 連用形
    node.katsuyou = RENYOU_KEI;
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail[0] != L'じ') break;
            node.pre = fields[I_FIELD_PRE] + L'じ';
            node.post = fields[I_FIELD_POST] + L'じ';
        } else {
            if (tail.empty() || tail[0] != L'し') break;
            node.pre = fields[I_FIELD_PRE] + L'し';
            node.post = fields[I_FIELD_POST] + L'し';
        }
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 終止形
    // 連用形
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail.substr(0, 2) != L"ずる") break;
            node.pre = fields[I_FIELD_PRE] + L"ずる";
            node.post = fields[I_FIELD_POST] + L"ずる";
        } else {
            if (tail.empty() || tail.substr(0, 2) != L"する") break;
            node.pre = fields[I_FIELD_PRE] + L"する";
            node.post = fields[I_FIELD_POST] + L"する";
        }
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.katsuyou = RENYOU_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail[0] != L'ず') break;
            node.pre = fields[I_FIELD_PRE] + L'ず';
            node.post = fields[I_FIELD_POST] + L'ず';
        } else {
            if (tail.empty() || tail[0] != L'す') break;
            node.pre = fields[I_FIELD_PRE] + L'す';
            node.post = fields[I_FIELD_POST] + L'す';
        }
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 仮定形
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail.substr(0, 2) != L"ずれ") break;
            node.pre = fields[I_FIELD_PRE] + L"ずれ";
            node.post = fields[I_FIELD_POST] + L"ずれ";
        } else {
            if (tail.empty() || tail.substr(0, 2) != L"すれ") break;
            node.pre = fields[I_FIELD_PRE] + L"すれ";
            node.post = fields[I_FIELD_POST] + L"すれ";
        }
        node.katsuyou = KATEI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 命令形
    node.katsuyou = MEIREI_KEI;
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail.substr(0, 2) != L"じろ") break;
            node.pre = fields[I_FIELD_PRE] + L"じろ";
            node.post = fields[I_FIELD_POST] + L"じろ";
        } else {
            if (tail.empty() || tail.substr(0, 2) != L"しろ") break;
            node.pre = fields[I_FIELD_PRE] + L"しろ";
            node.post = fields[I_FIELD_POST] + L"しろ";
        }
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (tail.empty() || tail.substr(0, 2) != L"ぜよ") break;
            node.pre = fields[I_FIELD_PRE] + L"ぜよ";
            node.post = fields[I_FIELD_POST] + L"ぜよ";
        } else {
            if (tail.empty() || tail.substr(0, 2) != L"せよ") break;
            node.pre = fields[I_FIELD_PRE] + L"せよ";
            node.post = fields[I_FIELD_POST] + L"せよ";
        }
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
} // Lattice::DoSahenDoushi

void Lattice::DoMeishi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());

    size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_MEISHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;

    // 名詞は活用なし。
    if (node.HasTag(L"[動植物]")) {
        // 動植物名は、カタカナでもよい。
        node.pre = fields[I_FIELD_PRE];
        node.post = lcmap(fields[I_FIELD_PRE], LCMAP_KATAKANA | LCMAP_FULLWIDTH);
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

        node.deltaCost += 30;
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } else {
        node.pre = fields[I_FIELD_PRE];
        node.post = fields[I_FIELD_POST];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    }

    // 名詞＋「っぽい」でい形容詞に。
    if (tail.size() >= 2 && tail[0] == L'っ' && tail[1] == L'ぽ') {
        WStrings new_fields = fields;
        new_fields[I_FIELD_PRE] += L"っぽ";
        new_fields[I_FIELD_POST] += L"っぽ";
        DoIkeiyoushi(index, new_fields, deltaCost);
    }

    // 名詞＋「みたいな」でな形容詞に。
    if (tail.size() >= 3 && tail[0] == L'み' && tail[1] == L'た' && tail[2] == L'い') {
        WStrings new_fields = fields;
        new_fields[I_FIELD_PRE] += L"みたい";
        new_fields[I_FIELD_POST] += L"みたい";
        DoNakeiyoushi(index, new_fields, deltaCost);
    }
    // 名詞＋「みたい」でい形容詞に。
    if (tail.size() >= 2 && tail[0] == L'み' && tail[1] == L'た') {
        WStrings new_fields = fields;
        new_fields[I_FIELD_PRE] += L"みた";
        new_fields[I_FIELD_POST] += L"みた";
        DoIkeiyoushi(index, new_fields, deltaCost);
    }

    // 名詞＋「する」「すれ」でサ変動詞に。
    if (tail.size() >= 2 && tail[0] == L'す' && (tail[1] == L'る' || tail[1] == L'れ')) {
        DoSahenDoushi(index, fields, deltaCost + 50);
    }

    // 名詞＋「し」でサ変動詞に
    if (tail.size() >= 1 && tail[0] == L'し') {
        DoSahenDoushi(index, fields, deltaCost + 500);
    }

    // 名詞＋「せよ」でサ変動詞に。
    if (tail.size() >= 2 && tail[0] == L'せ' && tail[1] == L'よ') {
        DoSahenDoushi(index, fields, deltaCost - 10);
    }

    // 名詞＋「な」でな形容詞に。
    if (tail.size() >= 1 && tail[0] == L'な') {
        DoNakeiyoushi(index, fields, deltaCost + 500);
    }

    // 名詞＋「たる」「たれ」で五段動詞に。
    if (tail.size() >= 2 && tail[0] == L'た' && (tail[1] == L'る' || tail[1] == L'れ')) {
        WStrings new_fields = fields;
        new_fields[I_FIELD_PRE] += L'た';
        new_fields[I_FIELD_POST] += L'た';
        new_fields[I_FIELD_HINSHI] = { MAKEWORD(HB_GODAN_DOUSHI, GYOU_RA) };
        DoGodanDoushi(index, new_fields, deltaCost);
    }
} // Lattice::DoMeishi

void Lattice::DoFukushi(size_t index, const WStrings& fields, INT deltaCost)
{
    FOOTMARK();
    ASSERT(fields.size() == NUM_FIELDS);
    ASSERT(fields[I_FIELD_PRE].size());

    size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    // 語幹の後の部分文字列。
    std::wstring tail = m_pre.substr(index + length);

    // ラティスノードの準備。
    LatticeNode node;
    node.bunrui = HB_FUKUSHI;
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;

    // 副詞。活用はない。
    node.pre = fields[I_FIELD_PRE];
    node.post = fields[I_FIELD_POST];
    m_chunks[index].push_back(std::make_shared<LatticeNode>(node));

    // 副詞なら最後に「と」「に」を付けてもいい。
    do {
        if (tail.size() < 1 || (tail[0] != L'と' && tail[0] != L'に')) break;
        node.pre += tail[0];
        node.post += tail[0];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);

    // 副詞なら最後に「っと」「って」を付けてもいい。
    do {
        if (tail.size() < 2 || tail[0] != L'っ' || (tail[1] != L'と' && tail[1] != L'て')) break;
        node.pre = fields[I_FIELD_PRE] + tail[0] + tail[1];
        node.post = fields[I_FIELD_POST] + tail[0] + tail[1];
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
    } while (0);
}

void Lattice::DoFields(size_t index, const WStrings& fields, INT deltaCost)
{
    ASSERT(fields.size() == NUM_FIELDS);
    if (fields.size() != NUM_FIELDS) {
        DPRINTW(L"%s, %s\n", fields[I_FIELD_PRE].c_str(), fields[I_FIELD_POST].c_str());
        return;
    }
    const size_t length = fields[I_FIELD_PRE].size();
    // 区間チェック。
    if (index + length > m_pre.size()) {
        return;
    }
    // 対象のテキストが語幹と一致するか確かめる。
    if (m_pre.substr(index, length) != fields[I_FIELD_PRE]) {
        return;
    }
    DPRINTW(L"DoFields: %s\n", fields[I_FIELD_PRE].c_str());

    // ラティスノードの準備。
    LatticeNode node;
    node.pre = fields[I_FIELD_PRE];
    node.post = fields[I_FIELD_POST];
    WORD w = fields[I_FIELD_HINSHI][0];
    node.bunrui = (HinshiBunrui)LOBYTE(w);
    node.gyou = (Gyou)HIBYTE(w);
    node.tags = fields[I_FIELD_TAGS];
    node.deltaCost = deltaCost;

    // 品詞分類で場合分けする。
    switch (node.bunrui) {
    case HB_MEISHI:
        DoMeishi(index, fields, deltaCost);
        break;
    case HB_PERIOD: case HB_COMMA: case HB_SYMBOL:
    case HB_RENTAISHI: 
    case HB_SETSUZOKUSHI: case HB_KANDOUSHI:
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI: case HB_SHUU_JOSHI:
    case HB_KANGO: case HB_SETTOUJI: case HB_SETSUBIJI:
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        break;
    case HB_FUKUSHI:
        DoFukushi(index, fields, deltaCost);
        break;
    case HB_IKEIYOUSHI: // い形容詞。
        DoIkeiyoushi(index, fields, deltaCost);
        break;
    case HB_NAKEIYOUSHI: // な形容詞。
        DoNakeiyoushi(index, fields, deltaCost);
        break;
    case HB_MIZEN_JODOUSHI: // 未然助動詞。
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = MIZEN_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        break;
    case HB_RENYOU_JODOUSHI: // 連用助動詞。
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = RENYOU_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        break;
    case HB_SHUUSHI_JODOUSHI: // 終止助動詞。
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = SHUUSHI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        break;
    case HB_RENTAI_JODOUSHI: // 連体助動詞。
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = RENTAI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        break;
    case HB_KATEI_JODOUSHI: // 仮定助動詞。
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = KATEI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        break;
    case HB_MEIREI_JODOUSHI: // 命令助動詞。
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = MEIREI_KEI;
        m_chunks[index].push_back(std::make_shared<LatticeNode>(node));
        break;
    case HB_GODAN_DOUSHI: // 五段動詞。
        DoGodanDoushi(index, fields, deltaCost);
        break;
    case HB_ICHIDAN_DOUSHI: // 一段動詞。
        DoIchidanDoushi(index, fields, deltaCost);
        break;
    case HB_KAHEN_DOUSHI: // カ変動詞。
        DoKahenDoushi(index, fields, deltaCost);
        break;
    case HB_SAHEN_DOUSHI: // サ変動詞。
        DoSahenDoushi(index, fields, deltaCost);
        break;
    default:
        break;
    }
} // Lattice::DoFields

// ラティスをダンプする。
void Lattice::Dump(int num)
{
    const size_t length = m_pre.size();
    DPRINTW(L"### Lattice::Dump(%d) ###\n", num);
    DPRINTW(L"Lattice length: %d\n", int(length));
    for (size_t i = 0; i < length; ++i) {
        DPRINTW(L"Lattice chunk #%d:", int(i));
        for (size_t k = 0; k < m_chunks[i].size(); ++k) {
            DPRINTW(L" %s(%s)", m_chunks[i][k]->post.c_str(),
                        BunruiToString(m_chunks[i][k]->bunrui));
        }
        DPRINTW(L"\n");
    }
} // Lattice::Dump

//////////////////////////////////////////////////////////////////////////////

// 逆向きブランチ群を追加する。
void Lattice::MakeReverseBranches(LatticeNode *ptr0)
{
    ASSERT(ptr0);

    if (!ptr0->linked || ptr0->bunrui == HB_TAIL)
        return;

    size_t i = 0;
    for (auto& ptr1 : ptr0->branches) {
        if (ptr1->reverse_branches.count(ptr0) == 0) {
            ptr1->reverse_branches.insert(ptr0);
            MakeReverseBranches(ptr1.get());
        }
        ++i;
    }
} // Lattice::MakeReverseBranches

// 複数文節変換において、ラティスを作成する。
BOOL Lattice::AddNodesForMulti(const std::wstring& pre)
{
    DPRINTW(L"%s\n", pre.c_str());

    // ラティスを初期化。
    ASSERT(pre.size() != 0);
    m_pre = pre; // 変換前の文字列。
    m_chunks.resize(pre.size() + 1);

    WCHAR *dict_data1 = g_basic_dict.Lock(); // 基本辞書をロック。
    if (dict_data1) {
        // ノード群を追加。
        AddNodesFromDict(0, dict_data1);

        g_basic_dict.Unlock(dict_data1); // 基本辞書のロックを解除。
    }

    WCHAR *dict_data2 = g_name_dict.Lock(); // 人名・地名辞書をロック。
    if (dict_data2) {
        // ノード群を追加。
        AddNodesFromDict(0, dict_data2);

        g_name_dict.Unlock(dict_data2); // 人名・地名辞書のロックを解除。
    }

    return TRUE;
} // Lattice::AddNodesForMulti

// 単一文節変換において、ラティスを作成する。
BOOL Lattice::AddNodesForSingle(const std::wstring& pre)
{
    DPRINTW(L"%s\n", pre.c_str());

    // ラティスを初期化。
    ASSERT(pre.size() != 0);
    m_pre = pre;
    m_chunks.resize(pre.size() + 1);

    BOOL bOK = TRUE;

    WCHAR *dict_data1 = g_basic_dict.Lock(); // 基本辞書をロックする。
    if (dict_data1) {
        // ノード群を追加。
        if (!AddNodesFromDict(dict_data1)) {
            AddComplement(0, pre.size(), pre.size());
        }

        g_basic_dict.Unlock(dict_data1); // 基本辞書のロックを解除。
    } else {
        bOK = FALSE;
    }

    WCHAR *dict_data2 = g_name_dict.Lock(); // 人名・地名辞書をロックする。
    if (dict_data2) {
        // ノード群を追加。
        if (!AddNodesFromDict(dict_data2)) {
            AddComplement(0, pre.size(), pre.size());
        }

        g_name_dict.Unlock(dict_data2); // 人名・地名辞書のロックを解除。
    } else {
        bOK = FALSE;
    }

    if (bOK)
        return TRUE;

    // ダンプ。
    Dump(4);
    return FALSE; // 失敗。
} // Lattice::AddNodesForSingle

// 複数文節変換において、変換結果を生成する。
void MzIme::MakeResultForMulti(MzConvResult& result, Lattice& lattice)
{
    DPRINTW(L"%s\n", lattice.m_pre.c_str());
    result.clear(); // 結果をクリア。

    LatticeNode* ptr0 = lattice.m_head.get();
    while (ptr0 && ptr0 != lattice.m_tail.get()) {
        LatticeNode* target = NULL;
        for (auto& ptr1 : ptr0->branches) {
            if (lattice.OptimizeMarking(ptr1.get())) {
                target = ptr1.get();
                break;
            }
        }

        if (!target || target->bunrui == HB_TAIL)
            break;

        MzConvClause clause;
        clause.add(target);

        for (auto& ptr1 : ptr0->branches) {
            if (target->pre.size() == ptr1->pre.size()) {
                if (target != ptr1.get()) {
                    clause.add(ptr1.get());
                }
            }
        }

        LatticeNode node;
        node.bunrui = HB_UNKNOWN;
        node.deltaCost = 3000;
        node.pre = lcmap(target->pre, LCMAP_HIRAGANA | LCMAP_FULLWIDTH);

        node.post = lcmap(node.pre, LCMAP_HIRAGANA | LCMAP_FULLWIDTH);
        clause.add(&node);

        node.post = lcmap(node.pre, LCMAP_KATAKANA | LCMAP_FULLWIDTH);
        clause.add(&node);

        node.post = lcmap(node.pre, LCMAP_KATAKANA | LCMAP_HALFWIDTH);
        clause.add(&node);

        node.post = lcmap(node.pre, LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
        clause.add(&node);

        node.post = lcmap(node.pre, LCMAP_UPPERCASE | LCMAP_FULLWIDTH);
        clause.add(&node);

        node.post = node.post[0] + lcmap(node.pre.substr(1), LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
        clause.add(&node);

        node.post = lcmap(node.pre, LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
        clause.add(&node);

        node.post = lcmap(node.pre, LCMAP_UPPERCASE | LCMAP_HALFWIDTH);
        clause.add(&node);

        node.post = node.post[0] + lcmap(node.pre.substr(1), LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
        clause.add(&node);

        result.clauses.push_back(clause);
        ptr0 = target;
    }

    // コストによりソートする。
    result.sort();
} // MzIme::MakeResultForMulti

// 変換に失敗したときの結果を作成する。
void MzIme::MakeResultOnFailure(MzConvResult& result, const std::wstring& pre)
{
    DPRINTW(L"%s\n", pre.c_str());
    MzConvClause clause; // 文節。
    result.clear(); // 結果をクリア。

    // ノードを初期化。
    LatticeNode node;
    node.pre = pre; // 変換前の文字列。
    node.deltaCost = 40; // コストは人名・地名よりも高くする。
    node.bunrui = HB_MEISHI; // 名詞。

    // 文節に無変換文字列を追加。
    node.post = pre; // 変換後の文字列。
    clause.add(&node);

    // 文節にひらがなを追加。
    node.post = lcmap(pre, LCMAP_HIRAGANA); // 変換後の文字列。
    clause.add(&node);

    // 文節にカタカナを追加。
    node.post = lcmap(pre, LCMAP_KATAKANA); // 変換後の文字列。
    clause.add(&node);

    // 文節に全角を追加。
    node.post = lcmap(pre, LCMAP_FULLWIDTH); // 変換後の文字列。
    clause.add(&node);

    // 文節に半角を追加。
    node.post = lcmap(pre, LCMAP_HALFWIDTH | LCMAP_KATAKANA); // 変換後の文字列。
    clause.add(&node);

    // 結果に文節を追加。
    result.clauses.push_back(clause);
} // MzIme::MakeResultOnFailure

// 単一文節変換の結果を作成する。
void MzIme::MakeResultForSingle(MzConvResult& result, Lattice& lattice)
{
    DPRINTW(L"%s\n", lattice.m_pre.c_str());
    result.clear(); // 結果をクリア。
    const size_t length = lattice.m_pre.size();

    // add other candidates
    MzConvClause clause;
    ASSERT(lattice.m_chunks.size());
    const LatticeChunk& chunk = ARRAY_AT(lattice.m_chunks, 0);
    for (size_t i = 0; i < chunk.size(); ++i) {
        if (ARRAY_AT(chunk, i)->pre.size() == length) {
            // add a candidate of same size
            clause.add(ARRAY_AT(chunk, i).get());
        }
    }

    // ノードを初期化する。
    std::wstring pre = lattice.m_pre; // 変換前の文字列。
    LatticeNode node;
    node.pre = pre;
    node.bunrui = HB_UNKNOWN;
    node.deltaCost = 40; // コストは人名・地名よりも高くする。

    // 文節に無変換文字列を追加。
    node.post = pre; // 変換後の文字列。
    clause.add(&node);

    // 文節にひらがなを追加。
    node.post = lcmap(pre, LCMAP_HIRAGANA); // 変換後の文字列。
    clause.add(&node);

    // 文節にカタカナを追加。
    node.post = lcmap(pre, LCMAP_KATAKANA); // 変換後の文字列。
    clause.add(&node);

    // 文節に全角を追加。
    node.post = lcmap(pre, LCMAP_FULLWIDTH); // 変換後の文字列。
    clause.add(&node);

    // 文節に半角を追加。
    node.post = lcmap(pre, LCMAP_HALFWIDTH | LCMAP_KATAKANA); // 変換後の文字列。
    clause.add(&node);

    // 結果に文節を追加。
    result.clauses.push_back(clause);
    ASSERT(ARRAY_AT(result.clauses, 0).candidates.size());

    // コストによりソートする。
    result.sort();
    ASSERT(ARRAY_AT(result.clauses, 0).candidates.size());
} // MzIme::MakeResultForSingle

// 複数文節を変換する。
BOOL MzIme::ConvertMultiClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    MzConvResult result;
    std::wstring str = ARRAY_AT(comp.extra.hiragana_clauses, comp.extra.iClause);
    if (!ConvertMultiClause(str, result)) {
        return FALSE;
    }
    return StoreResult(result, comp, cand);
} // MzIme::ConvertMultiClause

// 複数文節を変換する。
BOOL MzIme::ConvertMultiClause(const std::wstring& str, MzConvResult& result)
{
    DPRINTW(L"%s\n", str.c_str());

    // 変換前文字列をひらがな全角で取得。
    std::wstring pre = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);

    // ラティスを作成し、結果を作成する。
    Lattice lattice;
    lattice.AddNodesForMulti(pre);
    lattice.UpdateLinksAndBranches();
    lattice.CutUnlinkedNodes();
    lattice.AddComplement();
    lattice.MakeReverseBranches(lattice.m_head.get());

    lattice.m_tail->marked = 1;
    lattice.CalcSubTotalCosts(lattice.m_tail.get());

    lattice.m_head->marked = 1;
    lattice.OptimizeMarking(lattice.m_head.get());

    MakeResultForMulti(result, lattice);

    return TRUE;
} // MzIme::ConvertMultiClause

// 単一文節を変換する。
BOOL MzIme::ConvertSingleClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    DWORD iClause = comp.extra.iClause; // 現在の文節。

    // 変換する。
    MzConvResult result;
    std::wstring str = ARRAY_AT(comp.extra.hiragana_clauses, iClause);
    if (!ConvertSingleClause(str, result)) {
        return FALSE;
    }

    // 未確定文字列をセット。
    result.clauses.resize(1);
    MzConvClause& clause = ARRAY_AT(result.clauses, 0);
    comp.SetClauseCompString(iClause, ARRAY_AT(clause.candidates, 0).post);
    comp.SetClauseCompHiragana(iClause, ARRAY_AT(clause.candidates, 0).pre, bRoman);

    // 候補リストをセットする。
    LogCandList cand_list;
    for (auto& cand2 : clause.candidates) {
        cand_list.cand_strs.push_back(cand2.post);
    }
    ARRAY_AT(cand.cand_lists, iClause) = cand_list;

    // 現在の文節をセットする。
    cand.iClause = iClause;
    comp.extra.iClause = iClause;

    return TRUE;
} // MzIme::ConvertSingleClause

// 単一文節を変換する。
BOOL MzIme::ConvertSingleClause(const std::wstring& str, MzConvResult& result)
{
    DPRINTW(L"%s\n", str.c_str());
    result.clear(); // 結果をクリア。

    // 変換前文字列をひらがな全角で取得。
    std::wstring pre = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);

    // ラティスを作成する。
    Lattice lattice;
    lattice.AddNodesForSingle(pre);
    lattice.AddExtraNodes();

    // 結果を作成する。
    MakeResultForSingle(result, lattice);

    return TRUE;
} // MzIme::ConvertSingleClause

// 文節を左に伸縮する。
BOOL MzIme::StretchClauseLeft(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    DWORD iClause = comp.extra.iClause; // 現在の文節の位置。

    // 現在の文節を取得する。
    std::wstring str1 = comp.extra.hiragana_clauses[iClause];
    // 一文字以下の長さなら左に拡張できない。
    if (str1.size() <= 1)
        return FALSE;

    // この文節の最後の文字。
    WCHAR ch = str1[str1.size() - 1];
    // この文節を１文字縮小する。
    str1.resize(str1.size() - 1);

    // その文字を次の文節の先頭に追加する。
    std::wstring str2;
    BOOL bSplitted = FALSE; // 分離したか？
    if (iClause + 1 < comp.GetClauseCount()) {
        str2 = ch + comp.extra.hiragana_clauses[iClause + 1];
    } else {
        str2 += ch;
        bSplitted = TRUE; // 分離した。
    }

    // ２つの文節を単一文節変換する。
    MzConvResult result1, result2;
    if (!ConvertSingleClause(str1, result1)) {
        return FALSE;
    }
    if (!ConvertSingleClause(str2, result2)) {
        return FALSE;
    }

    // 文節が分離したら、新しい文節を挿入する。
    if (bSplitted) {
        std::wstring str;
        comp.extra.hiragana_clauses.insert(comp.extra.hiragana_clauses.begin() + iClause + 1, str);
        comp.extra.comp_str_clauses.insert(comp.extra.comp_str_clauses.begin() + iClause + 1, str);
    }

    // 未確定文字列をセット。
    auto& clause1 = result1.clauses[0];
    auto& clause2 = result2.clauses[0];
    comp.extra.hiragana_clauses[iClause] = str1;
    comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].post;
    comp.extra.hiragana_clauses[iClause + 1] = str2;
    comp.extra.comp_str_clauses[iClause + 1] = clause2.candidates[0].post;

    // 余剰情報から未確定文字列を更新する。
    comp.UpdateFromExtra(bRoman);

    // 候補リストをセットする。
    {
        LogCandList cand_list;
        for (auto& cand1 : clause1.candidates) {
            cand_list.cand_strs.push_back(cand1.post);
        }
        cand.cand_lists[iClause] = cand_list;
    }
    {
        LogCandList cand_list;
        for (auto& cand2 : clause2.candidates) {
            cand_list.cand_strs.push_back(cand2.post);
        }
        if (bSplitted) {
            cand.cand_lists.push_back(cand_list);
        } else {
            cand.cand_lists[iClause + 1] = cand_list;
        }
    }

    // 現在の文節をセットする。
    cand.iClause = iClause;
    comp.extra.iClause = iClause;

    // 文節属性をセットする。
    comp.SetClauseAttr(iClause, ATTR_TARGET_CONVERTED);

    return TRUE;
} // MzIme::StretchClauseLeft

// 文節を右に伸縮する。
BOOL MzIme::StretchClauseRight(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    DWORD iClause = comp.extra.iClause; // 現在の文節の位置。

    // 現在の文節を取得する。
    std::wstring str1 = comp.extra.hiragana_clauses[iClause];

    // 右端であれば右には拡張できない。
    if (iClause == comp.GetClauseCount() - 1)
        return FALSE;

    // 次の文節を取得する。
    std::wstring str2 = comp.extra.hiragana_clauses[iClause + 1];
    // 次の文節が空ならば、右には拡張できない。
    if (str2.empty())
        return FALSE;

    // str2の最初の文字。
    WCHAR ch = str2[0];

    // それをstr1の末尾に引っ越しする。
    str1 += ch;
    if (str2.size() == 1) {
        str2.clear();
    } else {
        str2 = str2.substr(1);
    }

    // 関係する文節を単一文節変換。
    MzConvResult result1, result2;
    if (!ConvertSingleClause(str1, result1)) {
        return FALSE;
    }
    if (str2.size() && !ConvertSingleClause(str2, result2)) {
        return FALSE;
    }

    // 現在の文節。
    auto& clause1 = result1.clauses[0];

    if (str2.empty()) { // 次の文節が空になったか？
        // 次の文節を削除する。
        comp.extra.hiragana_clauses.erase(comp.extra.hiragana_clauses.begin() + iClause + 1);
        comp.extra.comp_str_clauses.erase(comp.extra.comp_str_clauses.begin() + iClause + 1);
        comp.extra.hiragana_clauses[iClause] = str1;
        comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].post;
    } else {
        // ２つの文節情報をセットする。
        auto& clause2 = result2.clauses[0];
        comp.extra.hiragana_clauses[iClause] = str1;
        comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].post;
        comp.extra.hiragana_clauses[iClause + 1] = str2;
        comp.extra.comp_str_clauses[iClause + 1] = clause2.candidates[0].post;
    }

    // 余剰情報から未確定文字列を更新する。
    comp.UpdateFromExtra(bRoman);

    // 候補リストをセットする。
    {
        LogCandList cand_list;
        for (auto& cand1 : clause1.candidates) {
            cand_list.cand_strs.push_back(cand1.post);
        }
        cand.cand_lists[iClause] = cand_list;
    }
    if (str2.size()) {
        MzConvClause& clause2 = result2.clauses[0];
        LogCandList cand_list;
        for (auto& cand2 : clause2.candidates) {
            cand_list.cand_strs.push_back(cand2.post);
        }
        cand.cand_lists[iClause + 1] = cand_list;
    }

    // 現在の文節をセットする。
    cand.iClause = iClause;
    comp.extra.iClause = iClause;

    // 文節属性をセットする。
    comp.SetClauseAttr(iClause, ATTR_TARGET_CONVERTED);

    return TRUE;
} // MzIme::StretchClauseRight

// Shift_JISのマルチバイト文字の1バイト目か？
inline bool is_sjis_lead(BYTE ch)
{
    return (((0x81 <= ch) && (ch <= 0x9F)) || ((0xE0 <= ch) && (ch <= 0xEF)));
}

// Shift_JISのマルチバイト文字の2バイト目か？
inline bool is_sjis_trail(BYTE ch)
{
    return (((0x40 <= ch) && (ch <= 0x7E)) || ((0x80 <= ch) && (ch <= 0xFC)));
}

// JISバイトか？
inline bool is_jis_byte(BYTE ch)
{
    return ((0x21 <= ch) && (ch <= 0x7E));
}

// JISコードか？
inline bool is_jis_code(WORD w)
{
    BYTE ch0 = BYTE(w >> 8);
    BYTE ch1 = BYTE(w);
    return (is_jis_byte(ch0) && is_jis_byte(ch1));
}

// JISコードをShift_JISに変換する。
inline WORD jis2sjis(BYTE c0, BYTE c1)
{
    if (c0 & 0x01) {
        c0 >>= 1;
        if (c0 < 0x2F) {
            c0 += 0x71;
        } else {
            c0 -= 0x4F;
        }
        if (c1 > 0x5F) {
            c1 += 0x20;
        } else {
            c1 += 0x1F;
        }
    } else {
        c0 >>= 1;
        if (c0 < 0x2F) {
            c0 += 0x70;
        } else {
            c0 -= 0x50;
        }
        c1 += 0x7E;
    }
    WORD sjis_code = WORD((c0 << 8) | c1);
    return sjis_code;
} // jis2sjis

// JISコードをShift_JISコードに変換する。
inline WORD jis2sjis(WORD jis_code)
{
    BYTE c0 = BYTE(jis_code >> 8);
    BYTE c1 = BYTE(jis_code);
    return jis2sjis(c0, c1);
}

// Shift_JISコードか？
inline bool is_sjis_code(WORD w)
{
    return is_sjis_lead(BYTE(w >> 8)) && is_sjis_trail(BYTE(w));
}

// 区点からJISコードに変換。
inline WORD kuten_to_jis(const std::wstring& str)
{
    if (str.size() != 5) return 0; // 五文字でなければ区点コードではない。
    std::wstring ku_bangou = str.substr(0, 3); // 区番号。
    std::wstring ten_bangou = str.substr(3, 2); // 点番号。
    WORD ku = WORD(wcstoul(ku_bangou.c_str(), NULL, 10)); // 区番号を10進数として解釈。
    WORD ten = WORD(wcstoul(ten_bangou.c_str(), NULL, 10)); // 点番号を10進数として解釈。
    WORD jis_code = (ku + 32) * 256 + ten + 32; // 区と点によりJISコードを計算。
    return jis_code;
}

// コード変換。
BOOL MzIme::ConvertCode(const std::wstring& strTyping, MzConvResult& result)
{
    // ノードを初期化。
    LatticeNode node;
    node.pre = strTyping;
    node.bunrui = HB_UNKNOWN;

    // 16進を読み込み。
    ULONG hex_code = wcstoul(strTyping.c_str(), NULL, 16);

    // 文節情報。
    MzConvClause clause;

    // Unicodeのノードを文節に追加。
    WCHAR szUnicode[2];
    szUnicode[0] = WCHAR(hex_code);
    szUnicode[1] = 0;
    node.post = szUnicode; // 変換後の文字列。
    clause.add(&node);
    node.deltaCost++; // コストを１つ加算。

    // Shift_JISコードのノードを文節に追加。
    CHAR szSJIS[8];
    WORD wSJIS = WORD(hex_code);
    if (is_sjis_code(wSJIS)) {
        szSJIS[0] = HIBYTE(wSJIS);
        szSJIS[1] = LOBYTE(wSJIS);
        szSJIS[2] = 0;
        ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
        node.post = szUnicode; // 変換後の文字列。
        node.deltaCost++; // コストを１つ加算。
        clause.add(&node);
    }

    // JISコードのノードを文節に追加。
    if (is_jis_code(WORD(hex_code))) {
        wSJIS = jis2sjis(WORD(hex_code));
        if (is_sjis_code(wSJIS)) {
            szSJIS[0] = HIBYTE(wSJIS);
            szSJIS[1] = LOBYTE(wSJIS);
            szSJIS[2] = 0;
            ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
            node.post = szUnicode; // 変換後の文字列。
            node.deltaCost++; // コストを１つ加算。
            clause.add(&node);
        }
    }

    // 区点コードのノードを文節に追加。
    WORD wJIS = kuten_to_jis(strTyping);
    if (is_jis_code(wJIS)) {
        wSJIS = jis2sjis(wJIS);
        if (is_sjis_code(wSJIS)) {
            szSJIS[0] = HIBYTE(wSJIS);
            szSJIS[1] = LOBYTE(wSJIS);
            szSJIS[2] = 0;
            ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
            node.post = szUnicode; // 変換後の文字列。
            node.deltaCost++; // コストを１つ加算。
            clause.add(&node);
        }
    }

    // 元の入力文字列のノードを文節に追加。
    node.post = strTyping; // 変換後の文字列。
    node.deltaCost++; // コストを１つ加算。
    clause.add(&node);

    // 結果に文節情報をセット。
    result.clauses.clear();
    result.clauses.push_back(clause);

    return TRUE;
} // MzIme::ConvertCode

// コード変換。
BOOL MzIme::ConvertCode(LogCompStr& comp, LogCandInfo& cand)
{
    MzConvResult result;
    std::wstring strTyping = comp.extra.typing_clauses[comp.extra.iClause];
    if (!ConvertCode(strTyping, result)) {
        return FALSE;
    }
    return StoreResult(result, comp, cand);
} // MzIme::ConvertCode

// 結果を格納する。
BOOL MzIme::StoreResult(const MzConvResult& result, LogCompStr& comp, LogCandInfo& cand)
{
    // 未確定文字列をクリア。
    comp.comp_str.clear();
    comp.extra.clear();

    // 未確定文字列をセット。
    comp.comp_clause.resize(result.clauses.size() + 1);
    for (size_t iClause = 0; iClause < result.clauses.size(); ++iClause) {
        const MzConvClause& clause = result.clauses[iClause];
        for (auto& cand2 : clause.candidates) {
            comp.comp_clause[iClause] = (DWORD)comp.comp_str.size();
            comp.extra.hiragana_clauses.push_back(cand2.pre);
            std::wstring typing = hiragana_to_typing(cand2.pre);
            comp.extra.typing_clauses.push_back(typing);
            comp.comp_str += cand2.post;
            break;
        }
    }
    comp.comp_clause[result.clauses.size()] = (DWORD)comp.comp_str.size();
    comp.comp_attr.assign(comp.comp_str.size(), ATTR_CONVERTED);
    comp.extra.iClause = 0;
    comp.SetClauseAttr(comp.extra.iClause, ATTR_TARGET_CONVERTED);
    comp.dwCursorPos = (DWORD)comp.comp_str.size();
    comp.dwDeltaStart = 0;

    // 候補情報をセット。
    cand.clear();
    for (auto& clause : result.clauses) {
        LogCandList cand_list;
        for (auto& cand2 : clause.candidates) {
            cand_list.cand_strs.push_back(cand2.post);
        }
        cand.cand_lists.push_back(cand_list);
    }
    cand.iClause = 0;

    return TRUE;
} // MzIme::StoreResult

LPCTSTR KatsuyouToString(KatsuyouKei kk) {
    static const LPCWSTR s_array[] =
    {
        L"未然形",
        L"連用形",
        L"終止形",
        L"連体形",
        L"仮定形",
        L"命令形",
    };
    ASSERT(kk < _countof(s_array));
    return s_array[kk];
}

std::wstring MzConvResult::get_str(bool detailed) const
{
    std::wstring ret;
    size_t iClause = 0;
    for (auto& clause : clauses) {
        if (iClause)
            ret += L"|";
        if (clause.candidates.size() == 1 || !detailed) {
            ret += clause.candidates[0].post;
        } else {
            ret += L"(";
            size_t iCand = 0;
            for (auto& cand : clause.candidates) {
                if (iCand)
                    ret += L"|";
                ret += cand.post;
                ret += L":";
                if (cand.word_cost == MAXLONG)
                    ret += L"∞";
                else
                    ret += std::to_wstring(cand.word_cost);
                ret += L":";
                if (cand.cost == MAXLONG)
                    ret += L"∞";
                else
                    ret += std::to_wstring(cand.cost);
                ret += L":";
                ret += HinshiToString(cand.bunrui);
                if (cand.bunrui == HB_GODAN_DOUSHI) {
                    ret += L":";
                    ret += KatsuyouToString(cand.katsuyou);
                }
                ++iCand;
            }
            ret += L")";
        }
        ++iClause;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
