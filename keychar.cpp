// keychar.cpp
//////////////////////////////////////////////////////////////////////////////

#include "mzimeja.h"
#include "vksub.h"

#define UNBOOST_USE_STRING_ALGORITHM
#include "unboost.hpp"

//////////////////////////////////////////////////////////////////////////////

struct TABLEENTRY {
  LPCWSTR key;
  LPCWSTR value;
  LPCWSTR extra;
};

static TABLEENTRY halfkana_table[] = {
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç©ÇÁ
  {L"Å[", L"-"},
  {L"ÅE", L"•"},
  {L"ÅK", L"ﬂ"},
  {L"ÅJ", L"ﬁ"},
  {L"ÇÒ", L"›"},
  {L"Ç", L"¶"},
  {L"ÇÔ", L"¥"},
  {L"ÇÓ", L"≤"},
  {L"ÇÌ", L"‹"},
  {L"ÇÏ", L"‹"},
  {L"ÇÎ", L"€"},
  {L"ÇÍ", L"⁄"},
  {L"ÇÈ", L"Ÿ"},
  {L"ÇË", L"ÿ"},
  {L"ÇÁ", L"◊"},
  {L"ÇÊ", L"÷"},
  {L"ÇÂ", L"Æ"},
  {L"Ç‰", L"’"},
  {L"Ç„", L"≠"},
  {L"Ç‚", L"‘"},
  {L"Ç·", L"¨"},
  {L"Ç‡", L"”"},
  {L"Çﬂ", L"“"},
  {L"Çﬁ", L"—"},
  {L"Ç›", L"–"},
  {L"Ç‹", L"œ"},
  {L"Ç€", L"Œﬂ"},
  {L"Ç⁄", L"Œﬁ"},
  {L"ÇŸ", L"Œ"},
  {L"Çÿ", L"Õﬂ"},
  {L"Ç◊", L"Õﬁ"},
  {L"Ç÷", L"Õ"},
  {L"Ç’", L"Ãﬂ"},
  {L"Ç‘", L"Ãﬁ"},
  {L"Ç”", L"Ã"},
  {L"Ç“", L"Àﬂ"},
  {L"Ç—", L"Àﬁ"},
  {L"Ç–", L"À"},
  {L"Çœ", L" ﬂ"},
  {L"ÇŒ", L" ﬁ"},
  {L"ÇÕ", L" "},
  {L"ÇÃ", L"…"},
  {L"ÇÀ", L"»"},
  {L"Ç ", L"«"},
  {L"Ç…", L"∆"},
  {L"Ç»", L"≈"},
  {L"Ç«", L"ƒﬁ"},
  {L"Ç∆", L"ƒ"},
  {L"Ç≈", L"√ﬁ"},
  {L"Çƒ", L"√"},
  {L"Ç√", L"¬ﬁ"},
  {L"Ç¬", L"¬"},
  {L"Ç¡", L"Ø"},
  {L"Ç¿", L"¡ﬁ"},
  {L"Çø", L"¡"},
  {L"Çæ", L"¿ﬁ"},
  {L"ÇΩ", L"¿"},
  {L"Çº", L"øﬁ"},
  {L"Çª", L"ø"},
  {L"Ç∫", L"æﬁ"},
  {L"Çπ", L"æ"},
  {L"Ç∏", L"Ωﬁ"},
  {L"Ç∑", L"Ω"},
  {L"Ç∂", L"ºﬁ"},
  {L"Çµ", L"º"},
  {L"Ç¥", L"ªﬁ"},
  {L"Ç≥", L"ª"},
  {L"Ç≤", L"∫ﬁ"},
  {L"Ç±", L"∫"},
  {L"Ç∞", L"πﬁ"},
  {L"ÇØ", L"π"},
  {L"ÇÆ", L"∏ﬁ"},
  {L"Ç≠", L"∏"},
  {L"Ç¨", L"∑ﬁ"},
  {L"Ç´", L"∑"},
  {L"Ç™", L"∂ﬁ"},
  {L"Ç©", L"∂"},
  {L"Ç®", L"µ"},
  {L"Çß", L"´"},
  {L"Ç¶", L"¥"},
  {L"Ç•", L"™"},
  {L"Ç§ÅJ", L"≥ﬁ"},
  {L"Ç§", L"≥"},
  {L"Ç£", L"©"},
  {L"Ç¢", L"≤"},
  {L"Ç°", L"®"},
  {L"Ç†", L"±"},
  {L"Çü", L"ß"},
  {L"Åv", L"£"},
  {L"Åu", L"¢"},
  {L"ÅB", L"°"},
  {L"ÅA", L"§"},
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç‹Ç≈
};

static TABLEENTRY kana_table[] = {
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç©ÇÁ
  {L"ﬂ", L"ÅK"},
  {L"ﬁ", L"ÅJ"},
  {L"›", L"ÇÒ"},
  {L"‹", L"ÇÌ"},
  {L"€", L"ÇÎ"},
  {L"⁄", L"ÇÍ"},
  {L"Ÿ", L"ÇÈ"},
  {L"ÿ", L"ÇË"},
  {L"◊", L"ÇÁ"},
  {L"÷", L"ÇÊ"},
  {L"’", L"Ç‰"},
  {L"‘", L"Ç‚"},
  {L"”", L"Ç‡"},
  {L"“", L"Çﬂ"},
  {L"—", L"Çﬁ"},
  {L"–", L"Ç›"},
  {L"œ", L"Ç‹"},
  {L"Œ", L"ÇŸ"},
  {L"Õ", L"Ç÷"},
  {L"Ã", L"Ç”"},
  {L"À", L"Ç–"},
  {L" ", L"ÇÕ"},
  {L"…", L"ÇÃ"},
  {L"»", L"ÇÀ"},
  {L"«", L"Ç "},
  {L"∆", L"Ç…"},
  {L"≈", L"Ç»"},
  {L"ƒ", L"Ç∆"},
  {L"√", L"Çƒ"},
  {L"¬", L"Ç¬"},
  {L"¡", L"Çø"},
  {L"¿", L"ÇΩ"},
  {L"ø", L"Çª"},
  {L"æ", L"Çπ"},
  {L"Ω", L"Ç∑"},
  {L"º", L"Çµ"},
  {L"ª", L"Ç≥"},
  {L"∫", L"Ç±"},
  {L"π", L"ÇØ"},
  {L"∏", L"Ç≠"},
  {L"∑", L"Ç´"},
  {L"∂", L"Ç©"},
  {L"µ", L"Ç®"},
  {L"¥", L"Ç¶"},
  {L"≥", L"Ç§"},
  {L"≤", L"Ç¢"},
  {L"±", L"Ç†"},
  {L"∞", L"Å["},
  {L"Ø", L"Ç¡"},
  {L"Æ", L"ÇÂ"},
  {L"≠", L"Ç„"},
  {L"¨", L"Ç·"},
  {L"´", L"Çß"},
  {L"™", L"Ç•"},
  {L"©", L"Ç£"},
  {L"®", L"Ç°"},
  {L"ß", L"Çü"},
  {L"¶", L"Ç"},
  {L"•", L"ÅE"},
  {L"§", L"ÅA"},
  {L"£", L"Åv"},
  {L"¢", L"Åu"},
  {L"°", L"ÅB"},
  {L"ÇŸﬂ", L"Ç€"},
  {L"ÇŸﬁ", L"Ç⁄"},
  {L"Ç÷ﬂ", L"Çÿ"},
  {L"Ç÷ﬁ", L"Ç◊"},
  {L"Ç”ﬂ", L"Ç’"},
  {L"Ç”ﬁ", L"Ç‘"},
  {L"Ç–ﬂ", L"Ç“"},
  {L"Ç–ﬁ", L"Ç—"},
  {L"ÇÕﬂ", L"Çœ"},
  {L"ÇÕﬁ", L"ÇŒ"},
  {L"Ç∆ﬁ", L"Ç«"},
  {L"Çƒﬁ", L"Ç≈"},
  {L"Ç¬ﬁ", L"Ç√"},
  {L"Çøﬁ", L"Ç¿"},
  {L"ÇΩﬁ", L"Çæ"},
  {L"Çªﬁ", L"Çº"},
  {L"Çπﬁ", L"Ç∫"},
  {L"Ç∑ﬁ", L"Ç∏"},
  {L"Çµﬁ", L"Ç∂"},
  {L"Ç≥ﬁ", L"Ç¥"},
  {L"Ç±ﬁ", L"Ç≤"},
  {L"ÇØﬁ", L"Ç∞"},
  {L"Ç≠ﬁ", L"ÇÆ"},
  {L"Ç´ﬁ", L"Ç¨"},
  {L"Ç©ﬁ", L"Ç™"},
  {L"Ç§ﬁ", L"Éî"},
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç‹Ç≈
};

