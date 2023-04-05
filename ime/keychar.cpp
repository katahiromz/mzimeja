// keychar.cpp --- mzimeja keys and characters
// ÉLÅ[ì¸óÕÇ∆ï∂éöÅB
// (Japanese, Shift_JIS)
// NOTE: This file uses Japanese cp932 encoding. To compile this on g++,
//       please add options: -finput-charset=CP932 -fexec-charset=CP932

#include "mzimeja.h"
#include "vksub.h"

//////////////////////////////////////////////////////////////////////////////

struct KEYVALUE {
    const WCHAR *key;
    const WCHAR *value;
};

struct KEYVALUEEXTRA {
    const WCHAR *key;
    const WCHAR *value;
    const WCHAR *extra;
};

// ëSäpÇ–ÇÁÇ™Ç»Ç©ÇÁîºäpÇ÷ÇÃïœä∑ÉeÅ[ÉuÉãÅB
static KEYVALUE halfkana_table[] = {
    {L"ÅJ", L"ﬁ"},
    {L"ÅK", L"ﬂ"},
    {L"Å[", L"-"},
    {L"Åu", L"¢"},
    {L"Åv", L"£"},
    {L"ÅB", L"°"},
    {L"ÅA", L"§"},
    {L"ÅE", L"•"},
    {L"Ç§ÅJ", L"≥ﬁ"},
    {L"Ç†", L"±"},
    {L"Ç¢", L"≤"},
    {L"Ç§", L"≥"},
    {L"Ç¶", L"¥"},
    {L"Ç®", L"µ"},
    {L"Çü", L"ß"},
    {L"Ç°", L"®"},
    {L"Ç£", L"©"},
    {L"Ç•", L"™"},
    {L"Çß", L"´"},
    {L"Ç©", L"∂"},
    {L"Ç´", L"∑"},
    {L"Ç≠", L"∏"},
    {L"ÇØ", L"π"},
    {L"Ç±", L"∫"},
    {L"Ç™", L"∂ﬁ"},
    {L"Ç¨", L"∑ﬁ"},
    {L"ÇÆ", L"∏ﬁ"},
    {L"Ç∞", L"πﬁ"},
    {L"Ç≤", L"∫ﬁ"},
    {L"Ç≥", L"ª"},
    {L"Çµ", L"º"},
    {L"Ç∑", L"Ω"},
    {L"Çπ", L"æ"},
    {L"Çª", L"ø"},
    {L"Ç¥", L"ªﬁ"},
    {L"Ç∂", L"ºﬁ"},
    {L"Ç∏", L"Ωﬁ"},
    {L"Ç∫", L"æﬁ"},
    {L"Çº", L"øﬁ"},
    {L"ÇΩ", L"¿"},
    {L"Çø", L"¡"},
    {L"Ç¬", L"¬"},
    {L"Çƒ", L"√"},
    {L"Ç∆", L"ƒ"},
    {L"Çæ", L"¿ﬁ"},
    {L"Ç¿", L"¡ﬁ"},
    {L"Ç√", L"¬ﬁ"},
    {L"Ç≈", L"√ﬁ"},
    {L"Ç«", L"ƒﬁ"},
    {L"Ç¡", L"Ø"},
    {L"Ç»", L"≈"},
    {L"Ç…", L"∆"},
    {L"Ç ", L"«"},
    {L"ÇÀ", L"»"},
    {L"ÇÃ", L"…"},
    {L"ÇÕ", L" "},
    {L"Ç–", L"À"},
    {L"Ç”", L"Ã"},
    {L"Ç÷", L"Õ"},
    {L"ÇŸ", L"Œ"},
    {L"ÇŒ", L" ﬁ"},
    {L"Ç—", L"Àﬁ"},
    {L"Ç‘", L"Ãﬁ"},
    {L"Ç◊", L"Õﬁ"},
    {L"Ç⁄", L"Œﬁ"},
    {L"Çœ", L" ﬂ"},
    {L"Ç“", L"Àﬂ"},
    {L"Ç’", L"Ãﬂ"},
    {L"Çÿ", L"Õﬂ"},
    {L"Ç€", L"Œﬂ"},
    {L"Ç‹", L"œ"},
    {L"Ç›", L"–"},
    {L"Çﬁ", L"—"},
    {L"Çﬂ", L"“"},
    {L"Ç‡", L"”"},
    {L"Ç·", L"¨"},
    {L"Ç‚", L"‘"},
    {L"Ç„", L"≠"},
    {L"Ç‰", L"’"},
    {L"ÇÂ", L"Æ"},
    {L"ÇÊ", L"÷"},
    {L"ÇÁ", L"◊"},
    {L"ÇË", L"ÿ"},
    {L"ÇÈ", L"Ÿ"},
    {L"ÇÍ", L"⁄"},
    {L"ÇÎ", L"€"},
    {L"ÇÏ", L"‹"},
    {L"ÇÌ", L"‹"},
    {L"ÇÓ", L"≤"},
    {L"ÇÔ", L"¥"},
    {L"Ç", L"¶"},
    {L"ÇÒ", L"›"},
};

// îºäpÉJÉiÇ©ÇÁëSäpÇ–ÇÁÇ™Ç»Ç÷ÇÃïœä∑ÉeÅ[ÉuÉãÅB
static KEYVALUE kana_table[] = {
    {L"∞", L"Å["},
    {L"ﬁ", L"ÅJ"},
    {L"ﬂ", L"ÅK"},
    {L"¢", L"Åu"},
    {L"£", L"Åv"},
    {L"°", L"ÅB"},
    {L"§", L"ÅA"},
    {L"•", L"ÅE"},
    {L"±", L"Ç†"},
    {L"≤", L"Ç¢"},
    {L"≥", L"Ç§"},
    {L"¥", L"Ç¶"},
    {L"µ", L"Ç®"},
    {L"ß", L"Çü"},
    {L"®", L"Ç°"},
    {L"©", L"Ç£"},
    {L"™", L"Ç•"},
    {L"´", L"Çß"},
    {L"∂", L"Ç©"},
    {L"∑", L"Ç´"},
    {L"∏", L"Ç≠"},
    {L"π", L"ÇØ"},
    {L"∫", L"Ç±"},
    {L"ª", L"Ç≥"},
    {L"º", L"Çµ"},
    {L"Ω", L"Ç∑"},
    {L"æ", L"Çπ"},
    {L"ø", L"Çª"},
    {L"¿", L"ÇΩ"},
    {L"¡", L"Çø"},
    {L"¬", L"Ç¬"},
    {L"√", L"Çƒ"},
    {L"ƒ", L"Ç∆"},
    {L"Ø", L"Ç¡"},
    {L"≈", L"Ç»"},
    {L"∆", L"Ç…"},
    {L"«", L"Ç "},
    {L"»", L"ÇÀ"},
    {L"…", L"ÇÃ"},
    {L" ", L"ÇÕ"},
    {L"À", L"Ç–"},
    {L"Ã", L"Ç”"},
    {L"Õ", L"Ç÷"},
    {L"Œ", L"ÇŸ"},
    {L"œ", L"Ç‹"},
    {L"–", L"Ç›"},
    {L"—", L"Çﬁ"},
    {L"“", L"Çﬂ"},
    {L"”", L"Ç‡"},
    {L"¨", L"Ç·"},
    {L"‘", L"Ç‚"},
    {L"≠", L"Ç„"},
    {L"’", L"Ç‰"},
    {L"Æ", L"ÇÂ"},
    {L"÷", L"ÇÊ"},
    {L"◊", L"ÇÁ"},
    {L"ÿ", L"ÇË"},
    {L"Ÿ", L"ÇÈ"},
    {L"⁄", L"ÇÍ"},
    {L"€", L"ÇÎ"},
    {L"‹", L"ÇÌ"},
    {L"¶", L"Ç"},
    {L"›", L"ÇÒ"},
    {L"Ç§ﬁ", L"Éî"},
    {L"Ç©ﬁ", L"Ç™"},
    {L"Ç´ﬁ", L"Ç¨"},
    {L"Ç≠ﬁ", L"ÇÆ"},
    {L"ÇØﬁ", L"Ç∞"},
    {L"Ç±ﬁ", L"Ç≤"},
    {L"Ç≥ﬁ", L"Ç¥"},
    {L"Çµﬁ", L"Ç∂"},
    {L"Ç∑ﬁ", L"Ç∏"},
    {L"Çπﬁ", L"Ç∫"},
    {L"Çªﬁ", L"Çº"},
    {L"ÇΩﬁ", L"Çæ"},
    {L"Çøﬁ", L"Ç¿"},
    {L"Ç¬ﬁ", L"Ç√"},
    {L"Çƒﬁ", L"Ç≈"},
    {L"Ç∆ﬁ", L"Ç«"},
    {L"ÇÕﬁ", L"ÇŒ"},
    {L"Ç–ﬁ", L"Ç—"},
    {L"Ç”ﬁ", L"Ç‘"},
    {L"Ç÷ﬁ", L"Ç◊"},
    {L"ÇŸﬁ", L"Ç⁄"},
    {L"ÇÕﬂ", L"Çœ"},
    {L"Ç–ﬂ", L"Ç“"},
    {L"Ç”ﬂ", L"Ç’"},
    {L"Ç÷ﬂ", L"Çÿ"},
    {L"ÇŸﬂ", L"Ç€"},
};

// ãLçÜÇÃïœä∑ÉeÅ[ÉuÉãÅB
static KEYVALUE kigou_table[] = {
    //{L",", L"ÅA"}, // bCommaPeriodÇ≈èàóùÇ∑ÇÈÅB
    //{L".", L"ÅB"}, // bCommaPeriodÇ≈èàóùÇ∑ÇÈÅB
    {L"/", L"ÅE"},
    {L"~", L"Å`"},
    {L"[", L"Åu"},
    {L"]", L"Åv"},
    {L"{", L"Åo"},
    {L"}", L"Åp"},
    {L":", L"ÅF"},
    {L";", L"ÅG"},
    {L"<", L"ÅÉ"},
    {L"=", L"ÅÅ"},
    {L">", L"ÅÑ"},
    {L"?", L"ÅH"},
    {L"@", L"Åó"},
    {L"\\", L"Åè"},
    {L"^", L"ÅO"},
    {L"_", L"ÅQ"},
    {L"`", L"ÅM"},
    {L"|", L"Åb"},
    {L"!", L"ÅI"},
    {L"\"", L"Åç"},
    {L"#", L"Åî"},
    {L"$", L"Åê"},
    {L"%", L"Åì"},
    {L"&", L"Åï"},
    {L"'", L"Åf"},
    {L"(", L"Åi"},
    {L")", L"Åj"},
    {L"*", L"Åñ"},
    {L"+", L"Å{"},
    {L"-", L"Å["},
};

