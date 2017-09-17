// katsuyou.cpp
// (Japanese, Shift_JIS)
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <map>
using namespace std;

#define UNBOOST_USE_STRING_ALGORITHM
#include "../unboost.hpp"

enum HINSHI_BUNRUI {
  HB_START_NODE,        // ŠJnƒm[ƒh
  HB_MEISHI,            // –¼Œ
  HB_IKEIYOUSHI,        // ‚¢Œ`—eŒ
  HB_NAKEIYOUSHI,       // ‚ÈŒ`—eŒ
  HB_RENTAISHI,         // ˜A‘ÌŒ
  HB_FUKUSHI,           // •›Œ
  HB_SETSUZOKUSHI,      // Ú‘±Œ
  HB_KANDOUSHI,         // Š´“®Œ
  HB_JOSHI,             // •Œ
  HB_MIZEN_JODOUSHI,    // –¢‘R•“®Œ
  HB_RENYOU_JODOUSHI,   // ˜A—p•“®Œ
  HB_SHUUSHI_JODOUSHI,  // I~•“®Œ
  HB_RENTAI_JODOUSHI,   // ˜A‘Ì•“®Œ
  HB_KATEI_JODOUSHI,    // ‰¼’è•“®Œ
  HB_MEIREI_JODOUSHI,   // –½—ß•“®Œ
  HB_GODAN_DOUSHI,      // ŒÜ’i“®Œ
  HB_ICHIDAN_DOUSHI,    // ˆê’i“®Œ
  HB_KAHEN_DOUSHI,      // ƒJ•Ï“®Œ
  HB_SAHEN_DOUSHI,      // ƒT•Ï“®Œ
  HB_KANGO,             // Š¿Œê
  HB_SETTOUJI,          // Ú“ª«
  HB_SETSUBIJI,         // Ú”ö«
  HB_END_NODE           // I—¹ƒm[ƒh
};

struct DICT_ENTRY {
  std::wstring  pre;
  HINSHI_BUNRUI bunrui;
  std::wstring  post;
  std::wstring  tags;
};

std::vector<DICT_ENTRY> g_entries;
std::map<wchar_t,wchar_t> g_vowel_map;
std::map<wchar_t,wchar_t> g_consonant_map;

const wchar_t g_table[][5] = {
  {L'‚ ', L'‚¢', L'‚¤', L'‚¦', L'‚¨'},
  {L'‚©', L'‚«', L'‚­', L'‚¯', L'‚±'},
  {L'‚ª', L'‚¬', L'‚®', L'‚°', L'‚²'},
  {L'‚³', L'‚µ', L'‚·', L'‚¹', L'‚»'},
  {L'‚´', L'‚¶', L'‚¸', L'‚º', L'‚¼'},
  {L'‚½', L'‚¿', L'‚Â', L'‚Ä', L'‚Æ'},
  {L'‚¾', L'‚À', L'‚Ã', L'‚Å', L'‚Ç'},
  {L'‚È', L'‚É', L'‚Ê', L'‚Ë', L'‚Ì'},
  {L'‚Í', L'‚Ğ', L'‚Ó', L'‚Ö', L'‚Ù'},
  {L'‚Î', L'‚Ñ', L'‚Ô', L'‚×', L'‚Ú'},
  {L'‚Ï', L'‚Ò', L'‚Õ', L'‚Ø', L'‚Û'},
  {L'‚Ü', L'‚İ', L'‚Ş', L'‚ß', L'‚à'},
  {L'‚â', 0, L'‚ä', 0, L'‚æ'},
  {L'‚ç', L'‚è', L'‚é', L'‚ê', L'‚ë'},
  {L'‚í', 0, 0, 0, L'‚ğ'},
  {L'‚ñ', 0, 0, 0, 0},
};

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

BOOL is_kanji(WCHAR ch) {
  if (0x4E00 <= ch && ch <= 0x9FFF) return TRUE;
  if (0xF900 <= ch && ch <= 0xFAFF) return TRUE;
  return FALSE;
}