static TABLEENTRY kigou_table[] = {
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç©ÇÁ
  {L"~", L"Å`"},
  {L"}", L"Åp"},
  {L"|", L"Åb"},
  {L"{", L"Åo"},
  {L"`", L"ÅM"},
  {L"_", L"ÅQ"},
  {L"^", L"ÅO"},
  {L"]", L"Åv"},
  {L"\\", L"Åè"},
  {L"[", L"Åu"},
  {L"@", L"Åó"},
  {L"?", L"ÅH"},
  {L">", L"ÅÑ"},
  {L"=", L"ÅÅ"},
  {L"<", L"ÅÉ"},
  {L";", L"ÅG"},
  {L":", L"ÅF"},
  {L"/", L"ÅE"},
  {L".", L"ÅB"},
  {L"-", L"Å["},
  {L",", L"ÅA"},
  {L"+", L"Å{"},
  {L"*", L"Åñ"},
  {L")", L"Åj"},
  {L"(", L"Åi"},
  {L"'", L"Åf"},
  {L"&", L"Åï"},
  {L"%", L"Åì"},
  {L"$", L"Åê"},
  {L"#", L"Åî"},
  {L"\"", L"Åç"},
  {L"!", L"ÅI"},
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç‹Ç≈
};

static TABLEENTRY reverse_table[] = {
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç©ÇÁ
  {L"Å`", L"~"},
  {L"Å[", L"-"},
  {L"ÇÒ", L"n'"},
  {L"Ç", L"wo"},
  {L"ÇÔ", L"we"},
  {L"ÇÓ", L"wi"},
  {L"ÇÌ", L"wa"},
  {L"ÇÏ", L"xwa"},
  {L"ÇÎ", L"ro"},
  {L"ÇÍ", L"re"},
  {L"ÇÈ", L"ru"},
  {L"ÇËÇÂ", L"ryo"},
  {L"ÇËÇ„", L"ryu"},
  {L"ÇËÇ·", L"rya"},
  {L"ÇËÇ•", L"rye"},
  {L"ÇËÇ°", L"ryi"},
  {L"ÇË", L"ri"},
  {L"ÇÁ", L"ra"},
  {L"ÇÊ", L"yo"},
  {L"ÇÂ", L"xyo"},
  {L"Ç‰", L"yu"},
  {L"Ç„", L"xyu"},
  {L"Ç‚", L"ya"},
  {L"Ç·", L"xya"},
  {L"Ç‡", L"mo"},
  {L"Çﬂ", L"me"},
  {L"Çﬁ", L"mu"},
  {L"Ç›ÇÂ", L"myo"},
  {L"Ç›Ç„", L"myu"},
  {L"Ç›Ç·", L"mya"},
  {L"Ç›Ç•", L"mye"},
  {L"Ç›Ç°", L"myi"},
  {L"Ç›", L"mi"},
  {L"Ç‹", L"ma"},
  {L"Ç€", L"po"},
  {L"Ç⁄", L"bo"},
  {L"ÇŸ", L"ho"},
  {L"Çÿ", L"pe"},
  {L"Ç◊", L"be"},
  {L"Ç÷", L"he"},
  {L"Ç’", L"pu"},
  {L"Ç‘", L"bu"},
  {L"Ç”", L"hu"},
  {L"Ç“ÇÂ", L"pyo"},
  {L"Ç“Ç„", L"pyu"},
  {L"Ç“Ç·", L"pya"},
  {L"Ç“Ç•", L"pye"},
  {L"Ç“Ç°", L"pyi"},
  {L"Ç“", L"pi"},
  {L"Ç—ÇÂ", L"byo"},
  {L"Ç—Ç„", L"byu"},
  {L"Ç—Ç·", L"bya"},
  {L"Ç—Ç•", L"bye"},
  {L"Ç—Ç°", L"byi"},
  {L"Ç—", L"bi"},
  {L"Ç–ÇÂ", L"hyo"},
  {L"Ç–Ç„", L"hyu"},
  {L"Ç–Ç·", L"hya"},
  {L"Ç–Ç•", L"hye"},
  {L"Ç–Ç°", L"hyi"},
  {L"Ç–", L"hi"},
  {L"Çœ", L"pa"},
  {L"ÇŒ", L"ba"},
  {L"ÇÕ", L"ha"},
  {L"ÇÃ", L"no"},
  {L"ÇÀ", L"ne"},
  {L"Ç ", L"nu"},
  {L"Ç…ÇÂ", L"nyo"},
  {L"Ç…Ç„", L"nyu"},
  {L"Ç…Ç·", L"nya"},
  {L"Ç…Ç•", L"nye"},
  {L"Ç…Ç°", L"nyi"},
  {L"Ç…", L"ni"},
  {L"Ç»", L"na"},
  {L"Ç«", L"do"},
  {L"Ç∆", L"to"},
  {L"Ç≈Ç°", L"dhi"},
  {L"Ç≈", L"de"},
  {L"Çƒ", L"te"},
  {L"Ç√", L"du"},
  {L"Ç¬", L"tu"},
  {L"Ç¡Ç", L"wwo"},
  {L"Ç¡ÇÔ", L"wwe"},
  {L"Ç¡ÇÓ", L"wwi"},
  {L"Ç¡ÇÌ", L"wwa"},
  {L"Ç¡ÇÏ", L"xxwa"},
  {L"Ç¡ÇÎ", L"rro"},
  {L"Ç¡ÇÍ", L"rre"},
  {L"Ç¡ÇÈ", L"rru"},
  {L"Ç¡ÇË", L"r", L"ÇË"},
  {L"Ç¡ÇÁ", L"rra"},
  {L"Ç¡ÇÊ", L"yyo"},
  {L"Ç¡ÇÂ", L"xxyo"},
  {L"Ç¡Ç‰", L"yyu"},
  {L"Ç¡Ç„", L"xxyu"},
  {L"Ç¡Ç‚", L"yya"},
  {L"Ç¡Ç·", L"xxya"},
  {L"Ç¡Ç‡", L"mmo"},
  {L"Ç¡Çﬂ", L"mme"},
  {L"Ç¡Çﬁ", L"mmu"},
  {L"Ç¡Ç›", L"m", L"Ç›"},
  {L"Ç¡Ç‹", L"mma"},
  {L"Ç¡Ç€", L"ppo"},
  {L"Ç¡Ç⁄", L"bbo"},
  {L"Ç¡ÇŸ", L"hho"},
  {L"Ç¡Çÿ", L"ppe"},
  {L"Ç¡Ç◊", L"bbe"},
  {L"Ç¡Ç÷", L"hhe"},
  {L"Ç¡Ç’", L"ppu"},
  {L"Ç¡Ç‘", L"bbu"},
  {L"Ç¡Ç”", L"hhu"},
  {L"Ç¡Ç“", L"p", L"Ç“"},
  {L"Ç¡Ç—", L"b", L"Ç—"},
  {L"Ç¡Ç–", L"h", L"Ç–"},
  {L"Ç¡Çœ", L"ppa"},
  {L"Ç¡ÇŒ", L"bba"},
  {L"Ç¡ÇÕ", L"hha"},
  {L"Ç¡Ç«", L"ddo"},
  {L"Ç¡Ç∆", L"tto"},
  {L"Ç¡Ç≈", L"dde"},
  {L"Ç¡Çƒ", L"tte"},
  {L"Ç¡Ç√", L"ddu"},
  {L"Ç¡Ç¬", L"ttu"},
  {L"Ç¡Ç¿", L"d", L"Ç¿"},
  {L"Ç¡Çø", L"t", L"Çø"},
  {L"Ç¡Çæ", L"dda"},
  {L"Ç¡ÇΩ", L"tta"},
  {L"Ç¡Çº", L"zzo"},
  {L"Ç¡Çª", L"sso"},
  {L"Ç¡Ç∫", L"zze"},
  {L"Ç¡Çπ", L"sse"},
  {L"Ç¡Ç∏", L"zzu"},
  {L"Ç¡Ç∑", L"ssu"},
  {L"Ç¡Ç∂", L"z", L"Ç∂"},
  {L"Ç¡Çµ", L"s", L"Çµ"},
  {L"Ç¡Ç¥", L"zza"},
  {L"Ç¡Ç≥", L"ssa"},
  {L"Ç¡Ç≤", L"ggo"},
  {L"Ç¡Ç±", L"kko"},
  {L"Ç¡Ç∞", L"gge"},
  {L"Ç¡ÇØ", L"kke"},
  {L"Ç¡ÇÆ", L"ggu"},
  {L"Ç¡Ç≠", L"kku"},
  {L"Ç¡Ç¨", L"g", L"Ç¨"},
  {L"Ç¡Ç´", L"k", L"Ç´"},
  {L"Ç¡Ç™", L"gga"},
  {L"Ç¡Ç©", L"kka"},
  {L"Ç¡Çß", L"xxo"},
  {L"Ç¡Ç•", L"xxe"},
  {L"Ç¡Ç§ÅJ", L"v", L"Ç§ÅJ"},
  {L"Ç¡Ç£", L"xxu"},
  {L"Ç¡Ç°", L"xxi"},
  {L"Ç¡Çü", L"xxa"},
  {L"Ç¡", L"xtu"},
  {L"Ç¿ÇÂ", L"dyo"},
  {L"Ç¿Ç„", L"dyu"},
  {L"Ç¿Ç·", L"dya"},
  {L"Ç¿Ç•", L"dye"},
  {L"Ç¿Ç°", L"dyi"},
  {L"Ç¿", L"di"},
  {L"ÇøÇÂ", L"tyo"},
  {L"ÇøÇ„", L"tyu"},
  {L"ÇøÇ·", L"tya"},
  {L"ÇøÇ•", L"tye"},
  {L"ÇøÇ°", L"tyi"},
  {L"Çø", L"ti"},
  {L"Çæ", L"da"},
  {L"ÇΩ", L"ta"},
  {L"Çº", L"zo"},
  {L"Çª", L"so"},
  {L"Ç∫", L"ze"},
  {L"Çπ", L"se"},
  {L"Ç∏", L"zu"},
  {L"Ç∑", L"su"},
  {L"Ç∂ÇÂ", L"zyo"},
  {L"Ç∂Ç„", L"zyu"},
  {L"Ç∂Ç·", L"zya"},
  {L"Ç∂Ç•", L"zye"},
  {L"Ç∂Ç°", L"zyi"},
  {L"Ç∂", L"zi"},
  {L"ÇµÇÂ", L"syo"},
  {L"ÇµÇ„", L"syu"},
  {L"ÇµÇ·", L"sya"},
  {L"ÇµÇ•", L"sye"},
  {L"ÇµÇ°", L"syi"},
  {L"Çµ", L"si"},
  {L"Ç¥", L"za"},
  {L"Ç≥", L"sa"},
  {L"Ç≤", L"go"},
  {L"Ç±", L"ko"},
  {L"Ç∞", L"ge"},
  {L"ÇØ", L"ke"},
  {L"ÇÆ", L"gu"},
  {L"Ç≠", L"ku"},
  {L"Ç¨ÇÂ", L"gyo"},
  {L"Ç¨Ç„", L"gyu"},
  {L"Ç¨Ç·", L"gya"},
  {L"Ç¨Ç•", L"gye"},
  {L"Ç¨Ç°", L"gyi"},
  {L"Ç¨", L"gi"},
  {L"Ç´ÇÂ", L"kyo"},
  {L"Ç´Ç„", L"kyu"},
  {L"Ç´Ç·", L"kya"},
  {L"Ç´Ç•", L"kye"},
  {L"Ç´Ç°", L"kyi"},
  {L"Ç´", L"ki"},
  {L"Ç™", L"ga"},
  {L"Ç©", L"ka"},
  {L"Ç®", L"o"},
  {L"Çß", L"xo"},
  {L"Ç¶", L"e"},
  {L"Ç•", L"xe"},
  {L"Ç§ÅJÇß", L"vo"},
  {L"Ç§ÅJÇ•", L"ve"},
  {L"Ç§ÅJÇ°", L"vi"},
  {L"Ç§ÅJÇü", L"va"},
  {L"Ç§ÅJ", L"vu"},
  {L"Ç§", L"u"},
  {L"Ç£", L"xu"},
  {L"Ç¢", L"i"},
  {L"Ç°", L"xi"},
  {L"Ç†", L"a"},
  {L"Çü", L"xa"},
  {L"Åv", L"]"},
  {L"Åu", L"["},
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç‹Ç≈
};

