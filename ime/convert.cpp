// convert.cpp --- mzimeja kana kanji conversion
// ���Ȋ����ϊ��B
//////////////////////////////////////////////////////////////////////////////
// (Japanese, Shift_JIS)
// NOTE: This file uses Japanese cp932 encoding. To compile this on g++,
//       please add options: -finput-charset=CP932 -fexec-charset=CP932

#include "mzimeja.h"
#include "resource.h"
#include <algorithm>        // for std::sort

const DWORD c_dwMilliseconds = 8000;

// �Ђ炪�ȕ\�B�i���̊��p�Ŏg�p�����B
static const wchar_t s_hiragana_table[][5] = {
    // DAN_A, DAN_I, DAN_U, DAN_E, DAN_O
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_A
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_KA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_GA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_SA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_ZA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_TA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_DA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_NA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_HA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_BA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_PA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_MA
    {L'��',     0, L'��',     0, L'��'}, // GYOU_YA
    {L'��', L'��', L'��', L'��', L'��'}, // GYOU_RA
    {L'��',     0,     0,     0, L'��'}, // GYOU_WA
    {L'��',     0,     0,     0,     0}, // GYOU_NN
}; // �� s_hiragana_table[GYOU_DA][DAN_U] �̂悤�Ɏg�p����B

std::unordered_map<wchar_t,wchar_t>   g_vowel_map;      // �ꉹ�ʑ��B
std::unordered_map<wchar_t,wchar_t>   g_consonant_map;  // �q���ʑ��B

// �q���̎ʑ��ƕꉹ�̎ʑ����쐬����B
void MakeLiteralMaps() {
    if (g_consonant_map.size())
        return;
    g_consonant_map.clear();
    g_vowel_map.clear();
    const size_t count = _countof(s_hiragana_table);
    for (size_t i = 0; i < count; ++i) {
        for (size_t k = 0; k < 5; ++k) {
            g_consonant_map[s_hiragana_table[i][k]] = s_hiragana_table[i][0];
        }
        for (size_t k = 0; k < 5; ++k) {
            g_vowel_map[s_hiragana_table[i][k]] = s_hiragana_table[0][k];
        }
    }
} // MakeLiteralMaps

// ��Ǔ_��ϊ�����B
WCHAR ConvertCommaPeriod(WCHAR ch) {
    if (ch == L',') {
        if (Config_GetDWORD(L"bCommaPeriod", FALSE))
            ch = L'�C';
        else
            ch = L'�A';
    } else if (ch == L'.') {
        if (Config_GetDWORD(L"bCommaPeriod", FALSE))
            ch = L'�D';
        else
            ch = L'�B';
    }
    return ch;
}

