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
  HB_START_NODE,        // �J�n�m�[�h
  HB_MEISHI,            // ����
  HB_IKEIYOUSHI,        // ���`�e��
  HB_NAKEIYOUSHI,       // �Ȍ`�e��
  HB_RENTAISHI,         // �A�̎�
  HB_FUKUSHI,           // ����
  HB_SETSUZOKUSHI,      // �ڑ���
  HB_KANDOUSHI,         // ������
  HB_JOSHI,             // ����
  HB_MIZEN_JODOUSHI,    // ���R������
  HB_RENYOU_JODOUSHI,   // �A�p������
  HB_SHUUSHI_JODOUSHI,  // �I�~������
  HB_RENTAI_JODOUSHI,   // �A�̏�����
  HB_KATEI_JODOUSHI,    // ���菕����
  HB_MEIREI_JODOUSHI,   // ���ߏ�����
  HB_GODAN_DOUSHI,      // �ܒi����
  HB_ICHIDAN_DOUSHI,    // ��i����
  HB_KAHEN_DOUSHI,      // �J�ϓ���
  HB_SAHEN_DOUSHI,      // �T�ϓ���
  HB_KANGO,             // ����
  HB_SETTOUJI,          // �ړ���
  HB_SETSUBIJI,         // �ڔ���
  HB_END_NODE           // �I���m�[�h
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
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', 0, L'��', 0, L'��'},
  {L'��', L'��', L'��', L'��', L'��'},
  {L'��', 0, 0, 0, L'��'},
  {L'��', 0, 0, 0, 0},
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
  return wcschr(L"�ꉹ�J�~���ΉԊL�w�x��ʋ�C���������܌��Z�����O�R�l�q���������Ԏ�\�o��������X�l�����ԐΗ[���쑐��������j�|�������V�c�y������N�����S�{���ؖڕ��E�͗��јZ���H�_�����Ɖ��ĉȉ̉���C�G�O�p�y���Ԋ��ۋL�D�A�|���������ߍ��Z�`���v�������Ìˌ�ߌ�L���H�����l���s�������J���׍ˍ�Z����o�s�~�������F���Ў��T�H�t������H�V�e�S�}��������������ؑD���O�g�������̑��r�n�m�������������ʓ_�X�d�~�������������Ǔ�����n���������ԕ����������ĕ���k���������і���F�j�p�������b�v�����Èӈψ��@�����^�j�w���������׊J�E�K�ي������݋N���q�}�����������Ƌȋǋ����{�N���y�W���������ΌɍK���`�����ՎM�w�n�������d�g�����������ʎҐ̎������K�E�W�I�B�d�Z�h�����������͏�����A�g�i�\�[�^�_�����S�z�������������ő�Αґ��Z�Y�k�����������ǒ����J�S�]�s�x�����o�����������_�g�z�{����������ߔ�@���M�X�\�a�b�i�������������ԕו������ʖ���R���V�L�\�t�z�r�m�l�������Η������H�a���ĈȈʈ݈͈߈�p�h�����݉ۉ��ʉ�B���Q�X�o�e�֊��Ǌ��ϊ��G���I��@��c�~�����������������ɌP�R�S�^�a�i�|�����������Ō���D�q�N�����ō؍ލ��E���D�@�Q�U�Y�c���j���m�i���������؎���j�������Ώď��܏ۏƏȐM�b���Ð��Ȑϐߐܐ���I�󑃑��q�������������ђB�P�u������������I�T�`�k�w�����������ŔM�R�O�s�~���є��K�[�W�t�{�s�v���������ʕӕϕ֖@��]�q������������E�v�{�������ʗǋ����֗ޗߗ���A�V�J�^���ڈ��i�q�c�v�Չt�������������߉͉�������m�i�z��������K���Z�`�t���v�������ϋ֋�Q�o�����������������̌���u�\�z�k���������Ѝ̍ȍۍčݍߍ��G�^�_�x���t�u���}���������ӎɎ��C�q�p�����؏���������E�D�������������ŐӐѐݐڐ��K�c�f�����������������ݑޑԒc�f�z������G�K���������ƔC�R�\�j�ƔŔ�������U�]�n�x�z�w���������ҕٕە�L��h�\�������ȗA�]�a�e�������̖f���و��F�f������D�t�v�g�����Ȋ��Ŋ�������M�^�z�������؋Όh�n�x�����������������ȌČ�F�c�@�g�|�~�������������ύٍ���\�����������p���̎ˎڎ���@���O�A�]�c�k�n�������������Ꮻ����j�m��������������������P�w���t���n������������T�S�a�g�i�l���������������ɓW�}�����͓���F�]�[�h�x�w�o�q�ǔӔ۔�ᕠ���Õ����K��_�Y�S���������͖�D�X�c���~���������՘_�N��", ch) != NULL;
}

