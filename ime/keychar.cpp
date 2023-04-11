// keychar.cpp --- mzimeja keys and characters
// キー入力と文字。
// (Japanese, UTF-8)

#include "mzimeja.h"
#include "vksub.h"

//////////////////////////////////////////////////////////////////////////////

// 設定に応じて文字を変換する。
WCHAR translateChar(WCHAR ch, BOOL bCommaPeriod, BOOL bNoZenkakuAscii)
{
    if (is_comma(ch)) {
        if (bCommaPeriod) {
            if (Config_GetDWORD(L"bNoZenkakuAscii", FALSE))
                ch = L',';
            else
                ch = L'，';
        } else {
            ch = L'、';
        }
    } else if (is_period(ch)) {
        if (bCommaPeriod) {
            if (bNoZenkakuAscii)
                ch = L'.';
            else
                ch = L'．';
        } else {
            ch = L'。';
        }
    } else if (L'!' <= ch && ch <= L'~') {
        if (!bNoZenkakuAscii) {
            ch = (WCHAR)(ch + (L'！' - L'!'));
        }
    } else if (L'！' <= ch && ch <= L'～') {
        if (bNoZenkakuAscii) {
            ch = (WCHAR)(ch - (L'！' - L'!'));
        }
    }
    return ch;
}

// 設定に応じて文字を変換する。
WCHAR translateChar(WCHAR ch)
{
    return translateChar(ch,
                         Config_GetDWORD(L"bCommaPeriod", FALSE),
                         Config_GetDWORD(L"bNoZenkakuAscii", FALSE));
}

// 設定に応じて文字列を変換する。
std::wstring translateString(const std::wstring& str)
{
    BOOL bCommaPeriod = Config_GetDWORD(L"bCommaPeriod", FALSE);
    BOOL bNoZenkakuAscii = Config_GetDWORD(L"bNoZenkakuAscii", FALSE);

    std::wstring ret = str;
    for (WCHAR& ch : ret) {
        ch = translateChar(ch, bCommaPeriod, bNoZenkakuAscii);
    }
    return ret;
}

struct KEYVALUE {
    const WCHAR *key;
    const WCHAR *value;
};

struct KEYVALUEEXTRA {
    const WCHAR *key;
    const WCHAR *value;
    const WCHAR *extra;
};

// 全角ひらがなから半角への変換テーブル。
static KEYVALUE halfkana_table[] = {
    {L"゛", L"ﾞ"},
    {L"゜", L"ﾟ"},
    {L"ー", L"-"},
    {L"「", L"｢"},
    {L"」", L"｣"},
    {L"。", L"｡"},
    {L"、", L"､"},
    {L"・", L"･"},
    {L"う゛", L"ｳﾞ"},
    {L"あ", L"ｱ"},
    {L"い", L"ｲ"},
    {L"う", L"ｳ"},
    {L"え", L"ｴ"},
    {L"お", L"ｵ"},
    {L"ぁ", L"ｧ"},
    {L"ぃ", L"ｨ"},
    {L"ぅ", L"ｩ"},
    {L"ぇ", L"ｪ"},
    {L"ぉ", L"ｫ"},
    {L"か", L"ｶ"},
    {L"き", L"ｷ"},
    {L"く", L"ｸ"},
    {L"け", L"ｹ"},
    {L"こ", L"ｺ"},
    {L"が", L"ｶﾞ"},
    {L"ぎ", L"ｷﾞ"},
    {L"ぐ", L"ｸﾞ"},
    {L"げ", L"ｹﾞ"},
    {L"ご", L"ｺﾞ"},
    {L"さ", L"ｻ"},
    {L"し", L"ｼ"},
    {L"す", L"ｽ"},
    {L"せ", L"ｾ"},
    {L"そ", L"ｿ"},
    {L"ざ", L"ｻﾞ"},
    {L"じ", L"ｼﾞ"},
    {L"ず", L"ｽﾞ"},
    {L"ぜ", L"ｾﾞ"},
    {L"ぞ", L"ｿﾞ"},
    {L"た", L"ﾀ"},
    {L"ち", L"ﾁ"},
    {L"つ", L"ﾂ"},
    {L"て", L"ﾃ"},
    {L"と", L"ﾄ"},
    {L"だ", L"ﾀﾞ"},
    {L"ぢ", L"ﾁﾞ"},
    {L"づ", L"ﾂﾞ"},
    {L"で", L"ﾃﾞ"},
    {L"ど", L"ﾄﾞ"},
    {L"っ", L"ｯ"},
    {L"な", L"ﾅ"},
    {L"に", L"ﾆ"},
    {L"ぬ", L"ﾇ"},
    {L"ね", L"ﾈ"},
    {L"の", L"ﾉ"},
    {L"は", L"ﾊ"},
    {L"ひ", L"ﾋ"},
    {L"ふ", L"ﾌ"},
    {L"へ", L"ﾍ"},
    {L"ほ", L"ﾎ"},
    {L"ば", L"ﾊﾞ"},
    {L"び", L"ﾋﾞ"},
    {L"ぶ", L"ﾌﾞ"},
    {L"べ", L"ﾍﾞ"},
    {L"ぼ", L"ﾎﾞ"},
    {L"ぱ", L"ﾊﾟ"},
    {L"ぴ", L"ﾋﾟ"},
    {L"ぷ", L"ﾌﾟ"},
    {L"ぺ", L"ﾍﾟ"},
    {L"ぽ", L"ﾎﾟ"},
    {L"ま", L"ﾏ"},
    {L"み", L"ﾐ"},
    {L"む", L"ﾑ"},
    {L"め", L"ﾒ"},
    {L"も", L"ﾓ"},
    {L"ゃ", L"ｬ"},
    {L"や", L"ﾔ"},
    {L"ゅ", L"ｭ"},
    {L"ゆ", L"ﾕ"},
    {L"ょ", L"ｮ"},
    {L"よ", L"ﾖ"},
    {L"ら", L"ﾗ"},
    {L"り", L"ﾘ"},
    {L"る", L"ﾙ"},
    {L"れ", L"ﾚ"},
    {L"ろ", L"ﾛ"},
    {L"ゎ", L"ﾜ"},
    {L"わ", L"ﾜ"},
    {L"ゐ", L"ｲ"},
    {L"ゑ", L"ｴ"},
    {L"を", L"ｦ"},
    {L"ん", L"ﾝ"},
};

// 半角カナから全角ひらがなへの変換テーブル。
static KEYVALUE kana_table[] = {
    {L"ｰ", L"ー"},
    {L"ﾞ", L"゛"},
    {L"ﾟ", L"゜"},
    {L"｢", L"「"},
    {L"｣", L"」"},
    {L"｡", L"。"},
    {L"､", L"、"},
    {L"･", L"・"},
    {L"ｱ", L"あ"},
    {L"ｲ", L"い"},
    {L"ｳ", L"う"},
    {L"ｴ", L"え"},
    {L"ｵ", L"お"},
    {L"ｧ", L"ぁ"},
    {L"ｨ", L"ぃ"},
    {L"ｩ", L"ぅ"},
    {L"ｪ", L"ぇ"},
    {L"ｫ", L"ぉ"},
    {L"ｶ", L"か"},
    {L"ｷ", L"き"},
    {L"ｸ", L"く"},
    {L"ｹ", L"け"},
    {L"ｺ", L"こ"},
    {L"ｻ", L"さ"},
    {L"ｼ", L"し"},
    {L"ｽ", L"す"},
    {L"ｾ", L"せ"},
    {L"ｿ", L"そ"},
    {L"ﾀ", L"た"},
    {L"ﾁ", L"ち"},
    {L"ﾂ", L"つ"},
    {L"ﾃ", L"て"},
    {L"ﾄ", L"と"},
    {L"ｯ", L"っ"},
    {L"ﾅ", L"な"},
    {L"ﾆ", L"に"},
    {L"ﾇ", L"ぬ"},
    {L"ﾈ", L"ね"},
    {L"ﾉ", L"の"},
    {L"ﾊ", L"は"},
    {L"ﾋ", L"ひ"},
    {L"ﾌ", L"ふ"},
    {L"ﾍ", L"へ"},
    {L"ﾎ", L"ほ"},
    {L"ﾏ", L"ま"},
    {L"ﾐ", L"み"},
    {L"ﾑ", L"む"},
    {L"ﾒ", L"め"},
    {L"ﾓ", L"も"},
    {L"ｬ", L"ゃ"},
    {L"ﾔ", L"や"},
    {L"ｭ", L"ゅ"},
    {L"ﾕ", L"ゆ"},
    {L"ｮ", L"ょ"},
    {L"ﾖ", L"よ"},
    {L"ﾗ", L"ら"},
    {L"ﾘ", L"り"},
    {L"ﾙ", L"る"},
    {L"ﾚ", L"れ"},
    {L"ﾛ", L"ろ"},
    {L"ﾜ", L"わ"},
    {L"ｦ", L"を"},
    {L"ﾝ", L"ん"},
    {L"うﾞ", L"ヴ"},
    {L"かﾞ", L"が"},
    {L"きﾞ", L"ぎ"},
    {L"くﾞ", L"ぐ"},
    {L"けﾞ", L"げ"},
    {L"こﾞ", L"ご"},
    {L"さﾞ", L"ざ"},
    {L"しﾞ", L"じ"},
    {L"すﾞ", L"ず"},
    {L"せﾞ", L"ぜ"},
    {L"そﾞ", L"ぞ"},
    {L"たﾞ", L"だ"},
    {L"ちﾞ", L"ぢ"},
    {L"つﾞ", L"づ"},
    {L"てﾞ", L"で"},
    {L"とﾞ", L"ど"},
    {L"はﾞ", L"ば"},
    {L"ひﾞ", L"び"},
    {L"ふﾞ", L"ぶ"},
    {L"へﾞ", L"べ"},
    {L"ほﾞ", L"ぼ"},
    {L"はﾟ", L"ぱ"},
    {L"ひﾟ", L"ぴ"},
    {L"ふﾟ", L"ぷ"},
    {L"へﾟ", L"ぺ"},
    {L"ほﾟ", L"ぽ"},
};

// 記号の変換テーブル。
static KEYVALUE kigou_table[] = {
    //{L",", L"、"}, // bCommaPeriodで処理する。
    //{L".", L"。"}, // bCommaPeriodで処理する。
    {L"/", L"・"},
    {L"~", L"～"},
    {L"[", L"「"},
    {L"]", L"」"},
    {L"{", L"｛"},
    {L"}", L"｝"},
    {L":", L"："},
    {L";", L"；"},
    {L"<", L"＜"},
    {L"=", L"＝"},
    {L">", L"＞"},
    {L"?", L"？"},
    {L"@", L"＠"},
    {L"\\", L"￥"},
    {L"^", L"＾"},
    {L"_", L"＿"},
    {L"`", L"｀"},
    {L"|", L"｜"},
    {L"!", L"！"},
    {L"\"", L"″"},
    {L"#", L"＃"},
    {L"$", L"＄"},
    {L"%", L"％"},
    {L"&", L"＆"},
    {L"'", L"’"},
    {L"(", L"（"},
    {L")", L"）"},
    {L"*", L"＊"},
    {L"+", L"＋"},
    {L"-", L"ー"},
};