// �i�����ނ𕶎���ɕϊ�����i�f�o�b�O�p�j�B
LPCWSTR BunruiToString(HinshiBunrui bunrui) {
    int index = int(bunrui) - int(HB_HEAD);
    static const wchar_t *s_array[] = {
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

// �i���̘A���R�X�g�B
static int
CandConnectCost(HinshiBunrui bunrui1, HinshiBunrui bunrui2)
{
    if (bunrui2 == HB_PERIOD || bunrui2 == HB_COMMA) return 0;
    if (bunrui2 == HB_TAIL) return 0;
    if (bunrui1 == HB_SYMBOL || bunrui2 == HB_SYMBOL) return 0;
    if (bunrui1 == HB_UNKNOWN || bunrui2 == HB_UNKNOWN) return 0;
    switch (bunrui1) {
    case HB_HEAD:
        switch (bunrui2) {
        case HB_JODOUSHI: case HB_SHUU_JOSHI:
            return 5;
        default:
            break;
        }
        break;
    case HB_MEISHI: // ����
        switch (bunrui2) {
        case HB_MEISHI:
            return 10;
        case HB_SETTOUJI:
            return 10;
        case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
            return 5;
        default:
            break;
        }
        break;
    case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI: // ���`�e���A�Ȍ`�e��
        switch (bunrui1) {
        case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI:
            return 10;
        case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
        case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
            return 3;
        default:
            break;
        }
        break;
    case HB_MIZEN_JODOUSHI: case HB_RENYOU_JODOUSHI:
    case HB_SHUUSHI_JODOUSHI: case HB_RENTAI_JODOUSHI:
    case HB_KATEI_JODOUSHI: case HB_MEIREI_JODOUSHI:
        ASSERT(0);
        break;
    case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI: case HB_KAHEN_DOUSHI:
    case HB_SAHEN_DOUSHI: case HB_SETTOUJI:
        switch (bunrui2) {
        case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI: case HB_KAHEN_DOUSHI:
        case HB_SAHEN_DOUSHI: case HB_SETTOUJI:
            return 5;
        default:
            break;
        }
        break;
    case HB_RENTAISHI: case HB_FUKUSHI: case HB_SETSUZOKUSHI:
    case HB_KANDOUSHI: case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI: case HB_SHUU_JOSHI: case HB_JODOUSHI:
    case HB_SETSUBIJI: case HB_COMMA: case HB_PERIOD:
    default:
        break;
    }
    return 0;
} // CandConnectCost

// �i���̘A���\���B
static BOOL
IsNodeConnectable(const LatticeNode& node1, const LatticeNode& node2) {
    if (node2.bunrui == HB_PERIOD || node2.bunrui == HB_COMMA) return TRUE;
    if (node2.bunrui == HB_TAIL) return TRUE;
    if (node1.bunrui == HB_SYMBOL || node2.bunrui == HB_SYMBOL) return TRUE;
    if (node1.bunrui == HB_UNKNOWN || node2.bunrui == HB_UNKNOWN) return TRUE;

    switch (node1.bunrui) {
    case HB_HEAD:
        switch (node2.bunrui) {
        case HB_SHUU_JOSHI:
            return FALSE;
        default:
            break;
        }
        return TRUE;
    case HB_MEISHI: // ����
        switch (node2.bunrui) {
        case HB_SETTOUJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_IKEIYOUSHI: case HB_NAKEIYOUSHI: // ���`�e���A�Ȍ`�e��
        switch (node1.katsuyou) {
        case MIZEN_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[���R�`�ɘA��]")) {
                    if (node2.pre[0] == L'��' || node2.pre == L"��") {
                        return TRUE;
                    }
                }
            }
            return FALSE;
        case RENYOU_KEI:
            switch (node2.bunrui) {
            case HB_JODOUSHI:
                if (node2.HasTag(L"[�A�p�`�ɘA��]")) {
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
                if (node2.HasTag(L"[�I�~�`�ɘA��]")) {
                    return TRUE;
                }
                if (node2.HasTag(L"[��X�̌�]")) {
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
                if (node2.pre == L"��" || node2.pre == L"�ǂ�" || node2.pre == L"��") {
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
    case HB_RENTAISHI: // �A�̎�
        switch (node2.bunrui) {
        case HB_KANDOUSHI: case HB_JODOUSHI: case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_FUKUSHI: // ����
        switch (node2.bunrui) {
        case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
        case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_SETSUZOKUSHI: // �ڑ���
        switch (node2.bunrui) {
        case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
        case HB_FUKU_JOSHI: case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_KANDOUSHI: // ������
        switch (node2.bunrui) {
        case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
        case HB_FUKU_JOSHI: case HB_SETSUBIJI: case HB_JODOUSHI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI: case HB_FUKU_JOSHI:
        // �I�����ȊO�̏���
        switch (node2.bunrui) {
        case HB_SETSUBIJI:
            return FALSE;
        default:
            return TRUE;
        }
        break;
    case HB_SHUU_JOSHI: // �I����
        switch (node2.bunrui) {
        case HB_MEISHI: case HB_SETTOUJI: case HB_SHUU_JOSHI:
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case HB_JODOUSHI: // ������
        switch (node1.katsuyou) {
        case MIZEN_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[���R�`�ɘA��]")) {
                    return TRUE;
                }
            }
            return FALSE;
        case RENYOU_KEI:
            switch (node2.bunrui) {
            case HB_JODOUSHI:
                if (node2.HasTag(L"[�A�p�`�ɘA��]")) {
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
                if (node2.HasTag(L"[�I�~�`�ɘA��]")) {
                    return TRUE;
                }
                if (node2.HasTag(L"[��X�̌�]")) {
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
                if (node2.pre == L"��" || node2.pre == L"�ǂ�" || node2.pre == L"��") {
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
        // ����
        switch (node1.katsuyou) {
        case MIZEN_KEI:
            if (node2.bunrui == HB_JODOUSHI) {
                if (node2.HasTag(L"[���R�`�ɘA��]")) {
                    return TRUE;
                }
            }
            return FALSE;
        case RENYOU_KEI:
            switch (node2.bunrui) {
            case HB_JODOUSHI:
                if (node2.HasTag(L"[�A�p�`�ɘA��]")) {
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
                if (node2.HasTag(L"[�I�~�`�ɘA��]")) {
                    return TRUE;
                }
                if (node2.HasTag(L"[��X�̌�]")) {
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
                if (node2.pre == L"��" || node2.pre == L"�ǂ�" || node2.pre == L"��") {
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
    case HB_SETTOUJI: // �ړ���
        switch (node2.bunrui) {
        case HB_MEISHI:
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case HB_SETSUBIJI: // �ڔ���
        switch (node2.bunrui) {
        case HB_SETTOUJI:
            return FALSE;
        default:
            break;
        }
        break;
    case HB_COMMA: case HB_PERIOD: // �A�B
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

// ��{�����f�[�^���X�L��������B
static size_t ScanBasicDict(WStrings& records, const WCHAR *dict_data, WCHAR ch) {
    ASSERT(dict_data);

    if (ch == 0)
        return 0;

    // ���R�[�h��؂�ƕ���ch�̑g�ݍ��킹����������B
    // ����ŕ���ch�Ŏn�܂�P��������ł���B
    WCHAR sz[3] = {RECORD_SEP, ch, 0};
    const WCHAR *pch1 = wcsstr(dict_data, sz);
    if (pch1 == NULL)
        return FALSE;

    const WCHAR *pch2 = pch1; // ���݂̈ʒu�B
    const WCHAR *pch3;
    for (;;) {
        // ���݂̈ʒu�̎��̃��R�[�h��؂�ƕ���ch�̑g�ݍ��킹����������B
        pch3 = wcsstr(pch2 + 1, sz);
        if (pch3 == NULL) break; // �Ȃ���΃��[�v�I���B
        pch2 = pch3; // ���݂̈ʒu���X�V�B
    }
    pch3 = wcschr(pch2 + 1, RECORD_SEP); // ���݂̈ʒu�̎��̃��R�[�h��؂����������B
    if (pch3 == NULL)
        return FALSE;

    // �ŏ��ɔ����������R�[�h��؂肩��Ō�̃��R�[�h��؂�܂ł̕�������擾����B
    std::wstring str;
    str.assign(pch1 + 1, pch3);

    // ���R�[�h��؂�ŕ������ă��R�[�h���擾����B
    sz[0] = RECORD_SEP;
    sz[1] = 0;
    str_split(records, str, sz);
    ASSERT(records.size());
    return records.size();
} // ScanBasicDict

static WStrings s_UserDictRecords;

static INT CALLBACK UserDictProc(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr, LPVOID lpData) {
    ASSERT(lpStr && lpStr[0]);
    ASSERT(lpRead && lpRead[0]);
    Lattice *pThis = (Lattice *)lpData;
    ASSERT(pThis != NULL);

    // �f�[�^�̏������B
    std::wstring pre = lpRead;
    std::wstring post = lpStr;
    Gyou gyou = GYOU_A;
    HinshiBunrui bunrui = StyleToHinshi(dw);

    if (pre.size() <= 1)
        return 0;

    // �f�[�^�������`���ɕϊ�����B
    std::wstring substr;
    wchar_t ch;
    size_t i, ngyou;
    switch (bunrui) {
    case HB_NAKEIYOUSHI: // �Ȍ`�e��
        i = pre.size() - 1;
        if (pre[i] == L'��') pre.resize(i);
        i = post.size() - 1;
        if (post[i] == L'��') post.resize(i);
        break;
    case HB_IKEIYOUSHI: // ���`�e��
        i = pre.size() - 1;
        if (pre[i] == L'��') pre.resize(i);
        i = post.size() - 1;
        if (post[i] == L'��') post.resize(i);
        break;
    case HB_ICHIDAN_DOUSHI: // ��i����
        ASSERT(pre[pre.size() - 1] == L'��');
        ASSERT(post[post.size() - 1] == L'��');
        pre.resize(pre.size() - 1);
        post.resize(post.size() - 1);
        break;
    case HB_KAHEN_DOUSHI: // �J�ϓ���
        substr = pre.substr(pre.size() - 2, 2);
        if (substr != L"����") 
            return TRUE;
        pre = substr;
        substr = post.substr(pre.size() - 2, 2);
        post = substr;
        break;
    case HB_SAHEN_DOUSHI: // �T�ϓ���
        if (pre == L"����") 
            return TRUE;
        substr = pre.substr(pre.size() - 2, 2);
        if (substr == L"����")
            gyou = GYOU_SA;
        else if (substr != L"����") 
            gyou = GYOU_ZA;
        else
            return TRUE;
        pre = substr;
        post = post.substr(pre.size() - 2, 2);
        break;
    case HB_GODAN_DOUSHI: // �ܒi����
        MakeLiteralMaps();
        ch = pre[pre.size() - 1];
        if (g_vowel_map[ch] != L'��')
            return TRUE;
        pre.resize(pre.size() - 1);
        post.resize(post.size() - 1);
        ch = g_consonant_map[ch];
        ngyou = GYOU_A;
        for (i = 0; i < _countof(s_hiragana_table); ++i) {
            if (s_hiragana_table[i][0] == ch) {
                ngyou = i;
                break;
            }
        }
        gyou = (Gyou)ngyou;
        break;
    default:
        break;
    }

    // ���R�[�h�̎d�l�F
    //     fields[0]: std::wstring �ϊ��O������;
    //     fields[1]: { MAKEWORD(HinshiBunrui, Gyou), 0 };
    //     fields[2]: std::wstring �ϊ��㕶����;
    //     fields[3]: std::wstring tags;
    WStrings fields(4);
    fields[0] = pre;
    fields[1] += (WCHAR)MAKEWORD(bunrui, gyou);
    fields[2] = post;
    fields[3] = L"[���[�U����]";

    std::wstring sep = { FIELD_SEP };
    std::wstring record = str_join(fields, sep);
    s_UserDictRecords.push_back(record);

    return TRUE;
}

// ���[�U�[�����f�[�^���X�L��������B
static size_t ScanUserDict(WStrings& records, WCHAR ch, Lattice *pThis) {
    s_UserDictRecords.clear();
    ImeEnumRegisterWord(UserDictProc, NULL, 0, NULL, pThis);

    records.insert(records.end(), s_UserDictRecords.begin(), s_UserDictRecords.end());
    s_UserDictRecords.clear();
 
    return records.size();
}

//////////////////////////////////////////////////////////////////////////////
// Dict (dictionary) - �����f�[�^�B

// �����f�[�^�̃R���X�g���N�^�B
Dict::Dict() {
    m_hMutex = NULL;
    m_hFileMapping = NULL;
}

// �����f�[�^�̃f�X�g���N�^�B
Dict::~Dict() {
    Unload();
}

// �����f�[�^�t�@�C���̃T�C�Y���擾����B
DWORD Dict::GetSize() const {
    DWORD ret = 0;
    WIN32_FIND_DATAW find;
    find.nFileSizeLow = 0;
    HANDLE hFind = ::FindFirstFileW(m_strFileName.c_str(), &find);
    if (hFind != INVALID_HANDLE_VALUE) {
        ::CloseHandle(hFind);
        ret = find.nFileSizeLow;
    }
    return ret;
}

// ������ǂݍ��ށB
BOOL Dict::Load(const wchar_t *file_name, const wchar_t *object_name) {
    if (IsLoaded()) return TRUE; // ���łɓǂݍ��ݍς݁B

    m_strFileName = file_name;
    m_strObjectName = object_name;

    SECURITY_ATTRIBUTES *psa = CreateSecurityAttributes(); // �Z�L�����e�B�������쐬�B
    ASSERT(psa);

    // �~���[�e�b�N�X (�r��������s���I�u�W�F�N�g) ���쐬�B
    if (m_hMutex == NULL) {
        m_hMutex = ::CreateMutexW(psa, FALSE, m_strObjectName.c_str());
    }
    if (m_hMutex == NULL) {
        // free sa
        FreeSecurityAttributes(psa);
        return FALSE;
    }

    // �t�@�C���T�C�Y���擾�B
    DWORD cbSize = GetSize();
    if (cbSize == 0) return FALSE;

    BOOL ret = FALSE;
    DWORD wait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds); // �r�������҂B
    if (wait == WAIT_OBJECT_0) {
        // �t�@�C���}�b�s���O���쐬����B
        m_hFileMapping = ::CreateFileMappingW(
                INVALID_HANDLE_VALUE, psa, PAGE_READWRITE,
                0, cbSize, (m_strObjectName + L"FileMapping").c_str());
        if (m_hFileMapping) {
            // �t�@�C���}�b�s���O���쐬���ꂽ�B
            if (::GetLastError() == ERROR_ALREADY_EXISTS) {
                // �t�@�C���}�b�s���O�����łɑ��݂���B
                ret = TRUE;
            } else {
                // �V�����쐬���ꂽ�B�t�@�C����ǂݍ��ށB
                FILE *fp = _wfopen(m_strFileName.c_str(), L"rb");
                if (fp) {
                    wchar_t *pch = Lock();
                    if (pch) {
                        ret = (BOOL)fread(pch, cbSize, 1, fp);
                        Unlock(pch);
                    }
                    fclose(fp);
                }
            }
        }
        // �r�����������B
        ::ReleaseMutex(m_hMutex);
    }

    // free sa
    FreeSecurityAttributes(psa);

    return ret;
} // Dict::Load

// �������A�����[�h����B
void Dict::Unload() {
    if (m_hMutex) {
        if (m_hFileMapping) {
            DWORD wait = ::WaitForSingleObject(m_hMutex, c_dwMilliseconds); // �r�������҂B
            if (wait == WAIT_OBJECT_0) {
                // �t�@�C���}�b�s���O�����B
                if (m_hFileMapping) {
                    ::CloseHandle(m_hFileMapping);
                    m_hFileMapping = NULL;
                }
                // �r�����������B
                ::ReleaseMutex(m_hMutex);
            }
        }
        // �~���[�e�b�N�X�����B
        ::CloseHandle(m_hMutex);
        m_hMutex = NULL;
    }
}

// ���������b�N���ď��̎擾���J�n�B
wchar_t *Dict::Lock() {
    if (m_hFileMapping == NULL) return NULL;
    DWORD cbSize = GetSize();
    void *pv = ::MapViewOfFile(m_hFileMapping,
                               FILE_MAP_ALL_ACCESS, 0, 0, cbSize);
    return reinterpret_cast<wchar_t *>(pv);
}

// �����̃��b�N���������āA���̎擾���I���B
void Dict::Unlock(wchar_t *data) {
    ::UnmapViewOfFile(data);
}

// �����͓ǂݍ��܂ꂽ���H
BOOL Dict::IsLoaded() const {
    return (m_hMutex != NULL && m_hFileMapping != NULL);
}

//////////////////////////////////////////////////////////////////////////////
// MzConvResult, MzConvClause etc.

// ���߂Ƀm�[�h��ǉ�����B
void MzConvClause::add(const LatticeNode *node) {
    bool matched = false;
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (candidates[i].converted == node->post) {
            if (candidates[i].cost > node->cost)  {
                candidates[i].cost = node->cost;
                candidates[i].bunruis.insert(node->bunrui);
                candidates[i].tags += node->tags;
            }
            matched = true;
            break;
        }
    }

    if (!matched) {
        MzConvCandidate cand;
        cand.hiragana = node->pre;
        cand.converted = node->post;
        cand.cost = node->cost;
        cand.bunruis.insert(node->bunrui);
        cand.tags = node->tags;
        candidates.push_back(cand);
    }
}

static inline bool
CandidateCompare(const MzConvCandidate& cand1, const MzConvCandidate& cand2) {
    return cand1.cost < cand2.cost;
}

// �R�X�g�Ō����\�[�g����B
void MzConvClause::sort() {
    std::sort(candidates.begin(), candidates.end(), CandidateCompare);
}

// �R�X�g�Ō��ʂ��\�[�g����B
void MzConvResult::sort() {
    for (size_t i = 1; i < clauses.size(); ++i) {
        for (size_t iCand1 = 0; iCand1 < clauses[i - 1].candidates.size(); ++iCand1) {
            for (size_t iCand2 = 0; iCand2 < clauses[i].candidates.size(); ++iCand2) {
                MzConvCandidate& cand1 = clauses[i - 1].candidates[iCand1];
                MzConvCandidate& cand2 = clauses[i].candidates[iCand2];
                int min_cost = 0x7FFF;
                std::unordered_set<HinshiBunrui>::iterator it1, end1 = cand1.bunruis.end();
                std::unordered_set<HinshiBunrui>::iterator it2, end2 = cand2.bunruis.end();
                for (it1 = cand1.bunruis.begin(); it1 != end1; ++it1)  {
                    for (it2 = cand2.bunruis.begin(); it2 != end2; ++it2)  {
                        int cost = CandConnectCost(*it1, *it2);
                        if (cost < min_cost) {
                            min_cost = cost;
                        }
                    }
                }
                cand2.cost += min_cost;
            }
        }
    }

    for (size_t i = 0; i < clauses.size(); ++i) {
        clauses[i].sort();
    }
}

//////////////////////////////////////////////////////////////////////////////
// LatticeNode - ���e�B�X�ilattice�j�̃m�[�h�B

// �R�X�g���v�Z�B
int LatticeNode::CalcCost() const {
    int ret = 0;
    if (bunrui == HB_KANGO) ret += 200;
    if (bunrui == HB_SYMBOL) ret += 120;
    if (tags.size() != 0) {
        if (HasTag(L"[��W��]")) ret += 100;
        if (HasTag(L"[�s�ސT]")) ret += 50;
        if (HasTag(L"[�l��]")) ret += 30;
        else if (HasTag(L"[�w��]")) ret += 30;
        else if (HasTag(L"[�n��]")) ret += 30;

        // ���[�U�[�����P��̓R�X�g�}�C�i�X30�B
        if (HasTag(L"[���[�U����]")) ret -= 30;
    }
    return ret;
}

// �������H
bool LatticeNode::IsDoushi() const {
    switch (bunrui) {
    case HB_GODAN_DOUSHI: case HB_ICHIDAN_DOUSHI:
    case HB_KAHEN_DOUSHI: case HB_SAHEN_DOUSHI:
        return true;
    default:
        break;
    }
    return false;
}

// ���������H
bool LatticeNode::IsJodoushi() const {
    switch (bunrui) {
    case HB_JODOUSHI:
    case HB_MIZEN_JODOUSHI: case HB_RENYOU_JODOUSHI:
    case HB_SHUUSHI_JODOUSHI: case HB_RENTAI_JODOUSHI:
    case HB_KATEI_JODOUSHI: case HB_MEIREI_JODOUSHI:
        return true;
    default:
        break;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Lattice - ���e�B�X

// �ǉ����B
void Lattice::AddExtra() {
    // �����itoday�j
    if (pre == L"���傤") {
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        WCHAR sz[32];

        WStrings fields(4);
        fields[0] = pre;
        fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

        StringCchPrintfW(sz, _countof(sz), L"%u�N%u��%u��", st.wYear, st.wMonth, st.wDay);
        fields[2] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u/%02u/%02u", st.wYear, st.wMonth, st.wDay);
        fields[2] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%02u/%02u/%04u", st.wMonth, st.wDay, st.wYear);
        fields[2] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%04u-%02u-%02u", st.wYear, st.wMonth, st.wDay);
        fields[2] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%02u-%02u-%04u", st.wMonth, st.wDay, st.wYear);
        fields[2] = sz;
        DoFields(0, fields);
        return;
    }

    // ���N�ithis year�j
    if (pre == L"���Ƃ�") {
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        WCHAR sz[32];

        WStrings fields(4);
        fields[0] = pre;
        fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

        StringCchPrintfW(sz, _countof(sz), L"%u�N", st.wYear);
        fields[2] = sz;
        DoFields(0, fields);
        return;
    }

    // ���݂̎����icurrent time�j
    if (pre == L"������" || pre == L"��������") {
        SYSTEMTIME st;
        ::GetLocalTime(&st);
        WCHAR sz[32];

        WStrings fields(4);
        fields[0] = pre;
        fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));

        if (pre == L"��������") {
            fields[2] = L"������";
            DoFields(0, fields);
            fields[2] = L"����";
            DoFields(0, fields);
        }

        StringCchPrintfW(sz, _countof(sz), L"%u��%u��%u�b", st.wHour, st.wMinute, st.wSecond);
        fields[2] = sz;
        DoFields(0, fields);

        StringCchPrintfW(sz, _countof(sz), L"%02u��%02u��%02u�b", st.wHour, st.wMinute, st.wSecond);
        fields[2] = sz;
        DoFields(0, fields);

        if (st.wHour >= 12) {
            StringCchPrintfW(sz, _countof(sz), L"�ߌ�%u��%u��%u�b", st.wHour - 12, st.wMinute, st.wSecond);
            fields[2] = sz;
            DoFields(0, fields);

            StringCchPrintfW(sz, _countof(sz), L"�ߌ�%02u��%02u��%02u�b", st.wHour - 12, st.wMinute, st.wSecond);
            fields[2] = sz;
            DoFields(0, fields);
        } else {
            StringCchPrintfW(sz, _countof(sz), L"�ߑO%u��%u��%u�b", st.wHour, st.wMinute, st.wSecond);
            fields[2] = sz;
            DoFields(0, fields);

            StringCchPrintfW(sz, _countof(sz), L"�ߑO%02u��%02u��%02u�b", st.wHour, st.wMinute, st.wSecond);
            fields[2] = sz;
            DoFields(0, fields);
        }

        StringCchPrintfW(sz, _countof(sz), L"%02u:%02u:%02u", st.wHour, st.wMinute, st.wSecond);
        fields[2] = sz;
        DoFields(0, fields);
        return;
    }

    if (pre == L"���Ԃ�") { // myself
        WCHAR sz[64];
        DWORD dwSize = _countof(sz);
        if (::GetUserNameW(sz, &dwSize)) {
            WStrings fields(4);
            fields[0] = pre;
            fields[1].assign(1, MAKEWORD(HB_MEISHI, 0));
            fields[2] = sz;
            DoFields(0, fields);
        }
        return;
    }

    // �J�b�R (parens, brackets, braces, ...)
    if (pre == L"������") {
        WStrings items;
        str_split(items, TheIME.LoadSTR(IDS_PAREN), std::wstring(L"\t"));

        WStrings fields(4);
        fields[0] = pre;
        fields[1].assign(1, MAKEWORD(HB_SYMBOL, 0));
        for (size_t i = 0; i < items.size(); ++i) {
            fields[2] = items[i];
            DoFields(0, fields);
        }
        return;
    }

    // �L���isymbols�j
    static const wchar_t *s_words[] = {
        L"������",      // IDS_SYMBOLS
        L"��������",    // IDS_KEISEN
        L"��������",    // IDS_MATH
        L"���񂩂�",    // IDS_SANKAKU
        L"������",      // IDS_SHIKAKU
        L"������",      // IDS_ZUKEI
        L"�܂�",        // IDS_MARU
        L"�ق�",        // IDS_STARS
        L"�Ђ�",        // IDS_HISHI
        L"�Ă�",        // IDS_POINTS
        L"����",      // IDS_UNITS
        L"�ӂƂ�����",  // IDS_FUTOUGOU
        L"����",        // IDS_TATE
        L"���ĂЂ���",  // IDS_TATE_HIDARI
        L"���Ă݂�",    // IDS_TATE_MIGI
        L"�Ђ��肤��",  // IDS_HIDARI_UE
        L"�Ђ��肵��",  // IDS_HIDARI_SHITA
        L"�ӂƂ킭",    // IDS_FUTO_WAKU
        L"�ق��킭",    // IDS_HOSO_WAKU
        L"�܂�Ȃ�",    // IDS_MANNAKA
        L"�݂�����",    // IDS_MIGI_UE
        L"�݂�����",    // IDS_MIGI_SHITA
        L"�悱",        // IDS_YOKO
        L"�悱����",    // IDS_YOKO_UE
        L"�悱����",    // IDS_YOKO_SHITA
        L"���Ȃ�",      // IDS_SAME
        L"�₶�邵",    // IDS_ARROWS
        L"���肵��",    // IDS_GREEK
        L"����",        // IDS_UP
        L"����",        // IDS_DOWN
        L"�Ђ���",      // IDS_LEFT
        L"�݂�",        // IDS_RIGHT
    };
    for (size_t i = 0; i < _countof(s_words); ++i) {
        if (pre == s_words[i]) {
            WStrings items;
            WCHAR *pch = TheIME.LoadSTR(IDS_SYMBOLS + INT(i));
            WStrings fields(4);
            fields[0] = pre;
            fields[1].assign(1, MAKEWORD(HB_SYMBOL, 0));
            int cost = 0;
            while (*pch) {
                fields[2].assign(1, *pch++);
                DoFields(0, fields, cost);
                ++cost;
            }
            return;
        }
    }
} // Lattice::AddExtra

// ��������m�[�h��ǉ�����B
BOOL Lattice::AddNodes(size_t index, const WCHAR *dict_data) {
    const size_t length = pre.size();
    ASSERT(length);

    // separator
    std::wstring sep;
    sep += FIELD_SEP;

    WStrings fields, records;
    for (; index < length; ++index) {
        if (refs[index] == 0) continue;

        // periods (�B�B�B)
        if (is_period(pre[index])) {
            size_t saved = index;
            do {
                ++index;
            } while (is_period(pre[index]));

            fields.resize(4);
            fields[0] = pre.substr(saved, index - saved);
            fields[1] = MAKEWORD(HB_PERIOD, 0);
            switch (index - saved) {
            case 2:
                fields[2] += L'�d';
                break;
            case 3:
                fields[2] += L'�c';
                break;
            default:
                fields[2] = fields[0];
                break;
            }
            DoFields(saved, fields);
            --index;
            continue;
        }
        // center dots (�E�E�E)
        if (pre[index] == L'�E') {
            size_t saved = index;
            do {
                ++index;
            } while (pre[index] == L'�E');

            fields.resize(4);
            fields[0] = pre.substr(saved, index - saved);
            fields[1] = MAKEWORD(HB_SYMBOL, 0);
            switch (index - saved) {
            case 2:
                fields[2] += L'�d';
                break;
            case 3:
                fields[2] += L'�c';
                break;
            default:
                fields[2] = fields[0];
                break;
            }
            DoFields(saved, fields);
            --index;
            continue;
        }
        // commas (�A�A�A)
        if (is_comma(pre[index])) {
            size_t saved = index;
            do {
                ++index;
            } while (is_comma(pre[index]));

            fields.resize(4);
            fields[0] = pre.substr(saved, index - saved);
            fields[1] = MAKEWORD(HB_COMMA, 0);
            fields[2] = fields[0];
            DoFields(saved, fields);
            --index;
            continue;
        }
        // arrow right (��)
        if ((pre[index] == L'-' || pre[index] == L'�|' || pre[index] == L'�[') &&
            (pre[index + 1] == L'>' || pre[index + 1] == L'��'))
        {
            fields.resize(4);
            fields[0] = pre.substr(index, 2);
            fields[1] = MAKEWORD(HB_SYMBOL, 0);
            fields[2] += L'��';
            DoFields(index, fields);
            ++index;
            continue;
        }
        // arrow left (��)
        if ((pre[index] == L'<' || pre[index] == L'��') &&
            (pre[index + 1] == L'-' || pre[index + 1] == L'�|' ||
             pre[index + 1] == L'�['))
        {
            fields.resize(4);
            fields[0] = pre.substr(index, 2);
            fields[1] = MAKEWORD(HB_SYMBOL, 0);
            fields[2] += L'��';
            DoFields(index, fields);
            ++index;
            continue;
        }
        // other non-hiragana
        if (!is_hiragana(pre[index])) { // �Ђ炪�Ȃł͂Ȃ��H
            size_t saved = index;
            do {
                ++index;
            } while (!is_hiragana(pre[index]) && pre[index]);

            fields.resize(4);
            fields[0] = pre.substr(saved, index - saved);
            fields[1] = MAKEWORD(HB_MEISHI, 0);
            fields[2] = fields[0];
            DoMeishi(saved, fields);

            // �S���������Ȃ����ȕϊ����s���B
            if (are_all_chars_numeric(fields[0])) {
                fields[2] = convert_to_kansuuji(fields[0]);
                DoMeishi(saved, fields);
                fields[2] = convert_to_kansuuji_brief(fields[0]);
                DoMeishi(saved, fields);
                fields[2] = convert_to_kansuuji_formal(fields[0]);
                DoMeishi(saved, fields);

                // �X�֔ԍ��ϊ��B
                std::wstring postal = lcmap(fields[0], LCMAP_HALFWIDTH);
                if (postal.size() == 3)
                    postal += L"00";
                if (postal.size() == 5)
                    postal += L"00";
                if (postal.size() == 7) {
                    std::wstring addr = postal_code(postal.c_str());
                    if (addr.size()) {
                        fields[2] = addr;
                        DoMeishi(saved, fields, -10);
                    }
                }
            }

            --index;
            continue;
        }

        // ��{�������X�L��������B
        size_t count = ScanBasicDict(records, dict_data, pre[index]);
        DebugPrintW(L"ScanBasicDict(%c) count: %d\n", pre[index], count);

        // ���[�U�[�������X�L��������B
        count = ScanUserDict(records, pre[index], this);
        DebugPrintW(L"ScanUserDict(%c) count: %d\n", pre[index], count);

        // store data for each record
        for (size_t k = 0; k < records.size(); ++k) {
            const std::wstring& record = records[k];
            str_split(fields, record, std::wstring(sep));
            DoFields(index, fields);
        }

        // special cases
        switch (pre[index]) {
        case L'��': case L'��': case L'��': // KURU
            // �J�ϓ����B
            fields.resize(4);
            fields[1].push_back(MAKEWORD(HB_KAHEN_DOUSHI, GYOU_KA));
            DoKahenDoushi(index, fields);
            break;
        case L'��': case L'��': case L'��': case L'��': // SURU
            // �T�ϓ����B
            fields.resize(4);
            fields[1].push_back(MAKEWORD(HB_SAHEN_DOUSHI, GYOU_SA));
            DoSahenDoushi(index, fields);
            break;
        default:
            break;
        }
    }

    return TRUE;
} // Lattice::AddNodes

// for deleting the nodes of different size
struct DeleteDifferentSizeNode {
    size_t m_size;
    DeleteDifferentSizeNode(size_t size) {
        m_size = size;
    }
    bool operator()(const LatticeNodePtr& n) {
        return n->pre.size() != m_size;
    }
};

// �P�ꕶ�ߕϊ��p�̃m�[�h�Q��ǉ�����B
BOOL Lattice::AddNodesForSingle(const WCHAR *dict_data) {
    std::wstring sep;
    sep += FIELD_SEP;

    // ��{�������X�L��������B
    WStrings fields, records;
    size_t count = ScanBasicDict(records, dict_data, pre[0]);
    DebugPrintW(L"ScanBasicDict(%c) count: %d\n", pre[0], count);

    // ���[�U�[�������X�L��������B
    count = ScanUserDict(records, pre[0], this);
    DebugPrintW(L"ScanUserDict(%c) count: %d\n", pre[0], count);

    // store data for each record
    for (size_t k = 0; k < records.size(); ++k) {
        const std::wstring& record = records[k];
        str_split(fields, record, std::wstring(sep));
        DoFields(0, fields);
    }

    // delete the nodes of different size
    DeleteDifferentSizeNode del(pre.size());
    for (size_t i = 0; i < chunks[0].size(); ++i) {
        auto it = std::remove_if(chunks[0].begin(), chunks[0].end(), del);
        chunks[0].erase(it, chunks[0].end());
    }

    return !chunks[0].empty();
}

// �Q�Ƃ��X�V����B
void Lattice::UpdateRefs() {
    const size_t length = pre.size();

    // initialize the reference counts
    refs.assign(length + 1, 0);
    refs[0] = 1;

    // update the reference counts
    for (size_t index = 0; index < length; ++index) {
        if (refs[index] == 0) continue;
        LatticeChunk& chunk1 = chunks[index];
        for (size_t k = 0; k < chunk1.size(); ++k) {
            refs[index + chunk1[k]->pre.size()]++;
        }
    }
} // Lattice::UpdateRefs

// �����N���X�V����B
void Lattice::UpdateLinks() {
    const size_t length = pre.size();
    ASSERT(length);
    ASSERT(length + 1 == chunks.size());
    ASSERT(length + 1 == refs.size());

    UnlinkAllNodes(); // ���ׂẴm�[�h�̃����N����������B

    // add head and link to head
    {
        LatticeNode node;
        node.bunrui = HB_HEAD;
        node.linked = 1;
        LatticeChunk& chunk1 = chunks[0];
        for (size_t k = 0; k < chunk1.size(); ++k) {
            chunk1[k]->linked = 1;
            node.branches.push_back(chunk1[k]);
        }
        head = std::make_shared<LatticeNode>(node);
    }

    // add tail
    {
        LatticeNode node;
        node.bunrui = HB_TAIL;
        chunks[length].clear();
        chunks[length].push_back(std::make_shared<LatticeNode>(node));
    }

    // add links and branches
    size_t num_links = 0;
    for (size_t index = 0; index < length; ++index) {
        LatticeChunk& chunk1 = chunks[index];
        for (size_t k = 0; k < chunk1.size(); ++k) {
            if (!chunk1[k]->linked) continue;
            const std::wstring& pre = chunk1[k]->pre;
            LatticeChunk& chunk2 = chunks[index + pre.size()];
            for (size_t m = 0; m < chunk2.size(); ++m) {
                if (IsNodeConnectable(*chunk1[k].get(), *chunk2[m].get())) {
                    chunk1[k]->branches.push_back(chunk2[m]);
                    chunk2[m]->linked++;
                    num_links++;
                }
            }
        }
    }
} // Lattice::UpdateLinks

void Lattice::UnlinkAllNodes() {
    // clear the branch links and the linked counts
    const size_t length = pre.size();
    for (size_t index = 0; index < length; ++index) {
        LatticeChunk& chunk1 = chunks[index];
        for (size_t k = 0; k < chunk1.size(); ++k) {
            chunk1[k]->linked = 0;
            chunk1[k]->branches.clear();
        }
    }
} // Lattice::UnlinkAllNodes

void Lattice::AddComplement(size_t index, size_t min_size, size_t max_size) {
    const size_t length = pre.size();
    // add the undefined words on failure of conversion
    WStrings fields(4);
    fields[1].assign(1, MAKEWORD(HB_UNKNOWN, 0));
    //fields[3].clear();
    for (size_t count = min_size; count <= max_size; ++count) {
        if (length < index + count) continue;
        fields[0] = pre.substr(index, count);
        fields[2] = fields[0];
        DoFields(index, fields);
    }
} // Lattice::AddComplement

static inline bool IsNodeUnlinked(const LatticeNodePtr& node) {
    return node->linked == 0;
}

// �����N����Ă��Ȃ��m�[�h���폜�B
void Lattice::CutUnlinkedNodes() {
    const size_t length = pre.size();
    for (size_t index = 0; index < length; ++index) {
        LatticeChunk& chunk1 = chunks[index];
        auto it = std::remove_if(chunk1.begin(), chunk1.end(), IsNodeUnlinked);
        chunk1.erase(it, chunk1.end());
    }
} // Lattice::CutUnlinkedNodes

// �Ō�Ƀ����N���ꂽ�C���f�b�N�X���擾����B
size_t Lattice::GetLastLinkedIndex() const {
    // is the last node linked?
    const size_t length = pre.size();
    if (chunks[length][0]->linked) {
        return length; // return the last index
    }

    // scan chunks in reverse order
    for (size_t index = length; index > 0; ) {
        --index;
        for (size_t k = 0; k < chunks[index].size(); ++k) {
            if (chunks[index][k]->linked) {
                return index; // the linked node was found
            }
        }
    }
    return 0; // not found
} // Lattice::GetLastLinkedIndex

// �C�`�e����ϊ�����B
void Lattice::DoIkeiyoushi(size_t index, const WStrings& fields) {
    ASSERT(fields.size() == 4);
    ASSERT(fields[0].size());
    size_t length = fields[0].size();

    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    // get the right substring
    std::wstring str = pre.substr(index + length);

    LatticeNode node;
    node.bunrui = HB_IKEIYOUSHI;
    node.tags = fields[3];
    node.cost = node.CalcCost();

    // ���R�`
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.katsuyou = MIZEN_KEI;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    MakeLiteralMaps();

    // �A�p�`
    node.katsuyou = RENYOU_KEI;
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (fields[0].empty() || str.empty()) break;
        wchar_t ch0 = fields[0][fields[0].size() - 1];
        wchar_t ch1 = g_consonant_map[ch0];
        std::wstring addition;
        switch (g_vowel_map[ch0]) {
        case L'��':
            str = fields[1].substr(0, fields[1].size() - 1);
            for (size_t i = 0; i < _countof(s_hiragana_table); ++i) {
                if (s_hiragana_table[i][0] == ch1) {
                    addition = s_hiragana_table[i][DAN_O];;
                    addition += L'��';
                    str += addition;
                    break;
                }
            }
            node.post = str;
            str = fields[0].substr(0, fields[0].size() - 1);
            for (size_t i = 0; i < _countof(s_hiragana_table); ++i) {
                if (s_hiragana_table[i][0] == ch1) {
                    addition = s_hiragana_table[i][DAN_O];;
                    addition += L'��';
                    str += addition;
                    break;
                }
            }
            node.pre = str;
            if (str.empty() || str.substr(0, addition.size()) != addition) break;
            chunks[index].push_back(std::make_shared<LatticeNode>(node));
            refs[index + fields[0].size() + addition.size()]++;
            break;
        case L'��':
            if (str.empty() || str.substr(0, 2) != L"�イ") break;
            node.pre = fields[0] + L"�イ";
            node.post = fields[2] + L"�イ";
            chunks[index].push_back(std::make_shared<LatticeNode>(node));
            refs[index + fields[0].size() + 2]++;
        default:
            break;
        }
    } while(0);

    // �I�~�`
    node.katsuyou = SHUUSHI_KEI;
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �A�̌`
    node.katsuyou = RENTAI_KEI;
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // ����`
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    // �����`
    node.bunrui = HB_MEISHI;
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // (���`�e���̌ꊲ)+"������"
    if (str.size() >= 2 && str[0] == L'��' && str[1] == L'��') {
        WStrings new_fields = fields;
        new_fields[0] = fields[0] + L"����";
        new_fields[2] = fields[2] + L"����";
        DoNakeiyoushi(index, new_fields);
    }

    // �u�ɁB�v�u���B�v�Ȃ�
    if (str.size()) {
        switch (str[0]) {
        case L'�B': case L'�A': case L'�C': case L'�D': case 0:
            DoMeishi(index, fields);
            break;
        }
    }
} // Lattice::DoIkeiyoushi

// �i�`�e����ϊ�����B
void Lattice::DoNakeiyoushi(size_t index, const WStrings& fields) {
    ASSERT(fields.size() == 4);
    ASSERT(fields[0].size());
    size_t length = fields[0].size();
    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    // get the right substring
    std::wstring str = pre.substr(index + length);

    LatticeNode node;
    node.bunrui = HB_NAKEIYOUSHI;
    node.tags = fields[3];
    node.cost = node.CalcCost();

    // ���R�`
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.katsuyou = MIZEN_KEI;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    // �A�p�`
    node.katsuyou = RENYOU_KEI;
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �I�~�`
    do {
        if (str.empty() || str[0] != L'��') break;
        node.katsuyou = SHUUSHI_KEI;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �A�̌`
    do {
        if (str.empty() || str[0] != L'��') break;
        node.katsuyou = RENTAI_KEI;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // ����`
    do {
        if (str.empty() || str.substr(0, 2) != L"�Ȃ�") break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[0] + L"�Ȃ�";
        node.post = fields[2] + L"�Ȃ�";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    // �����`
    node.bunrui = HB_MEISHI;
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �u���ꂢ�B�v�u�Â��B�v�Ȃ�
    switch (str[0]) {
    case L'�B': case L'�A': case L'�C': case L'�D': case 0:
        DoMeishi(index, fields);
        break;
    }
} // Lattice::DoNakeiyoushi

// �ܒi������ϊ�����B
void Lattice::DoGodanDoushi(size_t index, const WStrings& fields) {
    ASSERT(fields.size() == 4);
    ASSERT(fields[0].size());
    size_t length = fields[0].size();
    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    // get the right substring
    std::wstring str = pre.substr(index + length);
    DebugPrintW(L"DoGodanDoushi: %s, %s\n", fields[0].c_str(), str.c_str());

    LatticeNode node;
    node.bunrui = HB_GODAN_DOUSHI;
    node.tags = fields[3];
    node.cost = node.CalcCost();

    WORD w = fields[1][0];
    node.gyou = (Gyou)HIBYTE(w);

    int type;
    switch (node.gyou) {
    case GYOU_KA: case GYOU_GA:                 type = 1; break;
    case GYOU_NA: case GYOU_BA: case GYOU_MA:   type = 2; break;
    case GYOU_TA: case GYOU_RA: case GYOU_WA:   type = 3; break;
    default:                                    type = 0; break;
    }

    // ���R�`
    do {
        node.katsuyou = MIZEN_KEI;
        if (node.gyou == GYOU_A) {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
            chunks[index].push_back(std::make_shared<LatticeNode>(node));
            refs[index + fields[0].size() + 1]++;
        } else {
            wchar_t ch = s_hiragana_table[node.gyou][DAN_A];
            if (str.empty() || str[0] != ch) break;
            node.pre = fields[0] + ch;
            node.post = fields[2] + ch;
            chunks[index].push_back(std::make_shared<LatticeNode>(node));
            refs[index + fields[0].size() + 1]++;
        }
    } while(0);

    // �A�p�`
    node.katsuyou = RENYOU_KEI;
    do {
        wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
        if (str.empty() || str[0] != ch) break;
        node.pre = fields[0] + ch;
        node.post = fields[2] + ch;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (type == 0) break;
        wchar_t ch;
        switch (type) {
        case 1:   ch = L'��'; break;
        case 2:   ch = L'��'; break;
        case 3:   ch = L'��'; break;
        }
        if (str.empty() || str[0] != ch) break;
        node.pre = fields[0] + ch;
        node.post = fields[2] + ch;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �I�~�`
    // �A�̌`
    do {
        wchar_t ch = s_hiragana_table[node.gyou][DAN_U];
        if (str.empty() || str[0] != ch) break;
        node.katsuyou = SHUUSHI_KEI;
        node.pre = fields[0] + ch;
        node.post = fields[2] + ch;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
        node.katsuyou = RENTAI_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // ����`
    // ���ߌ`
    do {
        wchar_t ch = s_hiragana_table[node.gyou][DAN_E];
        if (str.empty() || str[0] != ch) break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[0] + ch;
        node.post = fields[2] + ch;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
        node.katsuyou = MEIREI_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �����`
    node.bunrui = HB_MEISHI;
    do {
        wchar_t ch = s_hiragana_table[node.gyou][DAN_I];
        if (str.empty() || str[0] != ch) break;
        node.pre = fields[0] + ch;
        node.post = fields[2] + ch;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �u����(�ܒi)�v���u������(��i)�v�A
    // �u����(�ܒi)�v���u������(��i)�v�Ȃ�
    {
        WStrings new_fields = fields;
        new_fields[0] += s_hiragana_table[node.gyou][DAN_I];
        new_fields[2] += s_hiragana_table[node.gyou][DAN_I];
        DoIchidanDoushi(index, new_fields);
    }
} // Lattice::DoGodanDoushi

// ��i������ϊ�����B
void Lattice::DoIchidanDoushi(size_t index, const WStrings& fields) {
    ASSERT(fields.size() == 4);
    ASSERT(fields[0].size());
    size_t length = fields[0].size();
    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    // get the right substring
    std::wstring str = pre.substr(index + length);

    LatticeNode node;
    node.bunrui = HB_ICHIDAN_DOUSHI;
    node.tags = fields[3];
    node.cost = node.CalcCost();

    // ���R�`
    // �A�p�`
    do {
        node.katsuyou = MIZEN_KEI;
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size()]++;
        node.katsuyou = RENYOU_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size()]++;
    } while(0);

    // �I�~�`
    // �A�̌`
    do {
        if (str.empty() || str[0] != L'��') break;
        node.katsuyou = SHUUSHI_KEI;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
        node.katsuyou = RENTAI_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // ����`
    do {
        if (str.empty() || str[0] != L'��') break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // ���ߌ`
    node.katsuyou = MEIREI_KEI;
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �����`
    node.bunrui = HB_MEISHI;
    do {
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size()]++;
    } while(0);
} // Lattice::DoIchidanDoushi

// �J�ϓ�����ϊ�����B
void Lattice::DoKahenDoushi(size_t index, const WStrings& fields) {
    ASSERT(fields.size() == 4);
    ASSERT(fields[0].size());
    size_t length = fields[0].size();
    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    // get the right substring
    std::wstring str = pre.substr(index + length);

    LatticeNode node;
    node.bunrui = HB_KAHEN_DOUSHI;
    node.tags = fields[3];
    node.cost = node.CalcCost();

    // ���R�`
    do {
        if (str.empty() || str[0] != L'��') break;
        node.katsuyou = MIZEN_KEI;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �A�p�`
    do {
        if (str.empty() || str[0] != L'��') break;
        node.katsuyou = RENYOU_KEI;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �I�~�`
    // �A�p�`
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.katsuyou = SHUUSHI_KEI;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
        node.katsuyou = RENYOU_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    // ����`
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    // ���ߌ`
    do {
        if (str.empty() || str.substr(0, 2) != L"����") break;
        node.katsuyou = MEIREI_KEI;
        node.pre = fields[0] + L"����";
        node.post = fields[2] + L"����";
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    // �����`
    node.bunrui = HB_MEISHI;
    do {
        if (str.empty() || str[0] != L'��') break;
        node.pre = fields[0] + L'��';
        node.post = fields[2] + L'��';
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
} // Lattice::DoKahenDoushi

// �T�ϓ�����ϊ�����B
void Lattice::DoSahenDoushi(size_t index, const WStrings& fields) {
    ASSERT(fields.size() == 4);
    ASSERT(fields[0].size());
    size_t length = fields[0].size();
    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    // get the right substring
    std::wstring str = pre.substr(index + length);

    LatticeNode node;
    node.bunrui = HB_SAHEN_DOUSHI;
    node.tags = fields[3];
    node.cost = node.CalcCost();

    WORD w = fields[1][0];
    node.gyou = (Gyou)HIBYTE(w);

    // ���R�`
    node.katsuyou = MIZEN_KEI;
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        } else {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        }
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        } else {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        }
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        } else {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        }
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �A�p�`
    node.katsuyou = RENYOU_KEI;
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        } else {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        }
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // �I�~�`
    // �A�p�`
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        } else {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        }
        node.katsuyou = SHUUSHI_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;

        node.katsuyou = RENYOU_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        } else {
            if (str.empty() || str[0] != L'��') break;
            node.pre = fields[0] + L'��';
            node.post = fields[2] + L'��';
        }
        node.katsuyou = SHUUSHI_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 1]++;
    } while(0);

    // ����`
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        } else {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        }
        node.katsuyou = KATEI_KEI;
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);

    // ���ߌ`
    node.katsuyou = MEIREI_KEI;
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        } else {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        }
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);
    do {
        if (node.gyou == GYOU_ZA) {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        } else {
            if (str.empty() || str.substr(0, 2) != L"����") break;
            node.pre = fields[0] + L"����";
            node.post = fields[2] + L"����";
        }
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + fields[0].size() + 2]++;
    } while(0);
} // Lattice::DoSahenDoushi

void Lattice::DoMeishi(size_t index, const WStrings& fields, INT deltaCost) {
    ASSERT(fields.size() == 4);
    ASSERT(fields[0].size());

    size_t length = fields[0].size();
    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    // get the right substring
    std::wstring str = pre.substr(index + length);

    LatticeNode node;
    node.bunrui = HB_MEISHI;
    node.tags = fields[3];
    node.cost = node.CalcCost() + deltaCost;

    if (pre.substr(index, length) == fields[0]) {
        if (node.HasTag(L"[���A��]")) {
            // animals or plants can be written in katakana
            node.pre = fields[0];
            node.post = lcmap(fields[0], LCMAP_KATAKANA | LCMAP_FULLWIDTH);
            chunks[index].push_back(std::make_shared<LatticeNode>(node));
            refs[index + length]++;

            node.cost += 30;
            node.pre = fields[0];
            node.post = fields[2];
            chunks[index].push_back(std::make_shared<LatticeNode>(node));
            refs[index + length]++;
        } else {
            node.pre = fields[0];
            node.post = fields[2];
            chunks[index].push_back(std::make_shared<LatticeNode>(node));
            refs[index + length]++;
        }
    }

    // �����{�u���ۂ��v�Ō`�e����
    if (str.size() >= 2 && str[0] == L'��' && str[1] == L'��') {
        WStrings new_fields = fields;
        new_fields[0] += L"����";
        new_fields[2] += L"����";
        DoIkeiyoushi(index, new_fields);
    }
} // Lattice::DoMeishi

void Lattice::DoFields(size_t index, const WStrings& fields, int cost /* = 0*/) {
    ASSERT(fields.size() == 4);
    const size_t length = fields[0].size();
    // boundary check
    if (index + length > pre.size()) {
        return;
    }
    // check text matching
    if (pre.substr(index, length) != fields[0]) {
        return;
    }
    DebugPrintW(L"DoFields: %s\n", fields[0].c_str());

    // ���R�[�h�̎d�l�F
    //     fields[0]: std::wstring �ϊ��O������;
    //     fields[1]: { MAKEWORD(HinshiBunrui, Gyou), 0 };
    //     fields[2]: std::wstring �ϊ��㕶����;
    //     fields[3]: std::wstring tags;

    // initialize the node
    LatticeNode node;
    WORD w = fields[1][0];
    node.bunrui = (HinshiBunrui)LOBYTE(w);
    node.gyou = (Gyou)HIBYTE(w);
    node.tags = fields[3];
    node.cost = node.CalcCost() + cost;

    // add new entries by node classification (BUNRUI)
    switch (node.bunrui) {
    case HB_MEISHI:
        DoMeishi(index, fields);
        break;
    case HB_PERIOD: case HB_COMMA: case HB_SYMBOL:
    case HB_RENTAISHI: case HB_FUKUSHI:
    case HB_SETSUZOKUSHI: case HB_KANDOUSHI:
    case HB_KAKU_JOSHI: case HB_SETSUZOKU_JOSHI:
    case HB_FUKU_JOSHI: case HB_SHUU_JOSHI:
    case HB_KANGO: case HB_SETTOUJI: case HB_SETSUBIJI:
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + length]++;
        break;
    case HB_IKEIYOUSHI: // ���`�e���B
        DoIkeiyoushi(index, fields);
        break;
    case HB_NAKEIYOUSHI: // �Ȍ`�e���B
        DoNakeiyoushi(index, fields);
        break;
    case HB_MIZEN_JODOUSHI:
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = MIZEN_KEI;
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + length]++;
        break;
    case HB_RENYOU_JODOUSHI:
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = RENYOU_KEI;
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + length]++;
        break;
    case HB_SHUUSHI_JODOUSHI:
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = SHUUSHI_KEI;
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + length]++;
        break;
    case HB_RENTAI_JODOUSHI:
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = RENTAI_KEI;
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + length]++;
        break;
    case HB_KATEI_JODOUSHI:
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = KATEI_KEI;
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + length]++;
        break;
    case HB_MEIREI_JODOUSHI:
        node.bunrui = HB_JODOUSHI;
        node.katsuyou = MEIREI_KEI;
        node.pre = fields[0];
        node.post = fields[2];
        chunks[index].push_back(std::make_shared<LatticeNode>(node));
        refs[index + length]++;
        break;
    case HB_GODAN_DOUSHI:
        DoGodanDoushi(index, fields);
        break;
    case HB_ICHIDAN_DOUSHI:
        DoIchidanDoushi(index, fields);
        break;
    case HB_KAHEN_DOUSHI:
        DoKahenDoushi(index, fields);
        break;
    case HB_SAHEN_DOUSHI:
        DoSahenDoushi(index, fields);
        break;
    default:
        break;
    }
} // Lattice::DoFields

// ���e�B�X���_���v����B
void Lattice::Dump(int num) {
    const size_t length = pre.size();
    DebugPrintW(L"### Lattice::Dump(%d) ###\n", num);
    DebugPrintW(L"Lattice length: %d\n", int(length));
    for (size_t i = 0; i < length; ++i) {
        DebugPrintW(L"Lattice chunk #%d:", int(i));
        for (size_t k = 0; k < chunks[i].size(); ++k) {
            DebugPrintW(L" %s(%s)", chunks[i][k]->post.c_str(),
                        BunruiToString(chunks[i][k]->bunrui));
        }
        DebugPrintW(L"\n");
    }
} // Lattice::Dump

//////////////////////////////////////////////////////////////////////////////

BOOL Lattice::MakeLatticeInternal(size_t length, const WCHAR* dict_data)
{
    // link and cut not linked
    UpdateLinks();
    CutUnlinkedNodes();

    // does it reach the last?
    size_t index = GetLastLinkedIndex();
    if (index == length)
        return TRUE;

    // add complement
    UpdateRefs();
    AddComplement(index, 1, 5);
    AddNodes(index + 1, dict_data);

    return FALSE;
}

// �������ߕϊ��ɂ����āA���e�B�X���쐬����B
BOOL MzIme::MakeLatticeForMulti(Lattice& lattice, const std::wstring& pre) {
    // ��{�������ǂݍ��܂�Ă��Ȃ���Ύ��s�B
    if (!m_basic_dict.IsLoaded())
        return FALSE;

    ASSERT(pre.size() != 0);
    const size_t length = pre.size();

    // ���e�B�X���������B
    lattice.pre = pre; // �ϊ��O�̕�����B
    lattice.chunks.resize(length + 1);
    lattice.refs.assign(length + 1, 0);
    lattice.refs[0] = 1;

    size_t count = 0;
    const DWORD c_retry_count = 64; // �Ď��s�̍ő�񐔁B

    WCHAR *dict_data1 = m_basic_dict.Lock(); // ��{���������b�N�B
    if (dict_data1) {
        // �m�[�h��ǉ��B
        lattice.AddNodes(0, dict_data1);

        // repeat until linked to tail
        while (!lattice.MakeLatticeInternal(length, dict_data1)) {
            ++count;
            if (count >= c_retry_count)
                break;
        }

        m_basic_dict.Unlock(dict_data1); // ��{�����̃��b�N�������B
    }

    WCHAR *dict_data2 = m_name_dict.Lock(); // �l���E�n�����������b�N�B
    if (dict_data2) {
        // �m�[�h��ǉ��B
        lattice.AddNodes(0, dict_data2);

        // repeat until linked to tail
        while (!lattice.MakeLatticeInternal(length, dict_data2)) {
            ++count;
            if (count >= c_retry_count)
                break;
        }

        m_name_dict.Unlock(dict_data2); // �l���E�n�������̃��b�N�������B
    }

    if (count < c_retry_count)
        return TRUE; // �����B

    // �_���v�B
    lattice.Dump(4);
    return FALSE; // ���s�B
} // MzIme::MakeLatticeForMulti

// �P�ꕶ�ߕϊ��ɂ����āA���e�B�X���쐬����B
BOOL MzIme::MakeLatticeForSingle(Lattice& lattice, const std::wstring& pre) {
    // failure if the dictionary not loaded
    if (!m_basic_dict.IsLoaded()) {
        return FALSE;
    }

    ASSERT(pre.size() != 0);
    const size_t length = pre.size();

    // ���e�B�X���������B
    lattice.pre = pre;
    lattice.chunks.resize(length + 1);
    lattice.refs.assign(length + 1, 0);
    lattice.refs[0] = 1;

    WCHAR *dict_data = m_basic_dict.Lock(); // ��{���������b�N����B
    if (dict_data) {
        // �m�[�h�Q��ǉ��B
        if (!lattice.AddNodesForSingle(dict_data)) {
            lattice.AddComplement(0, pre.size(), pre.size());
        }

        m_basic_dict.Unlock(dict_data); // ��{�����̃��b�N�������B
        return TRUE; // success
    }

    // �_���v�B
    lattice.Dump(4);
    return FALSE; // ���s�B
} // MzIme::MakeLatticeForSingle

// �P�ꕶ�ߕϊ��ɂ����āA�ϊ����ʂ𐶐�����B
void MzIme::MakeResultForMulti(MzConvResult& result, Lattice& lattice) {
    result.clear(); // ���ʂ��N���A�B

    // 2���ߍŒ���v�@�E���B
    const size_t length = lattice.pre.size();
    LatticeNodePtr node1 = lattice.head;
    LatticeNodePtr tail = lattice.chunks[length][0];
    while (node1 != tail) {
        size_t kb1 = 0, max_len = 0, max_len1 = 0;
        for (size_t ib1 = 0; ib1 < node1->branches.size(); ++ib1) {
            LatticeNodePtr& node2 = node1->branches[ib1];
            if (node2->branches.empty()) {
                size_t len = node2->pre.size();
                // (doushi or jodoushi) + jodoushi
                if ((node1->IsDoushi() || node1->IsJodoushi()) && node2->IsJodoushi()) {
                    ++len;
                    node2->cost -= 80;
                }
                // jodoushi + shuu joshi
                if (node1->IsJodoushi() && node2->bunrui == HB_SHUU_JOSHI) {
                    ++len;
                    node2->cost -= 30;
                }
                // suushi + (suushi or number unit)
                if (node1->HasTag(L"[����]")) {
                    if (node2->HasTag(L"[����]") || node2->HasTag(L"[���P��]")) {
                        ++len;
                        node2->cost -= 100;
                    }
                }
                if (max_len < len) {
                    max_len1 = node2->pre.size();
                    max_len = len;
                    kb1 = ib1;
                }
            } else {
                for (size_t ib2 = 0; ib2 < node2->branches.size(); ++ib2) {
                    LatticeNodePtr& node3 = node2->branches[ib2];
                    size_t len = node2->pre.size() + node3->pre.size();
                    // (doushi or jodoushi) + jodoushi
                    if ((node1->IsDoushi() || node1->IsJodoushi()) && node2->IsJodoushi()) {
                        ++len;
                        node2->cost -= 80;
                    } else {
                        if ((node2->IsDoushi() || node2->IsJodoushi()) && node3->IsJodoushi()) {
                            ++len;
                            node2->cost -= 80;
                        }
                    }
                    // jodoushu + shuu joshi
                    if (node1->IsJodoushi() && node2->bunrui == HB_SHUU_JOSHI) {
                        ++len;
                        node2->cost -= 30;
                    } else {
                        if (node2->IsJodoushi() && node3->bunrui == HB_SHUU_JOSHI) {
                            ++len;
                            node2->cost -= 30;
                        }
                    }
                    // suushi + (suushi or number unit)
                    if (node1->HasTag(L"[����]")) {
                        if (node2->HasTag(L"[����]") || node2->HasTag(L"[���P��]")) {
                            ++len;
                            node2->cost -= 100;
                        }
                    } else {
                        if (node2->HasTag(L"[����]")) {
                            if (node3->HasTag(L"[����]") || node3->HasTag(L"[���P��]")) {
                                ++len;
                                node2->cost -= 100;
                            }
                        }
                    }
                    if (max_len < len) {
                        max_len1 = node2->pre.size();
                        max_len = len;
                        kb1 = ib1;
                    } else if (max_len == len) {
                        if (max_len1 < node2->pre.size()) {
                            max_len1 = node2->pre.size();
                            max_len = len;
                            kb1 = ib1;
                        }
                    }
                }
            }
        }

        // add clause
        if (node1->branches[kb1]->pre.size()) {
            MzConvClause clause;
            clause.add(node1->branches[kb1].get());
            result.clauses.push_back(clause);
        }

        // go next
        node1 = node1->branches[kb1];
    }

    // add other candidates
    size_t index = 0, iClause = 0;
    while (index < length && iClause < result.clauses.size()) {
        const LatticeChunk& chunk = lattice.chunks[index];
        MzConvClause& clause = result.clauses[iClause];

        std::wstring hiragana = clause.candidates[0].hiragana;
        const size_t size = hiragana.size();
        for (size_t i = 0; i < chunk.size(); ++i) {
            if (chunk[i]->pre.size() == size) {
                // add a candidate of same size
                clause.add(chunk[i].get());
            }
        }

        LatticeNode node;
        node.bunrui = HB_UNKNOWN;
        node.cost = 40; // �R�X�g�͐l���E�n��������������B
        node.pre = lcmap(hiragana, LCMAP_HIRAGANA | LCMAP_FULLWIDTH);

        // add hiragana
        node.post = lcmap(hiragana, LCMAP_HIRAGANA | LCMAP_FULLWIDTH);
        clause.add(&node);

        // add katakana
        node.post = lcmap(hiragana, LCMAP_KATAKANA | LCMAP_FULLWIDTH);
        clause.add(&node);

        // add halfwidth katakana
        node.post = lcmap(hiragana, LCMAP_KATAKANA | LCMAP_HALFWIDTH);
        clause.add(&node);

        // add the lowercase and fullwidth
        node.post = lcmap(hiragana, LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
        clause.add(&node);

        // add the uppercase and fullwidth
        node.post = lcmap(hiragana, LCMAP_UPPERCASE | LCMAP_FULLWIDTH);
        clause.add(&node);

        // add the capital and fullwidth
        node.post = node.post[0] + lcmap(hiragana.substr(1), LCMAP_LOWERCASE | LCMAP_FULLWIDTH);
        clause.add(&node);

        // add the lowercase and halfwidth
        node.post = lcmap(hiragana, LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
        clause.add(&node);

        // add the uppercase and halfwidth
        node.post = lcmap(hiragana, LCMAP_UPPERCASE | LCMAP_HALFWIDTH);
        clause.add(&node);

        // add the capital and halfwidth
        node.post = node.post[0] + lcmap(hiragana.substr(1), LCMAP_LOWERCASE | LCMAP_HALFWIDTH);
        clause.add(&node);

        // go to the next clause
        index += size;
        ++iClause;
    }

    // �R�X�g�ɂ��\�[�g����B
    result.sort();
} // MzIme::MakeResultForMulti

// �ϊ��Ɏ��s�����Ƃ��̌��ʂ��쐬����B
void MzIme::MakeResultOnFailure(MzConvResult& result, const std::wstring& pre) {
    MzConvClause clause; // ���߁B
    result.clear(); // ���ʂ��N���A�B

    // �m�[�h���������B
    LatticeNode node;
    node.pre = pre; // �ϊ��O�̕�����B
    node.cost = 40; // �R�X�g�͐l���E�n��������������B
    node.bunrui = HB_MEISHI; // �����B

    // ���߂ɖ��ϊ��������ǉ��B
    node.post = pre; // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɂЂ炪�Ȃ�ǉ��B
    node.post = lcmap(pre, LCMAP_HIRAGANA); // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɃJ�^�J�i��ǉ��B
    node.post = lcmap(pre, LCMAP_KATAKANA); // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɑS�p��ǉ��B
    node.post = lcmap(pre, LCMAP_FULLWIDTH); // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɔ��p��ǉ��B
    node.post = lcmap(pre, LCMAP_HALFWIDTH | LCMAP_KATAKANA); // �ϊ���̕�����B
    clause.add(&node);

    // ���ʂɕ��߂�ǉ��B
    result.clauses.push_back(clause);
} // MzIme::MakeResultOnFailure

// �P�ꕶ�ߕϊ��̌��ʂ��쐬����B
void MzIme::MakeResultForSingle(MzConvResult& result, Lattice& lattice) {
    result.clear(); // ���ʂ��N���A�B
    const size_t length = lattice.pre.size();

    // add other candidates
    MzConvClause clause;
    ASSERT(lattice.chunks.size());
    const LatticeChunk& chunk = lattice.chunks[0];
    for (size_t i = 0; i < chunk.size(); ++i) {
        if (chunk[i]->pre.size() == length) {
            // add a candidate of same size
            clause.add(chunk[i].get());
        }
    }

    // �m�[�h������������B
    std::wstring pre = lattice.pre; // �ϊ��O�̕�����B
    LatticeNode node;
    node.pre = pre;
    node.bunrui = HB_UNKNOWN;
    node.cost = 40; // �R�X�g�͐l���E�n��������������B

    // ���߂ɖ��ϊ��������ǉ��B
    node.post = pre; // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɂЂ炪�Ȃ�ǉ��B
    node.post = lcmap(pre, LCMAP_HIRAGANA); // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɃJ�^�J�i��ǉ��B
    node.post = lcmap(pre, LCMAP_KATAKANA); // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɑS�p��ǉ��B
    node.post = lcmap(pre, LCMAP_FULLWIDTH); // �ϊ���̕�����B
    clause.add(&node);

    // ���߂ɔ��p��ǉ��B
    node.post = lcmap(pre, LCMAP_HALFWIDTH | LCMAP_KATAKANA); // �ϊ���̕�����B
    clause.add(&node);

    // ���ʂɕ��߂�ǉ��B
    result.clauses.push_back(clause);
    ASSERT(result.clauses[0].candidates.size());

    // �R�X�g�ɂ��\�[�g����B
    result.sort();
    ASSERT(result.clauses[0].candidates.size());
} // MzIme::MakeResultForSingle

// �������߂�ϊ�����B
BOOL MzIme::ConvertMultiClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    MzConvResult result;
    std::wstring strHiragana = comp.extra.hiragana_clauses[comp.extra.iClause];
    if (!ConvertMultiClause(strHiragana, result)) {
        return FALSE;
    }
    return StoreResult(result, comp, cand);
} // MzIme::ConvertMultiClause

// �������߂�ϊ�����B
BOOL MzIme::ConvertMultiClause(const std::wstring& strHiragana, MzConvResult& result)
{
#if 1
    // failure if the dictionary not loaded
    if (!m_basic_dict.IsLoaded()) {
        return FALSE;
    }

    // make lattice and make result
    Lattice lattice;
    std::wstring pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
    if (MakeLatticeForMulti(lattice, pre)) {
        lattice.AddExtra();
        MakeResultForMulti(result, lattice);
    } else {
        lattice.AddExtra();
        MakeResultOnFailure(result, pre);
    }
#else
    // dummy sample
    WCHAR sz[64];
    result.clauses.clear();
    for (DWORD iClause = 0; iClause < 5; ++iClause) {
        MzConvClause clause;
        for (DWORD iCand = 0; iCand < 18; ++iCand) {
            MzConvCandidate cand;
            ::StringCchPrintfW(sz, _countof(sz), L"������%u-%u", iClause, iCand);
            cand.hiragana = sz;
            ::StringCchPrintfW(sz, _countof(sz), L"���%u-%u", iClause, iCand);
            cand.converted = sz;
            clause.candidates.push_back(cand);
        }
        result.clauses.push_back(clause);
    }
    result.clauses[0].candidates[0].hiragana = L"�Ђ��[";
    result.clauses[0].candidates[0].converted = L"�q�����[";
    result.clauses[1].candidates[0].hiragana = L"�Ƃ���";
    result.clauses[1].candidates[0].converted = L"�g�����v";
    result.clauses[2].candidates[0].hiragana = L"���񂾁[��";
    result.clauses[2].candidates[0].converted = L"�T���_�[�X";
    result.clauses[3].candidates[0].hiragana = L"�������";
    result.clauses[3].candidates[0].converted = L"�ЎR";
    result.clauses[4].candidates[0].hiragana = L"�����イ����";
    result.clauses[4].candidates[0].converted = L"�F���l";
#endif

    return TRUE;
} // MzIme::ConvertMultiClause

// �P�ꕶ�߂�ϊ�����B
BOOL MzIme::ConvertSingleClause(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    DWORD iClause = comp.extra.iClause;

    // convert
    MzConvResult result;
    std::wstring strHiragana = comp.extra.hiragana_clauses[iClause];
    if (!ConvertSingleClause(strHiragana, result)) {
        return FALSE;
    }

    // setting composition
    result.clauses.resize(1);
    MzConvClause& clause = result.clauses[0];
    comp.SetClauseCompString(iClause, clause.candidates[0].converted);
    comp.SetClauseCompHiragana(iClause, clause.candidates[0].hiragana, bRoman);

    // setting cand
    LogCandList cand_list;
    for (size_t i = 0; i < clause.candidates.size(); ++i) {
        MzConvCandidate& cand = clause.candidates[i];
        cand_list.cand_strs.push_back(cand.converted);
    }
    cand.cand_lists[iClause] = cand_list;
    cand.iClause = iClause;

    comp.extra.iClause = iClause;

    return TRUE;
} // MzIme::ConvertSingleClause

// �P�ꕶ�߂�ϊ�����B
BOOL MzIme::ConvertSingleClause(const std::wstring& strHiragana, MzConvResult& result)
{
    result.clear();

#if 1
    // make lattice and make result
    Lattice lattice;
    std::wstring pre = lcmap(strHiragana, LCMAP_FULLWIDTH | LCMAP_HIRAGANA);
    MakeLatticeForSingle(lattice, pre);
    lattice.AddExtra();
    MakeResultForSingle(result, lattice);
#else
    // dummy sample
    MzConvCandidate cand;
    cand.hiragana = L"���񂢂Ԃ񂹂ւ񂩂�";
    cand.converted = L"�P�ꕶ�ߕϊ�1";
    result.candidates.push_back(cand);
    cand.hiragana = L"���񂢂Ԃ񂹂ւ񂩂�";
    cand.converted = L"�P�ꕶ�ߕϊ�2";
    result.candidates.push_back(cand);
    cand.hiragana = L"���񂢂Ԃ񂹂ւ񂩂�";
    cand.converted = L"�P�ꕶ�ߕϊ�3";
    result.candidates.push_back(cand);
#endif
    return TRUE;
} // MzIme::ConvertSingleClause

// ���߂����ɐL�k����B
BOOL MzIme::StretchClauseLeft(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    DWORD iClause = comp.extra.iClause;

    // get the clause string
    std::wstring str1 = comp.extra.hiragana_clauses[iClause];
    if (str1.size() <= 1) return FALSE;

    // get the last character of this clause
    wchar_t ch = str1[str1.size() - 1];
    // shrink
    str1.resize(str1.size() - 1);

    // get the next clause string and add the character
    std::wstring str2;
    BOOL bSplitted = FALSE;
    if (iClause + 1 < comp.GetClauseCount()) {
        str2 = ch + comp.extra.hiragana_clauses[iClause + 1];
    } else {
        str2 += ch;
        bSplitted = TRUE;
    }

    // convert two clauses
    MzConvResult result1, result2;
    if (!ConvertSingleClause(str1, result1)) {
        return FALSE;
    }
    if (!ConvertSingleClause(str2, result2)) {
        return FALSE;
    }

    // add one clause if the clause was splitted
    if (bSplitted) {
        std::wstring str;
        comp.extra.hiragana_clauses.insert(
                comp.extra.hiragana_clauses.begin() + iClause + 1, str);
        comp.extra.comp_str_clauses.insert(
                comp.extra.comp_str_clauses.begin() + iClause + 1, str);
    }

    // seting composition
    MzConvClause& clause1 = result1.clauses[0];
    MzConvClause& clause2 = result2.clauses[0];
    comp.extra.hiragana_clauses[iClause] = str1;
    comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].converted;
    comp.extra.hiragana_clauses[iClause + 1] = str2;
    comp.extra.comp_str_clauses[iClause + 1] = clause2.candidates[0].converted;
    comp.UpdateFromExtra(bRoman);

    // seting the candidate list
    {
        LogCandList cand_list;
        for (size_t i = 0; i < clause1.candidates.size(); ++i) {
            MzConvCandidate& cand = clause1.candidates[i];
            cand_list.cand_strs.push_back(cand.converted);
        }
        cand.cand_lists[iClause] = cand_list;
    }
    {
        LogCandList cand_list;
        for (size_t i = 0; i < clause2.candidates.size(); ++i) {
            MzConvCandidate& cand = clause2.candidates[i];
            cand_list.cand_strs.push_back(cand.converted);
        }
        if (bSplitted) {
            cand.cand_lists.push_back(cand_list);
        } else {
            cand.cand_lists[iClause + 1] = cand_list;
        }
    }

    // set the current clause
    cand.iClause = iClause;
    comp.extra.iClause = iClause;
    // set the clause attributes
    comp.SetClauseAttr(iClause, ATTR_TARGET_CONVERTED);

    return TRUE;
} // MzIme::StretchClauseLeft

// ���߂��E�ɐL�k����B
BOOL MzIme::StretchClauseRight(LogCompStr& comp, LogCandInfo& cand, BOOL bRoman)
{
    DWORD iClause = comp.extra.iClause;

    // get the current clause
    std::wstring str1 = comp.extra.hiragana_clauses[iClause];
    // we cannot stretch right if the clause was the right end
    if (iClause == comp.GetClauseCount() - 1) return FALSE;

    // get the next clause
    std::wstring str2 = comp.extra.hiragana_clauses[iClause + 1];
    if (str2.empty()) return FALSE;

    // get the first character of the second clause
    wchar_t ch = str2[0];
    // add the character to the first clause
    str1 += ch;
    if (str2.size() == 1) {
        str2.clear();
    } else {
        str2 = str2.substr(1);
    }

    // convert
    MzConvResult result1, result2;
    if (!ConvertSingleClause(str1, result1)) {
        return FALSE;
    }
    if (str2.size() && !ConvertSingleClause(str2, result2)) {
        return FALSE;
    }

    MzConvClause& clause1 = result1.clauses[0];

    // if the second clause was joined?
    if (str2.empty()) {
        // delete the joined clause
        comp.extra.hiragana_clauses.erase(
                comp.extra.hiragana_clauses.begin() + iClause + 1);
        comp.extra.comp_str_clauses.erase(
                comp.extra.comp_str_clauses.begin() + iClause + 1);
        comp.extra.hiragana_clauses[iClause] = str1;
        comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].converted;
    } else {
        // seting two clauses
        MzConvClause& clause2 = result2.clauses[0];
        comp.extra.hiragana_clauses[iClause] = str1;
        comp.extra.comp_str_clauses[iClause] = clause1.candidates[0].converted;
        comp.extra.hiragana_clauses[iClause + 1] = str2;
        comp.extra.comp_str_clauses[iClause + 1] = clause2.candidates[0].converted;
    }
    // update composition by extra
    comp.UpdateFromExtra(bRoman);

    // seting the candidate list
    {
        LogCandList cand_list;
        for (size_t i = 0; i < clause1.candidates.size(); ++i) {
            MzConvCandidate& cand = clause1.candidates[i];
            cand_list.cand_strs.push_back(cand.converted);
        }
        cand.cand_lists[iClause] = cand_list;
    }
    if (str2.size()) {
        MzConvClause& clause2 = result2.clauses[0];
        LogCandList cand_list;
        for (size_t i = 0; i < clause2.candidates.size(); ++i) {
            MzConvCandidate& cand = clause2.candidates[i];
            cand_list.cand_strs.push_back(cand.converted);
        }
        cand.cand_lists[iClause + 1] = cand_list;
    }

    // set the current clause
    cand.iClause = iClause;
    comp.extra.iClause = iClause;
    // set the clause attributes
    comp.SetClauseAttr(iClause, ATTR_TARGET_CONVERTED);

    return TRUE;
} // MzIme::StretchClauseRight

// Shift_JIS�̃}���`�o�C�g������1�o�C�g�ڂ��H
inline bool is_sjis_lead(BYTE ch) {
    return (((0x81 <= ch) && (ch <= 0x9F)) || ((0xE0 <= ch) && (ch <= 0xEF)));
}

// Shift_JIS�̃}���`�o�C�g������2�o�C�g�ڂ��H
inline bool is_sjis_trail(BYTE ch) {
    return (((0x40 <= ch) && (ch <= 0x7E)) || ((0x80 <= ch) && (ch <= 0xFC)));
}

// JIS�o�C�g���H
inline bool is_jis_byte(BYTE ch) {
    return ((0x21 <= ch) && (ch <= 0x7E));
}

// JIS�R�[�h���H
inline bool is_jis_code(WORD w) {
    BYTE ch0 = BYTE(w >> 8);
    BYTE ch1 = BYTE(w);
    return (is_jis_byte(ch0) && is_jis_byte(ch1));
}

// JIS�R�[�h��Shift_JIS�ɕϊ�����B
inline WORD jis2sjis(BYTE c0, BYTE c1) {
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

// JIS�R�[�h��Shift_JIS�R�[�h�ɕϊ�����B
inline WORD jis2sjis(WORD jis_code) {
    BYTE c0 = BYTE(jis_code >> 8);
    BYTE c1 = BYTE(jis_code);
    return jis2sjis(c0, c1);
}

// Shift_JIS�R�[�h���H
inline bool is_sjis_code(WORD w) {
    return is_sjis_lead(BYTE(w >> 8)) && is_sjis_trail(BYTE(w));
}

// ��_����JIS�R�[�h�ɕϊ��B
inline WORD kuten_to_jis(const std::wstring& str) {
    if (str.size() != 5) return 0; // �ܕ����łȂ���΋�_�R�[�h�ł͂Ȃ��B
    std::wstring ku_bangou = str.substr(0, 3); // ��ԍ��B
    std::wstring ten_bangou = str.substr(3, 2); // �_�ԍ��B
    WORD ku = WORD(wcstoul(ku_bangou.c_str(), NULL, 10)); // ��ԍ���10�i���Ƃ��ĉ��߁B
    WORD ten = WORD(wcstoul(ten_bangou.c_str(), NULL, 10)); // �_�ԍ���10�i���Ƃ��ĉ��߁B
    WORD jis_code = (ku + 32) * 256 + ten + 32; // ��Ɠ_�ɂ��JIS�R�[�h���v�Z�B
    return jis_code;
}

// �R�[�h�ϊ��B
BOOL MzIme::ConvertCode(const std::wstring& strTyping, MzConvResult& result)
{
    result.clauses.clear();
    MzConvClause clause;

    // �m�[�h���������B
    LatticeNode node;
    node.pre = strTyping;
    node.bunrui = HB_UNKNOWN;
    node.cost = 0;

    // 16�i��ǂݍ��݁B
    ULONG hex_code = wcstoul(strTyping.c_str(), NULL, 16);

    // Unicode�̃m�[�h�𕶐߂ɒǉ��B
    WCHAR szUnicode[2];
    szUnicode[0] = WCHAR(hex_code);
    szUnicode[1] = 0;
    node.post = szUnicode; // �ϊ���̕�����B
    clause.add(&node);
    node.cost++; // �R�X�g���P���Z�B

    // Shift_JIS�R�[�h�̃m�[�h�𕶐߂ɒǉ��B
    CHAR szSJIS[8];
    WORD wSJIS = WORD(hex_code);
    if (is_sjis_code(wSJIS)) {
        szSJIS[0] = HIBYTE(wSJIS);
        szSJIS[1] = LOBYTE(wSJIS);
        szSJIS[2] = 0;
        ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
        node.post = szUnicode; // �ϊ���̕�����B
        node.cost++; // �R�X�g���P���Z�B
        clause.add(&node);
    }

    // JIS�R�[�h�̃m�[�h�𕶐߂ɒǉ��B
    if (is_jis_code(WORD(hex_code))) {
        wSJIS = jis2sjis(WORD(hex_code));
        if (is_sjis_code(wSJIS)) {
            szSJIS[0] = HIBYTE(wSJIS);
            szSJIS[1] = LOBYTE(wSJIS);
            szSJIS[2] = 0;
            ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
            node.post = szUnicode; // �ϊ���̕�����B
            node.cost++; // �R�X�g���P���Z�B
            clause.add(&node);
        }
    }

    // ��_�R�[�h�̃m�[�h�𕶐߂ɒǉ��B
    WORD wJIS = kuten_to_jis(strTyping);
    if (is_jis_code(wJIS)) {
        wSJIS = jis2sjis(wJIS);
        if (is_sjis_code(wSJIS)) {
            szSJIS[0] = HIBYTE(wSJIS);
            szSJIS[1] = LOBYTE(wSJIS);
            szSJIS[2] = 0;
            ::MultiByteToWideChar(932, 0, szSJIS, -1, szUnicode, 2);
            node.post = szUnicode; // �ϊ���̕�����B
            node.cost++; // �R�X�g���P���Z�B
            clause.add(&node);
        }
    }

    // ���̓��͕�����̃m�[�h�𕶐߂ɒǉ��B
    node.post = strTyping; // �ϊ���̕�����B
    node.cost++; // �R�X�g���P���Z�B
    clause.add(&node);

    result.clauses.push_back(clause);
    return TRUE;
} // MzIme::ConvertCode

// �R�[�h�ϊ��B
BOOL MzIme::ConvertCode(LogCompStr& comp, LogCandInfo& cand) {
    MzConvResult result;
    std::wstring strTyping = comp.extra.typing_clauses[comp.extra.iClause];
    if (!ConvertCode(strTyping, result)) {
        return FALSE;
    }
    return StoreResult(result, comp, cand);
} // MzIme::ConvertCode

// ���ʂ��i�[����B
BOOL MzIme::StoreResult(const MzConvResult& result, LogCompStr& comp, LogCandInfo& cand)
{
    comp.comp_str.clear();
    comp.extra.clear();

    // setting composition
    comp.comp_clause.resize(result.clauses.size() + 1);
    for (size_t k = 0; k < result.clauses.size(); ++k) {
        const MzConvClause& clause = result.clauses[k];
        for (size_t i = 0; i < clause.candidates.size(); ++i) {
            const MzConvCandidate& cand = clause.candidates[i];
            comp.comp_clause[k] = (DWORD)comp.comp_str.size();
            comp.extra.hiragana_clauses.push_back(cand.hiragana);
            std::wstring typing;
            typing = hiragana_to_typing(cand.hiragana);
            comp.extra.typing_clauses.push_back(typing);
            comp.comp_str += cand.converted;
            break;
        }
    }
    comp.comp_clause[result.clauses.size()] = (DWORD)comp.comp_str.size();
    comp.comp_attr.assign(comp.comp_str.size(), ATTR_CONVERTED);
    comp.extra.iClause = 0;
    comp.SetClauseAttr(comp.extra.iClause, ATTR_TARGET_CONVERTED);
    comp.dwCursorPos = (DWORD)comp.comp_str.size();
    comp.dwDeltaStart = 0;

    // setting cand
    cand.clear();
    for (size_t k = 0; k < result.clauses.size(); ++k) {
        const MzConvClause& clause = result.clauses[k];
        LogCandList cand_list;
        for (size_t i = 0; i < clause.candidates.size(); ++i) {
            const MzConvCandidate& cand = clause.candidates[i];
            cand_list.cand_strs.push_back(cand.converted);
        }
        cand.cand_lists.push_back(cand_list);
    }
    cand.iClause = 0;
    return TRUE;
} // MzIme::StoreResult

//////////////////////////////////////////////////////////////////////////////
