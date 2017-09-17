// keychar.cpp --- mzimeja keys and characters
//////////////////////////////////////////////////////////////////////////////
// NOTE: This file uses Japanese cp932 encoding. To compile this on g++,
//       please add options: -finput-charset=CP932 -fexec-charset=CP932

#include "../mzimeja.h"
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

static KEYVALUE halfkana_table[] = {
  {L"�J", L"�"},
  {L"�K", L"�"},
  {L"�[", L"-"},
  {L"�u", L"�"},
  {L"�v", L"�"},
  {L"�B", L"�"},
  {L"�A", L"�"},
  {L"�E", L"�"},
  {L"���J", L"��"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"��"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
  {L"��", L"�"},
};

static KEYVALUE kana_table[] = {
  {L"�", L"�["},
  {L"�", L"�J"},
  {L"�", L"�K"},
  {L"�", L"�u"},
  {L"�", L"�v"},
  {L"�", L"�B"},
  {L"�", L"�A"},
  {L"�", L"�E"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"�", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
  {L"���", L"��"},
};

static KEYVALUE kigou_table[] = {
  {L",", L"�A"},
  {L".", L"�B"},
  {L"/", L"�E"},
  {L"~", L"�`"},
  {L"[", L"�u"},
  {L"]", L"�v"},
  {L"{", L"�o"},
  {L"}", L"�p"},
  {L":", L"�F"},
  {L";", L"�G"},
  {L"<", L"��"},
  {L"=", L"��"},
  {L">", L"��"},
  {L"?", L"�H"},
  {L"@", L"��"},
  {L"\\", L"��"},
  {L"^", L"�O"},
  {L"_", L"�Q"},
  {L"`", L"�M"},
  {L"|", L"�b"},
  {L"!", L"�I"},
  {L"\"", L"��"},
  {L"#", L"��"},
  {L"$", L"��"},
  {L"%", L"��"},
  {L"&", L"��"},
  {L"'", L"�f"},
  {L"(", L"�i"},
  {L")", L"�j"},
  {L"*", L"��"},
  {L"+", L"�{"},
  {L"-", L"�["},
};

static KEYVALUEEXTRA reverse_roman_table[] = {
  {L"���J��", L"va"},
  {L"���J��", L"vi"},
  {L"���J", L"vu"},
  {L"���J��", L"ve"},
  {L"���J��", L"vo"},
  {L"�����J", L"v", L"���J"},
  {L"����", L"xxa"},
  {L"����", L"xxi"},
  {L"����", L"xxu"},
  {L"����", L"xxe"},
  {L"����", L"xxo"},
  {L"����", L"kka"},
  {L"����", L"k", L"��"},
  {L"����", L"kku"},
  {L"����", L"kke"},
  {L"����", L"kko"},
  {L"����", L"gga"},
  {L"����", L"g", L"��"},
  {L"����", L"ggu"},
  {L"����", L"gge"},
  {L"����", L"ggo"},
  {L"����", L"ssa"},
  {L"����", L"s", L"��"},
  {L"����", L"ssu"},
  {L"����", L"sse"},
  {L"����", L"sso"},
  {L"����", L"zza"},
  {L"����", L"z", L"��"},
  {L"����", L"zzu"},
  {L"����", L"zze"},
  {L"����", L"zzo"},
  {L"����", L"tta"},
  {L"����", L"t", L"��"},
  {L"����", L"ttu"},
  {L"����", L"tte"},
  {L"����", L"tto"},
  {L"����", L"dda"},
  {L"����", L"d", L"��"},
  {L"����", L"ddu"},
  {L"����", L"dde"},
  {L"����", L"ddo"},
  {L"����", L"hha"},
  {L"����", L"h", L"��"},
  {L"����", L"hhu"},
  {L"����", L"hhe"},
  {L"����", L"hho"},
  {L"����", L"bba"},
  {L"����", L"b", L"��"},
  {L"����", L"bbu"},
  {L"����", L"bbe"},
  {L"����", L"bbo"},
  {L"����", L"ppa"},
  {L"����", L"p", L"��"},
  {L"����", L"ppu"},
  {L"����", L"ppe"},
  {L"����", L"ppo"},
  {L"����", L"mma"},
  {L"����", L"m", L"��"},
  {L"����", L"mmu"},
  {L"����", L"mme"},
  {L"����", L"mmo"},
  {L"����", L"yya"},
  {L"����", L"yyu"},
  {L"����", L"yyo"},
  {L"����", L"xxya"},
  {L"����", L"xxyu"},
  {L"����", L"xxyo"},
  {L"����", L"rra"},
  {L"����", L"r", L"��"},
  {L"����", L"rru"},
  {L"����", L"rre"},
  {L"����", L"rro"},
  {L"����", L"xxwa"},
  {L"����", L"wwa"},
  {L"����", L"wwi"},
  {L"����", L"wwe"},
  {L"����", L"wwo"},
  {L"����", L"kya"},
  {L"����", L"kyi"},
  {L"����", L"kyu"},
  {L"����", L"kye"},
  {L"����", L"kyo"},
  {L"����", L"gya"},
  {L"����", L"gyi"},
  {L"����", L"gyu"},
  {L"����", L"gye"},
  {L"����", L"gyo"},
  {L"����", L"sya"},
  {L"����", L"syi"},
  {L"����", L"syu"},
  {L"����", L"sye"},
  {L"����", L"syo"},
  {L"����", L"zya"},
  {L"����", L"zyi"},
  {L"����", L"zyu"},
  {L"����", L"zye"},
  {L"����", L"zyo"},
  {L"����", L"tya"},
  {L"����", L"tyi"},
  {L"����", L"tyu"},
  {L"����", L"tye"},
  {L"����", L"tyo"},
  {L"����", L"dya"},
  {L"����", L"dyi"},
  {L"����", L"dyu"},
  {L"����", L"dye"},
  {L"����", L"dyo"},
  {L"�ł�", L"dhi"},
  {L"�ɂ�", L"nya"},
  {L"�ɂ�", L"nyi"},
  {L"�ɂ�", L"nyu"},
  {L"�ɂ�", L"nye"},
  {L"�ɂ�", L"nyo"},
  {L"�Ђ�", L"hya"},
  {L"�Ђ�", L"hyi"},
  {L"�Ђ�", L"hyu"},
  {L"�Ђ�", L"hye"},
  {L"�Ђ�", L"hyo"},
  {L"�т�", L"bya"},
  {L"�т�", L"byi"},
  {L"�т�", L"byu"},
  {L"�т�", L"bye"},
  {L"�т�", L"byo"},
  {L"�҂�", L"pya"},
  {L"�҂�", L"pyi"},
  {L"�҂�", L"pyu"},
  {L"�҂�", L"pye"},
  {L"�҂�", L"pyo"},
  {L"�݂�", L"mya"},
  {L"�݂�", L"myi"},
  {L"�݂�", L"myu"},
  {L"�݂�", L"mye"},
  {L"�݂�", L"myo"},
  {L"���", L"rya"},
  {L"�股", L"ryi"},
  {L"���", L"ryu"},
  {L"�肥", L"rye"},
  {L"���", L"ryo"},
  {L"��", L"n'a"},
  {L"��", L"n'i"},
  {L"��", L"n'u"},
  {L"��", L"n'e"},
  {L"��", L"n'o"},
  {L"��", L"a"},
  {L"��", L"i"},
  {L"��", L"u"},
  {L"��", L"e"},
  {L"��", L"o"},
  {L"��", L"xa"},
  {L"��", L"xi"},
  {L"��", L"xu"},
  {L"��", L"xe"},
  {L"��", L"xo"},
  {L"��", L"ka"},
  {L"��", L"ki"},
  {L"��", L"ku"},
  {L"��", L"ke"},
  {L"��", L"ko"},
  {L"��", L"ga"},
  {L"��", L"gi"},
  {L"��", L"gu"},
  {L"��", L"ge"},
  {L"��", L"go"},
  {L"��", L"sa"},
  {L"��", L"si"},
  {L"��", L"su"},
  {L"��", L"se"},
  {L"��", L"so"},
  {L"��", L"za"},
  {L"��", L"zi"},
  {L"��", L"zu"},
  {L"��", L"ze"},
  {L"��", L"zo"},
  {L"��", L"ta"},
  {L"��", L"ti"},
  {L"��", L"tu"},
  {L"��", L"te"},
  {L"��", L"to"},
  {L"��", L"da"},
  {L"��", L"di"},
  {L"��", L"du"},
  {L"��", L"de"},
  {L"��", L"do"},
  {L"��", L"xtu"},
  {L"��", L"na"},
  {L"��", L"ni"},
  {L"��", L"nu"},
  {L"��", L"ne"},
  {L"��", L"no"},
  {L"��", L"ha"},
  {L"��", L"hi"},
  {L"��", L"hu"},
  {L"��", L"he"},
  {L"��", L"ho"},
  {L"��", L"ba"},
  {L"��", L"bi"},
  {L"��", L"bu"},
  {L"��", L"be"},
  {L"��", L"bo"},
  {L"��", L"pa"},
  {L"��", L"pi"},
  {L"��", L"pu"},
  {L"��", L"pe"},
  {L"��", L"po"},
  {L"��", L"ma"},
  {L"��", L"mi"},
  {L"��", L"mu"},
  {L"��", L"me"},
  {L"��", L"mo"},
  {L"��", L"xya"},
  {L"��", L"ya"},
  {L"��", L"xyu"},
  {L"��", L"yu"},
  {L"��", L"xyo"},
  {L"��", L"yo"},
  {L"��", L"ra"},
  {L"��", L"ri"},
  {L"��", L"ru"},
  {L"��", L"re"},
  {L"��", L"ro"},
  {L"��", L"xwa"},
  {L"��", L"wa"},
  {L"��", L"wi"},
  {L"��", L"we"},
  {L"��", L"wo"},
  {L"��", L"n"},
  {L"�[", L"-"},
  {L"�`", L"~"},
  {L"�u", L"["},
  {L"�v", L"]"},
};

static KEYVALUEEXTRA normal_roman_table[] = {
  {L"a", L"��"},
  {L"i", L"��"},
  {L"u", L"��"},
  {L"e", L"��"},
  {L"o", L"��"},
  {L"ka", L"��"},
  {L"ki", L"��"},
  {L"ku", L"��"},
  {L"ke", L"��"},
  {L"ko", L"��"},
  {L"sa", L"��"},
  {L"si", L"��"},
  {L"su", L"��"},
  {L"se", L"��"},
  {L"so", L"��"},
  {L"ta", L"��"},
  {L"ti", L"��"},
  {L"tu", L"��"},
  {L"te", L"��"},
  {L"to", L"��"},
  {L"na", L"��"},
  {L"ni", L"��"},
  {L"nu", L"��"},
  {L"ne", L"��"},
  {L"no", L"��"},
  {L"ha", L"��"},
  {L"hi", L"��"},
  {L"hu", L"��"},
  {L"he", L"��"},
  {L"ho", L"��"},
  {L"ma", L"��"},
  {L"mi", L"��"},
  {L"mu", L"��"},
  {L"me", L"��"},
  {L"mo", L"��"},
  {L"ya", L"��"},
  {L"yi", L"��"},
  {L"yu", L"��"},
  {L"ye", L"����"},
  {L"yo", L"��"},
  {L"ra", L"��"},
  {L"ri", L"��"},
  {L"ru", L"��"},
  {L"re", L"��"},
  {L"ro", L"��"},
  {L"wa", L"��"},
  {L"wi", L"����"},
  {L"wu", L"��"},
  {L"we", L"����"},
  {L"wo", L"��"},
  {L"ga", L"��"},
  {L"gi", L"��"},
  {L"gu", L"��"},
  {L"ge", L"��"},
  {L"go", L"��"},
  {L"za", L"��"},
  {L"zi", L"��"},
  {L"zu", L"��"},
  {L"ze", L"��"},
  {L"zo", L"��"},
  {L"da", L"��"},
  {L"di", L"��"},
  {L"du", L"��"},
  {L"de", L"��"},
  {L"do", L"��"},
  {L"ba", L"��"},
  {L"bi", L"��"},
  {L"bu", L"��"},
  {L"be", L"��"},
  {L"bo", L"��"},
  {L"pa", L"��"},
  {L"pi", L"��"},
  {L"pu", L"��"},
  {L"pe", L"��"},
  {L"po", L"��"},
  {L"fa", L"�ӂ�"},
  {L"fi", L"�ӂ�"},
  {L"fu", L"��"},
  {L"fe", L"�ӂ�"},
  {L"fo", L"�ӂ�"},
  {L"ja", L"����"},
  {L"ji", L"��"},
  {L"ju", L"����"},
  {L"je", L"����"},
  {L"jo", L"����"},
  {L"ca", L"��"},
  {L"ci", L"��"},
  {L"cu", L"��"},
  {L"ce", L"��"},
  {L"co", L"��"},
  {L"qa", L"����"},
  {L"qi", L"����"},
  {L"qu", L"��"},
  {L"qe", L"����"},
  {L"qo", L"����"},
  {L"va", L"����"},
  {L"vi", L"����"},
  {L"vu", L"��"},
  {L"ve", L"����"},
  {L"vo", L"����"},
  {L"kya", L"����"},
  {L"kyi", L"����"},
  {L"kyu", L"����"},
  {L"kye", L"����"},
  {L"kyo", L"����"},
  {L"gya", L"����"},
  {L"gyi", L"����"},
  {L"gyu", L"����"},
  {L"gye", L"����"},
  {L"gyo", L"����"},
  {L"sya", L"����"},
  {L"syi", L"����"},
  {L"syu", L"����"},
  {L"sye", L"����"},
  {L"syo", L"����"},
  {L"zya", L"����"},
  {L"zyi", L"����"},
  {L"zyu", L"����"},
  {L"zye", L"����"},
  {L"zyo", L"����"},
  {L"tya", L"����"},
  {L"tyi", L"����"},
  {L"tyu", L"����"},
  {L"tye", L"����"},
  {L"tyo", L"����"},
  {L"dya", L"����"},
  {L"dyi", L"����"},
  {L"dyu", L"����"},
  {L"dye", L"����"},
  {L"dyo", L"����"},
  {L"nya", L"�ɂ�"},
  {L"nyi", L"�ɂ�"},
  {L"nyu", L"�ɂ�"},
  {L"nye", L"�ɂ�"},
  {L"nyo", L"�ɂ�"},
  {L"hya", L"�Ђ�"},
  {L"hyi", L"�Ђ�"},
  {L"hyu", L"�Ђ�"},
  {L"hye", L"�Ђ�"},
  {L"hyo", L"�Ђ�"},
  {L"bya", L"�т�"},
  {L"byi", L"�т�"},
  {L"byu", L"�т�"},
  {L"bye", L"�т�"},
  {L"byo", L"�т�"},
  {L"pya", L"�҂�"},
  {L"pyi", L"�҂�"},
  {L"pyu", L"�҂�"},
  {L"pye", L"�҂�"},
  {L"pyo", L"�҂�"},
  {L"mya", L"�݂�"},
  {L"myi", L"�݂�"},
  {L"myu", L"�݂�"},
  {L"mye", L"�݂�"},
  {L"myo", L"�݂�"},
  {L"rya", L"���"},
  {L"ryi", L"�股"},
  {L"ryu", L"���"},
  {L"rye", L"�肥"},
  {L"ryo", L"���"},
  {L"jya", L"����"},
  {L"jyi", L"����"},
  {L"jyu", L"����"},
  {L"jye", L"����"},
  {L"jyo", L"����"},
  {L"vya", L"����"},
  {L"vyi", L"����"},
  {L"vyu", L"����"},
  {L"vye", L"����"},
  {L"vyo", L"����"},
  {L"qya", L"����"},
  {L"qyi", L"����"},
  {L"qyu", L"����"},
  {L"qye", L"����"},
  {L"qyo", L"����"},
  {L"cya", L"����"},
  {L"cyi", L"����"},
  {L"cyu", L"����"},
  {L"cye", L"����"},
  {L"cyo", L"����"},
  {L"fya", L"�ӂ�"},
  {L"fyi", L"�ӂ�"},
  {L"fyu", L"�ӂ�"},
  {L"fye", L"�ӂ�"},
  {L"fyo", L"�ӂ�"},
  {L"sha", L"����"},
  {L"shi", L"��"},
  {L"shu", L"����"},
  {L"she", L"����"},
  {L"sho", L"����"},
  {L"cha", L"����"},
  {L"chi", L"��"},
  {L"chu", L"����"},
  {L"che", L"����"},
  {L"cho", L"����"},
  {L"tha", L"�Ă�"},
  {L"thi", L"�Ă�"},
  {L"thu", L"�Ă�"},
  {L"the", L"�Ă�"},
  {L"tho", L"�Ă�"},
  {L"dha", L"�ł�"},
  {L"dhi", L"�ł�"},
  {L"dhu", L"�ł�"},
  {L"dhe", L"�ł�"},
  {L"dho", L"�ł�"},
  {L"wha", L"����"},
  {L"whi", L"����"},
  {L"whu", L"��"},
  {L"whe", L"����"},
  {L"who", L"����"},
  {L"kwa", L"����"},
  {L"kwi", L"����"},
  {L"kwu", L"����"},
  {L"kwe", L"����"},
  {L"kwo", L"����"},
  {L"qwa", L"����"},
  {L"qwi", L"����"},
  {L"qwu", L"����"},
  {L"qwe", L"����"},
  {L"qwo", L"����"},
  {L"gwa", L"����"},
  {L"gwi", L"����"},
  {L"gwu", L"����"},
  {L"gwe", L"����"},
  {L"gwo", L"����"},
  {L"swa", L"����"},
  {L"swi", L"����"},
  {L"swu", L"����"},
  {L"swe", L"����"},
  {L"swo", L"����"},
  {L"twa", L"�Ƃ�"},
  {L"twi", L"�Ƃ�"},
  {L"twu", L"�Ƃ�"},
  {L"twe", L"�Ƃ�"},
  {L"two", L"�Ƃ�"},
  {L"dwa", L"�ǂ�"},
  {L"dwi", L"�ǂ�"},
  {L"dwu", L"�ǂ�"},
  {L"dwe", L"�ǂ�"},
  {L"dwo", L"�ǂ�"},
  {L"fwa", L"�ӂ�"},
  {L"fwi", L"�ӂ�"},
  {L"fwu", L"�ӂ�"},
  {L"fwe", L"�ӂ�"},
  {L"fwo", L"�ӂ�"},
  {L"tsa", L"��"},
  {L"tsi", L"��"},
  {L"tsu", L"��"},
  {L"tse", L"��"},
  {L"tso", L"��"},
  {L"la", L"��"},
  {L"li", L"��"},
  {L"lu", L"��"},
  {L"le", L"��"},
  {L"lo", L"��"},
  {L"lya", L"��"},
  {L"lyi", L"��"},
  {L"lyu", L"��"},
  {L"lye", L"��"},
  {L"lyo", L"��"},
  {L"xa", L"��"},
  {L"xi", L"��"},
  {L"xu", L"��"},
  {L"xe", L"��"},
  {L"xo", L"��"},
  {L"xya", L"��"},
  {L"xyi", L"��"},
  {L"xyu", L"��"},
  {L"xye", L"��"},
  {L"xyo", L"��"},
  {L"lwa", L"��"},
  {L"xwa", L"��"},
  {L"lka", L"��"},
  {L"xka", L"��"},
  {L"lke", L"��"},
  {L"xke", L"��"},
  {L"ltu", L"��"},
  {L"ltsu", L"��"},
  {L"xtu", L"��"},
  {L"nn", L"��"},
  {L"n�f", L"��"},
  {L"xn", L"��"},
  {L"nb", L"��", L"b"},
  {L"nc", L"��", L"c"},
  {L"nd", L"��", L"d"},
  {L"nf", L"��", L"f"},
  {L"ng", L"��", L"g"},
  {L"nh", L"��", L"h"},
  {L"nj", L"��", L"j"},
  {L"nk", L"��", L"k"},
  {L"nl", L"��", L"l"},
  {L"nm", L"��", L"m"},
  {L"np", L"��", L"p"},
  {L"nq", L"��", L"q"},
  {L"nr", L"��", L"r"},
  {L"ns", L"��", L"s"},
  {L"nt", L"��", L"t"},
  {L"nv", L"��", L"v"},
  {L"nw", L"��", L"w"},
  {L"nx", L"��", L"x"},
  {L"nz", L"��", L"z"},
  {L"n-", L"��", L"-"},
  {L"n@", L"��", L"@"},
};

static KEYVALUE sokuon_table[] = {
  {L"kka", L"����"},
  {L"kki", L"����"},
  {L"kku", L"����"},
  {L"kke", L"����"},
  {L"kko", L"����"},
  {L"ssa", L"����"},
  {L"ssi", L"����"},
  {L"ssu", L"����"},
  {L"sse", L"����"},
  {L"sso", L"����"},
  {L"tta", L"����"},
  {L"tti", L"����"},
  {L"ttu", L"����"},
  {L"tte", L"����"},
  {L"tto", L"����"},
  {L"hha", L"����"},
  {L"hhi", L"����"},
  {L"hhu", L"����"},
  {L"hhe", L"����"},
  {L"hho", L"����"},
  {L"mma", L"����"},
  {L"mmi", L"����"},
  {L"mmu", L"����"},
  {L"mme", L"����"},
  {L"mmo", L"����"},
  {L"yya", L"����"},
  {L"yyi", L"����"},
  {L"yyu", L"����"},
  {L"yye", L"������"},
  {L"yyo", L"����"},
  {L"rra", L"����"},
  {L"rri", L"����"},
  {L"rru", L"����"},
  {L"rre", L"����"},
  {L"rro", L"����"},
  {L"wwa", L"����"},
  {L"wwi", L"������"},
  {L"wwu", L"����"},
  {L"wwe", L"������"},
  {L"wwo", L"����"},
  {L"gga", L"����"},
  {L"ggi", L"����"},
  {L"ggu", L"����"},
  {L"gge", L"����"},
  {L"ggo", L"����"},
  {L"zza", L"����"},
  {L"zzi", L"����"},
  {L"zzu", L"����"},
  {L"zze", L"����"},
  {L"zzo", L"����"},
  {L"dda", L"����"},
  {L"ddi", L"����"},
  {L"ddu", L"����"},
  {L"dde", L"����"},
  {L"ddo", L"����"},
  {L"bba", L"����"},
  {L"bbi", L"����"},
  {L"bbu", L"����"},
  {L"bbe", L"����"},
  {L"bbo", L"����"},
  {L"ppa", L"����"},
  {L"ppi", L"����"},
  {L"ppu", L"����"},
  {L"ppe", L"����"},
  {L"ppo", L"����"},
  {L"ffa", L"���ӂ�"},
  {L"ffi", L"���ӂ�"},
  {L"ffu", L"����"},
  {L"ffe", L"���ӂ�"},
  {L"ffo", L"���ӂ�"},
  {L"jja", L"������"},
  {L"jji", L"����"},
  {L"jju", L"������"},
  {L"jje", L"������"},
  {L"jjo", L"������"},
  {L"cca", L"����"},
  {L"cci", L"����"},
  {L"ccu", L"����"},
  {L"cce", L"����"},
  {L"cco", L"����"},
  {L"qqa", L"������"},
  {L"qqi", L"������"},
  {L"qqu", L"����"},
  {L"qqe", L"������"},
  {L"qqo", L"������"},
  {L"vva", L"������"},
  {L"vvi", L"������"},
  {L"vvu", L"����"},
  {L"vve", L"������"},
  {L"vvo", L"������"},
  {L"kkya", L"������"},
  {L"kkyi", L"������"},
  {L"kkyu", L"������"},
  {L"kkye", L"������"},
  {L"kkyo", L"������"},
  {L"ggya", L"������"},
  {L"ggyi", L"������"},
  {L"ggyu", L"������"},
  {L"ggye", L"������"},
  {L"ggyo", L"������"},
  {L"ssya", L"������"},
  {L"ssyi", L"������"},
  {L"ssyu", L"������"},
  {L"ssye", L"������"},
  {L"ssyo", L"������"},
  {L"zzya", L"������"},
  {L"zzyi", L"������"},
  {L"zzyu", L"������"},
  {L"zzye", L"������"},
  {L"zzyo", L"������"},
  {L"ttya", L"������"},
  {L"ttyi", L"������"},
  {L"ttyu", L"������"},
  {L"ttye", L"������"},
  {L"ttyo", L"������"},
  {L"ddya", L"������"},
  {L"ddyi", L"������"},
  {L"ddyu", L"������"},
  {L"ddye", L"������"},
  {L"ddyo", L"������"},
  {L"hhya", L"���Ђ�"},
  {L"hhyi", L"���Ђ�"},
  {L"hhyu", L"���Ђ�"},
  {L"hhye", L"���Ђ�"},
  {L"hhyo", L"���Ђ�"},
  {L"bbya", L"���т�"},
  {L"bbyi", L"���т�"},
  {L"bbyu", L"���т�"},
  {L"bbye", L"���т�"},
  {L"bbyo", L"���т�"},
  {L"ppya", L"���҂�"},
  {L"ppyi", L"���҂�"},
  {L"ppyu", L"���҂�"},
  {L"ppye", L"���҂�"},
  {L"ppyo", L"���҂�"},
  {L"mmya", L"���݂�"},
  {L"mmyi", L"���݂�"},
  {L"mmyu", L"���݂�"},
  {L"mmye", L"���݂�"},
  {L"mmyo", L"���݂�"},
  {L"rrya", L"�����"},
  {L"rryi", L"���股"},
  {L"rryu", L"�����"},
  {L"rrye", L"���肥"},
  {L"rryo", L"�����"},
  {L"jjya", L"������"},
  {L"jjyi", L"������"},
  {L"jjyu", L"������"},
  {L"jjye", L"������"},
  {L"jjyo", L"������"},
  {L"vvya", L"������"},
  {L"vvyi", L"������"},
  {L"vvyu", L"������"},
  {L"vvye", L"������"},
  {L"vvyo", L"������"},
  {L"qqya", L"������"},
  {L"qqyi", L"������"},
  {L"qqyu", L"������"},
  {L"qqye", L"������"},
  {L"qqyo", L"������"},
  {L"ccya", L"������"},
  {L"ccyi", L"������"},
  {L"ccyu", L"������"},
  {L"ccye", L"������"},
  {L"ccyo", L"������"},
  {L"ffya", L"���ӂ�"},
  {L"ffyi", L"���ӂ�"},
  {L"ffyu", L"���ӂ�"},
  {L"ffye", L"���ӂ�"},
  {L"ffyo", L"���ӂ�"},
  {L"ssha", L"������"},
  {L"sshi", L"����"},
  {L"sshu", L"������"},
  {L"sshe", L"������"},
  {L"ssho", L"������"},
  {L"ccha", L"������"},
  {L"cchi", L"����"},
  {L"cchu", L"������"},
  {L"cche", L"������"},
  {L"ccho", L"������"},
  {L"ttha", L"���Ă�"},
  {L"tthi", L"���Ă�"},
  {L"tthu", L"���Ă�"},
  {L"tthe", L"���Ă�"},
  {L"ttho", L"���Ă�"},
  {L"ddha", L"���ł�"},
  {L"ddhi", L"���ł�"},
  {L"ddhu", L"���ł�"},
  {L"ddhe", L"���ł�"},
  {L"ddho", L"���ł�"},
  {L"wwha", L"������"},
  {L"wwhi", L"������"},
  {L"wwhu", L"����"},
  {L"wwhe", L"������"},
  {L"wwho", L"������"},
  {L"kkwa", L"������"},
  {L"kkwi", L"������"},
  {L"kkwu", L"������"},
  {L"kkwe", L"������"},
  {L"kkwo", L"������"},
  {L"qqwa", L"������"},
  {L"qqwi", L"������"},
  {L"qqwu", L"������"},
  {L"qqwe", L"������"},
  {L"qqwo", L"������"},
  {L"ggwa", L"������"},
  {L"ggwi", L"������"},
  {L"ggwu", L"������"},
  {L"ggwe", L"������"},
  {L"ggwo", L"������"},
  {L"sswa", L"������"},
  {L"sswi", L"������"},
  {L"sswu", L"������"},
  {L"sswe", L"������"},
  {L"sswo", L"������"},
  {L"ttwa", L"���Ƃ�"},
  {L"ttwi", L"���Ƃ�"},
  {L"ttwu", L"���Ƃ�"},
  {L"ttwe", L"���Ƃ�"},
  {L"ttwo", L"���Ƃ�"},
  {L"ddwa", L"���ǂ�"},
  {L"ddwi", L"���ǂ�"},
  {L"ddwu", L"���ǂ�"},
  {L"ddwe", L"���ǂ�"},
  {L"ddwo", L"���ǂ�"},
  {L"ffwa", L"���ӂ�"},
  {L"ffwi", L"���ӂ�"},
  {L"ffwu", L"���ӂ�"},
  {L"ffwe", L"���ӂ�"},
  {L"ffwo", L"���ӂ�"},
  {L"ttsa", L"����"},
  {L"ttsi", L"����"},
  {L"ttsu", L"����"},
  {L"ttse", L"����"},
  {L"ttso", L"����"},
  {L"lla", L"����"},
  {L"lli", L"����"},
  {L"llu", L"����"},
  {L"lle", L"����"},
  {L"llo", L"����"},
  {L"llya", L"����"},
  {L"llyi", L"����"},
  {L"llyu", L"����"},
  {L"llye", L"����"},
  {L"llyo", L"����"},
  {L"xxa", L"����"},
  {L"xxi", L"����"},
  {L"xxu", L"����"},
  {L"xxe", L"����"},
  {L"xxo", L"����"},
  {L"xxya", L"����"},
  {L"xxyi", L"����"},
  {L"xxyu", L"����"},
  {L"xxye", L"����"},
  {L"xxyo", L"����"},
  {L"llwa", L"����"},
  {L"xxwa", L"����"},
  {L"llka", L"����"},
  {L"xxka", L"����"},
  {L"llke", L"����"},
  {L"xxke", L"����"},
  {L"lltu", L"����"},
  {L"lltsu", L"����"},
  {L"xxtu", L"����"},
  {L"xxn", L"����"},
};

static KEYVALUE kana2type_table[] = {
  {L"��", L"3"},
  {L"��", L"e"},
  {L"��", L"4"},
  {L"��", L"5"},
  {L"��", L"6"},
  {L"��", L"t"},
  {L"��", L"g"},
  {L"��", L"h"},
  {L"��", L":"},
  {L"��", L"b"},
  {L"��", L"x"},
  {L"��", L"d"},
  {L"��", L"r"},
  {L"��", L"p"},
  {L"��", L"c"},
  {L"��", L"q"},
  {L"��", L"a"},
  {L"��", L"z"},
  {L"��", L"w"},
  {L"��", L"s"},
  {L"��", L"u"},
  {L"��", L"i"},
  {L"��", L"1"},
  {L"��", L","},
  {L"��", L"k"},
  {L"��", L"f"},
  {L"��", L"v"},
  {L"��", L"2"},
  {L"��", L"^"},
  {L"��", L"-"},
  {L"��", L"j"},
  {L"��", L"n"},
  {L"��", L"]"},
  {L"��", L"/"},
  {L"��", L"m"},
  {L"��", L"7"},
  {L"��", L"8"},
  {L"��", L"9"},
  {L"��", L"o"},
  {L"��", L"l"},
  {L"��", L"."},
  {L"��", L";"},
  {L"��", L"\\"},
  {L"��", L"0"},
  {L"��", L"0"},
  {L"��", L"y"},
  {L"��", L"t@"},
  {L"��", L"g@"},
  {L"��", L"h@"},
  {L"��", L":@"},
  {L"��", L"b@"},
  {L"��", L"x@"},
  {L"��", L"d@"},
  {L"��", L"r@"},
  {L"��", L"p@"},
  {L"��", L"c@"},
  {L"��", L"q@"},
  {L"��", L"a@"},
  {L"��", L"z@"},
  {L"��", L"w@"},
  {L"��", L"s@"},
  {L"��", L"f@"},
  {L"��", L"v@"},
  {L"��", L"2@"},
  {L"��", L"^@"},
  {L"��", L"-@"},
  {L"��", L"f["},
  {L"��", L"v["},
  {L"��", L"2["},
  {L"��", L"^["},
  {L"��", L"-["},
  {L"��", L"#"},
  {L"��", L"E"},
  {L"��", L"$"},
  {L"��", L"%"},
  {L"��", L"&"},
  {L"��", L"0"},
  {L"��", L"Z"},
  {L"�J", L"@"},
  {L"�K", L"["},
  {L"�[", L"-"},
};

//////////////////////////////////////////////////////////////////////////////

std::wstring hiragana_to_roman(std::wstring hiragana) {
  FOOTMARK();
  std::wstring roman;
  bool found;
  for (size_t k = 0; k < hiragana.size();) {
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

std::wstring roman_to_hiragana(std::wstring roman) {
  FOOTMARK();
  std::wstring hiragana, str;
  for (size_t k = 0; k < roman.size();) {
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
    if (!found) hiragana += roman[k++];
  }
  return hiragana;
} // roman_to_hiragana

std::wstring roman_to_katakana(std::wstring roman) {
  FOOTMARK();
  std::wstring katakana, str;
  for (size_t k = 0; k < roman.size();) {
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
    if (!found) katakana += roman[k++];
  }
  return katakana;
} // roman_to_katakana

std::wstring roman_to_halfwidth_katakana(std::wstring roman) {
  FOOTMARK();
  std::wstring katakana, str;
  //DebugPrintW(L"roman_to_halfwidth_katakana(%s)\n", roman.c_str());
  for (size_t k = 0; k < roman.size();) {
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

std::wstring roman_to_halfwidth_katakana(std::wstring roman, size_t ichTarget) {
  FOOTMARK();
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

std::wstring hiragana_to_typing(std::wstring hiragana) {
  FOOTMARK();
  std::wstring typing;
  for (size_t k = 0; k < hiragana.size();) {
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

WCHAR vkey_to_hiragana(BYTE vk, BOOL bShift) {
  FOOTMARK();
  switch (vk) {
  case VK_A:          return L'��';
  case VK_B:          return L'��';
  case VK_C:          return L'��';
  case VK_D:          return L'��';
  case VK_E:          return (bShift ? L'��' : L'��');
  case VK_F:          return L'��';
  case VK_G:          return L'��';
  case VK_H:          return L'��';
  case VK_I:          return L'��';
  case VK_J:          return L'��';
  case VK_K:          return L'��';
  case VK_L:          return L'��';
  case VK_M:          return L'��';
  case VK_N:          return L'��';
  case VK_O:          return L'��';
  case VK_P:          return L'��';
  case VK_Q:          return L'��';
  case VK_R:          return L'��';
  case VK_S:          return L'��';
  case VK_T:          return L'��';
  case VK_U:          return L'��';
  case VK_V:          return L'��';
  case VK_W:          return L'��';
  case VK_X:          return L'��';
  case VK_Y:          return L'��';
  case VK_Z:          return (bShift ? L'��' : L'��');
  case VK_0:          return (bShift ? L'��' : L'��');
  case VK_1:          return L'��';
  case VK_2:          return L'��';
  case VK_3:          return (bShift ? L'��' : L'��');
  case VK_4:          return (bShift ? L'��' : L'��');
  case VK_5:          return (bShift ? L'��' : L'��');
  case VK_6:          return (bShift ? L'��' : L'��');
  case VK_7:          return (bShift ? L'��' : L'��');
  case VK_8:          return (bShift ? L'��' : L'��');
  case VK_9:          return (bShift ? L'��' : L'��');
  case VK_OEM_PLUS:   return L'��';
  case VK_OEM_MINUS:  return L'��';
  case VK_OEM_COMMA:  return (bShift ? L'�A' : L'��');
  case VK_OEM_PERIOD: return (bShift ? L'�B' : L'��');
  case VK_OEM_1:      return L'��';
  case VK_OEM_2:      return (bShift ? L'�E' : L'��');
  case VK_OEM_3:      return L'�J';
  case VK_OEM_4:      return (bShift ? L'�u' : L'�K');
  case VK_OEM_5:      return L'�[';
  case VK_OEM_6:      return (bShift ? L'�v' : L'��');
  case VK_OEM_7:      return L'��';
  case VK_OEM_102:    return L'��';
  default:            return 0;
  }
} // vkey_to_hiragana

WCHAR typing_key_to_char(BYTE vk, BOOL bShift, BOOL bCapsLock) {
  FOOTMARK();
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

BOOL is_period(WCHAR ch) {
  return ch == L'�B' || ch == L'�D' || ch == L'.' || ch == L'�';
}

BOOL is_comma(WCHAR ch) {
  return ch == L'�A' || ch == L'�C' || ch == L',' || ch == L'�';
}

WCHAR get_period(void) {
  return L'�B';
}

WCHAR get_comma(void) {
  return L'�A';
}

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

BOOL is_halfwidth_katakana(WCHAR ch) {
  if (0xFF65 <= ch && ch <= 0xFF9F) return TRUE;
  switch (ch) {
  case 0xFF61: case 0xFF62: case 0xFF63: case 0xFF64:
    return TRUE;
  default:
    return FALSE;
  }
}

BOOL is_kanji(WCHAR ch) {
  if (0x4E00 <= ch && ch <= 0x9FFF) return TRUE;
  if (0xF900 <= ch && ch <= 0xFAFF) return TRUE;
  return FALSE;
}

BOOL is_education_kanji(WCHAR ch) {
  return wcschr(L"�ꉹ�J�~���ΉԊL�w�x��ʋ�C���������܌��Z�����O�R�l�q���������Ԏ�\�o��������X�l�����ԐΗ[���쑐��������j�|�������V�c�y������N�����S�{���ؖڕ��E�͗��јZ���H�_�����Ɖ��ĉȉ̉���C�G�O�p�y���Ԋ��ۋL�D�A�|���������ߍ��Z�`���v�������Ìˌ�ߌ�L���H�����l���s�������J���׍ˍ�Z����o�s�~�������F���Ў��T�H�t������H�V�e�S�}��������������ؑD���O�g�������̑��r�n�m�������������ʓ_�X�d�~�������������Ǔ�����n���������ԕ����������ĕ���k���������і���F�j�p�������b�v�����Èӈψ��@�����^�j�w���������׊J�E�K�ي������݋N���q�}�����������Ƌȋǋ����{�N���y�W���������ΌɍK���`�����ՎM�w�n�������d�g�����������ʎҐ̎������K�E�W�I�B�d�Z�h�����������͏�����A�g�i�\�[�^�_�����S�z�������������ő�Αґ��Z�Y�k�����������ǒ����J�S�]�s�x�����o�����������_�g�z�{����������ߔ�@���M�X�\�a�b�i�������������ԕו������ʖ���R���V�L�\�t�z�r�m�l�������Η������H�a���ĈȈʈ݈͈߈�p�h�����݉ۉ��ʉ�B���Q�X�o�e�֊��Ǌ��ϊ��G���I��@��c�~�����������������ɌP�R�S�^�a�i�|�����������Ō���D�q�N�����ō؍ލ��E���D�@�Q�U�Y�c���j���m�i���������؎���j�������Ώď��܏ۏƏȐM�b���Ð��Ȑϐߐܐ���I�󑃑��q�������������ђB�P�u������������I�T�`�k�w�����������ŔM�R�O�s�~���є��K�[�W�t�{�s�v���������ʕӕϕ֖@��]�q������������E�v�{�������ʗǋ����֗ޗߗ���A�V�J�^���ڈ��i�q�c�v�Չt�������������߉͉�������m�i�z��������K���Z�`�t���v�������ϋ֋�Q�o�����������������̌���u�\�z�k���������Ѝ̍ȍۍčݍߍ��G�^�_�x���t�u���}���������ӎɎ��C�q�p�����؏���������E�D�������������ŐӐѐݐڐ��K�c�f�����������������ݑޑԒc�f�z������G�K���������ƔC�R�\�j�ƔŔ�������U�]�n�x�z�w���������ҕٕە�L��h�\�������ȗA�]�a�e�������̖f���و��F�f������D�t�v�g�����Ȋ��Ŋ�������M�^�z�������؋Όh�n�x�����������������ȌČ�F�c�@�g�|�~�������������ύٍ���\�����������p���̎ˎڎ���@���O�A�]�c�k�n�������������Ꮻ����j�m��������������������P�w���t���n������������T�S�a�g�i�l���������������ɓW�}�����͓���F�]�[�h�x�w�o�q�ǔӔ۔�ᕠ���Õ����K��_�Y�S���������͖�D�X�c���~���������՘_�N��", ch) != NULL;
}

BOOL is_common_use_kanji(WCHAR ch) {
  return wcschr(L"���������B���������������ĈÈȈ߈ʈ͈�ˈψЈ׈؈݈шوڈވ̈֜b�ӈ�ۈԈ�܈������������������@���A���B�C�E�F�H�J�S�T���Y�^�_�i�j�p�f�h�c�r�e�s�q�Չu�v�t�w�x�z�y�{�~�����������Q�����������������������������������������������������������񉳉����������������Ή������ԉ����ʉ͉ՉȉˉĉƉ׉؉ى݉Q�߉ŉɉЌC�ǉ̉Ӊ҉ۉ�劢���������D��������������C�E�F�B�G�J�K�򞲉�׉���~�L�O�N�Q�R�U�X�S�W�Y�T�[�_�`�e�p�g�v�i�j�k�s�o�r�u�t�m�l�d�n�w�x�y�z�{�|�����������������������������������Ê��ʊ��̊������Ŋ׊������ъ��������������ԊՊ������������Ǌ֊��ĊɊ��ҊيȊϊ؊͊ӊۊ܊݊�ߊ���������C�����D��F�G�I�O���L�N�Q�S�A���K�T���������M���ʊ���E�P�@�R�Z�X�U�\�`�^�V�Y�[�]�c�e�g�i�l�p�q�r�t�s��v�y�|�u���x�z���P�������}�����{�~�����k�������������������������䋙�����������������������������������������������������ŋƋËȋǋɋʋЋҋϋߋ��ۋ΋Ջ؋͋֋ًыދ݋�������������������@�A�F�J�N�P�M�O�R�S�Q�Z�Y�`�n�a�s�W�^�_�v�b�[�f�k�o�u�h�i�y�X�g�p�w�c�یm�e�x�{�|�}�~�����������������������������������������������������������������������������������������������������������������ȌˌÌČŌҌՌǌʌ̌͌ɌΌٌ֌����ڌ܌݌ߌ�����������H�����E���I�L�b�������@�D�]�l�s�B�F�R�U�X���K�S�m����P�^�c�g�r�x����Z�k�q�v�~���N�T�[���A�Q�`�d�i���a�z�\�j�y�e���t�|�u�w����������������J����������������������������������������������������������������ˍčЍȍэӍɍ͍ʍ̍ύՍ֍׍؍ōٍÍǍ΍ڍۍ�ݍލ܍��ߍ����������|�������D�����A�E�@�B�C�G�M�O�R�Q�V�\�S�Y�P�U�Z�_�^�c�a�b�m�q�x�~���d�j�i�l�s��|�������f�u���g�h�n�o�}�����p�v�w�{�t���������������k�����������������������������������������������������a���������������������������������ŎʎЎԎɎҎˎ͎̎ΎώՎӎ׎֎ڎ؎ގߎݎ��������������������������������B�M�G���@�E�H�L�C���I㵏K�T�A�O�W�D�V�X�R�P�\�`�[�Z�_�d�]�a�e�b�c�f�j�h�i�l�k�m�n�o�q�p�r�t�u�{�����y�}���z�����������������������������@�������������������������я��������������������Ǐˏ̏Ώ����͏Џ׏������ďŏɏϏُ؏ۏ����Əڏ��ᓲ�Տ܏��ʏ���������������������������F�@�H�A�B���G���D�E�J�K�S�\�L�b�c�g�h�N�M�Ð_�O�P�U�Z�^�j�[�a�i�X�f�Q�T�V�R�k�d�e�l�n�m�s�v�r�w�q�t�{�}�����������������������䐏���������������������䐢�������������������Đ������Ȑ������������������������Ð������ŗ[�ːΐԐ̐͐ȐҐǐɐʐӐՐϐѐАؐܐِސڐݐ�ېߐ���������������������D����A�B�F�HⳑK�����J�I�E�@�N�S�O�P�R�T�Q�V�U�_�j�c�d�f�[�e�g�a�i�Y�k�b�o�s�������t���������q�{�}�K���|���]�u���n�r�������m�z�w�������H�����������������������������������������������������������������������������őÑ��đʑ��Α̑ϑґӑّޑёבܑ͑ߑ֑ݑ��ؑԑՑ������������������A�B�E�D�I�N�O�U�S�P�Y�_�T�W�Z�Q�[�]�a�b�c�j�i�f�e�g�k�d�n�r�m�l�p�v�x�s�t�u�k�|�{���~�z�����������������������������������������������������������������ޒ������\���������}���������������������������ǒŒĒʒɒ˒Вؒܒߒ�������@����������������������D�I�J�E�H�K�G�M�R�N�S�O�P�V�T�X�_�W�Y�]�U�c�`�a�d�l�f�i�k�r�s�n�h�q�y�z�w�x�{���~���������������|�������������}�����������������o������������������������������������������������������ēœƓǓȓʓ˓͓ԓؓ��Óݓܘ��ߓޓ������������������������A�C�D�E�F�J�M�N�O�P�S�R�Y�[�\�]�_�Z�c�g�h�j�e�n�k�l�q�t�w�x�o�z�r�s�p�y���{�~�|���}�����������������������������������������������������������ÔƔ��Ĕ������ŔǔȔʔ̔��є��ϔД͔ɔ˔ӔԔؔՔ��ܔ۔�ޔ���ڔ����ߔ����������������@�G�I�C�K��M�P�S�X�\�U�[�]�Y�W�c�b�a�`�L�i�l�n�o�p�q�r�s�v���t�z�}�{�|�����]�������w���x�����~�����������������������������������������������������������������������������Õ��������ݕĕ����ȕʕ̕ЕӕԕϕΕՕҕٕ֕ו��ۚM�ߕ�ܕ���������F�M�������@�A�E�����C���K��I�L�O�J�D�S�R�Z�V�W�Y�h�[�b�^�`�U�a�]�T�X�_�f�e�\�c�d�j�k�ؖp�q�r�l�n�o�v�u�x�{�z�|�}�~�������������������������������������������������������������������������������������ŖƖʖȖ˖Ζ͖іϖӖՖҖԖږٖ�������������ŗR���g���@�A���B�F�L�E�H�I�X�N�P�T�V�Y�U�J�Z�D�^�\�]�_�a�c�p�r�d�m�v�e�f�g�h�t�z�n���lᇗx�q�{�i�w�j�}�����~�����f����������������煗������������������������������������ɗ����������������������������Ǘ����˗ʗ��̗��×ėƗ͗ΗїЗϗ֗ח՗ڗܗݗۗޗߗ���ߗ����������������A�����B�C�F�G�H�I�V�J�M�Y�N�Q�L�O�R�ĘZ�^�[�_�a�b�d�e�f�g�p�r", ch) != NULL;
} // is_common_use_kanji

BOOL is_fullwidth_ascii(WCHAR ch) {
  return (0xFF00 <= ch && ch <= 0xFFEF);
}

BOOL are_all_chars_numeric(const std::wstring& str) {
  FOOTMARK();
  for (size_t i = 0; i < str.size(); ++i) {
    if (L'0' <= str[i] && str[i] <= L'9') {
      ;
    } else if (L'�O' <= str[i] && str[i] <= L'�X') {
      ;
    } else {
      return FALSE;
    }
  }
  return TRUE;
}

static const wchar_t s_szKanjiDigits[] = L"�Z���O�l�ܘZ������";

std::wstring convert_to_kansuuji_1(wchar_t ch, size_t digit_level) {
  FOOTMARK();
  static const wchar_t s_szKanjiDigitLevels[] = L" �\�S��";
  std::wstring ret;
  if (ch == L'0') return ret;
  assert(ch - L'0' < 10);
  ret += s_szKanjiDigits[ch - L'0'];
  if (digit_level > 0) {
    assert(digit_level < 4);
    ret += s_szKanjiDigitLevels[digit_level];
  }
  return ret;
}

std::wstring convert_to_kansuuji_4(const std::wstring& halfwidth) {
  FOOTMARK();
  assert(are_all_chars_numeric(halfwidth));
  assert(halfwidth.size() <= 4);
  const size_t length = halfwidth.size();
  std::wstring ret;
  size_t level = 0;
  for (size_t i = length; i > 0;) {
    --i;
    ret = convert_to_kansuuji_1(halfwidth[i], level) + ret;
    ++level;
  }
  return ret;
}

std::wstring convert_to_kansuuji(const std::wstring& str) {
  FOOTMARK();
  std::wstring halfwidth = lcmap(str, LCMAP_HALFWIDTH);
  assert(are_all_chars_numeric(halfwidth));
  if (halfwidth.size() >= 24) return halfwidth;
  static const wchar_t s_szKanjiGroupLevels[] = L" ����������";
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
  if (ret.empty()) ret = L"��";
  unboost::replace_all(ret, L"��\", L"�\");
  unboost::replace_all(ret, L"��S", L"�S");
  unboost::replace_all(ret, L"���", L"��");
  return ret;
}

std::wstring convert_to_kansuuji_brief(const std::wstring& str) {
  FOOTMARK();
  std::wstring halfwidth = lcmap(str, LCMAP_HALFWIDTH);
  assert(are_all_chars_numeric(halfwidth));
  std::wstring ret;
  for (size_t i = 0; i < halfwidth.size(); ++i) {
    ret += s_szKanjiDigits[halfwidth[i] - L'0'];
  }
  return ret;
}

std::wstring convert_to_kansuuji_formal(const std::wstring& str) {
  std::wstring ret = convert_to_kansuuji(str);
  unboost::replace_all(ret, L"��", L"��");
  unboost::replace_all(ret, L"��", L"��");
  unboost::replace_all(ret, L"�O", L"�Q");
  unboost::replace_all(ret, L"��", L"��");
  unboost::replace_all(ret, L"�\", L"�E");
  unboost::replace_all(ret, L"��", L"��");
  unboost::replace_all(ret, L"��", L"��");
  return ret;
}

WCHAR dakuon_shori(WCHAR ch0, WCHAR ch1) {
  FOOTMARK();
  switch (MAKELONG(ch0, ch1)) {
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'��';
  case MAKELONG(L'��', L'�J'): return L'\u3094';
  case MAKELONG(L'��', L'�K'): return L'��';
  case MAKELONG(L'��', L'�K'): return L'��';
  case MAKELONG(L'��', L'�K'): return L'��';
  case MAKELONG(L'��', L'�K'): return L'��';
  case MAKELONG(L'��', L'�K'): return L'��';
  case MAKELONG(L'�J', L'�J'): return L'�K';
  case MAKELONG(L'�L', L'�J'): return L'�M';
  case MAKELONG(L'�N', L'�J'): return L'�O';
  case MAKELONG(L'�P', L'�J'): return L'�Q';
  case MAKELONG(L'�R', L'�J'): return L'�S';
  case MAKELONG(L'�T', L'�J'): return L'�U';
  case MAKELONG(L'�V', L'�J'): return L'�W';
  case MAKELONG(L'�X', L'�J'): return L'�Y';
  case MAKELONG(L'�Z', L'�J'): return L'�[';
  case MAKELONG(L'�\', L'�J'): return L'�]';
  case MAKELONG(L'�^', L'�J'): return L'�_';
  case MAKELONG(L'�`', L'�J'): return L'�a';
  case MAKELONG(L'�c', L'�J'): return L'�d';
  case MAKELONG(L'�e', L'�J'): return L'�f';
  case MAKELONG(L'�g', L'�J'): return L'�h';
  case MAKELONG(L'�n', L'�J'): return L'�o';
  case MAKELONG(L'�q', L'�J'): return L'�r';
  case MAKELONG(L'�t', L'�J'): return L'�u';
  case MAKELONG(L'�w', L'�J'): return L'�x';
  case MAKELONG(L'�z', L'�J'): return L'�{';
  case MAKELONG(L'�E', L'�J'): return L'��';
  case MAKELONG(L'�n', L'�K'): return L'�p';
  case MAKELONG(L'�q', L'�K'): return L'�s';
  case MAKELONG(L'�t', L'�K'): return L'�v';
  case MAKELONG(L'�w', L'�K'): return L'�y';
  case MAKELONG(L'�z', L'�K'): return L'�|';
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

std::wstring fullwidth_ascii_to_halfwidth(const std::wstring& str) {
  std::wstring ret;
  const size_t count = str.size();
  wchar_t ch;
  for (size_t i = 0; i < count; ++i) {
    ch = str[i];
    if (L'��' <= ch && ch <= L'��') {
      ch += L'a' - L'��';
    } else if (L'�`' <= ch && ch <= L'�y') {
      ch += L'A' - L'�`';
    }
    ret += ch;
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