// ローマ字逆変換のテーブル。
static KEYVALUEEXTRA reverse_roman_table[] = {
    {L"う゛ぁ", L"va"},
    {L"う゛ぃ", L"vi"},
    {L"う゛", L"vu"},
    {L"う゛ぇ", L"ve"},
    {L"う゛ぉ", L"vo"},
    {L"っう゛", L"v", L"う゛"},
    {L"っぁ", L"xxa"},
    {L"っぃ", L"xxi"},
    {L"っぅ", L"xxu"},
    {L"っぇ", L"xxe"},
    {L"っぉ", L"xxo"},
    {L"っか", L"kka"},
    {L"っき", L"k", L"き"},
    {L"っく", L"kku"},
    {L"っけ", L"kke"},
    {L"っこ", L"kko"},
    {L"っが", L"gga"},
    {L"っぎ", L"g", L"ぎ"},
    {L"っぐ", L"ggu"},
    {L"っげ", L"gge"},
    {L"っご", L"ggo"},
    {L"っさ", L"ssa"},
    {L"っし", L"s", L"し"},
    {L"っす", L"ssu"},
    {L"っせ", L"sse"},
    {L"っそ", L"sso"},
    {L"っざ", L"zza"},
    {L"っじ", L"z", L"じ"},
    {L"っず", L"zzu"},
    {L"っぜ", L"zze"},
    {L"っぞ", L"zzo"},
    {L"った", L"tta"},
    {L"っち", L"t", L"ち"},
    {L"っつ", L"ttu"},
    {L"って", L"tte"},
    {L"っと", L"tto"},
    {L"っだ", L"dda"},
    {L"っぢ", L"d", L"ぢ"},
    {L"っづ", L"ddu"},
    {L"っで", L"dde"},
    {L"っど", L"ddo"},
    {L"っは", L"hha"},
    {L"っひ", L"h", L"ひ"},
    {L"っふ", L"hhu"},
    {L"っへ", L"hhe"},
    {L"っほ", L"hho"},
    {L"っば", L"bba"},
    {L"っび", L"b", L"び"},
    {L"っぶ", L"bbu"},
    {L"っべ", L"bbe"},
    {L"っぼ", L"bbo"},
    {L"っぱ", L"ppa"},
    {L"っぴ", L"p", L"ぴ"},
    {L"っぷ", L"ppu"},
    {L"っぺ", L"ppe"},
    {L"っぽ", L"ppo"},
    {L"っま", L"mma"},
    {L"っみ", L"m", L"み"},
    {L"っむ", L"mmu"},
    {L"っめ", L"mme"},
    {L"っも", L"mmo"},
    {L"っや", L"yya"},
    {L"っゆ", L"yyu"},
    {L"っよ", L"yyo"},
    {L"っゃ", L"xxya"},
    {L"っゅ", L"xxyu"},
    {L"っょ", L"xxyo"},
    {L"っら", L"rra"},
    {L"っり", L"r", L"り"},
    {L"っる", L"rru"},
    {L"っれ", L"rre"},
    {L"っろ", L"rro"},
    {L"っゎ", L"xxwa"},
    {L"っわ", L"wwa"},
    {L"っゐ", L"wwi"},
    {L"っゑ", L"wwe"},
    {L"っを", L"wwo"},
    {L"きゃ", L"kya"},
    {L"きぃ", L"kyi"},
    {L"きゅ", L"kyu"},
    {L"きぇ", L"kye"},
    {L"きょ", L"kyo"},
    {L"ぎゃ", L"gya"},
    {L"ぎぃ", L"gyi"},
    {L"ぎゅ", L"gyu"},
    {L"ぎぇ", L"gye"},
    {L"ぎょ", L"gyo"},
    {L"しゃ", L"sya"},
    {L"しぃ", L"syi"},
    {L"しゅ", L"syu"},
    {L"しぇ", L"sye"},
    {L"しょ", L"syo"},
    {L"じゃ", L"zya"},
    {L"じぃ", L"zyi"},
    {L"じゅ", L"zyu"},
    {L"じぇ", L"zye"},
    {L"じょ", L"zyo"},
    {L"ちゃ", L"tya"},
    {L"ちぃ", L"tyi"},
    {L"ちゅ", L"tyu"},
    {L"ちぇ", L"tye"},
    {L"ちょ", L"tyo"},
    {L"ぢゃ", L"dya"},
    {L"ぢぃ", L"dyi"},
    {L"ぢゅ", L"dyu"},
    {L"ぢぇ", L"dye"},
    {L"ぢょ", L"dyo"},
    {L"でぃ", L"dhi"},
    {L"にゃ", L"nya"},
    {L"にぃ", L"nyi"},
    {L"にゅ", L"nyu"},
    {L"にぇ", L"nye"},
    {L"にょ", L"nyo"},
    {L"ひゃ", L"hya"},
    {L"ひぃ", L"hyi"},
    {L"ひゅ", L"hyu"},
    {L"ひぇ", L"hye"},
    {L"ひょ", L"hyo"},
    {L"びゃ", L"bya"},
    {L"びぃ", L"byi"},
    {L"びゅ", L"byu"},
    {L"びぇ", L"bye"},
    {L"びょ", L"byo"},
    {L"ぴゃ", L"pya"},
    {L"ぴぃ", L"pyi"},
    {L"ぴゅ", L"pyu"},
    {L"ぴぇ", L"pye"},
    {L"ぴょ", L"pyo"},
    {L"みゃ", L"mya"},
    {L"みぃ", L"myi"},
    {L"みゅ", L"myu"},
    {L"みぇ", L"mye"},
    {L"みょ", L"myo"},
    {L"りゃ", L"rya"},
    {L"りぃ", L"ryi"},
    {L"りゅ", L"ryu"},
    {L"りぇ", L"rye"},
    {L"りょ", L"ryo"},
    {L"んあ", L"n'a"},
    {L"んい", L"n'i"},
    {L"んう", L"n'u"},
    {L"んえ", L"n'e"},
    {L"んお", L"n'o"},
    {L"あ", L"a"},
    {L"い", L"i"},
    {L"う", L"u"},
    {L"え", L"e"},
    {L"お", L"o"},
    {L"ぁ", L"xa"},
    {L"ぃ", L"xi"},
    {L"ぅ", L"xu"},
    {L"ぇ", L"xe"},
    {L"ぉ", L"xo"},
    {L"か", L"ka"},
    {L"き", L"ki"},
    {L"く", L"ku"},
    {L"け", L"ke"},
    {L"こ", L"ko"},
    {L"が", L"ga"},
    {L"ぎ", L"gi"},
    {L"ぐ", L"gu"},
    {L"げ", L"ge"},
    {L"ご", L"go"},
    {L"さ", L"sa"},
    {L"し", L"si"},
    {L"す", L"su"},
    {L"せ", L"se"},
    {L"そ", L"so"},
    {L"ざ", L"za"},
    {L"じ", L"zi"},
    {L"ず", L"zu"},
    {L"ぜ", L"ze"},
    {L"ぞ", L"zo"},
    {L"た", L"ta"},
    {L"ち", L"ti"},
    {L"つ", L"tu"},
    {L"て", L"te"},
    {L"と", L"to"},
    {L"だ", L"da"},
    {L"ぢ", L"di"},
    {L"づ", L"du"},
    {L"で", L"de"},
    {L"ど", L"do"},
    {L"っ", L"xtu"},
    {L"な", L"na"},
    {L"に", L"ni"},
    {L"ぬ", L"nu"},
    {L"ね", L"ne"},
    {L"の", L"no"},
    {L"は", L"ha"},
    {L"ひ", L"hi"},
    {L"ふ", L"hu"},
    {L"へ", L"he"},
    {L"ほ", L"ho"},
    {L"ば", L"ba"},
    {L"び", L"bi"},
    {L"ぶ", L"bu"},
    {L"べ", L"be"},
    {L"ぼ", L"bo"},
    {L"ぱ", L"pa"},
    {L"ぴ", L"pi"},
    {L"ぷ", L"pu"},
    {L"ぺ", L"pe"},
    {L"ぽ", L"po"},
    {L"ま", L"ma"},
    {L"み", L"mi"},
    {L"む", L"mu"},
    {L"め", L"me"},
    {L"も", L"mo"},
    {L"ゃ", L"xya"},
    {L"や", L"ya"},
    {L"ゅ", L"xyu"},
    {L"ゆ", L"yu"},
    {L"ょ", L"xyo"},
    {L"よ", L"yo"},
    {L"ら", L"ra"},
    {L"り", L"ri"},
    {L"る", L"ru"},
    {L"れ", L"re"},
    {L"ろ", L"ro"},
    {L"ゎ", L"xwa"},
    {L"わ", L"wa"},
    {L"ゐ", L"wi"},
    {L"ゑ", L"we"},
    {L"を", L"wo"},
    {L"ん", L"n"},
    {L"ー", L"-"},
    {L"～", L"~"},
    {L"「", L"["},
    {L"」", L"]"},
};