BOOL is_education_kanji(WCHAR ch) {
  return wcschr(L"ˆê‰¹‰J‰~‰¤‰Î‰ÔŠLŠw‹x‹ã‹Ê‹ó‹C‰ºŒŒ©Œ¢ŒÜŒûZ‹à¶ORlq…š¨µÔè\o—¬³ãXl…¶ÂÔÎ—[çæì‘‘‘«‘º‘å’j’|’’†’¬“V“c“y“ñ“ú“ü”N”’”ª•S–{–¼–Ø–Ú•¶‰E—Í—§—Ñ˜Zˆø‰H‰_‰“‰€‰Æ‰½‰Ä‰È‰Ì‰æ‰ï‰ñŠCŠGŠOŠpŠyŠˆŠÔŠçŠâŠÛ‹L‹D‹A‹|‹‹›‹³‹­‹ß¡ŒZŒ`‹ŒvŒ´Œ¾Œ³ŒÃŒËŒêŒßŒãLŒğHŒõŒöl‰©s‚‡‘’J•×ËìZ†–îos~›©FºĞãñTHt‘­êHVeS}“ª”¼¯º°áØ‘Dü‘O‘g‘–‘¾‘½‘Ì‘ä’í’r’n’m’ƒ’‹’·’©’¹’¼’Ê“_“X“d“~““–“š“Œ“¹“¯“Ç“à“ì“÷”n”ƒ”„””¼”Ô•ƒ•à•—•ª•·•Ä•ê•û–k–ˆ–…–œ–Â–¾–Ñ–å–ì–é—F—j—p—ˆ—¢—˜bvˆ«ˆÀˆÃˆÓˆÏˆãˆç‰@ˆõˆù‰^‰j‰w‰¡‰›‰®‰·‰×ŠJŠEŠKŠÙŠ´Š¦Š¿Šİ‹NŠú‹q‹}‹‰‹…‹†‹‹´‹Æ‹È‹Ç‹â‹ê‹æ‹ï‹{ŒN‰»ŒyŒWŒŒŒˆŒ§Œ¤ŒÎŒÉKŒü`†ªÕMwn•€dgŸ–®ÀÊÒÌğçåæóKEWIBdZh‹Š•ÁºÍ¤ŸæAgi\[^_¢®‘S‘z‘Š‘—‘§‘¬‘°‘¼‘Å‘ã‘Î‘Ò‘æ‘è’Z’Y’k’…’’Œ’²’ ’Ç’š’è’ë“J“S“]“s“x“‡“™“o“’“Š“¤“®“¶”_”g”z”{” ”¨”­”½”Ââ”ß”ç•@”ü•M•X•\•a•b•i•‰•”•Ÿ••¨•½•Ô•×•ú–¡–½–Ê–â–ò–ğ—R–û—V—L—\—t—z—r—m—l——·—¼—Î—¬—ç—ñ—û˜H˜aˆ¤ˆÄˆÈˆÊˆİˆÍˆßˆó‰p‰h‰–‰­‰İ‰Û‰Á‰Ê‰èŠB‰üŠQŠXŠoŠeŠÖŠ®ŠÇŠ¯ŠÏŠèŠì‹GŠø‹IŠó‹@Ší‹c‹~‹ƒ‹‹‹‹“‹¦‹¾‹£‹¤‹ÉŒPŒRŒSŒ^ŒaŒiŒ|Œ‹Œ‡ŒšŒ’Œ±ŒÅŒóŒ÷DqN·ÅØŞğEüD@QUYcjmi¡™«¸Øíüj‡‰¥ÎÄ¼ÜÛÆÈMb´Ã¬ÈÏßÜàí‘Ió‘ƒ‘ˆ‘q‘©‘¤‘±‘²‘·‘à‘Ñ’B’P’u’‡’™’›’°’â’á’ê“I“T“`“k“w“”“­“°“Á“¾“Å”M‘R”O”s”~””Ñ”ò”ï•K•[•W•t•{•s•v–³•›•²•º•Ê•Ó•Ï•Ö–@•ï–]–q––––¢–¬–¯–ñ—E—v—{——˜—¤—Ê—Ç‹™—¿—Ö—Ş—ß—â—á—ğ˜A˜V˜J˜^ˆ³ˆÚˆö‰i‰q‰c‰vˆÕ‰t‰‰÷‰‰‰¶‰¿‰ß‰Â‰Í‰¼‰ê‰ğ‰õŠmŠiŠzŠµŠ²Š§Šá‹KŠîŠñ‹Z‹`‹t‹Œ‹v‹–‹‹«‹Ï‹Ö‹åŒQŒoŒ‰Œ¯ŒŸŒ”ŒŒ»ŒÀŒ¸ŒÂŒÌŒìŒøu\zkŒú‹»¬¸ĞÌÈÛÄİßàG^_x‘tu”}¦—¯¿ÓÉöCqp€˜Ø³µíóğîED»¸«­§¨ÅÓÑİÚâã‘K‘c‘f‘‘‘œ‘¢‘¥‘ª‘®‘¹‘İ‘Ş‘Ô’c’f’z’£’ö’ñ“G“K““º“±“¿“Æ”C”R”\”j”Æ”Å”»”ì”ä”ñ”õ•U•]•n•x•z•w••œ•¡•§•Ò•Ù•Û•æ–L•ñ–h–\–²–±–À–È—A—]—a—e—¦—ª—¯—Ì–f—¥ˆÙˆâˆæ‰F‰f‰ˆ‰„‰äŠDŠtŠvŠgŠ„Š”ŠÈŠ±ŠÅŠªŠëŠöŠ÷‹M‹^‹z‹½‹¹‹Ÿ‹Ø‹ÎŒhŒnŒxŒ€ŒƒŒŠŒ¦Œ›Œ ŒµŒ¹ŒÈŒÄŒëFc@g|~’œ¢»ÀÏÙôû\‹„ŒŠp¥ÌËÚá÷@ûOA]cknƒˆ”œá«éöjm„‚¡½¹·êéõòô‘P‘w‘•‘t‘€‘n‘‹‘ ‘Ÿ‘¸‘¶‘î’T’S’a’g’i’l’ˆ’‰’˜’ª’¸’¡’À’É“W“}“¢“œ“Í“ï“û”F”]”[”h”x”w”o”q”Ç”Ó”Û”é”á• •±•Ã•À•Â•â•é–K•ó–_–Y–S–‡–‹–§–¿–Í–ó—D—X—c—‚—~—‘———— —Õ˜_˜N•Ğ", ch) != NULL;
}