static TABLEENTRY romaji_table[] = {
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç©ÇÁ
  {L"zyu", L"Ç∂Ç„"},
  {L"zyo", L"Ç∂ÇÂ"},
  {L"zyi", L"Ç∂Ç°"},
  {L"zye", L"Ç∂Ç•"},
  {L"zya", L"Ç∂Ç·"},
  {L"zu", L"Ç∏"},
  {L"zo", L"Çº"},
  {L"zi", L"Ç∂"},
  {L"ze", L"Ç∫"},
  {L"za", L"Ç¥"},
  {L"yu", L"Ç‰"},
  {L"yo", L"ÇÊ"},
  {L"yi", L"Ç¢"},
  {L"ye", L"Ç¢Ç•"},
  {L"ya", L"Ç‚"},
  {L"xyu", L"Ç„"},
  {L"xyo", L"ÇÂ"},
  {L"xyi", L"Ç°"},
  {L"xye", L"Ç•"},
  {L"xya", L"Ç·"},
  {L"xwa", L"ÇÏ"},
  {L"xu", L"Ç£"},
  {L"xtu", L"Ç¡"},
  {L"xo", L"Çß"},
  {L"xn", L"ÇÒ"},
  {L"xke", L"Éñ"},
  {L"xka", L"Éï"},
  {L"xi", L"Ç°"},
  {L"xe", L"Ç•"},
  {L"xa", L"Çü"},
  {L"wu", L"Ç§"},
  {L"wo", L"Ç"},
  {L"wi", L"Ç§Ç°"},
  {L"whu", L"Ç§"},
  {L"who", L"Ç§Çß"},
  {L"whi", L"Ç§Ç°"},
  {L"whe", L"Ç§Ç•"},
  {L"wha", L"Ç§Çü"},
  {L"we", L"Ç§Ç•"},
  {L"wa", L"ÇÌ"},
  {L"vyu", L"ÉîÇ„"},
  {L"vyo", L"ÉîÇÂ"},
  {L"vyi", L"ÉîÇ°"},
  {L"vye", L"ÉîÇ•"},
  {L"vya", L"ÉîÇ·"},
  {L"vu", L"Éî"},
  {L"vo", L"ÉîÇß"},
  {L"vi", L"ÉîÇ°"},
  {L"ve", L"ÉîÇ•"},
  {L"va", L"ÉîÇü"},
  {L"u", L"Ç§"},
  {L"tyu", L"ÇøÇ„"},
  {L"tyo", L"ÇøÇÂ"},
  {L"tyi", L"ÇøÇ°"},
  {L"tye", L"ÇøÇ•"},
  {L"tya", L"ÇøÇ·"},
  {L"twu", L"Ç∆Ç£"},
  {L"two", L"Ç∆Çß"},
  {L"twi", L"Ç∆Ç°"},
  {L"twe", L"Ç∆Ç•"},
  {L"twa", L"Ç∆Çü"},
  {L"tu", L"Ç¬"},
  {L"tsu", L"Ç¬"},
  {L"tso", L"Ç¬Çß"},
  {L"tsi", L"Ç¬Ç°"},
  {L"tse", L"Ç¬Ç•"},
  {L"tsa", L"Ç¬Çü"},
  {L"to", L"Ç∆"},
  {L"ti", L"Çø"},
  {L"thu", L"ÇƒÇ„"},
  {L"tho", L"ÇƒÇÂ"},
  {L"thi", L"ÇƒÇ°"},
  {L"the", L"ÇƒÇ•"},
  {L"tha", L"ÇƒÇ·"},
  {L"te", L"Çƒ"},
  {L"ta", L"ÇΩ"},
  {L"syu", L"ÇµÇ„"},
  {L"syo", L"ÇµÇÂ"},
  {L"syi", L"ÇµÇ°"},
  {L"sye", L"ÇµÇ•"},
  {L"sya", L"ÇµÇ·"},
  {L"swu", L"Ç∑Ç£"},
  {L"swo", L"Ç∑Çß"},
  {L"swi", L"Ç∑Ç°"},
  {L"swe", L"Ç∑Ç•"},
  {L"swa", L"Ç∑Çü"},
  {L"su", L"Ç∑"},
  {L"so", L"Çª"},
  {L"si", L"Çµ"},
  {L"shu", L"ÇµÇ„"},
  {L"sho", L"ÇµÇÂ"},
  {L"shi", L"Çµ"},
  {L"she", L"ÇµÇ•"},
  {L"sha", L"ÇµÇ·"},
  {L"se", L"Çπ"},
  {L"sa", L"Ç≥"},
  {L"ryu", L"ÇËÇ„"},
  {L"ryo", L"ÇËÇÂ"},
  {L"ryi", L"ÇËÇ°"},
  {L"rye", L"ÇËÇ•"},
  {L"rya", L"ÇËÇ·"},
  {L"ru", L"ÇÈ"},
  {L"ro", L"ÇÎ"},
  {L"ri", L"ÇË"},
  {L"re", L"ÇÍ"},
  {L"ra", L"ÇÁ"},
  {L"qyu", L"Ç≠Ç„"},
  {L"qyo", L"Ç≠ÇÂ"},
  {L"qyi", L"Ç≠Ç°"},
  {L"qye", L"Ç≠Ç•"},
  {L"qya", L"Ç≠Ç·"},
  {L"qwu", L"Ç≠Ç£"},
  {L"qwo", L"Ç≠Çß"},
  {L"qwi", L"Ç≠Ç°"},
  {L"qwe", L"Ç≠Ç•"},
  {L"qwa", L"Ç≠Çü"},
  {L"qu", L"Ç≠"},
  {L"qo", L"Ç≠Çß"},
  {L"qi", L"Ç≠Ç°"},
  {L"qe", L"Ç≠Ç•"},
  {L"qa", L"Ç≠Çü"},
  {L"pyu", L"Ç“Ç„"},
  {L"pyo", L"Ç“ÇÂ"},
  {L"pyi", L"Ç“Ç°"},
  {L"pye", L"Ç“Ç•"},
  {L"pya", L"Ç“Ç·"},
  {L"pu", L"Ç’"},
  {L"po", L"Ç€"},
  {L"pi", L"Ç“"},
  {L"pe", L"Çÿ"},
  {L"pa", L"Çœ"},
  {L"o", L"Ç®"},
  {L"nÅf", L"ÇÒ"},
  {L"nz", L"ÇÒ", L"z"},
  {L"nyu", L"Ç…Ç„"},
  {L"nyo", L"Ç…ÇÂ"},
  {L"nyi", L"Ç…Ç°"},
  {L"nye", L"Ç…Ç•"},
  {L"nya", L"Ç…Ç·"},
  {L"nx", L"ÇÒ", L"x"},
  {L"nw", L"ÇÒ", L"w"},
  {L"nv", L"ÇÒ", L"v"},
  {L"nu", L"Ç "},
  {L"nt", L"ÇÒ", L"t"},
  {L"ns", L"ÇÒ", L"s"},
  {L"nr", L"ÇÒ", L"r"},
  {L"nq", L"ÇÒ", L"q"},
  {L"np", L"ÇÒ", L"p"},
  {L"no", L"ÇÃ"},
  {L"nn", L"ÇÒ"},
  {L"nm", L"ÇÒ", L"m"},
  {L"nl", L"ÇÒ", L"l"},
  {L"nk", L"ÇÒ", L"k"},
  {L"nj", L"ÇÒ", L"j"},
  {L"ni", L"Ç…"},
  {L"nh", L"ÇÒ", L"h"},
  {L"ng", L"ÇÒ", L"g"},
  {L"nf", L"ÇÒ", L"f"},
  {L"ne", L"ÇÀ"},
  {L"nd", L"ÇÒ", L"d"},
  {L"nc", L"ÇÒ", L"c"},
  {L"nb", L"ÇÒ", L"b"},
  {L"na", L"Ç»"},
  {L"n@", L"ÇÒ", L"@"},
  {L"n-", L"ÇÒ", L"-"},
  {L"myu", L"Ç›Ç„"},
  {L"myo", L"Ç›ÇÂ"},
  {L"myi", L"Ç›Ç°"},
  {L"mye", L"Ç›Ç•"},
  {L"mya", L"Ç›Ç·"},
  {L"mu", L"Çﬁ"},
  {L"mo", L"Ç‡"},
  {L"mi", L"Ç›"},
  {L"me", L"Çﬂ"},
  {L"ma", L"Ç‹"},
  {L"lyu", L"Ç„"},
  {L"lyo", L"ÇÂ"},
  {L"lyi", L"Ç°"},
  {L"lye", L"Ç•"},
  {L"lya", L"Ç·"},
  {L"lwa", L"ÇÏ"},
  {L"lu", L"Ç£"},
  {L"ltu", L"Ç¡"},
  {L"ltsu", L"Ç¡"},
  {L"lo", L"Çß"},
  {L"lke", L"Éñ"},
  {L"lka", L"Éï"},
  {L"li", L"Ç°"},
  {L"le", L"Ç•"},
  {L"la", L"Çü"},
  {L"kyu", L"Ç´Ç„"},
  {L"kyo", L"Ç´ÇÂ"},
  {L"kyi", L"Ç´Ç°"},
  {L"kye", L"Ç´Ç•"},
  {L"kya", L"Ç´Ç·"},
  {L"kwu", L"Ç≠Ç£"},
  {L"kwo", L"Ç≠Çß"},
  {L"kwi", L"Ç≠Ç°"},
  {L"kwe", L"Ç≠Ç•"},
  {L"kwa", L"Ç≠Çü"},
  {L"ku", L"Ç≠"},
  {L"ko", L"Ç±"},
  {L"ki", L"Ç´"},
  {L"ke", L"ÇØ"},
  {L"ka", L"Ç©"},
  {L"jyu", L"Ç∂Ç„"},
  {L"jyo", L"Ç∂ÇÂ"},
  {L"jyi", L"Ç∂Ç°"},
  {L"jye", L"Ç∂Ç•"},
  {L"jya", L"Ç∂Ç·"},
  {L"ju", L"Ç∂Ç„"},
  {L"jo", L"Ç∂ÇÂ"},
  {L"ji", L"Ç∂"},
  {L"je", L"Ç∂Ç•"},
  {L"ja", L"Ç∂Ç·"},
  {L"i", L"Ç¢"},
  {L"hyu", L"Ç–Ç„"},
  {L"hyo", L"Ç–ÇÂ"},
  {L"hyi", L"Ç–Ç°"},
  {L"hye", L"Ç–Ç•"},
  {L"hya", L"Ç–Ç·"},
  {L"hu", L"Ç”"},
  {L"ho", L"ÇŸ"},
  {L"hi", L"Ç–"},
  {L"he", L"Ç÷"},
  {L"ha", L"ÇÕ"},
  {L"gyu", L"Ç¨Ç„"},
  {L"gyo", L"Ç¨ÇÂ"},
  {L"gyi", L"Ç¨Ç°"},
  {L"gye", L"Ç¨Ç•"},
  {L"gya", L"Ç¨Ç·"},
  {L"gwu", L"ÇÆÇ£"},
  {L"gwo", L"ÇÆÇß"},
  {L"gwi", L"ÇÆÇ°"},
  {L"gwe", L"ÇÆÇ•"},
  {L"gwa", L"ÇÆÇü"},
  {L"gu", L"ÇÆ"},
  {L"go", L"Ç≤"},
  {L"gi", L"Ç¨"},
  {L"ge", L"Ç∞"},
  {L"ga", L"Ç™"},
  {L"fyu", L"Ç”Ç„"},
  {L"fyo", L"Ç”ÇÂ"},
  {L"fyi", L"Ç”Ç°"},
  {L"fye", L"Ç”Ç•"},
  {L"fya", L"Ç”Ç·"},
  {L"fwu", L"Ç”Ç£"},
  {L"fwo", L"Ç”Çß"},
  {L"fwi", L"Ç”Ç°"},
  {L"fwe", L"Ç”Ç•"},
  {L"fwa", L"Ç”Çü"},
  {L"fu", L"Ç”"},
  {L"fo", L"Ç”Çß"},
  {L"fi", L"Ç”Ç°"},
  {L"fe", L"Ç”Ç•"},
  {L"fa", L"Ç”Çü"},
  {L"e", L"Ç¶"},
  {L"dyu", L"Ç¿Ç„"},
  {L"dyo", L"Ç¿ÇÂ"},
  {L"dyi", L"Ç¿Ç°"},
  {L"dye", L"Ç¿Ç•"},
  {L"dya", L"Ç¿Ç·"},
  {L"dwu", L"Ç«Ç£"},
  {L"dwo", L"Ç«Çß"},
  {L"dwi", L"Ç«Ç°"},
  {L"dwe", L"Ç«Ç•"},
  {L"dwa", L"Ç«Çü"},
  {L"du", L"Ç√"},
  {L"do", L"Ç«"},
  {L"di", L"Ç¿"},
  {L"dhu", L"Ç≈Ç„"},
  {L"dho", L"Ç≈ÇÂ"},
  {L"dhi", L"Ç≈Ç°"},
  {L"dhe", L"Ç≈Ç•"},
  {L"dha", L"Ç≈Ç·"},
  {L"de", L"Ç≈"},
  {L"da", L"Çæ"},
  {L"cyu", L"ÇøÇ„"},
  {L"cyo", L"ÇøÇÂ"},
  {L"cyi", L"ÇøÇ°"},
  {L"cye", L"ÇøÇ•"},
  {L"cya", L"ÇøÇ·"},
  {L"cu", L"Ç≠"},
  {L"co", L"Ç±"},
  {L"ci", L"Çµ"},
  {L"chu", L"ÇøÇ„"},
  {L"cho", L"ÇøÇÂ"},
  {L"chi", L"Çø"},
  {L"che", L"ÇøÇ•"},
  {L"cha", L"ÇøÇ·"},
  {L"ce", L"Çπ"},
  {L"ca", L"Ç©"},
  {L"byu", L"Ç—Ç„"},
  {L"byo", L"Ç—ÇÂ"},
  {L"byi", L"Ç—Ç°"},
  {L"bye", L"Ç—Ç•"},
  {L"bya", L"Ç—Ç·"},
  {L"bu", L"Ç‘"},
  {L"bo", L"Ç⁄"},
  {L"bi", L"Ç—"},
  {L"be", L"Ç◊"},
  {L"ba", L"ÇŒ"},
  {L"a", L"Ç†"},
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç‹Ç≈
};