// ローマ字変換のテーブル。
static KEYVALUEEXTRA normal_roman_table[] = {
    {L"a", L"あ"},
    {L"i", L"い"},
    {L"u", L"う"},
    {L"e", L"え"},
    {L"o", L"お"},
    {L"ka", L"か"},
    {L"ki", L"き"},
    {L"ku", L"く"},
    {L"ke", L"け"},
    {L"ko", L"こ"},
    {L"sa", L"さ"},
    {L"si", L"し"},
    {L"su", L"す"},
    {L"se", L"せ"},
    {L"so", L"そ"},
    {L"ta", L"た"},
    {L"ti", L"ち"},
    {L"tu", L"つ"},
    {L"te", L"て"},
    {L"to", L"と"},
    {L"na", L"な"},
    {L"ni", L"に"},
    {L"nu", L"ぬ"},
    {L"ne", L"ね"},
    {L"no", L"の"},
    {L"ha", L"は"},
    {L"hi", L"ひ"},
    {L"hu", L"ふ"},
    {L"he", L"へ"},
    {L"ho", L"ほ"},
    {L"ma", L"ま"},
    {L"mi", L"み"},
    {L"mu", L"む"},
    {L"me", L"め"},
    {L"mo", L"も"},
    {L"ya", L"や"},
    {L"yi", L"い"},
    {L"yu", L"ゆ"},
    {L"ye", L"いぇ"},
    {L"yo", L"よ"},
    {L"ra", L"ら"},
    {L"ri", L"り"},
    {L"ru", L"る"},
    {L"re", L"れ"},
    {L"ro", L"ろ"},
    {L"wa", L"わ"},
    {L"wi", L"うぃ"},
    {L"wu", L"う"},
    {L"we", L"うぇ"},
    {L"wo", L"を"},
    {L"ga", L"が"},
    {L"gi", L"ぎ"},
    {L"gu", L"ぐ"},
    {L"ge", L"げ"},
    {L"go", L"ご"},
    {L"za", L"ざ"},
    {L"zi", L"じ"},
    {L"zu", L"ず"},
    {L"ze", L"ぜ"},
    {L"zo", L"ぞ"},
    {L"da", L"だ"},
    {L"di", L"ぢ"},
    {L"du", L"づ"},
    {L"de", L"で"},
    {L"do", L"ど"},
    {L"ba", L"ば"},
    {L"bi", L"び"},
    {L"bu", L"ぶ"},
    {L"be", L"べ"},
    {L"bo", L"ぼ"},
    {L"pa", L"ぱ"},
    {L"pi", L"ぴ"},
    {L"pu", L"ぷ"},
    {L"pe", L"ぺ"},
    {L"po", L"ぽ"},
    {L"fa", L"ふぁ"},
    {L"fi", L"ふぃ"},
    {L"fu", L"ふ"},
    {L"fe", L"ふぇ"},
    {L"fo", L"ふぉ"},
    {L"ja", L"じゃ"},
    {L"ji", L"じ"},
    {L"ju", L"じゅ"},
    {L"je", L"じぇ"},
    {L"jo", L"じょ"},
    {L"ca", L"か"},
    {L"ci", L"し"},
    {L"cu", L"く"},
    {L"ce", L"せ"},
    {L"co", L"こ"},
    {L"qa", L"くぁ"},
    {L"qi", L"くぃ"},
    {L"qu", L"く"},
    {L"qe", L"くぇ"},
    {L"qo", L"くぉ"},
    {L"va", L"ヴぁ"},
    {L"vi", L"ヴぃ"},
    {L"vu", L"ヴ"},
    {L"ve", L"ヴぇ"},
    {L"vo", L"ヴぉ"},
    {L"kya", L"きゃ"},
    {L"kyi", L"きぃ"},
    {L"kyu", L"きゅ"},
    {L"kye", L"きぇ"},
    {L"kyo", L"きょ"},
    {L"gya", L"ぎゃ"},
    {L"gyi", L"ぎぃ"},
    {L"gyu", L"ぎゅ"},
    {L"gye", L"ぎぇ"},
    {L"gyo", L"ぎょ"},
    {L"sya", L"しゃ"},
    {L"syi", L"しぃ"},
    {L"syu", L"しゅ"},
    {L"sye", L"しぇ"},
    {L"syo", L"しょ"},
    {L"zya", L"じゃ"},
    {L"zyi", L"じぃ"},
    {L"zyu", L"じゅ"},
    {L"zye", L"じぇ"},
    {L"zyo", L"じょ"},
    {L"tya", L"ちゃ"},
    {L"tyi", L"ちぃ"},
    {L"tyu", L"ちゅ"},
    {L"tye", L"ちぇ"},
    {L"tyo", L"ちょ"},
    {L"dya", L"ぢゃ"},
    {L"dyi", L"ぢぃ"},
    {L"dyu", L"ぢゅ"},
    {L"dye", L"ぢぇ"},
    {L"dyo", L"ぢょ"},
    {L"nya", L"にゃ"},
    {L"nyi", L"にぃ"},
    {L"nyu", L"にゅ"},
    {L"nye", L"にぇ"},
    {L"nyo", L"にょ"},
    {L"hya", L"ひゃ"},
    {L"hyi", L"ひぃ"},
    {L"hyu", L"ひゅ"},
    {L"hye", L"ひぇ"},
    {L"hyo", L"ひょ"},
    {L"bya", L"びゃ"},
    {L"byi", L"びぃ"},
    {L"byu", L"びゅ"},
    {L"bye", L"びぇ"},
    {L"byo", L"びょ"},
    {L"pya", L"ぴゃ"},
    {L"pyi", L"ぴぃ"},
    {L"pyu", L"ぴゅ"},
    {L"pye", L"ぴぇ"},
    {L"pyo", L"ぴょ"},
    {L"mya", L"みゃ"},
    {L"myi", L"みぃ"},
    {L"myu", L"みゅ"},
    {L"mye", L"みぇ"},
    {L"myo", L"みょ"},
    {L"rya", L"りゃ"},
    {L"ryi", L"りぃ"},
    {L"ryu", L"りゅ"},
    {L"rye", L"りぇ"},
    {L"ryo", L"りょ"},
    {L"jya", L"じゃ"},
    {L"jyi", L"じぃ"},
    {L"jyu", L"じゅ"},
    {L"jye", L"じぇ"},
    {L"jyo", L"じょ"},
    {L"vya", L"ヴゃ"},
    {L"vyi", L"ヴぃ"},
    {L"vyu", L"ヴゅ"},
    {L"vye", L"ヴぇ"},
    {L"vyo", L"ヴょ"},
    {L"qya", L"くゃ"},
    {L"qyi", L"くぃ"},
    {L"qyu", L"くゅ"},
    {L"qye", L"くぇ"},
    {L"qyo", L"くょ"},
    {L"cya", L"ちゃ"},
    {L"cyi", L"ちぃ"},
    {L"cyu", L"ちゅ"},
    {L"cye", L"ちぇ"},
    {L"cyo", L"ちょ"},
    {L"fya", L"ふゃ"},
    {L"fyi", L"ふぃ"},
    {L"fyu", L"ふゅ"},
    {L"fye", L"ふぇ"},
    {L"fyo", L"ふょ"},
    {L"sha", L"しゃ"},
    {L"shi", L"し"},
    {L"shu", L"しゅ"},
    {L"she", L"しぇ"},
    {L"sho", L"しょ"},
    {L"cha", L"ちゃ"},
    {L"chi", L"ち"},
    {L"chu", L"ちゅ"},
    {L"che", L"ちぇ"},
    {L"cho", L"ちょ"},
    {L"tha", L"てゃ"},
    {L"thi", L"てぃ"},
    {L"thu", L"てゅ"},
    {L"the", L"てぇ"},
    {L"tho", L"てょ"},
    {L"dha", L"でゃ"},
    {L"dhi", L"でぃ"},
    {L"dhu", L"でゅ"},
    {L"dhe", L"でぇ"},
    {L"dho", L"でょ"},
    {L"wha", L"うぁ"},
    {L"whi", L"うぃ"},
    {L"whu", L"う"},
    {L"whe", L"うぇ"},
    {L"who", L"うぉ"},
    {L"kwa", L"くぁ"},
    {L"kwi", L"くぃ"},
    {L"kwu", L"くぅ"},
    {L"kwe", L"くぇ"},
    {L"kwo", L"くぉ"},
    {L"qwa", L"くぁ"},
    {L"qwi", L"くぃ"},
    {L"qwu", L"くぅ"},
    {L"qwe", L"くぇ"},
    {L"qwo", L"くぉ"},
    {L"gwa", L"ぐぁ"},
    {L"gwi", L"ぐぃ"},
    {L"gwu", L"ぐぅ"},
    {L"gwe", L"ぐぇ"},
    {L"gwo", L"ぐぉ"},
    {L"swa", L"すぁ"},
    {L"swi", L"すぃ"},
    {L"swu", L"すぅ"},
    {L"swe", L"すぇ"},
    {L"swo", L"すぉ"},
    {L"twa", L"とぁ"},
    {L"twi", L"とぃ"},
    {L"twu", L"とぅ"},
    {L"twe", L"とぇ"},
    {L"two", L"とぉ"},
    {L"dwa", L"どぁ"},
    {L"dwi", L"どぃ"},
    {L"dwu", L"どぅ"},
    {L"dwe", L"どぇ"},
    {L"dwo", L"どぉ"},
    {L"fwa", L"ふぁ"},
    {L"fwi", L"ふぃ"},
    {L"fwu", L"ふぅ"},
    {L"fwe", L"ふぇ"},
    {L"fwo", L"ふぉ"},
    {L"tsa", L"つぁ"},
    {L"tsi", L"つぃ"},
    {L"tsu", L"つ"},
    {L"tse", L"つぇ"},
    {L"tso", L"つぉ"},
    {L"la", L"ぁ"},
    {L"li", L"ぃ"},
    {L"lu", L"ぅ"},
    {L"le", L"ぇ"},
    {L"lo", L"ぉ"},
    {L"lya", L"ゃ"},
    {L"lyi", L"ぃ"},
    {L"lyu", L"ゅ"},
    {L"lye", L"ぇ"},
    {L"lyo", L"ょ"},
    {L"xa", L"ぁ"},
    {L"xi", L"ぃ"},
    {L"xu", L"ぅ"},
    {L"xe", L"ぇ"},
    {L"xo", L"ぉ"},
    {L"xya", L"ゃ"},
    {L"xyi", L"ぃ"},
    {L"xyu", L"ゅ"},
    {L"xye", L"ぇ"},
    {L"xyo", L"ょ"},
    {L"lwa", L"ゎ"},
    {L"xwa", L"ゎ"},
    {L"lka", L"ヵ"},
    {L"xka", L"ヵ"},
    {L"lke", L"ヶ"},
    {L"xke", L"ヶ"},
    {L"ltu", L"っ"},
    {L"ltsu", L"っ"},
    {L"xtu", L"っ"},
    {L"nn", L"ん"},
    {L"n’", L"ん"},
    {L"xn", L"ん"},
    {L"nb", L"ん", L"b"},
    {L"nc", L"ん", L"c"},
    {L"nd", L"ん", L"d"},
    {L"nf", L"ん", L"f"},
    {L"ng", L"ん", L"g"},
    {L"nh", L"ん", L"h"},
    {L"nj", L"ん", L"j"},
    {L"nk", L"ん", L"k"},
    {L"nl", L"ん", L"l"},
    {L"nm", L"ん", L"m"},
    {L"np", L"ん", L"p"},
    {L"nq", L"ん", L"q"},
    {L"nr", L"ん", L"r"},
    {L"ns", L"ん", L"s"},
    {L"nt", L"ん", L"t"},
    {L"nv", L"ん", L"v"},
    {L"nw", L"ん", L"w"},
    {L"nx", L"ん", L"x"},
    {L"nz", L"ん", L"z"},
    {L"n-", L"ん", L"-"},
    {L"n@", L"ん", L"@"},
};