// ÉçÅ[É}éöãtïœä∑ÇÃÉeÅ[ÉuÉãÅB
static KEYVALUEEXTRA reverse_roman_table[] = {
    {L"Ç§ÅJÇü", L"va"},
    {L"Ç§ÅJÇ°", L"vi"},
    {L"Ç§ÅJ", L"vu"},
    {L"Ç§ÅJÇ•", L"ve"},
    {L"Ç§ÅJÇß", L"vo"},
    {L"Ç¡Ç§ÅJ", L"v", L"Ç§ÅJ"},
    {L"Ç¡Çü", L"xxa"},
    {L"Ç¡Ç°", L"xxi"},
    {L"Ç¡Ç£", L"xxu"},
    {L"Ç¡Ç•", L"xxe"},
    {L"Ç¡Çß", L"xxo"},
    {L"Ç¡Ç©", L"kka"},
    {L"Ç¡Ç´", L"k", L"Ç´"},
    {L"Ç¡Ç≠", L"kku"},
    {L"Ç¡ÇØ", L"kke"},
    {L"Ç¡Ç±", L"kko"},
    {L"Ç¡Ç™", L"gga"},
    {L"Ç¡Ç¨", L"g", L"Ç¨"},
    {L"Ç¡ÇÆ", L"ggu"},
    {L"Ç¡Ç∞", L"gge"},
    {L"Ç¡Ç≤", L"ggo"},
    {L"Ç¡Ç≥", L"ssa"},
    {L"Ç¡Çµ", L"s", L"Çµ"},
    {L"Ç¡Ç∑", L"ssu"},
    {L"Ç¡Çπ", L"sse"},
    {L"Ç¡Çª", L"sso"},
    {L"Ç¡Ç¥", L"zza"},
    {L"Ç¡Ç∂", L"z", L"Ç∂"},
    {L"Ç¡Ç∏", L"zzu"},
    {L"Ç¡Ç∫", L"zze"},
    {L"Ç¡Çº", L"zzo"},
    {L"Ç¡ÇΩ", L"tta"},
    {L"Ç¡Çø", L"t", L"Çø"},
    {L"Ç¡Ç¬", L"ttu"},
    {L"Ç¡Çƒ", L"tte"},
    {L"Ç¡Ç∆", L"tto"},
    {L"Ç¡Çæ", L"dda"},
    {L"Ç¡Ç¿", L"d", L"Ç¿"},
    {L"Ç¡Ç√", L"ddu"},
    {L"Ç¡Ç≈", L"dde"},
    {L"Ç¡Ç«", L"ddo"},
    {L"Ç¡ÇÕ", L"hha"},
    {L"Ç¡Ç–", L"h", L"Ç–"},
    {L"Ç¡Ç”", L"hhu"},
    {L"Ç¡Ç÷", L"hhe"},
    {L"Ç¡ÇŸ", L"hho"},
    {L"Ç¡ÇŒ", L"bba"},
    {L"Ç¡Ç—", L"b", L"Ç—"},
    {L"Ç¡Ç‘", L"bbu"},
    {L"Ç¡Ç◊", L"bbe"},
    {L"Ç¡Ç⁄", L"bbo"},
    {L"Ç¡Çœ", L"ppa"},
    {L"Ç¡Ç“", L"p", L"Ç“"},
    {L"Ç¡Ç’", L"ppu"},
    {L"Ç¡Çÿ", L"ppe"},
    {L"Ç¡Ç€", L"ppo"},
    {L"Ç¡Ç‹", L"mma"},
    {L"Ç¡Ç›", L"m", L"Ç›"},
    {L"Ç¡Çﬁ", L"mmu"},
    {L"Ç¡Çﬂ", L"mme"},
    {L"Ç¡Ç‡", L"mmo"},
    {L"Ç¡Ç‚", L"yya"},
    {L"Ç¡Ç‰", L"yyu"},
    {L"Ç¡ÇÊ", L"yyo"},
    {L"Ç¡Ç·", L"xxya"},
    {L"Ç¡Ç„", L"xxyu"},
    {L"Ç¡ÇÂ", L"xxyo"},
    {L"Ç¡ÇÁ", L"rra"},
    {L"Ç¡ÇË", L"r", L"ÇË"},
    {L"Ç¡ÇÈ", L"rru"},
    {L"Ç¡ÇÍ", L"rre"},
    {L"Ç¡ÇÎ", L"rro"},
    {L"Ç¡ÇÏ", L"xxwa"},
    {L"Ç¡ÇÌ", L"wwa"},
    {L"Ç¡ÇÓ", L"wwi"},
    {L"Ç¡ÇÔ", L"wwe"},
    {L"Ç¡Ç", L"wwo"},
    {L"Ç´Ç·", L"kya"},
    {L"Ç´Ç°", L"kyi"},
    {L"Ç´Ç„", L"kyu"},
    {L"Ç´Ç•", L"kye"},
    {L"Ç´ÇÂ", L"kyo"},
    {L"Ç¨Ç·", L"gya"},
    {L"Ç¨Ç°", L"gyi"},
    {L"Ç¨Ç„", L"gyu"},
    {L"Ç¨Ç•", L"gye"},
    {L"Ç¨ÇÂ", L"gyo"},
    {L"ÇµÇ·", L"sya"},
    {L"ÇµÇ°", L"syi"},
    {L"ÇµÇ„", L"syu"},
    {L"ÇµÇ•", L"sye"},
    {L"ÇµÇÂ", L"syo"},
    {L"Ç∂Ç·", L"zya"},
    {L"Ç∂Ç°", L"zyi"},
    {L"Ç∂Ç„", L"zyu"},
    {L"Ç∂Ç•", L"zye"},
    {L"Ç∂ÇÂ", L"zyo"},
    {L"ÇøÇ·", L"tya"},
    {L"ÇøÇ°", L"tyi"},
    {L"ÇøÇ„", L"tyu"},
    {L"ÇøÇ•", L"tye"},
    {L"ÇøÇÂ", L"tyo"},
    {L"Ç¿Ç·", L"dya"},
    {L"Ç¿Ç°", L"dyi"},
    {L"Ç¿Ç„", L"dyu"},
    {L"Ç¿Ç•", L"dye"},
    {L"Ç¿ÇÂ", L"dyo"},
    {L"Ç≈Ç°", L"dhi"},
    {L"Ç…Ç·", L"nya"},
    {L"Ç…Ç°", L"nyi"},
    {L"Ç…Ç„", L"nyu"},
    {L"Ç…Ç•", L"nye"},
    {L"Ç…ÇÂ", L"nyo"},
    {L"Ç–Ç·", L"hya"},
    {L"Ç–Ç°", L"hyi"},
    {L"Ç–Ç„", L"hyu"},
    {L"Ç–Ç•", L"hye"},
    {L"Ç–ÇÂ", L"hyo"},
    {L"Ç—Ç·", L"bya"},
    {L"Ç—Ç°", L"byi"},
    {L"Ç—Ç„", L"byu"},
    {L"Ç—Ç•", L"bye"},
    {L"Ç—ÇÂ", L"byo"},
    {L"Ç“Ç·", L"pya"},
    {L"Ç“Ç°", L"pyi"},
    {L"Ç“Ç„", L"pyu"},
    {L"Ç“Ç•", L"pye"},
    {L"Ç“ÇÂ", L"pyo"},
    {L"Ç›Ç·", L"mya"},
    {L"Ç›Ç°", L"myi"},
    {L"Ç›Ç„", L"myu"},
    {L"Ç›Ç•", L"mye"},
    {L"Ç›ÇÂ", L"myo"},
    {L"ÇËÇ·", L"rya"},
    {L"ÇËÇ°", L"ryi"},
    {L"ÇËÇ„", L"ryu"},
    {L"ÇËÇ•", L"rye"},
    {L"ÇËÇÂ", L"ryo"},
    {L"ÇÒÇ†", L"n'a"},
    {L"ÇÒÇ¢", L"n'i"},
    {L"ÇÒÇ§", L"n'u"},
    {L"ÇÒÇ¶", L"n'e"},
    {L"ÇÒÇ®", L"n'o"},
    {L"Ç†", L"a"},
    {L"Ç¢", L"i"},
    {L"Ç§", L"u"},
    {L"Ç¶", L"e"},
    {L"Ç®", L"o"},
    {L"Çü", L"xa"},
    {L"Ç°", L"xi"},
    {L"Ç£", L"xu"},
    {L"Ç•", L"xe"},
    {L"Çß", L"xo"},
    {L"Ç©", L"ka"},
    {L"Ç´", L"ki"},
    {L"Ç≠", L"ku"},
    {L"ÇØ", L"ke"},
    {L"Ç±", L"ko"},
    {L"Ç™", L"ga"},
    {L"Ç¨", L"gi"},
    {L"ÇÆ", L"gu"},
    {L"Ç∞", L"ge"},
    {L"Ç≤", L"go"},
    {L"Ç≥", L"sa"},
    {L"Çµ", L"si"},
    {L"Ç∑", L"su"},
    {L"Çπ", L"se"},
    {L"Çª", L"so"},
    {L"Ç¥", L"za"},
    {L"Ç∂", L"zi"},
    {L"Ç∏", L"zu"},
    {L"Ç∫", L"ze"},
    {L"Çº", L"zo"},
    {L"ÇΩ", L"ta"},
    {L"Çø", L"ti"},
    {L"Ç¬", L"tu"},
    {L"Çƒ", L"te"},
    {L"Ç∆", L"to"},
    {L"Çæ", L"da"},
    {L"Ç¿", L"di"},
    {L"Ç√", L"du"},
    {L"Ç≈", L"de"},
    {L"Ç«", L"do"},
    {L"Ç¡", L"xtu"},
    {L"Ç»", L"na"},
    {L"Ç…", L"ni"},
    {L"Ç ", L"nu"},
    {L"ÇÀ", L"ne"},
    {L"ÇÃ", L"no"},
    {L"ÇÕ", L"ha"},
    {L"Ç–", L"hi"},
    {L"Ç”", L"hu"},
    {L"Ç÷", L"he"},
    {L"ÇŸ", L"ho"},
    {L"ÇŒ", L"ba"},
    {L"Ç—", L"bi"},
    {L"Ç‘", L"bu"},
    {L"Ç◊", L"be"},
    {L"Ç⁄", L"bo"},
    {L"Çœ", L"pa"},
    {L"Ç“", L"pi"},
    {L"Ç’", L"pu"},
    {L"Çÿ", L"pe"},
    {L"Ç€", L"po"},
    {L"Ç‹", L"ma"},
    {L"Ç›", L"mi"},
    {L"Çﬁ", L"mu"},
    {L"Çﬂ", L"me"},
    {L"Ç‡", L"mo"},
    {L"Ç·", L"xya"},
    {L"Ç‚", L"ya"},
    {L"Ç„", L"xyu"},
    {L"Ç‰", L"yu"},
    {L"ÇÂ", L"xyo"},
    {L"ÇÊ", L"yo"},
    {L"ÇÁ", L"ra"},
    {L"ÇË", L"ri"},
    {L"ÇÈ", L"ru"},
    {L"ÇÍ", L"re"},
    {L"ÇÎ", L"ro"},
    {L"ÇÏ", L"xwa"},
    {L"ÇÌ", L"wa"},
    {L"ÇÓ", L"wi"},
    {L"ÇÔ", L"we"},
    {L"Ç", L"wo"},
    {L"ÇÒ", L"n"},
    {L"Å[", L"-"},
    {L"Å`", L"~"},
    {L"Åu", L"["},
    {L"Åv", L"]"},
};