static TABLEENTRY sokuon_table[] = {
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç©ÇÁ
  {L"zzyu", L"Ç¡Ç∂Ç„"},
  {L"zzyo", L"Ç¡Ç∂ÇÂ"},
  {L"zzyi", L"Ç¡Ç∂Ç°"},
  {L"zzye", L"Ç¡Ç∂Ç•"},
  {L"zzya", L"Ç¡Ç∂Ç·"},
  {L"zzu", L"Ç¡Ç∏"},
  {L"zzo", L"Ç¡Çº"},
  {L"zzi", L"Ç¡Ç∂"},
  {L"zze", L"Ç¡Ç∫"},
  {L"zza", L"Ç¡Ç¥"},
  {L"yyu", L"Ç¡Ç‰"},
  {L"yyo", L"Ç¡ÇÊ"},
  {L"yyi", L"Ç¡Ç¢"},
  {L"yye", L"Ç¡Ç¢Ç•"},
  {L"yya", L"Ç¡Ç‚"},
  {L"xxyu", L"Ç¡Ç„"},
  {L"xxyo", L"Ç¡ÇÂ"},
  {L"xxyi", L"Ç¡Ç°"},
  {L"xxye", L"Ç¡Ç•"},
  {L"xxya", L"Ç¡Ç·"},
  {L"xxwa", L"Ç¡ÇÏ"},
  {L"xxu", L"Ç¡Ç£"},
  {L"xxtu", L"Ç¡Ç¡"},
  {L"xxo", L"Ç¡Çß"},
  {L"xxn", L"Ç¡ÇÒ"},
  {L"xxke", L"Ç¡Éñ"},
  {L"xxka", L"Ç¡Éï"},
  {L"xxi", L"Ç¡Ç°"},
  {L"xxe", L"Ç¡Ç•"},
  {L"xxa", L"Ç¡Çü"},
  {L"wwu", L"Ç¡Ç§"},
  {L"wwo", L"Ç¡Ç"},
  {L"wwi", L"Ç¡Ç§Ç°"},
  {L"wwhu", L"Ç¡Ç§"},
  {L"wwho", L"Ç¡Ç§Çß"},
  {L"wwhi", L"Ç¡Ç§Ç°"},
  {L"wwhe", L"Ç¡Ç§Ç•"},
  {L"wwha", L"Ç¡Ç§Çü"},
  {L"wwe", L"Ç¡Ç§Ç•"},
  {L"wwa", L"Ç¡ÇÌ"},
  {L"vvyu", L"Ç¡ÉîÇ„"},
  {L"vvyo", L"Ç¡ÉîÇÂ"},
  {L"vvyi", L"Ç¡ÉîÇ°"},
  {L"vvye", L"Ç¡ÉîÇ•"},
  {L"vvya", L"Ç¡ÉîÇ·"},
  {L"vvu", L"Ç¡Éî"},
  {L"vvo", L"Ç¡ÉîÇß"},
  {L"vvi", L"Ç¡ÉîÇ°"},
  {L"vve", L"Ç¡ÉîÇ•"},
  {L"vva", L"Ç¡ÉîÇü"},
  {L"ttyu", L"Ç¡ÇøÇ„"},
  {L"ttyo", L"Ç¡ÇøÇÂ"},
  {L"ttyi", L"Ç¡ÇøÇ°"},
  {L"ttye", L"Ç¡ÇøÇ•"},
  {L"ttya", L"Ç¡ÇøÇ·"},
  {L"ttwu", L"Ç¡Ç∆Ç£"},
  {L"ttwo", L"Ç¡Ç∆Çß"},
  {L"ttwi", L"Ç¡Ç∆Ç°"},
  {L"ttwe", L"Ç¡Ç∆Ç•"},
  {L"ttwa", L"Ç¡Ç∆Çü"},
  {L"ttu", L"Ç¡Ç¬"},
  {L"ttsu", L"Ç¡Ç¬"},
  {L"ttso", L"Ç¡Ç¬Çß"},
  {L"ttsi", L"Ç¡Ç¬Ç°"},
  {L"ttse", L"Ç¡Ç¬Ç•"},
  {L"ttsa", L"Ç¡Ç¬Çü"},
  {L"tto", L"Ç¡Ç∆"},
  {L"tti", L"Ç¡Çø"},
  {L"tthu", L"Ç¡ÇƒÇ„"},
  {L"ttho", L"Ç¡ÇƒÇÂ"},
  {L"tthi", L"Ç¡ÇƒÇ°"},
  {L"tthe", L"Ç¡ÇƒÇ•"},
  {L"ttha", L"Ç¡ÇƒÇ·"},
  {L"tte", L"Ç¡Çƒ"},
  {L"tta", L"Ç¡ÇΩ"},
  {L"ssyu", L"Ç¡ÇµÇ„"},
  {L"ssyo", L"Ç¡ÇµÇÂ"},
  {L"ssyi", L"Ç¡ÇµÇ°"},
  {L"ssye", L"Ç¡ÇµÇ•"},
  {L"ssya", L"Ç¡ÇµÇ·"},
  {L"sswu", L"Ç¡Ç∑Ç£"},
  {L"sswo", L"Ç¡Ç∑Çß"},
  {L"sswi", L"Ç¡Ç∑Ç°"},
  {L"sswe", L"Ç¡Ç∑Ç•"},
  {L"sswa", L"Ç¡Ç∑Çü"},
  {L"ssu", L"Ç¡Ç∑"},
  {L"sso", L"Ç¡Çª"},
  {L"ssi", L"Ç¡Çµ"},
  {L"sshu", L"Ç¡ÇµÇ„"},
  {L"ssho", L"Ç¡ÇµÇÂ"},
  {L"sshi", L"Ç¡Çµ"},
  {L"sshe", L"Ç¡ÇµÇ•"},
  {L"ssha", L"Ç¡ÇµÇ·"},
  {L"sse", L"Ç¡Çπ"},
  {L"ssa", L"Ç¡Ç≥"},
  {L"rryu", L"Ç¡ÇËÇ„"},
  {L"rryo", L"Ç¡ÇËÇÂ"},
  {L"rryi", L"Ç¡ÇËÇ°"},
  {L"rrye", L"Ç¡ÇËÇ•"},
  {L"rrya", L"Ç¡ÇËÇ·"},
  {L"rru", L"Ç¡ÇÈ"},
  {L"rro", L"Ç¡ÇÎ"},
  {L"rri", L"Ç¡ÇË"},
  {L"rre", L"Ç¡ÇÍ"},
  {L"rra", L"Ç¡ÇÁ"},
  {L"qqyu", L"Ç¡Ç≠Ç„"},
  {L"qqyo", L"Ç¡Ç≠ÇÂ"},
  {L"qqyi", L"Ç¡Ç≠Ç°"},
  {L"qqye", L"Ç¡Ç≠Ç•"},
  {L"qqya", L"Ç¡Ç≠Ç·"},
  {L"qqwu", L"Ç¡Ç≠Ç£"},
  {L"qqwo", L"Ç¡Ç≠Çß"},
  {L"qqwi", L"Ç¡Ç≠Ç°"},
  {L"qqwe", L"Ç¡Ç≠Ç•"},
  {L"qqwa", L"Ç¡Ç≠Çü"},
  {L"qqu", L"Ç¡Ç≠"},
  {L"qqo", L"Ç¡Ç≠Çß"},
  {L"qqi", L"Ç¡Ç≠Ç°"},
  {L"qqe", L"Ç¡Ç≠Ç•"},
  {L"qqa", L"Ç¡Ç≠Çü"},
  {L"ppyu", L"Ç¡Ç“Ç„"},
  {L"ppyo", L"Ç¡Ç“ÇÂ"},
  {L"ppyi", L"Ç¡Ç“Ç°"},
  {L"ppye", L"Ç¡Ç“Ç•"},
  {L"ppya", L"Ç¡Ç“Ç·"},
  {L"ppu", L"Ç¡Ç’"},
  {L"ppo", L"Ç¡Ç€"},
  {L"ppi", L"Ç¡Ç“"},
  {L"ppe", L"Ç¡Çÿ"},
  {L"ppa", L"Ç¡Çœ"},
  {L"mmyu", L"Ç¡Ç›Ç„"},
  {L"mmyo", L"Ç¡Ç›ÇÂ"},
  {L"mmyi", L"Ç¡Ç›Ç°"},
  {L"mmye", L"Ç¡Ç›Ç•"},
  {L"mmya", L"Ç¡Ç›Ç·"},
  {L"mmu", L"Ç¡Çﬁ"},
  {L"mmo", L"Ç¡Ç‡"},
  {L"mmi", L"Ç¡Ç›"},
  {L"mme", L"Ç¡Çﬂ"},
  {L"mma", L"Ç¡Ç‹"},
  {L"llyu", L"Ç¡Ç„"},
  {L"llyo", L"Ç¡ÇÂ"},
  {L"llyi", L"Ç¡Ç°"},
  {L"llye", L"Ç¡Ç•"},
  {L"llya", L"Ç¡Ç·"},
  {L"llwa", L"Ç¡ÇÏ"},
  {L"llu", L"Ç¡Ç£"},
  {L"lltu", L"Ç¡Ç¡"},
  {L"lltsu", L"Ç¡Ç¡"},
  {L"llo", L"Ç¡Çß"},
  {L"llke", L"Ç¡Éñ"},
  {L"llka", L"Ç¡Éï"},
  {L"lli", L"Ç¡Ç°"},
  {L"lle", L"Ç¡Ç•"},
  {L"lla", L"Ç¡Çü"},
  {L"kkyu", L"Ç¡Ç´Ç„"},
  {L"kkyo", L"Ç¡Ç´ÇÂ"},
  {L"kkyi", L"Ç¡Ç´Ç°"},
  {L"kkye", L"Ç¡Ç´Ç•"},
  {L"kkya", L"Ç¡Ç´Ç·"},
  {L"kkwu", L"Ç¡Ç≠Ç£"},
  {L"kkwo", L"Ç¡Ç≠Çß"},
  {L"kkwi", L"Ç¡Ç≠Ç°"},
  {L"kkwe", L"Ç¡Ç≠Ç•"},
  {L"kkwa", L"Ç¡Ç≠Çü"},
  {L"kku", L"Ç¡Ç≠"},
  {L"kko", L"Ç¡Ç±"},
  {L"kki", L"Ç¡Ç´"},
  {L"kke", L"Ç¡ÇØ"},
  {L"kka", L"Ç¡Ç©"},
  {L"jjyu", L"Ç¡Ç∂Ç„"},
  {L"jjyo", L"Ç¡Ç∂ÇÂ"},
  {L"jjyi", L"Ç¡Ç∂Ç°"},
  {L"jjye", L"Ç¡Ç∂Ç•"},
  {L"jjya", L"Ç¡Ç∂Ç·"},
  {L"jju", L"Ç¡Ç∂Ç„"},
  {L"jjo", L"Ç¡Ç∂ÇÂ"},
  {L"jji", L"Ç¡Ç∂"},
  {L"jje", L"Ç¡Ç∂Ç•"},
  {L"jja", L"Ç¡Ç∂Ç·"},
  {L"hhyu", L"Ç¡Ç–Ç„"},
  {L"hhyo", L"Ç¡Ç–ÇÂ"},
  {L"hhyi", L"Ç¡Ç–Ç°"},
  {L"hhye", L"Ç¡Ç–Ç•"},
  {L"hhya", L"Ç¡Ç–Ç·"},
  {L"hhu", L"Ç¡Ç”"},
  {L"hho", L"Ç¡ÇŸ"},
  {L"hhi", L"Ç¡Ç–"},
  {L"hhe", L"Ç¡Ç÷"},
  {L"hha", L"Ç¡ÇÕ"},
  {L"ggyu", L"Ç¡Ç¨Ç„"},
  {L"ggyo", L"Ç¡Ç¨ÇÂ"},
  {L"ggyi", L"Ç¡Ç¨Ç°"},
  {L"ggye", L"Ç¡Ç¨Ç•"},
  {L"ggya", L"Ç¡Ç¨Ç·"},
  {L"ggwu", L"Ç¡ÇÆÇ£"},
  {L"ggwo", L"Ç¡ÇÆÇß"},
  {L"ggwi", L"Ç¡ÇÆÇ°"},
  {L"ggwe", L"Ç¡ÇÆÇ•"},
  {L"ggwa", L"Ç¡ÇÆÇü"},
  {L"ggu", L"Ç¡ÇÆ"},
  {L"ggo", L"Ç¡Ç≤"},
  {L"ggi", L"Ç¡Ç¨"},
  {L"gge", L"Ç¡Ç∞"},
  {L"gga", L"Ç¡Ç™"},
  {L"ffyu", L"Ç¡Ç”Ç„"},
  {L"ffyo", L"Ç¡Ç”ÇÂ"},
  {L"ffyi", L"Ç¡Ç”Ç°"},
  {L"ffye", L"Ç¡Ç”Ç•"},
  {L"ffya", L"Ç¡Ç”Ç·"},
  {L"ffwu", L"Ç¡Ç”Ç£"},
  {L"ffwo", L"Ç¡Ç”Çß"},
  {L"ffwi", L"Ç¡Ç”Ç°"},
  {L"ffwe", L"Ç¡Ç”Ç•"},
  {L"ffwa", L"Ç¡Ç”Çü"},
  {L"ffu", L"Ç¡Ç”"},
  {L"ffo", L"Ç¡Ç”Çß"},
  {L"ffi", L"Ç¡Ç”Ç°"},
  {L"ffe", L"Ç¡Ç”Ç•"},
  {L"ffa", L"Ç¡Ç”Çü"},
  {L"ddyu", L"Ç¡Ç¿Ç„"},
  {L"ddyo", L"Ç¡Ç¿ÇÂ"},
  {L"ddyi", L"Ç¡Ç¿Ç°"},
  {L"ddye", L"Ç¡Ç¿Ç•"},
  {L"ddya", L"Ç¡Ç¿Ç·"},
  {L"ddwu", L"Ç¡Ç«Ç£"},
  {L"ddwo", L"Ç¡Ç«Çß"},
  {L"ddwi", L"Ç¡Ç«Ç°"},
  {L"ddwe", L"Ç¡Ç«Ç•"},
  {L"ddwa", L"Ç¡Ç«Çü"},
  {L"ddu", L"Ç¡Ç√"},
  {L"ddo", L"Ç¡Ç«"},
  {L"ddi", L"Ç¡Ç¿"},
  {L"ddhu", L"Ç¡Ç≈Ç„"},
  {L"ddho", L"Ç¡Ç≈ÇÂ"},
  {L"ddhi", L"Ç¡Ç≈Ç°"},
  {L"ddhe", L"Ç¡Ç≈Ç•"},
  {L"ddha", L"Ç¡Ç≈Ç·"},
  {L"dde", L"Ç¡Ç≈"},
  {L"dda", L"Ç¡Çæ"},
  {L"ccyu", L"Ç¡ÇøÇ„"},
  {L"ccyo", L"Ç¡ÇøÇÂ"},
  {L"ccyi", L"Ç¡ÇøÇ°"},
  {L"ccye", L"Ç¡ÇøÇ•"},
  {L"ccya", L"Ç¡ÇøÇ·"},
  {L"ccu", L"Ç¡Ç≠"},
  {L"cco", L"Ç¡Ç±"},
  {L"cci", L"Ç¡Çµ"},
  {L"cchu", L"Ç¡ÇøÇ„"},
  {L"ccho", L"Ç¡ÇøÇÂ"},
  {L"cchi", L"Ç¡Çø"},
  {L"cche", L"Ç¡ÇøÇ•"},
  {L"ccha", L"Ç¡ÇøÇ·"},
  {L"cce", L"Ç¡Çπ"},
  {L"cca", L"Ç¡Ç©"},
  {L"bbyu", L"Ç¡Ç—Ç„"},
  {L"bbyo", L"Ç¡Ç—ÇÂ"},
  {L"bbyi", L"Ç¡Ç—Ç°"},
  {L"bbye", L"Ç¡Ç—Ç•"},
  {L"bbya", L"Ç¡Ç—Ç·"},
  {L"bbu", L"Ç¡Ç‘"},
  {L"bbo", L"Ç¡Ç⁄"},
  {L"bbi", L"Ç¡Ç—"},
  {L"bbe", L"Ç¡Ç◊"},
  {L"bba", L"Ç¡ÇŒ"},
// Åyç~èáÇ…É\Å[ÉgÅzÇ±Ç±Ç‹Ç≈
};