// 促音テーブル。
static KEYVALUE sokuon_table[] = {
    {L"kka", L"っか"},
    {L"kki", L"っき"},
    {L"kku", L"っく"},
    {L"kke", L"っけ"},
    {L"kko", L"っこ"},
    {L"ssa", L"っさ"},
    {L"ssi", L"っし"},
    {L"ssu", L"っす"},
    {L"sse", L"っせ"},
    {L"sso", L"っそ"},
    {L"tta", L"った"},
    {L"tti", L"っち"},
    {L"ttu", L"っつ"},
    {L"tte", L"って"},
    {L"tto", L"っと"},
    {L"hha", L"っは"},
    {L"hhi", L"っひ"},
    {L"hhu", L"っふ"},
    {L"hhe", L"っへ"},
    {L"hho", L"っほ"},
    {L"mma", L"っま"},
    {L"mmi", L"っみ"},
    {L"mmu", L"っむ"},
    {L"mme", L"っめ"},
    {L"mmo", L"っも"},
    {L"yya", L"っや"},
    {L"yyi", L"っい"},
    {L"yyu", L"っゆ"},
    {L"yye", L"っいぇ"},
    {L"yyo", L"っよ"},
    {L"rra", L"っら"},
    {L"rri", L"っり"},
    {L"rru", L"っる"},
    {L"rre", L"っれ"},
    {L"rro", L"っろ"},
    {L"wwa", L"っわ"},
    {L"wwi", L"っうぃ"},
    {L"wwu", L"っう"},
    {L"wwe", L"っうぇ"},
    {L"wwo", L"っを"},
    {L"gga", L"っが"},
    {L"ggi", L"っぎ"},
    {L"ggu", L"っぐ"},
    {L"gge", L"っげ"},
    {L"ggo", L"っご"},
    {L"zza", L"っざ"},
    {L"zzi", L"っじ"},
    {L"zzu", L"っず"},
    {L"zze", L"っぜ"},
    {L"zzo", L"っぞ"},
    {L"dda", L"っだ"},
    {L"ddi", L"っぢ"},
    {L"ddu", L"っづ"},
    {L"dde", L"っで"},
    {L"ddo", L"っど"},
    {L"bba", L"っば"},
    {L"bbi", L"っび"},
    {L"bbu", L"っぶ"},
    {L"bbe", L"っべ"},
    {L"bbo", L"っぼ"},
    {L"ppa", L"っぱ"},
    {L"ppi", L"っぴ"},
    {L"ppu", L"っぷ"},
    {L"ppe", L"っぺ"},
    {L"ppo", L"っぽ"},
    {L"ffa", L"っふぁ"},
    {L"ffi", L"っふぃ"},
    {L"ffu", L"っふ"},
    {L"ffe", L"っふぇ"},
    {L"ffo", L"っふぉ"},
    {L"jja", L"っじゃ"},
    {L"jji", L"っじ"},
    {L"jju", L"っじゅ"},
    {L"jje", L"っじぇ"},
    {L"jjo", L"っじょ"},
    {L"cca", L"っか"},
    {L"cci", L"っし"},
    {L"ccu", L"っく"},
    {L"cce", L"っせ"},
    {L"cco", L"っこ"},
    {L"qqa", L"っくぁ"},
    {L"qqi", L"っくぃ"},
    {L"qqu", L"っく"},
    {L"qqe", L"っくぇ"},
    {L"qqo", L"っくぉ"},
    {L"vva", L"っヴぁ"},
    {L"vvi", L"っヴぃ"},
    {L"vvu", L"っヴ"},
    {L"vve", L"っヴぇ"},
    {L"vvo", L"っヴぉ"},
    {L"kkya", L"っきゃ"},
    {L"kkyi", L"っきぃ"},
    {L"kkyu", L"っきゅ"},
    {L"kkye", L"っきぇ"},
    {L"kkyo", L"っきょ"},
    {L"ggya", L"っぎゃ"},
    {L"ggyi", L"っぎぃ"},
    {L"ggyu", L"っぎゅ"},
    {L"ggye", L"っぎぇ"},
    {L"ggyo", L"っぎょ"},
    {L"ssya", L"っしゃ"},
    {L"ssyi", L"っしぃ"},
    {L"ssyu", L"っしゅ"},
    {L"ssye", L"っしぇ"},
    {L"ssyo", L"っしょ"},
    {L"zzya", L"っじゃ"},
    {L"zzyi", L"っじぃ"},
    {L"zzyu", L"っじゅ"},
    {L"zzye", L"っじぇ"},
    {L"zzyo", L"っじょ"},
    {L"ttya", L"っちゃ"},
    {L"ttyi", L"っちぃ"},
    {L"ttyu", L"っちゅ"},
    {L"ttye", L"っちぇ"},
    {L"ttyo", L"っちょ"},
    {L"ddya", L"っぢゃ"},
    {L"ddyi", L"っぢぃ"},
    {L"ddyu", L"っぢゅ"},
    {L"ddye", L"っぢぇ"},
    {L"ddyo", L"っぢょ"},
    {L"hhya", L"っひゃ"},
    {L"hhyi", L"っひぃ"},
    {L"hhyu", L"っひゅ"},
    {L"hhye", L"っひぇ"},
    {L"hhyo", L"っひょ"},
    {L"bbya", L"っびゃ"},
    {L"bbyi", L"っびぃ"},
    {L"bbyu", L"っびゅ"},
    {L"bbye", L"っびぇ"},
    {L"bbyo", L"っびょ"},
    {L"ppya", L"っぴゃ"},
    {L"ppyi", L"っぴぃ"},
    {L"ppyu", L"っぴゅ"},
    {L"ppye", L"っぴぇ"},
    {L"ppyo", L"っぴょ"},
    {L"mmya", L"っみゃ"},
    {L"mmyi", L"っみぃ"},
    {L"mmyu", L"っみゅ"},
    {L"mmye", L"っみぇ"},
    {L"mmyo", L"っみょ"},
    {L"rrya", L"っりゃ"},
    {L"rryi", L"っりぃ"},
    {L"rryu", L"っりゅ"},
    {L"rrye", L"っりぇ"},
    {L"rryo", L"っりょ"},
    {L"jjya", L"っじゃ"},
    {L"jjyi", L"っじぃ"},
    {L"jjyu", L"っじゅ"},
    {L"jjye", L"っじぇ"},
    {L"jjyo", L"っじょ"},
    {L"vvya", L"っヴゃ"},
    {L"vvyi", L"っヴぃ"},
    {L"vvyu", L"っヴゅ"},
    {L"vvye", L"っヴぇ"},
    {L"vvyo", L"っヴょ"},
    {L"qqya", L"っくゃ"},
    {L"qqyi", L"っくぃ"},
    {L"qqyu", L"っくゅ"},
    {L"qqye", L"っくぇ"},
    {L"qqyo", L"っくょ"},
    {L"ccya", L"っちゃ"},
    {L"ccyi", L"っちぃ"},
    {L"ccyu", L"っちゅ"},
    {L"ccye", L"っちぇ"},
    {L"ccyo", L"っちょ"},
    {L"ffya", L"っふゃ"},
    {L"ffyi", L"っふぃ"},
    {L"ffyu", L"っふゅ"},
    {L"ffye", L"っふぇ"},
    {L"ffyo", L"っふょ"},
    {L"ssha", L"っしゃ"},
    {L"sshi", L"っし"},
    {L"sshu", L"っしゅ"},
    {L"sshe", L"っしぇ"},
    {L"ssho", L"っしょ"},
    {L"ccha", L"っちゃ"},
    {L"cchi", L"っち"},
    {L"cchu", L"っちゅ"},
    {L"cche", L"っちぇ"},
    {L"ccho", L"っちょ"},
    {L"ttha", L"ってゃ"},
    {L"tthi", L"ってぃ"},
    {L"tthu", L"ってゅ"},
    {L"tthe", L"ってぇ"},
    {L"ttho", L"ってょ"},
    {L"ddha", L"っでゃ"},
    {L"ddhi", L"っでぃ"},
    {L"ddhu", L"っでゅ"},
    {L"ddhe", L"っでぇ"},
    {L"ddho", L"っでょ"},
    {L"wwha", L"っうぁ"},
    {L"wwhi", L"っうぃ"},
    {L"wwhu", L"っう"},
    {L"wwhe", L"っうぇ"},
    {L"wwho", L"っうぉ"},
    {L"kkwa", L"っくぁ"},
    {L"kkwi", L"っくぃ"},
    {L"kkwu", L"っくぅ"},
    {L"kkwe", L"っくぇ"},
    {L"kkwo", L"っくぉ"},
    {L"qqwa", L"っくぁ"},
    {L"qqwi", L"っくぃ"},
    {L"qqwu", L"っくぅ"},
    {L"qqwe", L"っくぇ"},
    {L"qqwo", L"っくぉ"},
    {L"ggwa", L"っぐぁ"},
    {L"ggwi", L"っぐぃ"},
    {L"ggwu", L"っぐぅ"},
    {L"ggwe", L"っぐぇ"},
    {L"ggwo", L"っぐぉ"},
    {L"sswa", L"っすぁ"},
    {L"sswi", L"っすぃ"},
    {L"sswu", L"っすぅ"},
    {L"sswe", L"っすぇ"},
    {L"sswo", L"っすぉ"},
    {L"ttwa", L"っとぁ"},
    {L"ttwi", L"っとぃ"},
    {L"ttwu", L"っとぅ"},
    {L"ttwe", L"っとぇ"},
    {L"ttwo", L"っとぉ"},
    {L"ddwa", L"っどぁ"},
    {L"ddwi", L"っどぃ"},
    {L"ddwu", L"っどぅ"},
    {L"ddwe", L"っどぇ"},
    {L"ddwo", L"っどぉ"},
    {L"ffwa", L"っふぁ"},
    {L"ffwi", L"っふぃ"},
    {L"ffwu", L"っふぅ"},
    {L"ffwe", L"っふぇ"},
    {L"ffwo", L"っふぉ"},
    {L"ttsa", L"っつぁ"},
    {L"ttsi", L"っつぃ"},
    {L"ttsu", L"っつ"},
    {L"ttse", L"っつぇ"},
    {L"ttso", L"っつぉ"},
    {L"lla", L"っぁ"},
    {L"lli", L"っぃ"},
    {L"llu", L"っぅ"},
    {L"lle", L"っぇ"},
    {L"llo", L"っぉ"},
    {L"llya", L"っゃ"},
    {L"llyi", L"っぃ"},
    {L"llyu", L"っゅ"},
    {L"llye", L"っぇ"},
    {L"llyo", L"っょ"},
    {L"xxa", L"っぁ"},
    {L"xxi", L"っぃ"},
    {L"xxu", L"っぅ"},
    {L"xxe", L"っぇ"},
    {L"xxo", L"っぉ"},
    {L"xxya", L"っゃ"},
    {L"xxyi", L"っぃ"},
    {L"xxyu", L"っゅ"},
    {L"xxye", L"っぇ"},
    {L"xxyo", L"っょ"},
    {L"llwa", L"っゎ"},
    {L"xxwa", L"っゎ"},
    {L"llka", L"っヵ"},
    {L"xxka", L"っヵ"},
    {L"llke", L"っヶ"},
    {L"xxke", L"っヶ"},
    {L"lltu", L"っっ"},
    {L"lltsu", L"っっ"},
    {L"xxtu", L"っっ"},
    {L"xxn", L"っん"},
};

