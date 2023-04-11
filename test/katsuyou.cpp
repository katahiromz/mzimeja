// katsuyou.cpp
// (Japanese, UTF-8)
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
  HB_START_NODE,        // 開始ノード
  HB_MEISHI,            // 名詞
  HB_IKEIYOUSHI,        // い形容詞
  HB_NAKEIYOUSHI,       // な形容詞
  HB_RENTAISHI,         // 連体詞
  HB_FUKUSHI,           // 副詞
  HB_SETSUZOKUSHI,      // 接続詞
  HB_KANDOUSHI,         // 感動詞
  HB_JOSHI,             // 助詞
  HB_MIZEN_JODOUSHI,    // 未然助動詞
  HB_RENYOU_JODOUSHI,   // 連用助動詞
  HB_SHUUSHI_JODOUSHI,  // 終止助動詞
  HB_RENTAI_JODOUSHI,   // 連体助動詞
  HB_KATEI_JODOUSHI,    // 仮定助動詞
  HB_MEIREI_JODOUSHI,   // 命令助動詞
  HB_GODAN_DOUSHI,      // 五段動詞
  HB_ICHIDAN_DOUSHI,    // 一段動詞
  HB_KAHEN_DOUSHI,      // カ変動詞
  HB_SAHEN_DOUSHI,      // サ変動詞
  HB_KANGO,             // 漢語
  HB_SETTOUJI,          // 接頭辞
  HB_SETSUBIJI,         // 接尾辞
  HB_END_NODE           // 終了ノード
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
  {L'あ', L'い', L'う', L'え', L'お'},
  {L'か', L'き', L'く', L'け', L'こ'},
  {L'が', L'ぎ', L'ぐ', L'げ', L'ご'},
  {L'さ', L'し', L'す', L'せ', L'そ'},
  {L'ざ', L'じ', L'ず', L'ぜ', L'ぞ'},
  {L'た', L'ち', L'つ', L'て', L'と'},
  {L'だ', L'ぢ', L'づ', L'で', L'ど'},
  {L'な', L'に', L'ぬ', L'ね', L'の'},
  {L'は', L'ひ', L'ふ', L'へ', L'ほ'},
  {L'ば', L'び', L'ぶ', L'べ', L'ぼ'},
  {L'ぱ', L'ぴ', L'ぷ', L'ぺ', L'ぽ'},
  {L'ま', L'み', L'む', L'め', L'も'},
  {L'や', 0, L'ゆ', 0, L'よ'},
  {L'ら', L'り', L'る', L'れ', L'ろ'},
  {L'わ', 0, 0, 0, L'を'},
  {L'ん', 0, 0, 0, 0},
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
  return wcschr(L"一音雨円王火花貝学休九玉空気下月見犬五口校金左三山四子糸字耳七車手十出女小正上森人水生青赤石夕千先川草早足村大男竹虫中町天田土二日入年白八百本名木目文右力立林六引羽雲遠園家何夏科歌画会回海絵外角楽活間顔岩丸記汽帰弓牛魚教強近今兄形京計原言元古戸語午後広交工光公考黄行高合国谷黒細才作算紙矢姉市止時寺自色室社弱首週秋春書少場食新親心図頭数西星声晴雪切船線前組走太多体台弟池地知茶昼長朝鳥直通点店電冬刀当答東道同読内南肉馬買売麦半番父歩風分聞米母方北毎妹万鳴明毛門野夜友曜用来里理話思悪安暗意委医育院員飲運泳駅横央屋温荷開界階館感寒漢岸起期客急級球究去橋業曲局銀苦区具宮君化軽係血決県研湖庫幸向港号根祭皿指始歯死詩仕使次持事式実写者昔酒守主取受習拾集終州重住宿暑所助消昭章商勝乗植身進申深真神世整全想相送息速族他打代対待第題短炭談着注柱調帳追丁定庭笛鉄転都度島等登湯投豆動童農波配倍箱畑発反板坂悲皮鼻美筆氷表病秒品負部福服物平返勉放味命面問薬役由油遊有予葉陽羊洋様落旅両緑流礼列練路和愛案以位胃囲衣印英栄塩億貨課加果芽械改害街覚各関完管官観願喜季旗紀希機器議救泣求給挙協鏡競共極訓軍郡型径景芸結欠建健験固候功好航康告差最菜材昨殺刷札察参散産残氏史試士司治児辞失借種周祝順初唱笑焼松賞象照省信臣清静成席積節折説戦選浅巣争倉束側続卒孫隊帯達単置仲貯兆腸停低底的典伝徒努灯働堂特得毒熱然念敗梅博飯飛費必票標付府不夫無副粉兵別辺変便法包望牧末満未脈民約勇要養浴利陸量良漁料輪類令冷例歴連老労録圧移因永衛営益易液演桜応往恩価過可河仮賀解快確格額慣幹刊眼規基寄技義逆旧久許居境均禁句群経潔険検券件現限減個故護効講構鉱耕厚興混査災採妻際再在罪財雑賛酸支資師志飼枝示似識質謝舎授修述術準序証承招常状条情職織製精性政制勢税責績設接絶舌銭祖素総増像造則測属損貸退態団断築張程提敵適統銅導徳独任燃能破犯版判肥比非備俵評貧富布婦武復複仏編弁保墓豊報防暴夢務迷綿輸余預容率略留領貿律異遺域宇映沿延我灰閣革拡割株簡干看巻危揮机貴疑吸郷胸供筋勤敬系警劇激穴絹憲権厳源己呼誤孝皇后紅鋼降穀刻骨困砂座済裁策冊蚕誌視私詞至姿磁捨射尺若樹宗収衆就従縦縮熟純署処諸除傷障将城蒸針仁推垂寸誠聖盛専宣染泉洗善層装奏操創窓蔵臓尊存宅探担誕暖段値宙忠著潮頂庁賃痛展党討糖届難乳認脳納派肺背俳拝班晩否秘批腹奮陛並閉補暮訪宝棒忘亡枚幕密盟模訳優郵幼翌欲卵乱覧裏臨論朗片", ch) != NULL;
}