//////////////////////////////////////////////////////////////////////////////

std::wstring zenkaku_to_hankaku(const std::wstring& zenkaku) {
  std::wstring hankaku;
  bool flag;
  wchar_t ch;
  for (size_t k = 0; k < zenkaku.size(); ++k) {
    flag = true;
    ch = zenkaku[k];
    for (size_t i = 0; i < _countof(halfkana_table); ++i) {
      if (ch == halfkana_table[i].key[0]) {
        hankaku += halfkana_table[i].value;
        flag = false;
        break;
      }
    }
    if (flag) {
      hankaku += ch;
    }
  }
  for (size_t i = 0; i < _countof(kigou_table); ++i) {
    for (size_t k = 0; k < zenkaku.size(); ++k) {
      if (hankaku[k] == kigou_table[i].value[0]) {
        hankaku[k] = kigou_table[i].key[0];
        break;
      }
    }
  }
  WCHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  szBuf[0] = 0;
  DWORD dwFlags = LCMAP_HALFWIDTH;
  ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                 hankaku.c_str(), -1, szBuf, 1024);
  return szBuf;
} // zenkaku_to_hankaku

std::wstring hankaku_to_zenkaku(const std::wstring& hankaku) {
  std::wstring zenkaku = hankaku;
  for (size_t i = 0; i < _countof(kana_table); ++i) {
    unboost::replace_all(zenkaku, kana_table[i].key, kana_table[i].value);
  }
  for (size_t i = 0; i < _countof(kigou_table); ++i) {
    for (size_t k = 0; k < zenkaku.size(); ++k) {
      if (zenkaku[k] == kigou_table[i].key[0]) {
        zenkaku[k] = kigou_table[i].value[0];
        break;
      }
    }
  }
  WCHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  szBuf[0] = 0;
  DWORD dwFlags = LCMAP_FULLWIDTH;
  ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                 zenkaku.c_str(), -1, szBuf, 1024);
  return szBuf;
} // hankaku_to_zenkaku