// カナ入力テーブル。
static KEYVALUE kana2type_table[] = {
    {L"あ", L"3"},
    {L"い", L"e"},
    {L"う", L"4"},
    {L"え", L"5"},
    {L"お", L"6"},
    {L"か", L"t"},
    {L"き", L"g"},
    {L"く", L"h"},
    {L"け", L":"},
    {L"こ", L"b"},
    {L"さ", L"x"},
    {L"し", L"d"},
    {L"す", L"r"},
    {L"せ", L"p"},
    {L"そ", L"c"},
    {L"た", L"q"},
    {L"ち", L"a"},
    {L"つ", L"z"},
    {L"て", L"w"},
    {L"と", L"s"},
    {L"な", L"u"},
    {L"に", L"i"},
    {L"ぬ", L"1"},
    {L"ね", L","},
    {L"の", L"k"},
    {L"は", L"f"},
    {L"ひ", L"v"},
    {L"ふ", L"2"},
    {L"へ", L"^"},
    {L"ほ", L"-"},
    {L"ま", L"j"},
    {L"み", L"n"},
    {L"む", L"]"},
    {L"め", L"/"},
    {L"も", L"m"},
    {L"や", L"7"},
    {L"ゆ", L"8"},
    {L"よ", L"9"},
    {L"ら", L"o"},
    {L"り", L"l"},
    {L"る", L"."},
    {L"れ", L";"},
    {L"ろ", L"\\"},
    {L"わ", L"0"},
    {L"を", L"0"},
    {L"ん", L"y"},
    {L"が", L"t@"},
    {L"ぎ", L"g@"},
    {L"ぐ", L"h@"},
    {L"げ", L":@"},
    {L"ご", L"b@"},
    {L"ざ", L"x@"},
    {L"じ", L"d@"},
    {L"ず", L"r@"},
    {L"ぜ", L"p@"},
    {L"ぞ", L"c@"},
    {L"だ", L"q@"},
    {L"ぢ", L"a@"},
    {L"づ", L"z@"},
    {L"で", L"w@"},
    {L"ど", L"s@"},
    {L"ば", L"f@"},
    {L"び", L"v@"},
    {L"ぶ", L"2@"},
    {L"べ", L"^@"},
    {L"ぼ", L"-@"},
    {L"ぱ", L"f["},
    {L"ぴ", L"v["},
    {L"ぷ", L"2["},
    {L"ぺ", L"^["},
    {L"ぽ", L"-["},
    {L"ぁ", L"#"},
    {L"ぃ", L"E"},
    {L"ぅ", L"$"},
    {L"ぇ", L"%"},
    {L"ぉ", L"&"},
    {L"ゎ", L"0"},
    {L"っ", L"Z"},
    {L"゛", L"@"},
    {L"゜", L"["},
    {L"ー", L"-"},
};

//////////////////////////////////////////////////////////////////////////////