// ÉçÅ[É}éöïœä∑ÇÃÉeÅ[ÉuÉãÅB
static KEYVALUEEXTRA normal_roman_table[] = {
    {L"a", L"Ç†"},
    {L"i", L"Ç¢"},
    {L"u", L"Ç§"},
    {L"e", L"Ç¶"},
    {L"o", L"Ç®"},
    {L"ka", L"Ç©"},
    {L"ki", L"Ç´"},
    {L"ku", L"Ç≠"},
    {L"ke", L"ÇØ"},
    {L"ko", L"Ç±"},
    {L"sa", L"Ç≥"},
    {L"si", L"Çµ"},
    {L"su", L"Ç∑"},
    {L"se", L"Çπ"},
    {L"so", L"Çª"},
    {L"ta", L"ÇΩ"},
    {L"ti", L"Çø"},
    {L"tu", L"Ç¬"},
    {L"te", L"Çƒ"},
    {L"to", L"Ç∆"},
    {L"na", L"Ç»"},
    {L"ni", L"Ç…"},
    {L"nu", L"Ç "},
    {L"ne", L"ÇÀ"},
    {L"no", L"ÇÃ"},
    {L"ha", L"ÇÕ"},
    {L"hi", L"Ç–"},
    {L"hu", L"Ç”"},
    {L"he", L"Ç÷"},
    {L"ho", L"ÇŸ"},
    {L"ma", L"Ç‹"},
    {L"mi", L"Ç›"},
    {L"mu", L"Çﬁ"},
    {L"me", L"Çﬂ"},
    {L"mo", L"Ç‡"},
    {L"ya", L"Ç‚"},
    {L"yi", L"Ç¢"},
    {L"yu", L"Ç‰"},
    {L"ye", L"Ç¢Ç•"},
    {L"yo", L"ÇÊ"},
    {L"ra", L"ÇÁ"},
    {L"ri", L"ÇË"},
    {L"ru", L"ÇÈ"},
    {L"re", L"ÇÍ"},
    {L"ro", L"ÇÎ"},
    {L"wa", L"ÇÌ"},
    {L"wi", L"Ç§Ç°"},
    {L"wu", L"Ç§"},
    {L"we", L"Ç§Ç•"},
    {L"wo", L"Ç"},
    {L"ga", L"Ç™"},
    {L"gi", L"Ç¨"},
    {L"gu", L"ÇÆ"},
    {L"ge", L"Ç∞"},
    {L"go", L"Ç≤"},
    {L"za", L"Ç¥"},
    {L"zi", L"Ç∂"},
    {L"zu", L"Ç∏"},
    {L"ze", L"Ç∫"},
    {L"zo", L"Çº"},
    {L"da", L"Çæ"},
    {L"di", L"Ç¿"},
    {L"du", L"Ç√"},
    {L"de", L"Ç≈"},
    {L"do", L"Ç«"},
    {L"ba", L"ÇŒ"},
    {L"bi", L"Ç—"},
    {L"bu", L"Ç‘"},
    {L"be", L"Ç◊"},
    {L"bo", L"Ç⁄"},
    {L"pa", L"Çœ"},
    {L"pi", L"Ç“"},
    {L"pu", L"Ç’"},
    {L"pe", L"Çÿ"},
    {L"po", L"Ç€"},
    {L"fa", L"Ç”Çü"},
    {L"fi", L"Ç”Ç°"},
    {L"fu", L"Ç”"},
    {L"fe", L"Ç”Ç•"},
    {L"fo", L"Ç”Çß"},
    {L"ja", L"Ç∂Ç·"},
    {L"ji", L"Ç∂"},
    {L"ju", L"Ç∂Ç„"},
    {L"je", L"Ç∂Ç•"},
    {L"jo", L"Ç∂ÇÂ"},
    {L"ca", L"Ç©"},
    {L"ci", L"Çµ"},
    {L"cu", L"Ç≠"},
    {L"ce", L"Çπ"},
    {L"co", L"Ç±"},
    {L"qa", L"Ç≠Çü"},
    {L"qi", L"Ç≠Ç°"},
    {L"qu", L"Ç≠"},
    {L"qe", L"Ç≠Ç•"},
    {L"qo", L"Ç≠Çß"},
    {L"va", L"ÉîÇü"},
    {L"vi", L"ÉîÇ°"},
    {L"vu", L"Éî"},
    {L"ve", L"ÉîÇ•"},
    {L"vo", L"ÉîÇß"},
    {L"kya", L"Ç´Ç·"},
    {L"kyi", L"Ç´Ç°"},
    {L"kyu", L"Ç´Ç„"},
    {L"kye", L"Ç´Ç•"},
    {L"kyo", L"Ç´ÇÂ"},
    {L"gya", L"Ç¨Ç·"},
    {L"gyi", L"Ç¨Ç°"},
    {L"gyu", L"Ç¨Ç„"},
    {L"gye", L"Ç¨Ç•"},
    {L"gyo", L"Ç¨ÇÂ"},
    {L"sya", L"ÇµÇ·"},
    {L"syi", L"ÇµÇ°"},
    {L"syu", L"ÇµÇ„"},
    {L"sye", L"ÇµÇ•"},
    {L"syo", L"ÇµÇÂ"},
    {L"zya", L"Ç∂Ç·"},
    {L"zyi", L"Ç∂Ç°"},
    {L"zyu", L"Ç∂Ç„"},
    {L"zye", L"Ç∂Ç•"},
    {L"zyo", L"Ç∂ÇÂ"},
    {L"tya", L"ÇøÇ·"},
    {L"tyi", L"ÇøÇ°"},
    {L"tyu", L"ÇøÇ„"},
    {L"tye", L"ÇøÇ•"},
    {L"tyo", L"ÇøÇÂ"},
    {L"dya", L"Ç¿Ç·"},
    {L"dyi", L"Ç¿Ç°"},
    {L"dyu", L"Ç¿Ç„"},
    {L"dye", L"Ç¿Ç•"},
    {L"dyo", L"Ç¿ÇÂ"},
    {L"nya", L"Ç…Ç·"},
    {L"nyi", L"Ç…Ç°"},
    {L"nyu", L"Ç…Ç„"},
    {L"nye", L"Ç…Ç•"},
    {L"nyo", L"Ç…ÇÂ"},
    {L"hya", L"Ç–Ç·"},
    {L"hyi", L"Ç–Ç°"},
    {L"hyu", L"Ç–Ç„"},
    {L"hye", L"Ç–Ç•"},
    {L"hyo", L"Ç–ÇÂ"},
    {L"bya", L"Ç—Ç·"},
    {L"byi", L"Ç—Ç°"},
    {L"byu", L"Ç—Ç„"},
    {L"bye", L"Ç—Ç•"},
    {L"byo", L"Ç—ÇÂ"},
    {L"pya", L"Ç“Ç·"},
    {L"pyi", L"Ç“Ç°"},
    {L"pyu", L"Ç“Ç„"},
    {L"pye", L"Ç“Ç•"},
    {L"pyo", L"Ç“ÇÂ"},
    {L"mya", L"Ç›Ç·"},
    {L"myi", L"Ç›Ç°"},
    {L"myu", L"Ç›Ç„"},
    {L"mye", L"Ç›Ç•"},
    {L"myo", L"Ç›ÇÂ"},
    {L"rya", L"ÇËÇ·"},
    {L"ryi", L"ÇËÇ°"},
    {L"ryu", L"ÇËÇ„"},
    {L"rye", L"ÇËÇ•"},
    {L"ryo", L"ÇËÇÂ"},
    {L"jya", L"Ç∂Ç·"},
    {L"jyi", L"Ç∂Ç°"},
    {L"jyu", L"Ç∂Ç„"},
    {L"jye", L"Ç∂Ç•"},
    {L"jyo", L"Ç∂ÇÂ"},
    {L"vya", L"ÉîÇ·"},
    {L"vyi", L"ÉîÇ°"},
    {L"vyu", L"ÉîÇ„"},
    {L"vye", L"ÉîÇ•"},
    {L"vyo", L"ÉîÇÂ"},
    {L"qya", L"Ç≠Ç·"},
    {L"qyi", L"Ç≠Ç°"},
    {L"qyu", L"Ç≠Ç„"},
    {L"qye", L"Ç≠Ç•"},
    {L"qyo", L"Ç≠ÇÂ"},
    {L"cya", L"ÇøÇ·"},
    {L"cyi", L"ÇøÇ°"},
    {L"cyu", L"ÇøÇ„"},
    {L"cye", L"ÇøÇ•"},
    {L"cyo", L"ÇøÇÂ"},
    {L"fya", L"Ç”Ç·"},
    {L"fyi", L"Ç”Ç°"},
    {L"fyu", L"Ç”Ç„"},
    {L"fye", L"Ç”Ç•"},
    {L"fyo", L"Ç”ÇÂ"},
    {L"sha", L"ÇµÇ·"},
    {L"shi", L"Çµ"},
    {L"shu", L"ÇµÇ„"},
    {L"she", L"ÇµÇ•"},
    {L"sho", L"ÇµÇÂ"},
    {L"cha", L"ÇøÇ·"},
    {L"chi", L"Çø"},
    {L"chu", L"ÇøÇ„"},
    {L"che", L"ÇøÇ•"},
    {L"cho", L"ÇøÇÂ"},
    {L"tha", L"ÇƒÇ·"},
    {L"thi", L"ÇƒÇ°"},
    {L"thu", L"ÇƒÇ„"},
    {L"the", L"ÇƒÇ•"},
    {L"tho", L"ÇƒÇÂ"},
    {L"dha", L"Ç≈Ç·"},
    {L"dhi", L"Ç≈Ç°"},
    {L"dhu", L"Ç≈Ç„"},
    {L"dhe", L"Ç≈Ç•"},
    {L"dho", L"Ç≈ÇÂ"},
    {L"wha", L"Ç§Çü"},
    {L"whi", L"Ç§Ç°"},
    {L"whu", L"Ç§"},
    {L"whe", L"Ç§Ç•"},
    {L"who", L"Ç§Çß"},
    {L"kwa", L"Ç≠Çü"},
    {L"kwi", L"Ç≠Ç°"},
    {L"kwu", L"Ç≠Ç£"},
    {L"kwe", L"Ç≠Ç•"},
    {L"kwo", L"Ç≠Çß"},
    {L"qwa", L"Ç≠Çü"},
    {L"qwi", L"Ç≠Ç°"},
    {L"qwu", L"Ç≠Ç£"},
    {L"qwe", L"Ç≠Ç•"},
    {L"qwo", L"Ç≠Çß"},
    {L"gwa", L"ÇÆÇü"},
    {L"gwi", L"ÇÆÇ°"},
    {L"gwu", L"ÇÆÇ£"},
    {L"gwe", L"ÇÆÇ•"},
    {L"gwo", L"ÇÆÇß"},
    {L"swa", L"Ç∑Çü"},
    {L"swi", L"Ç∑Ç°"},
    {L"swu", L"Ç∑Ç£"},
    {L"swe", L"Ç∑Ç•"},
    {L"swo", L"Ç∑Çß"},
    {L"twa", L"Ç∆Çü"},
    {L"twi", L"Ç∆Ç°"},
    {L"twu", L"Ç∆Ç£"},
    {L"twe", L"Ç∆Ç•"},
    {L"two", L"Ç∆Çß"},
    {L"dwa", L"Ç«Çü"},
    {L"dwi", L"Ç«Ç°"},
    {L"dwu", L"Ç«Ç£"},
    {L"dwe", L"Ç«Ç•"},
    {L"dwo", L"Ç«Çß"},
    {L"fwa", L"Ç”Çü"},
    {L"fwi", L"Ç”Ç°"},
    {L"fwu", L"Ç”Ç£"},
    {L"fwe", L"Ç”Ç•"},
    {L"fwo", L"Ç”Çß"},
    {L"tsa", L"Ç¬Çü"},
    {L"tsi", L"Ç¬Ç°"},
    {L"tsu", L"Ç¬"},
    {L"tse", L"Ç¬Ç•"},
    {L"tso", L"Ç¬Çß"},
    {L"la", L"Çü"},
    {L"li", L"Ç°"},
    {L"lu", L"Ç£"},
    {L"le", L"Ç•"},
    {L"lo", L"Çß"},
    {L"lya", L"Ç·"},
    {L"lyi", L"Ç°"},
    {L"lyu", L"Ç„"},
    {L"lye", L"Ç•"},
    {L"lyo", L"ÇÂ"},
    {L"xa", L"Çü"},
    {L"xi", L"Ç°"},
    {L"xu", L"Ç£"},
    {L"xe", L"Ç•"},
    {L"xo", L"Çß"},
    {L"xya", L"Ç·"},
    {L"xyi", L"Ç°"},
    {L"xyu", L"Ç„"},
    {L"xye", L"Ç•"},
    {L"xyo", L"ÇÂ"},
    {L"lwa", L"ÇÏ"},
    {L"xwa", L"ÇÏ"},
    {L"lka", L"Éï"},
    {L"xka", L"Éï"},
    {L"lke", L"Éñ"},
    {L"xke", L"Éñ"},
    {L"ltu", L"Ç¡"},
    {L"ltsu", L"Ç¡"},
    {L"xtu", L"Ç¡"},
    {L"nn", L"ÇÒ"},
    {L"nÅf", L"ÇÒ"},
    {L"xn", L"ÇÒ"},
    {L"nb", L"ÇÒ", L"b"},
    {L"nc", L"ÇÒ", L"c"},
    {L"nd", L"ÇÒ", L"d"},
    {L"nf", L"ÇÒ", L"f"},
    {L"ng", L"ÇÒ", L"g"},
    {L"nh", L"ÇÒ", L"h"},
    {L"nj", L"ÇÒ", L"j"},
    {L"nk", L"ÇÒ", L"k"},
    {L"nl", L"ÇÒ", L"l"},
    {L"nm", L"ÇÒ", L"m"},
    {L"np", L"ÇÒ", L"p"},
    {L"nq", L"ÇÒ", L"q"},
    {L"nr", L"ÇÒ", L"r"},
    {L"ns", L"ÇÒ", L"s"},
    {L"nt", L"ÇÒ", L"t"},
    {L"nv", L"ÇÒ", L"v"},
    {L"nw", L"ÇÒ", L"w"},
    {L"nx", L"ÇÒ", L"x"},
    {L"nz", L"ÇÒ", L"z"},
    {L"n-", L"ÇÒ", L"-"},
    {L"n@", L"ÇÒ", L"@"},
};