BOOL is_common_use_kanji(WCHAR ch) {
  return wcschr(L"ˆŸˆ£ˆ¥ˆ¤Bˆ«ˆ¬ˆ³ˆµˆ¶—’ˆÀˆÄˆÃˆÈˆßˆÊˆÍˆãˆËˆÏˆĞˆ×ˆØˆİˆÑˆÙˆÚˆŞˆÌˆÖœbˆÓˆáˆÛˆÔˆâˆÜˆæˆçˆêˆëˆíˆïˆğˆøˆóˆöˆôˆ÷ˆõ‰@ˆú‰Aˆù‰B‰C‰E‰F‰H‰J‰SŸT¤‰Y‰^‰_‰i‰j‰p‰f‰h‰c‰r‰e‰s‰qˆÕ‰u‰v‰t‰w‰x‰z‰y‰{‰~‰„‰ˆ‰Š‰…‰ƒ•Q‰‡‰€‰Œ‰‰“‰”‰–‰‰‰‰‰˜‰¤‰š‰›‰‰‰Ÿ‰ ‰¢‰£÷‰¥‰œ‰¡‰ª‰®‰­‰¯‰°‹ñ‰³‰´‰µ‰¹‰¶‰·‰¸‰º‰»‰Î‰Á‰Â‰¼‰½‰Ô‰À‰¿‰Ê‰Í‰Õ‰È‰Ë‰Ä‰Æ‰×‰Ø‰Ù‰İ‰Q‰ß‰Å‰É‰ĞŒC‰Ç‰Ì‰Ó‰Ò‰Û‰á‰åŠ¢‰ä‰æ‰è‰ê‰ë‰ì‰î‰ñŠD‰ï‰õ‰ú‰ü‰ö‰û‰÷ŠCŠEŠFŠBŠGŠJŠK‰ò²‰ğ’×‰ó‰ùæ~ŠLŠOŠNŠQŠRŠUŠXŠSŠWŠYŠTŠ[Š_Š`ŠeŠpŠgŠvŠiŠjŠkŠsŠoŠrŠuŠtŠmŠlŠdŠnŠwŠxŠyŠzŠ{Š|ŠƒŠ‡ŠˆŠ…Š‰Š„Š‹ŠŠŠŒŠŠŠ”Š˜Š™Š Š±Š§ŠÃŠ¾ŠÊŠ®ŠÌŠ¯Š¥ŠªŠÅŠ×Š£Š¨Š³ŠÑŠ¦Š«Š¬Š·Š¸Š»Š¼ŠÔŠÕŠ©Š°Š²Š´Š¿ŠµŠÇŠÖŠ½ŠÄŠÉŠ¶ŠÒŠÙŠÂŠÈŠÏŠØŠÍŠÓŠÛŠÜŠİŠâŠßŠáŠæŠçŠèŠéŠêŠëŠ÷‹CŠòŠóŠõ‹DŠï‹F‹G‹I‹OŠù‹L‹N‹Q‹S‹AŠîŠñ‹K‹TŠìŠôŠöŠúŠû‹MŠüšÊŠøŠí‹E‹P‹@‹R‹Z‹X‹U‹\‹`‹^‹V‹Y‹[‹]‹c‹e‹g‹i‹l‹p‹q‹r‹t‹s‹ã‹v‹y‹|‹u‹Œ‹x‹z‹€‰P‹‹†‹ƒ‹}‹‰‹Š‹{‹~‹…‹‹šk‹‡‹‹‹‹‹‘‹’‹“‹•‹–‹—‹›Œä‹™‹¥‹¤‹©‹¶‹‹‹Ÿ‹¦‹µ‹¬‹²‹·‹°‹±‹¹‹º‹­‹³‹½‹«‹´‹¸‹¾‹£‹¿‹Á‹Â‹Å‹Æ‹Ã‹È‹Ç‹É‹Ê‹Ğ‹Ò‹Ï‹ß‹à‹Û‹Î‹Õ‹Ø‹Í‹Ö‹Ù‹Ñ‹Ş‹İ‹á‹â‹æ‹å‹ê‹ì‹ïœœ‹ğ‹ó‹ô‹ö‹÷‹ø‹üŒ@ŒAŒFŒJŒNŒPŒMŒOŒRŒSŒQŒZŒYŒ`ŒnŒaŒsŒWŒ^Œ_ŒvŒbŒ[ŒfŒkŒoŒuŒhŒiŒyŒXŒgŒpŒwŒcœÛŒmŒeŒxŒ{Œ|Œ}Œ~Œ„Œ€Œ‚ŒƒŒ…Œ‡ŒŠŒŒŒˆŒ‹Œ†Œ‰ŒŒ¢ŒŒ©Œ”Œ¨ŒšŒ¤Œ§ŒŒ“Œ•ŒŒ¬Œ’Œ¯Œ—Œ˜ŒŸŒ™Œ£Œ¦Œ­Œ Œ›Œ«ŒªŒ®–šŒ°Œ±ŒœŒ³Œ¶ŒºŒ¾Œ·ŒÀŒ´Œ»Œ½Œ¸Œ¹ŒµŒÈŒËŒÃŒÄŒÅŒÒŒÕŒÇŒÊŒÌŒÍŒÂŒÉŒÎŒÙŒÖŒÛçüŒÚŒÜŒİŒßŒàŒãŒâŒåŒéŒêŒëŒìŒûHŒöŒùEŒ÷ILbŒğŒõŒü@D]lsBFRUXŒøKSmŒòŒúP^cgrxŒóZkqv~‚NT[‰©AQ`di€az\jye‹»t|uwŒî†‡‰„˜ü‹’J‘•’“–œ‹î ¡¢©¦ª¥¬­®°¤§¶²¹¸»´·¼½ÀÁËÄĞÈÑÓÉÍÊÌÏÕÖ×ØÅÙÂÃÇÎÚÛéİŞÜàßèìíğòõô|ïöçûDü™‹AE@BCGMORQV\SYPUZ_^cabmqx~djils–î|€…Šfu„ghno}ƒˆpvw{tœ“†‰‹‡Œ•k‘”“•’¦š›Ÿ¨©—™–˜¡ œ«¥‰a£­®¯²µ¶¸º¾·¼¹½¿ÀÅÊĞÔÉÒËÌÍÎÏÕÓ×ÖÚØŞßİáãâèåçéæëñêìğîíïõóôöùò÷ûúBMGü@EHLC‘³IãµKTAOWDVXRP\`[Z_d]aebcfjhilkmnoqprtu{„‚y}ƒz‡€…ˆ‰Š‘‹”—”@•˜–™œ¬¡­¢ °´Ñ®µ³¸¼Àºª«ÁÇËÌÎ¥¤ÂÍĞ×Ÿ¶»ÄÅÉÏÙØÛ§ÆÚ²á“²ÕÜÊàãäçğóæéòèíîêôö“êëìù÷øF@HABüGúDEJKS\LbcghNM’Ã_OPUZ^j[aiXfQTVRkdelnmsvrwqt{}…‚†ƒˆŠ„Œ‹‡•ä‘•’”˜™¡£¥ˆä¢³¶¬¼º§©ª«ÂÄ­¯µÈ¦À´·–¹°¨¹½¸»¾Ã¿®ÁÅ—[ËÎÔÌÍÈÒÇÉÊÓÕÏÑĞØÜÙŞÚİáÛßàãâçìåèæéêòóôõîğù‘Dí÷‘A‘B‘F‘Hâ³‘Köü‘J‘I‘E‘@‘N‘S‘O‘P‘R‘T‘Q‘V‘U‘_‘j‘c‘d‘f‘[‘e‘g‘a‘i‘Y‘k‘b‘o‘s‘‘ˆ‘–‘t‘Š‘‘‘‘—‘q‘{‘}ŒK‘ƒ‘|‘‚‘]‘u‘‹‘n‘r‘‰‘’‘•‘m‘z‘w‘‘˜‘…çH‘€‘‡‘š‘›‘”‘¢‘œ‘‘‘ ‘¡‘Ÿ‘¦‘©‘«‘£‘¥‘§‘¨‘¬‘¤‘ª‘­‘°‘®‘¯‘±‘²—¦‘¶‘º‘·‘¸‘¹‘»‘¼‘½‘¿‘Å‘Ã‘Á‘Â‘Ä‘Ê‘¾‘Î‘Ì‘Ï‘Ò‘Ó‘Ù‘Ş‘Ñ‘×‘Í‘Ü‘ß‘Ö‘İ‘à‘Ø‘Ô‘Õ‘å‘ã‘ä‘æ‘è‘ê‘î‘ğ‘ò‘ì‘ñ‘õ‘ó‘ø‘÷’A’B’E’D’I’N’O’U’S’P’Y’_’T’W’Z’Q’[’]’a’b’c’j’i’f’e’g’k’d’n’r’m’l’p’v’x’s’t’uãk’|’{’€’~’z’’‚’ƒ’…’„’†’‡’‰«’ˆ’‰’Š’’‹’Œ’’‘’’’“’˜’™’š’¢’¡’›’¬’·’§’ ’£’¤’­’Ş’¸’¹’©“\’´’°’µ’¥š}’ªŸ’²’®’¦’¼’º’»’¾’¿’½’Â’À’Á’Ç’Å’Ä’Ê’É’Ë’Ğ’Ø’Ü’ß’á’æ’ì’í’è’ê’ï“@’à’å’é’ù’ë’ü’â’ã’ç’ñ’ö’ø’÷’ú“D“I“J“E“H“K“G“M“R“N“S“O“P“V“T“X“_“W“Y“]“U“c“`“a“d“l“f“i“k“r“s“n“h“q“y“z“w“x“{““~“”“–“Š“¤“Œ““¦“|“€“‚“‡““¢“§“}“‰““©“ƒ“‹““’“—“o“š“™“›“ˆî“¥“œ“ª“£“¡“¬“«“¯“´“·“®“°“¶“¹“­“º“±“µ“»“½“Á“¾“Â“¿“Ä“Å“Æ“Ç“È“Ê“Ë“Í“Ô“Ø“ÚæÃ“İ“Ü˜¥“ß“Ş“à—œ“ä“ç“ì“î“ï“ñ“ò“ó“õ“÷“ø“ú“ü“û”A”C”D”E”F”J”M”N”O”P”S”R”Y”[”\”]”_”Z”c”g”h”j”e”n”k”l”q”t”w”x”o”z”r”s”p”y”„”{”~”|”†”}”ƒ”…”’”Œ””‘”—””•””–””™”›”š” ”¢”¨”§”ª”«”­”¯”°”²”±”´”½”¼”Ã”Æ”¿”Ä”º”»âã”Â”Å”Ç”È”Ê”Ì”Á”Ñ”À”Ï”Ğ”Í”É”Ë”Ó”Ô”Ø”Õ”ä”ç”Ü”Û”á”Ş”â”ì”ñ”Ú”ò”æ”é”í”ß”à”ï”è”ë”ğ”ö”û”ü”õ”÷•@•G•I•C•K”å•M•P•S•X•\•U•[•]•Y•W•c•b•a•`”L•i•l•n•o•p•q•r•s•v•ƒ•t•z•}•{•|•Œ•æ]•‰•‹•‚•w•„•x••…•~•†•Š•ˆ•••”•‘•••—•š••›••œ•Ÿ• •¡•¢•¥•¦•§•¨•²•´•µ•¬•­•®•±•ª•¶•·•¸•½•º•¹•À•¿•Ã•Â•»•¼•¾•Á–İ•Ä•Çàø•È•Ê•Ì•Ğ•Ó•Ô•Ï•Î•Õ•Ò•Ù•Ö•×•à•ÛšM•ß•â•Ü•ê•å•æ•ç•é•ë•û•ï–F–M•ò•ó•ø•ú–@–A–E•î•í•ô–C•ö–K•ñ–I–L–O–J–D–S–R–Z–V–W–Y–h–[–b–^–`–U–a–]–T–X–_–f–e–\–c–d–j–k–Ø–p–q–r–l–n–o–v–u–x–{–z–|–}–~–ƒ–€––‚–ˆ–…–‡–†–„–‹–Œ––”–––•–œ–––Ÿ–¢–¡–£–¦–§–¨–¬–­–¯–°–µ–±–³–²–¶–º–¼–½–¾–À–»–¿–Á–Â–Å–Æ–Ê–È–Ë–Î–Í–Ñ–Ï–Ó–Õ–Ò–Ô–Ú–Ù–å–ä–â–è–é–ì–í–ï–ğ–ñ–ó–ò–ôˆÅ—R–ûšg–ù—@—A–ü—B—F—L—E—H—I—X—N—P—T—V—Y—U—J—Z—D—^—\—]—_—a—c—p—r—d—m—v—e—f—g—h—t—z—n˜—lá‡—x—q—{—i—w—j—}—€——~—‚—ƒf—‡—…—ˆ—‹—Š———ç…——‘———”—•—“—™—˜—¢——Ÿ— —š——£—¤—§—¥œÉ—ª–ö—¬—¯—³—±—²—°—µ—·—¸—¶—¹—¼—Ç—¿—Á—Â—Ë—Ê—»—Ì—¾—Ã—Ä—Æ—Í—Î—Ñ—Ğ—Ï—Ö—×—Õ—Ú—Ü—İ—Û—Ş—ß—ç—â—ã–ß—á—é—ë—ì—ê—î—í—ï—ğ—ñ—ò—ó—ô—ö˜A—õ—û˜B˜C˜F˜G˜H˜I˜V˜J˜M˜Y˜N˜Q˜L˜O˜RâÄ˜Z˜^˜[˜_˜a˜b˜d˜e˜f˜g˜p˜r", ch) != NULL;
} // is_common_use_kanji