// ひらがなからローマ字へ文字列を変換。
std::wstring hiragana_to_roman(std::wstring hiragana)
{
    std::wstring roman;
    bool found;
    for (size_t k = 0; k < hiragana.size(); ) {
        found = false;
        if (!found) {
            for (size_t i = 0; i < _countof(sokuon_table); ++i) {
                if (hiragana[k] == sokuon_table[i].value[0]) {
                    if (hiragana.find(sokuon_table[i].value, k) == k) {
                        k += wcslen(sokuon_table[i].value);
                        roman += sokuon_table[i].key;
                        found = true;
                        break;
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(reverse_roman_table); ++i) {
                if (reverse_roman_table[i].extra) continue;
                if (hiragana[k] == reverse_roman_table[i].key[0]) {
                    if (hiragana.find(reverse_roman_table[i].key, k) == k) {
                        k += wcslen(reverse_roman_table[i].key);
                        roman += reverse_roman_table[i].value;
                        found = true;
                        break;
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(normal_roman_table); ++i) {
                if (normal_roman_table[i].extra) continue;
                if (hiragana[k] == normal_roman_table[i].value[0]) {
                    if (hiragana.find(normal_roman_table[i].value, k) == k) {
                        k += wcslen(normal_roman_table[i].value);
                        roman += normal_roman_table[i].key;
                        found = true;
                        break;
                    }
                }
            }
        }
        if (!found) roman += hiragana[k++];
    }
    return roman;
} // hiragana_to_roman

// ローマ字からひらがなへ文字列を変換。
std::wstring roman_to_hiragana(std::wstring roman)
{
    std::wstring hiragana, str;
    roman = lcmap(roman, LCMAP_HALFWIDTH); // 事前に半角にしておく。
    for (size_t k = 0; k < roman.size(); ) {
        bool found = false;
        if (!found) {
            for (size_t i = 0; i < _countof(sokuon_table); ++i) {
                if (roman[k] == sokuon_table[i].key[0]) {
                    if (roman.find(sokuon_table[i].key, k) == k) {
                        k += wcslen(sokuon_table[i].key);
                        hiragana += sokuon_table[i].value;
                        found = true;
                        break;
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(normal_roman_table); ++i) {
                if (normal_roman_table[i].extra) {
                    if (roman[k] == normal_roman_table[i].key[0]) {
                        if (roman.find(normal_roman_table[i].key, k) == k) {
                            roman.insert(k, normal_roman_table[i].extra);
                            k += wcslen(normal_roman_table[i].key);
                            hiragana += normal_roman_table[i].value;
                            found = true;
                            break;
                        }
                    }
                } else {
                    if (roman[k] == normal_roman_table[i].key[0]) {
                        if (roman.find(normal_roman_table[i].key, k) == k) {
                            k += wcslen(normal_roman_table[i].key);
                            hiragana += normal_roman_table[i].value;
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(kigou_table); ++i) {
                if (roman[k] == kigou_table[i].key[0]) {
                    hiragana += kigou_table[i].value;
                    k += 1;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            if (roman[k] == L',') {
                if (Config_GetDWORD(L"bCommaPeriod", FALSE))
                    hiragana += L'，';
                else
                    hiragana += L'、';
                k += 1;
                found = true;
            }
            else if (roman[k] == L'.') {
                if (Config_GetDWORD(L"bCommaPeriod", FALSE))
                    hiragana += L'．';
                else
                    hiragana += L'。';
                k += 1;
                found = true;
            }
        }
        if (!found) hiragana += roman[k++];
    }
    return hiragana;
} // roman_to_hiragana

// ローマ字からカタカナへ文字列を変換。
std::wstring roman_to_katakana(std::wstring roman)
{
    std::wstring katakana, str;
    for (size_t k = 0; k < roman.size(); ) {
        bool found = false;
        if (!found) {
            for (size_t i = 0; i < _countof(sokuon_table); ++i) {
                if (roman[k] == sokuon_table[i].key[0]) {
                    if (roman.find(sokuon_table[i].key, k) == k) {
                        k += wcslen(sokuon_table[i].key);
                        katakana += lcmap(sokuon_table[i].value, LCMAP_KATAKANA);
                        found = true;
                        break;
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(normal_roman_table); ++i) {
                if (normal_roman_table[i].extra) {
                    if (roman[k] == normal_roman_table[i].key[0]) {
                        if (roman.find(normal_roman_table[i].key, k) == k) {
                            roman.insert(k, normal_roman_table[i].extra);
                            k += wcslen(normal_roman_table[i].key);
                            katakana += lcmap(normal_roman_table[i].value, LCMAP_KATAKANA);
                            found = true;
                            break;
                        }
                    }
                } else {
                    if (roman[k] == normal_roman_table[i].key[0]) {
                        if (roman.find(normal_roman_table[i].key, k) == k) {
                            k += wcslen(normal_roman_table[i].key);
                            katakana += lcmap(normal_roman_table[i].value, LCMAP_KATAKANA);
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(kigou_table); ++i) {
                if (roman[k] == kigou_table[i].key[0]) {
                    katakana += kigou_table[i].value;
                    k += 1;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            if (roman[k] == L',') {
                if (Config_GetDWORD(L"bCommaPeriod", FALSE))
                    katakana += L'，';
                else
                    katakana += L'、';
                k += 1;
                found = true;
            }
            else if (roman[k] == L'.') {
                if (Config_GetDWORD(L"bCommaPeriod", FALSE))
                    katakana += L'．';
                else
                    katakana += L'。';
                k += 1;
                found = true;
            }
        }
        if (!found) katakana += roman[k++];
    }
    return katakana;
} // roman_to_katakana

// ローマ字から半角カナへ文字列を変換。
std::wstring roman_to_halfwidth_katakana(std::wstring roman)
{
    std::wstring katakana, str;
    //DPRINTW(L"roman_to_halfwidth_katakana(%s)\n", roman.c_str());
    for (size_t k = 0; k < roman.size(); ) {
        bool found = false;
        if (!found) {
            for (size_t i = 0; i < _countof(sokuon_table); ++i) {
                if (roman[k] == sokuon_table[i].key[0]) {
                    if (roman.find(sokuon_table[i].key, k) == k) {
                        k += wcslen(sokuon_table[i].key);
                        katakana += lcmap(sokuon_table[i].value,
                                          LCMAP_HALFWIDTH | LCMAP_KATAKANA);
                        found = true;
                        break;
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(normal_roman_table); ++i) {
                if (normal_roman_table[i].extra) {
                    if (roman[k] == normal_roman_table[i].key[0]) {
                        if (roman.find(normal_roman_table[i].key, k) == k) {
                            roman.insert(k, normal_roman_table[i].extra);
                            k += wcslen(normal_roman_table[i].key);
                            katakana += lcmap(normal_roman_table[i].value,
                                              LCMAP_HALFWIDTH | LCMAP_KATAKANA);
                            found = true;
                            break;
                        }
                    }
                } else {
                    if (roman[k] == normal_roman_table[i].key[0]) {
                        if (roman.find(normal_roman_table[i].key, k) == k) {
                            k += wcslen(normal_roman_table[i].key);
                            katakana += lcmap(normal_roman_table[i].value,
                                              LCMAP_HALFWIDTH | LCMAP_KATAKANA);
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            for (size_t i = 0; i < _countof(kigou_table); ++i) {
                if (roman[k] == kigou_table[i].key[0]) {
                    katakana += lcmap(kigou_table[i].value,
                                      LCMAP_HALFWIDTH | LCMAP_KATAKANA);
                    k += 1;
                    found = true;
                    break;
                }
            }
        }
        if (!found) katakana += roman[k++];
    }
    //DPRINTW(L"roman_to_halfwidth_katakana: %s\n", katakana.c_str());
    return katakana;
} // roman_to_halfwidth_katakana

// ローマ字からひらがなへ文字列を変換。
std::wstring roman_to_hiragana(std::wstring roman, size_t ichTarget)
{
    std::wstring str, key, value, extra;
    size_t key_len = 0;
    for (size_t i = 0; i < _countof(sokuon_table); ++i) {
        str = sokuon_table[i].key;
        key_len = str.size();
        if (key_len <= ichTarget) {
            if (roman.substr(ichTarget - key_len, key_len) == str) {
                if (key_len > key.size()) {
                    key = str;
                    value = sokuon_table[i].value;
                }
            }
        }
    }
    for (size_t i = 0; i < _countof(normal_roman_table); ++i) {
        if (normal_roman_table[i].extra) {
            str = normal_roman_table[i].key;
            key_len = str.size();
            if (key_len <= ichTarget) {
                if (roman.substr(ichTarget - key_len, key_len) == str) {
                    if (key_len > key.size()) {
                        key = str;
                        value = normal_roman_table[i].value;
                        extra = normal_roman_table[i].extra;
                    }
                }
            }
        } else {
            str = normal_roman_table[i].key;
            key_len = str.size();
            if (key_len <= ichTarget) {
                if (roman.substr(ichTarget - key_len, key_len) == str) {
                    if (key_len > key.size()) {
                        key = str;
                        value = normal_roman_table[i].value;
                        extra.clear();
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < _countof(kigou_table); ++i) {
        key_len = 1;
        if (roman[ichTarget - key_len] == kigou_table[i].key[0]) {
            if (key_len <= ichTarget) {
                key = kigou_table[i].key;
                value = kigou_table[i].value;
            }
        }
    }
    if (key.size()) {
        roman.replace(ichTarget - key.size(), key.size(), value + extra);
    }
    return roman;
} // roman_to_hiragana

// ローマ字からカタカナへ文字列を変換。
std::wstring roman_to_katakana(std::wstring roman, size_t ichTarget)
{
    std::wstring str, key, value, extra;
    size_t key_len = 0;
    for (size_t i = 0; i < _countof(sokuon_table); ++i) {
        str = sokuon_table[i].key;
        key_len = str.size();
        if (key_len <= ichTarget) {
            if (roman.substr(ichTarget - key_len, key_len) == str) {
                if (key_len > key.size()) {
                    key = str;
                    value = sokuon_table[i].value;
                }
            }
        }
    }
    for (size_t i = 0; i < _countof(normal_roman_table); ++i) {
        if (normal_roman_table[i].extra) {
            str = normal_roman_table[i].key;
            key_len = str.size();
            if (key_len <= ichTarget) {
                if (roman.substr(ichTarget - key_len, key_len) == str) {
                    if (key_len > key.size()) {
                        key = str;
                        value = normal_roman_table[i].value;
                        extra = normal_roman_table[i].extra;
                    }
                }
            }
        } else {
            str = normal_roman_table[i].key;
            key_len = str.size();
            if (key_len <= ichTarget) {
                if (roman.substr(ichTarget - key_len, key_len) == str) {
                    if (key_len > key.size()) {
                        key = str;
                        value = normal_roman_table[i].value;
                        extra.clear();
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < _countof(kigou_table); ++i) {
        key_len = 1;
        if (roman[ichTarget - key_len] == kigou_table[i].key[0]) {
            if (key_len <= ichTarget) {
                key = kigou_table[i].key;
                value = kigou_table[i].value;
            }
        }
    }
    if (key.size()) {
        value = lcmap(value, LCMAP_KATAKANA);
        roman.replace(ichTarget - key.size(), key.size(), value + extra);
    }
    return roman;
} // roman_to_katakana

// ローマ字から半角カナへ文字列を変換。
std::wstring roman_to_halfwidth_katakana(std::wstring roman, size_t ichTarget)
{
    //DPRINTW(L"roman_to_halfwidth_katakana(%s,%d)\n", roman.c_str(), (int)ichTarget);
    std::wstring str, key, value, extra;
    size_t key_len = 0;
    for (size_t i = 0; i < _countof(sokuon_table); ++i) {
        str = sokuon_table[i].key;
        key_len = str.size();
        if (key_len <= ichTarget) {
            if (roman.substr(ichTarget - key_len, key_len) == str) {
                if (key_len > key.size()) {
                    key = str;
                    value = sokuon_table[i].value;
                }
            }
        }
    }
    for (size_t i = 0; i < _countof(normal_roman_table); ++i) {
        if (normal_roman_table[i].extra) {
            str = normal_roman_table[i].key;
            key_len = str.size();
            if (key_len <= ichTarget) {
                if (roman.substr(ichTarget - key_len, key_len) == str) {
                    if (key_len > key.size()) {
                        key = str;
                        value = normal_roman_table[i].value;
                        extra = normal_roman_table[i].extra;
                    }
                }
            }
        } else {
            str = normal_roman_table[i].key;
            key_len = str.size();
            if (key_len <= ichTarget) {
                if (roman.substr(ichTarget - key_len, key_len) == str) {
                    if (key_len > key.size()) {
                        key = str;
                        value = normal_roman_table[i].value;
                        extra.clear();
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < _countof(kigou_table); ++i) {
        key_len = 1;
        if (roman[ichTarget - key_len] == kigou_table[i].key[0]) {
            if (key_len <= ichTarget) {
                key = kigou_table[i].key;
                value = kigou_table[i].value;
            }
        }
    }
    if (key.size()) {
        value = lcmap(value, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
        roman.replace(ichTarget - key.size(), key.size(), value + extra);
    }
    //DPRINTW(L"roman_to_halfwidth_katakana: %s\n", roman.c_str());
    return roman;
} // roman_to_halfwidth_katakana

// ひらがなから入力文字列へ文字列を変換。
std::wstring hiragana_to_typing(std::wstring hiragana)
{
    std::wstring typing;
    for (size_t k = 0; k < hiragana.size(); ) {
        bool found = false;
        if (!found) {
            for (size_t i = 0; i < _countof(kana2type_table); ++i) {
                if (hiragana[k] == kana2type_table[i].key[0]) {
                    hiragana += kana2type_table[i].value;
                    k += 1;
                    found = true;
                    break;
                }
            }
        }
        if (!found) typing += hiragana[k++];
    }
    return typing;
} // hiragana_to_typing

// 仮想キーからひらがなへ。
WCHAR vkey_to_hiragana(BYTE vk, BOOL bShift)
{
    switch (vk) {
    case VK_A:          return L'ち';
    case VK_B:          return L'こ';
    case VK_C:          return L'そ';
    case VK_D:          return L'し';
    case VK_E:          return (bShift ? L'ぃ' : L'い');
    case VK_F:          return L'は';
    case VK_G:          return L'き';
    case VK_H:          return L'く';
    case VK_I:          return L'に';
    case VK_J:          return L'ま';
    case VK_K:          return L'の';
    case VK_L:          return L'り';
    case VK_M:          return L'も';
    case VK_N:          return L'み';
    case VK_O:          return L'ら';
    case VK_P:          return L'せ';
    case VK_Q:          return L'た';
    case VK_R:          return L'す';
    case VK_S:          return L'と';
    case VK_T:          return L'か';
    case VK_U:          return L'な';
    case VK_V:          return L'ひ';
    case VK_W:          return L'て';
    case VK_X:          return L'さ';
    case VK_Y:          return L'ん';
    case VK_Z:          return (bShift ? L'っ' : L'つ');
    case VK_0:          return (bShift ? L'を' : L'わ');
    case VK_1:          return L'ぬ';
    case VK_2:          return L'ふ';
    case VK_3:          return (bShift ? L'ぁ' : L'あ');
    case VK_4:          return (bShift ? L'ぅ' : L'う');
    case VK_5:          return (bShift ? L'ぇ' : L'え');
    case VK_6:          return (bShift ? L'ぉ' : L'お');
    case VK_7:          return (bShift ? L'ゃ' : L'や');
    case VK_8:          return (bShift ? L'ゅ' : L'ゆ');
    case VK_9:          return (bShift ? L'ょ' : L'よ');
    case VK_OEM_PLUS:   return L'れ';
    case VK_OEM_MINUS:  return L'ほ';
    case VK_OEM_COMMA:
        if (Config_GetDWORD(TEXT("bCommaPeriod"), FALSE))
            return (bShift ? L'，' : L'ね');
        else
            return (bShift ? L'、' : L'ね');
    case VK_OEM_PERIOD:
        if (Config_GetDWORD(TEXT("bCommaPeriod"), FALSE))
            return (bShift ? L'．' : L'る');
        else
            return (bShift ? L'。' : L'る');
    case VK_OEM_1:      return L'け';
    case VK_OEM_2:      return (bShift ? L'・' : L'め');
    case VK_OEM_3:      return L'゛';
    case VK_OEM_4:      return (bShift ? L'「' : L'゜');
    case VK_OEM_5:      return L'ー';
    case VK_OEM_6:      return (bShift ? L'」' : L'む');
    case VK_OEM_7:      return L'へ';
    case VK_OEM_102:    return L'ろ';
    default:            return 0;
    }
} // vkey_to_hiragana

// 入力キーから文字へ。
WCHAR typing_key_to_char(BYTE vk, BOOL bShift, BOOL bCapsLock)
{
    if (VK_A <= vk && vk <= VK_Z) {
        if (!bShift == !bCapsLock) {
            return vk + (L'a' - VK_A);
        } else {
            return vk + (L'A' - VK_A);
        }
    }
    switch (vk) {
    case VK_0:          return L'0';
    case VK_1:          return (bShift ? L'!' : L'1');
    case VK_2:          return (bShift ? L'"' : L'2');
    case VK_3:          return (bShift ? L'#' : L'3');
    case VK_4:          return (bShift ? L'$' : L'4');
    case VK_5:          return (bShift ? L'%' : L'5');
    case VK_6:          return (bShift ? L'&' : L'6');
    case VK_7:          return (bShift ? L'\'' : L'7');
    case VK_8:          return (bShift ? L'(' : L'8');
    case VK_9:          return (bShift ? L')' : L'9');
    case VK_OEM_PLUS:   return (bShift ? L'+' : L';');
    case VK_OEM_MINUS:  return (bShift ? L'=' : L'-');
    case VK_OEM_COMMA:  return (bShift ? L'<' : L',');
    case VK_OEM_PERIOD: return (bShift ? L'>' : L'.');
    case VK_OEM_1:      return (bShift ? L'*' : L':');
    case VK_OEM_2:      return (bShift ? L'?' : L'/');
    case VK_OEM_3:      return (bShift ? L'@' : L'`');
    case VK_OEM_4:      return (bShift ? L'{' : L'[');
    case VK_OEM_5:      return (bShift ? L'|' : L'\\');
    case VK_OEM_6:      return (bShift ? L'}' : L']');
    case VK_OEM_7:      return (bShift ? L'~' : L'^');
    case VK_OEM_102:    return (bShift ? L'_' : L'\\');
    case VK_ADD:        return L'+';
    case VK_SUBTRACT:   return L'-';
    case VK_MULTIPLY:   return L'*';
    case VK_DIVIDE:     return L'/';
    case VK_SEPARATOR:  return L',';
    case VK_DECIMAL:    return L'.';
    case VK_NUMPAD0:    return L'0';
    case VK_NUMPAD1:    return L'1';
    case VK_NUMPAD2:    return L'2';
    case VK_NUMPAD3:    return L'3';
    case VK_NUMPAD4:    return L'4';
    case VK_NUMPAD5:    return L'5';
    case VK_NUMPAD6:    return L'6';
    case VK_NUMPAD7:    return L'7';
    case VK_NUMPAD8:    return L'8';
    case VK_NUMPAD9:    return L'9';
    default:            return 0;
    }
} // typing_key_to_char

// 句点またはピリオドか？
// 注意：変換しやすさのため、ずぼらな判定にしています。
BOOL is_period(WCHAR ch)
{
    return ch == L'。' || ch == L'．' || ch == L'.' || ch == L'｡';
}

// 読点またはカンマか？
// 注意：変換しやすさのため、ずぼらな判定にしています。
BOOL is_comma(WCHAR ch)
{
    return ch == L'、' || ch == L'，' || ch == L',' || ch == L'､';
}

// ハイフンまたは長音か？
// 注意：変換しやすさのため、ずぼらな判定にしています。
BOOL is_hyphen(WCHAR ch)
{
    return ch == L'-' || ch == L'－' || ch == L'ー' || ch == L'ｰ';
}

// ひらがなか？
BOOL is_hiragana(WCHAR ch)
{
    if (0x3040 <= ch && ch <= 0x309F) return TRUE;
    switch (ch) {
    case 0x3095: case 0x3096: case 0x3099: case 0x309A: case 0x309B:
    case 0x309C: case 0x309D: case 0x309E: case 0x30FC:
        return TRUE;
    default:
        return FALSE;
    }
}

// 全角カタカナか？
BOOL is_fullwidth_katakana(WCHAR ch)
{
    if (0x30A0 <= ch && ch <= 0x30FF) return TRUE;
    switch (ch) {
    case 0x30FD: case 0x30FE: case 0x3099: case 0x309A: case 0x309B:
    case 0x309C: case 0x30FC:
        return TRUE;
    default:
        return FALSE;
    }
}

// 半角カナか？
BOOL is_halfwidth_katakana(WCHAR ch)
{
    if (0xFF65 <= ch && ch <= 0xFF9F) return TRUE;
    switch (ch) {
    case 0xFF61: case 0xFF62: case 0xFF63: case 0xFF64:
        return TRUE;
    default:
        return FALSE;
    }
}

// 漢字か？
BOOL is_kanji(WCHAR ch)
{
    if (0x4E00 <= ch && ch <= 0x9FFF) return TRUE;
    if (0xF900 <= ch && ch <= 0xFAFF) return TRUE;
    return FALSE;
}

// 教育漢字か？
BOOL is_education_kanji(WCHAR ch)
{
    return wcschr(L"一音雨円王火花貝学休九玉空気下月見犬五口校金左三山四子糸字耳七車手十出女小正上森人水生青赤石夕千先川草早足村大男竹虫中町天田土二日入年白八百本名木目文右力立林六引羽雲遠園家何夏科歌画会回海絵外角楽活間顔岩丸記汽帰弓牛魚教強近今兄形京計原言元古戸語午後広交工光公考黄行高合国谷黒細才作算紙矢姉市止時寺自色室社弱首週秋春書少場食新親心図頭数西星声晴雪切船線前組走太多体台弟池地知茶昼長朝鳥直通点店電冬刀当答東道同読内南肉馬買売麦半番父歩風分聞米母方北毎妹万鳴明毛門野夜友曜用来里理話思悪安暗意委医育院員飲運泳駅横央屋温荷開界階館感寒漢岸起期客急級球究去橋業曲局銀苦区具宮君化軽係血決県研湖庫幸向港号根祭皿指始歯死詩仕使次持事式実写者昔酒守主取受習拾集終州重住宿暑所助消昭章商勝乗植身進申深真神世整全想相送息速族他打代対待第題短炭談着注柱調帳追丁定庭笛鉄転都度島等登湯投豆動童農波配倍箱畑発反板坂悲皮鼻美筆氷表病秒品負部福服物平返勉放味命面問薬役由油遊有予葉陽羊洋様落旅両緑流礼列練路和愛案以位胃囲衣印英栄塩億貨課加果芽械改害街覚各関完管官観願喜季旗紀希機器議救泣求給挙協鏡競共極訓軍郡型径景芸結欠建健験固候功好航康告差最菜材昨殺刷札察参散産残氏史試士司治児辞失借種周祝順初唱笑焼松賞象照省信臣清静成席積節折説戦選浅巣争倉束側続卒孫隊帯達単置仲貯兆腸停低底的典伝徒努灯働堂特得毒熱然念敗梅博飯飛費必票標付府不夫無副粉兵別辺変便法包望牧末満未脈民約勇要養浴利陸量良漁料輪類令冷例歴連老労録圧移因永衛営益易液演桜応往恩価過可河仮賀解快確格額慣幹刊眼規基寄技義逆旧久許居境均禁句群経潔険検券件現限減個故護効講構鉱耕厚興混査災採妻際再在罪財雑賛酸支資師志飼枝示似識質謝舎授修述術準序証承招常状条情職織製精性政制勢税責績設接絶舌銭祖素総増像造則測属損貸退態団断築張程提敵適統銅導徳独任燃能破犯版判肥比非備俵評貧富布婦武復複仏編弁保墓豊報防暴夢務迷綿輸余預容率略留領貿律異遺域宇映沿延我灰閣革拡割株簡干看巻危揮机貴疑吸郷胸供筋勤敬系警劇激穴絹憲権厳源己呼誤孝皇后紅鋼降穀刻骨困砂座済裁策冊蚕誌視私詞至姿磁捨射尺若樹宗収衆就従縦縮熟純署処諸除傷障将城蒸針仁推垂寸誠聖盛専宣染泉洗善層装奏操創窓蔵臓尊存宅探担誕暖段値宙忠著潮頂庁賃痛展党討糖届難乳認脳納派肺背俳拝班晩否秘批腹奮陛並閉補暮訪宝棒忘亡枚幕密盟模訳優郵幼翌欲卵乱覧裏臨論朗片", ch) != NULL;
}

// 常用漢字か？
BOOL is_common_use_kanji(WCHAR ch)
{
    return wcschr(L"亜哀挨愛曖悪握圧扱宛嵐安案暗以衣位囲医依委威為畏胃尉異移萎偉椅彙意違維慰遺緯域育一壱逸茨芋引印因咽姻員院淫陰飲隠韻右宇羽雨唄鬱畝浦運雲永泳英映栄営詠影鋭衛易疫益液駅悦越謁閲円延沿炎怨宴媛援園煙猿遠鉛塩演縁艶汚王凹央応往押旺欧殴桜翁奥横岡屋億憶臆虞乙俺卸音恩温穏下化火加可仮何花佳価果河苛科架夏家荷華菓貨渦過嫁暇禍靴寡歌箇稼課蚊牙瓦我画芽賀雅餓介回灰会快戒改怪拐悔海界皆械絵開階塊楷解潰壊懐諧貝外劾害崖涯街慨蓋該概骸垣柿各角拡革格核殻郭覚較隔閣確獲嚇穫学岳楽額顎掛潟括活喝渇割葛滑褐轄且株釜鎌刈干刊甘汗缶完肝官冠巻看陥乾勘患貫寒喚堪換敢棺款間閑勧寛幹感漢慣管関歓監緩憾還館環簡観韓艦鑑丸含岸岩玩眼頑顔願企伎危机気岐希忌汽奇祈季紀軌既記起飢鬼帰基寄規亀喜幾揮期棋貴棄毀旗器畿輝機騎技宜偽欺義疑儀戯擬犠議菊吉喫詰却客脚逆虐九久及弓丘旧休吸朽臼求究泣急級糾宮救球給嗅窮牛去巨居拒拠挙虚許距魚御漁凶共叫狂京享供協況峡挟狭恐恭胸脅強教郷境橋矯鏡競響驚仰暁業凝曲局極玉巾斤均近金菌勤琴筋僅禁緊錦謹襟吟銀区句苦駆具惧愚空偶遇隅串屈掘窟熊繰君訓勲薫軍郡群兄刑形系径茎係型契計恵啓掲渓経蛍敬景軽傾携継詣慶憬稽憩警鶏芸迎鯨隙劇撃激桁欠穴血決結傑潔月犬件見券肩建研県倹兼剣拳軒健険圏堅検嫌献絹遣権憲賢謙鍵繭顕験懸元幻玄言弦限原現舷減源厳己戸古呼固股虎孤弧故枯個庫湖雇誇鼓錮顧五互午呉後娯悟碁語誤護口工公勾孔功巧広甲交光向后好江考行坑孝抗攻更効幸拘肯侯厚恒洪皇紅荒郊香候校耕航貢降高康控梗黄喉慌港硬絞項溝鉱構綱酵稿興衡鋼講購乞号合拷剛傲豪克告谷刻国黒穀酷獄骨駒込頃今困昆恨根婚混痕紺魂墾懇左佐沙査砂唆差詐鎖座挫才再災妻采砕宰栽彩採済祭斎細菜最裁債催塞歳載際埼在材剤財罪崎作削昨柵索策酢搾錯咲冊札刷刹拶殺察撮擦雑皿三山参桟蚕惨産傘散算酸賛残斬暫士子支止氏仕史司四市矢旨死糸至伺志私使刺始姉枝祉肢姿思指施師恣紙脂視紫詞歯嗣試詩資飼誌雌摯賜諮示字寺次耳自似児事侍治持時滋慈辞磁餌璽鹿式識軸七叱失室疾執湿嫉漆質実芝写社車舎者射捨赦斜煮遮謝邪蛇尺借酌釈爵若弱寂手主守朱取狩首殊珠酒腫種趣寿受呪授需儒樹収囚州舟秀周宗拾秋臭修袖終羞習週就衆集愁酬醜蹴襲十汁充住柔重従渋銃獣縦叔祝宿淑粛縮塾熟出述術俊春瞬旬巡盾准殉純循順準潤遵処初所書庶暑署緒諸女如助序叙徐除小升少召匠床抄肖尚招承昇松沼昭宵将消症祥称笑唱商渉章紹訟勝掌晶焼焦硝粧詔証象傷奨照詳彰障憧衝賞償礁鐘上丈冗条状乗城浄剰常情場畳蒸縄壌嬢錠譲醸色拭食植殖飾触嘱織職辱尻心申伸臣芯身辛侵信津神唇娠振浸真針深紳進森診寝慎新審震薪親人刃仁尽迅甚陣尋腎須図水吹垂炊帥粋衰推酔遂睡穂随髄枢崇数据杉裾寸瀬是井世正生成西声制姓征性青斉政星牲省凄逝清盛婿晴勢聖誠精製誓静請整醒税夕斥石赤昔析席脊隻惜戚責跡積績籍切折拙窃接設雪摂節説舌絶千川仙占先宣専泉浅洗染扇栓旋船戦煎羨腺詮践箋銭潜線遷選薦繊鮮全前善然禅漸膳繕狙阻祖租素措粗組疎訴塑遡礎双壮早争走奏相荘草送倉捜挿桑巣掃曹曽爽窓創喪痩葬装僧想層総遭槽踪操燥霜騒藻造像増憎蔵贈臓即束足促則息捉速側測俗族属賊続卒率存村孫尊損遜他多汰打妥唾堕惰駄太対体耐待怠胎退帯泰堆袋逮替貸隊滞態戴大代台第題滝宅択沢卓拓託濯諾濁但達脱奪棚誰丹旦担単炭胆探淡短嘆端綻誕鍛団男段断弾暖談壇地池知値恥致遅痴稚置緻竹畜逐蓄築秩窒茶着嫡中仲虫沖宙忠抽注昼柱衷酎鋳駐著貯丁弔庁兆町長挑帳張彫眺釣頂鳥朝貼超腸跳徴嘲潮澄調聴懲直勅捗沈珍朕陳賃鎮追椎墜通痛塚漬坪爪鶴低呈廷弟定底抵邸亭貞帝訂庭逓停偵堤提程艇締諦泥的笛摘滴適敵溺迭哲鉄徹撤天典店点展添転填田伝殿電斗吐妬徒途都渡塗賭土奴努度怒刀冬灯当投豆東到逃倒凍唐島桃討透党悼盗陶塔搭棟湯痘登答等筒統稲踏糖頭謄藤闘騰同洞胴動堂童道働銅導瞳峠匿特得督徳篤毒独読栃凸突届屯豚頓貪鈍曇丼那奈内梨謎鍋南軟難二尼弐匂肉虹日入乳尿任妊忍認寧熱年念捻粘燃悩納能脳農濃把波派破覇馬婆罵拝杯背肺俳配排敗廃輩売倍梅培陪媒買賠白伯拍泊迫剥舶博薄麦漠縛爆箱箸畑肌八鉢発髪伐抜罰閥反半氾犯帆汎伴判坂阪板版班畔般販斑飯搬煩頒範繁藩晩番蛮盤比皮妃否批彼披肥非卑飛疲秘被悲扉費碑罷避尾眉美備微鼻膝肘匹必泌筆姫百氷表俵票評漂標苗秒病描猫品浜貧賓頻敏瓶不夫父付布扶府怖阜附訃負赴浮婦符富普腐敷膚賦譜侮武部舞封風伏服副幅復福腹複覆払沸仏物粉紛雰噴墳憤奮分文聞丙平兵併並柄陛閉塀幣弊蔽餅米壁璧癖別蔑片辺返変偏遍編弁便勉歩保哺捕補舗母募墓慕暮簿方包芳邦奉宝抱放法泡胞俸倣峰砲崩訪報蜂豊飽褒縫亡乏忙坊妨忘防房肪某冒剖紡望傍帽棒貿貌暴膨謀頬北木朴牧睦僕墨撲没勃堀本奔翻凡盆麻摩磨魔毎妹枚昧埋幕膜枕又末抹万満慢漫未味魅岬密蜜脈妙民眠矛務無夢霧娘名命明迷冥盟銘鳴滅免面綿麺茂模毛妄盲耗猛網目黙門紋問冶夜野弥厄役約訳薬躍闇由油喩愉諭輸癒唯友有勇幽悠郵湧猶裕遊雄誘憂融優与予余誉預幼用羊妖洋要容庸揚揺葉陽溶腰様瘍踊窯養擁謡曜抑沃浴欲翌翼拉裸羅来雷頼絡落酪辣乱卵覧濫藍欄吏利里理痢裏履璃離陸立律慄略柳流留竜粒隆硫侶旅虜慮了両良料涼猟陵量僚領寮療瞭糧力緑林厘倫輪隣臨瑠涙累塁類令礼冷励戻例鈴零霊隷齢麗暦歴列劣烈裂恋連廉練錬呂炉賂路露老労弄郎朗浪廊楼漏籠六録麓論和話賄脇惑枠湾腕", ch) != NULL;
} // is_common_use_kanji

// 全角ASCIIか？
BOOL is_fullwidth_ascii(WCHAR ch)
{
    return (0xFF00 <= ch && ch <= 0xFFEF);
}

// すべての文字が数字的か？
BOOL are_all_chars_numeric(const std::wstring& str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        if (L'0' <= str[i] && str[i] <= L'9') {
            ;
        } else if (L'０' <= str[i] && str[i] <= L'９') {
            ;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

static const wchar_t s_szKanjiDigits[] = L"〇一二三四五六七八九";

// 漢数字変換（その１）。
std::wstring convert_to_kansuuji_1(wchar_t ch, size_t digit_level)
{
    static const wchar_t s_szKanjiDigitLevels[] = L" 十百千";
    std::wstring ret;
    if (ch == L'0') return ret;
    ASSERT(ch - L'0' < 10);
    ret += s_szKanjiDigits[ch - L'0'];
    if (digit_level > 0) {
        ASSERT(digit_level < 4);
        ret += s_szKanjiDigitLevels[digit_level];
    }
    return ret;
}

// 漢数字変換（その２）。
std::wstring convert_to_kansuuji_4(const std::wstring& halfwidth)
{
    ASSERT(are_all_chars_numeric(halfwidth));
    ASSERT(halfwidth.size() <= 4);
    const size_t length = halfwidth.size();
    std::wstring ret;
    size_t level = 0;
    for (size_t i = length; i > 0; ) {
        --i;
        ret = convert_to_kansuuji_1(halfwidth[i], level) + ret;
        ++level;
    }
    return ret;
}

// 漢数字変換（その３）。
std::wstring convert_to_kansuuji(const std::wstring& str)
{
    std::wstring halfwidth = lcmap(str, LCMAP_HALFWIDTH);
    ASSERT(are_all_chars_numeric(halfwidth));
    if (halfwidth.size() >= 24) return halfwidth;
    static const wchar_t s_szKanjiGroupLevels[] = L" 万億兆京垓";
    std::wstring ret;
    size_t iGroup = 0;
    while (halfwidth.size()) {
        std::wstring group;
        if (halfwidth.size() >= 4) {
            group = halfwidth.substr(halfwidth.size() - 4);
        } else {
            group = halfwidth;
        }
        group = convert_to_kansuuji_4(group);
        if (group.size() && iGroup > 0) {
            ret = group + s_szKanjiGroupLevels[iGroup] + ret;
        } else {
            ret = group + ret;
        }
        if (halfwidth.size() >= 4) {
            halfwidth = halfwidth.substr(0, halfwidth.size() - 4);
        } else {
            break;
        }
        ++iGroup;
    }
    if (ret.empty()) ret = L"零";
    str_replace_all(ret, L"一十", L"十");
    str_replace_all(ret, L"一百", L"百");
    str_replace_all(ret, L"一千", L"千");
    return ret;
}

// 漢数字変換（その４）。
std::wstring convert_to_kansuuji_brief(const std::wstring& str)
{
    std::wstring halfwidth = lcmap(str, LCMAP_HALFWIDTH);
    ASSERT(are_all_chars_numeric(halfwidth));
    std::wstring ret;
    for (size_t i = 0; i < halfwidth.size(); ++i) {
        ret += s_szKanjiDigits[halfwidth[i] - L'0'];
    }
    return ret;
}

// 漢数字変換（その５）。
std::wstring convert_to_kansuuji_formal(const std::wstring& str)
{
    std::wstring ret = convert_to_kansuuji(str);
    str_replace_all(ret, L"一", L"壱");
    str_replace_all(ret, L"二", L"弐");
    str_replace_all(ret, L"三", L"参");
    str_replace_all(ret, L"五", L"伍");
    str_replace_all(ret, L"十", L"拾");
    str_replace_all(ret, L"千", L"仟");
    str_replace_all(ret, L"万", L"萬");
    return ret;
}

// 濁音処理。
WCHAR dakuon_shori(WCHAR ch0, WCHAR ch1)
{
    switch (MAKELONG(ch0, ch1)) {
    case MAKELONG(L'か', L'゛'): return L'が';
    case MAKELONG(L'き', L'゛'): return L'ぎ';
    case MAKELONG(L'く', L'゛'): return L'ぐ';
    case MAKELONG(L'け', L'゛'): return L'げ';
    case MAKELONG(L'こ', L'゛'): return L'ご';
    case MAKELONG(L'さ', L'゛'): return L'ざ';
    case MAKELONG(L'し', L'゛'): return L'じ';
    case MAKELONG(L'す', L'゛'): return L'ず';
    case MAKELONG(L'せ', L'゛'): return L'ぜ';
    case MAKELONG(L'そ', L'゛'): return L'ぞ';
    case MAKELONG(L'た', L'゛'): return L'だ';
    case MAKELONG(L'ち', L'゛'): return L'ぢ';
    case MAKELONG(L'つ', L'゛'): return L'づ';
    case MAKELONG(L'て', L'゛'): return L'で';
    case MAKELONG(L'と', L'゛'): return L'ど';
    case MAKELONG(L'は', L'゛'): return L'ば';
    case MAKELONG(L'ひ', L'゛'): return L'び';
    case MAKELONG(L'ふ', L'゛'): return L'ぶ';
    case MAKELONG(L'へ', L'゛'): return L'べ';
    case MAKELONG(L'ほ', L'゛'): return L'ぼ';
    case MAKELONG(L'う', L'゛'): return L'\u3094';
    case MAKELONG(L'は', L'゜'): return L'ぱ';
    case MAKELONG(L'ひ', L'゜'): return L'ぴ';
    case MAKELONG(L'ふ', L'゜'): return L'ぷ';
    case MAKELONG(L'へ', L'゜'): return L'ぺ';
    case MAKELONG(L'ほ', L'゜'): return L'ぽ';
    case MAKELONG(L'カ', L'゛'): return L'ガ';
    case MAKELONG(L'キ', L'゛'): return L'ギ';
    case MAKELONG(L'ク', L'゛'): return L'グ';
    case MAKELONG(L'ケ', L'゛'): return L'ゲ';
    case MAKELONG(L'コ', L'゛'): return L'ゴ';
    case MAKELONG(L'サ', L'゛'): return L'ザ';
    case MAKELONG(L'シ', L'゛'): return L'ジ';
    case MAKELONG(L'ス', L'゛'): return L'ズ';
    case MAKELONG(L'セ', L'゛'): return L'ゼ';
    case MAKELONG(L'ソ', L'゛'): return L'ゾ';
    case MAKELONG(L'タ', L'゛'): return L'ダ';
    case MAKELONG(L'チ', L'゛'): return L'ヂ';
    case MAKELONG(L'ツ', L'゛'): return L'ヅ';
    case MAKELONG(L'テ', L'゛'): return L'デ';
    case MAKELONG(L'ト', L'゛'): return L'ド';
    case MAKELONG(L'ハ', L'゛'): return L'バ';
    case MAKELONG(L'ヒ', L'゛'): return L'ビ';
    case MAKELONG(L'フ', L'゛'): return L'ブ';
    case MAKELONG(L'ヘ', L'゛'): return L'ベ';
    case MAKELONG(L'ホ', L'゛'): return L'ボ';
    case MAKELONG(L'ウ', L'゛'): return L'ヴ';
    case MAKELONG(L'ハ', L'゜'): return L'パ';
    case MAKELONG(L'ヒ', L'゜'): return L'ピ';
    case MAKELONG(L'フ', L'゜'): return L'プ';
    case MAKELONG(L'ヘ', L'゜'): return L'ペ';
    case MAKELONG(L'ホ', L'゜'): return L'ポ';
    default:                     return 0;
    }
}

// 文字種変換。
std::wstring lcmap(const std::wstring& str, DWORD dwFlags)
{
    WCHAR szBuf[1024];
    const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
    ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                   str.c_str(), -1, szBuf, 1024);
    return szBuf;
}

// 全角英数から半角への文字列変換。
std::wstring fullwidth_ascii_to_halfwidth(const std::wstring& str)
{
    std::wstring ret;
    const size_t count = str.size();
    wchar_t ch;
    for (size_t i = 0; i < count; ++i) {
        ch = str[i];
        if (L'ａ' <= ch && ch <= L'ｚ') {
            ch += L'a' - L'ａ';
        } else if (L'Ａ' <= ch && ch <= L'Ｚ') {
            ch += L'A' - L'Ａ';
        }
        ret += ch;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