BOOL is_common_use_kanji(WCHAR ch) {
  return wcschr(L"���������B���������������ĈÈȈ߈ʈ͈�ˈψЈ׈؈݈шوڈވ̈֜b�ӈ�ۈԈ�܈������������������@���A���B�C�E�F�H�J�S�T���Y�^�_�i�j�p�f�h�c�r�e�s�q�Չu�v�t�w�x�z�y�{�~�����������Q�����������������������������������������������������������񉳉����������������Ή������ԉ����ʉ͉ՉȉˉĉƉ׉؉ى݉Q�߉ŉɉЌC�ǉ̉Ӊ҉ۉ�劢���������D��������������C�E�F�B�G�J�K�򞲉�׉���~�L�O�N�Q�R�U�X�S�W�Y�T�[�_�`�e�p�g�v�i�j�k�s�o�r�u�t�m�l�d�n�w�x�y�z�{�|�����������������������������������Ê��ʊ��̊������Ŋ׊������ъ��������������ԊՊ������������Ǌ֊��ĊɊ��ҊيȊϊ؊͊ӊۊ܊݊�ߊ���������C�����D��F�G�I�O���L�N�Q�S�A���K�T���������M���ʊ���E�P�@�R�Z�X�U�\�`�^�V�Y�[�]�c�e�g�i�l�p�q�r�t�s��v�y�|�u���x�z���P�������}�����{�~�����k�������������������������䋙�����������������������������������������������������ŋƋËȋǋɋʋЋҋϋߋ��ۋ΋Ջ؋͋֋ًыދ݋�������������������@�A�F�J�N�P�M�O�R�S�Q�Z�Y�`�n�a�s�W�^�_�v�b�[�f�k�o�u�h�i�y�X�g�p�w�c�یm�e�x�{�|�}�~�����������������������������������������������������������������������������������������������������������������ȌˌÌČŌҌՌǌʌ̌͌ɌΌٌ֌����ڌ܌݌ߌ�����������H�����E���I�L�b�������@�D�]�l�s�B�F�R�U�X���K�S�m����P�^�c�g�r�x����Z�k�q�v�~���N�T�[���A�Q�`�d�i���a�z�\�j�y�e���t�|�u�w����������������J����������������������������������������������������������������ˍčЍȍэӍɍ͍ʍ̍ύՍ֍׍؍ōٍÍǍ΍ڍۍ�ݍލ܍��ߍ����������|�������D�����A�E�@�B�C�G�M�O�R�Q�V�\�S�Y�P�U�Z�_�^�c�a�b�m�q�x�~���d�j�i�l�s��|�������f�u���g�h�n�o�}�����p�v�w�{�t���������������k�����������������������������������������������������a���������������������������������ŎʎЎԎɎҎˎ͎̎ΎώՎӎ׎֎ڎ؎ގߎݎ��������������������������������B�M�G���@�E�H�L�C���I㵏K�T�A�O�W�D�V�X�R�P�\�`�[�Z�_�d�]�a�e�b�c�f�j�h�i�l�k�m�n�o�q�p�r�t�u�{�����y�}���z�����������������������������@�������������������������я��������������������Ǐˏ̏Ώ����͏Џ׏������ďŏɏϏُ؏ۏ����Əڏ��ᓲ�Տ܏��ʏ���������������������������F�@�H�A�B���G���D�E�J�K�S�\�L�b�c�g�h�N�M�Ð_�O�P�U�Z�^�j�[�a�i�X�f�Q�T�V�R�k�d�e�l�n�m�s�v�r�w�q�t�{�}�����������������������䐏���������������������䐢�������������������Đ������Ȑ������������������������Ð������ŗ[�ːΐԐ̐͐ȐҐǐɐʐӐՐϐѐАؐܐِސڐݐ�ېߐ���������������������D����A�B�F�HⳑK�����J�I�E�@�N�S�O�P�R�T�Q�V�U�_�j�c�d�f�[�e�g�a�i�Y�k�b�o�s�������t���������q�{�}�K���|���]�u���n�r�������m�z�w�������H�����������������������������������������������������������������������������őÑ��đʑ��Α̑ϑґӑّޑёבܑ͑ߑ֑ݑ��ؑԑՑ������������������A�B�E�D�I�N�O�U�S�P�Y�_�T�W�Z�Q�[�]�a�b�c�j�i�f�e�g�k�d�n�r�m�l�p�v�x�s�t�u�k�|�{���~�z�����������������������������������������������������������������ޒ������\���������}���������������������������ǒŒĒʒɒ˒Вؒܒߒ�������@����������������������D�I�J�E�H�K�G�M�R�N�S�O�P�V�T�X�_�W�Y�]�U�c�`�a�d�l�f�i�k�r�s�n�h�q�y�z�w�x�{���~���������������|�������������}�����������������o������������������������������������������������������ēœƓǓȓʓ˓͓ԓؓ��Óݓܘ��ߓޓ������������������������A�C�D�E�F�J�M�N�O�P�S�R�Y�[�\�]�_�Z�c�g�h�j�e�n�k�l�q�t�w�x�o�z�r�s�p�y���{�~�|���}�����������������������������������������������������������ÔƔ��Ĕ������ŔǔȔʔ̔��є��ϔД͔ɔ˔ӔԔؔՔ��ܔ۔�ޔ���ڔ����ߔ����������������@�G�I�C�K��M�P�S�X�\�U�[�]�Y�W�c�b�a�`�L�i�l�n�o�p�q�r�s�v���t�z�}�{�|�����]�������w���x�����~�����������������������������������������������������������������������������Õ��������ݕĕ����ȕʕ̕ЕӕԕϕΕՕҕٕ֕ו��ۚM�ߕ�ܕ���������F�M�������@�A�E�����C���K��I�L�O�J�D�S�R�Z�V�W�Y�h�[�b�^�`�U�a�]�T�X�_�f�e�\�c�d�j�k�ؖp�q�r�l�n�o�v�u�x�{�z�|�}�~�������������������������������������������������������������������������������������ŖƖʖȖ˖Ζ͖іϖӖՖҖԖږٖ�������������ŗR���g���@�A���B�F�L�E�H�I�X�N�P�T�V�Y�U�J�Z�D�^�\�]�_�a�c�p�r�d�m�v�e�f�g�h�t�z�n���lᇗx�q�{�i�w�j�}�����~�����f����������������煗������������������������������������ɗ����������������������������Ǘ����˗ʗ��̗��×ėƗ͗ΗїЗϗ֗ח՗ڗܗݗۗޗߗ���ߗ����������������A�����B�C�F�G�H�I�V�J�M�Y�N�Q�L�O�R�ĘZ�^�[�_�a�b�d�e�f�g�p�r", ch) != NULL;
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
        if (str.find(L"[��W��]") == std::wstring::npos &&
            str.find(L"[�l��]") == std::wstring::npos &&
            str.find(L"[�w��]") == std::wstring::npos &&
            str.find(L"[�n��]") == std::wstring::npos &&
            str.find(L"[���A��]") == std::wstring::npos)
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
      if (bunrui_str == L"����")              entry.bunrui = HB_MEISHI;
      else if (bunrui_str == L"���`�e��")     entry.bunrui = HB_IKEIYOUSHI;
      else if (bunrui_str == L"�Ȍ`�e��")     entry.bunrui = HB_NAKEIYOUSHI;
      else if (bunrui_str == L"�A�̎�")       entry.bunrui = HB_RENTAISHI;
      else if (bunrui_str == L"����")         entry.bunrui = HB_FUKUSHI;
      else if (bunrui_str == L"�ڑ���")       entry.bunrui = HB_SETSUZOKUSHI;
      else if (bunrui_str == L"������")       entry.bunrui = HB_KANDOUSHI;
      else if (bunrui_str == L"����")         entry.bunrui = HB_JOSHI;
      else if (bunrui_str == L"���R������")   entry.bunrui = HB_MIZEN_JODOUSHI;
      else if (bunrui_str == L"�A�p������")   entry.bunrui = HB_RENYOU_JODOUSHI;
      else if (bunrui_str == L"�I�~������")   entry.bunrui = HB_SHUUSHI_JODOUSHI;
      else if (bunrui_str == L"�A�̏�����")   entry.bunrui = HB_RENTAI_JODOUSHI;
      else if (bunrui_str == L"���菕����")   entry.bunrui = HB_KATEI_JODOUSHI;
      else if (bunrui_str == L"���ߏ�����")   entry.bunrui = HB_MEIREI_JODOUSHI;
      else if (bunrui_str == L"�ܒi����")     entry.bunrui = HB_GODAN_DOUSHI;
      else if (bunrui_str == L"��i����")     entry.bunrui = HB_ICHIDAN_DOUSHI;
      else if (bunrui_str == L"�J�ϓ���")     entry.bunrui = HB_KAHEN_DOUSHI;
      else if (bunrui_str == L"�T�ϓ���")     entry.bunrui = HB_SAHEN_DOUSHI;
      else if (bunrui_str == L"����")         entry.bunrui = HB_KANGO;
      else if (bunrui_str == L"�ړ���")       entry.bunrui = HB_SETTOUJI;
      else if (bunrui_str == L"�ڔ���")       entry.bunrui = HB_SETSUBIJI;
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
  if (str.empty() || str[str.size() - 1] != L'��') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2, temp3;
  temp0 = str;
  temp0 += L"����";
  do_wprintf(L"���R�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"����";
  temp1 = str;
  temp1 += L"��";
  temp2 = str;
  temp2 += L"��";
  size_t i, count = sizeof(g_table) / sizeof(g_table[0]);
  wchar_t ch0 = str[str.size() - 1];
  wchar_t ch1 = g_consonant_map[ch0];
  switch (g_vowel_map[ch0]) {
  case L'��':
    temp3 = str.substr(0, str.size() - 1);
    for (i = 0; i < count; ++i) {
      if (g_table[i][0] == ch1) {
        temp3 += g_table[i][4];
        temp3 += L"��";
        break;
      }
    }
    if (i == count) temp3.clear();
    break;
  case L'��':
    temp3 = str;
    temp3 += L"�イ";
    break;
  default:
    temp3.clear();
    break;
  }
  if (temp3.empty()) {
    do_wprintf(L"�A�p�`: %s %s %s\n",
      temp0.c_str(), temp1.c_str(), temp2.c_str());
  } else {
    do_wprintf(L"�A�p�`: %s %s %s %s\n",
      temp0.c_str(), temp1.c_str(), temp2.c_str(), temp3.c_str());
  }

  temp0 = str;
  temp0 += L"��";
  temp1 = str;
  if (str[str.size() - 1] != L'��') {
    temp1 += L"��";
  }
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str(), temp1.c_str());
  temp0 = str;
  temp0 += L"��";
  temp1 = str;
  temp1 += L"��";
  do_wprintf(L"�A�̌`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  temp0 += L"����";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  do_wprintf(L"���ߌ`: (�Ȃ�)\n");

  temp0 = str;
  temp0 += L"��";
  temp1 = str;
  temp1 += L"��";
  temp2 = str;
  temp2 += L"��";
  do_wprintf(L"�����`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  return true;
}

bool do_katsuyou_nakeiyoushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'��') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp0 += L"����";
  do_wprintf(L"���R�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"����";
  temp1 = str;
  temp1 += L"��";
  temp2 = str;
  temp2 += L"��";
  do_wprintf(L"�A�p�`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"�Ȃ�";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  do_wprintf(L"���ߌ`: (�Ȃ�)\n");

  temp0 = str;
  temp1 = str;
  temp1 += L"��";
  do_wprintf(L"�����`: %s %s\n", temp0.c_str(), temp1.c_str() );

  return true;
}

bool do_katsuyou_godan_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty()) return false;
  wchar_t ch = str[str.size() - 1];
  if (g_vowel_map[ch] != L'��') return false;
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
  case L'��': case L'��':              type = 1; break;
  case L'��': case L'��': case L'��':  type = 2; break;
  case L'��': case L'��': case L'��':  type = 3; break;
  default:                             type = 0; break;
  }

  std::wstring temp0, temp1, temp2;

  if (ngyou == 0) {
    temp0 = str;
    temp0 += L'��';
  } else {
    temp0 = str;
    temp0 += g_table[ngyou][0];
  }
  temp1 = str;
  temp1 += g_table[ngyou][4];
  do_wprintf(L"���R�`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  temp1 = str;
  switch (type) {
  case 1: temp1 += L'��'; break;
  case 2: temp1 += L'��'; break;
  case 3: temp1 += L'��'; break;
  default: temp1.clear(); break;
  }
  if (temp1.empty()) {
    do_wprintf(L"�A�p�`: %s\n", temp0.c_str());
  } else {
    do_wprintf(L"�A�p�`: %s %s\n", temp0.c_str(), temp1.c_str());
  }

  temp0 = str;
  temp0 += g_table[ngyou][2];
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][3];
  do_wprintf(L"����`: %s\n", temp0.c_str());
  do_wprintf(L"���ߌ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += g_table[ngyou][1];
  do_wprintf(L"�����`: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_ichidan_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str[str.size() - 1] != L'��') {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0, temp1;

  temp0 = str;
  do_wprintf(L"���R�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�p�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  temp1 = str;
  temp1 += L"��";
  do_wprintf(L"���ߌ`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  do_wprintf(L"�����`: %s\n", temp0.c_str());

  return true;
}

bool do_katsuyou_kahen_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.empty() || str.substr(str.size() - 2) != L"����") {
    return false;
  }
  str.resize(str.size() - 1);

  std::wstring temp0;

  temp0 = str;
  do_wprintf(L"���R�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�p�`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�I�~�`: %s\n", temp0.c_str());
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"����`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"���ߌ`: %s\n", temp0.c_str());

  temp0 = str;
  temp0 += L"��";
  do_wprintf(L"�����`: %s\n", temp0.c_str());
  
  return true;
}

bool do_katsuyou_sahen_doushi(const DICT_ENTRY& entry) {
  std::wstring str = entry.post;
  if (str.size() < 2) {
    return false;
  }
  bool flag = str.substr(str.size() - 2) == L"����";
  if (str.substr(str.size() - 2) != L"����" && !flag) {
    return false;
  }
  str.resize(str.size() - 2);

  std::wstring temp0, temp1, temp2;

  temp0 = str;
  temp1 = str;
  temp2 = str;
  if (flag) {
    temp0 += L"��";
    temp1 += L"��";
    temp2 += L"��";
  } else {
    temp0 += L"��";
    temp1 += L"��";
    temp2 += L"��";
  }
  do_wprintf(L"���R�`: %s %s %s\n", temp0.c_str(), temp1.c_str(), temp2.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"��";
  } else {
    temp0 += L"��";
  }
  do_wprintf(L"�A�p�`: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"����";
    temp1 += L"��";
  } else {
    temp0 += L"����";
    temp1 += L"��";
  }
  do_wprintf(L"�I�~�`: %s %s\n", temp0.c_str(), temp1.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"����";
  } else {
    temp0 += L"����";
  }
  do_wprintf(L"�A�̌`: %s\n", temp0.c_str());

  temp0 = str;
  if (flag) {
    temp0 += L"����";
  } else {
    temp0 += L"����";
  }
  do_wprintf(L"����`: %s\n", temp0.c_str());

  temp0 = str;
  temp1 = str;
  if (flag) {
    temp0 += L"����";
    temp1 += L"����";
  } else {
    temp0 += L"����";
    temp1 += L"����";
  }
  do_wprintf(L"���ߌ`: %s %s\n", temp0.c_str(), temp1.c_str());

  do_wprintf(L"�����`: (�Ȃ�)\n");

  return true;
}

bool do_katsuyou(const wchar_t *data) {
  size_t count = g_entries.size();
  for (size_t i = 0; i < count; ++i) {
    const DICT_ENTRY& entry = g_entries[i];
    if (data[0] != entry.pre[0]) continue;
    if (entry.pre == data) {
      if (entry.tags.find(L"[��W��]") != std::wstring::npos) continue;
      switch (entry.bunrui) {
      case HB_MEISHI:
        do_wprintf(L"# ����: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        break;
      case HB_IKEIYOUSHI:
        do_wprintf(L"# ���`�e��: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_ikeiyoushi(entry);
        break;
      case HB_NAKEIYOUSHI:
        do_wprintf(L"# �Ȍ`�e��: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_nakeiyoushi(entry);
        break;
      case HB_RENTAISHI:
        do_wprintf(L"# �A�̎�: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        break;
      case HB_GODAN_DOUSHI:
        do_wprintf(L"# �ܒi����: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_godan_doushi(entry);
        break;
      case HB_ICHIDAN_DOUSHI:
        do_wprintf(L"# ��i����: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_ichidan_doushi(entry);
        break;
      case HB_KAHEN_DOUSHI:
        do_wprintf(L"# �J�ϓ���: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
        do_katsuyou_kahen_doushi(entry);
        break;
      case HB_SAHEN_DOUSHI:
        do_wprintf(L"# �T�ϓ���: %s: %s\n", entry.pre.c_str(), entry.post.c_str());
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