BOOL is_common_use_kanji(WCHAR ch) {
  return wcschr(L"亜哀挨愛曖悪握圧扱宛嵐安案暗以衣位囲医依委威為畏胃尉異移萎偉椅彙意違維慰遺緯域育一壱逸茨芋引印因咽姻員院淫陰飲隠韻右宇羽雨唄鬱畝浦運雲永泳英映栄営詠影鋭衛易疫益液駅悦越謁閲円延沿炎怨宴媛援園煙猿遠鉛塩演縁艶汚王凹央応往押旺欧殴桜翁奥横岡屋億憶臆虞乙俺卸音恩温穏下化火加可仮何花佳価果河苛科架夏家荷華菓貨渦過嫁暇禍靴寡歌箇稼課蚊牙瓦我画芽賀雅餓介回灰会快戒改怪拐悔海界皆械絵開階塊楷解潰壊懐諧貝外劾害崖涯街慨蓋該概骸垣柿各角拡革格核殻郭覚較隔閣確獲嚇穫学岳楽額顎掛潟括活喝渇割葛滑褐轄且株釜鎌刈干刊甘汗缶完肝官冠巻看陥乾勘患貫寒喚堪換敢棺款間閑勧寛幹感漢慣管関歓監緩憾還館環簡観韓艦鑑丸含岸岩玩眼頑顔願企伎危机気岐希忌汽奇祈季紀軌既記起飢鬼帰基寄規亀喜幾揮期棋貴棄毀旗器畿輝機騎技宜偽欺義疑儀戯擬犠議菊吉喫詰却客脚逆虐九久及弓丘旧休吸朽臼求究泣急級糾宮救球給嗅窮牛去巨居拒拠挙虚許距魚御漁凶共叫狂京享供協況峡挟狭恐恭胸脅強教郷境橋矯鏡競響驚仰暁業凝曲局極玉巾斤均近金菌勤琴筋僅禁緊錦謹襟吟銀区句苦駆具惧愚空偶遇隅串屈掘窟熊繰君訓勲薫軍郡群兄刑形系径茎係型契計恵啓掲渓経蛍敬景軽傾携継詣慶憬稽憩警鶏芸迎鯨隙劇撃激桁欠穴血決結傑潔月犬件見券肩建研県倹兼剣拳軒健険圏堅検嫌献絹遣権憲賢謙鍵繭顕験懸元幻玄言弦限原現舷減源厳己戸古呼固股虎孤弧故枯個庫湖雇誇鼓錮顧五互午呉後娯悟碁語誤護口工公勾孔功巧広甲交光向后好江考行坑孝抗攻更効幸拘肯侯厚恒洪皇紅荒郊香候校耕航貢降高康控梗黄喉慌港硬絞項溝鉱構綱酵稿興衡鋼講購乞号合拷剛傲豪克告谷刻国黒穀酷獄骨駒込頃今困昆恨根婚混痕紺魂墾懇左佐沙査砂唆差詐鎖座挫才再災妻采砕宰栽彩採済祭斎細菜最裁債催塞歳載際埼在材剤財罪崎作削昨柵索策酢搾錯咲冊札刷刹拶殺察撮擦雑皿三山参桟蚕惨産傘散算酸賛残斬暫士子支止氏仕史司四市矢旨死糸至伺志私使刺始姉枝祉肢姿思指施師恣紙脂視紫詞歯嗣試詩資飼誌雌摯賜諮示字寺次耳自似児事侍治持時滋慈辞磁餌璽鹿式識軸七叱失室疾執湿嫉漆質実芝写社車舎者射捨赦斜煮遮謝邪蛇尺借酌釈爵若弱寂手主守朱取狩首殊珠酒腫種趣寿受呪授需儒樹収囚州舟秀周宗拾秋臭修袖終羞習週就衆集愁酬醜蹴襲十汁充住柔重従渋銃獣縦叔祝宿淑粛縮塾熟出述術俊春瞬旬巡盾准殉純循順準潤遵処初所書庶暑署緒諸女如助序叙徐除小升少召匠床抄肖尚招承昇松沼昭宵将消症祥称笑唱商渉章紹訟勝掌晶焼焦硝粧詔証象傷奨照詳彰障憧衝賞償礁鐘上丈冗条状乗城浄剰常情場畳蒸縄壌嬢錠譲醸色拭食植殖飾触嘱織職辱尻心申伸臣芯身辛侵信津神唇娠振浸真針深紳進森診寝慎新審震薪親人刃仁尽迅甚陣尋腎須図水吹垂炊帥粋衰推酔遂睡穂随髄枢崇数据杉裾寸瀬是井世正生成西声制姓征性青斉政星牲省凄逝清盛婿晴勢聖誠精製誓静請整醒税夕斥石赤昔析席脊隻惜戚責跡積績籍切折拙窃接設雪摂節説舌絶千川仙占先宣専泉浅洗染扇栓旋船戦煎羨腺詮践箋銭潜線遷選薦繊鮮全前善然禅漸膳繕狙阻祖租素措粗組疎訴塑遡礎双壮早争走奏相荘草送倉捜挿桑巣掃曹曽爽窓創喪痩葬装僧想層総遭槽踪操燥霜騒藻造像増憎蔵贈臓即束足促則息捉速側測俗族属賊続卒率存村孫尊損遜他多汰打妥唾堕惰駄太対体耐待怠胎退帯泰堆袋逮替貸隊滞態戴大代台第題滝宅択沢卓拓託濯諾濁但達脱奪棚誰丹旦担単炭胆探淡短嘆端綻誕鍛団男段断弾暖談壇地池知値恥致遅痴稚置緻竹畜逐蓄築秩窒茶着嫡中仲虫沖宙忠抽注昼柱衷酎鋳駐著貯丁弔庁兆町長挑帳張彫眺釣頂鳥朝貼超腸跳徴嘲潮澄調聴懲直勅捗沈珍朕陳賃鎮追椎墜通痛塚漬坪爪鶴低呈廷弟定底抵邸亭貞帝訂庭逓停偵堤提程艇締諦泥的笛摘滴適敵溺迭哲鉄徹撤天典店点展添転填田伝殿電斗吐妬徒途都渡塗賭土奴努度怒刀冬灯当投豆東到逃倒凍唐島桃討透党悼盗陶塔搭棟湯痘登答等筒統稲踏糖頭謄藤闘騰同洞胴動堂童道働銅導瞳峠匿特得督徳篤毒独読栃凸突届屯豚頓貪鈍曇丼那奈内梨謎鍋南軟難二尼弐匂肉虹日入乳尿任妊忍認寧熱年念捻粘燃悩納能脳農濃把波派破覇馬婆罵拝杯背肺俳配排敗廃輩売倍梅培陪媒買賠白伯拍泊迫剥舶博薄麦漠縛爆箱箸畑肌八鉢発髪伐抜罰閥反半氾犯帆汎伴判坂阪板版班畔般販斑飯搬煩頒範繁藩晩番蛮盤比皮妃否批彼披肥非卑飛疲秘被悲扉費碑罷避尾眉美備微鼻膝肘匹必泌筆姫百氷表俵票評漂標苗秒病描猫品浜貧賓頻敏瓶不夫父付布扶府怖阜附訃負赴浮婦符富普腐敷膚賦譜侮武部舞封風伏服副幅復福腹複覆払沸仏物粉紛雰噴墳憤奮分文聞丙平兵併並柄陛閉塀幣弊蔽餅米壁璧癖別蔑片辺返変偏遍編弁便勉歩保哺捕補舗母募墓慕暮簿方包芳邦奉宝抱放法泡胞俸倣峰砲崩訪報蜂豊飽褒縫亡乏忙坊妨忘防房肪某冒剖紡望傍帽棒貿貌暴膨謀頬北木朴牧睦僕墨撲没勃堀本奔翻凡盆麻摩磨魔毎妹枚昧埋幕膜枕又末抹万満慢漫未味魅岬密蜜脈妙民眠矛務無夢霧娘名命明迷冥盟銘鳴滅免面綿麺茂模毛妄盲耗猛網目黙門紋問冶夜野弥厄役約訳薬躍闇由油喩愉諭輸癒唯友有勇幽悠郵湧猶裕遊雄誘憂融優与予余誉預幼用羊妖洋要容庸揚揺葉陽溶腰様瘍踊窯養擁謡曜抑沃浴欲翌翼拉裸羅来雷頼絡落酪辣乱卵覧濫藍欄吏利里理痢裏履璃離陸立律慄略柳流留竜粒隆硫侶旅虜慮了両良料涼猟陵量僚領寮療瞭糧力緑林厘倫輪隣臨瑠涙累塁類令礼冷励戻例鈴零霊隷齢麗暦歴列劣烈裂恋連廉練錬呂炉賂路露老労弄郎朗浪廊楼漏籠六録麓論和話賄脇惑枠湾腕", ch) != NULL;
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
        if (str.find(L"[非標準]") == std::wstring::npos &&
            str.find(L"[人名]") == std::wstring::npos &&
            str.find(L"[駅名]") == std::wstring::npos &&
            str.find(L"[地名]") == std::wstring::npos &&
            str.find(L"[動植物]") == std::wstring::npos)
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
      if (bunrui_str == L"名詞")              entry.bunrui = HB_MEISHI;
      else if (bunrui_str == L"い形容詞")     entry.bunrui = HB_IKEIYOUSHI;
      else if (bunrui_str == L"な形容詞")     entry.bunrui = HB_NAKEIYOUSHI;
      else if (bunrui_str == L"連体詞")       entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"副詞")         entry.bunrui = HB_FUKUSHI;
      else if (bunrui_str == L"接続詞")       entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"感動詞")       entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"助詞")         entry.bunrui = HB_JOSHI;
      else if (bunrui_str == L"未然助動詞")   entry.bunrui = HB_MIZEN_JODOUSHI;
      else if (bunrui_str == L"連用助動詞")   entry.bunrui = HB_RENYOU_JODOUSHI;
      else if (bunrui_str == L"終止助動詞")   entry.bunrui = HB_SHUUSHI_JODOUSHI;
      else if (bunrui_str == L"連体助動詞")   entry.bunrui = HB_RENTAI_JODOUSHI;
      else if (bunrui_str == L"仮定助動詞")   entry.bunrui = HB_KATEI_JODOUSHI;
      else if (bunrui_str == L"命令助動詞")   entry.bunrui = HB_MEIREI_JODOUSHI;
      else if (bunrui_str == L"五段動詞")     entry.bunrui = HB_GODAN_DOUSHI;
      else if (bunrui_str == L"一段動詞")     entry.bunrui = HB_ICHIDAN_DOUSHI;
      else if (bunrui_str == L"カ変動詞")     entry.bunrui = HB_KAHEN_DOUSHI;
      else if (bunrui_str == L"サ変動詞")     entry.bunrui = HB_SAHEN_DOUSHI;
      else if (bunrui_str == L"漢語")         entry.bunrui = HB_KANGO;
      else if (bunrui_str == L"接頭辞")       entry.bunrui = HB_SETTOUJI;
      else if (bunrui_str == L"接尾辞")       entry.bunrui = HB_SETSUBIJI;
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
  if (str.empty() || str[str.size() - 1] != L'い') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2, temp3;
  temp0 = str;
  temp0 += L"かろ";
  do_wprintf(L"未然形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"かっ";
  temp1 = str;
  temp1 += L"く";
  temp2 = str;
  temp2 += L"う";
  size_t i, count = sizeof(g_table) / sizeof(g_table[0]);
  wchar_t ch0 = str[str.size() - 1];
  wchar_t ch1 = g_consonant_map[ch0];
  switch (g_vowel_map[ch0]) {
  case L'あ':
    temp3 = str.substr(0, str.size() - 1);
    for (i = 0; i < count; ++i) {
      if (g_table[i][0] == ch1) {
        temp3 += g_table[i][4];
        temp3 += L"う";
        break;
      }
    }
    if (i == count) temp3.clear();
    break;
  case L'い':
    temp3 = str;
    temp3 += L"ゅう";
    break;
  default:
    temp3.clear();
    break;
  }
  if (temp3.empty()) {
    do_wprintf(L"連用形: %s %s %s\n",
      temp0.c_str(), temp1.c_str(), temp2.c_str());
  } else {
    do_wprintf(L"連用形: %s %s %s %s\n",
      temp0.c_str(), temp1.c_str(), temp2.c_str(), temp3.c_str());
  }

  temp0 = str;
  temp0 += L"い";
  temp1 = str;
  if (str[str.size() - 1] != L'し') {
    temp1 += L"し";
  }
  do_wprintf(L"終止形: %s\n", temp0.c_str(), temp1.c_str());
  temp0 = str;
  temp0 += L"い";
  temp1 = str;
  temp1 += L"き";
  do_wprintf(L"連体形: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  temp0 += L"けれ";
  do_wprintf(L"仮定形: %s\n", temp0.c_str());

  do_wprintf(L"命令形: (なし)\n");

  temp0 = str;
  temp0 += L"さ";
  temp1 = str;
  temp1 += L"み";
  temp2 = str;
  temp2 += L"目";
  do_wprintf(L"名詞形: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  return true;
}

bool do_katsuyou_nakeiyoushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'な') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp0 += L"だろ";
  do_wprintf(L"未然形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"だっ";
  temp1 = str;
  temp1 += L"で";
  temp2 = str;
  temp2 += L"に";
  do_wprintf(L"連用形: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  temp0 += L"だ";
  do_wprintf(L"終止形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"な";
  do_wprintf(L"連体形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"なら";
  do_wprintf(L"仮定形: %s\n", temp0.c_str());

  do_wprintf(L"命令形: (なし)\n");

  temp0 = str;
  temp1 = str;
  temp1 += L"さ";
  do_wprintf(L"名詞形: %s %s\n", temp0.c_str(), temp1.c_str() );

  return true;
}

bool do_katsuyou_godan_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty()) return false;
  wchar_t ch = str[str.size() - 1];
  if (g_vowel_map[ch] != L'う') return false;
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
  case L'か': case L'が':              type = 1; break;
  case L'な': case L'ば': case L'ま':  type = 2; break;
  case L'た': case L'ら': case L'わ':  type = 3; break;
  default:                             type = 0; break;
  }

  std::wstring temp0, temp1, temp2;

  if (ngyou == 0) {
    temp0 = str;
    temp0 += L'わ';
  } else {
    temp0 = str;
    temp0 += g_table[ngyou][0];
  }
  temp1 = str;
  temp1 += g_table[ngyou][4];
  do_wprintf(L"未然形: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  temp1 = str;
  switch (type) {
  case 1: temp1 += L'い'; break;
  case 2: temp1 += L'ん'; break;
  case 3: temp1 += L'っ'; break;
  default: temp1.clear(); break;
  }
  if (temp1.empty()) {
    do_wprintf(L"連用形: %s\n", temp0.c_str());
  } else {
    do_wprintf(L"連用形: %s %s\n", temp0.c_str(), temp1.c_str());
  }

  temp0 = str;
  temp0 += g_table[ngyou][2];
  do_wprintf(L"終止形: %s\n", temp0.c_str());
  do_wprintf(L"連体形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][3];
  do_wprintf(L"仮定形: %s\n", temp0.c_str());
  do_wprintf(L"命令形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  do_wprintf(L"名詞形: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_ichidan_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'る') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1;

  temp0 = str;
  do_wprintf(L"未然形: %s\n", temp0.c_str());
  do_wprintf(L"連用形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"る";
  do_wprintf(L"終止形: %s\n", temp0.c_str());
  do_wprintf(L"連体形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"れ";
  do_wprintf(L"仮定形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"ろ";
  temp1 = str;
  temp1 += L"よ";
  do_wprintf(L"命令形: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  do_wprintf(L"名詞形: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_kahen_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str.substr(str.size() - 2) != L"来る") {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0;

  temp0 = str;
  do_wprintf(L"未然形: %s\n", temp0.c_str());
  do_wprintf(L"連用形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"る";
  do_wprintf(L"終止形: %s\n", temp0.c_str());
  do_wprintf(L"連体形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"れ";
  do_wprintf(L"仮定形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"い";
  do_wprintf(L"命令形: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"方";
  do_wprintf(L"名詞形: %s\n", temp0.c_str());
  
  return true;
}

bool do_katsuyou_sahen_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.size() < 2) {
    return false;
  }
  bool flag = str.substr(str.size() - 2) == L"ずる";
  if (str.substr(str.size() - 2) != L"する" && !flag) {
    return false;
  }
  str.resize(str.size() - 2);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp1 = str;
  temp2 = str;
  if (flag) {
    temp0 += L"ざ";
    temp1 += L"じ";
    temp2 += L"ぜ";
  } else {
    temp0 += L"さ";
    temp1 += L"し";
    temp2 += L"せ";
  }
  do_wprintf(L"未然形: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"じ";
  } else {
    temp0 += L"し";
  }
  do_wprintf(L"連用形: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"ずる";
    temp1 += L"ず";
  } else {
    temp0 += L"する";
    temp1 += L"す";
  }
  do_wprintf(L"終止形: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"ずる";
  } else {
    temp0 += L"する";
  }
  do_wprintf(L"連体形: %s\n", temp0.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"ずれ";
  } else {
    temp0 += L"すれ";
  }
  do_wprintf(L"仮定形: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"じろ";
    temp1 += L"ぜよ";
  } else {
    temp0 += L"しろ";
    temp1 += L"せよ";
  }
  do_wprintf(L"命令形: %s %s\n", temp0.c_str(), temp1.c_str());

  do_wprintf(L"名詞形: (なし)\n");

  return true;
}

bool do_katsuyou(const wchar_t *data) {
  size_t count = g_entries.size();
  for (size_t i = 0; i < count; ++i) {
    const DICT_ENTRY& entry = g_entries[i];
    if (data[0] != entry.pre[0]) continue;
    if (entry.pre == data) {
      if (entry.tags.find(L"[非標準]") != std::wstring::npos) continue;
      switch (entry.bunrui) {
      case HB_MEISHI:
        do_wprintf(L"# 名詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        break;
      case HB_IKEIYOUSHI:
        do_wprintf(L"# い形容詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_ikeiyoushi(entry);
        break;
      case HB_NAKEIYOUSHI:
        do_wprintf(L"# な形容詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_nakeiyoushi(entry);
        break;
      case HB_RENTAISHI:
        do_wprintf(L"# 連体詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        break;
      case HB_GODAN_DOUSHI:
        do_wprintf(L"# 五段動詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_godan_doushi(entry);
        break;
      case HB_ICHIDAN_DOUSHI:
        do_wprintf(L"# 一段動詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_ichidan_doushi(entry);
        break;
      case HB_KAHEN_DOUSHI:
        do_wprintf(L"# カ変動詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_kahen_doushi(entry);
        break;
      case HB_SAHEN_DOUSHI:
        do_wprintf(L"# サ変動詞: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
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