std::wstring lcmap(const std::wstring& str, DWORD dwFlags) {
  WCHAR szBuf[1024];
  const LCID langid = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
  ::LCMapStringW(MAKELCID(langid, SORT_DEFAULT), dwFlags,
    str.c_str(), -1, szBuf, 1024);
  return std::wstring(szBuf);
}

inline bool entry_compare(const DICT_ENTRY& e1, const DICT_ENTRY& e2) {
  return (e1.pre < e2.pre);
}

bool do_load(void) {
  char buf[256];
  wchar_t wbuf[256];
  std::wstring str;

  g_entries.reserve(60000);

  FILE *fp = fopen("..\\mzimeja.dic", "rb");
  if (fp == NULL) return false;

  int lineno = 0;
  while (fgets(buf, 256, fp) != NULL) {
    ++lineno;
    if (buf[0] == ';') continue;
    MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, 256);

    std::wstring str = wbuf;

    for (size_t i = 0; i < str.size(); ++i) {
      if (is_kanji(str[i]) && !is_common_use_kanji(str[i])) {
        if (str.find(L"[”ñ•W€]") == std::wstring::npos &&
            str.find(L"[l–¼]") == std::wstring::npos &&
            str.find(L"[‰w–¼]") == std::wstring::npos &&
            str.find(L"[’n–¼]") == std::wstring::npos &&
            str.find(L"[“®A•¨]") == std::wstring::npos)
        {
          //printf("..\\mzimeja.dic (%d): WARNING: non-common-use kanji found\n", lineno);
          break;
        }
      }
    }

    unboost::trim_right_if(str, unboost::is_any_of(L"\r\n"));
    std::vector<std::wstring> fields;
    unboost::split(fields, str, unboost::is_any_of(L"\t"));

    if (fields.empty()) {
      printf("..\\mzimeja.dic (%d): WARNING: empty line\n", lineno);
      continue;
    }

    DICT_ENTRY entry;
    if (fields.size() == 1) {
      entry.post = str;
      entry.bunrui = HB_MEISHI;
      if (is_fullwidth_katakana(str[0])) {
        std::wstring hiragana = lcmap(str, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
        entry.pre = hiragana;
      } else {
        entry.pre = str;
      }
    } else {
      const std::wstring& bunrui_str = fields[1];
      if (bunrui_str == L"–¼Œ")              entry.bunrui = HB_MEISHI;
      else if (bunrui_str == L"‚¢Œ`—eŒ")     entry.bunrui = HB_IKEIYOUSHI;
      else if (bunrui_str == L"‚ÈŒ`—eŒ")     entry.bunrui = HB_NAKEIYOUSHI;
      else if (bunrui_str == L"˜A‘ÌŒ")       entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"•›Œ")         entry.bunrui = HB_FUKUSHI;
      else if (bunrui_str == L"Ú‘±Œ")       entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"Š´“®Œ")       entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"•Œ")         entry.bunrui = HB_JOSHI;
      else if (bunrui_str == L"–¢‘R•“®Œ")   entry.bunrui = HB_MIZEN_JODOUSHI;
      else if (bunrui_str == L"˜A—p•“®Œ")   entry.bunrui = HB_RENYOU_JODOUSHI;
      else if (bunrui_str == L"I~•“®Œ")   entry.bunrui = HB_SHUUSHI_JODOUSHI;
      else if (bunrui_str == L"˜A‘Ì•“®Œ")   entry.bunrui = HB_RENTAI_JODOUSHI;
      else if (bunrui_str == L"‰¼’è•“®Œ")   entry.bunrui = HB_KATEI_JODOUSHI;
      else if (bunrui_str == L"–½—ß•“®Œ")   entry.bunrui = HB_MEIREI_JODOUSHI;
      else if (bunrui_str == L"ŒÜ’i“®Œ")     entry.bunrui = HB_GODAN_DOUSHI;
      else if (bunrui_str == L"ˆê’i“®Œ")     entry.bunrui = HB_ICHIDAN_DOUSHI;
      else if (bunrui_str == L"ƒJ•Ï“®Œ")     entry.bunrui = HB_KAHEN_DOUSHI;
      else if (bunrui_str == L"ƒT•Ï“®Œ")     entry.bunrui = HB_SAHEN_DOUSHI;
      else if (bunrui_str == L"Š¿Œê")         entry.bunrui = HB_KANGO;
      else if (bunrui_str == L"Ú“ª«")       entry.bunrui = HB_SETTOUJI;
      else if (bunrui_str == L"Ú”ö«")       entry.bunrui = HB_SETSUBIJI;
      else {
        printf("..\\mzimeja.dic (%d): WARNING: invalid bunrui\n", lineno);
        continue;
      }

      entry.pre = fields[0];
      if (fields.size() < 3 || fields[2].empty()) {
        entry.post = fields[0];
      } else {
        entry.post = fields[2];
      }

      if (fields.size() >= 4) {
        entry.tags = fields[3];
      }
    }
    g_entries.push_back(entry);
  }

  std::sort(g_entries.begin(), g_entries.end(), entry_compare);
  return true;
} // do_load