// ë£âπÉeÅ[ÉuÉãÅB
static KEYVALUE sokuon_table[] = {
    {L"kka", L"Ç¡Ç©"},
    {L"kki", L"Ç¡Ç´"},
    {L"kku", L"Ç¡Ç≠"},
    {L"kke", L"Ç¡ÇØ"},
    {L"kko", L"Ç¡Ç±"},
    {L"ssa", L"Ç¡Ç≥"},
    {L"ssi", L"Ç¡Çµ"},
    {L"ssu", L"Ç¡Ç∑"},
    {L"sse", L"Ç¡Çπ"},
    {L"sso", L"Ç¡Çª"},
    {L"tta", L"Ç¡ÇΩ"},
    {L"tti", L"Ç¡Çø"},
    {L"ttu", L"Ç¡Ç¬"},
    {L"tte", L"Ç¡Çƒ"},
    {L"tto", L"Ç¡Ç∆"},
    {L"hha", L"Ç¡ÇÕ"},
    {L"hhi", L"Ç¡Ç–"},
    {L"hhu", L"Ç¡Ç”"},
    {L"hhe", L"Ç¡Ç÷"},
    {L"hho", L"Ç¡ÇŸ"},
    {L"mma", L"Ç¡Ç‹"},
    {L"mmi", L"Ç¡Ç›"},
    {L"mmu", L"Ç¡Çﬁ"},
    {L"mme", L"Ç¡Çﬂ"},
    {L"mmo", L"Ç¡Ç‡"},
    {L"yya", L"Ç¡Ç‚"},
    {L"yyi", L"Ç¡Ç¢"},
    {L"yyu", L"Ç¡Ç‰"},
    {L"yye", L"Ç¡Ç¢Ç•"},
    {L"yyo", L"Ç¡ÇÊ"},
    {L"rra", L"Ç¡ÇÁ"},
    {L"rri", L"Ç¡ÇË"},
    {L"rru", L"Ç¡ÇÈ"},
    {L"rre", L"Ç¡ÇÍ"},
    {L"rro", L"Ç¡ÇÎ"},
    {L"wwa", L"Ç¡ÇÌ"},
    {L"wwi", L"Ç¡Ç§Ç°"},
    {L"wwu", L"Ç¡Ç§"},
    {L"wwe", L"Ç¡Ç§Ç•"},
    {L"wwo", L"Ç¡Ç"},
    {L"gga", L"Ç¡Ç™"},
    {L"ggi", L"Ç¡Ç¨"},
    {L"ggu", L"Ç¡ÇÆ"},
    {L"gge", L"Ç¡Ç∞"},
    {L"ggo", L"Ç¡Ç≤"},
    {L"zza", L"Ç¡Ç¥"},
    {L"zzi", L"Ç¡Ç∂"},
    {L"zzu", L"Ç¡Ç∏"},
    {L"zze", L"Ç¡Ç∫"},
    {L"zzo", L"Ç¡Çº"},
    {L"dda", L"Ç¡Çæ"},
    {L"ddi", L"Ç¡Ç¿"},
    {L"ddu", L"Ç¡Ç√"},
    {L"dde", L"Ç¡Ç≈"},
    {L"ddo", L"Ç¡Ç«"},
    {L"bba", L"Ç¡ÇŒ"},
    {L"bbi", L"Ç¡Ç—"},
    {L"bbu", L"Ç¡Ç‘"},
    {L"bbe", L"Ç¡Ç◊"},
    {L"bbo", L"Ç¡Ç⁄"},
    {L"ppa", L"Ç¡Çœ"},
    {L"ppi", L"Ç¡Ç“"},
    {L"ppu", L"Ç¡Ç’"},
    {L"ppe", L"Ç¡Çÿ"},
    {L"ppo", L"Ç¡Ç€"},
    {L"ffa", L"Ç¡Ç”Çü"},
    {L"ffi", L"Ç¡Ç”Ç°"},
    {L"ffu", L"Ç¡Ç”"},
    {L"ffe", L"Ç¡Ç”Ç•"},
    {L"ffo", L"Ç¡Ç”Çß"},
    {L"jja", L"Ç¡Ç∂Ç·"},
    {L"jji", L"Ç¡Ç∂"},
    {L"jju", L"Ç¡Ç∂Ç„"},
    {L"jje", L"Ç¡Ç∂Ç•"},
    {L"jjo", L"Ç¡Ç∂ÇÂ"},
    {L"cca", L"Ç¡Ç©"},
    {L"cci", L"Ç¡Çµ"},
    {L"ccu", L"Ç¡Ç≠"},
    {L"cce", L"Ç¡Çπ"},
    {L"cco", L"Ç¡Ç±"},
    {L"qqa", L"Ç¡Ç≠Çü"},
    {L"qqi", L"Ç¡Ç≠Ç°"},
    {L"qqu", L"Ç¡Ç≠"},
    {L"qqe", L"Ç¡Ç≠Ç•"},
    {L"qqo", L"Ç¡Ç≠Çß"},
    {L"vva", L"Ç¡ÉîÇü"},
    {L"vvi", L"Ç¡ÉîÇ°"},
    {L"vvu", L"Ç¡Éî"},
    {L"vve", L"Ç¡ÉîÇ•"},
    {L"vvo", L"Ç¡ÉîÇß"},
    {L"kkya", L"Ç¡Ç´Ç·"},
    {L"kkyi", L"Ç¡Ç´Ç°"},
    {L"kkyu", L"Ç¡Ç´Ç„"},
    {L"kkye", L"Ç¡Ç´Ç•"},
    {L"kkyo", L"Ç¡Ç´ÇÂ"},
    {L"ggya", L"Ç¡Ç¨Ç·"},
    {L"ggyi", L"Ç¡Ç¨Ç°"},
    {L"ggyu", L"Ç¡Ç¨Ç„"},
    {L"ggye", L"Ç¡Ç¨Ç•"},
    {L"ggyo", L"Ç¡Ç¨ÇÂ"},
    {L"ssya", L"Ç¡ÇµÇ·"},
    {L"ssyi", L"Ç¡ÇµÇ°"},
    {L"ssyu", L"Ç¡ÇµÇ„"},
    {L"ssye", L"Ç¡ÇµÇ•"},
    {L"ssyo", L"Ç¡ÇµÇÂ"},
    {L"zzya", L"Ç¡Ç∂Ç·"},
    {L"zzyi", L"Ç¡Ç∂Ç°"},
    {L"zzyu", L"Ç¡Ç∂Ç„"},
    {L"zzye", L"Ç¡Ç∂Ç•"},
    {L"zzyo", L"Ç¡Ç∂ÇÂ"},
    {L"ttya", L"Ç¡ÇøÇ·"},
    {L"ttyi", L"Ç¡ÇøÇ°"},
    {L"ttyu", L"Ç¡ÇøÇ„"},
    {L"ttye", L"Ç¡ÇøÇ•"},
    {L"ttyo", L"Ç¡ÇøÇÂ"},
    {L"ddya", L"Ç¡Ç¿Ç·"},
    {L"ddyi", L"Ç¡Ç¿Ç°"},
    {L"ddyu", L"Ç¡Ç¿Ç„"},
    {L"ddye", L"Ç¡Ç¿Ç•"},
    {L"ddyo", L"Ç¡Ç¿ÇÂ"},
    {L"hhya", L"Ç¡Ç–Ç·"},
    {L"hhyi", L"Ç¡Ç–Ç°"},
    {L"hhyu", L"Ç¡Ç–Ç„"},
    {L"hhye", L"Ç¡Ç–Ç•"},
    {L"hhyo", L"Ç¡Ç–ÇÂ"},
    {L"bbya", L"Ç¡Ç—Ç·"},
    {L"bbyi", L"Ç¡Ç—Ç°"},
    {L"bbyu", L"Ç¡Ç—Ç„"},
    {L"bbye", L"Ç¡Ç—Ç•"},
    {L"bbyo", L"Ç¡Ç—ÇÂ"},
    {L"ppya", L"Ç¡Ç“Ç·"},
    {L"ppyi", L"Ç¡Ç“Ç°"},
    {L"ppyu", L"Ç¡Ç“Ç„"},
    {L"ppye", L"Ç¡Ç“Ç•"},
    {L"ppyo", L"Ç¡Ç“ÇÂ"},
    {L"mmya", L"Ç¡Ç›Ç·"},
    {L"mmyi", L"Ç¡Ç›Ç°"},
    {L"mmyu", L"Ç¡Ç›Ç„"},
    {L"mmye", L"Ç¡Ç›Ç•"},
    {L"mmyo", L"Ç¡Ç›ÇÂ"},
    {L"rrya", L"Ç¡ÇËÇ·"},
    {L"rryi", L"Ç¡ÇËÇ°"},
    {L"rryu", L"Ç¡ÇËÇ„"},
    {L"rrye", L"Ç¡ÇËÇ•"},
    {L"rryo", L"Ç¡ÇËÇÂ"},
    {L"jjya", L"Ç¡Ç∂Ç·"},
    {L"jjyi", L"Ç¡Ç∂Ç°"},
    {L"jjyu", L"Ç¡Ç∂Ç„"},
    {L"jjye", L"Ç¡Ç∂Ç•"},
    {L"jjyo", L"Ç¡Ç∂ÇÂ"},
    {L"vvya", L"Ç¡ÉîÇ·"},
    {L"vvyi", L"Ç¡ÉîÇ°"},
    {L"vvyu", L"Ç¡ÉîÇ„"},
    {L"vvye", L"Ç¡ÉîÇ•"},
    {L"vvyo", L"Ç¡ÉîÇÂ"},
    {L"qqya", L"Ç¡Ç≠Ç·"},
    {L"qqyi", L"Ç¡Ç≠Ç°"},
    {L"qqyu", L"Ç¡Ç≠Ç„"},
    {L"qqye", L"Ç¡Ç≠Ç•"},
    {L"qqyo", L"Ç¡Ç≠ÇÂ"},
    {L"ccya", L"Ç¡ÇøÇ·"},
    {L"ccyi", L"Ç¡ÇøÇ°"},
    {L"ccyu", L"Ç¡ÇøÇ„"},
    {L"ccye", L"Ç¡ÇøÇ•"},
    {L"ccyo", L"Ç¡ÇøÇÂ"},
    {L"ffya", L"Ç¡Ç”Ç·"},
    {L"ffyi", L"Ç¡Ç”Ç°"},
    {L"ffyu", L"Ç¡Ç”Ç„"},
    {L"ffye", L"Ç¡Ç”Ç•"},
    {L"ffyo", L"Ç¡Ç”ÇÂ"},
    {L"ssha", L"Ç¡ÇµÇ·"},
    {L"sshi", L"Ç¡Çµ"},
    {L"sshu", L"Ç¡ÇµÇ„"},
    {L"sshe", L"Ç¡ÇµÇ•"},
    {L"ssho", L"Ç¡ÇµÇÂ"},
    {L"ccha", L"Ç¡ÇøÇ·"},
    {L"cchi", L"Ç¡Çø"},
    {L"cchu", L"Ç¡ÇøÇ„"},
    {L"cche", L"Ç¡ÇøÇ•"},
    {L"ccho", L"Ç¡ÇøÇÂ"},
    {L"ttha", L"Ç¡ÇƒÇ·"},
    {L"tthi", L"Ç¡ÇƒÇ°"},
    {L"tthu", L"Ç¡ÇƒÇ„"},
    {L"tthe", L"Ç¡ÇƒÇ•"},
    {L"ttho", L"Ç¡ÇƒÇÂ"},
    {L"ddha", L"Ç¡Ç≈Ç·"},
    {L"ddhi", L"Ç¡Ç≈Ç°"},
    {L"ddhu", L"Ç¡Ç≈Ç„"},
    {L"ddhe", L"Ç¡Ç≈Ç•"},
    {L"ddho", L"Ç¡Ç≈ÇÂ"},
    {L"wwha", L"Ç¡Ç§Çü"},
    {L"wwhi", L"Ç¡Ç§Ç°"},
    {L"wwhu", L"Ç¡Ç§"},
    {L"wwhe", L"Ç¡Ç§Ç•"},
    {L"wwho", L"Ç¡Ç§Çß"},
    {L"kkwa", L"Ç¡Ç≠Çü"},
    {L"kkwi", L"Ç¡Ç≠Ç°"},
    {L"kkwu", L"Ç¡Ç≠Ç£"},
    {L"kkwe", L"Ç¡Ç≠Ç•"},
    {L"kkwo", L"Ç¡Ç≠Çß"},
    {L"qqwa", L"Ç¡Ç≠Çü"},
    {L"qqwi", L"Ç¡Ç≠Ç°"},
    {L"qqwu", L"Ç¡Ç≠Ç£"},
    {L"qqwe", L"Ç¡Ç≠Ç•"},
    {L"qqwo", L"Ç¡Ç≠Çß"},
    {L"ggwa", L"Ç¡ÇÆÇü"},
    {L"ggwi", L"Ç¡ÇÆÇ°"},
    {L"ggwu", L"Ç¡ÇÆÇ£"},
    {L"ggwe", L"Ç¡ÇÆÇ•"},
    {L"ggwo", L"Ç¡ÇÆÇß"},
    {L"sswa", L"Ç¡Ç∑Çü"},
    {L"sswi", L"Ç¡Ç∑Ç°"},
    {L"sswu", L"Ç¡Ç∑Ç£"},
    {L"sswe", L"Ç¡Ç∑Ç•"},
    {L"sswo", L"Ç¡Ç∑Çß"},
    {L"ttwa", L"Ç¡Ç∆Çü"},
    {L"ttwi", L"Ç¡Ç∆Ç°"},
    {L"ttwu", L"Ç¡Ç∆Ç£"},
    {L"ttwe", L"Ç¡Ç∆Ç•"},
    {L"ttwo", L"Ç¡Ç∆Çß"},
    {L"ddwa", L"Ç¡Ç«Çü"},
    {L"ddwi", L"Ç¡Ç«Ç°"},
    {L"ddwu", L"Ç¡Ç«Ç£"},
    {L"ddwe", L"Ç¡Ç«Ç•"},
    {L"ddwo", L"Ç¡Ç«Çß"},
    {L"ffwa", L"Ç¡Ç”Çü"},
    {L"ffwi", L"Ç¡Ç”Ç°"},
    {L"ffwu", L"Ç¡Ç”Ç£"},
    {L"ffwe", L"Ç¡Ç”Ç•"},
    {L"ffwo", L"Ç¡Ç”Çß"},
    {L"ttsa", L"Ç¡Ç¬Çü"},
    {L"ttsi", L"Ç¡Ç¬Ç°"},
    {L"ttsu", L"Ç¡Ç¬"},
    {L"ttse", L"Ç¡Ç¬Ç•"},
    {L"ttso", L"Ç¡Ç¬Çß"},
    {L"lla", L"Ç¡Çü"},
    {L"lli", L"Ç¡Ç°"},
    {L"llu", L"Ç¡Ç£"},
    {L"lle", L"Ç¡Ç•"},
    {L"llo", L"Ç¡Çß"},
    {L"llya", L"Ç¡Ç·"},
    {L"llyi", L"Ç¡Ç°"},
    {L"llyu", L"Ç¡Ç„"},
    {L"llye", L"Ç¡Ç•"},
    {L"llyo", L"Ç¡ÇÂ"},
    {L"xxa", L"Ç¡Çü"},
    {L"xxi", L"Ç¡Ç°"},
    {L"xxu", L"Ç¡Ç£"},
    {L"xxe", L"Ç¡Ç•"},
    {L"xxo", L"Ç¡Çß"},
    {L"xxya", L"Ç¡Ç·"},
    {L"xxyi", L"Ç¡Ç°"},
    {L"xxyu", L"Ç¡Ç„"},
    {L"xxye", L"Ç¡Ç•"},
    {L"xxyo", L"Ç¡ÇÂ"},
    {L"llwa", L"Ç¡ÇÏ"},
    {L"xxwa", L"Ç¡ÇÏ"},
    {L"llka", L"Ç¡Éï"},
    {L"xxka", L"Ç¡Éï"},
    {L"llke", L"Ç¡Éñ"},
    {L"xxke", L"Ç¡Éñ"},
    {L"lltu", L"Ç¡Ç¡"},
    {L"lltsu", L"Ç¡Ç¡"},
    {L"xxtu", L"Ç¡Ç¡"},
    {L"xxn", L"Ç¡ÇÒ"},
};

