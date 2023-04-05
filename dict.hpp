#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif

// The separators.
// �����̋�؂�B
#define RECORD_SEP   L'\uFFFD'
#define FIELD_SEP    L'\uFFFC'

// Wide strings
typedef std::vector<std::wstring> WStrings;

// �s�B
enum Gyou {
    GYOU_A,     // ���s�B
    GYOU_KA,    // ���s�B
    GYOU_GA,    // ���s�B
    GYOU_SA,    // ���s�B
    GYOU_ZA,    // ���s�B
    GYOU_TA,    // ���s�B
    GYOU_DA,    // ���s�B
    GYOU_NA,    // �ȍs�B
    GYOU_HA,    // �͍s�B
    GYOU_BA,    // �΍s�B
    GYOU_PA,    // �ύs�B
    GYOU_MA,    // �܍s�B
    GYOU_YA,    // ��s�B
    GYOU_RA,    // ��s�B
    GYOU_WA,    // ��s�B
    GYOU_NN     // ��s�B
};

// �i�B
enum Dan {
    DAN_A,      // ���i�B
    DAN_I,      // ���i�B
    DAN_U,      // ���i�B
    DAN_E,      // ���i�B
    DAN_O       // ���i�B
};

// �i�����ށB
enum HinshiBunrui {
    HB_HEAD = 0x21,         // �ŏ��̃m�[�h
    HB_TAIL,                // �Ō�̃m�[�h
    HB_UNKNOWN,             // ���m�̕i��
    HB_MEISHI,              // ����
    HB_IKEIYOUSHI,          // ���`�e��
    HB_NAKEIYOUSHI,         // �Ȍ`�e��
    HB_RENTAISHI,           // �A�̎�
    HB_FUKUSHI,             // ����
    HB_SETSUZOKUSHI,        // �ڑ���
    HB_KANDOUSHI,           // ������
    HB_KAKU_JOSHI,          // �i����
    HB_SETSUZOKU_JOSHI,     // �ڑ�����
    HB_FUKU_JOSHI,          // ������
    HB_SHUU_JOSHI,          // �I����
    HB_JODOUSHI,            // ������
    HB_MIZEN_JODOUSHI,      // ���R������
    HB_RENYOU_JODOUSHI,     // �A�p������
    HB_SHUUSHI_JODOUSHI,    // �I�~������
    HB_RENTAI_JODOUSHI,     // �A�̏�����
    HB_KATEI_JODOUSHI,      // ���菕����
    HB_MEIREI_JODOUSHI,     // ���ߏ�����
    HB_GODAN_DOUSHI,        // �ܒi����
    HB_ICHIDAN_DOUSHI,      // ��i����
    HB_KAHEN_DOUSHI,        // �J�ϓ���
    HB_SAHEN_DOUSHI,        // �T�ϓ���
    HB_KANGO,               // ����
    HB_SETTOUJI,            // �ړ���
    HB_SETSUBIJI,           // �ڔ���
    HB_PERIOD,              // ��_�i�B�j
    HB_COMMA,               // �Ǔ_�i�A�j
    HB_SYMBOL,              // �L����
    HB_MAX = HB_SYMBOL
}; // enum HinshiBunrui

// �������p�`�B
enum KatsuyouKei {
    MIZEN_KEI,      // ���R�`
    RENYOU_KEI,     // �A�p�`
    SHUUSHI_KEI,    // �I�~�`
    RENTAI_KEI,     // �A�̌`
    KATEI_KEI,      // ����`
    MEIREI_KEI      // ���ߌ`
};

// �����̍��ځB
struct DictEntry {
    std::wstring pre;       // �ϊ��O�B
    std::wstring post;      // �ϊ���B
    HinshiBunrui bunrui;    // �i�����ށB
    std::wstring tags;      // �^�O�B
    Gyou gyou;              // ���p�̍s�B
};

struct LatticeNode;
typedef std::shared_ptr<LatticeNode>  LatticeNodePtr;

// ���e�B�X�ilattice�j�m�[�h�B
struct LatticeNode {
    std::wstring pre;                       // �ϊ��O�B
    std::wstring post;                      // �ϊ���B
    std::wstring tags;                      // �^�O�B
    HinshiBunrui bunrui;                    // ���ށB
    Gyou gyou;                              // ���p�̍s�B
    KatsuyouKei katsuyou;                   // �������p�`�B
    int cost;                               // �R�X�g�B
    DWORD linked;                           // �����N��B
    std::vector<LatticeNodePtr> branches;   // �}������B
    LatticeNode() {
        cost = 0;
        linked = 0;
    }
    int CalcCost() const;       // �R�X�g���v�Z�B
    bool IsDoushi() const;      // �������H
    bool IsJodoushi() const;    // ���������H

    // �w�肵���^�O�����邩�H
    bool HasTag(const wchar_t *tag) const {
        return tags.find(tag) != std::wstring::npos;
    }
};
typedef std::vector<LatticeNodePtr>   LatticeChunk;

// ���e�B�X�B
struct Lattice {
    size_t                          index;  // �C���f�b�N�X�B
    std::wstring                    pre;    // �ϊ��O�B
    LatticeNodePtr                  head;   // �擪�m�[�h�B
    std::vector<LatticeChunk>       chunks; // �`�����N�B
    std::vector<DWORD>              refs;   // �Q�ƁB
    // pre.size() + 1 == chunks.size().
    // pre.size() + 1 == refs.size().

    BOOL AddNodes(size_t index, const WCHAR *dict_data);
    BOOL AddNodesForSingle(const WCHAR *dict_data);
    void UpdateRefs();
    void UnlinkAllNodes();
    void UpdateLinks();
    void AddComplement(size_t index, size_t min_size, size_t max_size);
    void CutUnlinkedNodes();
    size_t GetLastLinkedIndex() const;

    void DoFields(size_t index, const WStrings& fields, int cost = 0);

    void DoMeishi(size_t index, const WStrings& fields);
    void DoIkeiyoushi(size_t index, const WStrings& fields);
    void DoNakeiyoushi(size_t index, const WStrings& fields);
    void DoGodanDoushi(size_t index, const WStrings& fields);
    void DoIchidanDoushi(size_t index, const WStrings& fields);
    void DoKahenDoushi(size_t index, const WStrings& fields);
    void DoSahenDoushi(size_t index, const WStrings& fields);

    void Dump(int num = 0);
    void Fix(const std::wstring& pre);
    void AddExtra();
    BOOL MakeLatticeInternal(size_t length, const WCHAR *dict_data);
};
