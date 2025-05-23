﻿#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif

// The separators.
// 辞書の区切り。
#define RECORD_SEP   L'\uFFFD'
#define FIELD_SEP    L'\uFFFC'

enum
{
    I_FIELD_PRE = 0,    // 変換前文字列。
    I_FIELD_HINSHI = 1, // { MAKEWORD(HinshiBunrui, Gyou) }
    I_FIELD_POST = 2,   // 変換後文字列。
    I_FIELD_TAGS = 3,   // タグ群。
    NUM_FIELDS          // １レコードあたりのフィールドの個数。
};

// Wide strings
typedef std::vector<std::wstring> WStrings;

// 行。
enum Gyou {
    GYOU_A,     // あ行。
    GYOU_KA,    // か行。
    GYOU_GA,    // が行。
    GYOU_SA,    // さ行。
    GYOU_ZA,    // ざ行。
    GYOU_TA,    // た行。
    GYOU_DA,    // だ行。
    GYOU_NA,    // な行。
    GYOU_HA,    // は行。
    GYOU_BA,    // ば行。
    GYOU_PA,    // ぱ行。
    GYOU_MA,    // ま行。
    GYOU_YA,    // や行。
    GYOU_RA,    // ら行。
    GYOU_WA,    // わ行。
    GYOU_NN     // ん行。
};

// 段。
enum Dan {
    DAN_A,      // あ段。
    DAN_I,      // い段。
    DAN_U,      // う段。
    DAN_E,      // え段。
    DAN_O       // お段。
};

// 品詞分類。
enum HinshiBunrui {
    HB_HEAD = 0x21,         // 最初のノード
    HB_TAIL,                // 最後のノード
    HB_UNKNOWN,             // 未知の品詞
    HB_MEISHI,              // 名詞
    HB_IKEIYOUSHI,          // い形容詞
    HB_NAKEIYOUSHI,         // な形容詞
    HB_RENTAISHI,           // 連体詞
    HB_FUKUSHI,             // 副詞
    HB_SETSUZOKUSHI,        // 接続詞
    HB_KANDOUSHI,           // 感動詞
    HB_KAKU_JOSHI,          // 格助詞
    HB_SETSUZOKU_JOSHI,     // 接続助詞
    HB_FUKU_JOSHI,          // 副助詞
    HB_SHUU_JOSHI,          // 終助詞
    HB_JODOUSHI,            // 助動詞
    HB_MIZEN_JODOUSHI,      // 未然助動詞
    HB_RENYOU_JODOUSHI,     // 連用助動詞
    HB_SHUUSHI_JODOUSHI,    // 終止助動詞
    HB_RENTAI_JODOUSHI,     // 連体助動詞
    HB_KATEI_JODOUSHI,      // 仮定助動詞
    HB_MEIREI_JODOUSHI,     // 命令助動詞
    HB_GODAN_DOUSHI,        // 五段動詞
    HB_ICHIDAN_DOUSHI,      // 一段動詞
    HB_KAHEN_DOUSHI,        // カ変動詞
    HB_SAHEN_DOUSHI,        // サ変動詞
    HB_KANGO,               // 漢語
    HB_SETTOUJI,            // 接頭辞
    HB_SETSUBIJI,           // 接尾辞
    HB_PERIOD,              // 句点（。）
    HB_COMMA,               // 読点（、）
    HB_SYMBOL,              // 記号類
    HB_MAX = HB_SYMBOL
}; // enum HinshiBunrui

// 動詞活用形。
enum KatsuyouKei {
    MIZEN_KEI,      // 未然形
    RENYOU_KEI,     // 連用形
    SHUUSHI_KEI,    // 終止形
    RENTAI_KEI,     // 連体形
    KATEI_KEI,      // 仮定形
    MEIREI_KEI      // 命令形
};

// 辞書の項目。
struct DictEntry {
    std::wstring pre;       // 変換前。
    std::wstring post;      // 変換後。
    HinshiBunrui bunrui;    // 品詞分類。
    std::wstring tags;      // タグ。
    Gyou gyou;              // 活用の行。
};