// ÉJÉiì¸óÕÉeÅ[ÉuÉãÅB
static KEYVALUE kana2type_table[] = {
    {L"Ç†", L"3"},
    {L"Ç¢", L"e"},
    {L"Ç§", L"4"},
    {L"Ç¶", L"5"},
    {L"Ç®", L"6"},
    {L"Ç©", L"t"},
    {L"Ç´", L"g"},
    {L"Ç≠", L"h"},
    {L"ÇØ", L":"},
    {L"Ç±", L"b"},
    {L"Ç≥", L"x"},
    {L"Çµ", L"d"},
    {L"Ç∑", L"r"},
    {L"Çπ", L"p"},
    {L"Çª", L"c"},
    {L"ÇΩ", L"q"},
    {L"Çø", L"a"},
    {L"Ç¬", L"z"},
    {L"Çƒ", L"w"},
    {L"Ç∆", L"s"},
    {L"Ç»", L"u"},
    {L"Ç…", L"i"},
    {L"Ç ", L"1"},
    {L"ÇÀ", L","},
    {L"ÇÃ", L"k"},
    {L"ÇÕ", L"f"},
    {L"Ç–", L"v"},
    {L"Ç”", L"2"},
    {L"Ç÷", L"^"},
    {L"ÇŸ", L"-"},
    {L"Ç‹", L"j"},
    {L"Ç›", L"n"},
    {L"Çﬁ", L"]"},
    {L"Çﬂ", L"/"},
    {L"Ç‡", L"m"},
    {L"Ç‚", L"7"},
    {L"Ç‰", L"8"},
    {L"ÇÊ", L"9"},
    {L"ÇÁ", L"o"},
    {L"ÇË", L"l"},
    {L"ÇÈ", L"."},
    {L"ÇÍ", L";"},
    {L"ÇÎ", L"\\"},
    {L"ÇÌ", L"0"},
    {L"Ç", L"0"},
    {L"ÇÒ", L"y"},
    {L"Ç™", L"t@"},
    {L"Ç¨", L"g@"},
    {L"ÇÆ", L"h@"},
    {L"Ç∞", L":@"},
    {L"Ç≤", L"b@"},
    {L"Ç¥", L"x@"},
    {L"Ç∂", L"d@"},
    {L"Ç∏", L"r@"},
    {L"Ç∫", L"p@"},
    {L"Çº", L"c@"},
    {L"Çæ", L"q@"},
    {L"Ç¿", L"a@"},
    {L"Ç√", L"z@"},
    {L"Ç≈", L"w@"},
    {L"Ç«", L"s@"},
    {L"ÇŒ", L"f@"},
    {L"Ç—", L"v@"},
    {L"Ç‘", L"2@"},
    {L"Ç◊", L"^@"},
    {L"Ç⁄", L"-@"},
    {L"Çœ", L"f["},
    {L"Ç“", L"v["},
    {L"Ç’", L"2["},
    {L"Çÿ", L"^["},
    {L"Ç€", L"-["},
    {L"Çü", L"#"},
    {L"Ç°", L"E"},
    {L"Ç£", L"$"},
    {L"Ç•", L"%"},
    {L"Çß", L"&"},
    {L"ÇÏ", L"0"},
    {L"Ç¡", L"Z"},
    {L"ÅJ", L"@"},
    {L"ÅK", L"["},
    {L"Å[", L"-"},
};

//////////////////////////////////////////////////////////////////////////////

// Ç–ÇÁÇ™Ç»Ç©ÇÁÉçÅ[É}éöÇ÷ï∂éöóÒÇïœä∑ÅB
std::wstring hiragana_to_roman(std::wstring hiragana) {
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

// ÉçÅ[É}éöÇ©ÇÁÇ–ÇÁÇ™Ç»Ç÷ï∂éöóÒÇïœä∑ÅB
std::wstring roman_to_hiragana(std::wstring roman) {
    std::wstring hiragana, str;
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
                    hiragana += L'ÅC';
                else
                    hiragana += L'ÅA';
                k += 1;
                found = true;
            }
            else if (roman[k] == L'.') {
                if (Config_GetDWORD(L"bCommaPeriod", FALSE))
                    hiragana += L'ÅD';
                else
                    hiragana += L'ÅB';
                k += 1;
                found = true;
            }
        }
        if (!found) hiragana += roman[k++];
    }
    return hiragana;
} // roman_to_hiragana

// ÉçÅ[É}éöÇ©ÇÁÉJÉ^ÉJÉiÇ÷ï∂éöóÒÇïœä∑ÅB
std::wstring roman_to_katakana(std::wstring roman) {
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
                    katakana += L'ÅC';
                else
                    katakana += L'ÅA';
                k += 1;
                found = true;
            }
            else if (roman[k] == L'.') {
                if (Config_GetDWORD(L"bCommaPeriod", FALSE))
                    katakana += L'ÅD';
                else
                    katakana += L'ÅB';
                k += 1;
                found = true;
            }
        }
        if (!found) katakana += roman[k++];
    }
    return katakana;
} // roman_to_katakana

// ÉçÅ[É}éöÇ©ÇÁîºäpÉJÉiÇ÷ï∂éöóÒÇïœä∑ÅB
std::wstring roman_to_halfwidth_katakana(std::wstring roman) {
    std::wstring katakana, str;
    //DebugPrintW(L"roman_to_halfwidth_katakana(%s)\n", roman.c_str());
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
    //DebugPrintW(L"roman_to_halfwidth_katakana: %s\n", katakana.c_str());
    return katakana;
} // roman_to_halfwidth_katakana