void do_wprintf(const wchar_t *format, ...) {
  wchar_t wbuf[512];
  char buf[512];
  va_list va;
  va_start(va, format);
  wvsprintfW(wbuf, format, va);
  WideCharToMultiByte(CP_ACP, 0, wbuf, -1, buf, 512, NULL, NULL);
  fputs(buf, stdout);
  va_end(va);
}

bool do_katsuyou_ikeiyoushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'‚¢') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2, temp3;
  temp0 = str;
  temp0 += L"‚©‚ë";
  do_wprintf(L"–¢‘RŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚©‚Á";
  temp1 = str;
  temp1 += L"‚­";
  temp2 = str;
  temp2 += L"‚¤";
  size_t i, count = sizeof(g_table) / sizeof(g_table[0]);
  wchar_t ch0 = str[str.size() - 1];
  wchar_t ch1 = g_consonant_map[ch0];
  switch (g_vowel_map[ch0]) {
  case L'‚ ':
    temp3 = str.substr(0, str.size() - 1);
    for (i = 0; i < count; ++i) {
      if (g_table[i][0] == ch1) {
        temp3 += g_table[i][4];
        temp3 += L"‚¤";
        break;
      }
    }
    if (i == count) temp3.clear();
    break;
  case L'‚¢':
    temp3 = str;
    temp3 += L"‚ã‚¤";
    break;
  default:
    temp3.clear();
    break;
  }
  if (temp3.empty()) {
    do_wprintf(L"˜A—pŒ`: %s %s %s\n",
      temp0.c_str(), temp1.c_str(), temp2.c_str());
  } else {
    do_wprintf(L"˜A—pŒ`: %s %s %s %s\n",
      temp0.c_str(), temp1.c_str(), temp2.c_str(), temp3.c_str());
  }

  temp0 = str;
  temp0 += L"‚¢";
  temp1 = str;
  if (str[str.size() - 1] != L'‚µ') {
    temp1 += L"‚µ";
  }
  do_wprintf(L"I~Œ`: %s\n", temp0.c_str(), temp1.c_str());
  temp0 = str;
  temp0 += L"‚¢";
  temp1 = str;
  temp1 += L"‚«";
  do_wprintf(L"˜A‘ÌŒ`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  temp0 += L"‚¯‚ê";
  do_wprintf(L"‰¼’èŒ`: %s\n", temp0.c_str());

  do_wprintf(L"–½—ßŒ`: (‚È‚µ)\n");

  temp0 = str;
  temp0 += L"‚³";
  temp1 = str;
  temp1 += L"‚İ";
  temp2 = str;
  temp2 += L"–Ú";
  do_wprintf(L"–¼ŒŒ`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  return true;
}

bool do_katsuyou_nakeiyoushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'‚È') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp0 += L"‚¾‚ë";
  do_wprintf(L"–¢‘RŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚¾‚Á";
  temp1 = str;
  temp1 += L"‚Å";
  temp2 = str;
  temp2 += L"‚É";
  do_wprintf(L"˜A—pŒ`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  temp0 += L"‚¾";
  do_wprintf(L"I~Œ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚È";
  do_wprintf(L"˜A‘ÌŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚È‚ç";
  do_wprintf(L"‰¼’èŒ`: %s\n", temp0.c_str());

  do_wprintf(L"–½—ßŒ`: (‚È‚µ)\n");

  temp0 = str;
  temp1 = str;
  temp1 += L"‚³";
  do_wprintf(L"–¼ŒŒ`: %s %s\n", temp0.c_str(), temp1.c_str() );

  return true;
}

bool do_katsuyou_godan_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty()) return false;
  wchar_t ch = str[str.size() - 1];
  if (g_vowel_map[ch] != L'‚¤') return false;
  str.resize(str.size() - 1);

  wchar_t gyou = g_consonant_map[ch];
  size_t ngyou = 0;
  const size_t count = sizeof(g_table) / sizeof(g_table[0]);
  for (size_t i = 0; i < count; ++i) {
    if (g_table[i][0] == gyou) {
      ngyou = i;
      break;
    }
  }

  int type;
  switch (g_consonant_map[ch]) {
  case L'‚©': case L'‚ª':              type = 1; break;
  case L'‚È': case L'‚Î': case L'‚Ü':  type = 2; break;
  case L'‚½': case L'‚ç': case L'‚í':  type = 3; break;
  default:                             type = 0; break;
  }

  std::wstring temp0, temp1, temp2;

  if (ngyou == 0) {
    temp0 = str;
    temp0 += L'‚í';
  } else {
    temp0 = str;
    temp0 += g_table[ngyou][0];
  }
  temp1 = str;
  temp1 += g_table[ngyou][4];
  do_wprintf(L"–¢‘RŒ`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  temp1 = str;
  switch (type) {
  case 1: temp1 += L'‚¢'; break;
  case 2: temp1 += L'‚ñ'; break;
  case 3: temp1 += L'‚Á'; break;
  default: temp1.clear(); break;
  }
  if (temp1.empty()) {
    do_wprintf(L"˜A—pŒ`: %s\n", temp0.c_str());
  } else {
    do_wprintf(L"˜A—pŒ`: %s %s\n", temp0.c_str(), temp1.c_str());
  }

  temp0 = str;
  temp0 += g_table[ngyou][2];
  do_wprintf(L"I~Œ`: %s\n", temp0.c_str());
  do_wprintf(L"˜A‘ÌŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][3];
  do_wprintf(L"‰¼’èŒ`: %s\n", temp0.c_str());
  do_wprintf(L"–½—ßŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  do_wprintf(L"–¼ŒŒ`: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_ichidan_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'‚é') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1;

  temp0 = str;
  do_wprintf(L"–¢‘RŒ`: %s\n", temp0.c_str());
  do_wprintf(L"˜A—pŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚é";
  do_wprintf(L"I~Œ`: %s\n", temp0.c_str());
  do_wprintf(L"˜A‘ÌŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚ê";
  do_wprintf(L"‰¼’èŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚ë";
  temp1 = str;
  temp1 += L"‚æ";
  do_wprintf(L"–½—ßŒ`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  do_wprintf(L"–¼ŒŒ`: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_kahen_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str.substr(str.size() - 2) != L"—ˆ‚é") {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0;

  temp0 = str;
  do_wprintf(L"–¢‘RŒ`: %s\n", temp0.c_str());
  do_wprintf(L"˜A—pŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚é";
  do_wprintf(L"I~Œ`: %s\n", temp0.c_str());
  do_wprintf(L"˜A‘ÌŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚ê";
  do_wprintf(L"‰¼’èŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"‚¢";
  do_wprintf(L"–½—ßŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"•û";
  do_wprintf(L"–¼ŒŒ`: %s\n", temp0.c_str());
  
  return true;
}

bool do_katsuyou_sahen_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.size() < 2) {
    return false;
  }
  bool flag = str.substr(str.size() - 2) == L"‚¸‚é";
  if (str.substr(str.size() - 2) != L"‚·‚é" && !flag) {
    return false;
  }
  str.resize(str.size() - 2);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp1 = str;
  temp2 = str;
  if (flag) {
    temp0 += L"‚´";
    temp1 += L"‚¶";
    temp2 += L"‚º";
  } else {
    temp0 += L"‚³";
    temp1 += L"‚µ";
    temp2 += L"‚¹";
  }
  do_wprintf(L"–¢‘RŒ`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"‚¶";
  } else {
    temp0 += L"‚µ";
  }
  do_wprintf(L"˜A—pŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"‚¸‚é";
    temp1 += L"‚¸";
  } else {
    temp0 += L"‚·‚é";
    temp1 += L"‚·";
  }
  do_wprintf(L"I~Œ`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"‚¸‚é";
  } else {
    temp0 += L"‚·‚é";
  }
  do_wprintf(L"˜A‘ÌŒ`: %s\n", temp0.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"‚¸‚ê";
  } else {
    temp0 += L"‚·‚ê";
  }
  do_wprintf(L"‰¼’èŒ`: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"‚¶‚ë";
    temp1 += L"‚º‚æ";
  } else {
    temp0 += L"‚µ‚ë";
    temp1 += L"‚¹‚æ";
  }
  do_wprintf(L"–½—ßŒ`: %s %s\n", temp0.c_str(), temp1.c_str());

  do_wprintf(L"–¼ŒŒ`: (‚È‚µ)\n");

  return true;
}

bool do_katsuyou(const wchar_t *data) {
  size_t count = g_entries.size();
  for (size_t i = 0; i < count; ++i) {
    const DICT_ENTRY& entry = g_entries[i];
    if (data[0] != entry.pre[0]) continue;
    if (entry.pre == data) {
      if (entry.tags.find(L"[”ñ•W€]") != std::wstring::npos) continue;
      switch (entry.bunrui) {
      case HB_MEISHI:
        do_wprintf(L"# –¼Œ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        break;
      case HB_IKEIYOUSHI:
        do_wprintf(L"# ‚¢Œ`—eŒ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_ikeiyoushi(entry);
        break;
      case HB_NAKEIYOUSHI:
        do_wprintf(L"# ‚ÈŒ`—eŒ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_nakeiyoushi(entry);
        break;
      case HB_RENTAISHI:
        do_wprintf(L"# ˜A‘ÌŒ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        break;
      case HB_GODAN_DOUSHI:
        do_wprintf(L"# ŒÜ’i“®Œ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_godan_doushi(entry);
        break;
      case HB_ICHIDAN_DOUSHI:
        do_wprintf(L"# ˆê’i“®Œ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_ichidan_doushi(entry);
        break;
      case HB_KAHEN_DOUSHI:
        do_wprintf(L"# ƒJ•Ï“®Œ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_kahen_doushi(entry);
        break;
      case HB_SAHEN_DOUSHI:
        do_wprintf(L"# ƒT•Ï“®Œ: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_sahen_doushi(entry);
        break;
      default:
        break;
      }
    }
  }
  return false;
}

void make_maps(void) {
  const size_t count = sizeof(g_table) / sizeof(g_table[0]);
  for (size_t i = 0; i < count; ++i) {
    for (size_t k = 0; k < 5; ++k) {
      g_consonant_map[g_table[i][k]] = g_table[i][0];
    }
    for (size_t k = 0; k < 5; ++k) {
      g_vowel_map[g_table[i][k]] = g_table[0][k];
    }
  }
} // make_maps

int main(int argc, char **argv) {
  make_maps();

  if (!do_load()) {
    printf("ERROR: cannot load dictionary\n");
    return 1;
  }

  wchar_t data[256];
  if (argc >= 2) {
    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, data, 256);
    do_katsuyou(data);
  }
  return 0;
}