std::wstring zenkaku_hiragana_to_katakana(const std::wstring& hiragana) {
  WCHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  szBuf[0] = 0;
  DWORD dwFlags = LCMAP_FULLWIDTH | LCMAP_KATAKANA;
  ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                 hiragana.c_str(), -1, szBuf, 1024);
  return szBuf;
} // zenkaku_hiragana_to_katakana

std::wstring zenkaku_katakana_to_hiragana(const std::wstring& katakana) {
  WCHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  szBuf[0] = 0;
  DWORD dwFlags = LCMAP_FULLWIDTH | LCMAP_HIRAGANA;
  ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
                 katakana.c_str(), -1, szBuf, 1024);
  return szBuf;
} // zenkaku_katakana_to_hiragana

std::wstring hiragana_to_romaji(const std::wstring& hiragana) {
  std::wstring romaji = hiragana;
  for (size_t i = 0; i < _countof(sokuon_table); ++i) {
    unboost::replace_all(romaji, sokuon_table[i].value, sokuon_table[i].key);
  }
  for (size_t i = 0; i < _countof(reverse_table); ++i) {
    unboost::replace_all(romaji, reverse_table[i].key, reverse_table[i].value);
  }
  for (size_t i = 0; i < _countof(romaji_table); ++i) {
    unboost::replace_all(romaji, romaji_table[i].value, romaji_table[i].key);
  }
  return romaji;
} // hiragana_to_romaji