// ÉçÅ[É}éöÇ©ÇÁÇ–ÇÁÇ™Ç»Ç÷ï∂éöóÒÇïœä∑ÅB
std::wstring roman_to_hiragana(std::wstring roman, size_t ichTarget) {
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

// ÉçÅ[É}éöÇ©ÇÁÉJÉ^ÉJÉiÇ÷ï∂éöóÒÇïœä∑ÅB
std::wstring roman_to_katakana(std::wstring roman, size_t ichTarget) {
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

// ÉçÅ[É}éöÇ©ÇÁîºäpÉJÉiÇ÷ï∂éöóÒÇïœä∑ÅB
std::wstring roman_to_halfwidth_katakana(std::wstring roman, size_t ichTarget) {
    //DebugPrintW(L"roman_to_halfwidth_katakana(%s,%d)\n", roman.c_str(), (int)ichTarget);
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
    //DebugPrintW(L"roman_to_halfwidth_katakana: %s\n", roman.c_str());
    return roman;
} // roman_to_halfwidth_katakana

// Ç–ÇÁÇ™Ç»Ç©ÇÁì¸óÕï∂éöóÒÇ÷ï∂éöóÒÇïœä∑ÅB
std::wstring hiragana_to_typing(std::wstring hiragana) {
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

// âºëzÉLÅ[Ç©ÇÁÇ–ÇÁÇ™Ç»Ç÷ÅB
WCHAR vkey_to_hiragana(BYTE vk, BOOL bShift) {
    switch (vk) {
    case VK_A:          return L'Çø';
    case VK_B:          return L'Ç±';
    case VK_C:          return L'Çª';
    case VK_D:          return L'Çµ';
    case VK_E:          return (bShift ? L'Ç°' : L'Ç¢');
    case VK_F:          return L'ÇÕ';
    case VK_G:          return L'Ç´';
    case VK_H:          return L'Ç≠';
    case VK_I:          return L'Ç…';
    case VK_J:          return L'Ç‹';
    case VK_K:          return L'ÇÃ';
    case VK_L:          return L'ÇË';
    case VK_M:          return L'Ç‡';
    case VK_N:          return L'Ç›';
    case VK_O:          return L'ÇÁ';
    case VK_P:          return L'Çπ';
    case VK_Q:          return L'ÇΩ';
    case VK_R:          return L'Ç∑';
    case VK_S:          return L'Ç∆';
    case VK_T:          return L'Ç©';
    case VK_U:          return L'Ç»';
    case VK_V:          return L'Ç–';
    case VK_W:          return L'Çƒ';
    case VK_X:          return L'Ç≥';
    case VK_Y:          return L'ÇÒ';
    case VK_Z:          return (bShift ? L'Ç¡' : L'Ç¬');
    case VK_0:          return (bShift ? L'Ç' : L'ÇÌ');
    case VK_1:          return L'Ç ';
    case VK_2:          return L'Ç”';
    case VK_3:          return (bShift ? L'Çü' : L'Ç†');
    case VK_4:          return (bShift ? L'Ç£' : L'Ç§');
    case VK_5:          return (bShift ? L'Ç•' : L'Ç¶');
    case VK_6:          return (bShift ? L'Çß' : L'Ç®');
    case VK_7:          return (bShift ? L'Ç·' : L'Ç‚');
    case VK_8:          return (bShift ? L'Ç„' : L'Ç‰');
    case VK_9:          return (bShift ? L'ÇÂ' : L'ÇÊ');
    case VK_OEM_PLUS:   return L'ÇÍ';
    case VK_OEM_MINUS:  return L'ÇŸ';
    case VK_OEM_COMMA:
        if (Config_GetDWORD(TEXT("bCommaPeriod"), FALSE))
            return (bShift ? L'ÅC' : L'ÇÀ');
        else
            return (bShift ? L'ÅA' : L'ÇÀ');
    case VK_OEM_PERIOD:
        if (Config_GetDWORD(TEXT("bCommaPeriod"), FALSE))
            return (bShift ? L'ÅD' : L'ÇÈ');
        else
            return (bShift ? L'ÅB' : L'ÇÈ');
    case VK_OEM_1:      return L'ÇØ';
    case VK_OEM_2:      return (bShift ? L'ÅE' : L'Çﬂ');
    case VK_OEM_3:      return L'ÅJ';
    case VK_OEM_4:      return (bShift ? L'Åu' : L'ÅK');
    case VK_OEM_5:      return L'Å[';
    case VK_OEM_6:      return (bShift ? L'Åv' : L'Çﬁ');
    case VK_OEM_7:      return L'Ç÷';
    case VK_OEM_102:    return L'ÇÎ';
    default:            return 0;
    }
} // vkey_to_hiragana

// ì¸óÕÉLÅ[Ç©ÇÁï∂éöÇ÷ÅB
WCHAR typing_key_to_char(BYTE vk, BOOL bShift, BOOL bCapsLock) {
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

// ãÂì_ÅiÉsÉäÉIÉhÅjÇ©ÅH
BOOL is_period(WCHAR ch) {
    return ch == L'ÅB' || ch == L'ÅD' || ch == L'.' || ch == L'°';
}

// ì«ì_ÅiÉRÉìÉ}ÅjÇ©ÅH
BOOL is_comma(WCHAR ch) {
    return ch == L'ÅA' || ch == L'ÅC' || ch == L',' || ch == L'§';
}

// Ç–ÇÁÇ™Ç»Ç©ÅH
BOOL is_hiragana(WCHAR ch) {
    if (0x3040 <= ch && ch <= 0x309F) return TRUE;
    switch (ch) {
    case 0x3095: case 0x3096: case 0x3099: case 0x309A: case 0x309B:
    case 0x309C: case 0x309D: case 0x309E: case 0x30FC:
        return TRUE;
    default:
        return FALSE;
    }
}

// ëSäpÉJÉ^ÉJÉiÇ©ÅH
BOOL is_fullwidth_katakana(WCHAR ch) {
    if (0x30A0 <= ch && ch <= 0x30FF) return TRUE;
    switch (ch) {
    case 0x30FD: case 0x30FE: case 0x3099: case 0x309A: case 0x309B:
    case 0x309C: case 0x30FC:
        return TRUE;
    default:
        return FALSE;
    }
}

// îºäpÉJÉiÇ©ÅH
BOOL is_halfwidth_katakana(WCHAR ch) {
    if (0xFF65 <= ch && ch <= 0xFF9F) return TRUE;
    switch (ch) {
    case 0xFF61: case 0xFF62: case 0xFF63: case 0xFF64:
        return TRUE;
    default:
        return FALSE;
    }
}

// äøéöÇ©ÅH
BOOL is_kanji(WCHAR ch) {
    if (0x4E00 <= ch && ch <= 0x9FFF) return TRUE;
    if (0xF900 <= ch && ch <= 0xFAFF) return TRUE;
    return FALSE;
}

// ã≥àÁäøéöÇ©ÅH
BOOL is_education_kanji(WCHAR ch) {
    return wcschr(L"àÍâπâJâ~â§âŒâ‘äLäwãxã„ã ãÛãCâ∫åéå©å¢å‹å˚çZã‡ç∂éOéRéléqéÖéöé®éµé‘éËè\èoèóè¨ê≥è„êXêlêÖê∂ê¬ê‘êŒó[êÁêÊêÏëêëÅë´ë∫ëÂíjí|íéíÜí¨ìVìcìyìÒì˙ì¸îNîíî™ïSñ{ñºñÿñ⁄ï∂âEóÕóßó—òZà¯âHâ_âìâÄâ∆âΩâƒâ»âÃâÊâÔâÒäCäGäOäpäyäàä‘äÁä‚ä€ãLãDãAã|ãçãõã≥ã≠ãﬂç°åZå`ãûåvå¥åæå≥å√åÀåÍåﬂå„çLåçHåıåˆçlâ©çsçÇçáçëíJçïç◊çÀçÏéZéÜñÓéoésé~éûéõé©êFé∫é–é„éÒèTèHètèëè≠èÍêHêVêeêSê}ì™êîêºêØê∫ê∞ê·êÿëDê¸ëOëgëñëæëΩëÃë‰íÌírínímíÉíãí∑í©íπíºí ì_ìXìdì~ìÅìñìöìåìπìØì«ì‡ìÏì˜înîÉîÑîûîºî‘ïÉï‡ïóï™ï∑ïƒïÍï˚ñkñàñÖñúñ¬ñæñ—ñÂñÏñÈóFójópóàó¢óùòbévà´à¿à√à”àœà„àÁâ@àıà˘â^âjâwâ°âõâÆâ∑â◊äJäEäKäŸä¥ä¶äøä›ãNä˙ãqã}ãâãÖãÜãéã¥ã∆ã»ã«ã‚ãÍãÊãÔã{åNâªåyåWåååàåßå§åŒå…çKå¸ç`çÜç™ç’éMéwénéïéÄéçédégéüéùéñéÆé¿é é“êÃééÁéÂéÊéÛèKèEèWèIèBèdèZèhèãèäèïè¡è∫èÕè§èüèÊêAêgêiê\ê[ê^ê_ê¢êÆëSëzëäëóëßë¨ë∞ëºë≈ë„ëŒë“ëÊëËíZíYíkíÖíçíåí≤í†í«íöíËíÎìJìSì]ìsìxìáìôìoìíìäì§ìÆì∂î_îgîzî{î†î®î≠îΩî¬ç‚îﬂîÁï@î¸ïMïXï\ïaïbïiïâïîïüïûï®ïΩï‘ï◊ï˙ñ°ñΩñ ñ‚ñÚñóRñ˚óVóLó\ótózórómólóéó∑óºóŒó¨óÁóÒó˚òHòaà§àƒà»à à›àÕàﬂàÛâpâhâñâ≠â›â€â¡â âËäBâ¸äQäXäoäeä÷äÆä«äØäœäËäÏãGä¯ãIäÛã@äÌãcã~ãÉãÅãããìã¶ãæã£ã§ã…åPåRåSå^åaåiå|åãåáåöåíå±å≈åÛå˜çDçqçNçêç∑ç≈çÿçﬁçéEç¸éDé@éQéUéYécéÅéjéééméié°éôé´é∏éÿéÌé¸èjèáèâè•èŒèƒèºè‹è€è∆è»êMêbê¥ê√ê¨ê»êœêﬂê‹ê‡êÌëIêÛëÉëàëqë©ë§ë±ë≤ë∑ë‡ë—íBíPíuíáíôíõí∞í‚í·íÍìIìTì`ìkìwìîì≠ì∞ì¡ìæì≈îMëRîOîsî~îéî—îÚîÔïKï[ïWïtï{ïsïvñ≥ïõï≤ï∫ï ï”ïœï÷ñ@ïÔñ]ñqñññûñ¢ñ¨ñØñÒóEóvó{óÅóòó§ó ó«ãôóøó÷óﬁóﬂó‚ó·óòAòVòJò^à≥à⁄àˆâiâqâcâvà’âtââç˜âûâùâ∂âøâﬂâ¬âÕâºâÍââıämäiäzäµä≤äßä·ãKäÓäÒãZã`ãtãåãvãñãèã´ãœã÷ãÂåQåoåâåØåüåîåèåªå¿å∏å¬åÃåÏå¯çuç\çzçkå˙ãªç¨ç∏ç–çÃç»ç€çƒç›çﬂç‡éGé^é_éxéëétéuéîé}é¶éóéØéøé”é…éˆèCèqèpèÄèòèÿè≥èµèÌèÛèèÓêEêDêªê∏ê´ê≠êßê®ê≈ê”ê—ê›ê⁄ê‚ê„ëKëcëfëçëùëúë¢ë•ë™ëÆëπë›ëﬁë‘ícífízí£íˆíÒìGìKìùì∫ì±ìøì∆îCîRî\îjî∆î≈îªîÏî‰îÒîıïUï]ïnïxïzïwïêïúï°ïßï“ïŸï€ïÊñLïÒñhñ\ñ≤ñ±ñ¿ñ»óAó]óaóeó¶ó™óØóÃñfó•àŸà‚àÊâFâfâàâÑâ‰äDätävägäÑäîä»ä±ä≈ä™äÎäˆä˜ãMã^ãzãΩãπãüãÿãŒåhånåxåÄåÉåäå¶åõå†åµåπå»åƒåÎçFçcç@çgç|ç~çíçèçúç¢çªç¿çœçŸçÙç˚é\éèéãéÑéåéäépé•éÃéÀé⁄é·é˜è@é˚èOèAè]ècèkènèÉèêèàèîèúèùè·è´èÈèˆêjêmêÑêÇê°êΩêπê∑êÍêÈêıêÚêÙëPëwëïëtëÄënëãë†ëüë∏ë∂ëÓíTíSíaígíiílíàíâíòí™í∏í°í¿í…ìWì}ì¢ìúìÕìÔì˚îFî]î[îhîxîwîoîqî«î”î€îÈî·ï†ï±ï√ï¿ï¬ï‚ïÈñKïÛñ_ñYñSñáñãñßñøñÕñÛóDóXócóÇó~óëóêóóó†ó’ò_òNï–", ch) != NULL;
}

// èÌópäøéöÇ©ÅH
BOOL is_common_use_kanji(WCHAR ch) {
    return wcschr(L"àüà£à•à§ûBà´à¨à≥àµà∂óíà¿àƒà√à»àﬂà àÕà„àÀàœà–à◊àÿà›à—àŸà⁄àﬁàÃà÷úbà”à·à€à‘à‚à‹àÊàÁàÍàÎàÌàÔàà¯àÛàˆàÙà˜àıâ@à˙âAà˘âBâCâEâFâHâJâSüTê§âYâ^â_âiâjâpâfâhâcârâeâsâqà’âuâvâtâwâxâzâyâ{â~âÑâàâäâÖâÉïQâáâÄâåâéâìâîâñâââèâêâòâ§âöâõâûâùâüâ†â¢â£ç˜â•âúâ°â™âÆâ≠âØâ∞ãÒâ≥â¥âµâπâ∂â∑â∏â∫âªâŒâ¡â¬âºâΩâ‘â¿âøâ âÕâ’â»âÀâƒâ∆â◊âÿâŸâ›âQâﬂâ≈â…â–åCâ«âÃâ”â“â€â·âÂä¢â‰âÊâËâÍâÎâÏâÓâÒäDâÔâıâ˙â¸âˆâ˚â˜äCäEäFäBäGäJäKâÚû≤âí◊âÛâ˘Ê~äLäOäNäQäRäUäXäSäWäYäTä[ä_ä`äeäpägäväiäjäkäsäoäräuätämälädänäwäxäyäzä{ä|äÉäáäàäÖäâäÑäãäääåäçäéäîäòäôä†ä±äßä√äæä äÆäÃäØä•ä™ä≈ä◊ä£ä®ä≥ä—ä¶ä´ä¨ä∑ä∏äªäºä‘ä’ä©ä∞ä≤ä¥äøäµä«ä÷äΩäƒä…ä∂ä“äŸä¬ä»äœäÿäÕä”ä€ä‹ä›ä‚äﬂä·äÊäÁäËäÈäÍäÎä˜ãCäÚäÛäıãDäÔãFãGãIãOä˘ãLãNãQãSãAäÓäÒãKãTäÏäÙäˆä˙ä˚ãMä¸ö ä¯äÌãEãPã@ãRãZãXãUã\ã`ã^ãVãYã[ã]ãcãeãgãiãlãpãqãrãtãsã„ãvãyã|ãuãåãxãzãÄâPãÅãÜãÉã}ãâãäã{ã~ãÖããökãáãçãéãêãèãëãíãìãïãñãóãõå‰ãôã•ã§ã©ã∂ãûãùãüã¶ãµã¨ã≤ã∑ã∞ã±ãπã∫ã≠ã≥ãΩã´ã¥ã∏ãæã£ãøã¡ã¬ã≈ã∆ã√ã»ã«ã…ã ã–ã“ãœãﬂã‡ã€ãŒã’ãÿãÕã÷ãŸã—ãﬁã›ã·ã‚ãÊãÂãÍãÏãÔúúããÛãÙãˆã˜ã¯ã¸å@åAåFåJåNåPåMåOåRåSåQåZåYå`ånåaåsåWå^å_åvåbå[åfåkåoåuåhåiåyåXågåpåwåcú€åmåeåxå{å|å}å~åÑåÄåÇåÉåÖåáåäåååàåãåÜåâåéå¢åèå©åîå®åöå§åßåêåìåïåùå¨åíåØåóåòåüåôå£å¶å≠å†åõå´å™åÆñöå∞å±åúå≥å∂å∫åæå∑å¿å¥åªåΩå∏åπåµå»åÀå√åƒå≈å“å’å«å åÃåÕå¬å…åŒåŸå÷å€Á¸å⁄å‹å›åﬂå‡å„å‚åÂåÈåÍåÎåÏå˚çHåˆå˘çEå˜çIçLçbååıå¸ç@çDç]çlçsçBçFçRçUçXå¯çKçSçmåÚå˙çPç^çcçgçrçxçÅåÛçZçkçqçvç~çÇçNçTç[â©çAçQç`çdçiçÄçaçzç\çjçyçeãªçtç|çuçwåÓçÜçáçâçÑò¸çãçéçêíJçèçëçïçíçìçñçúãÓçûç†ç°ç¢ç©ç¶ç™ç•ç¨ç≠çÆç∞ç§çßç∂ç≤çπç∏çªç¥ç∑çºçΩç¿ç¡çÀçƒç–ç»ç—ç”ç…çÕç çÃçœç’ç÷ç◊çÿç≈çŸç¬ç√ç«çŒç⁄ç€çÈç›çﬁç‹ç‡çﬂçËçÏçÌççÚçıçÙê|çÔçˆçÁç˚éDç¸ôãéAéEé@éBéCéGéMéOéRéQéVé\éSéYéPéUéZé_é^écéaébéméqéxé~éÅédéjéiélésñÓé|éÄéÖéäéféuéÑégéhénéoé}éÉéàépévéwé{étúìéÜéâéãéáéåéïékéééçéëéîéèéìùïéíéêé¶éöéõéüé®é©éóéôéñéòé°éùéûé†éúé´é•âaé£é≠éÆéØé≤éµé∂é∏é∫éæé∑éºéπéΩéøé¿é≈é é–é‘é…é“éÀéÃéÕéŒéœé’é”é◊é÷é⁄éÿéﬁéﬂé›é·é„é‚éËéÂéÁéÈéÊéÎéÒéÍéÏééÓéÌéÔéıéÛéÙéˆé˘éÚé˜é˚é˙èBèMèGé¸è@èEèHèLèCë≥èI„µèKèTèAèOèWèDèVèXèRèPè\è`è[èZè_èdè]èaèeèbècèfèjèhèièlèkèmènèoèqèpèrètèuè{èÑèÇèyè}èÉèzèáèÄèÅèÖèàèâèäèëèéèãèêèèèîèóî@èïèòèñèôèúè¨è°è≠è¢è†è∞è¥è—èÆèµè≥è∏èºè¿è∫è™è´è¡è«èÀèÃèŒè•è§è¬èÕè–è◊èüè∂èªèƒè≈è…èœèŸèÿè€èùèßè∆è⁄è≤è·ì≤è’è‹èûè è‡è„è‰èÁèèÛèÊèÈèÚèËèÌèÓèÍèÙèˆìÍèÎèÏè˘è˜è¯êFê@êHêAêBè¸êGè˙êDêEêJêKêSê\êLêbêcêgêhêNêMí√ê_êOêPêUêZê^êjê[êaêiêXêfêQêTêVêRêkêdêeêlênêmêsêvêrêwêqêtê{ê}êÖêÅêÇêÜêÉêàêäêÑêåêãêáï‰êèêëêïêíêîêòêôêûê°ê£ê•à‰ê¢ê≥ê∂ê¨êºê∫êßê©ê™ê´ê¬êƒê≠êØêµè»ê¶ê¿ê¥ê∑ñπê∞ê®êπêΩê∏êªêæê√êøêÆê¡ê≈ó[êÀêŒê‘êÃêÕê»ê“ê«ê…ê ê”ê’êœê—ê–êÿê‹êŸêﬁê⁄ê›ê·ê€êﬂê‡ê„ê‚êÁêÏêÂêËêÊêÈêÍêÚêÛêÙêıêÓêê˘ëDêÌê˜ëAëBëFëH‚≥ëKêˆê¸ëJëIëEë@ëNëSëOëPëRëTëQëVëUë_ëjëcëdëfë[ëeëgëaëiëYëkëbëoësëÅëàëñëtëäëëëêëóëqë{ë}åKëÉë|ëÇë]ëuëãënërëâëíëïëmëzëwëçëòëÖÁHëÄëáëöëõëîë¢ëúëùëûë†ë°ëüë¶ë©ë´ë£ë•ëßë®ë¨ë§ë™ë≠ë∞ëÆëØë±ë≤ó¶ë∂ë∫ë∑ë∏ëπëªëºëΩëøë≈ë√ë¡ë¬ëƒë ëæëŒëÃëœë“ë”ëŸëﬁë—ë◊ëÕë‹ëﬂë÷ë›ë‡ëÿë‘ë’ëÂë„ë‰ëÊëËëÍëÓëëÚëÏëÒëıëÛë¯ë˜íAíBíEíDíIíNíOíUíSíPíYí_íTíWíZíQí[í]íaíbícíjíiífíeígíkídínírímílípívíxísítíu„kí|í{íÄí~ízíÅíÇíÉíÖíÑíÜíáíéâ´íàíâíäíçíãíåíèíëíííìíòíôíöí¢í°íõí¨í∑íßí†í£í§í≠íﬁí∏íπí©ì\í¥í∞íµí•ö}í™êüí≤íÆí¶íºí∫íªíæíøíΩí¬í¿í¡í«í≈íƒí í…íÀí–íÿí‹íﬂí·íÊíÏíÌíËíÍíÔì@í‡íÂíÈí˘íÎí¸í‚í„íÁíÒíˆí¯í˜í˙ìDìIìJìEìHìKìGìMìRìNìSìOìPìVìTìXì_ìWìYì]ìUìcì`ìaìdìlìfìiìkìrìsìnìhìqìyìzìwìxì{ìÅì~ìîìñìäì§ìåìûì¶ì|ìÄìÇìáìçì¢ìßì}ìâìêì©ìÉìãìèìíìóìoìöìôìõìùàÓì•ìúì™ì£ì°ì¨ì´ìØì¥ì∑ìÆì∞ì∂ìπì≠ì∫ì±ìµìªìΩì¡ìæì¬ìøìƒì≈ì∆ì«ì»ì ìÀìÕì‘ìÿì⁄Ê√ì›ì‹ò•ìﬂìﬁì‡óúì‰ìÁìÏìÓìÔìÒìÚìÛìıì˜ì¯ì˙ì¸ì˚îAîCîDîEîFîJîMîNîOîPîSîRîYî[î\î]î_îZîcîgîhîjîeînîkîlîqîtîwîxîoîzîrîsîpîyîÑî{î~î|îÜî}îÉîÖîíîåîèîëîóîçîïîéîñîûîôîõîöî†î¢î®îßî™î´î≠îØî∞î≤î±î¥îΩîºî√î∆îøîƒî∫îªç‚ç„î¬î≈î«î»î îÃî¡î—î¿îœî–îÕî…îÀî”î‘îÿî’î‰îÁî‹î€î·îﬁî‚îÏîÒî⁄îÚîÊîÈîÌîﬂî‡îÔîËîÎîîˆî˚î¸îıî˜ï@ïGïIïCïKîÂïMïPïSïXï\ïUï[ï]ïYïWïcïbïaï`îLïiïlïnïoïpïqïrïsïvïÉïtïzï}ï{ï|ïåïçÊ]ïâïãïÇïwïÑïxïÅïÖï~ïÜïäïàïéïêïîïëïïïóïöïûïõïùïúïüï†ï°ï¢ï•ï¶ïßï®ï≤ï¥ïµï¨ï≠ïÆï±ï™ï∂ï∑ï∏ïΩï∫ïπï¿ïøï√ï¬ïªïºïæï¡ñ›ïƒï«‡¯ï»ï ïÃï–ï”ï‘ïœïŒï’ï“ïŸï÷ï◊ï‡ï€öMïﬂï‚ï‹ïÍïÂïÊïÁïÈïÎï˚ïÔñFñMïÚïÛï¯ï˙ñ@ñAñEïÓïÌïÙñCïˆñKïÒñIñLñOñJñDñSñRñZñVñWñYñhñ[ñbñ^ñ`ñUñañ]ñTñXñ_ñfñeñ\ñcñdñjñkñÿñpñqñrñlñnñoñvñuñxñ{ñzñ|ñ}ñ~ñÉñÄñÅñÇñàñÖñáñÜñÑñãñåñçñîñññïñúñûñùñüñ¢ñ°ñ£ñ¶ñßñ®ñ¨ñ≠ñØñ∞ñµñ±ñ≥ñ≤ñ∂ñ∫ñºñΩñæñ¿ñªñøñ¡ñ¬ñ≈ñ∆ñ ñ»ñÀñŒñÕñ—ñœñ”ñ’ñ“ñ‘ñ⁄ñŸñÂñ‰ñ‚ñËñÈñÏñÌñÔññÒñÛñÚñÙà≈óRñ˚ögñ˘ó@óAñ¸óBóFóLóEóHóIóXóNóPóTóVóYóUóJóZóDó^ó\ó]ó_óaócópóródómóvóeófógóhótózónçòól·áóxóqó{óiówójó}óÄóÅó~óÇóÉùfóáóÖóàóãóäóçóéóèÁÖóêóëóóóîóïóìóôóòó¢óùóüó†óöóûó£ó§óßó•ú…ó™ñˆó¨óØó≥ó±ó≤ó∞óµó∑ó∏ó∂óπóºó«óøó¡ó¬óÀó óªóÃóæó√óƒó∆óÕóŒó—ó–óœó÷ó◊ó’ó⁄ó‹ó›ó€óﬁóﬂóÁó‚ó„ñﬂó·óÈóÎóÏóÍóÓóÌóÔóóÒóÚóÛóÙóˆòAóıó˚òBòCòFòGòHòIòVòJòMòYòNòQòLòOòR‚ƒòZò^ò[ò_òaòbòdòeòfògòpòr", ch) != NULL;
} // is_common_use_kanji

// ëSäpASCIIÇ©ÅH
BOOL is_fullwidth_ascii(WCHAR ch) {
    return (0xFF00 <= ch && ch <= 0xFFEF);
}

// Ç∑Ç◊ÇƒÇÃï∂éöÇ™êîéöìIÇ©ÅH
BOOL are_all_chars_numeric(const std::wstring& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        if (L'0' <= str[i] && str[i] <= L'9') {
            ;
        } else if (L'ÇO' <= str[i] && str[i] <= L'ÇX') {
            ;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

static const wchar_t s_szKanjiDigits[] = L"ÅZàÍìÒéOélå‹òZéµî™ã„";

// äøêîéöïœä∑ÅiÇªÇÃÇPÅjÅB
std::wstring convert_to_kansuuji_1(wchar_t ch, size_t digit_level) {
    static const wchar_t s_szKanjiDigitLevels[] = L" è\ïSêÁ";
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

// äøêîéöïœä∑ÅiÇªÇÃÇQÅjÅB
std::wstring convert_to_kansuuji_4(const std::wstring& halfwidth) {
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

// äøêîéöïœä∑ÅiÇªÇÃÇRÅjÅB
std::wstring convert_to_kansuuji(const std::wstring& str) {
    std::wstring halfwidth = lcmap(str, LCMAP_HALFWIDTH);
    ASSERT(are_all_chars_numeric(halfwidth));
    if (halfwidth.size() >= 24) return halfwidth;
    static const wchar_t s_szKanjiGroupLevels[] = L" ñúâ≠íõãûö¥";
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
    if (ret.empty()) ret = L"óÎ";
    str_replace_all(ret, L"àÍè\", L"è\");
    str_replace_all(ret, L"àÍïS", L"ïS");
    str_replace_all(ret, L"àÍêÁ", L"êÁ");
    return ret;
}

// äøêîéöïœä∑ÅiÇªÇÃÇSÅjÅB
std::wstring convert_to_kansuuji_brief(const std::wstring& str) {
    std::wstring halfwidth = lcmap(str, LCMAP_HALFWIDTH);
    ASSERT(are_all_chars_numeric(halfwidth));
    std::wstring ret;
    for (size_t i = 0; i < halfwidth.size(); ++i) {
        ret += s_szKanjiDigits[halfwidth[i] - L'0'];
    }
    return ret;
}

// äøêîéöïœä∑ÅiÇªÇÃÇTÅjÅB
std::wstring convert_to_kansuuji_formal(const std::wstring& str) {
    std::wstring ret = convert_to_kansuuji(str);
    str_replace_all(ret, L"àÍ", L"àÎ");
    str_replace_all(ret, L"ìÒ", L"ìÛ");
    str_replace_all(ret, L"éO", L"éQ");
    str_replace_all(ret, L"å‹", L"åﬁ");
    str_replace_all(ret, L"è\", L"èE");
    str_replace_all(ret, L"êÁ", L"ò¿");
    str_replace_all(ret, L"ñú", L"‰›");
    return ret;
}

// ë˜âπèàóùÅB
WCHAR dakuon_shori(WCHAR ch0, WCHAR ch1) {
    switch (MAKELONG(ch0, ch1)) {
    case MAKELONG(L'Ç©', L'ÅJ'): return L'Ç™';
    case MAKELONG(L'Ç´', L'ÅJ'): return L'Ç¨';
    case MAKELONG(L'Ç≠', L'ÅJ'): return L'ÇÆ';
    case MAKELONG(L'ÇØ', L'ÅJ'): return L'Ç∞';
    case MAKELONG(L'Ç±', L'ÅJ'): return L'Ç≤';
    case MAKELONG(L'Ç≥', L'ÅJ'): return L'Ç¥';
    case MAKELONG(L'Çµ', L'ÅJ'): return L'Ç∂';
    case MAKELONG(L'Ç∑', L'ÅJ'): return L'Ç∏';
    case MAKELONG(L'Çπ', L'ÅJ'): return L'Ç∫';
    case MAKELONG(L'Çª', L'ÅJ'): return L'Çº';
    case MAKELONG(L'ÇΩ', L'ÅJ'): return L'Çæ';
    case MAKELONG(L'Çø', L'ÅJ'): return L'Ç¿';
    case MAKELONG(L'Ç¬', L'ÅJ'): return L'Ç√';
    case MAKELONG(L'Çƒ', L'ÅJ'): return L'Ç≈';
    case MAKELONG(L'Ç∆', L'ÅJ'): return L'Ç«';
    case MAKELONG(L'ÇÕ', L'ÅJ'): return L'ÇŒ';
    case MAKELONG(L'Ç–', L'ÅJ'): return L'Ç—';
    case MAKELONG(L'Ç”', L'ÅJ'): return L'Ç‘';
    case MAKELONG(L'Ç÷', L'ÅJ'): return L'Ç◊';
    case MAKELONG(L'ÇŸ', L'ÅJ'): return L'Ç⁄';
    case MAKELONG(L'Ç§', L'ÅJ'): return L'\u3094';
    case MAKELONG(L'ÇÕ', L'ÅK'): return L'Çœ';
    case MAKELONG(L'Ç–', L'ÅK'): return L'Ç“';
    case MAKELONG(L'Ç”', L'ÅK'): return L'Ç’';
    case MAKELONG(L'Ç÷', L'ÅK'): return L'Çÿ';
    case MAKELONG(L'ÇŸ', L'ÅK'): return L'Ç€';
    case MAKELONG(L'ÉJ', L'ÅJ'): return L'ÉK';
    case MAKELONG(L'ÉL', L'ÅJ'): return L'ÉM';
    case MAKELONG(L'ÉN', L'ÅJ'): return L'ÉO';
    case MAKELONG(L'ÉP', L'ÅJ'): return L'ÉQ';
    case MAKELONG(L'ÉR', L'ÅJ'): return L'ÉS';
    case MAKELONG(L'ÉT', L'ÅJ'): return L'ÉU';
    case MAKELONG(L'ÉV', L'ÅJ'): return L'ÉW';
    case MAKELONG(L'ÉX', L'ÅJ'): return L'ÉY';
    case MAKELONG(L'ÉZ', L'ÅJ'): return L'É[';
    case MAKELONG(L'É\', L'ÅJ'): return L'É]';
    case MAKELONG(L'É^', L'ÅJ'): return L'É_';
    case MAKELONG(L'É`', L'ÅJ'): return L'Éa';
    case MAKELONG(L'Éc', L'ÅJ'): return L'Éd';
    case MAKELONG(L'Ée', L'ÅJ'): return L'Éf';
    case MAKELONG(L'Ég', L'ÅJ'): return L'Éh';
    case MAKELONG(L'Én', L'ÅJ'): return L'Éo';
    case MAKELONG(L'Éq', L'ÅJ'): return L'Ér';
    case MAKELONG(L'Ét', L'ÅJ'): return L'Éu';
    case MAKELONG(L'Éw', L'ÅJ'): return L'Éx';
    case MAKELONG(L'Éz', L'ÅJ'): return L'É{';
    case MAKELONG(L'ÉE', L'ÅJ'): return L'Éî';
    case MAKELONG(L'Én', L'ÅK'): return L'Ép';
    case MAKELONG(L'Éq', L'ÅK'): return L'És';
    case MAKELONG(L'Ét', L'ÅK'): return L'Év';
    case MAKELONG(L'Éw', L'ÅK'): return L'Éy';
    case MAKELONG(L'Éz', L'ÅK'): return L'É|';
    default:                     return 0;
    }
}

// ï∂éöéÌïœä∑ÅB
std::wstring lcmap(const std::wstring& str, DWORD dwFlags) {
    WCHAR szBuf[1024];
    const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
    ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                   str.c_str(), -1, szBuf, 1024);
    return szBuf;
}

// ëSäpâpêîÇ©ÇÁîºäpÇ÷ÇÃï∂éöóÒïœä∑ÅB
std::wstring fullwidth_ascii_to_halfwidth(const std::wstring& str) {
    std::wstring ret;
    const size_t count = str.size();
    wchar_t ch;
    for (size_t i = 0; i < count; ++i) {
        ch = str[i];
        if (L'ÇÅ' <= ch && ch <= L'Çö') {
            ch += L'a' - L'ÇÅ';
        } else if (L'Ç`' <= ch && ch <= L'Çy') {
            ch += L'A' - L'Ç`';
        }
        ret += ch;
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