std::wstring romaji_to_hiragana(const std::wstring& romaji) {
  std::wstring hiragana = romaji;
  for (size_t i = 0; i < _countof(sokuon_table); ++i) {
    unboost::replace_all(hiragana, sokuon_table[i].key, sokuon_table[i].value);
  }
  for (size_t i = 0; i < _countof(romaji_table); ++i) {
    unboost::replace_all(hiragana, romaji_table[i].key, romaji_table[i].value);
  }
  return hiragana;
} // romaji_to_hiragana

WCHAR convert_key_to_kana(BYTE vk, BOOL bShift) {
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
  case VK_OEM_COMMA:  return (bShift ? L'ÅA' : L'ÇÀ');
  case VK_OEM_PERIOD: return (bShift ? L'ÅB' : L'ÇÈ');
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
} // convert_key_to_kana

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

BOOL is_hiragana(WCHAR ch) {
  if (0x3040 <= ch && ch <= 0x309F) return TRUE;
  switch (ch) {
  case 0x3095: case 0x3096: case 0x3099: case 0x309A:
  case 0x309B: case 0x309C: case 0x309D: case 0x309E:
  case 0x30FC:
    return TRUE;
  default:
    return FALSE;
  }
}

BOOL is_zenkaku_katakana(WCHAR ch) {
  if (0x30A0 <= ch && ch <= 0x30FF) return TRUE;
  switch (ch) {
  case 0x30FD: case 0x30FE: case 0x3099: case 0x309A:
  case 0x309B: case 0x309C: case 0x30FC:
    return TRUE;
  default:
    return FALSE;
  }
}

BOOL is_hankaku_katakana(WCHAR ch) {
  if (0xFF65 <= ch && ch <= 0xFF9F) return TRUE;
  switch (ch) {
  case 0xFF61: case 0xFF62: case 0xFF63: case 0xFF64:
    return TRUE;
  default:
    return FALSE;
  }
}

BOOL is_kanji(WCHAR ch) {
  // CJKìùçáäøéö
  if (0x4E00 <= ch && ch <= 0x9FFF) return TRUE;
  // CJKå›ä∑äøéö
  if (0xF900 <= ch && ch <= 0xFAFF) return TRUE;
  return FALSE;
}

BOOL is_fullwidth_ascii(WCHAR ch) {
  return (0xFF00 <= ch && ch <= 0xFFEF);
}

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
  default:                     return 0;
  }
}

std::wstring lcmap(const std::wstring& str, DWORD dwFlags) {
  WCHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
    str.c_str(), -1, szBuf, 1024);
  return szBuf;
}

//////////////////////////////////////////////////////////////////////////////
// LogCompStrExtra

std::wstring
LogCompStrExtra::Join(const std::vector<SmallWString>& strs) const {
  std::wstring str;
  for (size_t i = 0; i < strs.size(); ++i) {
    str += strs[i].c_str();
  }
  return str;
}

std::wstring
LogCompStrExtra::JoinLeft(const std::vector<SmallWString>& strs) const {
  std::wstring str;
  for (DWORD i = 0; i < dwPhonemeCursor; ++i) {
    str += strs[i].c_str();
  }
  return str;
}

std::wstring
LogCompStrExtra::JoinRight(const std::vector<SmallWString>& strs) const {
  std::wstring str;
  const DWORD dwCount = (DWORD)strs.size();
  for (DWORD i = dwPhonemeCursor; i < dwCount; ++i) {
    str += strs[i].c_str();
  }
  return str;
}

void LogCompStrExtra::InsertPos(
  std::vector<SmallWString>& strs, std::wstring& str)
{
  strs.insert(strs.begin() + dwPhonemeCursor, str.c_str());
}

WCHAR LogCompStrExtra::GetPrevChar() const {
  WCHAR ret = 0;
  if (dwCharDelta > 0) {
    assert(dwCharDelta - 1 < (DWORD)hiragana_phonemes[dwPhonemeCursor].size());
    ret = hiragana_phonemes[dwPhonemeCursor][dwCharDelta - 1];
  } else {
    if (dwPhonemeCursor > 0) {
      const std::wstring& str = hiragana_phonemes[dwPhonemeCursor - 1];
      assert(str.size() > 0);
      ret = str[str.size() - 1];
    }
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// LogCompStr

void LogCompStr::ExtraUpdated(INPUT_MODE imode) {
  std::wstring strLeft, strRight;
  switch (imode) {
  case IMODE_ZEN_HIRAGANA:
    strLeft = extra.JoinLeft(extra.hiragana_phonemes);
    strRight = extra.JoinRight(extra.hiragana_phonemes);
    break;
  case IMODE_ZEN_KATAKANA:
    strLeft = lcmap(extra.JoinLeft(extra.hiragana_phonemes), LCMAP_KATAKANA);
    strRight = lcmap(extra.JoinRight(extra.hiragana_phonemes), LCMAP_KATAKANA);
    break;
  case IMODE_HAN_KANA:
    strLeft = lcmap(extra.JoinLeft(extra.hiragana_phonemes),
                    LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    strRight = lcmap(extra.JoinRight(extra.hiragana_phonemes),
                     LCMAP_HALFWIDTH | LCMAP_KATAKANA);
    break;
  case IMODE_ZEN_EISUU:
    strLeft = lcmap(extra.JoinLeft(extra.typing_phonemes), LCMAP_FULLWIDTH);
    strRight = lcmap(extra.JoinRight(extra.typing_phonemes), LCMAP_FULLWIDTH);
    break;
  case IMODE_HAN_KANA:
    strLeft = lcmap(extra.JoinLeft(extra.hiragana_phonemes), LCMAP_HALFWIDTH);
    strRight = lcmap(extra.JoinRight(extra.hiragana_phonemes), LCMAP_HALFWIDTH);
    break;
  default:
    break;
  }
  comp_str = strLeft + strRight;
  dwCursorPos = (DWORD)strLeft.size() + extra.dwCharDelta;
}

void LogCompStr::AddKanaChar(
  std::wstring& typed, std::wstring& translated, INPUT_MODE imode)
{
  WCHAR chDakuon = dakuon_shori(extra.GetPrevChar(), translated[0]);
  if (chDakuon) {
    std::wstring str;
    str += chDakuon;
    hiragana_phonemes[dwPhonemeCursor - 1] = str;
  } else {
    // create the typed string
    extra.InsertPos(extra.typing_phonemes, typed);
    // create the translated string
    extra.InsertPos(extra.hiragana_phonemes, translated);
    extra.dwPhonemeCursor++;
  }
  // create the composition string
  ExtraUpdated(imode);
} // LogCompStr::AddKanaChar

void LogCompStr::AddRomanChar(
  std::wstring& typed, std::wstring& translated, INPUT_MODE imode)
{
  const WCHAR chTyped = typed[0];
  // create the typed string and translated string
  if (is_hiragana(chTyped)) {
    translated = typed;
    for (size_t i = 0; i < _countof(reverse_table); ++i) {
      if (reverse_table[i].key == translated) {
        typed = reverse_table[i].value;
        break;
      }
    }
    extra.InsertPos(extra.typing_phonemes, typed);
    extra.InsertPos(extra.hiragana_phonemes, translated);
  } else if (is_zenkaku_katakana(chTyped)) {
    translated = lcmap(typed, LCMAP_HIRAGANA);
    for (size_t i = 0; i < _countof(reverse_table); ++i) {
      if (reverse_table[i].key == translated) {
        typed = reverse_table[i].value;
        break;
      }
    }
    extra.InsertPos(extra.typing_phonemes, typed);
    extra.InsertPos(extra.hiragana_phonemes, translated);
  } else if (is_kanji(chTyped)) {
    translated = typed;
    extra.InsertPos(extra.typing_phonemes, typed);
    extra.InsertPos(extra.hiragana_phonemes, translated);
  } else if (std::isalnum(chTyped)) {
    WCHAR ch = extra.GetPrevChar();
    if (is_hiragana(ch) || ch == L'\'' || ch == 0) {
      translated = typed;
      extra.InsertPos(extra.typing_phonemes, typed);
      extra.InsertPos(extra.hiragana_phonemes, translated);
    } else {
      if (extra.dwCharDelta) {
        // TODO:
      } else {
        translated = typed;
        extra.InsertPos(extra.typing_phonemes, typed);
        extra.InsertPos(extra.hiragana_phonemes, translated);
      }
    }
  } else {
    translated = typed;
    extra.InsertPos(extra.typing_phonemes, typed);
    extra.InsertPos(extra.hiragana_phonemes, translated);
  }

  // create the composition string
  ExtraUpdated(imode);
} // LogCompStr::AddRomanChar

void LogCompStr::AddChar(WCHAR chTyped, WCHAR chTranslated, INPUT_MODE imode) {
  std::wstring strTyped, strTranslated;
  if (chTranslated) {   // kana input
    assert(is_hiragana(chTranslated));
    strTyped += chTyped;
    strTranslated += chTranslated;
    AddKanaChar(strTyped, strTranslated, imode);
  } else {  // roman input
    strTyped += chTyped;
    strTranslated = strTyped;
    AddRomanChar(strTyped, strTranslated, imode);
  }
  // create the reading string
  std::wstring str = extra.Join(hiragana_phonemes);
  comp_read_str = lcmap(str, LCMAP_HALFWIDTH | LCMAP_KATAKANA);
} // LogCompStr::AddChar

void LogCompStr::Revert() {
  // reset composition
  comp_str = lcmap(extra.Join(comp_read_str),
                   LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
  comp_clause.resize(2);
  comp_clause[0] = 0;
  comp_clause[1] = (DWORD)comp_str.size();
  comp_attr.assign(comp_read_str.size(), ATTR_INPUT);
  dwCursorPos = (DWORD)comp_str.size();
  dwDeltaStart = 0;
}

void LogCompStr::DeleteChar(BOOL bBackSpace/* = FALSE*/) {
  // delete char
  if (bBackSpace) {
    if (dwCursorPos == 0) {
      DebugPrint(TEXT("dwCursorPos == 0\n"));
      return;
    } else if (dwCursorPos <= comp_str.size()) {
      --dwCursorPos;
      comp_str.erase(dwCursorPos);
      dwDeltaStart = dwCursorPos;
    } else {
      dwCursorPos = (DWORD)comp_str.size();
      dwDeltaStart = dwCursorPos;
    }
  } else {
    if (dwCursorPos >= comp_str.size()) {
      DebugPrint(TEXT("dwCursorPos >= comp_str.size()\n"));
      return;
    } else {
      comp_str.erase(dwCursorPos);
      dwCursorPos = dwDeltaStart;
    }
  }

  // reset composition
  comp_str = comp_read_str;
  comp_clause.resize(2);
  comp_clause[0] = 0;
  comp_clause[1] = (DWORD)comp_str.size();
  comp_attr.assign(comp_read_str.size(), ATTR_INPUT);
  dwCursorPos = (DWORD)comp_str.size();
  dwDeltaStart = 0;
} // LogCompStr::DeleteChar

void LogCompStr::MakeResult() {
}

void LogCompStr::MakeHiragana() {
  std::wstring str;
  str = romaji_to_hiragana(comp_read_str);
  str = zenkaku_katakana_to_hiragana(str);

  // update composition
  comp_str = str;
  DWORD len = (DWORD)comp_str.size();
  dwCursorPos = len;
  dwDeltaStart = 0;
  comp_attr.assign(len, ATTR_INPUT);
  comp_clause.resize(2);
  comp_clause[0] = 0;
  comp_clause[1] = len;
}

void LogCompStr::MakeKatakana() {
  std::wstring str;
  str = romaji_to_hiragana(comp_read_str);
  str = zenkaku_hiragana_to_katakana(str);

  comp_str = str;
  DWORD len = (DWORD)comp_str.size();
  dwCursorPos = len;
  dwDeltaStart = 0;
  comp_attr.assign(len, ATTR_INPUT);
  comp_clause.resize(2);
  comp_clause[0] = 0;
  comp_clause[1] = len;
}

void LogCompStr::MakeHankaku() {
  std::wstring str;
  str = romaji_to_hiragana(comp_read_str);
  str = zenkaku_to_hankaku(str);

  comp_str = str;
  DWORD len = (DWORD)comp_str.size();
  dwCursorPos = len;
  dwDeltaStart = 0;
  comp_attr.assign(len, ATTR_INPUT);
  comp_clause.resize(2);
  comp_clause[0] = 0;
  comp_clause[1] = len;
}

void LogCompStr::MakeZenEisuu() {
  std::wstring str;
  str = hiragana_to_romaji(comp_read_str);
  str = hankaku_to_zenkaku(str);

  comp_str = str;
  DWORD len = (DWORD)comp_str.size();
  dwCursorPos = len;
  dwDeltaStart = 0;
  comp_attr.assign(len, ATTR_INPUT);
  comp_clause.resize(2);
  comp_clause[0] = 0;
  comp_clause[1] = len;
}

void LogCompStr::MakeHanEisuu() {
  std::wstring str;
  str = hiragana_to_romaji(comp_read_str);
  str = zenkaku_to_hankaku(str);

  comp_str = str;
  DWORD len = (DWORD)comp_str.size();
  dwCursorPos = len;
  dwDeltaStart = 0;
  comp_attr.assign(len, ATTR_INPUT);
  comp_clause.resize(2);
  comp_clause[0] = 0;
  comp_clause[1] = len;
}

void LogCompStr::MoveLeft() {
  DWORD dwCursorPos = log.dwCursorPos;
  if (bIsBeingConverted) {
    size_t i, siz = log.comp_clause.size();
    if (siz > 1) {
      for (i = 0; i < siz; ++i) {
        if (dwCursorPos <= log.comp_clause[i]) {
          if (i == 0) {
            i = siz - 2;
          } else {
            --i;
          }
          break;
        }
      }
      dwCursorPos = log.comp_clause[i];
    }
  } else {
    if (log.dwCursorPos > 0) {
      --dwCursorPos;
    } else {
      return;
    }
  }
  log.dwCursorPos = dwCursorPos;
} // LogCompStr::MoveLeft

void LogCompStr::MoveRight() {
  DWORD dwCursorPos = log.dwCursorPos;
  if (bIsBeingConverted) {
    size_t i, k, siz = log.comp_clause.size();
    if (siz > 1) {
      for (i = k = 0; i < siz; ++i) {
        if (log.comp_clause[i] <= dwCursorPos) {
          if (siz <= i + 1) {
            k = 0;
          } else {
            k = i + 1;
          }
        }
      }
      dwCursorPos = log.comp_clause[k];
    }
  } else {
    if (log.dwCursorPos < (DWORD)log.comp_str.size()) {
      ++dwCursorPos;
    } else {
      return;
    }
  }
  log.dwCursorPos = dwCursorPos;
} // LogCompStr::MoveRight

//////////////////////////////////////////////////////////////////////////////
